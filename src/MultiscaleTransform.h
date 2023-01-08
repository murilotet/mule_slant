#include "Matrix.h"
#include "Block4D.h"
#include <math.h>
#include <stdlib.h>

#ifndef MULTISCALETRANSFORM_H
#define MULTISCALETRANSFORM_H

struct MatrixListNode {
    Matrix *nodeMatrix;
    MatrixListNode *nextNode;
};

class MultiscaleTransform {
public:
    int mlength_t_max, mlength_s_max;               /*!< maximum transform size at directions t, s */
    int mlength_v_max, mlength_u_max;               /*!< maximum transform size at directions v, u */
    MatrixListNode *mTransform;                     /*!< list of transform matrices for every possible transform size  */
    int mNumberOfScales;                            /*!< number of transform matrices  */
    double mTransformGain_t, mTransformGain_s;      /*!< gain of the transform at directions t, s */ 
    double mTransformGain_v, mTransformGain_u;      /*!< gain of the transform at directions v, u */ 
    int mBypass_t, mBypass_s;                       /*!< bypass switch of the transform at directions t, s */ 
    int mBypass_v, mBypass_u;                       /*!< bypass switch of the transform at directions v, u */ 
    MultiscaleTransform(void);
    ~MultiscaleTransform(void);
    void SetDimension(int max_t, int max_s, int max_v, int max_u);
    double* TransformCoefficients(int length, int type);
    void Transform4D(Block4D &block);
    void InverseTransform4D(Block4D &block);
};

#endif /* MULTISCALETRANSFORM_H */

