#include "PartitionDecoder.h"

/*******************************************************************************/
/*                      PartitionDecoder class methods                         */
/*******************************************************************************/

PartitionDecoder :: PartitionDecoder(void) {
    mUseSameBitPlane = 1;
}
   
void PartitionDecoder :: DecodePartition(Hierarchical4DDecoder &entropyDecoder, MultiscaleTransform &mt, int getminimumbitplane) {
    
    
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
    
    if(getminimumbitplane == 1)
        entropyDecoder.mInferiorBitPlane = entropyDecoder.DecodeInteger(MINIMUM_BITPLANE_PRECISION);

    DecodePartitionStep(position, length, entropyDecoder, mt);
        
}

void PartitionDecoder :: DecodePartitionStep(int *position, int *length, Hierarchical4DDecoder &entropyDecoder, MultiscaleTransform &mt) {

    
    int flagCode = entropyDecoder.DecodePartitionFlag();
    
    if(flagCode == NOSPLITFLAGSYMBOL) {
 
        entropyDecoder.mSubbandLF.SetDimension(length[0], length[1], length[2], length[3]);
        entropyDecoder.mSubbandLF.Zeros();
        entropyDecoder.DecodeBlock(0, 0, 0, 0, length[0], length[1], length[2], length[3], entropyDecoder.mSuperiorBitPlane); 
        mt.InverseTransform4D(entropyDecoder.mSubbandLF);  
        mPartitionData.CopySubblockFrom(entropyDecoder.mSubbandLF, 0, 0, 0, 0, position[0], position[1], position[2], position[3]);
        return;
    }
    
    if(flagCode == INTRAVIEWSPLITFLAGSYMBOL) {
        
        int new_position[4], new_length[4];
        
        new_position[0] = position[0];
        new_position[1] = position[1];
        new_position[2] = position[2];
        new_position[3] = position[3];
        
        new_length[0] = length[0];
        new_length[1] = length[1];
        new_length[2] = length[2]/2;
        new_length[3] = length[3]/2;
        
        //Decode four spatial subblocks 
        DecodePartitionStep(new_position, new_length, entropyDecoder, mt);

        new_position[3] = position[3] + length[3]/2;
        new_length[3] = length[3] - length[3]/2;
        
        DecodePartitionStep(new_position, new_length, entropyDecoder, mt);

        new_position[2] = position[2] + length[2]/2;
        new_length[2] = length[2] - length[2]/2;
        
        DecodePartitionStep(new_position, new_length, entropyDecoder, mt);
        
        new_position[3] = position[3];
        new_length[3] = length[3]/2;
        
        DecodePartitionStep(new_position, new_length, entropyDecoder, mt);
        return;
    }
    
    if(flagCode == INTERVIEWSPLITFLAGSYMBOL) {
        
        int new_position[4], new_length[4];
        
        new_position[0] = position[0];
        new_position[1] = position[1];
        new_position[2] = position[2];
        new_position[3] = position[3];
        
        new_length[0] = length[0]/2;
        new_length[1] = length[1]/2;
        new_length[2] = length[2];
        new_length[3] = length[3];
        
        //Decode four view subblocks 
        DecodePartitionStep(new_position, new_length, entropyDecoder, mt);

        new_position[1] = position[1] + length[1]/2;
        new_length[1] = length[1] - length[1]/2;
        
        DecodePartitionStep(new_position, new_length, entropyDecoder, mt);

        new_position[0] = position[0] + length[0]/2;
        new_length[0] = length[0] - length[0]/2;
        
        DecodePartitionStep(new_position, new_length, entropyDecoder, mt);
        
        new_position[1] = position[1];
        new_length[1] = length[1]/2;
        
        DecodePartitionStep(new_position, new_length, entropyDecoder, mt);
        return;
    }
}
