#include "Hierarchical4DDecoder.h"
/*******************************************************************************/
/*                        Hierachical4DDeccoder class methods                  */
/*******************************************************************************/

Hierarchical4DDecoder :: Hierarchical4DDecoder(void) {
    mSuperiorBitPlane = 30;
    mInferiorBitPlane = 0;
    mPreSegmentation = 1;
    mSegmentationFlagProbabilityModelIndex = SEGMENTATION_PROB_MODEL_INDEX;
    mSymbolProbabilityModelIndex = SYMBOL_PROBABILITY_MODEL_INDEX;
    mPredictionFlagProbabilityModelIndex = PREDICTION_PROB_MODEL_INDEX;
    mPmodel = NULL;
    
}
Hierarchical4DDecoder :: ~Hierarchical4DDecoder(void) {
    
    if(mPmodel != NULL)
        delete [] mPmodel;
    
}

void Hierarchical4DDecoder :: StartDecoder(FILE *inputFilePointer) {
    
    
    mEntropyDecoder.InitDecoder(inputFilePointer);  //initializes entropy decoder
    
    if(mPmodel != NULL)
        delete [] mPmodel;
    
    mPmodel = new ProbabilityModel[NUMBER_OF_MODELS];
    for(int n = 0; n < NUMBER_OF_MODELS; n++) {
         mPmodel[n].ResetModel();
    }
    
}

void Hierarchical4DDecoder :: RestartProbabilisticModel(void) {

    for(int n = 0; n < NUMBER_OF_MODELS; n++) {
         mPmodel[n].ResetModel();
    }

}

void Hierarchical4DDecoder :: DecodeBlock(int position_t, int position_s, int position_v, int position_u, int length_t, int length_s, int length_v, int length_u, int bitplane) {
    
    if(bitplane < mInferiorBitPlane) {
        return;
    }
    
    if(length_t*length_s*length_v*length_u == 1) {
        int coefficient =  DecodeCoefficient(bitplane);     

        mSubbandLF.mPixel[position_t][position_s][position_v][position_u] = coefficient;
        return;
    }
    
    int Significance = DecodeSegmentationFlag(bitplane);
            
    if(Significance == 0) {
        
        DecodeBlock(position_t, position_s, position_v, position_u, length_t, length_s, length_v, length_u, bitplane-1);
        
        return;
    }
    if(Significance == 1) {
                
        int half_length_t = (length_t > 1) ? length_t/2 : 1;
        int half_length_s = (length_s > 1) ? length_s/2 : 1;
        int half_length_v = (length_v > 1) ? length_v/2 : 1;
        int half_length_u = (length_u > 1) ? length_u/2 : 1;
        
        int number_of_subdivisions_t = (length_t > 1) ? 2 : 1;
        int number_of_subdivisions_s = (length_s > 1) ? 2 : 1;
        int number_of_subdivisions_v = (length_v > 1) ? 2 : 1;
        int number_of_subdivisions_u = (length_u > 1) ? 2 : 1;
        
        for(int index_t = 0; index_t < number_of_subdivisions_t; index_t++) {
            
            for(int index_s = 0; index_s < number_of_subdivisions_s; index_s++) {
                
                for(int index_v = 0; index_v < number_of_subdivisions_v; index_v++) {
                    
                    for(int index_u = 0; index_u < number_of_subdivisions_u; index_u++) {
                        
                            
                        int new_position_t = position_t+index_t*half_length_t;
                        int new_position_s = position_s+index_s*half_length_s;
                        int new_position_v = position_v+index_v*half_length_v;
                        int new_position_u = position_u+index_u*half_length_u;
                        
                        int new_length_t = (index_t == 0) ? half_length_t : (length_t-half_length_t);
                        int new_length_s = (index_s == 0) ? half_length_s : (length_s-half_length_s);
                        int new_length_v = (index_v == 0) ? half_length_v : (length_v-half_length_v);
                        int new_length_u = (index_u == 0) ? half_length_u : (length_u-half_length_u);
                                           
                        DecodeBlock(new_position_t, new_position_s, new_position_v, new_position_u, new_length_t, new_length_s, new_length_v, new_length_u, bitplane);
                
                   }
                    
                }
                
            }
            
        }
       
        return;
    }
    if(Significance == 2) {
       
        for(int index_t = 0; index_t < length_t; index_t++) {
            for(int index_s = 0; index_s < length_s; index_s++) {
                for(int index_v = 0; index_v < length_v; index_v++) {
                    for(int index_u = 0; index_u < length_u; index_u++) {
                        mSubbandLF.mPixel[position_t+index_t][position_s+index_s][position_v+index_v][position_u+index_u] = 0;
                    }

                }

            }
           
        }
        return;
    }
}

int Hierarchical4DDecoder :: DecodeCoefficient(int bitplane) {
    
        int magnitude = 0;
        int bit;
        int bit_position=bitplane;
        
        for(bit_position = bitplane; bit_position >= mInferiorBitPlane; bit_position--) {
            
            magnitude = magnitude << 1;
            bit = mEntropyDecoder.DecodeBit(mPmodel[bit_position+mSymbolProbabilityModelIndex]);
            if(bit_position > BITPLANE_BYPASS) 
                mPmodel[bit_position+mSymbolProbabilityModelIndex].UpdateModel(bit);
            
            if(bit == 1) {
                magnitude++;
            }
            
        }

        magnitude = magnitude << mInferiorBitPlane;
        if(magnitude > 0) {
            magnitude += (1 << mInferiorBitPlane)/2;
            if(mEntropyDecoder.DecodeBit(mPmodel[0]) == 1) {
                magnitude = -magnitude;
            }
        }            
        

        return(magnitude);
}

int Hierarchical4DDecoder :: DecodeSegmentationFlag(int bitplane)  {

    int bit0=0;
    int bit1 = mEntropyDecoder.DecodeBit(mPmodel[2*bitplane+mSegmentationFlagProbabilityModelIndex]);
    if(bitplane > BITPLANE_BYPASS_FLAGS) 
        mPmodel[2*bitplane+mSegmentationFlagProbabilityModelIndex].UpdateModel(bit1);
    if(bit1 == 0) {
        bit0 = mEntropyDecoder.DecodeBit(mPmodel[2*bitplane+1+mSegmentationFlagProbabilityModelIndex]);
        if(bitplane > BITPLANE_BYPASS_FLAGS) 
            mPmodel[2*bitplane+1+mSegmentationFlagProbabilityModelIndex].UpdateModel(bit0);
    }
    return(bit0+2*bit1);
  
}

int Hierarchical4DDecoder :: DecodePartitionFlag(void)  {

    int flag = mEntropyDecoder.DecodeBit(mPmodel[0]);
    if(flag == 1) {
        int bit = mEntropyDecoder.DecodeBit(mPmodel[0]);
        if(bit == 1)
            flag++;
    }
    return(flag);
        
}

int Hierarchical4DDecoder :: DecodePredictionFlag(int tree_depth)  {

    int bit0=0;
    int bit1 = mEntropyDecoder.DecodeBit(mPmodel[2*tree_depth+mPredictionFlagProbabilityModelIndex]);
    mPmodel[2*tree_depth+mPredictionFlagProbabilityModelIndex].UpdateModel(bit1);
    if(bit1 == 0) {
        bit0 = mEntropyDecoder.DecodeBit(mPmodel[2*tree_depth+1+mPredictionFlagProbabilityModelIndex]);
        mPmodel[2*tree_depth+1+mPredictionFlagProbabilityModelIndex].UpdateModel(bit0);
    }
    return(bit0+2*bit1);
  
}

int Hierarchical4DDecoder :: DecodeInteger(int precision)  {

    int integerValue = 0;
    for(int n = 0; n < precision; n++) {
        int bit = mEntropyDecoder.DecodeBit(mPmodel[0]);
        integerValue = (integerValue << 1) + bit;
    }
    return(integerValue);
        
}


void Hierarchical4DDecoder :: DoneDecoding(void) {
    
    mEntropyDecoder.Finish();      
    
}

  