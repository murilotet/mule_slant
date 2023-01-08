#include "Hierarchical4DEncoder.h"
#include <string.h>
#include <stdlib.h>
/*******************************************************************************/
/*                        Hierachical4DEncoder class methods                   */
/*******************************************************************************/

Hierarchical4DEncoder :: Hierarchical4DEncoder(void) {
    mSuperiorBitPlane = 30;
    mInferiorBitPlane = 0;
    mPreSegmentation = 1;
    mSegmentationTreeCodeBuffer = NULL;
    mSegmentationTreeCodeBufferSize = 0;
    mSegmentationFlagProbabilityModelIndex = SEGMENTATION_PROB_MODEL_INDEX;
    mSymbolProbabilityModelIndex = SYMBOL_PROBABILITY_MODEL_INDEX;
    mPredictionFlagProbabilityModelIndex = PREDICTION_PROB_MODEL_INDEX;
    mPmodel = NULL;
    mOptimizationPmodel = NULL;
    
}
Hierarchical4DEncoder :: ~Hierarchical4DEncoder(void) {
    if(mSegmentationTreeCodeBuffer != NULL)
        delete [] mSegmentationTreeCodeBuffer;
    
    if(mPmodel != NULL)
        delete [] mPmodel;
    if(mOptimizationPmodel != NULL)
        delete [] mOptimizationPmodel;
}

void Hierarchical4DEncoder :: StartEncoder(FILE *outputFilePointer) {
    
    
    mEntropyCoder.InitEncoder(outputFilePointer);  //opens output file

    if(mPmodel != NULL)
        delete [] mPmodel;
    if(mOptimizationPmodel != NULL)
        delete [] mOptimizationPmodel;
    
    mPmodel = new ProbabilityModel[NUMBER_OF_MODELS];
    mOptimizationPmodel = new ProbabilityModel[NUMBER_OF_MODELS];
    for(int n = 0; n < NUMBER_OF_MODELS; n++) {
         mPmodel[n].ResetModel();
         mOptimizationPmodel[n].ResetModel();
    }
}

void Hierarchical4DEncoder :: RestartProbabilisticModel(void) {
    
     for(int n = 0; n < NUMBER_OF_MODELS; n++) {
         mPmodel[n].ResetModel();
         mOptimizationPmodel[n].ResetModel();
    }
   
    
}

void Hierarchical4DEncoder :: EncodeSubblock(double lambda) {
    
    int flagSearchIndex = 0;
    double Energy;
    
    strcpy(mSegmentationTreeCodeBuffer,"");
    RdOptimizeHexadecaTree(0, 0, 0, 0, mSubbandLF.mlength_t, mSubbandLF.mlength_s, mSubbandLF.mlength_v, mSubbandLF.mlength_u, lambda, mSuperiorBitPlane, &mSegmentationTreeCodeBuffer, Energy);
  
    flagSearchIndex = 0;
    RdEncodeHexadecatree(0, 0, 0, 0, mSubbandLF.mlength_t, mSubbandLF.mlength_s, mSubbandLF.mlength_v, mSubbandLF.mlength_u, mSuperiorBitPlane, flagSearchIndex);
}

double Hierarchical4DEncoder :: RdOptimizeHexadecaTree(int position_t, int position_s, int position_v, int position_u, int length_t, int length_s, int length_v, int length_u, double lambda, int bitplane, char **codeString, double &signalEnergy) {

    double J0, J1;
    double SignalEnergySum;
    ProbabilityModel currentProbabilityModel[NUMBER_OF_MODELS];
    
    if(bitplane < mInferiorBitPlane) {
      
        signalEnergy = 0;
        for(int index_t = 0; index_t < length_t; index_t++) {
            for(int index_s = 0; index_s < length_s; index_s++) {
                for(int index_v = 0; index_v < length_v; index_v++) {
                    for(int index_u = 0; index_u < length_u; index_u++) {
                        int coefficient = mSubbandLF.mPixel[position_t+index_t][position_s+index_s][position_v+index_v][position_u+index_u];
                        J0 = coefficient;
                        signalEnergy += J0*J0;
                    }
                }
            }
        }
        return (signalEnergy);
    }
    
    if(length_t*length_s*length_v*length_u == 1) {
        //evaluate the cost to encode coefficient
       
        int magnitude = mSubbandLF.mPixel[position_t][position_s][position_v][position_u];
        int signal = 0;
        if(magnitude < 0) {
            magnitude = -magnitude;
            signal = 1;
        }
        int allZeros = 1;
        
        signalEnergy = magnitude;
                
        signalEnergy *= signalEnergy;
        
        int onesMask = 0;
        onesMask = ~onesMask;

        double J = 0;
        double accumulatedRate = 0;
        for(int bit_position = bitplane; bit_position >= mInferiorBitPlane; bit_position--) {
            int bit = (magnitude >> bit_position)&01;
            accumulatedRate += mOptimizationPmodel[bit_position+mSymbolProbabilityModelIndex].Rate(bit);
            if(bit_position > BITPLANE_BYPASS) 
                mOptimizationPmodel[bit_position+mSymbolProbabilityModelIndex].UpdateModel(bit);
            
            if(bit == 1)
                allZeros = 0;
       }
        if(allZeros == 0)
            accumulatedRate += 1.0;

        int bitMask = onesMask << mInferiorBitPlane;
        int quantizedMagnitude = magnitude&bitMask;
        if(allZeros == 0) {
            quantizedMagnitude += (1 << mInferiorBitPlane)/2;
        }  
        J = magnitude - quantizedMagnitude;
        
        J = J*J + lambda*(accumulatedRate);
        
        *codeString[0] = 0;

        return(J);
    }
   
    for(int model_index = 0; model_index < NUMBER_OF_MODELS; model_index++)
        currentProbabilityModel[model_index].CopyModel(&mOptimizationPmodel[model_index]);
   
    char *codeString_0 = new char [2];
    strcpy(codeString_0, "");
    
    int Significance = 0;
    
    int Threshold = 1 << bitplane;
    
    for(int index_t = position_t; index_t < position_t+length_t; index_t++) {
        
        for(int index_s = position_s; index_s < position_s+length_s; index_s++) {
            
            for(int index_v = position_v; index_v < position_v+length_v; index_v++) {
                
                for(int index_u = position_u; index_u < position_u+length_u; index_u++) {
                    
                    if((index_t < mSubbandLF.mlength_t)&&(index_s < mSubbandLF.mlength_s)&&(index_v < mSubbandLF.mlength_v)&&(index_u < mSubbandLF.mlength_u)) {
                        
                        if(mSubbandLF.mPixel[index_t][index_s][index_v][index_u] >= Threshold) Significance = 1;
                        if(mSubbandLF.mPixel[index_t][index_s][index_v][index_u] <= -Threshold) Significance = 1;
                        if(Significance == 1) {
                            index_t = position_t+length_t;
                            index_s = position_s+length_s;
                            index_v = position_v+length_v;
                            index_u = position_u+length_u;
                        }
                    
                    }
                    
                }
                
            }
            
        }
        
    }
    
    //evaluate the cost of segmentation flags
    J0 = lambda*mOptimizationPmodel[2*bitplane+mSegmentationFlagProbabilityModelIndex].Rate(0);
    J0 += lambda*mOptimizationPmodel[2*bitplane+1+mSegmentationFlagProbabilityModelIndex].Rate(Significance);
    J1 = lambda*mOptimizationPmodel[2*bitplane+mSegmentationFlagProbabilityModelIndex].Rate(1);
    if(bitplane > BITPLANE_BYPASS_FLAGS) {
        mOptimizationPmodel[2*bitplane+mSegmentationFlagProbabilityModelIndex].UpdateModel(0);
        mOptimizationPmodel[2*bitplane+1+mSegmentationFlagProbabilityModelIndex].UpdateModel(Significance);
    }   
    
    //evaluate the cost J0 to encode this subblock
    if(Significance == 0) {
        
        J0 += RdOptimizeHexadecaTree(position_t, position_s, position_v, position_u, length_t, length_s, length_v, length_u, lambda, bitplane-1, &codeString_0, SignalEnergySum);
        
    }
    else {
         
        SignalEnergySum = 0;
        double Energy;

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
                        
                        char *codeString_1 = new char [2];
                            
                        strcpy(codeString_1, "");
                        J0 += RdOptimizeHexadecaTree(new_position_t, new_position_s, new_position_v, new_position_u, new_length_t, new_length_s, new_length_v, new_length_u, lambda, bitplane, &codeString_1, Energy);
                        char *tempString = new char[strlen(codeString_0)+strlen(codeString_1)+2];
                        strcpy(tempString, codeString_0);
                        strcat(tempString, codeString_1);
                        delete [] codeString_0;
                        delete [] codeString_1;
                        codeString_0 = tempString;
                            
                        SignalEnergySum += Energy;
                    }
                    
                }
                
            }
            
        }
            
        
              
    }    
    
    //evaluate the cost J1 to skip this subblock
    J1 += SignalEnergySum;
    
    //Choose the lowest cost
    if((J0 < J1)||((bitplane == mInferiorBitPlane)&&(Significance == 0))) {
        char *tempString = new char[strlen(*codeString)+strlen(codeString_0)+3];
        strcpy(tempString, *codeString);
        delete [] *codeString;
        *codeString = tempString;
        
        if(Significance == 1) {
            strcat(*codeString, "1");         
        }
        else {
            strcat(*codeString, "0");
        }
        strcat(*codeString, codeString_0);
    }
    else {
        char *tempString = new char[strlen(*codeString)+3];
        strcpy(tempString, *codeString);
        delete [] *codeString;
        *codeString = tempString;
        strcat(*codeString, "2");
        J0 = J1;
        
        for(int model_index = 0; model_index < NUMBER_OF_MODELS; model_index++)
            mOptimizationPmodel[model_index].CopyModel(&currentProbabilityModel[model_index]);
        
        if(bitplane > BITPLANE_BYPASS_FLAGS) 
            mOptimizationPmodel[2*bitplane+mSegmentationFlagProbabilityModelIndex].UpdateModel(1);
    }
  
    delete [] codeString_0;
    
    signalEnergy = SignalEnergySum;
        
    return(J0);
    
}
void Hierarchical4DEncoder :: RdEncodeHexadecatree(int position_t, int position_s, int position_v, int position_u, int length_t, int length_s, int length_v, int length_u, int bitplane, int &flagIndex) {
   
    if(bitplane < mInferiorBitPlane) {
      
        for(int index_t = position_t; index_t < position_t + length_t; index_t++) {
            for(int index_s = position_s; index_s < position_s + length_s; index_s++) {
                for(int index_v = position_v; index_v < position_v + length_v; index_v++) {
                    for(int index_u = position_u; index_u < position_u + length_u; index_u++) {
                        mSubbandLF.mPixel[index_t][index_s][index_v][index_u] = 0;
                    }
                }
            }
        }
      
        return;
    }
    
    if(length_t*length_s*length_v*length_u == 1) {
        //rd encode coefficient        
        EncodeCoefficient(position_t, position_s, position_v, position_u, bitplane);
        return;
    }
    
    if(mSegmentationTreeCodeBuffer[flagIndex] == '0') {
        
        EncodeSegmentationFlag(0, bitplane);
    
        flagIndex++;
        RdEncodeHexadecatree(position_t, position_s, position_v, position_u, length_t, length_s, length_v, length_u, bitplane-1, flagIndex);
        
        return;
    }
    
    if(mSegmentationTreeCodeBuffer[flagIndex] == '2') {
        
        EncodeSegmentationFlag(2, bitplane);
    
        flagIndex++;
    
        for(int index_t = position_t; index_t < position_t + length_t; index_t++) {
            for(int index_s = position_s; index_s < position_s + length_s; index_s++) {
                for(int index_v = position_v; index_v < position_v + length_v; index_v++) {
                    for(int index_u = position_u; index_u < position_u + length_u; index_u++) {
                        mSubbandLF.mPixel[index_t][index_s][index_v][index_u] = 0;
                    }
                }
            }
        }
        return;
    }

    if(mSegmentationTreeCodeBuffer[flagIndex] == '1') {
        
        EncodeSegmentationFlag(1, bitplane);
    
        flagIndex++;
        
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
                        
                        RdEncodeHexadecatree(new_position_t, new_position_s, new_position_v, new_position_u, new_length_t, new_length_s, new_length_v, new_length_u, bitplane, flagIndex);
                
                   }
                    
                }
                
            }
            
        }
        
        return;
    }
   
}

void Hierarchical4DEncoder :: EncodeCoefficient(int position_t, int position_s, int position_v, int position_u, int bitplane) {
    
    int coefficient = mSubbandLF.mPixel[position_t][position_s][position_v][position_u];
    int signal = 0;
    int allZeros = 1;
    int magnitude = coefficient;
    if(magnitude < 0) {
        magnitude = -magnitude;
        signal = 1;
    }
    
    for(int bit_position = bitplane; bit_position >= mInferiorBitPlane; bit_position--) {

        int bit = (magnitude >> (bit_position))&01;
        mEntropyCoder.EncodeBit(bit, mPmodel[bit_position+mSymbolProbabilityModelIndex]);
        if(bit_position > BITPLANE_BYPASS) 
            mPmodel[bit_position+mSymbolProbabilityModelIndex].UpdateModel(bit);        
         if(bit == 1) {
            allZeros = 0;
        }
    }
    if(allZeros == 0) {
        mEntropyCoder.EncodeBit(signal, mPmodel[0]);
    }
    
    int quantizedMagnitude = magnitude >> mInferiorBitPlane;
    quantizedMagnitude = quantizedMagnitude << mInferiorBitPlane;
    if(allZeros == 0) {
        quantizedMagnitude += (1 << mInferiorBitPlane)/2;
    }
    mSubbandLF.mPixel[position_t][position_s][position_v][position_u] = quantizedMagnitude;
    if(signal == 1)
        mSubbandLF.mPixel[position_t][position_s][position_v][position_u] *= -1;
   
}

void Hierarchical4DEncoder :: RdDecodeHexadecatree(void) {
  
    int flagSearchIndex = 0;
    mDecodedLF.SetDimension(mSubbandLF.mlength_t, mSubbandLF.mlength_s, mSubbandLF.mlength_v, mSubbandLF.mlength_u);
    mDecodedLF.Zeros();
    RdDecodeHexadecatreeStep(0, 0, 0, 0, mSubbandLF.mlength_t, mSubbandLF.mlength_s, mSubbandLF.mlength_v, mSubbandLF.mlength_u, mSuperiorBitPlane, flagSearchIndex);
}

void Hierarchical4DEncoder :: RdDecodeHexadecatreeStep(int position_t, int position_s, int position_v, int position_u, int length_t, int length_s, int length_v, int length_u, int bitplane, int &flagIndex) {
   
    if(bitplane < mInferiorBitPlane) {
      
        for(int index_t = position_t; index_t < position_t + length_t; index_t++) {
            for(int index_s = position_s; index_s < position_s + length_s; index_s++) {
                for(int index_v = position_v; index_v < position_v + length_v; index_v++) {
                    for(int index_u = position_u; index_u < position_u + length_u; index_u++) {
                        mDecodedLF.mPixel[index_t][index_s][index_v][index_u] = 0;
                    }
                }
            }
        }
      
        return;
    }
    
    if(length_t*length_s*length_v*length_u == 1) {

    int coefficient = mSubbandLF.mPixel[position_t][position_s][position_v][position_u];
    int signal = 0;
    int allZeros = 1;
    int magnitude = coefficient;
    if(magnitude < 0) {
        magnitude = -magnitude;
        signal = 1;
    }
    
    for(int bit_position = bitplane; bit_position >= mInferiorBitPlane; bit_position--) {

        int bit = (magnitude >> (bit_position))&01;
         if(bit == 1) {
            allZeros = 0;
        }
    }
    
    int quantizedMagnitude = magnitude >> mInferiorBitPlane;
    quantizedMagnitude = quantizedMagnitude << mInferiorBitPlane;
    if(allZeros == 0) {
        quantizedMagnitude += (1 << mInferiorBitPlane)/2;
    }
    mDecodedLF.mPixel[position_t][position_s][position_v][position_u] = quantizedMagnitude;
    if(signal == 1)
        mDecodedLF.mPixel[position_t][position_s][position_v][position_u] *= -1;

        return;
    }
    
    if(mSegmentationTreeCodeBuffer[flagIndex] == '0') {
        
    
        flagIndex++;
        RdDecodeHexadecatreeStep(position_t, position_s, position_v, position_u, length_t, length_s, length_v, length_u, bitplane-1, flagIndex);
        
        return;
    }
    
    if(mSegmentationTreeCodeBuffer[flagIndex] == '2') {
            
        flagIndex++;
    
        for(int index_t = position_t; index_t < position_t + length_t; index_t++) {
            for(int index_s = position_s; index_s < position_s + length_s; index_s++) {
                for(int index_v = position_v; index_v < position_v + length_v; index_v++) {
                    for(int index_u = position_u; index_u < position_u + length_u; index_u++) {
                        mDecodedLF.mPixel[index_t][index_s][index_v][index_u] = 0;
                    }
                }
            }
        }
        return;
    }

    if(mSegmentationTreeCodeBuffer[flagIndex] == '1') {
        
        flagIndex++;
        
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
                        
                        RdDecodeHexadecatreeStep(new_position_t, new_position_s, new_position_v, new_position_u, new_length_t, new_length_s, new_length_v, new_length_u, bitplane, flagIndex);
                
                   }
                    
                }
                
            }
            
        }
        
        return;
    }
   
}

void Hierarchical4DEncoder :: EncodeSegmentationFlag(int flag, int bitplane) {
        
    if(flag == 0) {
        mEntropyCoder.EncodeBit(0, mPmodel[2*bitplane+mSegmentationFlagProbabilityModelIndex]);
        mEntropyCoder.EncodeBit(0, mPmodel[2*bitplane+1+mSegmentationFlagProbabilityModelIndex]);
        if(bitplane > BITPLANE_BYPASS_FLAGS) {
            mPmodel[2*bitplane+mSegmentationFlagProbabilityModelIndex].UpdateModel(0);
            mPmodel[2*bitplane+1+mSegmentationFlagProbabilityModelIndex].UpdateModel(0);
        }
    }
    if(flag == 1) {
        mEntropyCoder.EncodeBit(0, mPmodel[2*bitplane+mSegmentationFlagProbabilityModelIndex]);
        mEntropyCoder.EncodeBit(1, mPmodel[2*bitplane+1+mSegmentationFlagProbabilityModelIndex]);
        if(bitplane > BITPLANE_BYPASS_FLAGS) {
            mPmodel[2*bitplane+mSegmentationFlagProbabilityModelIndex].UpdateModel(0);
            mPmodel[2*bitplane+1+mSegmentationFlagProbabilityModelIndex].UpdateModel(1);
        }
    }
    if(flag == 2) {
        mEntropyCoder.EncodeBit(1, mPmodel[2*bitplane+mSegmentationFlagProbabilityModelIndex]);
        if(bitplane > BITPLANE_BYPASS_FLAGS) 
            mPmodel[2*bitplane+mSegmentationFlagProbabilityModelIndex].UpdateModel(1);
    }
    
}

void Hierarchical4DEncoder :: EncodePartitionFlag(int symbol) {
     
    if(symbol == 2) {
        mEntropyCoder.EncodeBit(1, mPmodel[0]);
        mEntropyCoder.EncodeBit(1, mPmodel[0]);
    }
    if(symbol == 1) {
        mEntropyCoder.EncodeBit(1, mPmodel[0]);
        mEntropyCoder.EncodeBit(0, mPmodel[0]);
    }
    if(symbol == 0) {
        mEntropyCoder.EncodeBit(0, mPmodel[0]);
    }
   
}

void Hierarchical4DEncoder :: EncodePredictionFlag(int flag, int treedepth) {
        
    if(flag == 0) {
        mEntropyCoder.EncodeBit(0, mPmodel[2*treedepth+mPredictionFlagProbabilityModelIndex]);
        mEntropyCoder.EncodeBit(0, mPmodel[2*treedepth+1+mPredictionFlagProbabilityModelIndex]);
        mPmodel[2*treedepth+mPredictionFlagProbabilityModelIndex].UpdateModel(0);
        mPmodel[2*treedepth+1+mPredictionFlagProbabilityModelIndex].UpdateModel(0);
    }
    if(flag == 1) {
        mEntropyCoder.EncodeBit(0, mPmodel[2*treedepth+mPredictionFlagProbabilityModelIndex]);
        mEntropyCoder.EncodeBit(1, mPmodel[2*treedepth+1+mPredictionFlagProbabilityModelIndex]);
        mPmodel[2*treedepth+mPredictionFlagProbabilityModelIndex].UpdateModel(0);
        mPmodel[2*treedepth+1+mPredictionFlagProbabilityModelIndex].UpdateModel(1);
    }
    if(flag == 2) {
        mEntropyCoder.EncodeBit(1, mPmodel[2*treedepth+mPredictionFlagProbabilityModelIndex]);
        mPmodel[2*treedepth+mPredictionFlagProbabilityModelIndex].UpdateModel(1);
    }
    
}


void Hierarchical4DEncoder :: EncodeInteger(int integerValue, int precision)  {

    for(int n = precision-1; n >= 0; n--) {
        int bit = (integerValue >> n)&01;
        mEntropyCoder.EncodeBit(bit, mPmodel[0]);
    }
        
}

void Hierarchical4DEncoder :: DoneEncoding(void) {
    
    mEntropyCoder.Flush();      //flushes entropy encoder
    
}


void Hierarchical4DEncoder :: SetDimension(int length_t, int length_s, int length_v, int length_u)  {
    
    mSubbandLF.SetDimension(length_t, length_s, length_v, length_u);

    if(mSegmentationTreeCodeBuffer != NULL)
        delete [] mSegmentationTreeCodeBuffer;
        
    mSegmentationTreeCodeBufferSize = 2*length_t*length_s*length_v*length_u+1; //enough for at most one bit per pixel rates

    mSegmentationTreeCodeBuffer = new char [mSegmentationTreeCodeBufferSize];
}

int Hierarchical4DEncoder :: OptimumBitplane(double lambda) {
    
    long int subbandSize = mSubbandLF.mlength_t*mSubbandLF.mlength_s;
    subbandSize *= mSubbandLF.mlength_v*mSubbandLF.mlength_u;
    double Jmin=0;            //Irrelevant initial value
    int optimumBitplane=0;    //Irrelevant initial value
    
    double accumulatedRate = 0;
    
    for(int bit_position = mSuperiorBitPlane; bit_position >= 0; bit_position--) {
        
        double distortion = 0.0;
        double coefficientsDistortion = 0.0;
        double signalRate = 0.0;
        double J;
        int numberOfCoefficients = 0;
        
        int onesMask = 0;
        onesMask = ~onesMask;
        int bitMask = onesMask << bit_position;
       

        for(long int coefficient_index=0; coefficient_index < subbandSize; coefficient_index++) {
        
            int magnitude = mSubbandLF.mPixelData[coefficient_index];
            if(magnitude < 0) {
                magnitude = -magnitude;
            }
            int Threshold = (1 << bit_position);
            if(magnitude >= Threshold) {
                int bit = (magnitude >> bit_position)&01;
                accumulatedRate += mOptimizationPmodel[bit_position+mSymbolProbabilityModelIndex].Rate(bit);
                mOptimizationPmodel[bit_position+mSymbolProbabilityModelIndex].UpdateModel(bit);
                int quantizedMagnitude = magnitude&bitMask;
                if(quantizedMagnitude > 0) {
                    signalRate += 1.0;
                }
                numberOfCoefficients++;
            }
            int quantizedMagnitude = magnitude&bitMask;
            if(quantizedMagnitude > 0) {
                quantizedMagnitude += (1 << bit_position)/2;
            }  
            double magnitude_error = magnitude - quantizedMagnitude;
            
            distortion += magnitude_error*magnitude_error;
            if(magnitude >= (1 << bit_position)) {
                coefficientsDistortion += magnitude_error*magnitude_error;
            }  
        }
           
        J = distortion + lambda*(accumulatedRate + signalRate);
        
        if((J <= Jmin)||(bit_position == mSuperiorBitPlane)) {
            Jmin = J;
            optimumBitplane = bit_position;
        }
       
    }
    
    return(optimumBitplane);
}

void Hierarchical4DEncoder :: GetOptimizerProbabilisticModelState(ProbabilityModel **state) {
    
    DeleteProbabilisticModelState(state);
    ProbabilityModel *pmodelArray = new ProbabilityModel [NUMBER_OF_MODELS];
    for(int model_index = 0; model_index < NUMBER_OF_MODELS; model_index++) {
        pmodelArray[model_index].CopyModel(&mOptimizationPmodel[model_index]);    
    }
    *state = pmodelArray;
}

void Hierarchical4DEncoder :: SetOptimizerProbabilisticModelState(ProbabilityModel *state) {

     for(int model_index = 0; model_index < NUMBER_OF_MODELS; model_index++) {
        mOptimizationPmodel[model_index].CopyModel(&state[model_index]);    
    }
  
}

void Hierarchical4DEncoder :: DeleteProbabilisticModelState(ProbabilityModel **state) {
    
    if(*state != NULL) {
        delete [] *state;
        *state = NULL;
    }
     
}

void Hierarchical4DEncoder :: LoadOptimizerState(void) {

     for(int model_index = 0; model_index < NUMBER_OF_MODELS; model_index++) {
        mOptimizationPmodel[model_index].CopyModel(&mPmodel[model_index]);    
    }
  
}

void Hierarchical4DEncoder :: BlockQuantization(Block4D &lfBlock) {
  
    for(int index_t = 0; index_t < lfBlock.mlength_t; index_t++) {
        for(int index_s = 0; index_s < lfBlock.mlength_s; index_s++) {
            for(int index_v = 0; index_v < lfBlock.mlength_v; index_v++) {
                for(int index_u = 0; index_u < lfBlock.mlength_u; index_u++) {

                    int magnitude = lfBlock.mPixel[index_t][index_s][index_v][index_u];
                    int signal = (magnitude > 0) ? 1.0 : -1.0;
                    magnitude *= signal;
                    int quantizedMagnitude = magnitude >> mInferiorBitPlane;
                    quantizedMagnitude = quantizedMagnitude << mInferiorBitPlane;
                    if(quantizedMagnitude > 0) {
                        quantizedMagnitude += (1 << mInferiorBitPlane)/2;
                    }
                    lfBlock.mPixel[index_t][index_s][index_v][index_u] = signal*quantizedMagnitude;
                }
            }
        }
    }
}

void Hierarchical4DEncoder :: BlockQuantization(Block4D &outputBlock, Block4D const &inputBlock, char *segmentationCode, int bitplane) {
  
    outputBlock.SetDimension(inputBlock.mlength_t, inputBlock.mlength_s, inputBlock.mlength_v, inputBlock.mlength_u);
    //outputBlock.Zeros();
    int nextindex=0;
    BlockQuantizationStep(outputBlock, inputBlock, 0, 0, 0, 0, inputBlock.mlength_t, inputBlock.mlength_s, inputBlock.mlength_v, inputBlock.mlength_u, segmentationCode, nextindex, mSuperiorBitPlane, bitplane);
      
}

void Hierarchical4DEncoder :: BlockQuantizationStep(Block4D &outputBlock, Block4D const &inputBlock, int position_t, int position_s, int position_v, int position_u, int length_t, int length_s, int length_v, int length_u, char *segmentationCode, int &nextcharindex, int bitplane, int minimumbitplane) {
  
    if(bitplane < minimumbitplane) {
      
        for(int index_t = position_t; index_t < position_t + length_t; index_t++) {
            for(int index_s = position_s; index_s < position_s + length_s; index_s++) {
                for(int index_v = position_v; index_v < position_v + length_v; index_v++) {
                    for(int index_u = position_u; index_u < position_u + length_u; index_u++) {
                        outputBlock.mPixel[index_t][index_s][index_v][index_u] = 0;
                    }
                }
            }
        }
      
        return;
    }
    
    if(length_t*length_s*length_v*length_u == 1) {
      
        int magnitude = inputBlock.mPixel[position_t][position_s][position_v][position_u];
        int signal = (magnitude > 0) ? 1.0 : -1.0;
        magnitude *= signal;
        int quantizedMagnitude = magnitude >> minimumbitplane;
        quantizedMagnitude = quantizedMagnitude << minimumbitplane;
        if(quantizedMagnitude > 0) {
            quantizedMagnitude += (1 << minimumbitplane)/2;
        }
        outputBlock.mPixel[position_t][position_s][position_v][position_u] = signal*quantizedMagnitude;
        return;
    }
    
    if(segmentationCode[nextcharindex] == '0') {
        
        nextcharindex++;

        BlockQuantizationStep(outputBlock, inputBlock, position_t, position_s, position_v, position_u, length_t, length_s, length_v, length_u, segmentationCode, nextcharindex, bitplane-1, minimumbitplane);
        
        return;
    }
    if(segmentationCode[nextcharindex] == '1') {
        
        nextcharindex++;
        
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
                        
                        BlockQuantizationStep(outputBlock, inputBlock, new_position_t, new_position_s, new_position_v, new_position_u, new_length_t, new_length_s, new_length_v, new_length_u, segmentationCode, nextcharindex, bitplane, minimumbitplane);
                
                   }
                    
                }
                
            }
            
        }
        
        return;
    }
    if(segmentationCode[nextcharindex] == '2') {
      
        nextcharindex++;
        
        for(int index_t = position_t; index_t < position_t + length_t; index_t++) {
            for(int index_s = position_s; index_s < position_s + length_s; index_s++) {
                for(int index_v = position_v; index_v < position_v + length_v; index_v++) {
                    for(int index_u = position_u; index_u < position_u + length_u; index_u++) {
                        outputBlock.mPixel[index_t][index_s][index_v][index_u] = 0;
                    }
                }
            }
        }
        
        return;

    }
  
}

