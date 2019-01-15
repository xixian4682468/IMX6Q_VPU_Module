#include <stdio.h>
#include "math.h"
#include "string.h"
#include "stdlib.h"


void Frame_Diff_Method(unsigned char* pSrc, unsigned char *pDst, unsigned char *Mask,int search_x,int search_y,int search_width,int search_height)
{
	int tmp1,tmp2,dist,i,j;
	for(i=search_y;i<search_height+search_y;i++)
		for(j=search_x;j<search_width+search_x;j++)
		{
			tmp1 = pSrc[i*search_width+j];
			tmp2 = pDst[i*search_width+j];
			tmp1>=tmp2 ? (dist=tmp1-tmp2) : (dist=tmp2-tmp1);
			if(dist>60)
			{
				Mask[i*search_width+j] = 255;
			}
		}
}



