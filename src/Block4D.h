#ifndef BLOCK4D_H
#define  BLOCK4D_H

#define block4DElementType int

#define PI 3.141592653589793

#define EMPTY_VALUE (-(1<<((8*sizeof(block4DElementType))-1)))

class Block4D 
{
public: 
    block4DElementType *mPixelData;           /*!< pointer to a linear array of pixel data */
    block4DElementType ****mPixel;            /*!< pointer to a 4 dimensional array of pixel data that accesses the same positions as mPixelData */
    int mlength_u;                            /*!< u dimension block size */
    int mlength_v;                            /*!< v dimension block size */
    int mlength_s;                            /*!< s dimension block size */
    int mlength_t;                            /*!< t dimension block size */
    Block4D(void);
    ~Block4D(void);
    void SetDimension(int length_t, int length_s, int length_v, int length_u);  
    Block4D* operator + (const Block4D &B);
    Block4D* operator * (const Block4D &B);
    Block4D* operator - (const Block4D &B);
    Block4D* operator + (const int &a);
    Block4D* operator * (const int &a);
    Block4D* operator - (const int &a);
    Block4D* operator / (const int &a);
    void operator += (const Block4D &B);
    void operator += (const int &a);
    void operator *= (const Block4D &B);
    void operator *= (const double &a);
    void operator -= (const Block4D &B);
    void operator = (const Block4D &B);
    void operator = (Block4D* B);
    void CopySubblockFrom(const Block4D &B, int source_offset_t, int source_offset_s, int source_offset_v, int source_offset_u, int target_offset_t=0, int target_offset_s=0, int target_offset_v=0, int target_offset_u=0);
    void CopySubblockFrom(Block4D *B, int source_offset_t, int source_offset_s, int source_offset_v, int source_offset_u, int target_offset_t=0, int target_offset_s=0, int target_offset_v=0, int target_offset_u=0);
    void JoinTAxis(const Block4D &B0, const Block4D &B1);
    void JoinSAxis(const Block4D &B0, const Block4D &B1);
    void JoinVAxis(const Block4D &B0, const Block4D &B1);
    void JoinUAxis(const Block4D &B0, const Block4D &B1);
    void Ones(void);
    void Zeros(void);
    void Display(void);
    void DCT_U(int scale);
    void IDCT_U(int scale);
    void DCT_V(int scale);
    void IDCT_V(int scale);
    void DCT_S(int scale);
    void IDCT_S(int scale);
    void DCT_T(int scale);
    void IDCT_T(int scale);
    void TRANSFORM_U(double scale, double *coefficients);
    void TRANSFORM_V(double scale, double *coefficients);
    void TRANSFORM_S(double scale, double *coefficients);
    void TRANSFORM_T(double scale, double *coefficients);
    void DCT4(int scale);
    void IDCT4(int scale);
    double L2Norm(void);
    block4DElementType Max(void);
    block4DElementType Min(void);    
    //double Entropy(int number_of_bins=1024, int maxBitplane=30);
    double Entropy(int minBitplane=12);
    void Extend_U(int position_u);
    void Extend_V(int position_v);
    void Extend_S(int position_s);
    void Extend_T(int position_t);
    void Clip(int minValue, int maxValue);
    void Threshold(int minMagnitude, int maxMagnitude);
    void Shift_UVPlane(int shift, int position_t, int position_s);
    void SlantAndPadd(int dPv, int dPu, const Block4D &sourceBlock);
    void Slant(int dPv, int dPu, const Block4D &sourceBlock);
    void DeSlantMeanPadd(int dPv, int dPu, const Block4D &sourceBlock);
    void Padding(int dPv, int dPu, const Block4D &sourceBlock);
    int Padding(void);
    void DeSlant(int dPv, int dPu, const Block4D &sourceBlock);
    void Erase(void);
    void Trim(const Block4D &sourceBlock);
    void Trim(const Block4D &sourceBlock, int &offset_t, int &offset_s, int &offset_v, int &offset_u);
    int NumberOfValidElements(void);
    
	block4DElementType GetPixel(int position_t, int position_s, int position_v, int position_u) {
		return(mPixelData[LinearPosition(position_t, position_s, position_v, position_u)]);
	}

	void SetPixel(block4DElementType pixel_value, int position_t, int position_s, int position_v, int position_u) {
		mPixelData[LinearPosition(position_t, position_s, position_v, position_u)] = pixel_value;
	}

	long int LinearPosition(long int position_t, long int position_s, long int position_v, long int position_u) {
		long int linear_position = position_t*mlength_u*mlength_v*mlength_s;
		linear_position += position_s*mlength_u*mlength_v + position_v*mlength_u + position_u;
		return(linear_position);
	}
    void Print(char *outputFileName);
};

#endif