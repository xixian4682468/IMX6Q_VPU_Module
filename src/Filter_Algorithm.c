#include <stdio.h>
#include "math.h"
#include "string.h"
#include "stdlib.h"


//腐蚀运算
void Erosion_Filter(unsigned char* data, int width, int height)
{
    int i, j, index, sum, flag; 
    sum = height * width * sizeof(unsigned char);
    unsigned char *tmpdata = (unsigned char*)malloc(sum);
    memcpy(tmpdata, data, sum);
	int m,n;
    for(i=1; i<height-1; i++)
    {
        for(j=1; j<width-1; j++)
        {
            flag = 1;
            for(m=i-1; m<i+2; m++)
            {
                for(n=j-1; n<j+2; n++)
                {
                    //自身及领域中若有一个为0
                    //则将该点设为0
                    if(tmpdata[i*width+j] == 0 || tmpdata[m*width+n] == 0)
                    {
                        flag = 0;
                        break;
                    }
                }
                if(flag == 0)
                {
                    break;
                }
            }
            if(flag == 0)
            {
                data[i*width+j] = 0;
            }
            else
            {
                data[i*width+j] = 255;
            }
        }
    }
    free(tmpdata);
}

//膨胀运算
void Dilation_Filter(unsigned char* data, int width, int height)
{
    int i, j, index, sum, flag;
    sum = height * width * sizeof(unsigned char);
    unsigned char *tmpdata = (unsigned char*)malloc(sum);
    memcpy(tmpdata, data, sum);
	int m,n;
    for(i=1; i<height-1;i++)
    {
        for(j=1; j<width-1;j++)
        {
            flag = 1;
            for(m=i-1; m<i+2; m++)
            {
                for(n=j-1; n<j+2; n++)
                {
                    //自身及领域中若有一个为255
                    //则将该点设为255
                    if(tmpdata[i*width+j]==255 || tmpdata[m*width+n]==255)
                    {
                        flag = 0;
                        break;
                    }
                }
                if(flag == 0)
                {
                    break;
                }
            }
            if(flag == 0)
            {
                data[i*width+j] = 255;
            }
            else
            {
                data[i*width+j] = 0;
            }
        }
    }
    free(tmpdata);
}

//3*3均值滤波
void Mean_Filter(unsigned char * pSrc, unsigned char * pDst, int search_width, int search_height)
{
	unsigned char Temp[9] = {0};
	unsigned short Sum = 0;
	unsigned short i,j,k;
	for(i=1; i<search_height-1; i++)
	{
		for(j=1; j<search_width-1; j++)
		{
			Sum = 0;
			Temp[0] = pSrc[(i-1)*search_width + j-1];
			Temp[1] = pSrc[(i-1)*search_width + j  ];
			Temp[2] = pSrc[(i-1)*search_width + j+1];
			Temp[3] = pSrc[(i)*search_width + j-1  ];
			Temp[4] = pSrc[(i)*search_width + j    ];
			Temp[5] = pSrc[(i)*search_width + j+1  ];
			Temp[6] = pSrc[(i+1)*search_width + j-1];
			Temp[7] = pSrc[(i+1)*search_width + j  ];
			Temp[8] = pSrc[(i+1)*search_width + j+1];
			for(k=0; k<9; k++)
				Sum+= Temp[k];
			pDst[i*search_width + j] = Sum/9;
		}
	}
}



