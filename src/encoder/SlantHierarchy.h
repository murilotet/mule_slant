#include "Block4D.h"
#include "View.h"
#include "MultiscaleTransform.h"
#include "TransformPartition.h"
#include "Hierarchical4DEncoder.h"
#include "IntegerString.h"
#ifndef SLANT_HIERARCHY_H
#define SLANT_HIERARCHY_H

#define SPLIT_NODE 0
#define LEAF_NODE 1

class SlantHierarchy 
{
public:  
  int mMinBlockSize_u;         /*!< minimum slant block size u dimension */
  int mMinBlockSize_v;         /*!< minimum slant block size v dimension */
  int mSlantMax;               /*!< maximum slant */
  int mSlantBits;              /*!< number of bits to encode the slant */
  IntegerString mPreComputedSlants;  /*!< externaly evaluated slants */
  IntegerString mSlantTreeCode;    /*!< flags and slant values of the slant tree */
  int mMinimumBitplane;        /*!< Minimum bitplane to encode using the hexadecatree */ 
  View mInverseDepthMapPGM;    /*!< Inverse depth map to guide the search for optimum slants */
  double mInverseDepthMapGain; /*!< Scale factor for the inverse depth map */
  int mInverseDepthMapZero;    /*!< Zero point for the inverse depth map */
  int mUseDepthMap;            /*!< If "0" disables the use of the inverse depth map */
  float mSlantScaleU;          /*!< Disparity scale factor in the u direction  */
  float mSlantScaleV;          /*!< Disparity scale factor in the v direction  */
  int mMapAssistedSearchRange; /*!< Search range around the the inverse depth map clue */
  SlantHierarchy(void);
  ~SlantHierarchy(void);
  void FindOptimumSlantTree(const Block4D &inputBlock, MultiscaleTransform &mt, double lambda, int v_position, int u_position);
  double FindOptimumSlantTreeStep(const Block4D &inputBlock, IntegerString &treeCode, MultiscaleTransform &mt, double lambda, int *UVBlockPixelsPosition, int accumulatedSlant);
  void PruneOptimumSlantTree(const Block4D &inputBlock, MultiscaleTransform &mt, double lambda);
  double PruneOptimumSlantTreeStep(const Block4D &inputBlock, IntegerString &treeCode, MultiscaleTransform &mt, double lambda);
  int ReadOptimumSlantTree(const Block4D &inputBlock, MultiscaleTransform &mt, double lambda, FILE *ifp);
  int ReadOptimumSlantTreeStep(IntegerString &treeCode, FILE *ifp);
  void EncodeOptimumSlantTree(const Block4D &inputBlock, TransformPartition &tp, MultiscaleTransform &mt, Hierarchical4DEncoder &he, double lambda);
  void EncodeOptimumSlantTreeStep(const Block4D &inputBlock, TransformPartition &tp, MultiscaleTransform &mt, Hierarchical4DEncoder &he, double lambda);
  int OptimumBitPlane(const Block4D &inputBlock, MultiscaleTransform &mt, float lambda); 
  int EvaluateRate(const Block4D &lfBlock, int position_t, int position_s, int position_v, int position_u, int length_t, int length_s, int length_v, int length_u, int currentBitplane, int minBitplane);
  double EvaluateDistortion(const Block4D &lfBlock, int minBitplane);
  double EvaluateJ(const Block4D &lfBlock, int position_t, int position_s, int position_v, int position_u, int length_t, int length_s, int length_v, int length_u, int currentBitplane, int minBitplane, float lambda);
  int FindBestSlant4DCT(const Block4D &inputBlock, int min_slant, int max_slant, Block4D &slantedBlock, MultiscaleTransform &mt, double lambda, int &optimumSlant, double &optimumJ);
  void ScaleSlant(int slant, int &slant_v, int &slant_u, int length_t, int length_s);
  int GetMeanSlantFromMapPGM(int *position);
};
#endif
