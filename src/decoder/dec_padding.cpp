#include "LightField.h"
#include "Hierarchical4DDecoder.h"
//#include "Matrix.h"
//#include "MultiscaleTransform.h"
#include "PartitionDecoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

enum ExtensionType {ZERO_PADDING, REPEAT_LAST, CYCLIC, EXTENDED_IDCT, TRANSFORM_DOMAIN_ZERO_PADDING, MEAN_VALUE, NONE};
void ExtendIDCT(Matrix &extendedIDCT, ExtensionType extensionMethod, int transformLength, int extensionLength);
void ExtendBlock4D(Block4D &extendedblock, ExtensionType extensionMethod, int extensionLength, char direction);
void YCbCr2RGB_BT601(Block4D &R, Block4D &G, Block4D &B, Block4D const &Y, Block4D const &Cb, Block4D const &Cr, int Scale);
void YCoCg2RGB(Block4D &R, Block4D &G, Block4D &B, Block4D const &Y, Block4D const &Co, Block4D const &Cg, int Scale);
unsigned long int BigEndianUnsignedIntegerRead(int precision, FILE *inputFilePointer);

class DecoderParameters {
public:
    int firstHorizontalViewNumber;
    int firstVerticalViewNumber;
    char isLenslet13x13;
    char outputLightFieldDirectory[1024];
    char inputFileName[1024];
    char configurationFileName[1024];        
    ExtensionType extensionMethod;    
    double transform_scale_t;
    double transform_scale_s;
    double transform_scale_v;
    double transform_scale_u;    
    int colorTransformType;    
    int verbosity;
    void ReadConfigurationFile(char *parametersFileName); 
    void DisplayConfiguration(void);
};

void DecoderParameters :: ReadConfigurationFile(char *parametersFileName) {
    char command[128];
    FILE *parametersFilepointer;
    
    if((parametersFilepointer = fopen(parametersFileName, "r")) == NULL) {
        printf("ERROR: unable to open configuration file %s\n", parametersFileName);
        exit(0);
    } 
    
    fscanf(parametersFilepointer, "%s", command);
    while(feof(parametersFilepointer) == 0) {
        if(strcmp(command, "-lf") == 0) fscanf(parametersFilepointer, "%s", &outputLightFieldDirectory);
        if(strcmp(command, "-i") == 0) fscanf(parametersFilepointer, "%s", &inputFileName);
        if(strcmp(command, "-off_h") == 0) fscanf(parametersFilepointer, "%d", &firstHorizontalViewNumber);
        if(strcmp(command, "-off_v") == 0) fscanf(parametersFilepointer, "%d", &firstVerticalViewNumber);
        if(strcmp(command, "-lenslet13x13") == 0) isLenslet13x13 = 1;
        if(strcmp(command, "-extension_dct_zero") == 0) extensionMethod = TRANSFORM_DOMAIN_ZERO_PADDING;        
        if(strcmp(command, "-t_scale") == 0) fscanf(parametersFilepointer, "%lf", &transform_scale_t);        
        if(strcmp(command, "-s_scale") == 0) fscanf(parametersFilepointer, "%lf", &transform_scale_s);        
        if(strcmp(command, "-v_scale") == 0) fscanf(parametersFilepointer, "%lf", &transform_scale_v);        
        if(strcmp(command, "-u_scale") == 0) fscanf(parametersFilepointer, "%lf", &transform_scale_u);        
        if(strcmp(command, "-v0") == 0) verbosity = 0;        
        if(strcmp(command, "-bt601") == 0) colorTransformType = 0;        
        if(strcmp(command, "-ycocg") == 0) colorTransformType = 1;        
        fscanf(parametersFilepointer, "%s", command);
    }
}
void DecoderParameters :: DisplayConfiguration(void) {
    printf("firstHorizontalViewNumber = %d\n", firstHorizontalViewNumber);
    printf("firstVerticalViewNumber = %d\n", firstVerticalViewNumber);
    printf("isLenslet13x13 = %d\n", isLenslet13x13);
    printf("outputLightFieldDirectory = %s\n", outputLightFieldDirectory);
    printf("inputFileName = %s\n", inputFileName);
    printf("configurationFileName = %s\n", configurationFileName);        
    printf("extensionMethod = %d\n", extensionMethod);    
    printf("transform_scale_t = %f\n", transform_scale_t);
    printf("transform_scale_s = %f\n", transform_scale_s);
    printf("transform_scale_v = %f\n", transform_scale_v);
    printf("transform_scale_u = %f\n", transform_scale_u);    
    printf("colorTransformType = %d\n", colorTransformType);    
    printf("verbosity = %d\n", verbosity);    
}

int main(int argc, char **argv) {
    
    DecoderParameters par;
        
    par.firstHorizontalViewNumber=0;
    par.firstVerticalViewNumber=0;
    
    strcpy(par.inputFileName, "out.comp");
    strcpy(par.outputLightFieldDirectory, "./");
    strcpy(par.configurationFileName, "");
    
    par.isLenslet13x13=0;
    
    int numberOfCacheViewLines=434;
    
    par.extensionMethod=NONE;
    
    par.transform_scale_t=1.0;
    par.transform_scale_s=1.0;
    par.transform_scale_v=1.0;
    par.transform_scale_u=1.0;
    
    par.colorTransformType = 0;

    par.verbosity=1;
    
    for(int n = 0; n < argc; n++) {
        if(strcmp(argv[n], "-cf") == 0) {
            strcpy(par.configurationFileName, argv[n+1]);
            par.ReadConfigurationFile(par.configurationFileName);
        }
    }
    for(int n = 0; n < argc; n++) {
        if(strcmp(argv[n], "-lf") == 0) strcpy(par.outputLightFieldDirectory, argv[n+1]);
        if(strcmp(argv[n], "-i") == 0) strcpy(par.inputFileName, argv[n+1]);        
        if(strcmp(argv[n], "-off_h") == 0) par.firstHorizontalViewNumber = atoi(argv[n+1]);
        if(strcmp(argv[n], "-off_v") == 0) par.firstVerticalViewNumber = atoi(argv[n+1]);
        if(strcmp(argv[n], "-lenslet13x13") == 0) par.isLenslet13x13 = 1;
        if(strcmp(argv[n], "-extension_dct_zero") == 0) par.extensionMethod = TRANSFORM_DOMAIN_ZERO_PADDING;        
        if(strcmp(argv[n], "-t_scale") == 0) par.transform_scale_t = atof(argv[n+1]);        
        if(strcmp(argv[n], "-s_scale") == 0) par.transform_scale_s = atof(argv[n+1]);        
        if(strcmp(argv[n], "-v_scale") == 0) par.transform_scale_v = atof(argv[n+1]);        
        if(strcmp(argv[n], "-u_scale") == 0) par.transform_scale_u = atof(argv[n+1]);        
        if(strcmp(argv[n], "-bt601") == 0) par.colorTransformType = 0;        
        if(strcmp(argv[n], "-ycocg") == 0) par.colorTransformType = 1;        
        if(strcmp(argv[n], "-v0") == 0) par.verbosity = 0;        
    }
  
    if(par.verbosity > 0) par.DisplayConfiguration();
    
    Hierarchical4DDecoder hdt;

    FILE *inputFileNamePointer;
    if((inputFileNamePointer = fopen(par.inputFileName, "rb")) == NULL) {
        printf("Error: input file %s not found\n", par.inputFileName);
        exit(0);
    }
    
    //reads the superior bit plane value
    hdt.mSuperiorBitPlane = BigEndianUnsignedIntegerRead(2, inputFileNamePointer);
    
    //reads the maximum transform sizes
    int transformLength_t = BigEndianUnsignedIntegerRead(2, inputFileNamePointer);
    int transformLength_s = BigEndianUnsignedIntegerRead(2, inputFileNamePointer);
    int transformLength_v = BigEndianUnsignedIntegerRead(2, inputFileNamePointer);
    int transformLength_u = BigEndianUnsignedIntegerRead(2, inputFileNamePointer);
    
    //reads the number of views of the lightfield
    int NumberOfVerticalViews = BigEndianUnsignedIntegerRead(2, inputFileNamePointer);
    int NumberOfHorizontalViews = BigEndianUnsignedIntegerRead(2, inputFileNamePointer);
    
    //reads the number of lines and columns of each view
    int NumberOfViewLines = BigEndianUnsignedIntegerRead(2, inputFileNamePointer);
    int NumberOfViewColumns = BigEndianUnsignedIntegerRead(2, inputFileNamePointer);
    
    //reads the bit precision of each component of the pixels of the views
    int PGMScale = BigEndianUnsignedIntegerRead(2, inputFileNamePointer);
    
    //reads the maximum slant
    int maximumSlant = BigEndianUnsignedIntegerRead(2, inputFileNamePointer);
    
    hdt.StartDecoder(inputFileNamePointer);
    
    numberOfCacheViewLines = transformLength_v+maximumSlant;
    LightField outputLF(NumberOfVerticalViews,NumberOfHorizontalViews,numberOfCacheViewLines);

    outputLF.mPGMScale = PGMScale;
    outputLF.mNumberOfViewColumns = NumberOfViewColumns+maximumSlant*NumberOfViewColumns/transformLength_u;
    outputLF.mNumberOfViewLines = NumberOfViewLines+maximumSlant*NumberOfViewLines/transformLength_v;
    outputLF.mVerticalViewNumberOffset = par.firstVerticalViewNumber;
    outputLF.mHorizontalViewNumberOffset = par.firstHorizontalViewNumber;
    outputLF.mNumberOfVerticalViews = NumberOfVerticalViews;
    outputLF.mNumberOfHorizontalViews = NumberOfHorizontalViews;
    
    Block4D lfBlock, rBlock, gBlock, bBlock, yBlock, cbBlock, crBlock;
    lfBlock.SetDimension(transformLength_t,transformLength_s,transformLength_v+maximumSlant,transformLength_u+maximumSlant);
    rBlock.SetDimension(transformLength_t,transformLength_s,transformLength_v+maximumSlant,transformLength_u+maximumSlant);
    gBlock.SetDimension(transformLength_t,transformLength_s,transformLength_v+maximumSlant,transformLength_u+maximumSlant);
    bBlock.SetDimension(transformLength_t,transformLength_s,transformLength_v+maximumSlant,transformLength_u+maximumSlant);
    yBlock.SetDimension(transformLength_t,transformLength_s,transformLength_v+maximumSlant,transformLength_u+maximumSlant);
    cbBlock.SetDimension(transformLength_t,transformLength_s,transformLength_v+maximumSlant,transformLength_u+maximumSlant);
    crBlock.SetDimension(transformLength_t,transformLength_s,transformLength_v+maximumSlant,transformLength_u+maximumSlant);
   
    //int subbandsTotalNumberOfVerticalViews = (outputLF.mNumberOfVerticalViews%transformLength_t == 0) ? outputLF.mNumberOfVerticalViews : transformLength_t*(NumberOfVerticalViews/transformLength_t) + transformLength_t;
    //int subbandsTotalNumberOfHorizontalViews = (outputLF.mNumberOfHorizontalViews%transformLength_s == 0) ? outputLF.mNumberOfHorizontalViews : transformLength_s*(NumberOfHorizontalViews/transformLength_s) + transformLength_s;

    //if(transformLength_t > outputLF.mNumberOfVerticalViews) subbandsTotalNumberOfVerticalViews = transformLength_t;
    //if(transformLength_s > outputLF.mNumberOfHorizontalViews) subbandsTotalNumberOfHorizontalViews = transformLength_s;

    //int subbandsTotalNumberOfViewLines = (outputLF.mNumberOfViewLines%transformLength_v == 0) ? outputLF.mNumberOfViewLines : transformLength_v*(outputLF.mNumberOfViewLines/transformLength_v) + transformLength_v;
    //int subbandsTotalNumberOfViewColumns = (outputLF.mNumberOfViewColumns%transformLength_u == 0) ? outputLF.mNumberOfViewColumns : transformLength_u*(outputLF.mNumberOfViewColumns/transformLength_u) + transformLength_u;

        
    outputLF.OpenLightFieldPPM(par.outputLightFieldDirectory, ".ppm", outputLF.mNumberOfVerticalViews, outputLF.mNumberOfHorizontalViews, 3, 3, 'w');

    MultiscaleTransform IDCTarray;
    
    IDCTarray.SetDimension(transformLength_t, transformLength_s, transformLength_v+maximumSlant, transformLength_u+maximumSlant);
    IDCTarray.mTransformGain_t = par.transform_scale_t;
    IDCTarray.mTransformGain_s = par.transform_scale_s;
    IDCTarray.mTransformGain_v = par.transform_scale_v;
    IDCTarray.mTransformGain_u = par.transform_scale_u;

    int extensionLength_t = outputLF.mNumberOfVerticalViews%transformLength_t;
    int extensionLength_s = outputLF.mNumberOfHorizontalViews%transformLength_s;
    int extensionLength_v = outputLF.mNumberOfViewLines%transformLength_v;
    int extensionLength_u = outputLF.mNumberOfViewColumns%transformLength_u;
    
    PartitionDecoder pd;
    //pd.mPartitionData.SetDimension(transformLength_t,transformLength_s,transformLength_v+maximumSlant,transformLength_u+maximumSlant);
   
    int optimumSlant;
    for(int verticalView = 0; verticalView < outputLF.mNumberOfVerticalViews; verticalView += transformLength_t) {
        for(int horizontalView = 0; horizontalView < outputLF.mNumberOfHorizontalViews; horizontalView += transformLength_s) {
            for(int viewLine = 0; viewLine < outputLF.mNumberOfViewLines; viewLine += transformLength_v+maximumSlant) {
                for(int viewColumn = 0; viewColumn < outputLF.mNumberOfViewColumns; viewColumn += transformLength_u+maximumSlant) {
                    for(int spectralComponent = 0; spectralComponent < 3; spectralComponent++) {
                        printf("\nProcessing spectral component %d\n", spectralComponent);
                        if(par.verbosity > 0) 
                            printf("transforming the 4D block at position (%d %d %d %d)\n", verticalView, horizontalView, viewLine, viewColumn);
			
                        
                        //reads the optimum slant
                        optimumSlant = hdt.DecodeInteger(8);
                        optimumSlant -= 128;
			
printf("optimumSlant = %d\n", optimumSlant);
			
			pd.mPartitionData.SetDimension(transformLength_t,transformLength_s,transformLength_v+abs(optimumSlant),transformLength_u+abs(optimumSlant));
			pd.mPartitionData.Zeros();
                        hdt.RestartProbabilisticModel();
                        pd.DecodePartition(hdt, IDCTarray, 1);
			
                        lfBlock.Zeros();			
                        //lfBlock.DeSlanted(optimumSlant, optimumSlant, pd.mPartitionData);
                        lfBlock.CopySubblockFrom(pd.mPartitionData,0,0,0,0);

                        if(viewColumn + transformLength_u > outputLF.mNumberOfViewColumns)
                            ExtendBlock4D(lfBlock, par.extensionMethod, extensionLength_u, 'u');                                   
                        if(viewLine + transformLength_v > outputLF.mNumberOfViewLines) 
                            ExtendBlock4D(lfBlock, par.extensionMethod, extensionLength_v, 'v');                                  
                        if(horizontalView + transformLength_s > outputLF.mNumberOfHorizontalViews)
                            ExtendBlock4D(lfBlock, par.extensionMethod, extensionLength_s, 's');                                  
                        if(verticalView + transformLength_t > outputLF.mNumberOfVerticalViews)
                            ExtendBlock4D(lfBlock, par.extensionMethod, extensionLength_t, 't');                                  
                                    

                        lfBlock = lfBlock + (outputLF.mPGMScale+1)/2;
                                                                        
                        //lfBlock.Clip(0, outputLF.mPGMScale);
                        if(spectralComponent == 0)
                            yBlock.CopySubblockFrom(lfBlock, 0, 0, 0, 0);
                        if(spectralComponent == 1)
                            cbBlock.CopySubblockFrom(lfBlock, 0, 0, 0, 0);
                        if(spectralComponent == 2)
                            crBlock.CopySubblockFrom(lfBlock, 0, 0, 0, 0);
                    }
                    if(par.colorTransformType == 0)
                        YCbCr2RGB_BT601(rBlock, gBlock, bBlock, yBlock, cbBlock, crBlock, outputLF.mPGMScale);
                    if(par.colorTransformType == 1)
                        YCoCg2RGB(rBlock, gBlock, bBlock, yBlock, cbBlock, crBlock, outputLF.mPGMScale);
                    if(par.isLenslet13x13 == 1) {
                        if(verticalView == 0) {
                            if(horizontalView == 0) {
                                rBlock.Shift_UVPlane(-2, 0, 0);
                                gBlock.Shift_UVPlane(-2, 0, 0);
                                bBlock.Shift_UVPlane(-2, 0, 0);
                            }
                            if((horizontalView + transformLength_s >= outputLF.mNumberOfHorizontalViews)&&(horizontalView <= outputLF.mNumberOfHorizontalViews)) { 
                                rBlock.Shift_UVPlane(-2, 0, outputLF.mNumberOfHorizontalViews-horizontalView-1);
                                gBlock.Shift_UVPlane(-2, 0, outputLF.mNumberOfHorizontalViews-horizontalView-1);
                                bBlock.Shift_UVPlane(-2, 0, outputLF.mNumberOfHorizontalViews-horizontalView-1);
                            }
                        }
                        if((verticalView + transformLength_t >= outputLF.mNumberOfVerticalViews)&&(verticalView <= outputLF.mNumberOfVerticalViews)) {
                            if(horizontalView == 0) {
                                rBlock.Shift_UVPlane(-2, outputLF.mNumberOfVerticalViews-verticalView-1, 0);
                                gBlock.Shift_UVPlane(-2, outputLF.mNumberOfVerticalViews-verticalView-1, 0);
                                bBlock.Shift_UVPlane(-2, outputLF.mNumberOfVerticalViews-verticalView-1, 0);
                            }
                            if((horizontalView + transformLength_s >= outputLF.mNumberOfHorizontalViews)&&(horizontalView <= outputLF.mNumberOfHorizontalViews)) {
                                rBlock.Shift_UVPlane(-2, outputLF.mNumberOfVerticalViews-verticalView-1, outputLF.mNumberOfHorizontalViews-horizontalView-1);
                                gBlock.Shift_UVPlane(-2, outputLF.mNumberOfVerticalViews-verticalView-1, outputLF.mNumberOfHorizontalViews-horizontalView-1);
                                bBlock.Shift_UVPlane(-2, outputLF.mNumberOfVerticalViews-verticalView-1, outputLF.mNumberOfHorizontalViews-horizontalView-1);
                            }
                        }
                    }
    
                    rBlock.Clip(0, outputLF.mPGMScale);
                    gBlock.Clip(0, outputLF.mPGMScale);
                    bBlock.Clip(0, outputLF.mPGMScale);
                    outputLF.WriteBlock4DtoLightField(&bBlock, verticalView, horizontalView, viewLine, viewColumn, 2);
                    outputLF.WriteBlock4DtoLightField(&gBlock, verticalView, horizontalView, viewLine, viewColumn, 1);
                    outputLF.WriteBlock4DtoLightField(&rBlock, verticalView, horizontalView, viewLine, viewColumn, 0);
                    //outputLF.WriteBlock4DtoLightField(&bBlock, verticalView, horizontalView, viewLine+maximumSlant+optimumSlant, viewColumn+maximumSlant+optimumSlant, 2);
                    //outputLF.WriteBlock4DtoLightField(&gBlock, verticalView, horizontalView, viewLine+maximumSlant+optimumSlant, viewColumn+maximumSlant+optimumSlant, 1);
                    //outputLF.WriteBlock4DtoLightField(&rBlock, verticalView, horizontalView, viewLine+maximumSlant+optimumSlant, viewColumn+maximumSlant+optimumSlant, 0);
                }
            }
        }
    }
    hdt.DoneDecoding();
        
    outputLF.CloseLightField();

    fclose(inputFileNamePointer);
}

void ExtendIDCT(Matrix &extendedIDCT, ExtensionType extensionMethod, int transformLength, int extensionLength) {
    
        extendedIDCT.SetDimension(transformLength,transformLength);
        if(extensionMethod == EXTENDED_IDCT) {
            Matrix C00, C01, C10, C11, INVC11;
            extendedIDCT.IDCT();
            C00.SetDimension(extensionLength,extensionLength);
            C01.SetDimension(extensionLength,transformLength-extensionLength);
            C10.SetDimension(transformLength-extensionLength,extensionLength);
            C11.SetDimension(transformLength-extensionLength,transformLength-extensionLength);
            INVC11.SetDimension(transformLength-extensionLength,transformLength-extensionLength);
            C00.CopyFrom(extendedIDCT);
            C01.CopyFrom(extendedIDCT,0,extensionLength);
            C10.CopyFrom(extendedIDCT,extensionLength,0);
            C11.CopyFrom(extendedIDCT,extensionLength,extensionLength);
            INVC11.Inverse(C11);
            INVC11.Multiply(-1.0);
            C10.PreMultiply(INVC11);    //C10 <- - INVC11 * C10
            C10.PreMultiply(C01);       //C10 <- - C01 * INVC11 * C10
            C00.Add(C10);               //C00 <- C00 - C01 * INVC11 * C10
            extendedIDCT.Zeros();
            extendedIDCT.CopyFrom(C00);
        }
        if(extensionMethod == REPEAT_LAST) {
            extendedIDCT.IDCT();
            extendedIDCT.AccumulateFromColumn(extensionLength-1);
        }
        if(extensionMethod == TRANSFORM_DOMAIN_ZERO_PADDING) {
            Matrix C00;
            C00.SetDimension(extensionLength,extensionLength);
            C00.IDCT();
            C00.Multiply((1.0*transformLength)/extensionLength);
            extendedIDCT.Zeros();
            extendedIDCT.CopyFrom(C00);
        }
        if(extensionMethod == ZERO_PADDING) {
            Matrix C00;
            C00.SetDimension(transformLength,extensionLength);
            extendedIDCT.IDCT();
            C00.CopyFrom(extendedIDCT);
            extendedIDCT.Zeros();
            extendedIDCT.CopyFrom(C00);
        }
        if(extensionMethod == NONE) {
            extendedIDCT.IDCT();
        }
    
}

void ExtendBlock4D(Block4D &extendedBlock, ExtensionType extensionMethod, int extensionLength, char direction) {
    
    if(extensionMethod == REPEAT_LAST) {
        
        if(direction == 't') 
            extendedBlock.Extend_T(extensionLength-1);
        if(direction == 's') 
            extendedBlock.Extend_S(extensionLength-1);
        if(direction == 'v') 
            extendedBlock.Extend_V(extensionLength-1);
        if(direction == 'u') 
            extendedBlock.Extend_U(extensionLength-1);
        
    }
    if(extensionMethod == CYCLIC) {
        
        if(direction == 't') 
            extendedBlock.CopySubblockFrom(extendedBlock, 0, 0, 0, 0, extensionLength, 0, 0, 0);
        if(direction == 's') 
            extendedBlock.CopySubblockFrom(extendedBlock, 0, 0, 0, 0, 0, extensionLength, 0, 0);
        if(direction == 'v') 
            extendedBlock.CopySubblockFrom(extendedBlock, 0, 0, 0, 0, 0, 0, extensionLength, 0);
        if(direction == 'u') 
            extendedBlock.CopySubblockFrom(extendedBlock, 0, 0, 0, 0, 0, 0, 0, extensionLength);
        
    }
    if(extensionMethod == NONE) {
            
    }
}


void YCbCr2RGB_BT601(Block4D &R, Block4D &G, Block4D &B, Block4D const &Y, Block4D const &Cb, Block4D const &Cr, int Scale) {
    
    for(int n = 0; n < R.mlength_t*R.mlength_s*R.mlength_v*R.mlength_u; n++) {
                    double pixel_Y = Y.mPixelData[n];
                    double pixel_Cb = Cb.mPixelData[n]-(Scale+1)/2;
                    double pixel_Cr = Cr.mPixelData[n]-(Scale+1)/2;
                    R.mPixelData[n] = round(pixel_Y - 0.0000071525 * pixel_Cb + 1.4020 * pixel_Cr);
                    G.mPixelData[n] = round(pixel_Y -0.34413 * pixel_Cb - 0.71414 * pixel_Cr);
                    B.mPixelData[n] = round(pixel_Y + 1.7720 * pixel_Cb - 0.000040249 * pixel_Cr);
    }
          
}

void YCoCg2RGB(Block4D &R, Block4D &G, Block4D &B, Block4D const &Y, Block4D const &Co, Block4D const &Cg, int Scale) {
    
    for(int n = 0; n < R.mlength_t*R.mlength_s*R.mlength_v*R.mlength_u; n++) {
        Co.mPixelData[n] = Co.mPixelData[n] - (Scale+1)/2;
        Cg.mPixelData[n] = Cg.mPixelData[n] - (Scale+1)/2;
        int t = Y.mPixelData[n] - (Cg.mPixelData[n]>>1);
        G.mPixelData[n] = Cg.mPixelData[n] + t;
        B.mPixelData[n] = t - (Co.mPixelData[n]>>1);
        R.mPixelData[n] = B.mPixelData[n] + Co.mPixelData[n];
    }
          
}

unsigned long int BigEndianUnsignedIntegerRead(int precision, FILE *inputFilePointer) {

    if(precision > sizeof(long int)) {
        printf("Unsuported %d bytes integer precision\n", precision);
        exit(0);
    }
    
    unsigned char input_byte;
    unsigned long int value = 0;
    for(int byte_index = precision-1; byte_index >= 0; byte_index--) {
        fread(&input_byte, 1, 1, inputFilePointer);
        value = (value << 8);
        value += input_byte;
    }
    return (value);
}
