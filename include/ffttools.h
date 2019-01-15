/*
 * ffttools.h
 *
 *  Created on: 2016-11-13
 *      Author: ZQ
 */
#ifndef FFTTOOLS_H_
#define FFTTOOLS_H_

#define max(a,b)((a)>(b)?(a):(b))
#define min(a,b)((a)>(b)?(b):(a))

void copyMakeBorder(IplImage*soure, IplImage *dst,int top,int bottom, int left, int right);
void complexDivision(Complex* a, Complex* b,Complex* c);
void complexMultiplication(Complex* a, Complex* b,Complex* c);
void fftd(Complex *src,int sizey, int sizex,Complex *dst,int backwards);
//void ifftd(Complex *src,int sizey, int sizex,Complex *dst);
void rearrange(Complex *img,int sizey, int sizex);
void subwindow( IplImage* in,  Rect_int  window, IplImage *res);


#endif /* FFTTOOLS_H_ */
