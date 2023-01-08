#ifndef PROBMODEL_H
#define PROBMODEL_H

#define MAXFREQUENCY 4095
//#define MAXFREQUENCY 2047

/*********************************************************************************************/
/*                                   class ProbabilityModel                                  */
/*********************************************************************************************/
class ProbabilityModel {
  public:  
    unsigned int mCumulativeFrequency[2];  /*> cumulative number of occurences of each symbol.*/
    double mRate[2];                       /*> rates needed to encode each symbol.*/
    double mLog_2;
    ProbabilityModel(void);
    void UpdateModel(int bit);
    void ResetModel();
    void CopyModel(ProbabilityModel *prob_model);
    double Rate(int bit);
};

#endif /* PROBMODEL_H */
