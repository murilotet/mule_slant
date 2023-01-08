#include "MultiscaleTransform.h"
#include "Hierarchical4DDecoder.h"
#include <math.h>
#include <string.h>

#ifndef TRANSFORMPARTITION_H
#define TRANSFORMPARTITION_H

#define NOSPLITFLAGSYMBOL 0
#define INTRAVIEWSPLITFLAGSYMBOL 1
#define INTERVIEWSPLITFLAGSYMBOL 2
#define MINIMUM_BITPLANE_PRECISION 5


class PartitionDecoder {
public:  
    int mPartitionCodeMaxLength;        /*!< Maximum length of the partition tree code string */
    int mUseSameBitPlane;               /*!< Forces to use the same minimum bitplane for all subblocks */
    Block4D mPartitionData;             /*!< DCT of all subblocks of the partition */
    PartitionDecoder(void);
    void DecodePartition(Hierarchical4DDecoder &entropyDecoder, MultiscaleTransform &mt, int getminimumbitplane);
    void DecodePartitionStep(int *position, int *length, Hierarchical4DDecoder &entropyDecoder, MultiscaleTransform &mt);
};

#endif /* TRANSFORMOPTIMIZATION_H */

