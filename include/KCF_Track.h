/*
 * kcftracker.h
 *
 *  Created on: 2016-11-13
 *      Author: ZQ
 */

#ifndef KCFTRACKER_H_
#define KCFTRACKER_H_

#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "float.h"
#include "string.h"

#define LATENT_SVM_OK 0
#define LATENT_SVM_MEM_NULL 2
#define uchar unsigned char
#define NULL  0
#define NUM_SECTOR 2    // The number of sectors in gradient histogram building
#define true  1
#define false 0
#define PI   3.1415926535897932384626433832795
#define SEARCH           0x00
#define TRACK            0xff
#define SMALLSTATE       0x00
#define READY    0x58


typedef struct
{
     uchar  *imageData;
     float *imageData_f;
     int   width;
     int   height;
     int   widthStep;
}IplImage;

typedef struct
{
	float real;
	float imagin;
}Complex;

typedef struct
{
     int sizeX;
     int sizeY;
     int numFeatures;
     float *map;
}CvLSVMFeatureMapCaskade;

typedef struct
{
     int x;
     int y;
     int width;
     int height;
}Rect;

typedef struct
{
     int x;
     int y;
     int width;
     int height;
}Rect_int;

typedef struct
{
	int x;
	int y;
}Point;

typedef struct
{
     float  x;
     float  y;
}Point_f;

typedef struct
{
    int width;
    int height;
}Size;

void tracker_ini(uchar hog, uchar fixed_window, uchar multiscale, uchar lab);
void init(IplImage* image,Rect *_roi);
void update(IplImage* image, Rect_int* result, Rect* _roi);
void createHanningMats();
void detect(float* z, float* x, float* peak_value, Point_f *p);
void gaussianCorrelation(const float* x1, const float* x2, Complex *k);
void getFeatures(IplImage*  image, unsigned char inithann, float* dst, float scale_adjust,Rect *_roi);
void MaxLoc(Complex *res, float *pv, Point *pi);
void mulSpectrums(Complex* srcA, Complex* srcB, Complex* dst);
void resize(IplImage* imgeSrc,IplImage* imgeDst);
float subPixelPeak(float left, float center, float right);
void train(float * x, float train_interp_factor,Complex *_prob,Complex *_alphaf);
void linearCorrelation(const float* x1, const float* x2, Complex *k);

void KCF_Parameter_Init(void);
void KCF_Parameter_UnInit(void);


#endif /* KCFTRACKER_H_ */
