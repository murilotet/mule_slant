#include "Block4D.h"
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*******************************************************************************/
/*                         Block4D class methods                               */
/*******************************************************************************/

Block4D :: Block4D(void) {
    mPixelData = NULL;
    mPixel = NULL;
    mlength_u = mlength_v = mlength_s = mlength_t = 0;
}

Block4D :: ~Block4D(void) {
    SetDimension(0, 0, 0, 0);
}


void Block4D :: SetDimension(int length_t, int length_s, int length_v, int length_u) {
/*! used to allocate memory and set the 4 dimensional array of pointers to manipulate a 4 dimensional block of pixels */ 
    if(mPixelData != NULL) {
	
        for(int t_index = 0; t_index < mlength_t; t_index++) {
            for(int s_index = 0; s_index < mlength_s; s_index++) {
                delete [] mPixel[t_index][s_index];
	    }
            delete [] mPixel[t_index];
	}
        delete [] mPixel;
	
	delete [] mPixelData;
	mPixelData = NULL;
    }
    
    mlength_u = length_u;
    mlength_v = length_v;
    mlength_s = length_s;
    mlength_t = length_t;
    
    if(mlength_u*mlength_v*mlength_s*mlength_t != 0) {
      
        //mPixelData = new block4DElementType [mlength_u*mlength_v*mlength_s*mlength_t];
        long int block_size = mlength_u*mlength_v;
        block_size *= mlength_s*mlength_t;
        mPixelData = new block4DElementType [block_size];
    
        mPixel = new block4DElementType *** [mlength_t];
        for(int t_index = 0; t_index < mlength_t; t_index++) {
            mPixel[t_index] = new block4DElementType ** [mlength_s];
            for(int s_index = 0; s_index < mlength_s; s_index++) {
                mPixel[t_index][s_index] = new block4DElementType * [mlength_v];
                for(int v_index = 0; v_index < mlength_v; v_index++) {
	            mPixel[t_index][s_index][v_index] = &mPixelData[LinearPosition(t_index, s_index, v_index, 0)];
	        }
	    }
        }
        
    }
}

Block4D* Block4D :: operator + (const Block4D &B) {
  
    if((mlength_u == B.mlength_u)&&(mlength_v == B.mlength_v)&&(mlength_s == B.mlength_s)&&(mlength_t == B.mlength_t)) {
        Block4D *Result;
        Result = new Block4D;
        Result->SetDimension(mlength_t,mlength_s,mlength_v,mlength_u);
        for(int n = 0; n < mlength_u*mlength_v*mlength_s*mlength_t; n++) {
  
            Result->mPixelData[n] = mPixelData[n] + B.mPixelData[n];

        }
        return(Result);

    }
    else {
        printf("Error: trying to add two 4D blocks of different dimensions\n");
		throw std::runtime_error("Error: trying to add two 4D blocks of different dimensions\n");
    }
    
}

Block4D* Block4D :: operator * (const Block4D &B) {
  
    if((mlength_u == B.mlength_u)&&(mlength_v == B.mlength_v)&&(mlength_s == B.mlength_s)&&(mlength_t == B.mlength_t)) {
        Block4D *Result;
	Result = new Block4D;
        Result->SetDimension(mlength_t,mlength_s,mlength_v,mlength_u);
        for(int n = 0; n < mlength_u*mlength_v*mlength_s*mlength_t; n++) {
  
            Result->mPixelData[n] = mPixelData[n] * B.mPixelData[n];

        }
        return(Result);

    }
    else {
        printf("Error: trying to multiply two 4D blocks of different dimensions\n");
		throw std::runtime_error("Error: trying to multiply two 4D blocks of different dimensions\n");
    }
    
}

Block4D* Block4D :: operator - (const Block4D &B) {
  
    if((mlength_u == B.mlength_u)&&(mlength_v == B.mlength_v)&&(mlength_s == B.mlength_s)&&(mlength_t == B.mlength_t)) {
        Block4D *Result;
        Result = new Block4D;
        Result->SetDimension(mlength_t,mlength_s,mlength_v,mlength_u);
        for(int n = 0; n < mlength_u*mlength_v*mlength_s*mlength_t; n++) {
  
            Result->mPixelData[n] = mPixelData[n] - B.mPixelData[n];

        }
        return(Result);

    }
    else {
        printf("Error: trying to subtract two 4D blocks of different dimensions\n");
		throw std::runtime_error("Error: trying to subtract two 4D blocks of different dimensions\n");
    }
    
}

Block4D* Block4D :: operator + (const int &a) {
  
        Block4D *Result;
        Result = new Block4D;
        Result->SetDimension(mlength_t,mlength_s,mlength_v,mlength_u);
        for(int n = 0; n < mlength_u*mlength_v*mlength_s*mlength_t; n++) {
  
            Result->mPixelData[n] = mPixelData[n] + a;

        }
        return(Result);

    
}

Block4D* Block4D :: operator * (const int &a) {
  
        Block4D *Result;
	Result = new Block4D;
        Result->SetDimension(mlength_t,mlength_s,mlength_v,mlength_u);
        for(int n = 0; n < mlength_u*mlength_v*mlength_s*mlength_t; n++) {
  
            Result->mPixelData[n] = mPixelData[n] * a;

        }
        return(Result);
    
}

Block4D* Block4D :: operator - (const int &a) {
  
        Block4D *Result;
        Result = new Block4D;
        Result->SetDimension(mlength_t,mlength_s,mlength_v,mlength_u);
        for(int n = 0; n < mlength_u*mlength_v*mlength_s*mlength_t; n++) {
  
            Result->mPixelData[n] = mPixelData[n] - a;

        }
        return(Result);
    
}

Block4D* Block4D :: operator / (const int &a) {
  
        Block4D *Result;
        Result = new Block4D;
        Result->SetDimension(mlength_t,mlength_s,mlength_v,mlength_u);
        for(int n = 0; n < mlength_u*mlength_v*mlength_s*mlength_t; n++) {
  
            Result->mPixelData[n] = mPixelData[n] / a;

        }
        return(Result);
    
}

void Block4D :: operator += (const Block4D &B) {
  
    if((mlength_u == B.mlength_u)&&(mlength_v == B.mlength_v)&&(mlength_s == B.mlength_s)&&(mlength_t == B.mlength_t)) {
        for(int n = 0; n < mlength_u*mlength_v*mlength_s*mlength_t; n++) {
  
            mPixelData[n] += B.mPixelData[n];

        }

    }
    else {
        printf("Error: trying to add two 4D blocks of different dimensions\n");
		throw std::runtime_error("Error: trying to add two 4D blocks of different dimensions\n");
    }
    
}
void Block4D :: operator += (const int &a) {
  
    for(int n = 0; n < mlength_u*mlength_v*mlength_s*mlength_t; n++) {
  
        mPixelData[n] += a;

    }
   
}

void Block4D :: operator *= (const Block4D &B) {
  
    if((mlength_u == B.mlength_u)&&(mlength_v == B.mlength_v)&&(mlength_s == B.mlength_s)&&(mlength_t == B.mlength_t)) {
        for(int n = 0; n < mlength_u*mlength_v*mlength_s*mlength_t; n++) {
  
            mPixelData[n] *= B.mPixelData[n];

        }

    }
    else {
        printf("Error: trying to add two 4D blocks of different dimensions\n");
		throw std::runtime_error("Error: trying to add two 4D blocks of different dimensions\n");
    }
    
}

void Block4D :: operator *= (const double &a) {
  
    for(int n = 0; n < mlength_u*mlength_v*mlength_s*mlength_t; n++) {
  
        double aux = mPixelData[n];
	aux *= a;
        mPixelData[n] = aux;

    }
   
}

void Block4D :: operator -= (const Block4D &B) {
  
    if((mlength_u == B.mlength_u)&&(mlength_v == B.mlength_v)&&(mlength_s == B.mlength_s)&&(mlength_t == B.mlength_t)) {
        for(int n = 0; n < mlength_u*mlength_v*mlength_s*mlength_t; n++) {
  
            mPixelData[n] -= B.mPixelData[n];

        }

    }
    else {
        printf("Error: trying to add two 4D blocks of different dimensions\n");
		throw std::runtime_error("Error: trying to add two 4D blocks of different dimensions\n");
    }
    
}

void Block4D :: operator = (const Block4D &B) {
  
    SetDimension(B.mlength_t,B.mlength_s,B.mlength_v,B.mlength_u);
    for(int n = 0; n < mlength_u*mlength_v*mlength_s*mlength_t; n++) {
  
        mPixelData[n] = B.mPixelData[n];

    }
    
}

void Block4D :: operator = (Block4D* B) {
  
    SetDimension(B->mlength_t,B->mlength_s,B->mlength_v,B->mlength_u);
    for(int n = 0; n < mlength_u*mlength_v*mlength_s*mlength_t; n++) {
  
        mPixelData[n] = B->mPixelData[n];

    }
    if(this != B) {
        B->SetDimension(0, 0, 0, 0);
        delete B;
    }
}

void Block4D :: CopySubblockFrom(const Block4D &B, int source_offset_t, int source_offset_s, int source_offset_v, int source_offset_u, int target_offset_t, int target_offset_s, int target_offset_v, int target_offset_u) {
/*! copy data from the 4DBlock B starting from pixel position at source_offset_t, source_offset_s, source_offset_v and source_offset_u
 to the THIS 4DBlock starting from pixel position at target_offset_t, target_offset_s, target_offset_v and target_offset_u */ 
    
    int Length_t = (mlength_t < B.mlength_t) ? mlength_t : B.mlength_t;
    int Length_s = (mlength_s < B.mlength_s) ? mlength_s : B.mlength_s;
    int Length_v = (mlength_v < B.mlength_v) ? mlength_v : B.mlength_v;
    int Length_u = (mlength_u < B.mlength_u) ? mlength_u : B.mlength_u;
    
   for (int index_t = 0; index_t < Length_t; index_t++) {
        for(int index_s = 0; index_s < Length_s; index_s++) {
            for (int index_v = 0; index_v < Length_v; index_v++) {
                for (int index_u = 0; index_u < Length_u; index_u++) {
                    if((index_s+source_offset_s < B.mlength_s)&&(index_t+source_offset_t < B.mlength_t)&&(index_v+source_offset_v < B.mlength_v)&&(index_u+source_offset_u < B.mlength_u)) {
                        if((index_s+target_offset_s < mlength_s)&&(index_t+target_offset_t < mlength_t)&&(index_v+target_offset_v < mlength_v)&&(index_u+target_offset_u < mlength_u)) {
                            mPixel[index_t+target_offset_t][index_s+target_offset_s][index_v+target_offset_v][index_u+target_offset_u] = B.mPixel[index_t+source_offset_t][index_s+source_offset_s][index_v+source_offset_v][index_u+source_offset_u];
                        }
                    }
                }
            }
        }
    }
}

void Block4D :: CopySubblockFrom(Block4D *B, int source_offset_t, int source_offset_s, int source_offset_v, int source_offset_u, int target_offset_t, int target_offset_s, int target_offset_v, int target_offset_u) {
/*! copy data from the 4DBlock pointed by B starting from pixel position at source_offset_t, source_offset_s, source_offset_v and source_offset_u
 to the THIS 4DBlock starting from pixel position at target_offset_t, target_offset_s, target_offset_v and target_offset_u */ 
    
    int Length_t = (mlength_t < B->mlength_t) ? mlength_t : B->mlength_t;
    int Length_s = (mlength_s < B->mlength_s) ? mlength_s : B->mlength_s;
    int Length_v = (mlength_v < B->mlength_v) ? mlength_v : B->mlength_v;
    int Length_u = (mlength_u < B->mlength_u) ? mlength_u : B->mlength_u;
    
   for (int index_t = 0; index_t < Length_t; index_t++) {
        for(int index_s = 0; index_s < Length_s; index_s++) {
            for (int index_v = 0; index_v < Length_v; index_v++) {
                for (int index_u = 0; index_u < Length_u; index_u++) {
                    if((index_s+source_offset_s < B->mlength_s)&&(index_t+source_offset_t < B->mlength_t)&&(index_v+source_offset_v < B->mlength_v)&&(index_u+source_offset_u < B->mlength_u)) {
                        if((index_s+target_offset_s < mlength_s)&&(index_t+target_offset_t < mlength_t)&&(index_v+target_offset_v < mlength_v)&&(index_u+target_offset_u < mlength_u)) {
                            mPixel[index_t+target_offset_t][index_s+target_offset_s][index_v+target_offset_v][index_u+target_offset_u] = B->mPixel[index_t+source_offset_t][index_s+source_offset_s][index_v+source_offset_v][index_u+source_offset_u];
                        }
                    }
                }
            }
        }
    }
}



void Block4D :: JoinTAxis(const Block4D &B0, const Block4D &B1) {
/*! THIS block is loaded with data from B0 and B1, concatenated side by side across the t dimension*/    
    if((B0.mlength_s == B1.mlength_s)&&(B0.mlength_v == B1.mlength_v)&&(B0.mlength_u == B1.mlength_u)) {

        SetDimension(B0.mlength_t+B1.mlength_t,B0.mlength_s,B0.mlength_v,B0.mlength_u);
        for (int index_t = 0; index_t < B0.mlength_t; index_t++) {
            for(int index_s = 0; index_s < B0.mlength_s; index_s++) {
                for (int index_v = 0; index_v < B0.mlength_v; index_v++) {
                    for (int index_u = 0; index_u < B0.mlength_u; index_u++) {
                        mPixel[index_t][index_s][index_v][index_u] = B0.mPixel[index_t][index_s][index_v][index_u];
                        
                    }
                }
            }
        }
        for (int index_t = 0; index_t < B1.mlength_t; index_t++) {
            for(int index_s = 0; index_s < B1.mlength_s; index_s++) {
                for (int index_v = 0; index_v < B1.mlength_v; index_v++) {
                    for (int index_u = 0; index_u < B1.mlength_u; index_u++) {
                        mPixel[index_t+B0.mlength_t][index_s][index_v][index_u] = B1.mPixel[index_t][index_s][index_v][index_u];
                        
                    }
                }
            }
        }
    }
    else {
        printf("Error: trying to join (direction t) two 4D blocks of incompatible dimensions\n");
		throw std::runtime_error("Error: trying to join (direction v) two 4D blocks of incompatible dimensions\n");
    }
}

void Block4D :: JoinSAxis(const Block4D &B0, const Block4D &B1) {
/*! THIS block is loaded with data from B0 and B1, concatenated side by side across the s dimension*/    
    
    if((B0.mlength_t == B1.mlength_t)&&(B0.mlength_v == B1.mlength_v)&&(B0.mlength_u == B1.mlength_u)) {

        SetDimension(B0.mlength_t,B0.mlength_s+B1.mlength_s,B0.mlength_v,B0.mlength_u);
        for (int index_t = 0; index_t < B0.mlength_t; index_t++) {
            for(int index_s = 0; index_s < B0.mlength_s; index_s++) {
                 for (int index_v = 0; index_v < B0.mlength_v; index_v++) {
                    for (int index_u = 0; index_u < B0.mlength_u; index_u++) {
                        mPixel[index_t][index_s][index_v][index_u] = B0.mPixel[index_t][index_s][index_v][index_u];
                        
                    }
                }
            }
            for (int index_s = 0; index_s < B1.mlength_s; index_s++) {
                for (int index_v = 0; index_v < B1.mlength_v; index_v++) {
                    for (int index_u = 0; index_u < B1.mlength_u; index_u++) {
                        mPixel[index_t][index_s+B0.mlength_s][index_v][index_u] = B1.mPixel[index_t][index_s][index_v][index_u];
                        
                    }
                }
            }
        }
    }
    else {
        printf("Error: trying to join (direction s) two 4D blocks of incompatible dimensions\n");
		throw std::runtime_error("Error: trying to join (direction u) two 4D blocks of incompatible dimensions\n");
    }

}

void Block4D :: JoinVAxis(const Block4D &B0, const Block4D &B1) {
/*! THIS block is loaded with data from B0 and B1, concatenated side by side across the v dimension*/    
    
    if((B0.mlength_s == B1.mlength_s)&&(B0.mlength_t == B1.mlength_t)&&(B0.mlength_u == B1.mlength_u)) {

        SetDimension(B0.mlength_t,B0.mlength_s,B0.mlength_v+B1.mlength_v,B0.mlength_u);
        for (int index_t = 0; index_t < B0.mlength_t; index_t++) {
            for(int index_s = 0; index_s < B0.mlength_s; index_s++) {
                for (int index_v = 0; index_v < B0.mlength_v; index_v++) {
                    for (int index_u = 0; index_u < B0.mlength_u; index_u++) {
                        mPixel[index_t][index_s][index_v][index_u] = B0.mPixel[index_t][index_s][index_v][index_u];                       
                    }
                }
                for (int index_v = 0; index_v < B1.mlength_v; index_v++) {
                    for (int index_u = 0; index_u < B1.mlength_u; index_u++) {
                        mPixel[index_t][index_s][index_v+B0.mlength_v][index_u] = B1.mPixel[index_t][index_s][index_v][index_u];                       
                    }
                }
            }
        }
    }
    else {
        printf("Error: trying to join (direction v) two 4D blocks of incompatible dimensions\n");
		throw std::runtime_error("Error: trying to join (direction y) two 4D blocks of incompatible dimensions\n");
    }

}

void Block4D :: JoinUAxis(const Block4D &B0, const Block4D &B1) {
/*! THIS block is loaded with data from B0 and B1, concatenated side by side across the u dimension*/    
    
    if((B0.mlength_s == B1.mlength_s)&&(B0.mlength_t == B1.mlength_t)&&(B0.mlength_v == B1.mlength_v)) {

        SetDimension(B0.mlength_t,B0.mlength_s,B0.mlength_v,B0.mlength_u+B1.mlength_u);
        for (int index_t = 0; index_t < B0.mlength_t; index_t++) {
            for(int index_s = 0; index_s < B0.mlength_s; index_s++) {
                for (int index_v = 0; index_v < B0.mlength_v; index_v++) {
                    for (int index_u = 0; index_u < B0.mlength_u; index_u++) {
                        mPixel[index_t][index_s][index_v][index_u] = B0.mPixel[index_t][index_s][index_v][index_u];                       
                    }
                    for (int index_u = 0; index_u < B1.mlength_u; index_u++) {
                        mPixel[index_t][index_s][index_v][index_u+B0.mlength_u] = B1.mPixel[index_t][index_s][index_v][index_u];                       
                    }
                }
            }
        }
    }
    else {
        printf("Error: trying to join (direction u) two 4D blocks of incompatible dimensions\n");
		throw std::runtime_error("Error: trying to join (direction x) two 4D blocks of incompatible dimensions\n");
    }

}

void Block4D :: Ones(void) {
/*! THIS block is loaded unitary pixels */ 
    for(int n = 0; n < mlength_u*mlength_v*mlength_s*mlength_t; n++) {
  
        mPixelData[n] = 1.0;

    }
   
}

void Block4D :: Zeros(void) {
 /*! THIS block is loaded zero valued pixels */ 
  
    for(int n = 0; n < mlength_u*mlength_v*mlength_s*mlength_t; n++) {
  
        mPixelData[n] = 0.0;

    }
   
}

void Block4D :: Display(void) {
 /*! prints the pixel values on stdio */ 
  
    for(int index_t = 0; index_t < mlength_t; index_t++) {
        for(int index_s = 0; index_s < mlength_s; index_s++) {
            printf("view (v u) = (%d %d)\n", index_t, index_s);
            for(int index_v = 0; index_v < mlength_v; index_v++) {
                for(int index_u = 0; index_u < mlength_u; index_u++) {
		  
                    if(mPixelData[LinearPosition(index_t,index_s,index_v,index_u)] != EMPTY_VALUE) 
                        printf("%d ", mPixelData[LinearPosition(index_t,index_s,index_v,index_u)]);
                    else
                    printf("Empty ");

                }
                printf("\n");
            }
            printf("\n\n");
        }
        printf("\n\n");
    }
}

void Block4D :: Print(char *outputFileName) {
 /*! prints the pixel values on File */ 
 
    FILE *ofp;
    if((ofp=fopen(outputFileName, "w"))==NULL) {
        printf("unable to open %s file\n", outputFileName);
	exit(0);
    }
    
    for(int index_t = 0; index_t < mlength_t; index_t++) {
        for(int index_s = 0; index_s < mlength_s; index_s++) {
            fprintf(ofp, "view (v u) = (%d %d)\n", index_t, index_s);
            for(int index_v = 0; index_v < mlength_v; index_v++) {
                for(int index_u = 0; index_u < mlength_u; index_u++) {
		  
                    if(mPixelData[LinearPosition(index_t,index_s,index_v,index_u)] != EMPTY_VALUE) 
                        fprintf(ofp, "%d ", mPixelData[LinearPosition(index_t,index_s,index_v,index_u)]);
                    else
                    fprintf(ofp, "Empty ");

                }
                fprintf(ofp, "\n");
            }
            fprintf(ofp, "\n\n");
        }
        fprintf(ofp, "\n\n");
    }
    fclose(ofp);
}

void Block4D :: DCT_U(int scale) {
/*! performs the one dimensional DCT transform on the u dimension */ 
    
    double *temp = new double [mlength_u];
    
    for ( int index_t = 0; index_t < mlength_t; index_t++ ) {
        
	for ( int index_s = 0; index_s < mlength_s; index_s++ ) {
            
            for ( int index_v = 0; index_v < mlength_v; index_v++ ) {
            
                for ( int index_u = 0; index_u < mlength_u; index_u++ ) {

                    temp[index_u] = 0;
                    for ( int summation_index_u = 0; summation_index_u < mlength_u; summation_index_u++ ) {
             
			double angle = PI * ( double ) ( index_u * ( 2 * summation_index_u + 1 ) ) / ( double ) ( 2 * mlength_u );
			temp[index_u] +=  scale * cos ( angle ) * mPixel[index_t][index_s][index_v][summation_index_u];
                        
                    }
                    temp[index_u] *= sqrt ( 2.0 );
                         
                }
                temp[0] /= sqrt(2.0);
                for ( int index_u = 0; index_u < mlength_u; index_u++ ) {
                    mPixel[index_t][index_s][index_v][index_u] = temp[index_u];
                }
            }
	}
		
    }

    delete [] temp;
     
}

void Block4D :: IDCT_U(int scale) {
/*! performs the one dimensional inverse DCT transform on the u dimension */ 
    
    double *temp = new double [mlength_u];
    
   for ( int index_t = 0; index_t < mlength_t; index_t++ ) {
        
	for ( int index_s = 0; index_s < mlength_s; index_s++ ) {
            
            for ( int index_v = 0; index_v < mlength_v; index_v++ ) {
            
                for ( int index_u = 0; index_u < mlength_u; index_u++ ) {

                    temp[index_u] =  mPixel[index_t][index_s][index_v][0] / sqrt( 2.0 );
                    for ( int summation_index_u = 1; summation_index_u < mlength_u; summation_index_u++ ) {
             
			double angle = PI * ( double ) ( (2 * index_u + 1) * summation_index_u ) / ( double ) ( 2 * mlength_u );
			temp[index_u] +=  cos ( angle ) * mPixel[index_t][index_s][index_v][summation_index_u];
                        
                    }
                    temp[index_u] *= sqrt( 2.0 )/( ( double ) ( mlength_u ));
                        
                }
                
                for ( int index_u = 0; index_u < mlength_u; index_u++ ) {
                    mPixel[index_t][index_s][index_v][index_u] = (temp[index_u] + 0.5)/scale;
                }
            }
	}
		
    }

    delete [] temp;
    
}

void Block4D :: DCT_V(int scale) {
/*! performs the one dimensional DCT transform on the v dimension */ 
    
    double *temp = new double [mlength_v];
    
    for ( int index_t = 0; index_t < mlength_t; index_t++ ) {
        
	for ( int index_s = 0; index_s < mlength_s; index_s++ ) {
            
            for ( int index_u = 0; index_u < mlength_u; index_u++ ) {
                
                for ( int index_v = 0; index_v < mlength_v; index_v++ ) {
            
                    temp[index_v] = 0;
                    for ( int summation_index_v = 0; summation_index_v < mlength_v; summation_index_v++ ) {
             
			double angle = PI * ( double ) ( index_v * ( 2 * summation_index_v + 1 ) ) / ( double ) ( 2 * mlength_v );
			temp[index_v] +=  scale * cos ( angle ) * mPixel[index_t][index_s][summation_index_v][index_u];
                        
                    }
                    temp[index_v] *= sqrt ( 2.0 );
                         
                }
                temp[0] /= sqrt(2.0);
                for ( int index_v = 0; index_v < mlength_v; index_v++ ) {
                    mPixel[index_t][index_s][index_v][index_u] = temp[index_v];
                }
            }
	}
		
    }

    delete [] temp;
     
}

void Block4D :: IDCT_V(int scale) {
/*! performs the one dimensional inverse DCT transform on the u dimension */ 
    
    double *temp = new double [mlength_v];
    
    for ( int index_t = 0; index_t < mlength_t; index_t++ ) {
        
	for ( int index_s = 0; index_s < mlength_s; index_s++ ) {
            
            for ( int index_u = 0; index_u < mlength_u; index_u++ ) {
                
                for ( int index_v = 0; index_v < mlength_v; index_v++ ) {
            
                    temp[index_v] =  mPixel[index_t][index_s][0][index_u] / sqrt( 2.0 );
                    for ( int summation_index_v = 1; summation_index_v < mlength_v; summation_index_v++ ) {
             
			double angle = PI * ( double ) ( (2 * index_v + 1) * summation_index_v ) / ( double ) ( 2 * mlength_v );
			temp[index_v] +=  cos ( angle ) * mPixel[index_t][index_s][summation_index_v][index_u];
                        
                    }
                    temp[index_v] *= sqrt( 2.0 )/( ( double ) ( mlength_v ));
                        
                }
                
                for ( int index_v = 0; index_v < mlength_v; index_v++ ) {
                    mPixel[index_t][index_s][index_v][index_u] = (temp[index_v] + 0.5)/scale;
                }
            }
	}
		
    }

    delete [] temp;
    
}

void Block4D :: DCT_S(int scale) {
/*! performs the one dimensional DCT transform on the s dimension */ 
    
    double *temp = new double [mlength_s];
    
    for ( int index_t = 0; index_t < mlength_t; index_t++ ) {
        
        for ( int index_v = 0; index_v < mlength_v; index_v++ ) {
            
            for ( int index_u = 0; index_u < mlength_u; index_u++ ) {

                for ( int index_s = 0; index_s < mlength_s; index_s++ ) {
            
                    temp[index_s] = 0;
                    for ( int summation_index_s = 0; summation_index_s < mlength_s; summation_index_s++ ) {
             
			double angle = PI * ( double ) ( index_s * ( 2 * summation_index_s + 1 ) ) / ( double ) ( 2 * mlength_s );
			temp[index_s] +=  scale * cos ( angle ) * mPixel[index_t][summation_index_s][index_v][index_u];
                        
                    }
                    temp[index_s] *= sqrt ( 2.0 );
                         
                }
                temp[0] /= sqrt(2.0);
                for ( int index_s = 0; index_s < mlength_s; index_s++ ) {
                    mPixel[index_t][index_s][index_v][index_u] = temp[index_s];
                }
            }
	}
		
    }

    delete [] temp;
     
}

void Block4D :: IDCT_S(int scale) {
/*! performs the one dimensional inverse DCT transform on the s dimension */ 
    
    double *temp = new double [mlength_s];
    
    for ( int index_t = 0; index_t < mlength_t; index_t++ ) {
        
        for ( int index_v = 0; index_v < mlength_v; index_v++ ) {
            
            for ( int index_u = 0; index_u < mlength_u; index_u++ ) {

                for ( int index_s = 0; index_s < mlength_s; index_s++ ) {
            
                    temp[index_s] =  mPixel[index_t][0][index_v][index_u] / sqrt( 2.0 );
                    for ( int summation_index_s = 1; summation_index_s < mlength_s; summation_index_s++ ) {
             
			double angle = PI * ( double ) ( (2 * index_s + 1) * summation_index_s ) / ( double ) ( 2 * mlength_s );
			temp[index_s] +=  cos ( angle ) * mPixel[index_t][summation_index_s][index_v][index_u];
                        
                    }
                    temp[index_s] *= sqrt( 2.0 )/( ( double ) ( mlength_s ));
                        
                }
                
                for ( int index_s = 0; index_s < mlength_s; index_s++ ) {
                    mPixel[index_t][index_s][index_v][index_u] = (temp[index_s] + 0.5)/scale;
                }
            }
	}
		
    }

    delete [] temp;
    
}

void Block4D :: DCT_T(int scale) {
/*! performs the one dimensional DCT transform on the t dimension */ 
    
    double *temp = new double [mlength_t];
    
    for ( int index_s = 0; index_s < mlength_s; index_s++ ) {
            
        for ( int index_v = 0; index_v < mlength_v; index_v++ ) {
            
            for ( int index_u = 0; index_u < mlength_u; index_u++ ) {

                for ( int index_t = 0; index_t < mlength_t; index_t++ ) {
        
                    temp[index_t] = 0;
                    for ( int summation_index_t = 0; summation_index_t < mlength_t; summation_index_t++ ) {
             
			double angle = PI * ( double ) ( index_t * ( 2 * summation_index_t + 1 ) ) / ( double ) ( 2 * mlength_t );
			temp[index_t] +=  scale * cos ( angle ) * mPixel[summation_index_t][index_s][index_v][index_u];
                        
                    }
                    temp[index_t] *= sqrt ( 2.0 );
                         
                }
                temp[0] /= sqrt(2.0);
                for ( int index_t = 0; index_t < mlength_t; index_t++ ) {
                    mPixel[index_t][index_s][index_v][index_u] = temp[index_t];
                }
            }
	}
		
    }

    delete [] temp;
     
}

void Block4D :: IDCT_T(int scale) {
/*! performs the one dimensional inverse DCT transform on the t dimension */ 
    
    double *temp = new double [mlength_t];
    
    for ( int index_s = 0; index_s < mlength_s; index_s++ ) {
            
        for ( int index_v = 0; index_v < mlength_v; index_v++ ) {
            
            for ( int index_u = 0; index_u < mlength_u; index_u++ ) {

                for ( int index_t = 0; index_t < mlength_t; index_t++ ) {
        
                    temp[index_t] =  mPixel[0][index_s][index_v][index_u] / sqrt( 2.0 );
                    for ( int summation_index_t = 1; summation_index_t < mlength_t; summation_index_t++ ) {
             
			double angle = PI * ( double ) ( (2 * index_t + 1) * summation_index_t ) / ( double ) ( 2 * mlength_t );
			temp[index_t] +=  cos ( angle ) * mPixel[summation_index_t][index_s][index_v][index_u];
                        
                    }
                    temp[index_t] *= sqrt( 2.0 )/( ( double ) ( mlength_t ));
                        
                }
                
                for ( int index_t = 0; index_t < mlength_t; index_t++ ) {
                    mPixel[index_t][index_s][index_v][index_u] = (temp[index_t] + 0.5)/scale;
                }
            }
	}
		
    }

    delete [] temp;
    
}

void Block4D :: TRANSFORM_U(double scale, double *coefficients) {
/*! performs a one dimensional generic transform defined by the values pointed by *coefficients on the u dimension */ 
    
    double *temp = new double [mlength_u];
    
    for ( int index_t = 0; index_t < mlength_t; index_t++ ) {
        
	for ( int index_s = 0; index_s < mlength_s; index_s++ ) {
            
            for ( int index_v = 0; index_v < mlength_v; index_v++ ) {
            
                for ( int index_u = 0; index_u < mlength_u; index_u++ ) {

                    temp[index_u] = 0;
                    for ( int summation_index_u = 0; summation_index_u < mlength_u; summation_index_u++ ) {
             
			temp[index_u] +=  scale * mPixel[index_t][index_s][index_v][summation_index_u] * coefficients[index_u*mlength_u + summation_index_u];
                        
                    }
                         
                }
                for ( int index_u = 0; index_u < mlength_u; index_u++ ) {
                    mPixel[index_t][index_s][index_v][index_u] = temp[index_u];
                    if(temp[index_u] > 0)
                        mPixel[index_t][index_s][index_v][index_u] = temp[index_u]+0.5;
                    if(temp[index_u] < 0)
                        mPixel[index_t][index_s][index_v][index_u] = temp[index_u]-0.5;
                }
            }
	}
		
    }

    delete [] temp;
     
}

void Block4D :: TRANSFORM_V(double scale, double *coefficients) {
/*! performs a one dimensional generic transform defined by the values pointed by *coefficients on the v dimension */ 
    
    double *temp = new double [mlength_v];
    
    for ( int index_t = 0; index_t < mlength_t; index_t++ ) {
        
	for ( int index_s = 0; index_s < mlength_s; index_s++ ) {
            
            for ( int index_u = 0; index_u < mlength_u; index_u++ ) {
                
                for ( int index_v = 0; index_v < mlength_v; index_v++ ) {
            
                    temp[index_v] = 0;
                    for ( int summation_index_v = 0; summation_index_v < mlength_v; summation_index_v++ ) {
             
			temp[index_v] +=  scale * mPixel[index_t][index_s][summation_index_v][index_u] * coefficients[index_v*mlength_v + summation_index_v];
                        
                    }
                        
                }
                for ( int index_v = 0; index_v < mlength_v; index_v++ ) {
                    mPixel[index_t][index_s][index_v][index_u] = temp[index_v];
                    if(temp[index_v] > 0)
                        mPixel[index_t][index_s][index_v][index_u] = temp[index_v]+0.5;
                    if(temp[index_v] < 0)
                        mPixel[index_t][index_s][index_v][index_u] = temp[index_v]-0.5;
                }
            }
	}
		
    }

    delete [] temp;
     
}

void Block4D :: TRANSFORM_S(double scale, double *coefficients) {
/*! performs a one dimensional generic transform defined by the values pointed by *coefficients on the s dimension */ 
    
    double *temp = new double [mlength_s];
    
    for ( int index_t = 0; index_t < mlength_t; index_t++ ) {
        
        for ( int index_v = 0; index_v < mlength_v; index_v++ ) {
            
            for ( int index_u = 0; index_u < mlength_u; index_u++ ) {

                for ( int index_s = 0; index_s < mlength_s; index_s++ ) {
            
                    temp[index_s] = 0;
                    for ( int summation_index_s = 0; summation_index_s < mlength_s; summation_index_s++ ) {
             
			temp[index_s] +=  scale * mPixel[index_t][summation_index_s][index_v][index_u] * coefficients[index_s*mlength_s + summation_index_s];
                        
                    }
                         
                }
                for ( int index_s = 0; index_s < mlength_s; index_s++ ) {
                    mPixel[index_t][index_s][index_v][index_u] = temp[index_s];
                    if(temp[index_s] > 0)
                        mPixel[index_t][index_s][index_v][index_u] = temp[index_s]+0.5;
                    if(temp[index_s] < 0)
                        mPixel[index_t][index_s][index_v][index_u] = temp[index_s]-0.5;
                }
            }
	}
		
    }

    delete [] temp;
     
}

void Block4D :: TRANSFORM_T(double scale, double *coefficients) {
/*! performs a one dimensional generic transform defined by the values pointed by *coefficients on the t dimension */ 
    
    double *temp = new double [mlength_t];
    
    for ( int index_s = 0; index_s < mlength_s; index_s++ ) {
            
        for ( int index_v = 0; index_v < mlength_v; index_v++ ) {
            
            for ( int index_u = 0; index_u < mlength_u; index_u++ ) {

                for ( int index_t = 0; index_t < mlength_t; index_t++ ) {
        
                    temp[index_t] = 0;
                    for ( int summation_index_t = 0; summation_index_t < mlength_t; summation_index_t++ ) {
             
			temp[index_t] +=  scale * mPixel[summation_index_t][index_s][index_v][index_u] * coefficients[index_t*mlength_t + summation_index_t];
                        
                    }
                         
                }
                for ( int index_t = 0; index_t < mlength_t; index_t++ ) {
                    mPixel[index_t][index_s][index_v][index_u] = temp[index_t];
                    if(temp[index_t] > 0)
                        mPixel[index_t][index_s][index_v][index_u] = temp[index_t]+0.5;
                    if(temp[index_t] < 0)
                        mPixel[index_t][index_s][index_v][index_u] = temp[index_t]-0.5;
                }
            }
	}
		
    }

    delete [] temp;
     
}

double Block4D :: L2Norm(void) {
/*! evaluates the sum of squared pixel values of this block */ 
    
    double l2sum=0;
    long int size = mlength_u;
    size *= mlength_v;
    size *= mlength_s;
    size *= mlength_t;
    
    for(int n = 0; n < size; n++) {
        
        double doublePixel = mPixelData[n];
        l2sum += doublePixel*doublePixel;
        
    }
    return(l2sum);
}

block4DElementType Block4D :: Max(void) {
/*! evaluates the maximum pixel value of this block */ 
    
    long int size = mlength_u;
    size *= mlength_v;
    size *= mlength_s;
    size *= mlength_t;
 
    block4DElementType maxVal = mPixelData[0];
    
    for(int n = 1; n < size; n++) {
        
        if(mPixelData[n] > maxVal)
           maxVal = mPixelData[n];
        
    }
    return(maxVal);
}
block4DElementType Block4D :: Min(void) {
/*! evaluates the minimum pixel value of this block */ 
    
    long int size = mlength_u;
    size *= mlength_v;
    size *= mlength_s;
    size *= mlength_t;
 
    block4DElementType minVal = mPixelData[0];
    
    for(int n = 1; n < size; n++) {
        
        if(mPixelData[n] < minVal)
           minVal = mPixelData[n];
        
    }
    return(minVal);
}
/*
double Block4D :: Entropy(int number_of_bins, int maxBitplane) {
/*! evaluates the entropy of pixel values of this block */ 
    
/*
    long int size = mlength_u;
    size *= mlength_v;
    size *= mlength_s;
    size *= mlength_t;
    
    int *bins = new int [number_of_bins]; 
    for (int n = 0; n < number_of_bins; n++)
         bins[n] = 0;
    
    long int range = 1<<maxBitplane;
    int quantizerStep = 2*range/number_of_bins;
    //int minimumPixelValue = Min();
    //long int range = Max() - minimumPixelValue;
    //int quantizerStep = range/number_of_bins;
    
    for(int n = 0; n < size; n++) {
        
        bins[(mPixelData[n]+range)/quantizerStep]++;
        //bins[(mPixelData[n]-minimumPixelValue)/quantizerStep]++;
        
    }
    double entropy = 0.0;
    for (int n = 0; n < number_of_bins; n++) {
         if(bins[n] != 0) {
	     double probability = (1.0*bins[n])/size;
             entropy -= probability*log2(probability);
         }
    }
    delete [] bins;
    return(entropy);
}
*/
double Block4D :: Entropy(int minBitplane) {
/*! evaluates the entropy of pixel values of this block */ 
    

    long int size = mlength_u;
    size *= mlength_v;
    size *= mlength_s;
    size *= mlength_t;
    
    int number_of_bins = 1<<(31-minBitplane);
    int *bins = new int [number_of_bins]; 
    for (int n = 0; n < number_of_bins; n++)
         bins[n] = 0;
    
    int range = number_of_bins>>1;
    for(int n = 0; n < size; n++) {
        
        bins[(mPixelData[n]>>minBitplane)+range]++;
        
    }
    double entropy = 0.0;
    for (int n = 0; n < number_of_bins; n++) {
         if(bins[n] != 0) {
	     double probability = (1.0*bins[n])/size;
             entropy -= probability*log2(probability);
         }
    }
    delete [] bins;
    return(entropy);
}

void Block4D :: Extend_U(int position_u) {
/*! extend this block by repeating the pixel at position_u in the u dimension from that position to the end of the block*/
    
    if(position_u >= mlength_u - 1) {
        return;
    }
    for ( int index_t = 0; index_t < mlength_t; index_t++ ) {
        
	for ( int index_s = 0; index_s < mlength_s; index_s++ ) {
            
            for ( int index_v = 0; index_v < mlength_v; index_v++ ) {
            
                for ( int index_u = position_u+1; index_u < mlength_u; index_u++ ) {

                    mPixel[index_t][index_s][index_v][index_u] = mPixel[index_t][index_s][index_v][position_u];
                    
                }
            }
	}
		
    }
}

void Block4D :: Extend_V(int position_v) {
/*! extend this block by repeating the pixel at position_v in the v dimension from that position to the end of the block*/
    
    if(position_v >= mlength_v - 1) {
        return;
    }
    for ( int index_t = 0; index_t < mlength_t; index_t++ ) {
        
	for ( int index_s = 0; index_s < mlength_s; index_s++ ) {
            
            for ( int index_v = position_v+1; index_v < mlength_v; index_v++ ) {
            
                for ( int index_u = 0; index_u < mlength_u; index_u++ ) {

                    mPixel[index_t][index_s][index_v][index_u] = mPixel[index_t][index_s][position_v][index_u];
                    
                }
            }
	}
		
    }
}

void Block4D :: Extend_S(int position_s) {
/*! extend this block by repeating the pixel at position_s in the s dimension from that position to the end of the block*/
    
    if(position_s >= mlength_s - 1) {
        return;
    }
    for ( int index_t = 0; index_t < mlength_t; index_t++ ) {
        
	for ( int index_s = position_s+1; index_s < mlength_s; index_s++ ) {
            
            for ( int index_v = 0; index_v < mlength_v; index_v++ ) {
            
                for ( int index_u = 0; index_u < mlength_u; index_u++ ) {

                    mPixel[index_t][index_s][index_v][index_u] = mPixel[index_t][position_s][index_v][index_u];
                    
                }
            }
	}
		
    }
}

void Block4D :: Extend_T(int position_t) {
/*! extend this block by repeating the pixel at position_t in the t dimension from that position to the end of the block*/
    
    if(position_t >= mlength_t - 1) {
        return;
    }
    for ( int index_t = position_t+1; index_t < mlength_t; index_t++ ) {
        
	for ( int index_s = 0; index_s < mlength_s; index_s++ ) {
            
            for ( int index_v = 0; index_v < mlength_v; index_v++ ) {
            
                for ( int index_u = 0; index_u < mlength_u; index_u++ ) {

                    mPixel[index_t][index_s][index_v][index_u] = mPixel[position_t][index_s][index_v][index_u];
                    
                }
            }
	}
		
    }
}

void Block4D :: Clip(int minValue, int maxValue) {
/*! restrict pixels values to the interval [minValue, maxValue] */
    
    for(int n = 0; n < mlength_t*mlength_s*mlength_v*mlength_u; n++) {
        mPixelData[n] = (mPixelData[n] < minValue) ? minValue : mPixelData[n];
        mPixelData[n] = (mPixelData[n] > maxValue) ? maxValue : mPixelData[n];
    }
}

void Block4D :: Threshold(int minMagnitude, int maxMagnitude) {
/*! if the magnitude of the pixel data is below minMagnitude it is adjusted to equal minMagnitude
 if the magnitude of the pixel is above maxMagnitude it is set to maxMagnitude. the signal is preserved*/
    
    for(int n = 0; n < mlength_t*mlength_s*mlength_v*mlength_u; n++) {
        int magnitude = mPixelData[n];
        int signal = (mPixelData[n] > 0) ? 1 : -1;
        magnitude = signal * magnitude;
        magnitude = (magnitude < minMagnitude) ? 0 : magnitude;
        magnitude = (magnitude > maxMagnitude) ? (1<<(8*sizeof(block4DElementType)-1))-1 : magnitude;
        mPixelData[n] = signal * magnitude;
    }
}

void Block4D :: Shift_UVPlane(int shift, int position_t, int position_s) {
/*! The pixel values of the UV plane at st coordinates (position_t, position_s) are left shifted by shift bits if shift is positive,
 and right shifted by -shift if shift is negative */  
    
    for ( int index_v = 0; index_v < mlength_v; index_v++ ) {
            
        for ( int index_u = 0; index_u < mlength_u; index_u++ ) {

            if(shift > 0) 
                mPixel[position_t][position_s][index_v][index_u] = mPixel[position_t][position_s][index_v][index_u] << shift;
            if(shift < 0)
                mPixel[position_t][position_s][index_v][index_u] = mPixel[position_t][position_s][index_v][index_u] >> -shift;
                    
        }
    }

}

void Block4D :: SlantAndPadd(int dPv, int dPu, const Block4D &sourceBlock) {
  
    if(sourceBlock.mlength_t ==1)
        dPv = 0;
    if(sourceBlock.mlength_s ==1)
        dPu = 0;
    
    SetDimension(sourceBlock.mlength_t,sourceBlock.mlength_s,sourceBlock.mlength_v+abs(dPv),sourceBlock.mlength_u+abs(dPu));
    Zeros();
    
    for(int index_v = 0; index_v < mlength_v; index_v++) {

        for(int index_u = 0; index_u < mlength_u; index_u++) {
      
	 
            for(int index_t = 0; index_t < mlength_t; index_t++) {
        
                int position_v = index_v;
                if(mlength_t > 1) {
                    position_v += round((dPv*index_t)/(mlength_t-1));
                    if(dPv > 0)
                        position_v += -round(dPv); 
                }
                
                //v padding 
                int position_t = index_t;    
		    
		if(dPv < 0) {
		      
                    if(position_v < 0) {
			  
                        while(position_v < 0) {
                            position_t--;
                            position_v = index_v + round((dPv*position_t)/(mlength_t-1));
                        }
		
                    }
                    if(position_v >= sourceBlock.mlength_v) {
			  
                        while(position_v  >= sourceBlock.mlength_v) {
                            position_t++;
                            position_v = index_v + round((dPv*position_t)/(mlength_t-1));
                        }
                        
                    }
                }
                else  {
			  
                    if(position_v < 0) {
			  
                        while(position_v < 0) {
                            position_t++;
                            position_v = index_v -dPv + round((dPv*position_t)/(mlength_t-1));
                        }
                        
                    }      
                    if(position_v >= sourceBlock.mlength_v) {
			  
                        while(position_v >= sourceBlock.mlength_v ) {
                            position_t--;
                            position_v = index_v -dPv + round((dPv*position_t)/(mlength_t-1));
                        }
                        
                    }      
		      
                }
                
	        for(int index_s = 0; index_s < mlength_s; index_s++) {

                    int position_u = index_u;
                    if(mlength_s > 1) {
                        position_u += round((dPu*index_s)/(mlength_s-1));
                        if(dPu > 0)
                            position_u += -round(dPu); 
                    }
                                        
                    
                       
                    // u padding
                    int position_s = index_s;  
		    
		    if(dPu < 0) {
		      
                        if(position_u < 0) {
			  
                            while(position_u < 0) {
                                position_s--;
                                position_u = index_u + round((dPu*position_s)/(mlength_s-1));
                            }
                       
                        }
                        if(position_u >= sourceBlock.mlength_u) {
			  
                            while(position_u >= sourceBlock.mlength_u) {
                                position_s++;
                                position_u = index_u + round((dPu*position_s)/(mlength_s-1));
                            }
                       
                        }
                    }
                    else  {
			  
                        if(position_u < 0) {
			  
                            while(position_u < 0) {
                                position_s++;
                                position_u = index_u -dPu + round((dPu*position_s)/(mlength_s-1));
                            }
                       
                        }      
                        if(position_u >= sourceBlock.mlength_u) {
			  
                            while(position_u >= sourceBlock.mlength_u) {
                                position_s--;
                                position_u = index_u -dPu + round((dPu*position_s)/(mlength_s-1));
                            }
                        
                       }      
		      
                    }

                    
                    mPixel[index_t][index_s][index_v][index_u] = sourceBlock.mPixel[position_t][position_s][position_v][position_u];                    
                   
                }
            }

        }
           
    }
  
}

void Block4D :: DeSlantMeanPadd(int dPv, int dPu, const Block4D &sourceBlock) {
  
    if(sourceBlock.mlength_t ==1)
        dPv = 0;
    if(sourceBlock.mlength_s ==1)
        dPu = 0;
      
    Block4D countBlock;

    countBlock.SetDimension(mlength_t,mlength_s,mlength_v,mlength_u);
    
    countBlock.Zeros();
    
    Zeros();

    for(int index_v = 0; index_v < sourceBlock.mlength_v; index_v++) {

        for(int index_u = 0; index_u < sourceBlock.mlength_u; index_u++) {
      
	 
            for(int index_t = 0; index_t < mlength_t; index_t++) {
        
                int position_v = index_v;
                if(mlength_t > 1) {
                    position_v += round((dPv*index_t)/(mlength_t-1));
                    if(dPv > 0)
                        position_v += -round(dPv); 
                }
                
                //v padding 
                int position_t = index_t;    
		    
		if(dPv < 0) {
		      
                    if(position_v < 0) {
			  
                        while(position_v < 0) {
                            position_t--;
                            position_v = index_v + round((dPv*position_t)/(mlength_t-1));
                        }
		
                    }
                    if(position_v >= mlength_v) {
			  
                        while(position_v  >= mlength_v) {
                            position_t++;
                            position_v = index_v + round((dPv*position_t)/(mlength_t-1));
                        }
                        
                    }
                }
                else  {
			  
                    if(position_v < 0) {
			  
                        while(position_v < 0) {
                            position_t++;
                            position_v = index_v -dPv + round((dPv*position_t)/(mlength_t-1));
                        }
                        
                    }      
                    if(position_v >= mlength_v) {
			  
                        while(position_v >= mlength_v ) {
                            position_t--;
                            position_v = index_v -dPv + round((dPv*position_t)/(mlength_t-1));
                        }
                        
                    }      
		      
                }
                
	        for(int index_s = 0; index_s < mlength_s; index_s++) {

                    int position_u = index_u;
                    if(mlength_s > 1) {
                        position_u += round((dPu*index_s)/(mlength_s-1));
                        if(dPu > 0)
                            position_u += -round(dPu); 
                    }
                   
                     // u padding
                    int position_s = index_s;  
		    
		    if(dPu < 0) {
		      
                        if(position_u < 0) {
			  
                            while(position_u < 0) {
                                position_s--;
                                position_u = index_u + round((dPu*position_s)/(mlength_s-1));
                            }
                       
                        }
                        if(position_u >= mlength_u) {
			  
                            while(position_u >= mlength_u) {
                                position_s++;
                                position_u = index_u + round((dPu*position_s)/(mlength_s-1));
                            }
                       
                        }
                    }
                    else  {
			  
                        if(position_u < 0) {
			  
                            while(position_u < 0) {
                                position_s++;
                                position_u = index_u -dPu + round((dPu*position_s)/(mlength_s-1));
                            }
                       
                        }      
                        if(position_u >= mlength_u) {
			  
                            while(position_u >= mlength_u) {
                                position_s--;
                                position_u = index_u -dPu + round((dPu*position_s)/(mlength_s-1));
                            }
                        
                       }      
		      
                    }
                    
                    mPixel[position_t][position_s][position_v][position_u] += sourceBlock.mPixel[index_t][index_s][index_v][index_u];                    
		    countBlock.mPixel[position_t][position_s][position_v][position_u]++;
		    
		    
                }
            }

        }
           
    }
    
    long int numberOfPixels = mlength_t*mlength_s;
    numberOfPixels *= mlength_v*mlength_u;
    
    for(long int pixelIndex = 0; pixelIndex < numberOfPixels; pixelIndex++)
        mPixelData[pixelIndex] /= countBlock.mPixelData[pixelIndex];

    countBlock.SetDimension(0,0,0,0);
}


void Block4D :: Slant(int dPv, int dPu, const Block4D &sourceBlock) {
  
    if(sourceBlock.mlength_t ==1)
        dPv = 0;
    if(sourceBlock.mlength_s ==1)
        dPu = 0;
    
    SetDimension(sourceBlock.mlength_t,sourceBlock.mlength_s,sourceBlock.mlength_v+abs(dPv),sourceBlock.mlength_u+abs(dPu));
    Erase();
    
    for(int index_v = 0; index_v < mlength_v; index_v++) {

        for(int index_u = 0; index_u < mlength_u; index_u++) {
      
	 
            for(int index_t = 0; index_t < mlength_t; index_t++) {
        
                int position_v = index_v;
                if(mlength_t > 1) {
                    position_v += round((dPv*index_t)/(mlength_t-1));
                    if(dPv > 0)
                        position_v += -round(dPv); 
                }
                
                
	        for(int index_s = 0; index_s < mlength_s; index_s++) {

                    int position_u = index_u;
                    if(mlength_s > 1) {
                        position_u += round((dPu*index_s)/(mlength_s-1));
                        if(dPu > 0)
                            position_u += -round(dPu); 
                    }
                                        
                    
 	            if((position_v >= 0)&&(position_v < sourceBlock.mlength_v)&&(position_u >= 0)&&(position_u < sourceBlock.mlength_u))
                        mPixel[index_t][index_s][index_v][index_u] = sourceBlock.mPixel[index_t][index_s][position_v][position_u];                    
                                                             
                }
            }

        }
           
    }
  
}

void Block4D :: DeSlant(int dPv, int dPu, const Block4D &sourceBlock) {
  
    if(sourceBlock.mlength_t ==1)
        dPv = 0;
    if(sourceBlock.mlength_s ==1)
        dPu = 0;
      
    
    //Zeros();
    SetDimension(sourceBlock.mlength_t,sourceBlock.mlength_s,sourceBlock.mlength_v-abs(dPv),sourceBlock.mlength_u-abs(dPu));
    Erase();

    for(int index_v = 0; index_v < sourceBlock.mlength_v; index_v++) {

        for(int index_u = 0; index_u < sourceBlock.mlength_u; index_u++) {
      
	 
            for(int index_t = 0; index_t < mlength_t; index_t++) {
        
                int position_v = index_v;
                if(mlength_t > 1) {
                    position_v += round((dPv*index_t)/(mlength_t-1));
                    if(dPv > 0)
                        position_v += -round(dPv); 
                }
                
                
	        for(int index_s = 0; index_s < mlength_s; index_s++) {

                    int position_u = index_u;
                    if(mlength_s > 1) {
                        position_u += round((dPu*index_s)/(mlength_s-1));
                        if(dPu > 0)
                            position_u += -round(dPu); 
                    }
                   
                    
 	            if((position_v >= 0)&&(position_v < mlength_v)&&(position_u >= 0)&&(position_u < mlength_u))
                        mPixel[index_t][index_s][position_v][position_u] = sourceBlock.mPixel[index_t][index_s][index_v][index_u];                    
		    
		    
                }
            }

        }
           
    }
    
}
  
void Block4D :: Padding(int dPv, int dPu, const Block4D &sourceBlock) {
  
    if(sourceBlock.mlength_t ==1)
        dPv = 0;
    if(sourceBlock.mlength_s ==1)
        dPu = 0;
        
    for(int index_v = 0; index_v < mlength_v; index_v++) {

        for(int index_u = 0; index_u < mlength_u; index_u++) {
      
	 
            for(int index_t = 0; index_t < mlength_t; index_t++) {
        
                int position_v = index_v;
                if(mlength_t > 1) {
                    position_v += round((dPv*index_t)/(mlength_t-1));
                    if(dPv > 0)
                        position_v += -round(dPv); 
                }
                
                //v padding 
                int position_t = index_t;    
		    
                int doPadding_v = 0;

		if(dPv < 0) {
		      
                    if(position_v < 0) {
			  
                        while(position_v < 0) {
                            position_t--;
                            position_v = index_v + round((dPv*position_t)/(mlength_t-1));
                        }
                        doPadding_v = 1;
		
                    }
                    if(position_v >= sourceBlock.mlength_v) {
			  
                        while(position_v  >= sourceBlock.mlength_v) {
                            position_t++;
                            position_v = index_v + round((dPv*position_t)/(mlength_t-1));
                        }
                        doPadding_v = 1;
                        
                    }
                }
                else  {
			  
                    if(position_v < 0) {
			  
                        while(position_v < 0) {
                            position_t++;
                            position_v = index_v -dPv + round((dPv*position_t)/(mlength_t-1));
                        }
                        doPadding_v = 1;
                        
                    }      
                    if(position_v >= sourceBlock.mlength_v) {
			  
                        while(position_v >= sourceBlock.mlength_v ) {
                            position_t--;
                            position_v = index_v -dPv + round((dPv*position_t)/(mlength_t-1));
                        }
                        doPadding_v = 1;
                        
                    }      
		      
                }
                
	        for(int index_s = 0; index_s < mlength_s; index_s++) {

                    int position_u = index_u;
                    if(mlength_s > 1) {
                        position_u += round((dPu*index_s)/(mlength_s-1));
                        if(dPu > 0)
                            position_u += -round(dPu); 
                    }
                                        
                    
                       
                    // u padding
                    int position_s = index_s;  
		    
                    int doPadding_u = 0;

		    if(dPu < 0) {
		      
                        if(position_u < 0) {
			  
                            while(position_u < 0) {
                                position_s--;
                                position_u = index_u + round((dPu*position_s)/(mlength_s-1));
                            }
                            doPadding_u = 1;
                        }
                        if(position_u >= sourceBlock.mlength_u) {
			  
                            while(position_u >= sourceBlock.mlength_u) {
                                position_s++;
                                position_u = index_u + round((dPu*position_s)/(mlength_s-1));
                            }
                            doPadding_u = 1;
                       
                        }
                    }
                    else  {
			  
                        if(position_u < 0) {
			  
                            while(position_u < 0) {
                                position_s++;
                                position_u = index_u -dPu + round((dPu*position_s)/(mlength_s-1));
                            }
                            doPadding_u = 1;
                       
                        }      
                        if(position_u >= sourceBlock.mlength_u) {
			  
                            while(position_u >= sourceBlock.mlength_u) {
                                position_s--;
                                position_u = index_u -dPu + round((dPu*position_s)/(mlength_s-1));
                            }
                            doPadding_u = 1;
                        
                       }      
		      
                    }

                    if((doPadding_v == 1)||(doPadding_u == 1))
                        mPixel[index_t][index_s][index_v][index_u] = sourceBlock.mPixel[position_t][position_s][position_v][position_u]; 
                   
                }
            }

        }
           
    }
  
}
    

void Block4D :: Erase(void) {
  
    long int numberOfPixels = mlength_t*mlength_s;
    numberOfPixels *= mlength_v*mlength_u;
    
    block4DElementType empty=EMPTY_VALUE;
    
    for(long int pixelIndex = 0; pixelIndex < numberOfPixels; pixelIndex++)
        mPixelData[pixelIndex] = empty;
}

void Block4D :: Trim(const Block4D &sourceBlock) {
  
   //find boudaries coordinates
   int max_u_coord = 0;
   int min_u_coord = sourceBlock.mlength_u-1;
   int max_v_coord = 0;
   int min_v_coord = sourceBlock.mlength_v-1;
   int max_t_coord = 0;
   int min_t_coord = sourceBlock.mlength_t-1;
   int max_s_coord = 0;
   int min_s_coord = sourceBlock.mlength_s-1;
   
   for(int index_v = 0; index_v < sourceBlock.mlength_v; index_v++) {
       for(int index_u = 0; index_u < sourceBlock.mlength_u; index_u++) {
           for(int index_t = 0; index_t < sourceBlock.mlength_t; index_t++) {
               for(int index_s = 0; index_s < sourceBlock.mlength_s; index_s++) {

                   if((sourceBlock.mPixel[index_t][index_s][index_v][index_u] != EMPTY_VALUE)&&(min_u_coord > index_u)) {
                       min_u_coord = index_u;
                   }

                   if((sourceBlock.mPixel[index_t][index_s][index_v][index_u] != EMPTY_VALUE)&&(max_u_coord < index_u)) {
                       max_u_coord = index_u;
                   }

                   if((sourceBlock.mPixel[index_t][index_s][index_v][index_u] != EMPTY_VALUE)&&(min_v_coord > index_v)) {
                       min_v_coord = index_v;
                   }

                   if((sourceBlock.mPixel[index_t][index_s][index_v][index_u] != EMPTY_VALUE)&&(max_v_coord < index_v)) {
                       max_v_coord = index_v;
                   }

                   if((sourceBlock.mPixel[index_t][index_s][index_v][index_u] != EMPTY_VALUE)&&(min_t_coord > index_t)) {
                       min_t_coord = index_t;
                   }

                   if((sourceBlock.mPixel[index_t][index_s][index_v][index_u] != EMPTY_VALUE)&&(max_t_coord < index_t)) {
                       max_t_coord = index_t;
                   }

                   if((sourceBlock.mPixel[index_t][index_s][index_v][index_u] != EMPTY_VALUE)&&(min_s_coord > index_s)) {
                       min_s_coord = index_s;
                   }

                   if((sourceBlock.mPixel[index_t][index_s][index_v][index_u] != EMPTY_VALUE)&&(max_s_coord < index_s)) {
                       max_s_coord = index_s;
                   }
               }
           }

       }
   }
   
   SetDimension(max_t_coord-min_t_coord+1, max_s_coord-min_s_coord+1, max_v_coord-min_v_coord+1, max_u_coord-min_u_coord+1);
   CopySubblockFrom(sourceBlock, min_t_coord, min_s_coord, min_v_coord, min_u_coord);
  
}

void Block4D :: Trim(const Block4D &sourceBlock, int &offset_t, int &offset_s, int &offset_v, int &offset_u) {
  
   //find boudaries coordinates
   int max_u_coord = 0;
   int min_u_coord = sourceBlock.mlength_u-1;
   int max_v_coord = 0;
   int min_v_coord = sourceBlock.mlength_v-1;
   int max_t_coord = 0;
   int min_t_coord = sourceBlock.mlength_t-1;
   int max_s_coord = 0;
   int min_s_coord = sourceBlock.mlength_s-1;
   
   for(int index_v = 0; index_v < sourceBlock.mlength_v; index_v++) {
       for(int index_u = 0; index_u < sourceBlock.mlength_u; index_u++) {
           for(int index_t = 0; index_t < sourceBlock.mlength_t; index_t++) {
               for(int index_s = 0; index_s < sourceBlock.mlength_s; index_s++) {

                   if((sourceBlock.mPixel[index_t][index_s][index_v][index_u] != EMPTY_VALUE)&&(min_u_coord > index_u)) {
                       min_u_coord = index_u;
                   }

                   if((sourceBlock.mPixel[index_t][index_s][index_v][index_u] != EMPTY_VALUE)&&(max_u_coord < index_u)) {
                       max_u_coord = index_u;
                   }

                   if((sourceBlock.mPixel[index_t][index_s][index_v][index_u] != EMPTY_VALUE)&&(min_v_coord > index_v)) {
                       min_v_coord = index_v;
                   }

                   if((sourceBlock.mPixel[index_t][index_s][index_v][index_u] != EMPTY_VALUE)&&(max_v_coord < index_v)) {
                       max_v_coord = index_v;
                   }

                   if((sourceBlock.mPixel[index_t][index_s][index_v][index_u] != EMPTY_VALUE)&&(min_t_coord > index_t)) {
                       min_t_coord = index_t;
                   }

                   if((sourceBlock.mPixel[index_t][index_s][index_v][index_u] != EMPTY_VALUE)&&(max_t_coord < index_t)) {
                       max_t_coord = index_t;
                   }

                   if((sourceBlock.mPixel[index_t][index_s][index_v][index_u] != EMPTY_VALUE)&&(min_s_coord > index_s)) {
                       min_s_coord = index_s;
                   }

                   if((sourceBlock.mPixel[index_t][index_s][index_v][index_u] != EMPTY_VALUE)&&(max_s_coord < index_s)) {
                       max_s_coord = index_s;
                   }
               }
           }

       }
   }
   
   SetDimension(max_t_coord-min_t_coord+1, max_s_coord-min_s_coord+1, max_v_coord-min_v_coord+1, max_u_coord-min_u_coord+1);
   CopySubblockFrom(sourceBlock, min_t_coord, min_s_coord, min_v_coord, min_u_coord);
  
   offset_t = min_t_coord;
   offset_s = min_s_coord;
   offset_v = min_v_coord;
   offset_u = min_u_coord;
}

int Block4D :: Padding(void) {
     
   for(int index_v = 0; index_v < mlength_v; index_v++) {
       for(int index_u = 0; index_u < mlength_u; index_u++) {

           //for(int index_t = mlength_t-1; index_t >= 0; index_t--) {
           for(int index_t = 0; index_t < mlength_t; index_t++) {
                int lower_index_s = -1;
                int search_index_s = 0;
		int nonempty_pixel_found = 0;
                while((nonempty_pixel_found == 0)&&(search_index_s < mlength_s)) {
		    if(mPixel[index_t][search_index_s][index_v][index_u] != EMPTY_VALUE) {
                        nonempty_pixel_found = 1;
                        lower_index_s = search_index_s;
                    }
                    search_index_s++;		  
                }
                for(search_index_s = 0; search_index_s < lower_index_s; search_index_s++) {
                    mPixel[index_t][search_index_s][index_v][index_u] = mPixel[index_t][lower_index_s][index_v][index_u];
                }
               
		int higher_index_s = mlength_s;
                search_index_s = mlength_s-1;
		nonempty_pixel_found = 0;
                while((nonempty_pixel_found == 0)&&(search_index_s >= 0)) {
		    if(mPixel[index_t][search_index_s][index_v][index_u] != EMPTY_VALUE) {
                        nonempty_pixel_found = 1;
                        higher_index_s = search_index_s;
                    }
                    search_index_s--;		  
                }
                for(search_index_s = higher_index_s+1; search_index_s < mlength_s; search_index_s++) {
                    mPixel[index_t][search_index_s][index_v][index_u] = mPixel[index_t][higher_index_s][index_v][index_u];
                }
            }
       }
   }
   for(int index_v = 0; index_v < mlength_v; index_v++) {
       for(int index_u = 0; index_u < mlength_u; index_u++) {
	 
            //for(int index_s = mlength_s-1; index_s >= 0; index_s--) {
            for(int index_s = 0; index_s < mlength_s; index_s++) {
                int lower_index_t = -1;
                int search_index_t = 0;
		int nonempty_pixel_found = 0;
                while((nonempty_pixel_found == 0)&&(search_index_t < mlength_t)) {
		    if(mPixel[search_index_t][index_s][index_v][index_u] != EMPTY_VALUE) {
                        nonempty_pixel_found = 1;
                        lower_index_t = search_index_t;
                    }
                    search_index_t++;		  
                }
                for(search_index_t = 0; search_index_t < lower_index_t; search_index_t++) {
                    mPixel[search_index_t][index_s][index_v][index_u] = mPixel[lower_index_t][index_s][index_v][index_u];
                }
               
		int higher_index_t = mlength_t;
                search_index_t = mlength_t-1;
		nonempty_pixel_found = 0;
                while((nonempty_pixel_found == 0)&&(search_index_t >= 0)) {
		    if(mPixel[search_index_t][index_s][index_v][index_u] != EMPTY_VALUE) {
                        nonempty_pixel_found = 1;
                        higher_index_t = search_index_t;
                    }
                    search_index_t--;		  
                }
                
                for(search_index_t = higher_index_t+1; search_index_t < mlength_t; search_index_t++) {
                    mPixel[search_index_t][index_s][index_v][index_u] = mPixel[higher_index_t][index_s][index_v][index_u];
                }
                                
            }
       }
   }
   
   for(int index_v = 0; index_v < mlength_v; index_v++) {
       for(int index_u = 0; index_u < mlength_u; index_u++) {	 
            for(int index_t = 0; index_t < mlength_t; index_t++) {
                for(int index_s = 0; index_s < mlength_s; index_s++) {
                    if(mPixel[index_t][index_s][index_v][index_u] == EMPTY_VALUE)
                        return(1);
                }
            }
       }
   }

  return(0);
}

int Block4D :: NumberOfValidElements(void) {
   
   int number_of_valid_elements=0;
   
   for(int index_v = 0; index_v < mlength_v; index_v++) {
       for(int index_u = 0; index_u < mlength_u; index_u++) {	 
            for(int index_t = 0; index_t < mlength_t; index_t++) {
                for(int index_s = 0; index_s < mlength_s; index_s++) {
                    if(mPixel[index_t][index_s][index_v][index_u] != EMPTY_VALUE)
                        number_of_valid_elements++;
                }
            }
       }
   }
   return(number_of_valid_elements);
}

