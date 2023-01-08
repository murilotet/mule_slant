#include "MultiscaleTransform.h"
/*******************************************************************************/
/*                        MultiscaleTransform class methods                    */
/*******************************************************************************/
MultiscaleTransform :: MultiscaleTransform(void) {
    
    mlength_t_max = 0;
    mlength_s_max = 0;
    mlength_v_max = 0;
    mlength_u_max = 0;
    mTransform = NULL;
    mTransformGain_t = 1.0;
    mTransformGain_s = 1.0;
    mTransformGain_v = 1.0;
    mTransformGain_u = 1.0;
    mBypass_t = 0;
    mBypass_s = 0;
    mBypass_v = 0;
    mBypass_u = 0;
   
}
MultiscaleTransform :: ~MultiscaleTransform(void) {
 
    if(mTransform != NULL) {
        MatrixListNode *searchMatrixPointer = mTransform;
        searchMatrixPointer->nodeMatrix->SetDimension(0,0);
        while(searchMatrixPointer->nextNode != NULL) {
            searchMatrixPointer->nodeMatrix->SetDimension(0,0);
            searchMatrixPointer = searchMatrixPointer->nextNode;
        }
        delete [] mTransform;
        mTransform = NULL;
    }
    
}

void MultiscaleTransform :: SetDimension(int max_t, int max_s, int max_v, int max_u) {

    
    
    if(mTransform != NULL) {
        MatrixListNode *searchMatrixPointer = mTransform;
        searchMatrixPointer->nodeMatrix->SetDimension(0,0);
        while(searchMatrixPointer->nextNode != NULL) {
            searchMatrixPointer->nodeMatrix->SetDimension(0,0);
            searchMatrixPointer = searchMatrixPointer->nextNode;
        }
        delete [] mTransform;
        mTransform = NULL;
    }
    
    if(max_t*max_s*max_v*max_u == 0) {
        mlength_t_max = 0;
        mlength_s_max = 0;
        mlength_v_max = 0;
        mlength_u_max = 0;
        return;
    }
        
    mlength_t_max = max_t;
    mlength_s_max = max_s;
    mlength_v_max = max_v;
    mlength_u_max = max_u;

    mNumberOfScales = 0;
    printf("Maximum transform lengths = (%d, %d, %d, %d)\n", mlength_t_max, mlength_s_max, mlength_v_max, mlength_u_max);
    
}


double* MultiscaleTransform :: TransformCoefficients(int length, int type) {
      
    if((length > mlength_t_max)&&(length > mlength_s_max)&&(length > mlength_v_max)&&(length > mlength_u_max)) {
        printf("Error: request for transform lenght (%d) greater than the maximum allowed \n", length);
        exit(0);
    }
    MatrixListNode *searchMatrixPointer = mTransform;
    if(searchMatrixPointer != NULL) {
        if(length == searchMatrixPointer->nodeMatrix->mLines) {
            return(searchMatrixPointer->nodeMatrix->mElementData);
        }        
        while(searchMatrixPointer->nextNode != NULL) {
 
            searchMatrixPointer = searchMatrixPointer->nextNode;
            if(length == searchMatrixPointer->nodeMatrix->mLines) {
                return(searchMatrixPointer->nodeMatrix->mElementData);
            }        
        }
    }

    
    //include new length x length dct or idct transform matrix in the list
    if(searchMatrixPointer != NULL) {
        searchMatrixPointer->nextNode = new MatrixListNode;
        searchMatrixPointer = searchMatrixPointer->nextNode;
    }
    else {
        mTransform = new MatrixListNode;
        searchMatrixPointer = mTransform;
    }
 
    searchMatrixPointer->nodeMatrix = new Matrix;
    searchMatrixPointer->nodeMatrix->SetDimension(length, length);
    
    if(type == 0) 
        searchMatrixPointer->nodeMatrix->DCT();
    
    if(type == 1) 
        searchMatrixPointer->nodeMatrix->IDCT();
    
    searchMatrixPointer->nextNode = NULL;
    mNumberOfScales++;
printf("Incluiu a %d escala (%d)\n",  mNumberOfScales, length);
    return searchMatrixPointer->nodeMatrix->mElementData;

}

void MultiscaleTransform :: Transform4D(Block4D &block) {
  
     if(mBypass_u == 0) {
        double transform_gain_u = 1.0*mlength_u_max;
        transform_gain_u = sqrt(transform_gain_u/block.mlength_u);
        block.TRANSFORM_U(mTransformGain_u*transform_gain_u, TransformCoefficients(block.mlength_u, 0));
    }
    else
        block *= mTransformGain_u;
    if(mBypass_v == 0) {
        double transform_gain_v = 1.0*mlength_v_max;
        transform_gain_v = sqrt(transform_gain_v/block.mlength_v);
        block.TRANSFORM_V(mTransformGain_v*transform_gain_v, TransformCoefficients(block.mlength_v, 0));
    }
    else
        block *= mTransformGain_v;
    if(mBypass_s == 0) {
        double transform_gain_s = 1.0*mlength_s_max;
        transform_gain_s = sqrt(transform_gain_s/block.mlength_s);
        block.TRANSFORM_S(mTransformGain_s*transform_gain_s, TransformCoefficients(block.mlength_s, 0));
    }
    else
        block *= mTransformGain_s;
    if(mBypass_t == 0) {
        double transform_gain_t = 1.0*mlength_t_max;
        transform_gain_t = sqrt(transform_gain_t/block.mlength_t);
        block.TRANSFORM_T(mTransformGain_t*transform_gain_t, TransformCoefficients(block.mlength_t, 0)); 
    }
    else
        block *= mTransformGain_t;
      
}

void MultiscaleTransform :: InverseTransform4D(Block4D &block) {
    
     if(mBypass_u == 0) {
        double transform_gain_u = 1.0*mlength_u_max;
        transform_gain_u = sqrt(transform_gain_u/block.mlength_u);
        block.TRANSFORM_U(mTransformGain_u/transform_gain_u, TransformCoefficients(block.mlength_u, 1));
    }
    else
        block *= mTransformGain_u;
    if(mBypass_v == 0) {
        double transform_gain_v = 1.0*mlength_v_max;
        transform_gain_v = sqrt(transform_gain_v/block.mlength_v);
        block.TRANSFORM_V(mTransformGain_v/transform_gain_v, TransformCoefficients(block.mlength_v, 1));
    }
    else
        block *= mTransformGain_v;
    if(mBypass_s == 0) {
        double transform_gain_s = 1.0*mlength_s_max;
        transform_gain_s = sqrt(transform_gain_s/block.mlength_s);
        block.TRANSFORM_S(mTransformGain_s/transform_gain_s, TransformCoefficients(block.mlength_s, 1));
    }
    else
        block *= mTransformGain_s;
    if(mBypass_t == 0) {
        double transform_gain_t = 1.0*mlength_t_max;
        transform_gain_t = sqrt(transform_gain_t/block.mlength_t);
        block.TRANSFORM_T(mTransformGain_t/transform_gain_t, TransformCoefficients(block.mlength_t, 1));   
    }
    else
        block *= mTransformGain_t;
  
}
