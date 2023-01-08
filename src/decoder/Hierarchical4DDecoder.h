/* 
 * File:   Hierarchical4DDecoder.h
 * Author: murilo
 *
 * Created on December 28, 2017, 11:41 AM
 */
#include "Block4D.h"
#include "ABADecoder.h"
#include "ProbabilityModel.h"
#include <stdlib.h>
#include <stdio.h>

#ifndef HIERARCHICAL4DDECODER_H
#define HIERARCHICAL4DDECODER_H

#define BITPLANE_BYPASS -1
#define BITPLANE_BYPASS_FLAGS -1
#define SYMBOL_PROBABILITY_MODEL_INDEX 1
#define SEGMENTATION_PROB_MODEL_INDEX 33
#define PARTITION_PROB_MODEL_INDEX 97
#define PREDICTION_PROB_MODEL_INDEX 161
#define NUMBER_OF_MODELS 226

class Hierarchical4DDecoder {
public:
    Block4D mSubbandLF;
    ABADecoder mEntropyDecoder;
    ProbabilityModel *mPmodel;
    int mSuperiorBitPlane, mInferiorBitPlane;
    int mSegmentationFlagProbabilityModelIndex;
    int mSymbolProbabilityModelIndex;
    int mPredictionFlagProbabilityModelIndex;
    int mPreSegmentation;
    Hierarchical4DDecoder(void);
    ~Hierarchical4DDecoder(void);
    void StartDecoder(FILE *inputFilePointer);
    void RestartProbabilisticModel(void);
    void DecodeBlock(int position_t, int position_s, int position_v, int position_u, int length_t, int length_s, int length_v, int length_u, int bitplane);
    int DecodeCoefficient(int bitplane);
    int DecodeSegmentationFlag(int bitplane);
    int DecodePredictionFlag(int tree_depth);
    int DecodePartitionFlag(void);
    int DecodeInteger(int precision);
    void DoneDecoding(void);
};
#endif /* HIERARCHICAL4DDECODER_H */

