#include "Block4D.h"
#include "MultiscaleTransform.h"
#include "SlantHierarchy.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*******************************************************************************/
/*                         SlantHierarchy class methods                        */
/*******************************************************************************/
SlantHierarchy :: SlantHierarchy(void) {
    mPreComputedSlants.Clear();
    mSlantTreeCode.Clear();
    mUseDepthMap = 0;
    mInverseDepthMapZero = 0;
    mInverseDepthMapGain = 1.0;
    mSlantScaleU = 1.0;
    mSlantScaleV = 1.0;
    mMapAssistedSearchRange = 0;
}

SlantHierarchy :: ~SlantHierarchy(void) {
    mPreComputedSlants.Clear();
    mSlantTreeCode.Clear();
}

void SlantHierarchy :: FindOptimumSlantTree(const Block4D &inputBlock, MultiscaleTransform &mt, double lambda, int v_position, int u_position) {
  
    mSlantBits = 1;
    int value = 2;
    while(value-1 < mSlantMax) {
        value = value << 1;
        mSlantBits++;
    }
    mSlantBits++;
        
    //evaluates the optimum bitplane for the DCT-transformed input block
    mMinimumBitplane = OptimumBitPlane(inputBlock, mt, lambda);
printf("minimum bitplane optimization %d\n",  mMinimumBitplane); 
    mSlantTreeCode.Clear();
    int uvPixelsPosition[4];
    uvPixelsPosition[0] = v_position;
    uvPixelsPosition[1] = v_position+inputBlock.mlength_v-1;
    uvPixelsPosition[2] = u_position;
    uvPixelsPosition[3] = u_position+inputBlock.mlength_u-1;
    FindOptimumSlantTreeStep(inputBlock, mSlantTreeCode, mt, lambda, uvPixelsPosition, 0);
    
}

double SlantHierarchy :: FindOptimumSlantTreeStep(const Block4D &inputBlock, IntegerString &treeCode, MultiscaleTransform &mt, double lambda, int *UVBlockPixelsPosition, int accumulatedSlant) {
 
    //Evaluate cost of leaf node
    int optimumSlant;
    double J0;
    Block4D slantedBlock, trimmedBlock;
    int maxSlant = mSlantMax;
    int validSlant = 0;
/*    
    if(maxSlant > inputBlock.mlength_u*(inputBlock.mlength_s-1))
        maxSlant = inputBlock.mlength_u*(inputBlock.mlength_s-1);
    if(maxSlant > inputBlock.mlength_v*(inputBlock.mlength_t-1))
        maxSlant = inputBlock.mlength_v*(inputBlock.mlength_t-1);
*/    
    int slant_v, slant_u;
    ScaleSlant(maxSlant, slant_v, slant_u, inputBlock.mlength_t, inputBlock.mlength_s);
    while((slant_v > inputBlock.mlength_v*(inputBlock.mlength_t-1))||(slant_u > inputBlock.mlength_u*(inputBlock.mlength_s-1))) {
        maxSlant--;     
        ScaleSlant(maxSlant, slant_v, slant_u, inputBlock.mlength_t, inputBlock.mlength_s);
   }
    
    if((mPreComputedSlants.mLength > 0)&&(mPreComputedSlants.mSearchIndex <= mPreComputedSlants.mLength)) {
        optimumSlant = mPreComputedSlants.GetInt();
	int slant;
	validSlant = FindBestSlant4DCT(inputBlock, optimumSlant, optimumSlant, slantedBlock, mt, lambda, slant, J0);
    }
    else {
        if(mUseDepthMap == 1) {
printf("UVBlockPixelsPosition = [%d %d %d %d]\n", UVBlockPixelsPosition[0], UVBlockPixelsPosition[1], UVBlockPixelsPosition[2], UVBlockPixelsPosition[3]);
            int meanSlant = GetMeanSlantFromMapPGM(UVBlockPixelsPosition);
printf("meanSlant = %d\n", meanSlant);
printf("accumulatedSlant = %d\n", accumulatedSlant); 
	    meanSlant = meanSlant - accumulatedSlant;
            int slant_inf = meanSlant - mMapAssistedSearchRange;
            if(slant_inf < -maxSlant)
                slant_inf = -maxSlant;
	    
	    int slant_sup = meanSlant + mMapAssistedSearchRange;
            if(slant_sup > maxSlant)
                slant_sup = maxSlant;
	    
            validSlant = FindBestSlant4DCT(inputBlock, slant_inf, slant_sup, slantedBlock, mt, lambda, optimumSlant, J0);
       }
        else
            validSlant = FindBestSlant4DCT(inputBlock, -maxSlant, maxSlant, slantedBlock, mt, lambda, optimumSlant, J0);
    } 
    
    if(validSlant == 0) {       
printf("invalidSlant = %d\n", optimumSlant); 
	FindBestSlant4DCT(inputBlock, 0, 0, slantedBlock, mt, lambda, optimumSlant, J0);
        if(optimumSlant != 0) {
            printf("ERROR: invalidSlant but optimumSlant != 0\n");
	    exit(0);
        }
    }
    
    ScaleSlant(optimumSlant, slant_v, slant_u, inputBlock.mlength_t, inputBlock.mlength_s);
    slantedBlock.Slant(slant_v, slant_u, inputBlock);
    trimmedBlock.Trim(slantedBlock);
    treeCode.Append(optimumSlant);
    J0 = J0+lambda*(1.0+mSlantBits); 
    //J0 = J0+1.0+mSlantBits; 

    //returns if minimum slant block size reached
    //if((trimmedBlock.mlength_v < 2*mMinBlockSize_v)||(trimmedBlock.mlength_u < 2*mMinBlockSize_u)) {
    if((trimmedBlock.mlength_v < 2*mMinBlockSize_v)||(trimmedBlock.mlength_u < 2*mMinBlockSize_u)||(validSlant == 0)) {
      
        slantedBlock.SetDimension(0,0,0,0);
        trimmedBlock.SetDimension(0,0,0,0);
        treeCode.Append(LEAF_NODE);
	return(J0);
      
    }
    
    //Evaluate cost of splitting
    IntegerString subTreeCode00, subTreeCode01, subTreeCode10, subTreeCode11;
    subTreeCode00.Clear();
    subTreeCode01.Clear();
    subTreeCode10.Clear();
    subTreeCode11.Clear();
    
    int uvSubblockPixelsPosition00[4];
    int uvSubblockPixelsPosition01[4];
    int uvSubblockPixelsPosition10[4];
    int uvSubblockPixelsPosition11[4];
    
    int length_v = UVBlockPixelsPosition[1]-UVBlockPixelsPosition[0]+1;
    int length_u = UVBlockPixelsPosition[3]-UVBlockPixelsPosition[2]+1;
    
    uvSubblockPixelsPosition00[0]= UVBlockPixelsPosition[0];
    uvSubblockPixelsPosition00[1]= UVBlockPixelsPosition[0]+length_v/2-1;
    uvSubblockPixelsPosition00[2]= UVBlockPixelsPosition[2];
    uvSubblockPixelsPosition00[3]= UVBlockPixelsPosition[2]+length_u/2-1;
    
    uvSubblockPixelsPosition01[0]= UVBlockPixelsPosition[0];
    uvSubblockPixelsPosition01[1]= UVBlockPixelsPosition[0]+length_v/2-1;
    uvSubblockPixelsPosition01[2]= UVBlockPixelsPosition[2]+length_u/2;
    uvSubblockPixelsPosition01[3]= UVBlockPixelsPosition[3];
    
    uvSubblockPixelsPosition10[0]= UVBlockPixelsPosition[0]+length_v/2;
    uvSubblockPixelsPosition10[1]= UVBlockPixelsPosition[1];
    uvSubblockPixelsPosition10[2]= UVBlockPixelsPosition[2];
    uvSubblockPixelsPosition10[3]= UVBlockPixelsPosition[2]+length_u/2-1;
    
    uvSubblockPixelsPosition11[0]= UVBlockPixelsPosition[0]+length_v/2;
    uvSubblockPixelsPosition11[1]= UVBlockPixelsPosition[1];
    uvSubblockPixelsPosition11[2]= UVBlockPixelsPosition[2]+length_u/2;
    uvSubblockPixelsPosition11[3]= UVBlockPixelsPosition[3];
   
    Block4D subBlock00;
    subBlock00.SetDimension(trimmedBlock.mlength_t, trimmedBlock.mlength_s, trimmedBlock.mlength_v/2, trimmedBlock.mlength_u/2);
    subBlock00.CopySubblockFrom(trimmedBlock,0,0,0,0);
    double J1 = FindOptimumSlantTreeStep(subBlock00, subTreeCode00, mt, lambda, uvSubblockPixelsPosition00, accumulatedSlant+optimumSlant);

    Block4D subBlock01;
    subBlock01.SetDimension(trimmedBlock.mlength_t, trimmedBlock.mlength_s, trimmedBlock.mlength_v/2, trimmedBlock.mlength_u-trimmedBlock.mlength_u/2);
    subBlock01.CopySubblockFrom(trimmedBlock,0,0,0,trimmedBlock.mlength_u/2);
    J1 += FindOptimumSlantTreeStep(subBlock01, subTreeCode01, mt, lambda, uvSubblockPixelsPosition01, accumulatedSlant+optimumSlant);

    Block4D subBlock10;
    subBlock10.SetDimension(trimmedBlock.mlength_t, trimmedBlock.mlength_s, trimmedBlock.mlength_v-trimmedBlock.mlength_v/2, trimmedBlock.mlength_u/2);
    subBlock10.CopySubblockFrom(trimmedBlock,0,0,trimmedBlock.mlength_v/2,0);
    J1 += FindOptimumSlantTreeStep(subBlock10, subTreeCode10, mt, lambda, uvSubblockPixelsPosition10, accumulatedSlant+optimumSlant);
     
    Block4D subBlock11;
    subBlock11.SetDimension(trimmedBlock.mlength_t, trimmedBlock.mlength_s, trimmedBlock.mlength_v-trimmedBlock.mlength_v/2, trimmedBlock.mlength_u-trimmedBlock.mlength_u/2);
    subBlock11.CopySubblockFrom(trimmedBlock,0,0,trimmedBlock.mlength_v/2, trimmedBlock.mlength_u/2);
    J1 += FindOptimumSlantTreeStep(subBlock11, subTreeCode11, mt, lambda, uvSubblockPixelsPosition11, accumulatedSlant+optimumSlant);

    subBlock00.SetDimension(0,0,0,0);
    subBlock01.SetDimension(0,0,0,0);
    subBlock10.SetDimension(0,0,0,0);
    subBlock11.SetDimension(0,0,0,0);
    slantedBlock.SetDimension(0,0,0,0);
    trimmedBlock.SetDimension(0,0,0,0);
    
    J1 += lambda*(1.0+mSlantBits);
    //J1 += 1.0+mSlantBits;

    if(J0 <= J1) {
        treeCode.Append(LEAF_NODE);
	return(J0);
    }
    else {
        treeCode.Append(SPLIT_NODE);
	treeCode.Cat(subTreeCode00);
	treeCode.Cat(subTreeCode01);
	treeCode.Cat(subTreeCode10);
	treeCode.Cat(subTreeCode11);
	return(J1);      
    }
}

int SlantHierarchy :: ReadOptimumSlantTree(const Block4D &inputBlock, MultiscaleTransform &mt, double lambda, FILE *ifp) {
  
    mSlantBits = 1;
    int value = 2;
    while(value-1 < mSlantMax) {
        value = value << 1;
        mSlantBits++;
    }
    mSlantBits++;
        
    //evaluates the optimum bitplane for the DCT-transformed input block
    mMinimumBitplane = OptimumBitPlane(inputBlock, mt, lambda);
printf("minimum bitplane optimization %d\n",  mMinimumBitplane); 
    mSlantTreeCode.Clear();
    
    int failFlag=0;
    failFlag = ReadOptimumSlantTreeStep(mSlantTreeCode, ifp);
    
    return(failFlag);
    
}

int SlantHierarchy :: ReadOptimumSlantTreeStep(IntegerString &treeCode, FILE *ifp) {
 
    int optimumSlant;
    int numberOfItemsRead=0;
    char slantFlag[32];
    char fillerString[32];
    int failFlag=0;

    //Reads slant flag from file
    numberOfItemsRead = fscanf(ifp, "%s", slantFlag);
//printf("Leu %s ", slantFlag);
    numberOfItemsRead = fscanf(ifp, "%s", fillerString);
    numberOfItemsRead = fscanf(ifp, "%s", fillerString);

    //Reads optimum slant from file
    numberOfItemsRead = fscanf(ifp, "%d", &optimumSlant);
//printf("Leu %d\n", optimumSlant);
    
    if(numberOfItemsRead == 1)
        treeCode.Append(optimumSlant);
    else {
        failFlag = 1;
        printf("ERROR recovering slant tree.\n");
        return(failFlag);
    }

    //returns if leaf node reached
    if(strcmp(slantFlag, "SLANT_LEAF_NODE,") == 0) {
      
        treeCode.Append(LEAF_NODE);
	return(failFlag);
      
    }
    if(strcmp(slantFlag, "SLANT_SPLIT_NODE,") != 0) {
        printf("ERROR recovering slant tree.\n");
	failFlag = 1;
	return(failFlag);
    }
        
    IntegerString subTreeCode00, subTreeCode01, subTreeCode10, subTreeCode11;
    subTreeCode00.Clear();
    subTreeCode01.Clear();
    subTreeCode10.Clear();
    subTreeCode11.Clear();

    failFlag = ReadOptimumSlantTreeStep(subTreeCode00, ifp);
    if(failFlag == 1) {
        printf("ERROR recovering slant tree.\n");
        return(failFlag);
    }
    
    failFlag = ReadOptimumSlantTreeStep(subTreeCode01, ifp);
    if(failFlag == 1) {
        printf("ERROR recovering slant tree.\n");
        return(failFlag);
    }
    
    failFlag = ReadOptimumSlantTreeStep(subTreeCode10, ifp);
    if(failFlag == 1) {
        printf("ERROR recovering slant tree.\n"); 
        return(failFlag);
    }
    
    failFlag = ReadOptimumSlantTreeStep(subTreeCode11, ifp);
    if(failFlag == 1) {
        printf("ERROR recovering slant tree.\n"); 
        return(failFlag);
    }
    
    treeCode.Append(SPLIT_NODE);
    treeCode.Cat(subTreeCode00);
    treeCode.Cat(subTreeCode01);
    treeCode.Cat(subTreeCode10);
    treeCode.Cat(subTreeCode11);   
    
    return(failFlag);
    
}

void SlantHierarchy :: EncodeOptimumSlantTree(const Block4D &inputBlock, TransformPartition &tp, MultiscaleTransform &mt, Hierarchical4DEncoder &he, double lambda) {

    mSlantTreeCode.Rewind();
    EncodeOptimumSlantTreeStep(inputBlock, tp, mt, he, lambda);
  
}

void SlantHierarchy :: EncodeOptimumSlantTreeStep(const Block4D &inputBlock, TransformPartition &tp, MultiscaleTransform &mt, Hierarchical4DEncoder &he, double lambda) {
          
    //encode slant using mSlantBits)
    int optimumSlant = mSlantTreeCode.GetInt();
    he.EncodeInteger(optimumSlant+mSlantMax, mSlantBits);
    
    Block4D slantedBlock, trimmedBlock;
    
    int slant_v, slant_u;
    ScaleSlant(optimumSlant, slant_v, slant_u, inputBlock.mlength_t, inputBlock.mlength_s);
    slantedBlock.Slant(slant_v, slant_u, inputBlock);
    trimmedBlock.Trim(slantedBlock);

    int slantTreeFlag = mSlantTreeCode.GetInt();
    //if leaf
    if(slantTreeFlag == LEAF_NODE) {
        
        //encode LEAF_NODE flag
        he.EncodeInteger(LEAF_NODE, 1);
        printf("SLANT_LEAF_NODE, optimumSlant = %d\n", optimumSlant);
        printf("slant_v = %d, slant_u = %d\n", slant_v, slant_u);
	//encode block

        he.RestartProbabilisticModel();
	trimmedBlock.Padding();
for(int t=0; t < trimmedBlock.mlength_t; t++) {
  for(int s=0; s < trimmedBlock.mlength_s; s++) {
    for(int v=0; v < trimmedBlock.mlength_v; v++) {
      for(int u=0; u < trimmedBlock.mlength_u; u++) {
	 if(trimmedBlock.mPixel[t][s][v][u] == EMPTY_VALUE) {
	    printf("Error!!! EMPTY_VALUE at %d %d %d %d\n", t, s, v, u); 
	    exit(0);
	 }
      }
    }
  }
}
  
        tp.RDoptimizeTransform(trimmedBlock, mt, he, lambda, 1);
        tp.EncodePartition(he, lambda, 1);
        printf("block of size (%d %d %d %d) encoded\n", trimmedBlock.mlength_t, trimmedBlock.mlength_s, trimmedBlock.mlength_v, trimmedBlock.mlength_u);

      
    }
    else {
    
        //encode SPLIT_NODE flag
        he.EncodeInteger(SPLIT_NODE, 1);
        printf("SLANT_SPLIT_NODE, optimumSlant = %d\n", optimumSlant);
        printf("slant_v = %d, slant_u = %d\n", slant_v, slant_u);

	//encode sub-trees
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
	
        EncodeOptimumSlantTreeStep(subBlock00, tp, mt, he, lambda);
        EncodeOptimumSlantTreeStep(subBlock01, tp, mt, he, lambda);
        EncodeOptimumSlantTreeStep(subBlock10, tp, mt, he, lambda);
        EncodeOptimumSlantTreeStep(subBlock11, tp, mt, he, lambda);
	
        subBlock00.SetDimension(0,0,0,0);
        subBlock01.SetDimension(0,0,0,0);
        subBlock10.SetDimension(0,0,0,0);
        subBlock11.SetDimension(0,0,0,0);
	
    }
    
    slantedBlock.SetDimension(0,0,0,0);
    trimmedBlock.SetDimension(0,0,0,0);
    
}

void SlantHierarchy :: PruneOptimumSlantTree(const Block4D &inputBlock, MultiscaleTransform &mt, double lambda) {
  
    mSlantBits = 1;
    int value = 2;
    while(value-1 < mSlantMax) {
        value = value << 1;
        mSlantBits++;
    }
    mSlantBits++;
    
    //evaluates the optimum bitplane for the DCT-transformed input block
    //mMinimumBitplane = OptimumBitPlane(inputBlock, mt, lambda);
    
    mPreComputedSlants.Copy(mSlantTreeCode);

    mSlantTreeCode.Clear();
    PruneOptimumSlantTreeStep(inputBlock, mSlantTreeCode, mt, lambda);
    mPreComputedSlants.Clear();
    
}

double SlantHierarchy :: PruneOptimumSlantTreeStep(const Block4D &inputBlock, IntegerString &treeCode, MultiscaleTransform &mt, double lambda) {

    //Evaluate cost of leaf node
    int optimumSlant;
    double J0;
    double Distortion;
    Block4D slantedBlock, trimmedBlock;
     
    optimumSlant = mPreComputedSlants.GetInt();
    
    int slant_v, slant_u;
    ScaleSlant(optimumSlant, slant_v, slant_u, inputBlock.mlength_t, inputBlock.mlength_s);
    slantedBlock.Slant(slant_v, slant_u, inputBlock);
    trimmedBlock.Trim(slantedBlock);
    treeCode.Append(optimumSlant);
    slantedBlock.SetDimension(trimmedBlock.mlength_t, trimmedBlock.mlength_s, trimmedBlock.mlength_v, trimmedBlock.mlength_u);
    slantedBlock.CopySubblockFrom(trimmedBlock,0,0,0,0);
    int number_of_elements = slantedBlock.NumberOfValidElements();
    slantedBlock.Padding();
    mt.Transform4D(slantedBlock);
    J0 = EvaluateRate(slantedBlock, 0, 0, 0, 0, slantedBlock.mlength_t, slantedBlock.mlength_s, slantedBlock.mlength_v, slantedBlock.mlength_u, 30, mMinimumBitplane);
    Distortion = EvaluateDistortion(slantedBlock, mMinimumBitplane);
    Distortion *= number_of_elements;
    Distortion /= slantedBlock.mlength_t*slantedBlock.mlength_s*slantedBlock.mlength_v*slantedBlock.mlength_u;
    J0 = J0+1.0+mSlantBits; 
    J0 = Distortion + lambda*J0;
    //J0 = lambda*J0;

    int slantTreeFlag = mPreComputedSlants.GetInt();
    //returns if minimum slant block size reached
    if(slantTreeFlag == LEAF_NODE) {
      
        slantedBlock.SetDimension(0,0,0,0);
        trimmedBlock.SetDimension(0,0,0,0);
        treeCode.Append(LEAF_NODE);
	return(J0);
      
    }
    
    //Evaluate cost of splitting
    IntegerString subTreeCode00, subTreeCode01, subTreeCode10, subTreeCode11;
    subTreeCode00.Clear();
    subTreeCode01.Clear();
    subTreeCode10.Clear();
    subTreeCode11.Clear();
    
    Block4D subBlock00;
    subBlock00.SetDimension(trimmedBlock.mlength_t, trimmedBlock.mlength_s, trimmedBlock.mlength_v/2, trimmedBlock.mlength_u/2);
    subBlock00.CopySubblockFrom(trimmedBlock,0,0,0,0);
    double J1 = PruneOptimumSlantTreeStep(subBlock00, subTreeCode00, mt, lambda);

    Block4D subBlock01;
    subBlock01.SetDimension(trimmedBlock.mlength_t, trimmedBlock.mlength_s, trimmedBlock.mlength_v/2, trimmedBlock.mlength_u-trimmedBlock.mlength_u/2);
    subBlock01.CopySubblockFrom(trimmedBlock,0,0,0,trimmedBlock.mlength_u/2);
    J1 += PruneOptimumSlantTreeStep(subBlock01, subTreeCode01, mt, lambda);

    Block4D subBlock10;
    subBlock10.SetDimension(trimmedBlock.mlength_t, trimmedBlock.mlength_s, trimmedBlock.mlength_v-trimmedBlock.mlength_v/2, trimmedBlock.mlength_u/2);
    subBlock10.CopySubblockFrom(trimmedBlock,0,0,trimmedBlock.mlength_v/2,0);
    J1 += PruneOptimumSlantTreeStep(subBlock10, subTreeCode10, mt, lambda);
     
    Block4D subBlock11;
    subBlock11.SetDimension(trimmedBlock.mlength_t, trimmedBlock.mlength_s, trimmedBlock.mlength_v-trimmedBlock.mlength_v/2, trimmedBlock.mlength_u-trimmedBlock.mlength_u/2);
    subBlock11.CopySubblockFrom(trimmedBlock,0,0,trimmedBlock.mlength_v/2, trimmedBlock.mlength_u/2);
    J1 += PruneOptimumSlantTreeStep(subBlock11, subTreeCode11, mt, lambda);

    subBlock00.SetDimension(0,0,0,0);
    subBlock01.SetDimension(0,0,0,0);
    subBlock10.SetDimension(0,0,0,0);
    subBlock11.SetDimension(0,0,0,0);
    slantedBlock.SetDimension(0,0,0,0);
    trimmedBlock.SetDimension(0,0,0,0);
    
    J1 += lambda*(1.0+mSlantBits);
    //J1 += 1.0+mSlantBits;

    if(J0 <= J1) {
        treeCode.Append(LEAF_NODE);
	return(J0);
    }
    else {
        treeCode.Append(SPLIT_NODE);
	treeCode.Cat(subTreeCode00);
	treeCode.Cat(subTreeCode01);
	treeCode.Cat(subTreeCode10);
	treeCode.Cat(subTreeCode11);
	return(J1);      
    }
}

int SlantHierarchy :: FindBestSlant4DCT(const Block4D &inputBlock, int min_slant, int max_slant, Block4D &slantedBlock, MultiscaleTransform &mt, double lambda, int &optimumSlant, double &optimumJ) {
  
  double rate;
  double J;
  double Distortion=0.0;
  
  Block4D preTrimmedBlock;
  
  int firstValid = 0;
  
  //INIT optimumJ here to avoid compilation warnings
    
  for(int slant = min_slant; slant <= max_slant; slant++) { 
//printf("block (%d %d %d %d), try slant = %d\n", inputBlock.mlength_t, inputBlock.mlength_s, inputBlock.mlength_v, inputBlock.mlength_u, slant);    
      int slant_v, slant_u;
      ScaleSlant(slant, slant_v, slant_u, inputBlock.mlength_t, inputBlock.mlength_s);
      preTrimmedBlock.Slant(slant_v, slant_u, inputBlock);
      slantedBlock.Trim(preTrimmedBlock);
      int number_of_elements = slantedBlock.NumberOfValidElements();
      int error = slantedBlock.Padding();

      if(error == 0) {
          mt.Transform4D(slantedBlock);
    
          rate = EvaluateRate(slantedBlock, 0, 0, 0, 0, slantedBlock.mlength_t, slantedBlock.mlength_s, slantedBlock.mlength_v, slantedBlock.mlength_u, 30, mMinimumBitplane);
          Distortion = EvaluateDistortion(slantedBlock, mMinimumBitplane);
          Distortion *= number_of_elements;
          Distortion /= slantedBlock.mlength_t*slantedBlock.mlength_s*slantedBlock.mlength_v*slantedBlock.mlength_u;
          J = Distortion + lambda*rate;
	  //J = rate;
	  //J = lambda*rate;
	  
          if((J < optimumJ)||(firstValid == 0)) {
              optimumJ = J;
              optimumSlant = slant;
              firstValid = 1;
          }
      }
    
  }
/*  
  if(firstValid == 0) { //Failed to find a suitable slant
    
      optimumSlant = 0; //best choice if slants fail
      slantedBlock.Trim(inputBlock);
      mt.Transform4D(slantedBlock);
      rate = EvaluateRate(slantedBlock, 0, 0, 0, 0, slantedBlock.mlength_t, slantedBlock.mlength_s, slantedBlock.mlength_v, slantedBlock.mlength_u, 30, mMinimumBitplane);
      Distortion = EvaluateDistortion(slantedBlock, mMinimumBitplane);
      optimumJ = Distortion + lambda*rate;
      
  }
*/
  return(firstValid);
  preTrimmedBlock.SetDimension(0,0,0,0);
}

int SlantHierarchy :: EvaluateRate(const Block4D &lfBlock, int position_t, int position_s, int position_v, int position_u, int length_t, int length_s, int length_v, int length_u, int currentBitplane, int minBitplane) {

    if(currentBitplane < minBitplane) {
            
        return(0);
    }
    
    if(length_t*length_s*length_v*length_u == 1) {
        int rate = currentBitplane-minBitplane+1;
	int magnitude = lfBlock.mPixel[position_t][position_s][position_v][position_u];
        if(magnitude < 0)
            magnitude *= -1;
	if((magnitude>>minBitplane) != 0)
            rate++;
        return(rate);
    }
    
    int Significance = 0;
    
    int Threshold = 1 << currentBitplane;
    
    for(int index_t = position_t; index_t < position_t+length_t; index_t++) {
        
        for(int index_s = position_s; index_s < position_s+length_s; index_s++) {
            
            for(int index_v = position_v; index_v < position_v+length_v; index_v++) {
                
                for(int index_u = position_u; index_u < position_u+length_u; index_u++) {
                    
                    if((index_t < lfBlock.mlength_t)&&(index_s < lfBlock.mlength_s)&&(index_v < lfBlock.mlength_v)&&(index_u < lfBlock.mlength_u)) {
                        
                        if(lfBlock.mPixel[index_t][index_s][index_v][index_u] >= Threshold) Significance = 1;
                        if(lfBlock.mPixel[index_t][index_s][index_v][index_u] <= -Threshold) Significance = 1;
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
    
    if(Significance == 0) {
        
        int bits;
        bits = EvaluateRate(lfBlock, position_t, position_s, position_v, position_u, length_t, length_s, length_v, length_u, currentBitplane-1, minBitplane);
        
        return(bits);
    }
    
    else {
      
        int bits = 0;
       
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
                        
                        bits += EvaluateRate(lfBlock, new_position_t, new_position_s, new_position_v, new_position_u, new_length_t, new_length_s, new_length_v, new_length_u, currentBitplane, minBitplane);
                
                   }
                    
                }
                
            }
            
        }
        
        return(bits);
    }

}

double SlantHierarchy :: EvaluateDistortion(const Block4D &lfBlock, int minBitplane) {
  
    double Distortion=0;
    for(int index_t = 0; index_t < lfBlock.mlength_t; index_t++) {
        for(int index_s = 0; index_s < lfBlock.mlength_s; index_s++) {
            for(int index_v = 0; index_v < lfBlock.mlength_v; index_v++) {
                for(int index_u = 0; index_u < lfBlock.mlength_u; index_u++) {
                    long int coefficient = lfBlock.mPixel[index_t][index_s][index_v][index_u];
                    long int quantizedMagnitude = abs(coefficient);
		    quantizedMagnitude = (quantizedMagnitude>>minBitplane)<<minBitplane;
		    if(quantizedMagnitude > 0) 
                        quantizedMagnitude += (1 << minBitplane)/2;  
		    if(quantizedMagnitude < 0) {
                       printf("ERROR: Magnitude < 0\n");
		       exit(0);
                    }
                    //double error = abs(coefficient)-((coefficient>>minBitplane)<<minBitplane);
                    double error = abs(coefficient)-quantizedMagnitude;
                    Distortion += error*error;
                }
            }
        }
    }
    
    return(Distortion);
  
}

double SlantHierarchy :: EvaluateJ(const Block4D &lfBlock, int position_t, int position_s, int position_v, int position_u, int length_t, int length_s, int length_v, int length_u, int currentBitplane, int minBitplane, float lambda) {

    if(currentBitplane < minBitplane) {
            
        double signalEnergy = 0;
        for(int index_t = 0; index_t < length_t; index_t++) {
            for(int index_s = 0; index_s < length_s; index_s++) {
                for(int index_v = 0; index_v < length_v; index_v++) {
                    for(int index_u = 0; index_u < length_u; index_u++) {
                        int coefficient = lfBlock.mPixel[position_t+index_t][position_s+index_s][position_v+index_v][position_u+index_u];
                        double J = coefficient;
                        signalEnergy += J*J;
                    }
                }
            }
        }
        return (signalEnergy);
        
    }
    
    if(length_t*length_s*length_v*length_u == 1) {
      
        int magnitude = lfBlock.mPixel[position_t][position_s][position_v][position_u];
        int signal = 0;
        if(magnitude < 0) {
            magnitude = -magnitude;
            signal = 1;
        }
        int allZeros = 1;
                
        int onesMask = 0;
        onesMask = ~onesMask;

        double J = 0;
        double accumulatedRate = 0;
        for(int bit_position = currentBitplane; bit_position >= minBitplane; bit_position--) {
            accumulatedRate += 1.0; 
	    int bit = (magnitude >> bit_position)&01;
            if(bit == 1)
                allZeros = 0;
       }
        if(allZeros == 0)
            accumulatedRate += 1.0;

        int bitMask = onesMask << minBitplane;
        int quantizedMagnitude = magnitude&bitMask;
        if(allZeros == 0) {
            quantizedMagnitude += (1 << minBitplane)/2;
        }  
        J = magnitude - quantizedMagnitude;
        
        J = J*J + lambda*(accumulatedRate);
        
        return(J);
        
    }
    
    int Significance = 0;
    
    int Threshold = 1 << currentBitplane;
    
    for(int index_t = position_t; index_t < position_t+length_t; index_t++) {
        
        for(int index_s = position_s; index_s < position_s+length_s; index_s++) {
            
            for(int index_v = position_v; index_v < position_v+length_v; index_v++) {
                
                for(int index_u = position_u; index_u < position_u+length_u; index_u++) {
                    
                    if((index_t < lfBlock.mlength_t)&&(index_s < lfBlock.mlength_s)&&(index_v < lfBlock.mlength_v)&&(index_u < lfBlock.mlength_u)) {
                        
                        if(lfBlock.mPixel[index_t][index_s][index_v][index_u] >= Threshold) Significance = 1;
                        if(lfBlock.mPixel[index_t][index_s][index_v][index_u] <= -Threshold) Significance = 1;
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
    
    if(Significance == 0) {
        
        double J;
        J = EvaluateRate(lfBlock, position_t, position_s, position_v, position_u, length_t, length_s, length_v, length_u, currentBitplane-1, minBitplane);
        
        return(J);
    }
    
    else {
      
        double J = 0;
       
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
                        
                        J += EvaluateRate(lfBlock, new_position_t, new_position_s, new_position_v, new_position_u, new_length_t, new_length_s, new_length_v, new_length_u, currentBitplane, minBitplane);
                
                   }
                    
                }
                
            }
            
        }
        
        return(J+lambda);
    }

}

int SlantHierarchy :: OptimumBitPlane(const Block4D &inputBlock, MultiscaleTransform &mt, float lambda) {
  
    Block4D lfBlock;
    lfBlock.SetDimension(inputBlock.mlength_t, inputBlock.mlength_s, inputBlock.mlength_v, inputBlock.mlength_u);
    lfBlock.CopySubblockFrom(inputBlock,0,0,0,0);
    mt.Transform4D(lfBlock);

    long int blockSize = lfBlock.mlength_t*lfBlock.mlength_s;
    blockSize *= lfBlock.mlength_v*lfBlock.mlength_u;
    double Jmin=0;            //Irrelevant initial value
    int optimumBitplane=0;    //Irrelevant initial value
    
    double accumulatedRate = 0;
    
    
    
    for(int bit_position = 30; bit_position >= 0; bit_position--) {
        
        double distortion = 0.0;
        double coefficientsDistortion = 0.0;
        double signalRate = 0.0;
        double J;
        int numberOfCoefficients = 0;
        
        int onesMask = 0;
        onesMask = ~onesMask;
        int bitMask = onesMask << bit_position;
       

        for(long int coefficient_index=0; coefficient_index < blockSize; coefficient_index++) {
        
            int magnitude = lfBlock.mPixelData[coefficient_index];
            if(magnitude < 0) {
                magnitude = -magnitude;
            }
            int Threshold = (1 << bit_position);
            if(magnitude >= Threshold) {
                //int bit = (magnitude >> bit_position)&01;
                accumulatedRate += 1.0;
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
        
        if((J <= Jmin)||(bit_position == 30)) {
            Jmin = J;
            optimumBitplane = bit_position;
        }
       
    }
    lfBlock.SetDimension(0,0,0,0);
    return(optimumBitplane);
}

void SlantHierarchy :: ScaleSlant(int slant, int &slant_v, int &slant_u, int length_t, int length_s) {
  
    if(length_t < length_s) {
        slant_v = round((mSlantScaleV*slant*(length_t-1))/(length_s-1));
        slant_u = round(mSlantScaleU*slant);
    }
    else {
        slant_u = round((mSlantScaleU*slant*(length_s-1))/(length_t-1));
        slant_v = round(mSlantScaleV*slant);
    }
    
}

int SlantHierarchy :: GetMeanSlantFromMapPGM(int *position) {
  
    long int meanDisparity=0;
    int numberOfPoints=0;
    int position_v = position[0]; 
    int position_u = position[2];
    int length_v = position[1]-position[0];
    int length_u = position[3]-position[2];
    
    for(int index_v = position_v; index_v < position_v+length_v; index_v++) {
      
        for(int index_u = position_u; index_u < position_u+length_u; index_u++) {
            if((index_v < mInverseDepthMapPGM.mLines)&&(index_u < mInverseDepthMapPGM.mColumns)) {
                meanDisparity += mInverseDepthMapPGM.GetPixel(index_v, index_u);
                numberOfPoints++;
            }
        }
    }
    
    if(numberOfPoints > 0)
        meanDisparity /= numberOfPoints;
    else {
        printf("ERROR: Empty map region\n");
	exit(0);
    }
    meanDisparity -= mInverseDepthMapZero;
    return(round(mInverseDepthMapGain*meanDisparity));
}
