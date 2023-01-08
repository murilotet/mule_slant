#include "ProbabilityModel.h"
#include <math.h>

/*********************************************************************************************/
/*                               class ProbabilityModel methods                              */
/*********************************************************************************************/

ProbabilityModel :: ProbabilityModel(void) {
    mCumulativeFrequency[0] = 1;    
    mCumulativeFrequency[1] = 2;  
    mRate[0] = 1.0;
    mRate[1] = 1.0;
    mLog_2 = log(2);
}

void ProbabilityModel :: UpdateModel(int bit) {
    mCumulativeFrequency[bit]++;
    if(bit == 0)
        mCumulativeFrequency[1]++;
    
    if(mCumulativeFrequency[1] >= MAXFREQUENCY) {
        mCumulativeFrequency[0] =  mCumulativeFrequency[0] >> 1;
        mCumulativeFrequency[1] =  mCumulativeFrequency[1] >> 1;
        if(mCumulativeFrequency[0] == 0) {
            mCumulativeFrequency[0]++;
            mCumulativeFrequency[1]++;
	}
        if(mCumulativeFrequency[0] == mCumulativeFrequency[1]) 
             mCumulativeFrequency[1]++;
   }
   
   double f0 = mCumulativeFrequency[0];
   double f1 = mCumulativeFrequency[1]-mCumulativeFrequency[0]; 
   //mRate[0] = -log(f0/mCumulativeFrequency[1])/mLog_2;
   //mRate[1] = -log(f1/mCumulativeFrequency[1])/mLog_2;
   mRate[0] = -log2(f0/mCumulativeFrequency[1]);
   mRate[1] = -log2(f1/mCumulativeFrequency[1]);
}

void ProbabilityModel :: ResetModel() {
    mCumulativeFrequency[0] = 1;
    mCumulativeFrequency[1] = 2;    
    mRate[0] = 1.0;
    mRate[1] = 1.0;
}

void ProbabilityModel :: CopyModel(ProbabilityModel *prob_model) {
    mCumulativeFrequency[0] = prob_model->mCumulativeFrequency[0];
    mCumulativeFrequency[1] = prob_model->mCumulativeFrequency[1];    
    mRate[0] = prob_model->mRate[0];
    mRate[1] = prob_model->mRate[1];
}

double ProbabilityModel :: Rate(int symbol) {
  
    return(mRate[symbol]);
}
