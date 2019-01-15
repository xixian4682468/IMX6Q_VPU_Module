/*
 * ffttools.c
 *
 *  Created on: 2016-10-21
 *      Author: ZQ
 */
#include "KCF_Track.h"
#include "ffttools.h"
#include "recttools.h"
#include "DFT.h"

extern int size_patch[3];

void  copyMakeBorder(IplImage*soure, IplImage *dst,int top,int bottom, int left, int right)
{
	int i,j;
	uchar *s_current,*d_current;

	//构建上面所有行
	for (i = 0; i<top; i++)
	{
		for (j = 0; j<soure->width + left + right; j++)
		{
			if (j<left) s_current = soure->imageData;
			else if (j<soure->width + left)
				s_current = soure->imageData + (j - left) * 1;
			else s_current = soure->imageData + soure->widthStep - 1;

			d_current = dst->imageData + (i*(dst->widthStep) + j * 1);
			memcpy(d_current, s_current, 1);
		}
	}
	//构建中间部分
	for (i = top; i<top + soure->height; i++)
	{
		for (j = 0; j<soure->width + left + right; j++)
		{
			if (j<left) s_current = soure->imageData + soure->widthStep*(i - top);
			else if (j<soure->width + left)
				s_current = soure->imageData + soure->widthStep*(i - top) + (j - left) * 1;
			else s_current = soure->imageData + soure->widthStep*(i - top + 1) - 1;
			d_current = dst->imageData + (i*(dst->widthStep) + j * 1);
			memcpy(d_current, s_current, 1);
		}
	}
	//构建下面几行
	for (i = top + soure->height; i<top + soure->height + bottom; i++)
	{
		for (j = 0; j<soure->width + left + right; j++)
		{
			if (j<left) s_current = soure->imageData + soure->widthStep*(soure->height - 1);
			else if (j<soure->width + left)
				s_current = soure->imageData + soure->widthStep*(soure->height - 1) + (j - left) * 1;
			else s_current = soure->imageData + soure->widthStep*soure->height - 1;

			d_current = dst->imageData + (i*(dst->widthStep) + j * 1);
			memcpy(d_current, s_current, 1);
		}
	}
}


void subwindow( IplImage* in, Rect_int window, IplImage *res)
{
    Rect_int cutWindow = window;
    limit(&cutWindow, in->width,in->height);

    res->width=cutWindow.width;
    res->height=cutWindow.height;
    res->widthStep=cutWindow.width;
	
	printf("res->width=%d,res->height=%d,res->widthStep=%d\n",res->width,res->height,res->widthStep);

    int i;
    uchar *src=(uchar *)(in->imageData + cutWindow.y*in->width*1 + cutWindow.x*1);
    uchar * dst,*src1;
    for(i=0; i<res->height; i++)
    {
    	src1=src+i*in->widthStep;
    	dst=res->imageData+i*res->widthStep;
    	memcpy(dst,src1,res->widthStep);
    }
}


/************************fft正变换***************************/
void fftd(Complex *src,int sizey, int sizex,Complex *dst,int backwards)
{
	int i,j;
	my_mat my_img1 = { 2, sizey, sizex, \
			sizex * 8, 2, (uchar *)src };
	my_dft(&my_img1, &my_img1, backwards ? (1 | 2) : 0, 0);
}

void complexDivision(Complex* a, Complex* b,Complex* c)
{
	int i,j;
	//float *divisor=(float *)0x0087A5E0;//大小40*40*4:0x1900
	float *divisor=(float *)malloc(40*40*sizeof(float));
	float sum1=0.0,sum2=0.0,sum3=0.0;

    for ( i = 0; i<size_patch[0]; i++)
           {
        	  for ( j = 0; j<size_patch[1]; j++)
        		{
//        		  sum1=0.0;

        		  sum2=(b+i*size_patch[1]+j)->real*(b+i*size_patch[1]+j)->real;
        		  sum3=(b+i*size_patch[1]+j)->imagin*(b+i*size_patch[1]+j)->imagin;
        		  sum1=sum2+sum3;
        		  divisor[i*size_patch[1]+j]=1.0/(sum1);
        		}
          }
    for ( i = 0; i<size_patch[0]; i++)
            {
         	  for ( j = 0; j<size_patch[1]; j++)
         		{
//        		  sum2=0.0;

         		 sum2=(a+i*size_patch[1]+j)->real*(b+i*size_patch[1]+j)->real;
         		 sum3=(a+i*size_patch[1]+j)->imagin*(b+i*size_patch[1]+j)->imagin;
        		  sum1=sum2+sum3;
                  (c+i*size_patch[1]+j)->real=sum1*divisor[i*size_patch[1]+j];
        		}
          }
    for ( i = 0; i<size_patch[0]; i++)
            {
         	  for ( j = 0; j<size_patch[1]; j++)
         		{
//          		  sum3=0.0;
         		 sum2=(a+i*size_patch[1]+j)->imagin*(b+i*size_patch[1]+j)->real;
         		 sum3=(a+i*size_patch[1]+j)->real*(b+i*size_patch[1]+j)->imagin;
        		  sum1=sum2+sum3;
                  (c+i*size_patch[1]+j)->imagin=sum1*divisor[i*size_patch[1]+j];
        		}
          }
	
	free(divisor);
}
void complexMultiplication(Complex* a, Complex* b,Complex* c)
{

    int i,j;
    float temp1=0.0,temp2=0.0,temp3=0.0,temp4=0.0;
	for ( i = 0; i<size_patch[0]; i++)
              {
           	  for ( j = 0; j<size_patch[1]; j++)
           		{
           		  temp1=(a+i*size_patch[1]+j)->real*(b+i*size_patch[1]+j)->real;
           		  temp2=(a+i*size_patch[1]+j)->imagin*(b+i*size_patch[1]+j)->imagin;
           		  (c+i*size_patch[1]+j)->real=temp1-temp2;
//                     *(a+i*size_patch[1]+j)->real*=divisor[i*size_patch[1]+j];
           		}
             }
	for ( i = 0; i<size_patch[0]; i++)
              {
				for ( j = 0; j<size_patch[1]; j++)
				 {
           		  temp3=(a+i*size_patch[1]+j)->real*(b+i*size_patch[1]+j)->imagin;
           		  temp4=(a+i*size_patch[1]+j)->imagin*(b+i*size_patch[1]+j)->real;
           		  (c+i*size_patch[1]+j)->imagin=temp3+temp4;
//                     *(a+i*size_patch[1]+j)->imagin*=divisor[i*size_patch[1]+j];
           		}
             }


}

//交换1-3象限   2-4象限数据
void rearrange(Complex *img,int sizey, int sizex)
{
    int cx = sizex >> 1;
    int cy = sizey >> 1;
    int i,j;

   // float *tmp1=(float *)0x00874500;//大小cx*cy*4:20*20*4:0x640
    float *tmp1=(float *)malloc(20*20*sizeof(float));
    //float *tmp2=(float *)0x00874B40;//大小cx*cy*4:20*20*4:0x640
    float *tmp2=(float *)malloc(20*20*sizeof(float));
    //对一象限作备份
    for(i=0;i<cy;i++)
    {
    	for(j=0;j<cx;j++)
    	{
    		tmp1[i*cx+j]=(img+i*sizex+j)->real;
    		tmp2[i*cx+j]=(img+i*sizex+j)->imagin;
//    	}
//    }
//把4象限复制到1象限
//    for(i=0;i<cy;i++)
//       {
//       	for(j=0;j<cx;j++)
//       	{
       		(img+i*sizex+j)->real=(img+cy*sizex+cx+i*sizex+j)->real;
       		(img+i*sizex+j)->imagin=(img+cy*sizex+cx+i*sizex+j)->imagin;
//       	}
//       }
    //把备份复制到4象限
//        for(i=0;i<cy;i++)
//           {
//           	for(j=0;j<cx;j++)
//           	{
           		(img+cy*sizex+cx+i*sizex+j)->real=tmp1[i*cx+j];
           		(img+cy*sizex+cx+i*sizex+j)->imagin=tmp2[i*cx+j];
//           	}
//           }
////对2象限做备份
//        for(i=0;i<cy;i++)
//            {
//            	for(j=0;j<cx;j++)
//            	{
           		    tmp1[i*cx+j]=(img+cx+i*sizex+j)->real;
           		    tmp2[i*cx+j]=(img+cx+i*sizex+j)->imagin;
//            	}
//            }
//
//        //把3象限复制到2象限
//            for(i=0;i<cy;i++)
//               {
//               	for(j=0;j<cx;j++)
//               	{
            		(img+cx+i*sizex+j)->real=(img+cy*sizex+i*sizex+j)->real;
            		(img+cx+i*sizex+j)->imagin=(img+cy*sizex+i*sizex+j)->imagin;
//               	}
//               }
//            //把备份复制到3象限
//             for(i=0;i<cy;i++)
//                  {
//                   	for(j=0;j<cx;j++)
//                     {
                   		(img+cy*sizex+i*sizex+j)->real=tmp1[i*cx+j];
                    	(img+cy*sizex+i*sizex+j)->imagin=tmp2[i*cx+j];
                     }
                  }

	free(tmp1);
	free(tmp2);
}





