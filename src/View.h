#include "Block4D.h"
#include <stdio.h>

#ifndef VIEW_H
#define VIEW_H

#define PixelType unsigned short int 
#define DefaultPixelValue 0

class View
{
public:
    FILE *mViewFilePointer;                     /*!< pointer to view file */
    PixelType *mPixelData;                      /*!< pointer to the circular array of pixels for the line cache */
    int mNumberCacheLines;                      /*!< maximum number of view lines in the cache */
    int mFirstPixelPosition;                    /*!< position of the first pixel of the first loaded line in the circular cache */
    int mLines;                                 /*!< view number of lines */
    int mColumns;                               /*!< view number of columns */
    int mLastLine;                              /*!< last view line index currently loaded in the cache */
    int mFirstLine;                             /*!< first view line index currently loaded in the cache */
    int mNumberOfPixelsInCache;                 /*!< total number of valid pixels in the cache */
    char mReadOrWriteView;                      /*!< write or read view type flag */
    int mNumberOfFileBytesPerPixelComponent;    /*!< pixel precision in bytes */
    int mNumberOfBands;                         /*!< number of bands of a multispectral image */
    int mPGMScale;                              /*!< scale of the PGM view file */
    long int mPGMDataOffset;                    /*!< PGM header size in bytes */
    char mUnavailable;                          /*!< if set the view contains no data */
    char mVerbosity;                            /*!< indicates the verbosity level for console messages */
    View(void);
    ~View(void);
    void SetCacheSize(int maxBlockLines);
    void OpenViewFilePGM(char *name, char readOrWriteView);
    int OpenViewFilePPM(char *name, char readOrWriteView);
    void CloseViewFile(void);
    PixelType GetPixel(int position_v, int position_u, int component=0);
    void PutPixel(PixelType pixelValue, int position_v, int position_u, int component=0);
    void GetBlock2D(block4DElementType **pixel, int position_v, int position_u, int length_v, int length_u, int component=0);
    void PutBlock2D(block4DElementType **pixelValue, int position_v, int position_u, int length_v, int length_u, int component=0);
    void ReadPixelFromFile(int pixelPositionInCache);
    void WritePixelToFile(int pixelPositionInCache);    
 };

#endif