#include "TransformPartition.h"

/*******************************************************************************/
/*                      TransformPartition class methods                    */
/*******************************************************************************/

TransformPartition :: TransformPartition(void) {
    mPartitionCode = NULL;
    mlength_t_min = 1;
    mlength_s_min = 1;
    mlength_v_min = 4;
    mlength_u_min = 4;
}
TransformPartition :: ~TransformPartition(void) {
    if(mPartitionCode != NULL)
        delete [] mPartitionCode;
}

void TransformPartition :: SetDimension(int length_t, int length_s, int length_v, int length_u, int length_t_min, int length_s_min, int length_v_min, int length_u_min) {
  
    mPartitionData.SetDimension(length_t, length_s, length_v, length_u);
    mDecodedData.SetDimension(length_t, length_s, length_v, length_u);
    mlength_t_min = length_t_min;
    mlength_s_min = length_s_min;
    mlength_v_min = length_v_min;
    mlength_u_min = length_u_min;
    
}

double TransformPartition :: RDoptimizeTransform(Block4D &inputBlock, MultiscaleTransform &mt, MultiscaleTransform &imt, Hierarchical4DEncoder &entropyCoder, double lambda, int evaluate_minimum_bitplane) {
/*! Evaluates the Lagrangian cost of the optimum multiscale transform for the input block as well as the transformed block */   
    if(mPartitionCode != NULL)
        delete [] mPartitionCode;
    mPartitionCode = new char [1];
    mPartitionCode[0] = 0;          //initializes the partition code string as the null string
    //mEvaluateOptimumBitPlane = 1;
    mEvaluateOptimumBitPlane = evaluate_minimum_bitplane;
    mPartitionData.SetDimension(inputBlock.mlength_t, inputBlock.mlength_s, inputBlock.mlength_v, inputBlock.mlength_u);
    mDecodedData.SetDimension(inputBlock.mlength_t, inputBlock.mlength_s, inputBlock.mlength_v, inputBlock.mlength_u);
    //double scaledLambda = mPartitionData.mlength_t*mPartitionData.mlength_s;
    //scaledLambda *= lambda*mPartitionData.mlength_v*mPartitionData.mlength_u;
    
    int position[4];
    position[0] = 0;
    position[1] = 0;
    position[2] = 0;
    position[3] = 0;

    int length[4];
    length[0] = inputBlock.mlength_t;
    length[1] = inputBlock.mlength_s;
    length[2] = inputBlock.mlength_v;
    length[3] = inputBlock.mlength_u;
   
    //copies the current entropyCoder arithmetic model to the optimizer model. 
    //entropyCoder.LoadOptimizerState();
    
    Block4D transformedBlock;
    transformedBlock.SetDimension(length[0], length[1], length[2], length[3]);
    Block4D decodedBlock;
    decodedBlock.SetDimension(length[0], length[1], length[2], length[3]);

    
    mLagrangianCost = RDoptimizeTransformStep(inputBlock, transformedBlock, decodedBlock, position, length, mt, imt, entropyCoder, lambda, &mPartitionCode);
    
    mPartitionData.CopySubblockFrom(transformedBlock, 0, 0, 0, 0);
    
    mDecodedData.CopySubblockFrom(decodedBlock, 0, 0, 0, 0);
    
    //Restores state since the encoder will reevaluate it    
    //entropyCoder.LoadOptimizerState();
    
    //printf("mPartitionCode = %s\n", mPartitionCode);    
    //printf("mInferiorBitPlane = %d\n", entropyCoder.mInferiorBitPlane);

    return(mLagrangianCost);    
}

double TransformPartition :: RDoptimizeTransformStep(Block4D &inputBlock, Block4D &transformedBlock, Block4D &decodedBlock, int *position, int *length, MultiscaleTransform &mt, MultiscaleTransform &imt, Hierarchical4DEncoder &entropyCoder, double lambda, char **partitionCode) {
/*! returns the Lagrangian cost of one step of the optimization of the multiscale transform for the input block as well as the transformed block */   
 
    //J0 = cost of full transform
    //saves the current entropyCoder arithmetic model to current_model. 
    ProbabilityModel *currentCoderModelState=NULL;
    entropyCoder.GetOptimizerProbabilisticModelState(&currentCoderModelState);
    char *partitionCodeS=NULL, *partitionCodeV=NULL; 
    
    //copy the inputBlock to block_0 and apply transformation using the appropriate scale from mt    
    Block4D block_0;
    block_0.SetDimension(length[0], length[1], length[2], length[3]);
    block_0.CopySubblockFrom(inputBlock, position[0], position[1], position[2], position[3]);
    mt.Transform4D(block_0);
   
    //copy the transformed input block to entropyCoder.mSubbandLF    
    entropyCoder.mSubbandLF.SetDimension(block_0.mlength_t, block_0.mlength_s, block_0.mlength_v, block_0.mlength_u);
    entropyCoder.mSubbandLF.CopySubblockFrom(block_0, 0, 0, 0, 0);
    double Energy;
    if(mEvaluateOptimumBitPlane == 1) {
        entropyCoder.mInferiorBitPlane = entropyCoder.OptimumBitplane(lambda);
        entropyCoder.LoadOptimizerState();
        mEvaluateOptimumBitPlane = 0;
    }

    Block4D decodedBlock_0;
    decodedBlock_0.SetDimension(length[0], length[1], length[2], length[3]);
    
    //call RdOptimizeHexadecaTree method from entropyCoder to evaluate J0
    if(entropyCoder.mSegmentationTreeCodeBuffer != NULL)
        delete [] entropyCoder.mSegmentationTreeCodeBuffer;
    entropyCoder.mSegmentationTreeCodeBuffer = new char [2];
    strcpy(entropyCoder.mSegmentationTreeCodeBuffer,"");
    double J0 = entropyCoder.RdOptimizeHexadecaTree(0, 0, 0, 0, entropyCoder.mSubbandLF.mlength_t, entropyCoder.mSubbandLF.mlength_s, entropyCoder.mSubbandLF.mlength_v, entropyCoder.mSubbandLF.mlength_u, lambda, entropyCoder.mSuperiorBitPlane, &entropyCoder.mSegmentationTreeCodeBuffer, Energy);

    entropyCoder.RdDecodeHexadecatree();
    decodedBlock_0.CopySubblockFrom(entropyCoder.mDecodedLF,0,0,0,0);
   
    //char *block0SegmentationCode = new char [1+strlen(entropyCoder.mSegmentationTreeCodeBuffer)];
    //strcpy(block0SegmentationCode, entropyCoder.mSegmentationTreeCodeBuffer);
    
    //int block0MinimumBitplane = entropyCoder.mInferiorBitPlane;
    
    //saves the resulting entropyCoder arithmetic model to model_0
    ProbabilityModel *coderModelState_0=NULL;
    entropyCoder.GetOptimizerProbabilisticModelState(&coderModelState_0);
    
    //JS = cost of four quarter spatial subblocks
    //Restores the current arithmetic model using current_model. 
    entropyCoder.SetOptimizerProbabilisticModelState(currentCoderModelState);
    double JS = -1.0;
    Block4D transformedBlockS;
    transformedBlockS.SetDimension(length[0], length[1], length[2], length[3]);
    
    Block4D decodedBlockS;
    decodedBlockS.SetDimension(length[0], length[1], length[2], length[3]);

    if((length[3] >= 2*mlength_u_min)&&(length[2] >= 2*mlength_v_min)) {
        JS = 0.0;
        
        
        char *partitionCodeS00 = new char[1];
        char *partitionCodeS01 = new char[1];
        char *partitionCodeS10 = new char[1];
        char *partitionCodeS11 = new char[1];
        
        partitionCodeS00[0] = 0;
        partitionCodeS01[0] = 0;
        partitionCodeS10[0] = 0;
        partitionCodeS11[0] = 0;
       
        int new_position[4], new_length[4];
        
        new_position[0] = position[0];
        new_position[1] = position[1];
        new_position[2] = position[2];
        new_position[3] = position[3];
        
        new_length[0] = length[0];
        new_length[1] = length[1];
        new_length[2] = length[2]/2;
        new_length[3] = length[3]/2;
          
        //optimize partition for Block_S returning JS, the transformed Block_S, partitionCode_S and arithmetic_model_S
        Block4D transformedBlockS00;
        transformedBlockS00.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        Block4D decodedBlockS00;
        decodedBlockS00.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);

        JS += RDoptimizeTransformStep(inputBlock, transformedBlockS00, decodedBlockS00, new_position, new_length, mt, imt, entropyCoder, lambda, &partitionCodeS00);

        new_position[3] = position[3] + length[3]/2;
        new_length[3] = length[3] - length[3]/2;
                 
        Block4D transformedBlockS01;
        transformedBlockS01.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        Block4D decodedBlockS01;
        decodedBlockS01.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);

        JS += RDoptimizeTransformStep(inputBlock, transformedBlockS01, decodedBlockS01, new_position, new_length, mt, imt, entropyCoder, lambda, &partitionCodeS01);

        new_position[2] = position[2] + length[2]/2;
        new_length[2] = length[2] - length[2]/2;
        
        Block4D transformedBlockS11;
        transformedBlockS11.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        Block4D decodedBlockS11;
        decodedBlockS11.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        JS += RDoptimizeTransformStep(inputBlock, transformedBlockS11, decodedBlockS11, new_position, new_length, mt, imt, entropyCoder, lambda, &partitionCodeS11);
        
        new_position[3] = position[3];
        new_length[3] = length[3]/2;
        
        Block4D transformedBlockS10;
        transformedBlockS10.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        Block4D decodedBlockS10;
        decodedBlockS10.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        JS += RDoptimizeTransformStep(inputBlock, transformedBlockS10, decodedBlockS10, new_position, new_length, mt, imt, entropyCoder, lambda, &partitionCodeS10);
              
        partitionCodeS = new char [2+strlen(partitionCodeS00)+strlen(partitionCodeS01)+strlen(partitionCodeS10)+strlen(partitionCodeS11)];
        strcpy(partitionCodeS, partitionCodeS00);
        strcat(partitionCodeS, partitionCodeS01);
        strcat(partitionCodeS, partitionCodeS11);
        strcat(partitionCodeS, partitionCodeS10);
        
        transformedBlockS.CopySubblockFrom(transformedBlockS00, 0, 0, 0, 0);
        transformedBlockS.CopySubblockFrom(transformedBlockS01, 0, 0, 0, 0, 0, 0, 0, length[3]/2);
        transformedBlockS.CopySubblockFrom(transformedBlockS11, 0, 0, 0, 0, 0, 0, length[2]/2, length[3]/2);
        transformedBlockS.CopySubblockFrom(transformedBlockS10, 0, 0, 0, 0, 0, 0, length[2]/2, 0);
        
        decodedBlockS.CopySubblockFrom(decodedBlockS00, 0, 0, 0, 0);
        decodedBlockS.CopySubblockFrom(decodedBlockS01, 0, 0, 0, 0, 0, 0, 0, length[3]/2);
        decodedBlockS.CopySubblockFrom(decodedBlockS11, 0, 0, 0, 0, 0, 0, length[2]/2, length[3]/2);
        decodedBlockS.CopySubblockFrom(decodedBlockS10, 0, 0, 0, 0, 0, 0, length[2]/2, 0);
        
        delete [] partitionCodeS00;
        delete [] partitionCodeS01;
        delete [] partitionCodeS10;
        delete [] partitionCodeS11;
        
        transformedBlockS00.SetDimension(0, 0, 0, 0);
        transformedBlockS01.SetDimension(0, 0, 0, 0);
        transformedBlockS11.SetDimension(0, 0, 0, 0);
        transformedBlockS10.SetDimension(0, 0, 0, 0);
        
        decodedBlockS00.SetDimension(0, 0, 0, 0);
        decodedBlockS01.SetDimension(0, 0, 0, 0);
        decodedBlockS11.SetDimension(0, 0, 0, 0);
        decodedBlockS10.SetDimension(0, 0, 0, 0);

     }
    //saves the resulting entropyCoder arithmetic model to model_s
    ProbabilityModel *coderModelState_s=NULL;
    entropyCoder.GetOptimizerProbabilisticModelState(&coderModelState_s);
    
    //JV = cost of four quarter view subblocks
    //Restores the current arithmetic model using current_model. 
    entropyCoder.SetOptimizerProbabilisticModelState(currentCoderModelState);
    double JV = -1.0;
    Block4D transformedBlockV;
    transformedBlockV.SetDimension(length[0], length[1], length[2], length[3]);

    Block4D decodedBlockV;
    decodedBlockV.SetDimension(length[0], length[1], length[2], length[3]);
    
    if((length[0] >= 2*mlength_t_min)&&(length[1] >= 2*mlength_s_min)) {
        JV = 0.0;
        
        
        char *partitionCodeV00 = new char[1];
        char *partitionCodeV01 = new char[1];
        char *partitionCodeV10 = new char[1];
        char *partitionCodeV11 = new char[1];
        
        partitionCodeV00[0] = 0;
        partitionCodeV01[0] = 0;
        partitionCodeV10[0] = 0;
        partitionCodeV11[0] = 0;
       
        int new_position[4], new_length[4];
        
        new_position[0] = position[0];
        new_position[1] = position[1];
        new_position[2] = position[2];
        new_position[3] = position[3];
        
        new_length[0] = length[0]/2;
        new_length[1] = length[1]/2;
        new_length[2] = length[2];
        new_length[3] = length[3];
        
        //optimize partition for Block_V returning JV, the transformed Block_V, partitionCode_S and arithmetic_model_S
        Block4D transformedBlockV00;
        transformedBlockV00.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        Block4D decodedBlockV00;
        decodedBlockV00.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);

        JV += RDoptimizeTransformStep(inputBlock, transformedBlockV00, decodedBlockV00, new_position, new_length, mt, imt, entropyCoder, lambda, &partitionCodeV00);

        new_position[1] = position[1] + length[1]/2;
        new_length[1] = length[1] - length[1]/2;
        
        Block4D transformedBlockV01;
        transformedBlockV01.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        Block4D decodedBlockV01;
        decodedBlockV01.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);

        JV += RDoptimizeTransformStep(inputBlock, transformedBlockV01, decodedBlockV01, new_position, new_length, mt, imt, entropyCoder, lambda, &partitionCodeV01);

        new_position[0] = position[0] + length[0]/2;
        new_length[0] = length[0] - length[0]/2;
        
        Block4D transformedBlockV11;
        transformedBlockV11.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        Block4D decodedBlockV11;
        decodedBlockV11.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        JV += RDoptimizeTransformStep(inputBlock, transformedBlockV11, decodedBlockV11, new_position, new_length, mt, imt, entropyCoder, lambda, &partitionCodeV11);
        
        new_position[1] = position[1];
        new_length[1] = length[1]/2;
        
        Block4D transformedBlockV10;
        transformedBlockV10.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        Block4D decodedBlockV10;
        decodedBlockV10.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        JV += RDoptimizeTransformStep(inputBlock, transformedBlockV10, decodedBlockV10, new_position, new_length, mt, imt, entropyCoder, lambda, &partitionCodeV10);
        
        partitionCodeV = new char [2+strlen(partitionCodeV00)+strlen(partitionCodeV01)+strlen(partitionCodeV10)+strlen(partitionCodeV11)];
        strcpy(partitionCodeV, partitionCodeV00);
        strcat(partitionCodeV, partitionCodeV01);
        strcat(partitionCodeV, partitionCodeV11);
        strcat(partitionCodeV, partitionCodeV10);
        
        transformedBlockV.CopySubblockFrom(transformedBlockV00, 0, 0, 0, 0);
        transformedBlockV.CopySubblockFrom(transformedBlockV01, 0, 0, 0, 0, 0, length[1]/2, 0, 0);
        transformedBlockV.CopySubblockFrom(transformedBlockV11, 0, 0, 0, 0, length[0]/2, length[1]/2, 0, 0);
        transformedBlockV.CopySubblockFrom(transformedBlockV10, 0, 0, 0, 0, length[0]/2, 0, 0, 0);
        
        decodedBlockV.CopySubblockFrom(decodedBlockV00, 0, 0, 0, 0);
        decodedBlockV.CopySubblockFrom(decodedBlockV01, 0, 0, 0, 0, 0, length[1]/2, 0, 0);
        decodedBlockV.CopySubblockFrom(decodedBlockV11, 0, 0, 0, 0, length[0]/2, length[1]/2, 0, 0);
        decodedBlockV.CopySubblockFrom(decodedBlockV10, 0, 0, 0, 0, length[0]/2, 0, 0, 0);

        delete [] partitionCodeV00;
        delete [] partitionCodeV01;
        delete [] partitionCodeV10;
        delete [] partitionCodeV11;
        
        transformedBlockV00.SetDimension(0, 0, 0, 0);
        transformedBlockV01.SetDimension(0, 0, 0, 0);
        transformedBlockV11.SetDimension(0, 0, 0, 0);
        transformedBlockV10.SetDimension(0, 0, 0, 0);
        
        decodedBlockV00.SetDimension(0, 0, 0, 0);
        decodedBlockV01.SetDimension(0, 0, 0, 0);
        decodedBlockV11.SetDimension(0, 0, 0, 0);
        decodedBlockV10.SetDimension(0, 0, 0, 0);

   }

    //saves the resulting entropyCoder arithmetic model to model_v
    ProbabilityModel *coderModelState_v=NULL;
    entropyCoder.GetOptimizerProbabilisticModelState(&coderModelState_v);

    if(J0 > 0) 
        J0 += 1.0*lambda;
    if(JV > 0)
        JV += 2.0*lambda;
    if(JS > 0)
        JS += 2.0*lambda;
    
    //choose the lower cost and returns the corresponding cost,  the partition code and the arithmetic coder model
    //find best J
    int interview_split = 0;
    int intraview_split = 0;
    int no_split = 0;
   
    if(JV >= 0) {
        if(JS >= 0) {
            if(JV < JS) {
                if(JV < J0) {
                    interview_split = 1;
                }
                else {
                    no_split = 1;                    
                }
            }
            else {                
                if(JS < J0) {
                    intraview_split = 1;
                }
                else {
                    no_split = 1;                    
                }
            }
        }
        else {
            if(JV < J0) {
                interview_split = 1;
            }
            else {
                no_split = 1;
            }            
        }
    }
    else {
        if(JS >= 0) {
            if(JS < J0) {
                intraview_split = 1;
            }
            else {
                no_split = 1;
            }
        }
        else {
            no_split = 1;
        }
    }
    double optimumJ=0;
    if((interview_split + intraview_split + no_split) != 1) {
        printf("ERRO: partition fail/n");
        exit(0);
    }
    //reallocates memory for the partitionCode string based on the current length and the length of the chosen one
    //copies data from the chosen arithmetic coder model to the current model
    char flagCode[2];
    flagCode[1] = 0;
    if(interview_split == 1) {
        optimumJ = JV;
        char *code = new char[2+strlen(*partitionCode)+strlen(partitionCodeV)];
        strcpy(code, *partitionCode);
        flagCode[0] = INTERVIEWSPLITFLAG;
        strcat(code, flagCode);
        strcat(code, partitionCodeV);
        delete(*partitionCode);
        *partitionCode = code;
        entropyCoder.SetOptimizerProbabilisticModelState(coderModelState_v);
        transformedBlock.CopySubblockFrom(transformedBlockV, 0, 0, 0, 0);
        decodedBlock.CopySubblockFrom(decodedBlockV, 0, 0, 0, 0);
    }
    if(intraview_split == 1) {
        optimumJ = JS;
        char *code = new char[2+strlen(*partitionCode)+strlen(partitionCodeS)];
        strcpy(code, *partitionCode);
        flagCode[0] = INTRAVIEWSPLITFLAG;
        strcat(code, flagCode);
        strcat(code, partitionCodeS);
        delete(*partitionCode);
        *partitionCode = code;
        entropyCoder.SetOptimizerProbabilisticModelState(coderModelState_s);
        transformedBlock.CopySubblockFrom(transformedBlockS, 0, 0, 0, 0);
        decodedBlock.CopySubblockFrom(decodedBlockS, 0, 0, 0, 0);
    }
    if(no_split == 1) {
        optimumJ = J0;   
        char *code = new char[2+strlen(*partitionCode)];
        strcpy(code, *partitionCode);
        flagCode[0] = NOSPLITFLAG;
        strcat(code, flagCode);
        delete(*partitionCode);
        *partitionCode = code;
        entropyCoder.SetOptimizerProbabilisticModelState(coderModelState_0);
        transformedBlock.CopySubblockFrom(block_0, 0, 0, 0, 0);
        //Block4D decodedBlock_0;
        //entropyCoder.BlockQuantization(decodedBlock_0, block_0, block0SegmentationCode, block0MinimumBitplane);    
        imt.InverseTransform4D(decodedBlock_0);
        decodedBlock.CopySubblockFrom(decodedBlock_0, 0, 0, 0, 0);
        //decodedBlock_0.SetDimension(0,0,0,0);
    }
    //deletes temporary strings, blocks and models
   
    if(partitionCodeV != NULL) {
        delete [] partitionCodeV;
    }
    if(partitionCodeS != NULL) {
        delete [] partitionCodeS;
    }

    //delete [] block0SegmentationCode;
    
    entropyCoder.DeleteProbabilisticModelState(&currentCoderModelState);
    entropyCoder.DeleteProbabilisticModelState(&coderModelState_0);
    entropyCoder.DeleteProbabilisticModelState(&coderModelState_s);
    entropyCoder.DeleteProbabilisticModelState(&coderModelState_v);
    
    block_0.SetDimension(0,0,0,0);
    decodedBlock_0.SetDimension(0,0,0,0);
    transformedBlockS.SetDimension(0,0,0,0);
    transformedBlockV.SetDimension(0,0,0,0);

    //return optimum J
    return(optimumJ);
    
}
    
void TransformPartition :: EncodePartition(MultiscaleTransform &imt, Hierarchical4DEncoder &entropyCoder, double lambda, int encode_minimum_bitplane) {
    
    //double scaledLambda = mPartitionData.mlength_t*mPartitionData.mlength_s;
    //scaledLambda *= lambda*mPartitionData.mlength_v*mPartitionData.mlength_u;
    
    mPartitionCodeIndex = 0;
    
    int position[4];
    position[0] = 0;
    position[1] = 0;
    position[2] = 0;
    position[3] = 0;

    int length[4];
    length[0] = mPartitionData.mlength_t;
    length[1] = mPartitionData.mlength_s;
    length[2] = mPartitionData.mlength_v;
    length[3] = mPartitionData.mlength_u;
    
    printf("mPartitionCode = %s\n", mPartitionCode);    
    printf("mInferiorBitPlane = %d\n", entropyCoder.mInferiorBitPlane);
     
    if(encode_minimum_bitplane == 1)
        entropyCoder.EncodeInteger(entropyCoder.mInferiorBitPlane, MINIMUM_BITPLANE_PRECISION);

    //EncodePartitionStep(imt, position, length, entropyCoder, scaledLambda);
    EncodePartitionStep(imt, position, length, entropyCoder, lambda);
        
}

void TransformPartition :: EncodePartitionStep(MultiscaleTransform &imt, int *position, int *length, Hierarchical4DEncoder &entropyCoder, double lambda) {
    
    if(mPartitionCode[mPartitionCodeIndex] == NOSPLITFLAG) {
      
        mPartitionCodeIndex++;
        
        entropyCoder.EncodePartitionFlag(NOSPLITFLAGSYMBOL);
        
        entropyCoder.mSubbandLF.SetDimension(length[0], length[1], length[2], length[3]);
        entropyCoder.mSubbandLF.CopySubblockFrom(mPartitionData, position[0], position[1], position[2], position[3]);
        entropyCoder.EncodeSubblock(lambda);

        imt.InverseTransform4D(entropyCoder.mSubbandLF);
        mPartitionData.CopySubblockFrom(entropyCoder.mSubbandLF, 0, 0, 0, 0, position[0], position[1], position[2], position[3]);
        
        return;
    }
    if(mPartitionCode[mPartitionCodeIndex] == INTRAVIEWSPLITFLAG) {
        
        mPartitionCodeIndex++;
        
        entropyCoder.EncodePartitionFlag(INTRAVIEWSPLITFLAGSYMBOL);
        
        int new_position[4], new_length[4];
        
        new_position[0] = position[0];
        new_position[1] = position[1];
        new_position[2] = position[2];
        new_position[3] = position[3];
        
        new_length[0] = length[0];
        new_length[1] = length[1];
        new_length[2] = length[2]/2;
        new_length[3] = length[3]/2;
        
        //Encode four spatial subblocks 
        EncodePartitionStep(imt, new_position, new_length, entropyCoder, lambda);

        new_position[3] = position[3] + length[3]/2;
        new_length[3] = length[3] - length[3]/2;
        
        EncodePartitionStep(imt, new_position, new_length, entropyCoder, lambda);

        new_position[2] = position[2] + length[2]/2;
        new_length[2] = length[2] - length[2]/2;
        
        EncodePartitionStep(imt, new_position, new_length, entropyCoder, lambda);
        
        new_position[3] = position[3];
        new_length[3] = length[3]/2;
        
        EncodePartitionStep(imt, new_position, new_length, entropyCoder, lambda);
        return;
    }
    if(mPartitionCode[mPartitionCodeIndex] == INTERVIEWSPLITFLAG) {
        
        mPartitionCodeIndex++;
        
        entropyCoder.EncodePartitionFlag(INTERVIEWSPLITFLAGSYMBOL);
        
        int new_position[4], new_length[4];
        
        new_position[0] = position[0];
        new_position[1] = position[1];
        new_position[2] = position[2];
        new_position[3] = position[3];
        
        new_length[0] = length[0]/2;
        new_length[1] = length[1]/2;
        new_length[2] = length[2];
        new_length[3] = length[3];
        
        //Encode four view subblocks 
        EncodePartitionStep(imt, new_position, new_length, entropyCoder, lambda);
        //optimize partition for Block_V returning JV, the transformed Block_V, partitionCode_S and arithmetic_model_S

        new_position[1] = position[1] + length[1]/2;
        new_length[1] = length[1] - length[1]/2;
        
        EncodePartitionStep(imt, new_position, new_length, entropyCoder, lambda);

        new_position[0] = position[0] + length[0]/2;
        new_length[0] = length[0] - length[0]/2;
        
        EncodePartitionStep(imt, new_position, new_length, entropyCoder, lambda);
        
        new_position[1] = position[1];
        new_length[1] = length[1]/2;
        
        EncodePartitionStep(imt, new_position, new_length, entropyCoder, lambda);
        return;
    }
}

/*************** NO INVERSE ************************/
double TransformPartition :: RDoptimizeTransform(Block4D &inputBlock, MultiscaleTransform &mt, Hierarchical4DEncoder &entropyCoder, double lambda, int evaluate_minimum_bitplane) {
/*! Evaluates the Lagrangian cost of the optimum multiscale transform for the input block as well as the transformed block */   
    if(mPartitionCode != NULL)
        delete [] mPartitionCode;
    mPartitionCode = new char [1];
    mPartitionCode[0] = 0;          //initializes the partition code string as the null string
    //mEvaluateOptimumBitPlane = 1;
    mEvaluateOptimumBitPlane = evaluate_minimum_bitplane;
    mPartitionData.SetDimension(inputBlock.mlength_t, inputBlock.mlength_s, inputBlock.mlength_v, inputBlock.mlength_u);
    mDecodedData.SetDimension(inputBlock.mlength_t, inputBlock.mlength_s, inputBlock.mlength_v, inputBlock.mlength_u);
    //double scaledLambda = mPartitionData.mlength_t*mPartitionData.mlength_s;
    //scaledLambda *= lambda*mPartitionData.mlength_v*mPartitionData.mlength_u;
    
    int position[4];
    position[0] = 0;
    position[1] = 0;
    position[2] = 0;
    position[3] = 0;

    int length[4];
    length[0] = inputBlock.mlength_t;
    length[1] = inputBlock.mlength_s;
    length[2] = inputBlock.mlength_v;
    length[3] = inputBlock.mlength_u;
   
    //copies the current entropyCoder arithmetic model to the optimizer model. 
    //entropyCoder.LoadOptimizerState();
    
    Block4D transformedBlock;
    transformedBlock.SetDimension(length[0], length[1], length[2], length[3]);
    Block4D decodedBlock;
    decodedBlock.SetDimension(length[0], length[1], length[2], length[3]);

    
    mLagrangianCost = RDoptimizeTransformStep(inputBlock, transformedBlock, decodedBlock, position, length, mt, entropyCoder, lambda, &mPartitionCode);
    
    mPartitionData.CopySubblockFrom(transformedBlock, 0, 0, 0, 0);
    
    mDecodedData.CopySubblockFrom(decodedBlock, 0, 0, 0, 0);
    
    //Restores state since the encoder will reevaluate it    
    //entropyCoder.LoadOptimizerState();
    
    //printf("mPartitionCode = %s\n", mPartitionCode);    
    //printf("mInferiorBitPlane = %d\n", entropyCoder.mInferiorBitPlane);

    return(mLagrangianCost);    
}

double TransformPartition :: RDoptimizeTransformStep(Block4D &inputBlock, Block4D &transformedBlock, Block4D &decodedBlock, int *position, int *length, MultiscaleTransform &mt, Hierarchical4DEncoder &entropyCoder, double lambda, char **partitionCode) {
/*! returns the Lagrangian cost of one step of the optimization of the multiscale transform for the input block as well as the transformed block */   
 
    //J0 = cost of full transform
    //saves the current entropyCoder arithmetic model to current_model. 
    ProbabilityModel *currentCoderModelState=NULL;
    entropyCoder.GetOptimizerProbabilisticModelState(&currentCoderModelState);
    char *partitionCodeS=NULL, *partitionCodeV=NULL; 
    
    //copy the inputBlock to block_0 and apply transformation using the appropriate scale from mt    
    Block4D block_0;
    block_0.SetDimension(length[0], length[1], length[2], length[3]);
    block_0.CopySubblockFrom(inputBlock, position[0], position[1], position[2], position[3]);
    mt.Transform4D(block_0);
   
    //copy the transformed input block to entropyCoder.mSubbandLF    
    entropyCoder.mSubbandLF.SetDimension(block_0.mlength_t, block_0.mlength_s, block_0.mlength_v, block_0.mlength_u);
    entropyCoder.mSubbandLF.CopySubblockFrom(block_0, 0, 0, 0, 0);
    double Energy;
    if(mEvaluateOptimumBitPlane == 1) {
        entropyCoder.mInferiorBitPlane = entropyCoder.OptimumBitplane(lambda);
        entropyCoder.LoadOptimizerState();
        mEvaluateOptimumBitPlane = 0;
    }

    Block4D decodedBlock_0;
    decodedBlock_0.SetDimension(length[0], length[1], length[2], length[3]);
    
    //call RdOptimizeHexadecaTree method from entropyCoder to evaluate J0
    if(entropyCoder.mSegmentationTreeCodeBuffer != NULL)
        delete [] entropyCoder.mSegmentationTreeCodeBuffer;
    entropyCoder.mSegmentationTreeCodeBuffer = new char [2];
    strcpy(entropyCoder.mSegmentationTreeCodeBuffer,"");
    double J0 = entropyCoder.RdOptimizeHexadecaTree(0, 0, 0, 0, entropyCoder.mSubbandLF.mlength_t, entropyCoder.mSubbandLF.mlength_s, entropyCoder.mSubbandLF.mlength_v, entropyCoder.mSubbandLF.mlength_u, lambda, entropyCoder.mSuperiorBitPlane, &entropyCoder.mSegmentationTreeCodeBuffer, Energy);

    entropyCoder.RdDecodeHexadecatree();
    decodedBlock_0.CopySubblockFrom(entropyCoder.mDecodedLF,0,0,0,0);
   
    //char *block0SegmentationCode = new char [1+strlen(entropyCoder.mSegmentationTreeCodeBuffer)];
    //strcpy(block0SegmentationCode, entropyCoder.mSegmentationTreeCodeBuffer);
    
    //int block0MinimumBitplane = entropyCoder.mInferiorBitPlane;
    
    //saves the resulting entropyCoder arithmetic model to model_0
    ProbabilityModel *coderModelState_0=NULL;
    entropyCoder.GetOptimizerProbabilisticModelState(&coderModelState_0);
    
    //JS = cost of four quarter spatial subblocks
    //Restores the current arithmetic model using current_model. 
    entropyCoder.SetOptimizerProbabilisticModelState(currentCoderModelState);
    double JS = -1.0;
    Block4D transformedBlockS;
    transformedBlockS.SetDimension(length[0], length[1], length[2], length[3]);
    
    Block4D decodedBlockS;
    decodedBlockS.SetDimension(length[0], length[1], length[2], length[3]);

    if((length[3] >= 2*mlength_u_min)&&(length[2] >= 2*mlength_v_min)) {
        JS = 0.0;
        
        
        char *partitionCodeS00 = new char[1];
        char *partitionCodeS01 = new char[1];
        char *partitionCodeS10 = new char[1];
        char *partitionCodeS11 = new char[1];
        
        partitionCodeS00[0] = 0;
        partitionCodeS01[0] = 0;
        partitionCodeS10[0] = 0;
        partitionCodeS11[0] = 0;
       
        int new_position[4], new_length[4];
        
        new_position[0] = position[0];
        new_position[1] = position[1];
        new_position[2] = position[2];
        new_position[3] = position[3];
        
        new_length[0] = length[0];
        new_length[1] = length[1];
        new_length[2] = length[2]/2;
        new_length[3] = length[3]/2;
          
        //optimize partition for Block_S returning JS, the transformed Block_S, partitionCode_S and arithmetic_model_S
        Block4D transformedBlockS00;
        transformedBlockS00.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        Block4D decodedBlockS00;
        decodedBlockS00.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);

        JS += RDoptimizeTransformStep(inputBlock, transformedBlockS00, decodedBlockS00, new_position, new_length, mt, entropyCoder, lambda, &partitionCodeS00);

        new_position[3] = position[3] + length[3]/2;
        new_length[3] = length[3] - length[3]/2;
                 
        Block4D transformedBlockS01;
        transformedBlockS01.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        Block4D decodedBlockS01;
        decodedBlockS01.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);

        JS += RDoptimizeTransformStep(inputBlock, transformedBlockS01, decodedBlockS01, new_position, new_length, mt, entropyCoder, lambda, &partitionCodeS01);

        new_position[2] = position[2] + length[2]/2;
        new_length[2] = length[2] - length[2]/2;
        
        Block4D transformedBlockS11;
        transformedBlockS11.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        Block4D decodedBlockS11;
        decodedBlockS11.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        JS += RDoptimizeTransformStep(inputBlock, transformedBlockS11, decodedBlockS11, new_position, new_length, mt, entropyCoder, lambda, &partitionCodeS11);
        
        new_position[3] = position[3];
        new_length[3] = length[3]/2;
        
        Block4D transformedBlockS10;
        transformedBlockS10.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        Block4D decodedBlockS10;
        decodedBlockS10.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        JS += RDoptimizeTransformStep(inputBlock, transformedBlockS10, decodedBlockS10, new_position, new_length, mt, entropyCoder, lambda, &partitionCodeS10);
              
        partitionCodeS = new char [2+strlen(partitionCodeS00)+strlen(partitionCodeS01)+strlen(partitionCodeS10)+strlen(partitionCodeS11)];
        strcpy(partitionCodeS, partitionCodeS00);
        strcat(partitionCodeS, partitionCodeS01);
        strcat(partitionCodeS, partitionCodeS11);
        strcat(partitionCodeS, partitionCodeS10);
        
        transformedBlockS.CopySubblockFrom(transformedBlockS00, 0, 0, 0, 0);
        transformedBlockS.CopySubblockFrom(transformedBlockS01, 0, 0, 0, 0, 0, 0, 0, length[3]/2);
        transformedBlockS.CopySubblockFrom(transformedBlockS11, 0, 0, 0, 0, 0, 0, length[2]/2, length[3]/2);
        transformedBlockS.CopySubblockFrom(transformedBlockS10, 0, 0, 0, 0, 0, 0, length[2]/2, 0);
        
        decodedBlockS.CopySubblockFrom(decodedBlockS00, 0, 0, 0, 0);
        decodedBlockS.CopySubblockFrom(decodedBlockS01, 0, 0, 0, 0, 0, 0, 0, length[3]/2);
        decodedBlockS.CopySubblockFrom(decodedBlockS11, 0, 0, 0, 0, 0, 0, length[2]/2, length[3]/2);
        decodedBlockS.CopySubblockFrom(decodedBlockS10, 0, 0, 0, 0, 0, 0, length[2]/2, 0);
        
        delete [] partitionCodeS00;
        delete [] partitionCodeS01;
        delete [] partitionCodeS10;
        delete [] partitionCodeS11;
        
        transformedBlockS00.SetDimension(0, 0, 0, 0);
        transformedBlockS01.SetDimension(0, 0, 0, 0);
        transformedBlockS11.SetDimension(0, 0, 0, 0);
        transformedBlockS10.SetDimension(0, 0, 0, 0);
        
        decodedBlockS00.SetDimension(0, 0, 0, 0);
        decodedBlockS01.SetDimension(0, 0, 0, 0);
        decodedBlockS11.SetDimension(0, 0, 0, 0);
        decodedBlockS10.SetDimension(0, 0, 0, 0);

     }
    //saves the resulting entropyCoder arithmetic model to model_s
    ProbabilityModel *coderModelState_s=NULL;
    entropyCoder.GetOptimizerProbabilisticModelState(&coderModelState_s);
    
    //JV = cost of four quarter view subblocks
    //Restores the current arithmetic model using current_model. 
    entropyCoder.SetOptimizerProbabilisticModelState(currentCoderModelState);
    double JV = -1.0;
    Block4D transformedBlockV;
    transformedBlockV.SetDimension(length[0], length[1], length[2], length[3]);

    Block4D decodedBlockV;
    decodedBlockV.SetDimension(length[0], length[1], length[2], length[3]);
    
    if((length[0] >= 2*mlength_t_min)&&(length[1] >= 2*mlength_s_min)) {
        JV = 0.0;
        
        
        char *partitionCodeV00 = new char[1];
        char *partitionCodeV01 = new char[1];
        char *partitionCodeV10 = new char[1];
        char *partitionCodeV11 = new char[1];
        
        partitionCodeV00[0] = 0;
        partitionCodeV01[0] = 0;
        partitionCodeV10[0] = 0;
        partitionCodeV11[0] = 0;
       
        int new_position[4], new_length[4];
        
        new_position[0] = position[0];
        new_position[1] = position[1];
        new_position[2] = position[2];
        new_position[3] = position[3];
        
        new_length[0] = length[0]/2;
        new_length[1] = length[1]/2;
        new_length[2] = length[2];
        new_length[3] = length[3];
        
        //optimize partition for Block_V returning JV, the transformed Block_V, partitionCode_S and arithmetic_model_S
        Block4D transformedBlockV00;
        transformedBlockV00.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        Block4D decodedBlockV00;
        decodedBlockV00.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);

        JV += RDoptimizeTransformStep(inputBlock, transformedBlockV00, decodedBlockV00, new_position, new_length, mt, entropyCoder, lambda, &partitionCodeV00);

        new_position[1] = position[1] + length[1]/2;
        new_length[1] = length[1] - length[1]/2;
        
        Block4D transformedBlockV01;
        transformedBlockV01.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        Block4D decodedBlockV01;
        decodedBlockV01.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);

        JV += RDoptimizeTransformStep(inputBlock, transformedBlockV01, decodedBlockV01, new_position, new_length, mt, entropyCoder, lambda, &partitionCodeV01);

        new_position[0] = position[0] + length[0]/2;
        new_length[0] = length[0] - length[0]/2;
        
        Block4D transformedBlockV11;
        transformedBlockV11.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        Block4D decodedBlockV11;
        decodedBlockV11.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        JV += RDoptimizeTransformStep(inputBlock, transformedBlockV11, decodedBlockV11, new_position, new_length, mt, entropyCoder, lambda, &partitionCodeV11);
        
        new_position[1] = position[1];
        new_length[1] = length[1]/2;
        
        Block4D transformedBlockV10;
        transformedBlockV10.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        Block4D decodedBlockV10;
        decodedBlockV10.SetDimension(new_length[0], new_length[1], new_length[2], new_length[3]);
        
        JV += RDoptimizeTransformStep(inputBlock, transformedBlockV10, decodedBlockV10, new_position, new_length, mt, entropyCoder, lambda, &partitionCodeV10);
        
        partitionCodeV = new char [2+strlen(partitionCodeV00)+strlen(partitionCodeV01)+strlen(partitionCodeV10)+strlen(partitionCodeV11)];
        strcpy(partitionCodeV, partitionCodeV00);
        strcat(partitionCodeV, partitionCodeV01);
        strcat(partitionCodeV, partitionCodeV11);
        strcat(partitionCodeV, partitionCodeV10);
        
        transformedBlockV.CopySubblockFrom(transformedBlockV00, 0, 0, 0, 0);
        transformedBlockV.CopySubblockFrom(transformedBlockV01, 0, 0, 0, 0, 0, length[1]/2, 0, 0);
        transformedBlockV.CopySubblockFrom(transformedBlockV11, 0, 0, 0, 0, length[0]/2, length[1]/2, 0, 0);
        transformedBlockV.CopySubblockFrom(transformedBlockV10, 0, 0, 0, 0, length[0]/2, 0, 0, 0);
        
        decodedBlockV.CopySubblockFrom(decodedBlockV00, 0, 0, 0, 0);
        decodedBlockV.CopySubblockFrom(decodedBlockV01, 0, 0, 0, 0, 0, length[1]/2, 0, 0);
        decodedBlockV.CopySubblockFrom(decodedBlockV11, 0, 0, 0, 0, length[0]/2, length[1]/2, 0, 0);
        decodedBlockV.CopySubblockFrom(decodedBlockV10, 0, 0, 0, 0, length[0]/2, 0, 0, 0);

        delete [] partitionCodeV00;
        delete [] partitionCodeV01;
        delete [] partitionCodeV10;
        delete [] partitionCodeV11;
        
        transformedBlockV00.SetDimension(0, 0, 0, 0);
        transformedBlockV01.SetDimension(0, 0, 0, 0);
        transformedBlockV11.SetDimension(0, 0, 0, 0);
        transformedBlockV10.SetDimension(0, 0, 0, 0);
        
        decodedBlockV00.SetDimension(0, 0, 0, 0);
        decodedBlockV01.SetDimension(0, 0, 0, 0);
        decodedBlockV11.SetDimension(0, 0, 0, 0);
        decodedBlockV10.SetDimension(0, 0, 0, 0);

   }

    //saves the resulting entropyCoder arithmetic model to model_v
    ProbabilityModel *coderModelState_v=NULL;
    entropyCoder.GetOptimizerProbabilisticModelState(&coderModelState_v);

    if(J0 > 0) 
        J0 += 1.0*lambda;
    if(JV > 0)
        JV += 2.0*lambda;
    if(JS > 0)
        JS += 2.0*lambda;
    
    //choose the lower cost and returns the corresponding cost,  the partition code and the arithmetic coder model
    //find best J
    int interview_split = 0;
    int intraview_split = 0;
    int no_split = 0;
   
    if(JV >= 0) {
        if(JS >= 0) {
            if(JV < JS) {
                if(JV < J0) {
                    interview_split = 1;
                }
                else {
                    no_split = 1;                    
                }
            }
            else {                
                if(JS < J0) {
                    intraview_split = 1;
                }
                else {
                    no_split = 1;                    
                }
            }
        }
        else {
            if(JV < J0) {
                interview_split = 1;
            }
            else {
                no_split = 1;
            }            
        }
    }
    else {
        if(JS >= 0) {
            if(JS < J0) {
                intraview_split = 1;
            }
            else {
                no_split = 1;
            }
        }
        else {
            no_split = 1;
        }
    }
    double optimumJ=0;
    if((interview_split + intraview_split + no_split) != 1) {
        printf("ERRO: partition fail/n");
        exit(0);
    }
    //reallocates memory for the partitionCode string based on the current length and the length of the chosen one
    //copies data from the chosen arithmetic coder model to the current model
    char flagCode[2];
    flagCode[1] = 0;
    if(interview_split == 1) {
        optimumJ = JV;
        char *code = new char[2+strlen(*partitionCode)+strlen(partitionCodeV)];
        strcpy(code, *partitionCode);
        flagCode[0] = INTERVIEWSPLITFLAG;
        strcat(code, flagCode);
        strcat(code, partitionCodeV);
        delete(*partitionCode);
        *partitionCode = code;
        entropyCoder.SetOptimizerProbabilisticModelState(coderModelState_v);
        transformedBlock.CopySubblockFrom(transformedBlockV, 0, 0, 0, 0);
        decodedBlock.CopySubblockFrom(decodedBlockV, 0, 0, 0, 0);
    }
    if(intraview_split == 1) {
        optimumJ = JS;
        char *code = new char[2+strlen(*partitionCode)+strlen(partitionCodeS)];
        strcpy(code, *partitionCode);
        flagCode[0] = INTRAVIEWSPLITFLAG;
        strcat(code, flagCode);
        strcat(code, partitionCodeS);
        delete(*partitionCode);
        *partitionCode = code;
        entropyCoder.SetOptimizerProbabilisticModelState(coderModelState_s);
        transformedBlock.CopySubblockFrom(transformedBlockS, 0, 0, 0, 0);
        decodedBlock.CopySubblockFrom(decodedBlockS, 0, 0, 0, 0);
    }
    if(no_split == 1) {
        optimumJ = J0;   
        char *code = new char[2+strlen(*partitionCode)];
        strcpy(code, *partitionCode);
        flagCode[0] = NOSPLITFLAG;
        strcat(code, flagCode);
        delete(*partitionCode);
        *partitionCode = code;
        entropyCoder.SetOptimizerProbabilisticModelState(coderModelState_0);
        transformedBlock.CopySubblockFrom(block_0, 0, 0, 0, 0);
        //Block4D decodedBlock_0;
        //entropyCoder.BlockQuantization(decodedBlock_0, block_0, block0SegmentationCode, block0MinimumBitplane);    
        //imt.InverseTransform4D(decodedBlock_0);
        //decodedBlock.CopySubblockFrom(decodedBlock_0, 0, 0, 0, 0);
        //decodedBlock_0.SetDimension(0,0,0,0);
    }
    //deletes temporary strings, blocks and models
   
    if(partitionCodeV != NULL) {
        delete [] partitionCodeV;
    }
    if(partitionCodeS != NULL) {
        delete [] partitionCodeS;
    }

    //delete [] block0SegmentationCode;
    
    entropyCoder.DeleteProbabilisticModelState(&currentCoderModelState);
    entropyCoder.DeleteProbabilisticModelState(&coderModelState_0);
    entropyCoder.DeleteProbabilisticModelState(&coderModelState_s);
    entropyCoder.DeleteProbabilisticModelState(&coderModelState_v);
    
    block_0.SetDimension(0,0,0,0);
    decodedBlock_0.SetDimension(0,0,0,0);
    transformedBlockS.SetDimension(0,0,0,0);
    transformedBlockV.SetDimension(0,0,0,0);

    //return optimum J
    return(optimumJ);
    
}

void TransformPartition :: EncodePartition(Hierarchical4DEncoder &entropyCoder, double lambda, int encode_minimum_bitplane) {
    
    //double scaledLambda = mPartitionData.mlength_t*mPartitionData.mlength_s;
    //scaledLambda *= lambda*mPartitionData.mlength_v*mPartitionData.mlength_u;
    
    mPartitionCodeIndex = 0;
    
    int position[4];
    position[0] = 0;
    position[1] = 0;
    position[2] = 0;
    position[3] = 0;

    int length[4];
    length[0] = mPartitionData.mlength_t;
    length[1] = mPartitionData.mlength_s;
    length[2] = mPartitionData.mlength_v;
    length[3] = mPartitionData.mlength_u;
    
    printf("mPartitionCode = %s\n", mPartitionCode);    
    printf("mInferiorBitPlane = %d\n", entropyCoder.mInferiorBitPlane);
     
    if(encode_minimum_bitplane == 1)
        entropyCoder.EncodeInteger(entropyCoder.mInferiorBitPlane, MINIMUM_BITPLANE_PRECISION);

    //EncodePartitionStep(imt, position, length, entropyCoder, scaledLambda);
    EncodePartitionStep(position, length, entropyCoder, lambda);
        
}

void TransformPartition :: EncodePartitionStep(int *position, int *length, Hierarchical4DEncoder &entropyCoder, double lambda) {
    
    if(mPartitionCode[mPartitionCodeIndex] == NOSPLITFLAG) {
      
        mPartitionCodeIndex++;
        
        entropyCoder.EncodePartitionFlag(NOSPLITFLAGSYMBOL);
        
        entropyCoder.mSubbandLF.SetDimension(length[0], length[1], length[2], length[3]);
        entropyCoder.mSubbandLF.CopySubblockFrom(mPartitionData, position[0], position[1], position[2], position[3]);
        entropyCoder.EncodeSubblock(lambda);
       
        return;
    }
    if(mPartitionCode[mPartitionCodeIndex] == INTRAVIEWSPLITFLAG) {
        
        mPartitionCodeIndex++;
        
        entropyCoder.EncodePartitionFlag(INTRAVIEWSPLITFLAGSYMBOL);
        
        int new_position[4], new_length[4];
        
        new_position[0] = position[0];
        new_position[1] = position[1];
        new_position[2] = position[2];
        new_position[3] = position[3];
        
        new_length[0] = length[0];
        new_length[1] = length[1];
        new_length[2] = length[2]/2;
        new_length[3] = length[3]/2;
        
        //Encode four spatial subblocks 
        EncodePartitionStep(new_position, new_length, entropyCoder, lambda);

        new_position[3] = position[3] + length[3]/2;
        new_length[3] = length[3] - length[3]/2;
        
        EncodePartitionStep(new_position, new_length, entropyCoder, lambda);

        new_position[2] = position[2] + length[2]/2;
        new_length[2] = length[2] - length[2]/2;
        
        EncodePartitionStep(new_position, new_length, entropyCoder, lambda);
        
        new_position[3] = position[3];
        new_length[3] = length[3]/2;
        
        EncodePartitionStep(new_position, new_length, entropyCoder, lambda);
        return;
    }
    if(mPartitionCode[mPartitionCodeIndex] == INTERVIEWSPLITFLAG) {
        
        mPartitionCodeIndex++;
        
        entropyCoder.EncodePartitionFlag(INTERVIEWSPLITFLAGSYMBOL);
        
        int new_position[4], new_length[4];
        
        new_position[0] = position[0];
        new_position[1] = position[1];
        new_position[2] = position[2];
        new_position[3] = position[3];
        
        new_length[0] = length[0]/2;
        new_length[1] = length[1]/2;
        new_length[2] = length[2];
        new_length[3] = length[3];
        
        //Encode four view subblocks 
        EncodePartitionStep(new_position, new_length, entropyCoder, lambda);
        //optimize partition for Block_V returning JV, the transformed Block_V, partitionCode_S and arithmetic_model_S

        new_position[1] = position[1] + length[1]/2;
        new_length[1] = length[1] - length[1]/2;
        
        EncodePartitionStep(new_position, new_length, entropyCoder, lambda);

        new_position[0] = position[0] + length[0]/2;
        new_length[0] = length[0] - length[0]/2;
        
        EncodePartitionStep(new_position, new_length, entropyCoder, lambda);
        
        new_position[1] = position[1];
        new_length[1] = length[1]/2;
        
        EncodePartitionStep(new_position, new_length, entropyCoder, lambda);
        return;
    }
}

