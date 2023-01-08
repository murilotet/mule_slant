#include "Block4D.h"
#include "MultiscaleTransform.h"
#include "PartitionDecoder.h"
#include "Hierarchical4DDecoder.h"

#ifndef SLANT_HIERARCHY_DECODER_H
#define SLANT_HIERARCHY_DECODER_H

#define SPLIT_NODE 0
#define LEAF_NODE 1

class SlantHierarchyDecoder 
{
public: 
  int mSlantMax;               /*!< maximum slant */
  int mSlantBits;              /*!< number of bits to decode slant */
  float mSlantScaleU;
  float mSlantScaleV;
  SlantHierarchyDecoder(void);
  ~SlantHierarchyDecoder(void);
  void DecodeOptimumSlantTree(Block4D &decodedBlock, PartitionDecoder &pd, MultiscaleTransform &mt, Hierarchical4DDecoder &hd);
  void DecodeOptimumSlantTreeStep(Block4D &decodedBlock, PartitionDecoder &pd, MultiscaleTransform &mt, Hierarchical4DDecoder &hd);
  void ScaleSlant(int slant, int &slant_v, int &slant_u, int length_t, int length_s);
};
#endif
