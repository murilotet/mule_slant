#include "Block4D.h"
#include "MultiscaleTransform.h"
#include "SlantHierarchyDecoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*******************************************************************************/
/*                         SlantHierarchyDecoder class methods                 */
/*******************************************************************************/
SlantHierarchyDecoder :: SlantHierarchyDecoder(void) {
    mSlantScaleU = 1.0;
    mSlantScaleV = 1.0;
}

SlantHierarchyDecoder :: ~SlantHierarchyDecoder(void) {
}

void SlantHierarchyDecoder :: DecodeOptimumSlantTree(Block4D &decodedBlock, PartitionDecoder &pd, MultiscaleTransform &mt, Hierarchical4DDecoder &hd) {

    mSlantBits = 1;
    int value = 2;
    while(value-1 < mSlantMax) {
        value = value << 1;
        mSlantBits++;
    }
    mSlantBits++;
    
    decodedBlock.Zeros();
    DecodeOptimumSlantTreeStep(decodedBlock, pd, mt, hd);
  
}

void SlantHierarchyDecoder :: DecodeOptimumSlantTreeStep(Block4D &decodedBlock, PartitionDecoder &pd, MultiscaleTransform &mt, Hierarchical4DDecoder &hd) {
      
    //decode slant using mSlantBits
    int optimumSlant  = hd.DecodeInteger(mSlantBits);
    optimumSlant -= mSlantMax;

    //position offset of the trimmed block inside the slanted block
    int delta_t, delta_s, delta_v, delta_u; 

    //original  block size
    int length_t = decodedBlock.mlength_t;
    int length_s = decodedBlock.mlength_s;
    int length_v = decodedBlock.mlength_v;
    int length_u = decodedBlock.mlength_u;
    
    //apply slant and trim to zero block to find correct size
    Block4D slantedBlock;
    int slant_v, slant_u;
    ScaleSlant(optimumSlant, slant_v, slant_u, length_t, length_s);
    slantedBlock.Slant(slant_v, slant_u, decodedBlock);
    Block4D trimmedBlock;
    trimmedBlock.Trim(slantedBlock,delta_t, delta_s, delta_v, delta_u);
    slantedBlock.Erase();
    
    
    //decode LEAF_NODE/SPIT_NODE flag
    int flag = hd.DecodeInteger(1);

    //if leaf
    if(flag == LEAF_NODE) {
        
        printf("SLANT_LEAF_NODE, optimumSlant = %d\n", optimumSlant);
	//decode block

	pd.mPartitionData.SetDimension(trimmedBlock.mlength_t, trimmedBlock.mlength_s, trimmedBlock.mlength_v, trimmedBlock.mlength_u);
	pd.mPartitionData.Zeros();
        hd.RestartProbabilisticModel();
        pd.DecodePartition(hd, mt, 1);
	slantedBlock.CopySubblockFrom(pd.mPartitionData,0,0,0,0,delta_t,delta_s,delta_v,delta_u);
	
        printf("block of size (%d %d %d %d) decoded\n", trimmedBlock.mlength_t, trimmedBlock.mlength_s, trimmedBlock.mlength_v, trimmedBlock.mlength_u);

      
    }
    else {
    
        printf("SLANT_SPLIT_NODE, optimumSlant = %d\n", optimumSlant);
	//decode subtrees

        Block4D subBlock00;
        subBlock00.SetDimension(trimmedBlock.mlength_t, trimmedBlock.mlength_s, trimmedBlock.mlength_v/2, trimmedBlock.mlength_u/2);
        subBlock00.CopySubblockFrom(trimmedBlock,0,0,0,0);

        Block4D subBlock01;
        subBlock01.SetDimension(trimmedBlock.mlength_t, trimmedBlock.mlength_s, trimmedBlock.mlength_v/2, trimmedBlock.mlength_u-trimmedBlock.mlength_u/2);
        subBlock01.CopySubblockFrom(trimmedBlock,0,0,0,trimmedBlock.mlength_u/2);

        Block4D subBlock10;
        subBlock10.SetDimension(trimmedBlock.mlength_t, trimmedBlock.mlength_s, trimmedBlock.mlength_v-trimmedBlock.mlength_v/2, trimmedBlock.mlength_u/2);
        subBlock10.CopySubblockFrom(trimmedBlock,0,0,trimmedBlock.mlength_v/2,0);
     
        Block4D subBlock11;
        subBlock11.SetDimension(trimmedBlock.mlength_t, trimmedBlock.mlength_s, trimmedBlock.mlength_v-trimmedBlock.mlength_v/2, trimmedBlock.mlength_u-trimmedBlock.mlength_u/2);
        subBlock11.CopySubblockFrom(trimmedBlock,0,0,trimmedBlock.mlength_v/2, trimmedBlock.mlength_u/2);
	
	//decode sub-trees
        DecodeOptimumSlantTreeStep(subBlock00, pd, mt, hd);
        DecodeOptimumSlantTreeStep(subBlock01, pd, mt, hd);
        DecodeOptimumSlantTreeStep(subBlock10, pd, mt, hd);
        DecodeOptimumSlantTreeStep(subBlock11, pd, mt, hd);
	
	trimmedBlock.CopySubblockFrom(subBlock00,0,0,0,0);
	trimmedBlock.CopySubblockFrom(subBlock01,0,0,0,0,0,0,0,trimmedBlock.mlength_u/2);
	trimmedBlock.CopySubblockFrom(subBlock10,0,0,0,0,0,0,trimmedBlock.mlength_v/2,0);
	trimmedBlock.CopySubblockFrom(subBlock11,0,0,0,0,0,0,trimmedBlock.mlength_v/2, trimmedBlock.mlength_u/2);
	
	slantedBlock.CopySubblockFrom(trimmedBlock,0,0,0,0,delta_t,delta_s,delta_v,delta_u);
	
	subBlock00.SetDimension(0,0,0,0);
	subBlock01.SetDimension(0,0,0,0);
	subBlock10.SetDimension(0,0,0,0);
	subBlock11.SetDimension(0,0,0,0);
	
    }
    
    ScaleSlant(optimumSlant, slant_v, slant_u, length_t, length_s);
    decodedBlock.DeSlant(slant_v, slant_u, slantedBlock);
    
    if((decodedBlock.mlength_u != length_u)||(decodedBlock.mlength_v != length_v)) {
      printf("Erro de sincronizacao!!!\n");
      printf(" %d <> %d %d <> %d\n",decodedBlock.mlength_v, length_v, decodedBlock.mlength_u, length_u);
      printf(" %d %d\n",slantedBlock.mlength_v, slantedBlock.mlength_u);
      printf(" %d %d\n",delta_v, delta_u);
      exit(0);
    }

    slantedBlock.SetDimension(0,0,0,0);
    trimmedBlock.SetDimension(0,0,0,0);
}

void SlantHierarchyDecoder :: ScaleSlant(int slant, int &slant_v, int &slant_u, int length_t, int length_s) {
  
    if(length_t < length_s) {
        slant_v = round((mSlantScaleV*slant*(length_t-1))/(length_s-1));
        slant_u = round(mSlantScaleU*slant);
    }
    else {
        slant_u = round((mSlantScaleU*slant*(length_s-1))/(length_t-1));
        slant_v = round(mSlantScaleV*slant);
    }
    
}