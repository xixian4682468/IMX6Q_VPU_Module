/*
 * fhog.c
 *
 *  Created on: 2016-10-31
 *      Author: ZQ
 */

#include "KCF_Track.h"
#include <string.h>

int allocFeatureMapObject(CvLSVMFeatureMapCaskade **obj, const int sizeX,
                          const int sizeY, const int numFeatures)
{
    int i;
    (*obj) = (CvLSVMFeatureMapCaskade *)malloc(sizeof(CvLSVMFeatureMapCaskade));
    //(*obj) = (CvLSVMFeatureMapCaskade *)0x0C04E2A0;//大小0x10
    
    (*obj)->sizeX       = sizeX;
    (*obj)->sizeY       = sizeY;
	printf("(*obj)->sizeX = %d,(*obj)->sizeY = %d,sizeX =%d,sizeY = %d\n",(*obj)->sizeX,(*obj)->sizeY,sizeX,sizeY);
	
    (*obj)->numFeatures = numFeatures;
    (*obj)->map = (float *)malloc(sizeof(float)*(sizeX * sizeY  * numFeatures));//26*26*6
    //(*obj)->map = (float *)0x00860000;//4*32*32*15:0xF000

    memset((*obj)->map,0,sizeof(float) *sizeX * sizeY * numFeatures);


	//free((*obj)->map);
	//free(*obj);
	
    return LATENT_SVM_OK;
}

//求X方向的梯度
void Grad_X(const IplImage* matSrc, IplImage* dx)
{
	int i, j;
	for (i = 0; i<matSrc->height; i++)
	{
		for (j = 0; j<matSrc->width; j++)
		{
			float *d_current = (float*)((dx->imageData) + (i*(dx->widthStep) + j * 1 * 4 ));
			uchar *s_current = matSrc->imageData + (i*(matSrc->widthStep) + j * 1 * 1);
//			if (j == 0)
//			{
//				*(d_current + 0) = (*(s_current + 1)) - (*(s_current)); //B通道X方向梯度
////				*(d_current + 1) = (*(s_current + 1 + 3)) - (*(s_current + 1));//G通道X方向梯度
////				*(d_current + 2) = (*(s_current + 2 + 3)) - (*(s_current + 2));//R通道X方向梯度
//			}
//			else if (j == matSrc->width - 1)
//			{
//				*d_current = (*(s_current)) - (*(s_current - 1)); //B通道X方向梯度
////				*(d_current + 1) = (*(s_current + 1)) - (*(s_current + 1 - 3));//G通道X方向梯度
////				*(d_current + 2) = (*(s_current + 2)) - (*(s_current + 2 - 3));//R通道X方向梯度
//			}
//			else
//			{
				*d_current = (*(s_current+1)) - (*(s_current - 1)); //B通道X方向梯度
//				*(d_current + 1) = (*(s_current + 1+3)) - (*(s_current + 1 - 3));//G通道X方向梯度
//				*(d_current + 2) = (*(s_current + 2 + 3)) - (*(s_current + 2 - 3));//R通道X方向梯度
			//}
		}
	}
}

//求y方向的梯度
void Grad_Y(const IplImage* matSrc, IplImage* dy)
{
	int i, j;
	for (i = 0; i<matSrc->height; i++)
	{
		for (j = 0; j<matSrc->width; j++)
		{
			float *d_current = (float*)((dy->imageData) + (i*(dy->widthStep) + j * 1 * 4 ));
			uchar *s_current = matSrc->imageData + (i*(matSrc->widthStep) + j * 1 * 1);
//			if (i == 0)
//			{
//				*(d_current + 0) = *(s_current + matSrc->widthStep) - *(s_current); //B通道Y方向梯度
////				*(d_current + 1) = *(s_current + 1 + matSrc->widthStep) - *(s_current + 1);//G通道Y方向梯度
////				*(d_current + 2) = *(s_current + 2 + matSrc->widthStep) - *(s_current + 2);//R通道Y方向梯度
//			}
//			else if (i == matSrc->height - 1)
//			{
//				*(d_current + 0) = *(s_current)-*(s_current - matSrc->widthStep); //B通道Y方向梯度
////				*(d_current + 1) = *(s_current + 1) - *(s_current + 1 - matSrc->widthStep);//G通道Y方向梯度
////				*(d_current + 2) = *(s_current + 2) - *(s_current + 2 - matSrc->widthStep);//R通道Y方向梯度
//			}
//			else
//			{
				*(d_current + 0) = *(s_current + matSrc->widthStep) - *(s_current - matSrc->widthStep); //B通道Y方向梯度
//				*(d_current + 1) = *(s_current + 1 + matSrc->widthStep) - *(s_current + 1 - matSrc->widthStep);//G通道Y方向梯度
//				*(d_current + 2) = *(s_current + 2 + matSrc->widthStep) - *(s_current + 2 - matSrc->widthStep);//R通道Y方向梯度
			//}
		}
	}
}

void  getFeatureMaps(const IplImage* image, const int k, CvLSVMFeatureMapCaskade **map)
{
    int sizeX, sizeY;
    int p, px, stringSize;
    int height, width, numChannels;
    int i, j, kk, c, ii, jj, d;
    float  * datadx, * datady;

    int   ch;
    float magnitude,x, y, tx, ty;
    //IplImage  *dx_img=(IplImage  *)0x0C017060;//大小0x14
    IplImage  *dx_img = (IplImage  *)malloc(sizeof(IplImage));
    //IplImage  *dy_img=(IplImage  *)0x0C017080;//大小0x14
	IplImage  *dy_img = (IplImage  *)malloc(sizeof(IplImage));

    int *nearest;
    float *w, a_x, b_x;

    float * r;
    int   * alfa;

    float boundary_x[NUM_SECTOR + 1]; //NUM_SECTOR = 2
    float boundary_y[NUM_SECTOR + 1];
    float max, dotProd;
    int   maxi;

    height = image->height;
    width  = image->width ;
	//printf("image->width =%d,image->height =%d\n",image->width, image->height);
    numChannels =1;
	
    dx_img->width=image->width;
    dx_img->height=image->height;
//    dx->imageData=(char *)malloc(image->width*image->height*sizeof(float));
    //dx_img->imageData=(uchar *)0x0C0170A0;//大小128*128*4:0x10000
    
    dx_img->imageData=(unsigned char *)malloc(image->width*image->height*sizeof(unsigned char)*4); //**********x4
    dx_img->widthStep=image->width*sizeof(float)*1;

	
    dy_img->width=image->width;
    dy_img->height=image->height;
//    dy->imageData=(char *)malloc(image->width*image->height*sizeof(float));
    //dy_img->imageData=(uchar *)0x0C0270A0;//大小168*168*4:0x10000
    
    dy_img->imageData=(unsigned char *)malloc(image->width*image->height*sizeof(unsigned char)*4); //***********x4
    dy_img->widthStep=image->width*sizeof(float)*1;

    sizeX = width  / k; //k = 4
    sizeY = height / k;
    px    = 3 * NUM_SECTOR;
    p     = px;
    stringSize = sizeX * p;
	
	printf("sizeX = %d,sizeY = %d, p=%d\n",sizeX,sizeY , p);
    allocFeatureMapObject(map, sizeX, sizeY, p); //p = 6
	printf("allocFeatureMapObject (*map)->sizeX =%d,(*map)->sizeY = %d,\n", (*map)->sizeX, (*map)->sizeY );

    Grad_X(image, dx_img);
    Grad_Y(image, dy_img);

    float arg_vector;
    for(i = 0; i <= NUM_SECTOR; i++)
    {
        arg_vector    = ( (float) i ) * ((float)(PI) / (float)(NUM_SECTOR));
        boundary_x[i] = cosf(arg_vector);
        boundary_y[i] = sinf(arg_vector);
    }
	
	//printf("hello r alfa \n");
    r    = (float *)malloc( sizeof(float) * (width * height));
    alfa = (int   *)malloc( sizeof(int  ) * (width * height * 2));
    //r    = (float *)0x0C06CAF0;//大小：168*168*4，0x1B900
    //alfa = (int   *)0x0C0883F0;//大小：168*168*4*2，0x37200
	//printf("hello r alfa11 \n");
	
    for(j = 1; j < height - 1; j++)
    {
        datadx = (float*)(dx_img->imageData + dx_img->widthStep * j);
        datady = (float*)(dy_img->imageData + dy_img->widthStep * j);
        for(i = 1; i < width - 1; i++)
        {
            //c = 0;
            //x = (datadx[i * numChannels + c]);
        	x = (datadx[i * numChannels]);
            //y = (datady[i * numChannels + c]);
        	y = (datady[i * numChannels]);

            r[j * width + i] =sqrtf(x * x + y * y);
            for(ch = 1; ch < numChannels; ch++)
            {
                tx = (datadx[i * numChannels + ch]);
                ty = (datady[i * numChannels + ch]);
                magnitude = sqrtf(tx * tx + ty * ty);
                if(magnitude > r[j * width + i])
                {
                    r[j * width + i] = magnitude;
                    //c = ch;
                    x = tx;
                    y = ty;
                }
            }/*for(ch = 1; ch < numChannels; ch++)*/

            max  = boundary_x[0] * x + boundary_y[0] * y;
            maxi = 0;
            for (kk = 0; kk < NUM_SECTOR; kk++)
            {
                dotProd = boundary_x[kk] * x + boundary_y[kk] * y;
                if (dotProd > max)
                {
                    max  = dotProd;
                    maxi = kk;
                }
                else
                {
                    if (-dotProd > max)
                    {
                        max  = -dotProd;
                        maxi = kk + NUM_SECTOR;
                    }
                }
            }
            alfa[j * width * 2 + i * 2    ] = maxi % NUM_SECTOR;
            alfa[j * width * 2 + i * 2 + 1] = maxi;
        }/*for(i = 0; i < width; i++)*/
    }/*for(j = 0; j < height; j++)*/


	//printf("hello nearest w \n");
	//printf("hello nearest w11 \n");


    nearest = (int  *)malloc(sizeof(int  ) *  k);
	//printf("hello nearest w12 \n");
    w       = (float*)malloc(sizeof(float) * (k * 2));
    //nearest = (int  *)0x0C0BF5F0;//大小：4*4，0x10
    //w       = (float*)0x0C0BF600;//大小：4*4*2，0x20
	//printf("hello nearest w22********** \n");


    for(i = 0; i < (k >> 1); i++)
    {
        nearest[i] = -1;
    }/*for(i = 0; i < k / 2; i++)*/
    for(i = (k >> 1); i < k; i++)
    {
        nearest[i] = 1;
    }/*for(i = k / 2; i < k; i++)*/

    for(j = 0; j < (k >> 1); j++)
    {
        b_x = (k >> 1) + j + 0.5f;
        a_x = (k >> 1) - j - 0.5f;
        w[j * 2    ] = 1.0f/a_x * ((a_x * b_x) / ( a_x + b_x));
        w[j * 2 + 1] = 1.0f/b_x * ((a_x * b_x) / ( a_x + b_x));
    }/*for(j = 0; j < k / 2; j++)*/
    for(j = (k >> 1); j < k; j++)
    {
        a_x = j - (k >> 1) + 0.5f;
        b_x =-j + (k >> 1) - 0.5f + k;
        w[j * 2    ] = 1.0f/a_x * ((a_x * b_x) / ( a_x + b_x));
        w[j * 2 + 1] = 1.0f/b_x * ((a_x * b_x) / ( a_x + b_x));
    }/*for(j = k / 2; j < k; j++)*/
//printf("map  malloc\n");
    for(i = 0; i < sizeY; i++)
    {
      for(j = 0; j < sizeX; j++)
      {
        for(ii = 0; ii < k; ii++)
        {
          for(jj = 0; jj < k; jj++)
          {
            if ((i * k + ii > 0) &&
                (i * k + ii < height - 1) &&
                (j * k + jj > 0) &&
                (j * k + jj < width  - 1))
            {
              d = (k * i + ii) * width + (j * k + jj);
              (*map)->map[ i * stringSize + j * p + alfa[d * 2    ]] +=
                  r[d] * w[ii * 2] * w[jj * 2];
              (*map)->map[ i * stringSize + j * p + alfa[d * 2 + 1] + NUM_SECTOR] +=
                  r[d] * w[ii * 2] * w[jj * 2];
              if ((i + nearest[ii] >= 0) &&
                  (i + nearest[ii] <= sizeY - 1))
              {
                (*map)->map[(i + nearest[ii]) * stringSize + j * p + alfa[d * 2    ]             ] +=
                  r[d] * w[ii * 2 + 1] * w[jj * 2 ];
                (*map)->map[(i + nearest[ii]) * stringSize + j * p + alfa[d * 2 + 1] + NUM_SECTOR] +=
                  r[d] * w[ii * 2 + 1] * w[jj * 2 ];
              }
              if ((j + nearest[jj] >= 0) &&
                  (j + nearest[jj] <= sizeX - 1))
              {
                (*map)->map[i * stringSize + (j + nearest[jj]) *p + alfa[d * 2    ]             ] +=
                  r[d] * w[ii * 2] * w[jj * 2 + 1];
                (*map)->map[i * stringSize + (j + nearest[jj]) * p + alfa[d * 2 + 1] + NUM_SECTOR] +=
                  r[d] * w[ii * 2] * w[jj * 2 + 1];
              }
              if ((i + nearest[ii] >= 0) &&
                  (i + nearest[ii] <= sizeY - 1) &&
                  (j + nearest[jj] >= 0) &&
                  (j + nearest[jj] <= sizeX - 1))
              {
                (*map)->map[(i + nearest[ii]) * stringSize + (j + nearest[jj]) * p + alfa[d * 2    ]             ] +=
                  r[d] * w[ii * 2 + 1] * w[jj * 2 + 1];
                (*map)->map[(i + nearest[ii]) * stringSize + (j + nearest[jj]) * p + alfa[d * 2 + 1] + NUM_SECTOR] +=
                  r[d] * w[ii * 2 + 1] * w[jj * 2 + 1];
              }
            }
          }/*for(jj = 0; jj < k; jj++)*/
        }/*for(ii = 0; ii < k; ii++)*/
      }/*for(j = 1; j < sizeX - 1; j++)*/
    }/*for(i = 1; i < sizeY - 1; i++)*/

//    cvReleaseImage(&dx);
//    cvReleaseImage(&dy);
//    free(dx.imageData);
//    free(dy.imageData);
    //free(dx);
    //free(dy);
    //printf("hello free\n");
	free(dx_img->imageData);
	free(dy_img->imageData);
	free(dx_img);
	free(dy_img);
	//printf("hello free1\n");

    free(w);
    free(nearest);
	//printf("hello free2\n");

    free(r);
    free(alfa);
//printf("hello free3\n");

//    return LATENT_SVM_OK;
}

void  normalizeAndTruncate(CvLSVMFeatureMapCaskade *map, const float alfa)
{
    int i,j, ii;
    int sizeX, sizeY, p, pos, pp, xp, pos1, pos2;
    float * partOfNorm; // norm of C(i, j)
    float * newData;
    float   valOfNorm;

    sizeX     = map->sizeX;
    sizeY     = map->sizeY;
	//printf("sizeX  = %d ,sizeY = %d \n",sizeX,sizeY);
    //partOfNorm = (float *)malloc (sizeof(float) * (sizeX * sizeY));
    //partOfNorm = (float *)0x0C0BF620;	//大小42*42*4：0x1B90
    partOfNorm = (float *)malloc(sizeof(float) *(sizeX * sizeY));
	
    p  = NUM_SECTOR;
    xp = NUM_SECTOR * 3;
    pp = NUM_SECTOR * 12;

    for(i = 0; i < sizeX * sizeY; i++)
    {
        valOfNorm = 0.0f;
        pos = i * map->numFeatures;
        for(j = 0; j < p; j++)
        {
            valOfNorm += map->map[pos + j] * map->map[pos + j];
        }/*for(j = 0; j < p; j++)*/
        partOfNorm[i] = valOfNorm;
    }/*for(i = 0; i < sizeX * sizeY; i++)*/

    sizeX -= 2;
    sizeY -= 2;
	//printf("sizeX  = %d ,sizeY = %d pp = %d\n",sizeX,sizeY,pp);

    //newData = (float *)malloc (sizeof(float) * (sizeX * sizeY * pp));
    //newData = (float *)0x0C0883F0;//模板为48*48时，大小30*30*5*12*4：0x34BC0,从此处开始sizeX,sizeY均减2
	  newData = (float *)malloc (sizeof(float) * (sizeX * sizeY * pp));

//normalization
    for(i = 1; i <= sizeY; i++)
    {
        for(j = 1; j <= sizeX; j++)
        {
            valOfNorm = sqrtf(
                partOfNorm[(i    )*(sizeX + 2) + (j    )] +
                partOfNorm[(i    )*(sizeX + 2) + (j + 1)] +
                partOfNorm[(i + 1)*(sizeX + 2) + (j    )] +
                partOfNorm[(i + 1)*(sizeX + 2) + (j + 1)]) + FLT_EPSILON;
            pos1 = (i  ) * (sizeX + 2) * xp + (j  ) * xp;
            pos2 = (i-1) * (sizeX    ) * pp + (j-1) * pp;
            for(ii = 0; ii < p; ii++)
            {
                newData[pos2 + ii        ] = map->map[pos1 + ii    ] / valOfNorm;
            }/*for(ii = 0; ii < p; ii++)*/
            for(ii = 0; ii < 2 * p; ii++)
            {
                newData[pos2 + ii + p * 4] = map->map[pos1 + ii + p] / valOfNorm;
            }/*for(ii = 0; ii < 2 * p; ii++)*/
            valOfNorm = sqrtf(
                partOfNorm[(i    )*(sizeX + 2) + (j    )] +
                partOfNorm[(i    )*(sizeX + 2) + (j + 1)] +
                partOfNorm[(i - 1)*(sizeX + 2) + (j    )] +
                partOfNorm[(i - 1)*(sizeX + 2) + (j + 1)]) + FLT_EPSILON;
            for(ii = 0; ii < p; ii++)
            {
                newData[pos2 + ii + p    ] = map->map[pos1 + ii    ] / valOfNorm;
            }/*for(ii = 0; ii < p; ii++)*/
            for(ii = 0; ii < 2 * p; ii++)
            {
                newData[pos2 + ii + p * 6] = map->map[pos1 + ii + p] / valOfNorm;
            }/*for(ii = 0; ii < 2 * p; ii++)*/
            valOfNorm = sqrtf(
                partOfNorm[(i    )*(sizeX + 2) + (j    )] +
                partOfNorm[(i    )*(sizeX + 2) + (j - 1)] +
                partOfNorm[(i + 1)*(sizeX + 2) + (j    )] +
                partOfNorm[(i + 1)*(sizeX + 2) + (j - 1)]) + FLT_EPSILON;
            for(ii = 0; ii < p; ii++)
            {
                newData[pos2 + ii + p * 2] = map->map[pos1 + ii    ] / valOfNorm;
            }/*for(ii = 0; ii < p; ii++)*/
            for(ii = 0; ii < 2 * p; ii++)
            {
                newData[pos2 + ii + p * 8] = map->map[pos1 + ii + p] / valOfNorm;
            }/*for(ii = 0; ii < 2 * p; ii++)*/
            valOfNorm = sqrtf(
                partOfNorm[(i    )*(sizeX + 2) + (j    )] +
                partOfNorm[(i    )*(sizeX + 2) + (j - 1)] +
                partOfNorm[(i - 1)*(sizeX + 2) + (j    )] +
                partOfNorm[(i - 1)*(sizeX + 2) + (j - 1)]) + FLT_EPSILON;
            for(ii = 0; ii < p; ii++)
            {
                newData[pos2 + ii + p * 3 ] = map->map[pos1 + ii    ] / valOfNorm;
            }/*for(ii = 0; ii < p; ii++)*/
            for(ii = 0; ii < 2 * p; ii++)
            {
                newData[pos2 + ii + p * 10] = map->map[pos1 + ii + p] / valOfNorm;
            }/*for(ii = 0; ii < 2 * p; ii++)*/
        }/*for(j = 1; j <= sizeX; j++)*/
    }/*for(i = 1; i <= sizeY; i++)*/
//truncation
    for(i = 0; i < sizeX * sizeY * pp; i++)
    {
        //if(newData [i] > alfa) newData [i] = alfa;
    	newData [i] = (newData [i] > alfa) ? alfa : newData [i];
    }/*for(i = 0; i < sizeX * sizeY * pp; i++)*/
//swop data

    map->numFeatures  = pp;
    map->sizeX = sizeX;
    map->sizeY = sizeY;
	
	
	free (map->map);
    free (partOfNorm);
	map->map = newData;
	//free (newData);  //可能有问题

//    return LATENT_SVM_OK;
}
void PCAFeatureMaps(CvLSVMFeatureMapCaskade *map)
{
    int i,j, ii, jj, k;
    int sizeX, sizeY, p,  pp, xp, yp, pos1, pos2;
    float * newData;
    float val;
    float nx, ny;

    sizeX = map->sizeX;
    sizeY = map->sizeY;
    p     = map->numFeatures;
    pp    = NUM_SECTOR * 3 + 4;
    yp    = 4;
    xp    = NUM_SECTOR;

    nx    = 1.0f / sqrtf((float)(xp * 2));
    //ny    = 1.0f / sqrtf((float)(yp    ));
    ny    = 0.5;//ny    = 1.0f / sqrtf((float)(yp    ));

    newData = (float *)malloc (sizeof(float) * (sizeX * sizeY * pp));
    //newData = (float *)0x0088C880;//大小30*30*19*4：0x10B30
    for(i = 0; i < sizeY; i++)
    {
        for(j = 0; j < sizeX; j++)
        {
            pos1 = ((i)*sizeX + j)*p;
            pos2 = ((i)*sizeX + j)*pp;
            k = 0;
            for(jj = 0; jj < xp * 2; jj++)
            {
                val = 0;
                for(ii = 0; ii < yp; ii++)
                {
                    val += map->map[pos1 + yp * xp + ii * xp * 2 + jj];
                }/*for(ii = 0; ii < yp; ii++)*/
                newData[pos2 + k] = val * ny;
                k++;
            }/*for(jj = 0; jj < xp * 2; jj++)*/
            for(jj = 0; jj < xp; jj++)
            {
                val = 0;
                for(ii = 0; ii < yp; ii++)
                {
                    val += map->map[pos1 + ii * xp + jj];
                }/*for(ii = 0; ii < yp; ii++)*/
                newData[pos2 + k] = val * ny;
                k++;
            }/*for(jj = 0; jj < xp; jj++)*/
            for(ii = 0; ii < yp; ii++)
            {
                val = 0;
                for(jj = 0; jj < 2 * xp; jj++)
                {
                    val += map->map[pos1 + yp * xp + ii * xp * 2 + jj];
                }/*for(jj = 0; jj < xp; jj++)*/
                newData[pos2 + k] = val * nx;
                k++;
            } /*for(ii = 0; ii < yp; ii++)*/
        }/*for(j = 0; j < sizeX; j++)*/
    }/*for(i = 0; i < sizeY; i++)*/
//swop data

    map->numFeatures = pp;

    free (map->map);

    map->map = newData;
	//printf("map->sizeX =%d,map->sizeY = %d,\n",map->sizeX,map->sizeY);
	//free (newData);//注意有问题

//    return LATENT_SVM_OK;
}

int freeFeatureMapObject (CvLSVMFeatureMapCaskade **obj)
{
    if(*obj == NULL) 
		return LATENT_SVM_MEM_NULL;
    free((*obj)->map);
    free(*obj);
    (*obj) = NULL;
    return LATENT_SVM_OK;
}

