#include "Matrix.h"
#include <math.h>

Matrix :: Matrix(int lines, int columns) {
  
    mElementData = NULL;
    mElement = NULL;
    mLines = 0;
    mColumns = 0;
    SetDimension(lines, columns);
    
}

Matrix :: ~Matrix(void) {
  
    SetDimension(0, 0);
}

void Matrix :: SetDimension(int lines, int columns) {
/*! sets the number of lines and columns and allocates memory for the element data */  

    if(lines * columns == 0) {
        if(mElementData != NULL) {
            delete [] mElementData;
        }
        if(mElement != NULL) {
            delete [] mElement;
        }
        mLines = 0;
        mColumns = 0;
        mElementData = NULL;
        mElement = NULL;
        return;
    }
    
    mElementData = new double [lines * columns];
    mElement = new double * [lines];
    for(int line_index = 0; line_index < lines; line_index++) {
         mElement[line_index] = &mElementData[line_index * columns];
    }
    mLines = lines;
    mColumns = columns;
}

int Matrix :: PreMultiply(Matrix const &M) {
/*! premultiplies this matrix by matrix M */  
    if(M.mColumns != mLines) {
        printf("Can't multiply matrices of incompatible dimensions\n");
        return(WRONG_SIZE__MATRIX);
    }
    
    Matrix T(M.mLines, mColumns);
    for(int line_index = 0; line_index < M.mLines; line_index++) {
        for(int column_index = 0; column_index < mColumns; column_index++) {
            T.mElement[line_index][column_index] = 0.0; 
            for(int product_index = 0; product_index < mLines; product_index++) {
                  T.mElement[line_index][column_index] += M.mElement[line_index][product_index] * mElement[product_index][column_index];  
            }  
        }
    }
    SetDimension(M.mLines, mColumns);
    for(int n = 0; n < mLines*mColumns; n++)
        mElementData[n] = T.mElementData[n];
    //T.SetDimension(0,0);
    return(0);
}

int Matrix :: PostMultiply(Matrix const &M) {
/*! postmultiplies this matrix by matrix M */  
  
    if(mColumns != M.mLines) {
        printf("Can't multiply matrices of incompatible dimensions\n");
        return(WRONG_SIZE__MATRIX);
    }
    
    Matrix T(mLines, M.mColumns);
    for(int line_index = 0; line_index < mLines; line_index++) {
        for(int column_index = 0; column_index < M.mColumns; column_index++) {
            T.mElement[line_index][column_index] = 0.0; 
            for(int product_index = 0; product_index < mColumns; product_index++) {
                  T.mElement[line_index][column_index] += mElement[line_index][product_index] * M.mElement[product_index][column_index];   
            }  
        }
    }
    SetDimension(mLines, M.mColumns);
    for(int n = 0; n < mLines*mColumns; n++)
        mElementData[n] = T.mElementData[n];
    //T.SetDimension(0,0);
    return(VALID_MATRIX);
  
    
}

void Matrix :: Multiply(double const &a) {
/*! multiplies this matrix by scalar a */  
  
    for(int n = 0; n < mLines*mColumns; n++)
        mElementData[n] *= a;

}

void Matrix :: AccumulateFromColumn(int const &pivot_column) {
/*! accumulates the element values, on the columns direction, from pivot_column to the end */
    
    for(int line_index = 0; line_index < mLines; line_index++) {
        for(int column_index = pivot_column+1; column_index < mColumns; column_index++) {
            mElement[line_index][pivot_column] += mElement[line_index][column_index];
            mElement[line_index][column_index] = 0.0;
        }
    }
}

int Matrix :: Add(Matrix const &M) {
/*! adds matrix M to this matrix */  
  
    if((mColumns != M.mColumns)||(mLines != M.mLines)) {
        printf("Can't add matrices of different dimensions\n");
        return(WRONG_SIZE__MATRIX);
    }
    
    for(int n = 0; n < mLines*mColumns; n++)
        mElementData[n] += M.mElementData[n];
    
    return(VALID_MATRIX);
}

void Matrix :: Add(double const &a) {
/*! adds scalar a to all elements of this matrix */  
  
    for(int n = 0; n < mLines*mColumns; n++)
        mElementData[n] += a;

}

void Matrix :: CopyFrom(Matrix const &M, int source_line_offset, int source_column_offset, int target_line_offset, int target_column_offset) {
/*! copy a sub matrix from M */

    for(int line_index = 0; line_index < mLines; line_index++) {
        for(int column_index = 0; column_index < mColumns; column_index++) {
            if((line_index+source_line_offset < M.mLines)&&(column_index+source_column_offset < M.mColumns))
                if((line_index+target_line_offset < mLines)&&(column_index+target_column_offset < mColumns))
                    mElement[line_index+target_line_offset][column_index+target_column_offset] = M.mElement[line_index+source_line_offset][column_index+source_column_offset];
            }  
        }
}
int Matrix :: Inverse(Matrix &M) {
/*! evaluates the inverse of this matrix*/ 
    
    if(mLines != mColumns) {
        printf("Can't evaluate the inverse of a non-square matrix\n");
        return(NON_SQUARE_MATRIX);
    }
    
    Matrix S(M.mLines, 2*M.mColumns);
    S.Zeros();
    S.CopyFrom(M);
    
    double *temp_line = new double [2*M.mColumns];
    
    for(int diagonal_index = 0; diagonal_index < M.mLines; diagonal_index++) {
        S.mElement[diagonal_index][diagonal_index+M.mColumns] = 1.0;
    }
    for(int pivot_index = 0; pivot_index < M.mColumns; pivot_index++) {
        
        if(S.mElement[pivot_index][pivot_index] == 0) {
            int search_index = pivot_index;
            while((S.mElement[search_index][pivot_index] == 0)&&(search_index < M.mLines)) 
                search_index++;
            if(search_index == M.mLines) {
                delete [] temp_line;
                printf("Matrix is singular\n");
                return(SINGULAR_MATRIX);                        
            }
            for(int temp_index = 0; temp_index < 2*M.mColumns; temp_index++) {
                temp_line[temp_index] = S.mElement[search_index][temp_index];
                S.mElement[search_index][temp_index] = S.mElement[pivot_index][temp_index];
                S.mElement[pivot_index][temp_index] = temp_line[temp_index];
            }
        }
        for(int temp_index = 2*M.mColumns-1; temp_index >= pivot_index; temp_index--) 
            S.mElement[pivot_index][temp_index] /= S.mElement[pivot_index][pivot_index];
        for(int line_index = 0; line_index < M.mLines; line_index++) {
            for(int temp_index = 2*M.mColumns-1; temp_index >= pivot_index; temp_index--) {
                if(line_index != pivot_index) 
                   S.mElement[line_index][temp_index] -= S.mElement[line_index][pivot_index]*S.mElement[pivot_index][temp_index];
            }
                
        }
        
    }
    SetDimension(M.mLines, M.mColumns);
    CopyFrom(S,0,M.mColumns);

    delete [] temp_line;
    return(VALID_MATRIX);
}

double Matrix :: Determinant(void) {
/*! returns the determinant of this matrix */
    
    if(mLines != mColumns) {
        printf("Can't evaluate the determinant of a non-square matrix\n");
        return(0);
    }
    
    if((mLines == 1)&&(mColumns == 1)) {
        return(mElement[0][0]); 
    }    
    
    if((mLines == 2)&&(mColumns == 2)) {
        return(mElement[0][0]*mElement[1][1] - mElement[0][1]*mElement[1][0]); 
    }
    double Det;
    for(int pivot_line_index = 0; pivot_line_index < mLines; pivot_line_index++) {
        Matrix M(mLines-1,mColumns-1);
        M.SubMatrix(this, pivot_line_index, 0);
        //printf("M(%d, 0)\n", pivot_line_index);
        //M.Display();
        if(pivot_line_index%2 == 0) 
           Det += mElement[pivot_line_index][0] * M.Determinant(); 
        else 
           Det -= mElement[pivot_line_index][0] * M.Determinant(); 
    }
    return(Det);
}

void Matrix :: Display(void) {
/*! print the matrix on the console */
    
    for(int line_index = 0; line_index < mLines; line_index++) {
        for(int column_index = 0; column_index < mColumns; column_index++) {
	    printf(" %f", mElement[line_index][column_index]);
	}
	printf("\n");
    }
} 

int Matrix :: SubMatrix(Matrix const &M, int pivot_line, int pivot_column) {
/*! copy from matrix M the sub matrix deleting the pivot_line line and the pivot_column column */
    
    if((mLines >= M.mLines)||(mColumns >= M.mColumns)) {
        printf("Source matrix must be larger than current matrix to compute the submatrix\n");
        return(WRONG_SIZE__MATRIX);
    }
    for(int line_index = 0; line_index < mLines; line_index++) {
        for(int column_index = 0; column_index < mColumns; column_index++) {
            int source_line_index = (line_index < pivot_line) ? line_index : line_index+1;
            int source_column_index = (column_index < pivot_column) ? column_index : column_index+1;
            mElement[line_index][column_index] = M.mElement[source_line_index][source_column_index];
        }
    }
    return(VALID_MATRIX);
}

int Matrix :: SubMatrix(Matrix *M, int pivot_line, int pivot_column) {
/*! copy from matrix pointed by *M the sub matrix deleting the pivot_line line and the pivot_column column */

    if((mLines >= M->mLines)||(mColumns >= M->mColumns)) {
        printf("Source matrix must be larger than current matrix to compute the submatrix\n");
        return(WRONG_SIZE__MATRIX);
    }
    for(int line_index = 0; line_index < mLines; line_index++) {
        for(int column_index = 0; column_index < mColumns; column_index++) {
            int source_line_index = (line_index < pivot_line) ? line_index : line_index+1;
            int source_column_index = (column_index < pivot_column) ? column_index : column_index+1;
            mElement[line_index][column_index] = M->mElement[source_line_index][source_column_index];
        }
    }
    return(VALID_MATRIX);
}

void Matrix :: DCT(void) {
/*! loads the DCT transform matrix */
    
    if(mLines * mColumns == 0)
        return;
    
    for(int column_index = 0; column_index < mColumns; column_index++) {
        mElement[0][column_index] = 1.0; 
    }
    for(int line_index = 1; line_index < mLines; line_index++) {
        for(int column_index = 0; column_index < mColumns; column_index++) {
            mElement[line_index][column_index] = sqrt( 2.0 ) *  cos (PI * ( line_index * ( 2 * column_index + 1 ) ) / ( double ) ( 2 * mLines )); 
        }
    }
}

void Matrix :: IDCT(void) {
/*! loads the IDCT transform matrix */
  
    if(mLines * mColumns == 0)
        return;
    
    for(int line_index = 0; line_index < mLines; line_index++) {
        mElement[line_index][0] = 1.0 / ( double ) ( mLines ); 
        for(int column_index = 1; column_index < mColumns; column_index++) {
            mElement[line_index][column_index] = sqrt( 2.0 ) *  cos (PI * ( ( 2 *line_index + 1 ) * column_index ) / ( double ) ( 2 * mLines )) / ( double ) ( mLines ); 
        }
    }
 
}

void Matrix :: Zeros(void) {
/*! resets all elements to zero */
  
    for(int n = 0; n < mLines*mColumns; n++)
        mElementData[n] = 0.0;
}

void Matrix :: Set(double const &a) {
/*! sets all elements to a */
    
    for(int n = 0; n < mLines*mColumns; n++)
        mElementData[n] = a;
}

double Matrix :: Mean(void) {
/*! returns the mean value of the elements of this matrix */
    
    double mean_value=0.0;
    for(int n = 0; n < mLines*mColumns; n++)
        mean_value += mElementData[n];
    //return(mean_value);
    return(mean_value/(mLines*mColumns));
}
void Matrix :: Eye(void) {
/*! loads the identity matrix */
  
    int range = (mLines < mColumns) ? mLines : mColumns;
    for(int n = 0; n < range; n++)
        mElement[n][n] = 1.0;
}
