#include "MultiscaleTransform.h"
#include "Hierarchical4DEncoder.h"
#include <math.h>
#include <string.h>

#ifndef TRANSFORMPARTITION_H
#define TRANSFORMPARTITION_H

#define NOSPLITFLAG 'T'
#define INTRAVIEWSPLITFLAG 'S'
#define INTERVIEWSPLITFLAG 'V'
#define NOSPLITFLAGSYMBOL 0
#define INTRAVIEWSPLITFLAGSYMBOL 1
#define INTERVIEWSPLITFLAGSYMBOL 2
#define MINIMUM_BITPLANE_PRECISION 5

class TransformPartition {
public:  
    char *mPartitionCode;               /*!< String of flags defining the partition tree */
    int mPartitionCodeIndex;            /*!< Scan index for the partition tree code string */
    double mLagrangianCost;             /*!< Lagrangian cost of the chosen partition */
    int mEvaluateOptimumBitPlane;       /*!< Toggles the optimum bit plane evaluation procedure on and off */
    Block4D mPartitionData;             /*!< DCT of all subblocks of the partition */
    Block4D mDecodedData;               /*!< Inverse DCT of all quantized subblocks of the partition */
    int mlength_t_min, mlength_s_min;   /*!< minimum subblock size at directions t, s */
    int mlength_v_min, mlength_u_min;   /*!< minimum subblock size at directions v, u */
    TransformPartition(void);
    ~TransformPartition(void);
    void SetDimension(int mlength_t, int mlength_s, int mlength_v, int mlength_u, int mlength_t_min, int mlength_s_min, int mlength_v_min, int mlength_u_min);
    double RDoptimizeTransform(Block4D &inputBlock, MultiscaleTransform &mt, MultiscaleTransform &imt, Hierarchical4DEncoder &entropyCoder, double lambda, int evaluate_minimum_bitplane);
    double RDoptimizeTransformStep(Block4D &inputBlock, Block4D &transformedBlock, Block4D &decodedBlock, int *position, int *length, MultiscaleTransform &mt, MultiscaleTransform &imt, Hierarchical4DEncoder &entropyCoder, double lambda, char **partitionCode);
    double RDoptimizeTransform(Block4D &inputBlock, MultiscaleTransform &mt, Hierarchical4DEncoder &entropyCoder, double lambda, int evaluate_minimum_bitplane);
    double RDoptimizeTransformStep(Block4D &inputBlock, Block4D &transformedBlock, Block4D &decodedBlock, int *position, int *length, MultiscaleTransform &mt, Hierarchical4DEncoder &entropyCoder, double lambda, char **partitionCode);
    void EncodePartition(Hierarchical4DEncoder &entropyCoder, double lambda, int encode_minimum_bitplane);
    void EncodePartitionStep(int *position, int *length, Hierarchical4DEncoder &entropyCoder, double lambda);
    void EncodePartition(MultiscaleTransform &imt, Hierarchical4DEncoder &entropyCoder, double lambda, int encode_minimum_bitplane);
    void EncodePartitionStep(MultiscaleTransform &imt, int *position, int *length, Hierarchical4DEncoder &entropyCoder, double lambda);
};

#endif /* TRANSFORMOPTIMIZATION_H */

