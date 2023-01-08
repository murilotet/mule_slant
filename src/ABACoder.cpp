#include "ABACoder.h"
#include <math.h>
/*********************************************************************************************/
/*                                    class ABACoder methods                                  */
/*********************************************************************************************/
ABACoder :: ABACoder(void) {
    mOutputFilePointer = NULL;
    mLow = 0;
    mHigh = MAXINT;   
    mNumberOfBitsInBuffer = 0;
    mScalingsCounter = 0;
}
ABACoder :: ~ABACoder(void) {
  
}
void ABACoder :: InitEncoder(FILE *ofp) {
/*! initializes arithmetic coding: sets the output file, sets the interval to [0,1) */   
  
    mOutputFilePointer = ofp;
      
    mLow = 0;
    mHigh = MAXINT;
    
    mNumberOfBitsInBuffer = 0;
    mScalingsCounter = 0;
    mBitBuffer = 0;
    
}
void ABACoder :: EncodeBit(int inputbit, ProbabilityModel &mPmodel) {
/*! encodes a binary symbol using the given probability model */   

    unsigned long int acumFreq_0 = mPmodel.mCumulativeFrequency[0];
    unsigned long int acumFreq_1 = mPmodel.mCumulativeFrequency[1];
    unsigned long int length_0 = (((mHigh - mLow + 1) * acumFreq_0)/acumFreq_1);

    if(inputbit == 0) {
        mHigh = mLow + length_0 - 1;
    }
    else {
        mLow = mLow + length_0;
    }

    while(((mLow & MSB_MASK) == (mHigh & MSB_MASK)) || ((mLow >= SECOND_MSB_MASK) && (mHigh < (MSB_MASK + SECOND_MSB_MASK)))) {
        if((mLow & MSB_MASK) == (mHigh & MSB_MASK)) {
            int bit = 1;
            if((mLow & MSB_MASK) == 0)
                bit = 0;
            
            OutputBit(bit);
                        
            mLow = mLow << 1;
            mHigh = mHigh << 1;
            mLow = mLow & RESET_LSB_MASK;
            mHigh = mHigh | SET_LSB_MASK;
            mHigh = mHigh & MAXINT;
            mLow = mLow & MAXINT;
            while(mScalingsCounter > 0) {
                mScalingsCounter--;
                OutputBit(1-bit);
 
            }                
        }
        if((mLow >= SECOND_MSB_MASK) && (mHigh < (MSB_MASK + SECOND_MSB_MASK))) {
            mLow = mLow << 1;
            mHigh = mHigh << 1;
            mLow = mLow & RESET_LSB_MASK;
            mHigh = mHigh | SET_LSB_MASK;
            mLow = mLow ^ MSB_MASK;
            mHigh = mHigh ^ MSB_MASK;
            mScalingsCounter++;

            mHigh = mHigh & MAXINT;
            mLow = mLow & MAXINT;
        }
    }
    mHigh = mHigh & MAXINT;
    mLow = mLow & MAXINT;
}


void ABACoder :: Flush(void) {
/*! write remaining bits in the outputfile */   
    
    mScalingsCounter++;
    int bit;
    if(mLow >= SECOND_MSB_MASK)
        bit = 1;
    else
        bit = 0;
    
    OutputBit(bit);
    while(mScalingsCounter > 0) {
        OutputBit(1-bit);
        mScalingsCounter--;
    }                
    
    if(mNumberOfBitsInBuffer > 0)
        for(int n = mNumberOfBitsInBuffer; n < 8; n++)
            mBitBuffer = mBitBuffer >> 1;
    
    fprintf(mOutputFilePointer, "%c", mBitBuffer);
    
    mNumberOfBitsInBuffer = 0;    
}

void ABACoder :: OutputBit(int bit) {
/*! write bits in the output file */   

    mBitBuffer = mBitBuffer >> 1;
    if (bit == 1)
        mBitBuffer = mBitBuffer | 0x80;
    
    mNumberOfBitsInBuffer++;
    
    if(mNumberOfBitsInBuffer == 8) {
        fprintf(mOutputFilePointer, "%c", mBitBuffer);
        mNumberOfBitsInBuffer = 0;
    }
}