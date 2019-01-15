/*
 * kcftracker.c
 *
 *  Created on: 2016-10-21
 *      Author: ZQ
 */
 
#include "KCF_Track.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ffttools.h"
#include "recttools.h"
#include "fhog.h"

//extern msm_variable *msm;
Size _tmpl_sz;//
float interp_factor; // linear interpolation factor for adaptation
float sigma; // gaussian kernel bandwidth
float lambda; // regularization
int cell_size; // HOG cell size
int cell_sizeQ; // cell size^2, to avoid repeated operations
float padding; // extra area surrounding the target
float output_sigma_factor; // bandwidth of gaussian target
int template_size; // template size
float scale_step; // scale step for multi-scale estimation
float scale_weight;  // to downweight detection scores of other scales for added stability
int size_xy;
Complex *_prob;//
float *_tmpl;//
float *_tmpl_big;
float *_tmpl_small;
Complex *_alphaf;
//Complex *_alphaf_big;
//Complex *_alphaf_small;
float _scale;
int _gaussian_size;
unsigned char _hogfeatures;
unsigned char _labfeatures;
int size_patch[3];

/*
float *hann1t=(float *)0x84040000;//大小size_patch[0]*4:40*4:0xA0
float *hann2t=(float *)0x84041000;//大小size_patch[1]*4:40*4:0xA0
float *hann2d=(float *)0x84042000;//大小size_patch[0]*size_patch[1]*4:30*30*4:0xE10

Complex  *_src_data=(Complex  *)0x0C0370A0;//大小size_patch[0]*size_patch[1]*4*2:40*40*4*2:0x1C20
Complex  *_prob_data=(Complex  *)0x0C0370A0;//大小size_patch[0]*size_patch[1]*4*2:30*30*4*2:0x1C20
Complex  *_alphaf_data=(Complex  *)0x00889040;//大小size_patch[0]*size_patch[1]*4*2:40*40*4*2:0x3200

float  *hann=(float *)0x008ADEE0;//大小：30*30*19*4:0x10B30;
float  *_tmpl_data=(float *)0x0089D3B0;  //大小：30*30*19*4:0x10B30;
float  *_tmpl_data1=(float *)0x0088C880;//大小：30*30*19*4:0x10B30;
float  *x_data=(float *)0x0088C880;//大小：40*40*31*4:0x30700;
*/
float *hann1t;//大小size_patch[0]*4:40*4:0xA0
float *hann2t;//大小size_patch[1]*4:40*4:0xA0
float *hann2d;//大小size_patch[0]*size_patch[1]*4:30*30*4:0xE10

Complex  *_src_data;//大小size_patch[0]*size_patch[1]*4*2:40*40*4*2:0x1C20
Complex  *_prob_data;//大小size_patch[0]*size_patch[1]*4*2:30*30*4*2:0x1C20
Complex  *_alphaf_data;//大小size_patch[0]*size_patch[1]*4*2:40*40*4*2:0x3200

float  *hann;//大小：30*30*19*4:0x10B30;
float  *_tmpl_data; //大小：30*30*19*4:0x10B30;
float  *_tmpl_data1;//大小：30*30*19*4:0x10B30;
float  *x_data;//大小：40*40*31*4:0x30700;

void KCF_Parameter_Init(void)
{
	hann1t = (float *)malloc(40*sizeof(float));
	hann2t = (float *)malloc(40*sizeof(float));
	hann2d = (float *)malloc(30*30*sizeof(float));
	_src_data = (Complex  *)malloc(40*40*sizeof(Complex));
	_prob_data = (Complex  *)malloc(30*30*sizeof(Complex));
	_alphaf_data = (Complex  *)malloc(40*40*sizeof(Complex));
	hann = (float *)malloc(30*30*19*sizeof(float));
	_tmpl_data = (float *)malloc(30*30*19*sizeof(float));
	_tmpl_data1 = (float *)malloc(30*30*19*sizeof(float));
	x_data = (float *)malloc(40*40*31*sizeof(float));
	
}

void KCF_Parameter_UnInit(void)
{
	free(hann1t);
	free(hann2t);
	free(hann2d);
	free(_src_data);
	free(_prob_data);
	free(_alphaf_data);
	free(hann);
	free(_tmpl_data);
	free(_tmpl_data1);
	free(x_data);
}

void tracker_ini(uchar hog, uchar fixed_window, uchar multiscale, uchar lab)
{
	lambda = 0.0001; // Parameters equal in all cases
    padding = 2.5;
    output_sigma_factor = 0.125;

    if (hog)  // HOG
	{   
        // VOT
        interp_factor = 0.012;
        sigma = 0.6;
        cell_size = 4;
        _hogfeatures = true;

        if (lab) 
		{
            interp_factor = 0.005;
            sigma = 0.4;
            //output_sigma_factor = 0.025;
            output_sigma_factor = 0.1;

            _labfeatures = true;
            cell_sizeQ = cell_size*cell_size;
        }
        else
            _labfeatures = false;
    }
    else 
	{   // RAW
        interp_factor = 0.075;
        sigma = 0.2;
        cell_size = 1;
        _hogfeatures = false;

        if (lab) 
		{
            printf("Lab features are only used with HOG features.\n");
            _labfeatures = false;
        }
    }

    if (multiscale)
	{ // multiscale
        template_size = 96;
        scale_step = 1.1;//1.05;
        scale_weight = 0.95;
        if (!fixed_window) 
            fixed_window = true;
    }
    else if (fixed_window) 
	{  // fit correction without multiscale
        template_size = 96;
        //template_size = 100;
        scale_step = 1;
    }
    else 
	{
        template_size = 1;
        scale_step = 1;
    }
	
}

void createHanningMats()
{
	int i,j;
    for ( i = 0; i < size_patch[1]; i++)
        hann1t[i] = 0.5 * (1 - cos(2 * 3.14159265358979323846 * i / (size_patch[1] - 1)));
    for ( i = 0; i < size_patch[0]; i++)
        hann2t[i] = 0.5 * (1 - cos(2 * 3.14159265358979323846 * i / (size_patch[0] - 1)));

    for(i=0;i<size_patch[0];i++)
    	for(j=0;j<size_patch[1];j++)
    		hann2d[i*size_patch[1]+j]=hann1t[j]*hann2t[i];
   		
    // HOG features
    if (_hogfeatures) 	
        for ( i = 0; i < size_patch[2]; i++) 	
            for ( j = 0; j<size_patch[0]*size_patch[1]; j++) 		
                hann[i*size_patch[0]*size_patch[1]+j] = hann2d[j]; 

}

void resize(IplImage* imgeSrc,IplImage* imgeDst)
{
	int i,j,k;
	float scale_x = (float)imgeSrc->width / imgeDst->width;
	float scale_y = (float)imgeSrc->height / imgeDst->height;
	int iWidthSrc = imgeSrc->width;
	int iHiehgtSrc =imgeSrc->height;

	uchar* data_Dst= (uchar *)imgeDst->imageData;
	int stepDst = imgeDst->widthStep;
	uchar* dataSrc = (uchar *)imgeSrc->imageData;
	int stepSrc = imgeSrc->widthStep;

	for ( j = 0; j < imgeDst->height; ++j)
	{
		float fy = (float)((j + 0.5) * scale_y - 0.5);
		int sy = (int)floor(fy);
		fy -= sy;
		sy = (sy>iHiehgtSrc - 2)?iHiehgtSrc - 2:sy;
		sy = (0>sy)?0:sy;

		short cbufy[2];
		cbufy[0] = (short)((1.f - fy) * 2048);  //cv::saturate_cast<short>
		cbufy[1] = 2048 - cbufy[0];

		for ( i = 0; i < imgeDst->width; ++i)
		{
			float fx = (float)((i + 0.5) * scale_x - 0.5);
			int sx = (int)floor(fx);
			fx -= sx;

			if (sx < 0) {
				fx = 0, sx = 0;
			}
			if (sx >= iWidthSrc - 1) {
				fx = 0, sx = iWidthSrc - 2;
			}

			short cbufx[2];
			cbufx[0] = (short)((1.f - fx) * 2048);  //cv::saturate_cast<short>
			cbufx[1] = 2048 - cbufx[0];

			*(data_Dst+ j*stepDst + i) = (*(dataSrc + sy*stepSrc + sx) * cbufx[0] * cbufy[0] +
				*(dataSrc + (sy+1)*stepSrc + sx ) * cbufx[0] * cbufy[1] +
				*(dataSrc + sy*stepSrc + (sx+1)) * cbufx[1] * cbufy[0] +
				*(dataSrc + (sy+1)*stepSrc + (sx+1)) * cbufx[1] * cbufy[1]) >> 22;
		}
	}
}


void getFeatures(IplImage*  image, unsigned char inithann, float* dst, float scale_adjust, Rect *_roi)
{
    Rect_int extracted_roi;
    int i,j;
    float cx = _roi->x + _roi->width / 2;
    float cy = _roi->y + _roi->height / 2;

    if (inithann) //inithann = 1 ***
	{
        int padded_w = _roi->width * padding;       //padding = 2.5 ***
        int padded_h = _roi->height * padding;

        if (template_size > 1) // Fit largest dimension to the given template size
		{  
            if (padded_w >= padded_h)  //fit to width
                _scale = (float)padded_w / (float) template_size;
            else
                _scale = (float)padded_h / (float) template_size;

            _tmpl_sz.width = padded_w / _scale;
            _tmpl_sz.height = padded_h / _scale;
        }
        else                  //template_size = 1 ***
		{  //No template size given, use ROI size
            _tmpl_sz.width = padded_w;
            _tmpl_sz.height = padded_h;
            _scale = 1;
			printf("_tmpl_sz.width =%d, _tmpl_sz.height=%d\n",_tmpl_sz.width,_tmpl_sz.height);
        }

        if (_hogfeatures)    //_hogfeatures = 1 ***
		{
            // Round to cell size and also make it even
            _tmpl_sz.width = ( ( (int)(_tmpl_sz.width / (2 * cell_size)) ) * 2 * cell_size ) + cell_size*2; //cell_size=4***
            _tmpl_sz.height = ( ( (int)(_tmpl_sz.height / (2 * cell_size)) ) * 2 * cell_size ) + cell_size*2;
        }
        else 
		{  //Make number of pixels even (helps with some logic involving half-dimensions)
            _tmpl_sz.width = (_tmpl_sz.width / 2) * 2;
            _tmpl_sz.height = (_tmpl_sz.height / 2) * 2;
        }
    }

    extracted_roi.width =  scale_adjust * _scale * _tmpl_sz.width; //scale_adjust = 1, _scale = 1, 
    extracted_roi.height = scale_adjust * _scale * _tmpl_sz.height;

    // center roi with new size
    extracted_roi.x = cx - extracted_roi.width / 2;
    extracted_roi.y = cy - extracted_roi.height / 2;

	printf("extracted_roi.x=%d extracted_roi.y=%d,width=%d,height=%d \n",extracted_roi.x,extracted_roi.y,extracted_roi.width,extracted_roi.height);
    IplImage FeaturesMap;
    IplImage z;

    //z.imageData=(uchar *)0x0C010200;//感兴趣图像区域大小，存放大小约128*128:0x4000
	z.imageData = (unsigned char *)malloc(128*128*sizeof(unsigned char));//感兴趣图像区域大小，存放大小约128*128:0x4000
	//将跟踪目标框放大2.5倍，调整框边界后将区域传递给z.imageData
	subwindow((IplImage*)image, extracted_roi, &z);


    // HOG features
    if (_hogfeatures) //_hogfeatures = 1***
	{
        IplImage z_ipl = z;
        //CvLSVMFeatureMapCaskade *map=(CvLSVMFeatureMapCaskade *)0x0C017040;//大小0x10
        CvLSVMFeatureMapCaskade *map = (CvLSVMFeatureMapCaskade *)malloc(sizeof(CvLSVMFeatureMapCaskade));//大小0x10
        //map->map = (float *)malloc(sizeof (float)*(26* 26 * 2));
		
		getFeatureMaps(&z_ipl, cell_size, &map); //cell_size=4***
		
        //if(z.imageData!=NULL)
            //free(z.imageData);
        
        printf("map->sizeX =%d,map->sizeY = %d,\n",map->sizeX,map->sizeY);
        printf("normalizeAndTruncate	front \n");   
        normalizeAndTruncate(map,0.2f);
		printf("normalizeAndTruncate	after \n");
		
        PCAFeatureMaps(map);
		printf("PCAFeatureMaps	after \n");
		
        size_patch[0] = map->sizeY;
        size_patch[1] = map->sizeX;
		printf("size_patch[0] = %d,size_patch[1] = %d map->sizeY = %d,map->sizeX = %d \n",size_patch[0],size_patch[1],map->sizeY,map->sizeX);
        size_patch[2] = map->numFeatures;
		printf("size_patch[2] = %d \n",size_patch[2]);
		
        FeaturesMap.width=map->numFeatures;
        FeaturesMap.height=map->sizeX*map->sizeY;
        size_xy=size_patch[0]*size_patch[1];
        FeaturesMap.imageData_f=map->map;
		
        //把临时变量转为全局变量 防止因动态分配内存释放造成的数据丢失
  		printf("hello world 1\n");
        //float *data_map=(float *)0x0C0883F0;//大小：40*40*31*4:0x30700
        float *data_map=(float *)malloc(40*40*31*sizeof(float));//大小：40*40*31*4:0x30700
        memcpy(data_map,FeaturesMap.imageData_f,FeaturesMap.width*FeaturesMap.height*sizeof(float));
		printf("hello world 2\n");

		int i,j;
        for(i=0;i<FeaturesMap.height;i++)
        	for(j=0;j<FeaturesMap.width;j++)
        	{
        		*(FeaturesMap.imageData_f+j*FeaturesMap.height+i) = *(data_map+i*FeaturesMap.width+j);//转置
        	}
		printf("hello world 3\n");		
        free(data_map);
		printf("hello world 3.0\n");
        //freeFeatureMapObject(&map);
        	//free(map->map);
		   printf("hello world 3.1\n");
		  // free(map);
		   printf("hello world 3.2\n");
       
    }

    if (inithann) {
    	//创建汉宁窗
        createHanningMats();
    }
	printf("hello world 4\n");

    //矩阵mul运算
    for(i=0;i<FeaturesMap.width;i++)
    	for(j=0;j<FeaturesMap.height;j++)
    		*(dst+i*FeaturesMap.height+j) = *(FeaturesMap.imageData_f+i*FeaturesMap.height+j)*hann[i*FeaturesMap.height+j];
	printf("hello world 5\n");
		


	free(z.imageData);
}

void createGaussianPeak(int sizey, int sizex,Complex *dst)
{
    int i,j;
    int syh = (sizey) / 2;
    int sxh = (sizex) / 2;
    float output_sigma = 0;
    float mult =0;
    output_sigma=sqrt((double) sizex * sizey) / padding * output_sigma_factor;
    mult =-0.5 / (output_sigma * output_sigma);

    for ( i = 0; i < sizey; i++)
        for ( j = 0; j < sizex; j++)
        {
            int ih = i - syh;
            int jh = j - sxh;
            _src_data[i*sizex+j].real = exp(mult * (float)(ih * ih + jh * jh));
            _src_data[i*sizex+j].imagin=0.0;
        }
	printf("hello world 1\n");
    fftd(_src_data,sizey,sizex,dst,0);
	printf("hello world 2\n");
}



void init(IplImage* image,Rect *roi)
{
	_tmpl=_tmpl_data;
    getFeatures((IplImage*)image, 1, _tmpl, 1.0, (Rect *)roi);
	//printf("getFeatures() \n");
    _prob=_prob_data;
	printf("size_patch[0]=%d size_patch[1] = %d size_patch[2] = %d \n", size_patch[0], size_patch[1],size_patch[2]);
    createGaussianPeak(size_patch[0], size_patch[1], _prob);
	//printf("createGaussianPeak() \n");
    memset(_alphaf_data,0,size_patch[0]*size_patch[1]*sizeof(float)*2);
    _alphaf=_alphaf_data;
    train(_tmpl, 1.0,_prob,_alphaf); // train with initial frame
   // printf("train() \n");
}

void mulSpectrums(Complex* srcA, Complex* srcB, Complex* dst)
{
	int i,j;
	for (i=0; i<size_patch[0]; i++)
	{
		for (j = 0; j < size_patch[1]; j++)
		{
			(dst+i*size_patch[1]+j)->real=(srcA+i*size_patch[1]+j)->real*\
					(srcB+i*size_patch[1]+j)->real	+(srcA+i*size_patch[1]+j)->imagin*\
					(srcB+i*size_patch[1]+j)->imagin;
			(dst+i*size_patch[1]+j)->imagin=-(srcA+i*size_patch[1]+j)->real*\
					(srcB+i*size_patch[1]+j)->imagin+(srcA+i*size_patch[1]+j)->imagin*\
					(srcB+i*size_patch[1]+j)->real;
		}
	}
}

void gaussianCorrelation(const float* x1, const float* x2, Complex *k)
{
	//float *c=(float *)0x00860000;//size_patch[1]*size_patch[0]*4:40*40*4:0x1900
	float *c=(float *)malloc(40*40*sizeof(float));
    memset(c,0,size_xy*sizeof(float));

    //float *temp=(float *)0x0088C880;//size_xy*size_patch[2]:40*40*31*4:0x30700
    float *temp=(float *)malloc(40*40*31*sizeof(float));
	//float *temp1=(float *)0x85150040;//size_xy*size_patch[2]:40*40*31*4:0x30700
	float *temp1=(float *)malloc(40*40*31*sizeof(float));

    int i,j,m;
	// HOG features
    if (_hogfeatures) {
    	
    	//Complex *caux=(Complex *)0x00861900;//size_xy*4*2:40*40*4*2:0x3200
    	Complex *caux=(Complex *)malloc(40*40*sizeof(Complex));
    	//Complex *caux_ifft=(Complex *)0x00864B00;//size_xy*4*2:40*40*4*2:0x3200
    	Complex *caux_ifft=(Complex *)malloc(40*40*sizeof(Complex));

    	//Complex *x1aux=(Complex *)0x00867D00;//size_xy*4*2:40*40*4*2:0x3200
		Complex *x1aux=(Complex *)malloc(40*40*sizeof(Complex));

    	//Complex *x2aux=(Complex *)0x0086AF00;//size_xy*4*2:40*40*4*2:0x3200
    	Complex *x2aux=(Complex *)malloc(40*40*sizeof(Complex));
		
		memset(x1aux,0,size_xy*sizeof(float)*2);
    	memset(x2aux,0,size_xy*sizeof(float)*2);

    	//Complex *x1aux_fft=(Complex *)0x0086E100;//size_xy*4*2:40*40*4*2:0x3200
    	Complex *x1aux_fft=(Complex *)malloc(40*40*sizeof(Complex));
    	//Complex *x2aux_fft=(Complex *)0x00871300;//size_xy*4*2:40*40*4*2:0x3200
		Complex *x2aux_fft=(Complex *)malloc(40*40*sizeof(Complex));

        for ( i = 0; i < size_patch[2]; i++) 
		{
        	for(j=0;j<size_xy;j++)
        	{
        		x1aux[j].real=*(x1+i*size_xy+j);
        		x1aux[j].imagin=0;
        		x2aux[j].real=*(x2+i*size_xy+j);
        		x2aux[j].imagin=0;
        	}
        	fftd(x1aux,size_patch[0],size_patch[1],x1aux_fft,0);
        	fftd(x2aux,size_patch[0],size_patch[1],x2aux_fft,0);
            mulSpectrums(x1aux_fft, x2aux_fft, caux);

            fftd(caux, size_patch[0], size_patch[1], caux_ifft, 1);
            rearrange(caux_ifft,size_patch[0],size_patch[1]);
            //提取复数的实部
            for(m=0;m<size_xy;m++)
            	c[m]= c[m]+ (caux_ifft+m)->real;
        }
		free(caux);
		free(caux_ifft);
		free(x1aux);
		free(x2aux);
		free(x1aux_fft);		
		free(x2aux_fft);
    }
    //计算 x1.mul(x1)
    for(i=0;i<size_patch[2];i++)
		for(j=0;j<size_xy;j++)
		{
			temp[i*size_xy+j]=*(x1+i*size_xy+j)*(*(x1+i*size_xy+j));
			temp1[i*size_xy+j]=*(x2+i*size_xy+j)*(*(x2+i*size_xy+j));
		}

    float sum=0.0;
    float sum1=0.0;
    for(i=0;i<size_patch[2];i++)
    	for(j=0;j<size_xy;j++)
    	{
    		sum+=temp[i*size_xy+j];
    		sum1+=temp1[i*size_xy+j];
        }
   
    for(i=0;i<size_patch[0];i++)
		for(j=0;j<size_patch[1];j++)
			c[i*size_patch[1]+j]=((sum1+sum)-c[i*size_patch[1]+j]*2.0)/(size_xy*size_patch[2]);
	
    for(i=0;i<size_patch[0];i++)
		for(j=0;j<size_patch[1];j++)
			c[i*size_patch[1]+j]=max(c[i*size_patch[1]+j],0);
		
    for(i=0;i<size_patch[0];i++)
		for(j=0;j<size_patch[1];j++)
			(k+i*size_patch[1]+j)->real=exp(-(c[i*size_patch[1]+j])/(sigma * sigma));
		
	free(c);
	free(temp);
	free(temp1);
}

void linearCorrelation(const float* x1, const float* x2, Complex *k)
{
	//float *c=(float *)0x00860000;//size_patch[1]*size_patch[0]*4:40*40*4:0x1900
	float *c=(float *)malloc(40*40*sizeof(float));
    memset(c,0,size_xy*sizeof(float));
    int i,j,m;
	// HOG features
    if (_hogfeatures) 
	{
    	//Complex *caux=(Complex *)0x00861900;//size_xy*4*2:40*40*4*2:0x3200
		Complex *caux=(Complex *)malloc(40*40*sizeof(Complex));

    	//Complex *caux_ifft=(Complex *)0x00861900;//size_xy*4*2:40*40*4*2:0x3200
		Complex *caux_ifft=(Complex *)malloc(40*40*sizeof(Complex));

    	//Complex *x1aux=(Complex *)0x00867D00;//size_xy*4*2:40*40*4*2:0x3200
    	Complex *x1aux=(Complex *)malloc(40*40*sizeof(Complex));
		
    	//Complex *x2aux=(Complex *)0x0086AF00;//size_xy*4*2:40*40*4*2:0x3200
    	Complex *x2aux=(Complex *)malloc(40*40*sizeof(Complex));
		memset(x1aux,0,size_xy*sizeof(float)*2);
    	memset(x2aux,0,size_xy*sizeof(float)*2);

    	//Complex *x1aux_fft=(Complex *)0x00867D00;//size_xy*4*2:40*40*4*2:0x3200
		Complex *x1aux_fft=(Complex *)malloc(40*40*sizeof(Complex));

    	//Complex *x2aux_fft=(Complex *)0x0086AF00;//size_xy*4*2:40*40*4*2:0x3200
		Complex *x2aux_fft=(Complex *)malloc(40*40*sizeof(Complex));

        for ( i = 0; i < size_patch[2]; i++) 
		{
        	for(j=0;j<size_xy;j++)
        	{
        		x1aux[j].real=*(x1+i*size_xy+j);
        		x1aux[j].imagin=0;
        		x2aux[j].real=*(x2+i*size_xy+j);
        		x2aux[j].imagin=0;
        	}
        	fftd(x1aux,size_patch[0],size_patch[1],x1aux_fft,0);
        	fftd(x2aux,size_patch[0],size_patch[1],x2aux_fft,0);
            mulSpectrums(x1aux_fft, x2aux_fft, caux);

            fftd(caux, size_patch[0], size_patch[1], caux_ifft,1);
            rearrange(caux_ifft,size_patch[0],size_patch[1]);
            //提取复数的实部
            for(m=0;m<size_xy;m++)
            	c[m] += (caux_ifft+m)->real;
        }
		
		free(caux);
		free(caux_ifft);
		free(x1aux);
		free(x2aux);
		free(x1aux_fft);
		free(x2aux_fft);
    }
    
    for(i=0;i<size_patch[0];i++)
		for(j=0;j<size_patch[1];j++)
			(k+i*size_patch[1]+j)->real=c[i*size_patch[1]+j]/(size_xy*size_patch[2]);

	free(c);
}


void train(float * x, float train_interp_factor,Complex *_prob,Complex *_alphaf)
{
	int i,j;

	//Complex *k=(Complex *)0x00875180;//48*48模板：大小size_xy*4*2:30*30*4*2:0x1C20
	Complex *k=(Complex *)malloc(30*30*sizeof(Complex));
	memset(k,0,sizeof(float)*2*size_xy);

	//Complex *dst=(Complex *)0x00875180;//48*48模板：大小size_xy*4*2:30*30*4*2:0x1C20
	Complex *dst=(Complex *)malloc(30*30*sizeof(Complex));
	//Complex *alphaf=(Complex *)0x008789C0;//48*48模板：大小size_xy*4*2:30*30*4*2:0x1C20
	Complex *alphaf=(Complex *)malloc(30*30*sizeof(Complex));

	//linearCorrelation((float *)x, (float *)x,k);//*chang***********************
	gaussianCorrelation((float *)x, (float *)x,k);
    fftd(k,size_patch[0],size_patch[1],dst,0);
    for ( i = 0; i<size_patch[0]; i++)
    	for ( j = 0; j<size_patch[1]; j++)
    		dst[i*size_patch[1]+j].real+=lambda;

    complexDivision(_prob, dst,alphaf);
    for ( i = 0; i<size_patch[2]; i++)
		for ( j = 0; j<size_xy; j++)
      		*(_tmpl+i*size_xy+j )= *(_tmpl+i*size_xy+j )*(1 - train_interp_factor)+ *(x+i*size_xy+j )*train_interp_factor ;

    for ( i = 0; i<size_patch[0]; i++)
		for ( j = 0; j<size_patch[1]; j++)
		{
			(_alphaf+i*size_patch[1]+j)->real=(_alphaf+i*size_patch[1]+j)->real*(1-train_interp_factor)\
			                                  +(alphaf+i*size_patch[1]+j)->real*train_interp_factor;
			(_alphaf+i*size_patch[1]+j)->imagin=(_alphaf+i*size_patch[1]+j)->imagin*(1-train_interp_factor)\
			                                  +(alphaf+i*size_patch[1]+j)->imagin*train_interp_factor;
		}
		
	free(k);
	free(dst);
	free(alphaf);
}
float subPixelPeak(float left, float center, float right)
{
    float divisor = 2 * center - right - left;

    if (divisor == 0)
        return 0;

    return 0.5 * (right - left) / divisor;
}

void MaxLoc(Complex *res, float *pv, Point *pi)
{
   int i,j;
   *pv=res->real;
	for ( i = 0; i<size_patch[0]; i++)
		for ( j = 0; j<size_patch[1]; j++)
			if(*pv<(res+i*size_patch[1]+j)->real)
			{
				*pv=(res+i*size_patch[1]+j)->real;
				pi->x=j;
				pi->y=i;
			}
}

void detect(float* z, float* x, float *peak_value, Point_f *p)
{
	//Complex *k=(Complex *)0x00875180;//48*48模板：大小size_xy*4*2:30*30*4*2:0x1C20
	Complex *k=(Complex *)malloc(30*30*sizeof(Complex));
	memset(k,0,sizeof(float)*2*size_xy);

	//Complex *dst=(Complex *)0x00875180;//48*48模板：大小size_xy*4*2:30*30*4*2:0x1C20
	Complex *dst=(Complex *)malloc(30*30*sizeof(Complex));

	//Complex *temp=(Complex *)0x008789C0;//48*48模板：大小size_xy*4*2:30*30*4*2:0x1C20
	Complex *temp=(Complex *)malloc(30*30*sizeof(Complex));

	//Complex *ifft_dst=(Complex *)0x008789C0;//48*48模板：大小size_xy*4*2:30*30*4*2:0x1C20
	Complex *ifft_dst=(Complex *)malloc(30*30*sizeof(Complex));

	//linearCorrelation(x, z, k);//**change*******************************
	gaussianCorrelation(x, z, k);

    fftd(k,size_patch[0],size_patch[1],dst,0);
    complexMultiplication(_alphaf, dst,temp);
    fftd(temp,size_patch[0],size_patch[1],ifft_dst,1);

    //minMaxLoc only accepts doubles for the peak, and integer points for the coordinates
    Point pi;
    float pv;
    MaxLoc(ifft_dst, &pv, &pi);
    *peak_value = (float) pv;

    //subpixel peak estimation, coordinates will be non-integer
    p->x=(float)pi.x;
    p->y=(float)pi.y;

    if (pi.x > 0 && pi.x < size_patch[1]-1) 
        p->x += subPixelPeak(ifft_dst[pi.y*size_patch[1]+pi.x-1].real, *peak_value, ifft_dst[pi.y*size_patch[1]+pi.x+1].real);
    
    if (pi.y > 0 && pi.y < size_patch[0]-1) 
        p->y += subPixelPeak(ifft_dst[(pi.y-1)*size_patch[1]+pi.x].real, *peak_value, ifft_dst[(pi.y+1)*size_patch[1]+pi.x].real);
    

    p->x -= (size_patch[1]) / 2;
    p->y -= (size_patch[0]) / 2;

	free(k);
	free(dst);
	free(temp);
	free(ifft_dst);
}

void update(IplImage* image, Rect_int* result, Rect* _roi)
{
    float cx = 0;
    float cy = 0;
    cx = _roi->x + _roi->width / 2.0f;
    cy = _roi->y + _roi->height / 2.0f;

    float peak_value=0.0;
    getFeatures((IplImage*)image, 0, _tmpl_data1, 1.0f, (Rect*)_roi);
    Point_f res;
    detect((float *)_tmpl, _tmpl_data1, &peak_value, &res);

    if (scale_step != 1.0f) {
        // Test at a smaller _scale
        float new_peak_value=0.0;
        Point_f new_res ;
        getFeatures((IplImage*)image, 0, _tmpl_data1, 1.0f / scale_step, (Rect*)_roi);
        detect(_tmpl, _tmpl_data1, &new_peak_value, &new_res);

        if (scale_weight * new_peak_value > peak_value) 
		{
            res = new_res;
            peak_value = new_peak_value;
            _scale /= scale_step;
            _roi->width /= scale_step;
            _roi->height /= scale_step;
        }

        // Test at a bigger _scale
        getFeatures((IplImage*)image, 0, _tmpl_data1, scale_step, (Rect*)_roi);
        detect(_tmpl, _tmpl_data1, &new_peak_value,&new_res);

        if (scale_weight * new_peak_value > peak_value) 
		{
            res = new_res;
            peak_value = new_peak_value;
            _scale *= scale_step;
            _roi->width *= scale_step;
            _roi->height *= scale_step;
        }
    }

    // Adjust by cell size and _scale
    _roi->x = cx - _roi->width / 2.0f + ((float) res.x * cell_size * _scale);
    _roi->y = cy - _roi->height / 2.0f + ((float) res.y * cell_size * _scale);

    if (_roi->x >= image->width - 1) 
		_roi->x = image->width - 1;
    if (_roi->y >= image->height - 1) 
		_roi->y = image->height - 1;
    if (_roi->x + _roi->width <= 0) 
		_roi->x = -_roi->width + 2;
    if (_roi->y + _roi->height <= 0)
		_roi->y = -_roi->height + 2;

    getFeatures((IplImage*)image, 0, x_data, 1.0f, (Rect*)_roi);
    train(x_data, interp_factor, _prob, _alphaf);
    result->height = _roi->height;
    result->width  = _roi->width;
    result->x      = _roi->x;
    result->y      = _roi->y;
}



