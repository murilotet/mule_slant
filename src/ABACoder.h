#include "ProbabilityModel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef ABACODER_H
#define ABACODER_H

#define MAXINT 0xffff
#define RESET_LSB_MASK 0xfffe
#define SET_LSB_MASK 0x0001
#define INTERVAL_PRECISION 16
#define MSB_MASK 0x8000
#define SECOND_MSB_MASK 0x4000

/*********************************************************************************************/
/*                                         class ABACoder                                     */
/*********************************************************************************************/
class ABACoder {
public:
  unsigned char mBitBuffer;        /*!< bit-writable buffer */
  int mNumberOfBitsInBuffer;       /*!< number of valid bits in buffer */
  int mScalingsCounter;            /*!< number of renormalizations performed */
  FILE *mOutputFilePointer;        /*!< pointer to output file */
  unsigned int mLow;               /*!< interval lower limit */
  unsigned int mHigh;              /*!< interval upper limit */
  ABACoder(void);
  ~ABACoder(void);
  void InitEncoder(FILE *ofp);
  void EncodeBit(int bit, ProbabilityModel &mPmodel);
  void Flush(void);  
  void OutputBit(int bit);
};

#endif // ABACODER_H

