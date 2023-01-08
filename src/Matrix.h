#include <stdio.h>

#ifndef MATRIX_H
#define MATRIX_H

#define VALID_MATRIX 0
#define NON_SQUARE_MATRIX -1
#define SINGULAR_MATRIX -2
#define WRONG_SIZE__MATRIX -3

#define PI 3.141592653589793

class Matrix {
public:
    double *mElementData;       /*!< pointer to a linear array of element data */
    double **mElement;          /*!< array of pointers to allow two-dimensional indexing of the element data */
    int mLines;                 /*!< number of lines of the matrix */
    int mColumns;               /*!< number of columns of the matrix */
    Matrix(int lines=0, int columns=0);
    ~Matrix(void);
    void SetDimension(int lines, int columns);
    int PreMultiply(Matrix const &M);
    int PostMultiply(Matrix const &M);
    void Multiply(double const &a); 
    void AccumulateFromColumn(int const &pivot_column);
    int Add(Matrix const &M);
    void Add(double const &a); 
    void CopyFrom(Matrix const &M, int source_line_offset=0, int source_column_offset=0, int target_line_offset=0, int target_column_offset=0);
    int Inverse(Matrix &M);
    double Determinant(void);
    int SubMatrix(Matrix const &M, int pivot_line, int pivot_column);
    int SubMatrix(Matrix *M, int pivot_line, int pivot_column);
    void Zeros(void);
    void Set(double const &a);
    double Mean(void);
    void Eye(void);
    void DCT(void);
    void IDCT(void);
    void Display(void);    
};

#endif