#include <stdio.h>
#include "math.h"
#include "string.h"
#include "stdlib.h"
#include "RunLength_Algorithm.h"

#define       MAXPOSSNUM  16

volatile int  m_PossTarNum, m_CandTarNum;
volatile int  Seg_Count;

RUNLENGTHCODE   *Seg_Code;   

unsigned short	*block;
unsigned int	*posxy;
unsigned short	*M    ;
unsigned short	*linsum ;            //linsum[n]:n行上的游程数量，暂且最大1080行

TARFEATURE     	*CandObject ;
TARFEATURE     	*Candidate  ;
TARFEATURE 		*g_Candidate;        //存放300个目标，直接阈值
TARFEATURE      *tempFeat   ;


void Init_RunLengthParameter(void)
{
	Seg_Code    = (RUNLENGTHCODE *)malloc(3000*sizeof(RUNLENGTHCODE));
	block       = (unsigned short*)malloc(3000*4*sizeof(unsigned short));
	posxy       = (unsigned int*)malloc(3000*8*sizeof(unsigned int));
	M        	= (unsigned short*)malloc(3000*sizeof(unsigned short));
	linsum      = (unsigned short*)malloc(1080*sizeof(unsigned short));
	CandObject  = (TARFEATURE *)malloc(300*sizeof(TARFEATURE));
	Candidate   = (TARFEATURE *)malloc(300*sizeof(TARFEATURE));
	g_Candidate = (TARFEATURE *)malloc(300*sizeof(TARFEATURE));
	tempFeat    = (TARFEATURE *)malloc(300*sizeof(TARFEATURE));
}

void UnInit_RunLengthParameter(void)
{
	free(Seg_Code);    
	free(block);      
	free(posxy);
	free(M);
	free(linsum);
	free(CandObject);
	free(Candidate);
	free(g_Candidate);
	free(tempFeat);
}


//目标游程检测
void RunLengthDetect(int search_x,int search_y,int search_width,int search_height,unsigned char * mask)
{
    int i,j;
	Seg_Count=0;
	Seg_Code[Seg_Count].CEnd = 0;
	for(i=search_y; i<search_height+search_y; i++)
	{
	   for(j=search_x; j<search_width+search_x; j++)
	   {
	       if((mask[i*search_width+j]==255)&&(Seg_Count<2000)) //生成最多2000个游程
		   {
				if(j-Seg_Code[Seg_Count].CEnd==1)
			    {
				    Seg_Code[Seg_Count].CEnd = j;
			    }
			    else
			    {
					Seg_Count++;
					Seg_Code[Seg_Count].CStart = j;
					Seg_Code[Seg_Count].CEnd   = j;
					Seg_Code[Seg_Count].Row    = i;
                }
		    }
	    }
	}
}

void AutoTarDetect(unsigned short StartRow,unsigned short EndRow,unsigned short StartCol,unsigned short EndCol)
{
	unsigned short label;
	unsigned short start,stop;
	int temp;
	int i,j,k,l,m;
	char repeat;
	int maxTarNum = -1;
	block[0] = 0;
	block[1] = 0;
	k = 0;

	if(Seg_Count == 0)
	{
		maxTarNum = 0;
	}
	if(Seg_Count > 3000)
	{
		Seg_Count = 3000;
	}
	//for(j=0;j<512;j++)
	for(j=StartRow; j<EndRow; j++) //每行游程数量赋初值0;
	{
		linsum[j] = 0;
	}
	for(i=0; i<3000; i++)
	{
		block[i*4]   = 0;
		block[i*4+1] = 0;
		block[i*4+2] = 0;
		block[i*4+3] = 0;
		M[i]         = 0;
	}
	for(i=0; i<Seg_Count; i++)
	{
		linsum[Seg_Code[i].Row]++;
		block[i*4]   = (short)Seg_Code[i].Row;
		block[i*4+1] = (short)Seg_Code[i].CStart;
		block[i*4+2] = (short)Seg_Code[i].CEnd;
		block[i*4+3] = 0;
	}
	if(Seg_Count!=0)
	{
		label=1;
		block[3]=1;

		j=1080;
		//for(i=0;i<512;i++)
		for(i=StartRow; i<EndRow; i++)
		{
			if((linsum[i]>0))
			{
				j=i;                //获得含有游程最开始的一行的行数！
				break;
			}
		}
		for(i=1; i<linsum[j]; i++)  //第一行的游程是分开的，所以lable也是分开的！
		{
			label++;
			block[i*4+3] = label;
		}
		start = 0;
		stop  = linsum[j];
		repeat = 0;
		for(k=j+1; k<EndRow; k++)
		{
			for(i=stop; i<stop+linsum[k]; i++)
			{
				repeat = 0;
				for(l=start; l<stop; l++)
				{
					if(((block[i*4+1]>=(block[l*4+1]-1))&&(block[i*4+1]<=(block[l*4+2]+1)))
					||((block[i*4+2]>=(block[l*4+1]-1))&&(block[i*4+2]<=(block[l*4+2]+1)))
					||((block[i*4+1]<=(block[l*4+1]-1))&&(block[i*4+2]>=(block[l*4+2]+1))))
					{
						if(repeat==0)
						{
							block[i*4+3] = block[l*4+3];
							repeat = 1;
						}
						else
						{
							for(m=0; m<i; m++)
							{
								if(block[m*4+3] == block[l*4+3])
								{
									block[m*4+3] = block[i*4+3];
								}
							}
						}
					}
				}
				if(repeat==0)
				{
					label++;
					block[i*4+3]=label;
				}
			}
			start=stop;
			stop=stop+linsum[k];
		}
		
		for(i=0; i<label; i++)
		{
			posxy[i*8]   = 0;
			posxy[i*8+1] = 0;
			posxy[i*8+2] = 0;
			posxy[i*8+3] = 0;
			posxy[i*8+4] = EndRow;
			posxy[i*8+5] = 0;
			posxy[i*8+6] = EndCol;
			posxy[i*8+7] = 0;
		}
		for(i=0; i<Seg_Count-1; i++)
		{
			temp = block[i*4+3] - 1;
			posxy[temp*8]   = posxy[temp*8]   + block[i*4]; // Row
			posxy[temp*8+1] = posxy[temp*8+1] + (block[i*4+1] + block[i*4+2])/2;// Col
			posxy[temp*8+2] = posxy[temp*8+2] + 1;
			for(j=block[i*4+1]; j<block[i*4+2]; j++)
			{
				M[temp] = M[temp] + 1;
			}
			posxy[temp*8+3] = posxy[temp*8+3]+1;
			posxy[temp*8+4] = (block[i*4]<=posxy[temp*8+4])? block[i*4]:posxy[temp*8+4];//目标上边界
			posxy[temp*8+5] = (posxy[temp*8+5]<=block[i*4])? block[i*4]:posxy[temp*8+5];//目标下边界
			posxy[temp*8+6] = (block[i*4+1]<=posxy[temp*8+6])? block[i*4+1]:posxy[temp*8+6];//目标左边界
			posxy[temp*8+7] = (posxy[temp*8+7]<=block[i*4+2])? block[i*4+2]:posxy[temp*8+7];//目标右下边界
			maxTarNum = maxTarNum>(temp+1)?maxTarNum:(temp+1);
		}
	}
	GetTarInformation(maxTarNum,StartRow,EndRow,StartCol,EndCol);
}


void GetTarInformation(int tarnum,short StartRow,short EndRow,short StartCol,short EndCol)
{
	int i,k11=0;
	short SuitNum=0;
	for(i = 0; i <= 100; i++)
	{
		CandObject[i].s_Area          	= 0;
		CandObject[i].s_MeanLight     	= 0;
		CandObject[i].s_PeakLight     	= 0;
		CandObject[i].s_MaxWidth      	= 0;
		CandObject[i].s_MaxHeight     	= 0;
		CandObject[i].s_C_Row         	= 0;
		CandObject[i].s_C_Col         	= 0;
	}
	m_CandTarNum=0;
	for(i=0; i<tarnum; i++)
	{
		if((posxy[i*8+2]>0) && (M[i]>20)&&(M[i]<4000))
		{
			CandObject[k11].s_Area      = M[i];
			CandObject[k11].s_C_Row     = (posxy[i*8]/posxy[i*8+2]);
			CandObject[k11].s_C_Col     = (posxy[i*8+1]/posxy[i*8+2]);
			CandObject[k11].s_MeanLight = 255;
			CandObject[k11].s_MaxHeight = posxy[i*8+5]-posxy[i*8+4]+1;
			CandObject[k11].s_MaxWidth  = posxy[i*8+7]-posxy[i*8+6]+1;
			k11++;
		}
	}
	SuitNum=k11;
	for(i=0; i<SuitNum; i++)
	{
		if(m_CandTarNum<16)
		{
			Candidate[m_CandTarNum] = CandObject[i];
			m_CandTarNum++;
		}
	}

	m_PossTarNum = (m_CandTarNum < 16) ? m_CandTarNum : MAXPOSSNUM;
	CopyCandidate(g_Candidate, Candidate);
}

//==========================================================================================================
/*函数作用：把Candidate赋值给g_Candidate,并计算出各个目标的空间方位角和空间俯仰角
  输入：g_Candidate：存放图像目标的数组变量
        Candidate:图像中提取出的目标
  输出：g_Candidate：当前帧图像中提取的各个目标，且计算出了各个目标的空间方位角和空间俯仰角
*/
void CopyCandidate(TARFEATURE* g_Candidate, TARFEATURE* Candidate)
{
	int i;
	for(i = 0; i < m_PossTarNum; i++)
	{
		g_Candidate[i] = Candidate[i];
		g_Candidate[i].s_C_fangweiPixel = g_Candidate[i].s_C_Col;
		g_Candidate[i].s_C_fuyangPixel  = g_Candidate[i].s_C_Row;
	}
}


int Max_To_Min_TargetArea(void)
{
	int i, j;
	if(m_PossTarNum == 0)
		return -1;
	for(i = 0; i < m_PossTarNum-1; i++)
	{
		for(j = 0; j < m_PossTarNum-1-i; j++)
			if(g_Candidate[j].s_Area < g_Candidate[j+1].s_Area)
			{
				tempFeat[0] 	 = g_Candidate[j];
				g_Candidate[j]   = g_Candidate[j+1];
				g_Candidate[j+1] = tempFeat[0];
			}
	}
	return 0;
}


void Maxarea_target(unsigned short * tar_x, unsigned short * tar_y)
{
	int i=0;
	for( i = m_PossTarNum-1; i > 0; i--)
	{
		if( g_Candidate[i].s_Area > g_Candidate[i-1].s_Area)
		{
			tempFeat[0] 	 = g_Candidate[i];
			g_Candidate[i]   = g_Candidate[i-1];
			g_Candidate[i-1] = tempFeat[0];
		}
	}
	*tar_x = g_Candidate[0].s_C_fangweiPixel;              ///???????????????????注意4倍
	*tar_y = g_Candidate[0].s_C_fuyangPixel;
}
