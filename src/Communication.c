#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "File_Queue.h"
#include "Init_dev.h"
#include "RunLength_Algorithm.h"
#include "Communication.h"
#include "qsImgLib.h"


#define PI 3.1415926


//***********************************************************

//解析控制台发送过来的TCP/IP协议指令

//***********************************************************

//网络协议传输相关命令定义***********************************
//跟踪器命令功能参数
	//红外跟踪命令相关参数***************
	char  IR_TrackerStatus = 0;//0x00——红外停止跟踪，0x01——红外准备跟踪，0x02——红外开始跟踪。
	char  IR_Cross = 0;        //0x00——红外不显示十字叉，0x01——红外显示十字叉。
	short IR_ShiChang_x = 0;   //红外水平视场，单位0.001度。
	short IR_ShiChang_y = 0;   //红外垂直视场，单位0.001度。
	short IR_Center_x = 0;     //红外跟踪中心水平坐标。
	short IR_Center_y = 0;     //红外跟踪中心垂直坐标。
	short IR_BoMen_x0 = 0;     //红外波门中心水平坐标。
	short IR_BoMen_y0 = 0;     //红外波门中心垂直坐标。
	short IR_BoMen_W = 0;      //红外波门宽度。
	short IR_BoMen_H = 0;      //红外波门高度。
	short IR_MuBiao_x0 = 0;    //红外目标中心水平坐标。
	short IR_MuBiao_y0 = 0;    //红外目标中心垂直坐标。
	short IR_MuBiao_W = 0;     //红外目标宽度。
	short IR_MuBiao_H = 0;     //红外目标高度。
	//************************************
	
	//可见光跟踪命令相关参数**************
	char  CCD_TrackerStatus = 0;//0x00——可见光停止跟踪，0x01——可见光准备跟踪，0x02——可见光开始跟踪。
	char  CCD_Cross = 0;        //0x00——可见光不显示十字叉，0x01——可见光显示十字叉。
	short CCD_ShiChang_x = 0;   //可见光水平视场，单位0.001度。
	short CCD_ShiChang_y = 0;   //可见光垂直视场，单位0.001度。
	short CCD_Center_x = 0;     //可见光跟踪中心水平坐标。
	short CCD_Center_y = 0;     //可见光跟踪中心垂直坐标。
	short CCD_BoMen_x0 = 0;     //可见光波门中心水平坐标。
	short CCD_BoMen_y0 = 0;     //可见光波门中心垂直坐标。
	short CCD_BoMen_W = 0;      //可见光波门宽度。
	short CCD_BoMen_H = 0;      //可见光波门高度。
	short CCD_MuBiao_x0 = 0;    //可见光目标中心水平坐标。
	short CCD_MuBiao_y0 = 0;    //可见光目标中心垂直坐标。
	short CCD_MuBiao_W = 0;     //可见光目标宽度。
	short CCD_MuBiao_H = 0;     //可见光目标高度。
	//************************************

	//读红外跟踪信息**********************
	char  IR_mubiao_Status = 0;     //0x00——失锁，0x01——锁定，0xFF——转台要求停止跟踪。
	short IR_MuBiao_x0_FeedBack = 0;//红外目标中心水平坐标。
	short IR_MuBiao_y0_FeedBack = 0;//红外目标中心垂直坐标。
	short IR_MuBiao_W_FeedBack = 0; //红外目标宽度。
	short IR_MuBiao_H_FeedBack = 0; //红外目标高度。
	//************************************
	
	//读可见光跟踪信息********************
	char  CCD_mubiao_Status = 0;     //0x00——失锁，0x01——锁定，0xFF——转台要求停止跟踪。
	short CCD_MuBiao_x0_FeedBack = 0;//可见光目标中心水平坐标。
	short CCD_MuBiao_y0_FeedBack = 0;//可见光目标中心垂直坐标。
	short CCD_MuBiao_W_FeedBack = 0; //可见光目标宽度。
	short CCD_MuBiao_H_FeedBack = 0; //可见光目标高度。
	//************************************

//告警器命令功能参数
	int startblock = 0;     //起始处理块   起始处理块位置，范围0~180（默认为0）
	int endblock = 0;	    //结束处理块   结束处理块位置，范围0~180（默认为180）
	int gray_low = 0;	    //灰度门限   （默认为0）
	int mComparedot = 0;    //告警能力    范围100~110 （默认104）
	int studytimes = 0;	    //学习次数    保留
	int distance = 0;       //固定目标的间距  保留
	int Probability = 0;    //固定目标出现概率  保留
	int isstudy = 0;        //是否学习        保留
	int ly = 0;             //ly为俯仰位置，单位0.000001度。
	int isIROrCCD = 0;      //1: IR 0: CCD
	int filter = 0;	        //滤波器      滤波器设置：0为无滤波 1为中值滤波（默认为0）
	int showtypes = 0;      //告警功能设置  0全不开，1运动目标，2静止目标， 4随机点， 5有批号的目标，6运动目标加随机目标（按照位修改）
	int BGBkitMethed = 0;   //背景去除  背景去除功能（替代学习功能）0为不去除，1为低效去除，2为中效去除，3为高效去除

//压缩板FPGA命令功能参数
	unsigned char begin_frame = 0;
	unsigned char end_frame = 0;


/***********************************************************
#define IR_Track 0
#define IR_Alarm 1
#define CCD_Track 2
#define CCD_Alarm 3

#define IR_SENSOR 0
#define CCD_SENSOR 1
#define TRACK_STATE 0
#define ALARM_STATE 1
***********************************************************/
unsigned char Current_Cmd_State;//当前跟踪搜索状态
unsigned char SENSOR_IR_CCD = 0;    //红外 可见光
unsigned char SENSOR_STATE = 0;     //跟踪 搜索

extern unsigned short VPU_Resolution ;    //VPU设置图像分辨率
extern unsigned char VPU_Format;         //VPU压缩格式
extern int quitflag;
extern unsigned char IR_Track_Begin_Flag ;
extern unsigned char CCD_Track_Begin_Flag ;
//***********************************************************
extern QSRECT rcInit;


int Analysis_Msg(void)
{
	int i;
	unsigned char ACK_IR[11]  = {0,0,0,0,0,0,0,0,0,0,0};
	unsigned char ACK_CCD[11] = {1,1,1,1,1,1,1,1,1,1,1};
	
	if((SENSOR_IR_CCD == 0)&&(SENSOR_STATE == 0)&&(Current_Cmd_State != 0))
	{
		quitflag = 1;
		if(Current_Cmd_State != 1)
			write(UART2_fd, ACK_IR, 11);
		VPU_Resolution = 512;//
		VPU_Format = 2;
		Current_Cmd_State = 0;
	}
	else if((SENSOR_IR_CCD == 0)&&(SENSOR_STATE == 1)&&(Current_Cmd_State != 1))
	{
		quitflag = 1;
		if(Current_Cmd_State != 0)
			write(UART2_fd, ACK_IR, 11);
		VPU_Resolution = 512;
		VPU_Format = 7;
		Current_Cmd_State = 1;
	}
	else if((SENSOR_IR_CCD == 1)&&(SENSOR_STATE == 0)&&(Current_Cmd_State != 2))
	{
		quitflag = 1;
		if(Current_Cmd_State != 3)
			write(UART2_fd, ACK_CCD, 11);
		VPU_Resolution = 544;
		VPU_Format = 2;
		Current_Cmd_State = 2;
		//unsigned char UART[8] = {0xC0,0xC0,0x03,0x03,0x55,0xAA,0x02,0xCF};
		//unsigned char UART[9] = {0xC0,0xC0,0x04,0x03,0x55,0xAA,0x00,0x02,0xCF};
		//write(UART1_fd,UART,9);
	}
	else if((SENSOR_IR_CCD == 1)&&(SENSOR_STATE == 1)&&(Current_Cmd_State != 3))
	{
		quitflag = 1;
		if(Current_Cmd_State != 2)
			write(UART2_fd, ACK_CCD, 11);
		VPU_Resolution = 544;
		VPU_Format = 7;
		Current_Cmd_State = 3;
	}
	else
	{
		//printf("No SWITCH THE VPU Camera******\n");
		return -1;
	}
	return 0;
}



int Command_Analysis_SwitchVideo(int bIrOrCCD)
{
	if(bIrOrCCD == 0)
	{
		SENSOR_IR_CCD = IR_SENSOR;
		//printf("\n");
		//printf("SENSOR_IR_CCD IR_SENSOR*****\n");
		//printf("\n");
	}
	else if(bIrOrCCD == 1)
	{
		SENSOR_IR_CCD = CCD_SENSOR;
		//printf("\n");
		//printf("SENSOR_IR_CCD CCD_SENSOR*****\n");
		//printf("\n");
	}
	else
	{
		//printf("Command_Analysis_SwitchVideo bIrOrCCD ERROR****\n");
		return -1;
	}
	return 0;
}

//响应
//***********************************************************

int Command_Analysis(int TCP_Accept_fd, unsigned char* buffer, unsigned int len)
{
	//char buffer[1024]= {0};
	int DATA_length=0;
	unsigned char DATA_buffer[1024];
	unsigned char CRC_sum = 0;
	unsigned char DATA_num = 0;        //数据中真实数据个数
	
	int i;
/*
	printf("来自客户端数据：");
	printf("len : %d\n",len);
	for(i=0; i<len; i++)
		printf("0x%x ",buffer[i]);
	printf("\n");
*/	
	if((buffer[0]==0xC0) && (buffer[1]==0xC0))
	{
		//注意命令长度包括：Packet_head_char（包头字节数）+ DATA_length_char（数据长度字节数）+ 
		//                  DATA_length（数据长度）+ CRC_sum_char（校验和字节数）+ 1（结束符）
		unsigned char Packet_head_char = 2;            //包头字节数
		unsigned char DATA_length_char = 1;            //数据长度字节数
		unsigned char CRC_sum_char = 1;                //校验和字节数
	
		//判断数据长度是否含0xCA
		if(buffer[Packet_head_char]==0xCA)
		{
			DATA_length_char=2;          
			DATA_length = buffer[Packet_head_char+DATA_length_char-1];           //数据长度
		}
		else
		{
			DATA_length_char=1;  
			DATA_length = buffer[Packet_head_char+DATA_length_char-1];           //数据长度
		}
		//判断校验和长度是否含0xCA
		if(buffer[Packet_head_char+DATA_length_char+DATA_length]==0xCA)
		{
			CRC_sum_char=2;
			CRC_sum = buffer[Packet_head_char+DATA_length_char+DATA_length+CRC_sum_char-1];
		}
		else
		{
			CRC_sum_char=1;
			CRC_sum = buffer[Packet_head_char+DATA_length_char+DATA_length];
		}
		
		//判断接收数据长度是否为命令长度
		if(len != (Packet_head_char+DATA_length_char+DATA_length+CRC_sum_char+1))
		{
			unsigned char ERROR_buffer[] = {0xC0,0xC0,0x03,0x00,0xFF,0x53,0x00,0xCF};	
			if(buffer[2] == 0xCA)                                   //判断数据长度是否为0xC0、0xCA、0xCF
				ERROR_buffer[3] = buffer[5];                        //自身设备代码
			else
				ERROR_buffer[3] = buffer[4]; 
			ERROR_buffer[6] = ERROR_buffer[3] + 0xFF + 0x53;       //校验和
			//send(TCP_Accept_fd, ERROR_buffer, 8, 0);
			printf("字符长度过长！！！\n");
			printf("Packet_length was wrong !!!!\n");
			return -1;
		}
		
		if(buffer[Packet_head_char+DATA_length_char+DATA_length+CRC_sum_char]==0xCF)    //判断包尾是否为0xCF
		{
			int i;
			unsigned char CRC_data=0;     //求出的校验和
			DATA_num = 0;
			 //解析出数据中的真实数据
			for(i=0; i<DATA_length; i++)                            
			{
				if(buffer[Packet_head_char+DATA_length_char+i]==0xCA)
				{
					DATA_buffer[DATA_num] = buffer[Packet_head_char+DATA_length_char+i+1];           //0xCA 后一位
					DATA_num++;
					i++;
				}
				else
				{
					DATA_buffer[DATA_num] = buffer[Packet_head_char+DATA_length_char+i];
					DATA_num++;
				}
			}
			//判断校验和
			for(i=0; i<DATA_num; i++)
				CRC_data+= DATA_buffer[i];
			if(CRC_data != CRC_sum)
			{
				printf("CRC_data:0x%x \n", CRC_data);
				printf("CRC wrong !!!\n");
				unsigned char CRC_ERROR_buffer[] = {0xC0,0xC0,0x03,0x00,0xFF,0x52,0x00,0xCF};					
				CRC_ERROR_buffer[3] = DATA_buffer[0];                        //自身设备代码
				CRC_ERROR_buffer[6] = CRC_ERROR_buffer[3] + 0xFF + 0x52;     //校验和
				//send(TCP_Accept_fd, CRC_ERROR_buffer, 8, 0);
				return -1;
			}
			
			//解析数据内容***********************************************************************************
			//
			//跟踪器（地址码：0x06）
			if(DATA_buffer[0]==0x06)       
			{
				//printf("***************************跟踪器（地址码：0x06）***************************\n");
				//判断命令内容
				//if((DATA_buffer[1]==0x00)&&(DATA_num ==28))   //红外跟踪   0x00
				if((DATA_buffer[1]==0x00)&&(DATA_num ==19))   //红外跟踪   0x00
				{
					if(DATA_buffer[2]== 0x00)        //0x00——红外停止跟踪
					{
						//printf("红外停止跟踪!!!\n");
					}
					if(DATA_buffer[2]== 0x01)        //0x01——红外准备跟踪
					{
						//printf("红外准备跟踪!!!\n");
					}
					if(DATA_buffer[2]== 0x02)        //0x01——红外开始跟踪
					{
						//printf("红外开始跟踪!!!\n");
					}
					IR_TrackerStatus  = DATA_buffer[2];                     //0x00——红外停止跟踪，0x01——红外准备跟踪，0x02——红外开始跟踪。
					//IR_Cross          = DATA_buffer[3];                     //0x00——红外不显示十字叉，0x01——红外显示十字叉。
					IR_ShiChang_x     = DATA_buffer[3]|(DATA_buffer[4]<<8);//红外水平视场，单位0.001度。
					IR_ShiChang_y     = DATA_buffer[5]|(DATA_buffer[6]<<8);//红外垂直视场，单位0.001度。
					//printf(" IR_ShiChang_x = %d    IR_ShiChang_y = %d ********\n",IR_ShiChang_x,IR_ShiChang_y);
					IR_Center_x       = DATA_buffer[7]|(DATA_buffer[8]<<8);//红外跟踪中心水平坐标。
					IR_Center_y       = DATA_buffer[9]|(DATA_buffer[10]<<8);//红外跟踪中心垂直坐标。
					//printf(" IR_Center_x = %d    IR_Center_y = %d ********\n",IR_Center_x,IR_Center_y);
					IR_BoMen_x0       = DATA_buffer[11]|(DATA_buffer[12]<<8);//红外波门中心水平坐标。
					IR_BoMen_y0       = DATA_buffer[13]|(DATA_buffer[14]<<8);//红外波门中心垂直坐标。
					//printf(" IR_BoMen_x0 = %d    IR_BoMen_y0 = %d ********\n",IR_BoMen_x0,IR_BoMen_y0);
					IR_BoMen_W        = DATA_buffer[15]|(DATA_buffer[16]<<8);//红外波门宽度。
					IR_BoMen_H        = DATA_buffer[17]|(DATA_buffer[18]<<8);//红外波门高度。
					//printf(" IR_BoMen_W = %d    IR_BoMen_H = %d ********\n",IR_BoMen_W,IR_BoMen_H);
					//IR_MuBiao_x0      = DATA_buffer[20]|(DATA_buffer[21]<<8);//红外目标中心水平坐标。
					//IR_MuBiao_y0      = DATA_buffer[22]|(DATA_buffer[23]<<8);//红外目标中心垂直坐标。
					//IR_MuBiao_W       = DATA_buffer[24]|(DATA_buffer[25]<<8);//红外目标宽度。
					//IR_MuBiao_H       = DATA_buffer[26]|(DATA_buffer[27]<<8);//红外目标高度。
					if(IR_TrackerStatus == 0x01)
						IR_Track_Begin_Flag = 1;
					else
						IR_Track_Begin_Flag = 0;
				
				}
				//else if((DATA_buffer[1]==0x01)&&(DATA_num ==28))   //可见光跟踪  0x01
				else if((DATA_buffer[1]==0x01)&&(DATA_num ==19))   //可见光跟踪  0x01
				{
					if(DATA_buffer[2]== 0x00)        //0x00——可见光停止跟踪
					{
						//printf("可见光停止跟踪!!!\n");
					}
					if(DATA_buffer[2]== 0x01)        //0x01——可见光准备跟踪
					{
						//printf("可见光准备跟踪!!!\n");
					}
					if(DATA_buffer[2]== 0x02)        //0x01——可见光开始跟踪
					{
						//printf("可见光开始跟踪!!!\n");
					}
					
					CCD_TrackerStatus = DATA_buffer[2];                     //0x00——可见光停止跟踪，0x01——可见光准备跟踪，0x02——可见光开始跟踪。
					//CCD_Cross         = DATA_buffer[3];                     //0x00——可见光不显示十字叉，0x01——可见光显示十字叉。
					CCD_ShiChang_x    = DATA_buffer[3]|(DATA_buffer[4]<<8);//可见光水平视场，单位0.001度。
					CCD_ShiChang_y    = DATA_buffer[5]|(DATA_buffer[6]<<8);//可见光垂直视场，单位0.001度。
					//printf(" CCD_ShiChang_x = %d    CCD_ShiChang_y = %d ********\n",CCD_ShiChang_x,CCD_ShiChang_y);
					CCD_Center_x      = DATA_buffer[7]|(DATA_buffer[8]<<8);//可见光跟踪中心水平坐标。
					CCD_Center_y      = DATA_buffer[9]|(DATA_buffer[10]<<8);//可见光跟踪中心垂直坐标。
					//printf(" CCD_Center_x = %d    CCD_Center_y = %d ********\n",CCD_Center_x,CCD_Center_y);
					CCD_BoMen_x0      = DATA_buffer[11]|(DATA_buffer[12]<<8);//可见光波门中心水平坐标。
					CCD_BoMen_y0      = DATA_buffer[13]|(DATA_buffer[14]<<8);//可见光波门中心垂直坐标。
					//printf(" CCD_BoMen_x0 = %d    CCD_BoMen_y0 = %d ********\n",CCD_BoMen_x0,CCD_BoMen_y0);
					CCD_BoMen_W       = DATA_buffer[15]|(DATA_buffer[16]<<8);//可见光波门宽度。
					CCD_BoMen_H       = DATA_buffer[17]|(DATA_buffer[18]<<8);//可见光波门高度。
					//printf(" CCD_BoMen_W = %d    CCD_BoMen_H = %d ********\n",CCD_BoMen_W,CCD_BoMen_H);
					//CCD_MuBiao_x0     = DATA_buffer[20]|(DATA_buffer[21]<<8);//可见光目标中心水平坐标。
					//CCD_MuBiao_y0     = DATA_buffer[22]|(DATA_buffer[23]<<8);//可见光目标中心垂直坐标。
					//CCD_MuBiao_W      = DATA_buffer[24]|(DATA_buffer[25]<<8);//可见光目标宽度。
					//CCD_MuBiao_H      = DATA_buffer[26]|(DATA_buffer[27]<<8);//可见光目标高度。
					
					if(CCD_TrackerStatus == 0x01)
					{
						rcInit.left = CCD_BoMen_x0 - CCD_BoMen_W/2;
						rcInit.right = CCD_BoMen_x0 + CCD_BoMen_W/2;
						rcInit.top = CCD_BoMen_y0 - CCD_BoMen_H/2;
						rcInit.bottom = CCD_BoMen_y0 + CCD_BoMen_H/2;
						CCD_Track_Begin_Flag = 1;
						//unsigned char UART[8] = {0xC0,0xC0,0x03,0x03,0x55,0xAA,0x02,0xCF};
						//unsigned char UART[9] = {0xC0,0xC0,0x04,0x03,0x55,0xAA,0x00,0x02,0xCF};
						//write(UART1_fd,UART,9);
					}
					else
					{
						CCD_Track_Begin_Flag = 0;
						//unsigned char UART[8] = {0xC0,0xC0,0x03,0x03,0x55,0xAA,0x02,0xCF};
						//unsigned char UART[9] = {0xC0,0xC0,0x04,0x03,0x55,0x55,0x00,0xAD,0xCF};
						//write(UART1_fd,UART,8);
					}
				}
				else if((DATA_buffer[1]==0x02)&&(DATA_num ==2))  // 读红外跟踪信息  0x02
				{
					//无命令附加数据
					//printf("reading Infrared image information!!!\n");									

					unsigned char temp[9] = {0};
					unsigned char CRC = 0;
					unsigned char DATA_buffer_length = 11;     //初始命令数据长度 ：目标设备 命令代码 数据（9Byte）
					unsigned char i;
					unsigned char DATA_buffer_length_char = 1;
					unsigned char CRC_char = 1;
					//仿造测试
					IR_mubiao_Status        =  0x00;   //0x00——失锁，0x01——锁定，0xFF——转台要求停止跟踪。
					IR_MuBiao_x0_FeedBack   =  0x00C7; //红外目标中心水平坐标。
					IR_MuBiao_y0_FeedBack   =  0x0000; //红外目标中心垂直坐标。
					IR_MuBiao_W_FeedBack    =  0x0000; //红外目标宽度。
					IR_MuBiao_H_FeedBack    =  0x0000; //红外目标高度。
					
					temp[0] = IR_mubiao_Status;               //0x00——失锁，0x01——锁定，0xFF——转台要求停止跟踪。
					temp[1] = IR_MuBiao_x0_FeedBack & 0xFF;   //红外目标中心水平坐标。
					temp[2] = IR_MuBiao_x0_FeedBack >> 8;
					temp[3] = IR_MuBiao_y0_FeedBack & 0xFF;   //红外目标中心垂直坐标。
					temp[4] = IR_MuBiao_y0_FeedBack >> 8;
					temp[5] = IR_MuBiao_W_FeedBack & 0xFF;    //红外目标宽度。
					temp[6] = IR_MuBiao_W_FeedBack >> 8;
					temp[7] = IR_MuBiao_H_FeedBack & 0xFF;    //红外目标高度
					temp[8] = IR_MuBiao_H_FeedBack >> 8;
					for(i=0; i<9; i++)
					{
						if((temp[i]==0xC0)||(temp[i]==0xCA)||(temp[i]==0xCF))
							DATA_buffer_length ++;
						CRC += temp[i];
					}
					CRC += DATA_buffer[0];                                  //DATA_buffer[0]应该为0x06
					CRC += 0x02;                                            //响应码
					if((CRC==0xC0)||(CRC==0xCA)||(CRC==0xCF))
						CRC_char ++;
					if((DATA_buffer_length==0xC0)||(DATA_buffer_length==0xCA)||(DATA_buffer_length==0xCF))
						DATA_buffer_length_char ++;
					unsigned char response_buffer_length = 3 + DATA_buffer_length_char + DATA_buffer_length + CRC_char;   //包头包尾 = 3
					//printf("response_buffer_length = %d \n", response_buffer_length);
					unsigned char response_buffer[50]={0};                 //
					unsigned char num = 3;
					
					//将被读取的数据写入response_buffer[]缓存	
					response_buffer[0] = 0xC0;
					response_buffer[1] = 0xC0;
					response_buffer[response_buffer_length-1] = 0xCF;
					if(DATA_buffer_length_char==2)
					{
						response_buffer[2] = 0xCA;
						response_buffer[3] = DATA_buffer_length;
						num++;
					}
					else
						response_buffer[2] = DATA_buffer_length;
					
					response_buffer[response_buffer_length-2] = CRC;          //校验和
					if(CRC_char==2)
						response_buffer[response_buffer_length-3] = 0xCA;
					response_buffer[num] = 0x06;                              //自身设备代码 
					num++;
					response_buffer[num] = 0x02;                              //命令代码
					num++;
					for(i=0; i<9; i++)
					{
						if((temp[i]==0xC0)||(temp[i]==0xCA)||(temp[i]==0xCF))
						{
							response_buffer[num] = 0xCA;
							num++;
							response_buffer[num] = temp[i];
						}
						else
							response_buffer[num] = temp[i];
						num++;
					}
					for(i=0; i<response_buffer_length; i++)
						printf("0x%x ",response_buffer[i]);
					//printf("\n");
					//send(TCP_Accept_fd, response_buffer, response_buffer_length, 0);
				}	
				else if((DATA_buffer[1]==0x03)&&(DATA_num ==2))  // 读可见光跟踪信息  0x03
				{
					//无命令附加数据
					//printf("reading Visible light image information!!!\n");
					unsigned char temp[9] = {0};
					unsigned char CRC = 0;
					unsigned char DATA_buffer_length = 11;     //初始命令数据长度 ：目标设备 命令代码 数据（9Byte）
					unsigned char i;
					unsigned char DATA_buffer_length_char = 1;
					unsigned char CRC_char = 1;
					//仿造测试
					CCD_mubiao_Status       = 0x01;     //0x00——失锁，0x01——锁定，0xFF——转台要求停止跟踪。
					CCD_MuBiao_x0_FeedBack  = 0x56C0;   //可见光目标中心水平坐标。
					CCD_MuBiao_y0_FeedBack  = 0xCF89;   //可见光目标中心垂直坐标。
					CCD_MuBiao_W_FeedBack   = 0xC0C0;   //可见光目标宽度。
					CCD_MuBiao_H_FeedBack   = 0x4581;   //可见光目标高度。
					
					temp[0] = CCD_mubiao_Status;               //0x00——失锁，0x01——锁定，0xFF——转台要求停止跟踪。
					temp[1] = CCD_MuBiao_x0_FeedBack & 0xFF;   //红外目标中心水平坐标。
					temp[2] = CCD_MuBiao_x0_FeedBack >> 8;
					temp[3] = CCD_MuBiao_y0_FeedBack & 0xFF;   //红外目标中心垂直坐标。
					temp[4] = CCD_MuBiao_y0_FeedBack >> 8;
					temp[5] = CCD_MuBiao_W_FeedBack & 0xFF;    //红外目标宽度。
					temp[6] = CCD_MuBiao_W_FeedBack >> 8;
					temp[7] = CCD_MuBiao_H_FeedBack & 0xFF;    //红外目标高度。
					temp[8] = CCD_MuBiao_H_FeedBack >> 8;
					for(i=0; i<9; i++)
					{
						if((temp[i]==0xC0)||(temp[i]==0xCA)||(temp[i]==0xCF))
							DATA_buffer_length ++;
						CRC += temp[i];
					}
					CRC += DATA_buffer[0];                                  //DATA_buffer[0]应该为0x06
					CRC += 0x02;                                            //响应码
					if((CRC==0xC0)||(CRC==0xCA)||(CRC==0xCF))
						CRC_char ++;
					if((DATA_buffer_length==0xC0)||(DATA_buffer_length==0xCA)||(DATA_buffer_length==0xCF))
						DATA_buffer_length_char ++;
					unsigned char response_buffer_length = 3 + DATA_buffer_length_char + DATA_buffer_length + CRC_char;   //包头包尾 = 3
					//printf("response_buffer_length = %d \n", response_buffer_length);
					unsigned char response_buffer[50]={0};                 //
					unsigned char num = 3;
					
					//将被读取的数据写入response_buffer[]缓存	
					response_buffer[0] = 0xC0;
					response_buffer[1] = 0xC0;
					response_buffer[response_buffer_length-1] = 0xCF;
					if(DATA_buffer_length_char==2)
					{
						response_buffer[2] = 0xCA;
						response_buffer[3] = DATA_buffer_length;
						num++;
					}
					else
						response_buffer[2] = DATA_buffer_length;
					
					response_buffer[response_buffer_length-2] = CRC;          //校验和
					if(CRC_char==2)
						response_buffer[response_buffer_length-3] = 0xCA;
					response_buffer[num] = 0x06;                              //自身设备代码 
					num++;
					response_buffer[num] = 0x02;                              //命令代码
					num++;
					for(i=0; i<9; i++)
					{
						if((temp[i]==0xC0)||(temp[i]==0xCA)||(temp[i]==0xCF))
						{
							response_buffer[num] = 0xCA;
							num++;
							response_buffer[num] = temp[i];
						}
						else
							response_buffer[num] = temp[i];
						num++;
					}
					for(i=0; i<response_buffer_length; i++)
						printf("0x%x ",response_buffer[i]);
					//printf("\n");
					//send(TCP_Accept_fd, response_buffer, response_buffer_length, 0);
				}
				else
				{
					unsigned char ERROR_buffer[] = {0xC0,0xC0,0x03,0x00,0xFF,0x55,0x00,0xCF};					
					ERROR_buffer[3] = DATA_buffer[0];                    //自身设备代码
					ERROR_buffer[6] = ERROR_buffer[3] + 0xFF + 0x55;     //校验和
					//send(TCP_Accept_fd, ERROR_buffer, 8, 0);
					printf("***非法数据，命令代码不正确！！！***\n");
					return -1;
				}
			}
			
			//告警器（地址码：0x07）
			else if(DATA_buffer[0]==0x07)
			{
				//printf("***************************告警器（地址码：0x07）***************************\n");
				if(DATA_num == 6)
				{
					if(DATA_buffer[1]==0x00)   //起始处理块
					{
						startblock = DATA_buffer[2]|(DATA_buffer[3]<<8)|(DATA_buffer[4]<<16)|(DATA_buffer[5]<<24);
						//printf("startblock 0x01= 0x%x \n",startblock);
					}
					if(DATA_buffer[1]==0x01)   //结束处理块
					{
						endblock   = DATA_buffer[2]|(DATA_buffer[3]<<8)|(DATA_buffer[4]<<16)|(DATA_buffer[5]<<24);
						//printf("endblock 0x01= 0x%x \n",endblock);
					}
					if(DATA_buffer[1]==0x02)   //灰度门限
					{
						gray_low   = DATA_buffer[2]|(DATA_buffer[3]<<8)|(DATA_buffer[4]<<16)|(DATA_buffer[5]<<24);
						//printf("gray_low 0x02= 0x%x \n",gray_low);
					}
					if(DATA_buffer[1]==0x03)   //告警能力
					{
						mComparedot = DATA_buffer[2]|(DATA_buffer[3]<<8)|(DATA_buffer[4]<<16)|(DATA_buffer[5]<<24);
						//printf("mComparedot 0x03= 0x%x \n",mComparedot);
					}	
					if(DATA_buffer[1]==0x04)   //学习次数
					{
						studytimes = DATA_buffer[2]|(DATA_buffer[3]<<8)|(DATA_buffer[4]<<16)|(DATA_buffer[5]<<24);
						//printf("studytimes 0x04= 0x%x \n",studytimes);
					}	
					if(DATA_buffer[1]==0x05)   //固定目标的间距
					{
						distance   = DATA_buffer[2]|(DATA_buffer[3]<<8)|(DATA_buffer[4]<<16)|(DATA_buffer[5]<<24);
						//printf("distance 0x05= 0x%x \n",distance);
					}	
					if(DATA_buffer[1]==0x06)   //固定目标出现概率
					{
						Probability = DATA_buffer[2]|(DATA_buffer[3]<<8)|(DATA_buffer[4]<<16)|(DATA_buffer[5]<<24);
						//printf("Probability 0x06= 0x%x \n",Probability);
					}
					if(DATA_buffer[1]==0x07)   //是否学习
					{
						isstudy     = DATA_buffer[2]|(DATA_buffer[3]<<8)|(DATA_buffer[4]<<16)|(DATA_buffer[5]<<24);
						//printf("isstudy 0x07= 0x%x \n",isstudy);
					}
					if(DATA_buffer[1]==0x08)   //是否是新的参数
					{
						ly     = DATA_buffer[2]|(DATA_buffer[3]<<8)|(DATA_buffer[4]<<16)|(DATA_buffer[5]<<24);
						//printf("ly 0x08= 0x%x \n",ly);
					}
					if(DATA_buffer[1]==0x09)   //滤波器
					{
						filter      = DATA_buffer[2]|(DATA_buffer[3]<<8)|(DATA_buffer[4]<<16)|(DATA_buffer[5]<<24);
						//printf("filter 0x09= 0x%x \n",filter);
					}
					if(DATA_buffer[1]==0x0A)   //1: IR 0: CCD
					{
						isIROrCCD   = DATA_buffer[2]|(DATA_buffer[3]<<8)|(DATA_buffer[4]<<16)|(DATA_buffer[5]<<24);
						//printf("isIROrCCD 0x0A= 0x%x \n",isIROrCCD);
						//************************//1: IR 0: CCD**********************
						if(isIROrCCD == 1)
						{
							SENSOR_IR_CCD = IR_SENSOR;
							//printf("\n");
							//printf("SENSOR_IR_CCD IR_SENSOR*****\n");
							//printf("\n");
						}
						else if(isIROrCCD == 0)
						{
							SENSOR_IR_CCD = CCD_SENSOR;
							//printf("\n");
							//printf("SENSOR_IR_CCD CCD_SENSOR*****\n");
							//printf("\n");
						}
						//***********************************************************
					}
					if(DATA_buffer[1]==0x0B)   //保留
					if(DATA_buffer[1]==0x0C)   //保留
					if(DATA_buffer[1]==0x0D)   //告警功能设置
					{
						showtypes   = DATA_buffer[2]|(DATA_buffer[3]<<8)|(DATA_buffer[4]<<16)|(DATA_buffer[5]<<24);
						printf("showtypes 0x0D= 0x%x \n",showtypes);
					}	
					if(DATA_buffer[1]==0x0E)   //背景去除
					{
						BGBkitMethed = DATA_buffer[2]|(DATA_buffer[3]<<8)|(DATA_buffer[4]<<16)|(DATA_buffer[5]<<24);
						printf("BGBkitMethed 0x0E= 0x%x \n",BGBkitMethed);
					}
				}
				else
				{
					unsigned char ERROR_buffer[] = {0xC0,0xC0,0x03,0x00,0xFF,0x55,0x00,0xCF};					
					ERROR_buffer[3] = DATA_buffer[0];                    //自身设备代码
					ERROR_buffer[6] = ERROR_buffer[3] + 0xFF + 0x55;     //校验和
					//send(TCP_Accept_fd, ERROR_buffer, 8, 0);
					printf("***非法数据，命令代码不正确！！！***\n");
					return -1;
				}
			}
			
			//压缩板FPGA指令（0x0D）
			else if(DATA_buffer[0]==0x0D)
			{
				//printf("***************************压缩板FPGA指令(0x0D)***************************\n");
				//搜索、跟踪状态切换
				if((DATA_buffer[1]==0x01)&&(DATA_num ==3))
				{
					if(DATA_buffer[2]==0x55)  
					{
						//printf("0x55：搜索!!!\n");	
						SENSOR_STATE = ALARM_STATE;
					}
					else if(DATA_buffer[2]==0xAA)
					{
						//printf("0xAA：跟踪!!!\n");
						SENSOR_STATE = TRACK_STATE;
					}
					else
					{
						printf("压缩板FPGA指令 ERROR!!!\n");
					}
					//***********************************************************************************
					/*int i,j;
					unsigned char ERROR_buffer[256] = {0};	
					for(i=0;i<256;i++)
						ERROR_buffer[i]  = i;
					for(i=0; i<1024; i++)
						for(j=0; j<1024; j++)
						{
							send(TCP_Accept_fd, ERROR_buffer, 256, 0);
						}
							
					printf("发送1MByte *******Successed!!!!!\n");*/
					
					
					//***********************************************************************************
				}
				//切换自动计算阈值
				else if((DATA_buffer[1]==0x02)&&(DATA_num == 4))
				{
					//printf("起始帧~结束帧!!!\n");
					begin_frame = DATA_buffer[2];
					end_frame   = DATA_buffer[3];
					//printf("起始帧: %d 结束帧: %d  !!!\n",begin_frame,end_frame);
				}
				else
				{
					unsigned char ERROR_buffer[] = {0xC0,0xC0,0x03,0x00,0xFF,0x55,0x00,0xCF};					
					//ERROR_buffer[3] = DATA_buffer[0];                  //自身设备代码
					ERROR_buffer[3] = 0x06;                              //由于自身设备码不正确，统一使用0x06 设备代码
					ERROR_buffer[6] = ERROR_buffer[3] + 0xFF + 0x55;     //校验和
					//send(TCP_Accept_fd, ERROR_buffer, 8, 0);
					printf("***非法数据，命令代码不正确！！！***\n");
					return -1;
				}
			}
			else
			{
				unsigned char ERROR_buffer[] = {0xC0,0xC0,0x03,0x00,0xFF,0x55,0x00,0xCF};					
				//ERROR_buffer[3] = DATA_buffer[0];                  //自身设备代码
				ERROR_buffer[3] = 0x06;                              //由于自身设备码不正确，统一使用0x06 设备代码
				ERROR_buffer[6] = ERROR_buffer[3] + 0xFF + 0x55;     //校验和
				//send(TCP_Accept_fd, ERROR_buffer, 8, 0);
				printf("***非法数据，命令代码不正确！！！***\n");
				return -1;
			}
			//
			//printf("**************************解析successed   !!!*****************************\n");
			//printf("\n");
			//printf("**************************************************************************\n");
			//printf("\n");
			
		}
		else
		{
			printf("Char of end was wrong !!!\n");
		}
	}
	else
	{
		printf("Packet_head was wrong!!!\n");
	}
	return 0;
}


int ADD_CA_COM_MSG(char *pComMSG, int ComMSG_Len, char *p_ComMSG_CA, int *ComMSG_CA_Len)
{
	int i;
	int Input_MSG_Len = 0;
	int Output_MSG_Len = 0;
	Input_MSG_Len = ComMSG_Len;
	if((pComMSG[0]!=0xC0)||(pComMSG[0]!=0xC0)||(pComMSG[ComMSG_Len-1]!=0xCF))
	{
		//printf("COMMOM_MSG ADD 0xCA is ERROR*********\n");
		return -1;
	}
	//****************************************************************
	//将头部0xC0,0xC0赋给输出
	for(i=0; i<2; i++)
	{
		p_ComMSG_CA[Output_MSG_Len] = pComMSG[i];
		Output_MSG_Len++;
	}
	//************存在0xC0,0xCA,0xCF的数据前面添加0xCA****************
	for(i=2; i<Input_MSG_Len-1; i++)
	{	
		if((pComMSG[i]==0xC0)||(pComMSG[i]==0xCA)||(pComMSG[i]==0xCF))
		{
			p_ComMSG_CA[Output_MSG_Len] = 0xCA;
			Output_MSG_Len++;
			p_ComMSG_CA[Output_MSG_Len] = pComMSG[i];
		}
		else
		{
			p_ComMSG_CA[Output_MSG_Len] = pComMSG[i];
			Output_MSG_Len++;
		}
	}
	//将尾部0xCF赋给输出
	p_ComMSG_CA[Output_MSG_Len] = pComMSG[Input_MSG_Len-1];
	Output_MSG_Len++;
	*ComMSG_CA_Len = Output_MSG_Len;
	return 0;
}

extern TARFEATURE	*g_Candidate;

int Send_Alarm_to_PC(int Frame_NUM, TARFEATURE	*g_Candidate, unsigned char m_PossTarNum,unsigned char sensor)
//int Send_Alarm_to_PC(int Frame_NUM, unsigned char m_PossTarNum, unsigned char sensor)
{	
	int i,ret;
	int pLen;
	char p_ComMSG_CA[256];
	unsigned char PC422_Send[256] = {0};
	PC422_Send[0] = 0xC0;
	PC422_Send[1] = 0xC0;
	PC422_Send[2] = 0x25;    //5+4*8
	PC422_Send[3] = 0x07;
	PC422_Send[4] = 0x0B;
	PC422_Send[5] = 1-sensor;  //红外还是可见光? 0:可见光 1：红外
	PC422_Send[6] = Frame_NUM ;   // 帧号
	PC422_Send[7] = 0x01; 
	if(m_PossTarNum >= 4)
		m_PossTarNum = 4;

	for(i=0; i<4; i++)
		PC422_Send[8 + i*8 + 1] = i+1;//四个目标号
	for(i=0; i<m_PossTarNum; i++)
	{
		PC422_Send[8 + i*8] = 0;
		//PC422_Send[8 + i*8 + 1] = i+1;//四个目标号
		PC422_Send[8 + i*8 + 2] = g_Candidate[i].s_C_fangweiPixel-4 ;
		PC422_Send[8 + i*8 + 3] = (g_Candidate[i].s_C_fangweiPixel-4 & 0xff00)>>8;
		PC422_Send[8 + i*8 + 4] = g_Candidate[i].s_C_fuyangPixel ;
		PC422_Send[8 + i*8 + 5] = (g_Candidate[i].s_C_fuyangPixel & 0xff00)>>8;

		//PC422_Send[8 + i*8 + 2] = 320 & 0xff;
		//PC422_Send[8 + i*8 + 3] = (320 & 0xff00)>>8;
		//PC422_Send[8 + i*8 + 4] = 256 & 0xff;
		//PC422_Send[8 + i*8 + 5] = (256 & 0xff00)>>8;
		
		PC422_Send[8 + i*8 + 6] = 0;   
		PC422_Send[8 + i*8 + 7] = 0;
	}
	for(i=3; i<40; i++)		
		PC422_Send[40] +=  PC422_Send[i];
	PC422_Send[41] = 0xCF;
	if(ADD_CA_COM_MSG(PC422_Send, 42, p_ComMSG_CA, &pLen) < 0)
		return -1;
	/*
	printf("\n");
	for(i=0; i<42; i++)
		printf("0x%02x ",PC422_Send[i]);
	printf("\n");
	
	printf("pLen: %d \n",pLen);
	for(i=0; i<pLen; i++)
		printf("0x%02x ",p_ComMSG_CA[i]);
	printf("\n");
	*/
	if(Send_CMD(p_ComMSG_CA, pLen)< 0)
		return -1;
	return 0;
}


int Send_Target_to_PC(unsigned short x, unsigned short y, unsigned char sensor)
{
	int i,ret;
	int  pLen;
	char p_ComMSG_CA[256];
	unsigned char PC422_Send[256] = {0};
	PC422_Send[0] = 0xC0;
	PC422_Send[0] = 0xC0;
	PC422_Send[2] = 0x0B;
	PC422_Send[3] = 0x06;
	if(sensor == 0 ) //0: IR ,1:CCD
		PC422_Send[4] = 0x02;
	else
		PC422_Send[4] = 0x03;
	PC422_Send[5] = x & 0xFF;
	PC422_Send[6] = x >> 8;
	PC422_Send[7] = y & 0xFF;
	PC422_Send[8] = y >> 8;
	PC422_Send[9] = 48 & 0xFF;
	PC422_Send[10] = 48 >> 8;
	PC422_Send[11] = 48 & 0xFF;
	PC422_Send[12] = 48 >> 8;
	for(i=3; i<13; i++)
		PC422_Send[13] += PC422_Send[i];
	PC422_Send[14] = 0xCF;

	if(ADD_CA_COM_MSG(PC422_Send, 15, p_ComMSG_CA, &pLen) < 0)
		return -1;
	write(UART1_fd, p_ComMSG_CA, pLen);
}

int Send_Target_to_SIFU(unsigned short x, unsigned short y, unsigned char sensor)
{
	int Target_x,Target_y;
	int i,ret;
	int pLen;
	char p_ComMSG_CA[256];
	unsigned char PC422_Send[256] = {0};
	PC422_Send[0] = 0xC0;
	PC422_Send[1] = 0xC0;
	PC422_Send[2] = 0x0C;//长度12
	PC422_Send[3] = 0x03;//0x05 | 0x03
	PC422_Send[4] = 0x14;//0xAA | 0x14
	//*****************************************************
	if(sensor == 0)
	{
		Target_x = (int)((320-x)*IR_ShiChang_x*1000*0.75/640);
		Target_y = (int)((256-y)*IR_ShiChang_y*1000*1.2/512);
	}	
	else
	{
		Target_x = (int)((480-x)*CCD_ShiChang_x*1000/960);
		Target_y = (int)((272-y)*CCD_ShiChang_y*1000*1.1/544);
	}
	//printf("x=%d y=%d CCD_ShiChang_x=%d CCD_ShiChang_y=%d \n ",x,y,CCD_ShiChang_x,CCD_ShiChang_y);
	//printf("********Target_x = %d Target_y = %d  ******\n",Target_x,Target_y);
	//printf("********Target_x = 0x%08x Target_y = 0x%08x  ******\n",Target_x,Target_y);
	PC422_Send[5] = Target_x >> 24;
	PC422_Send[6] = Target_x >> 16;
	PC422_Send[7] = Target_x >> 8;
	PC422_Send[8] = Target_x;
	PC422_Send[9] = Target_y >> 24;
	PC422_Send[10] = Target_y >> 16;
	PC422_Send[11] = Target_y >> 8;
	PC422_Send[12] = Target_y;
	if(sensor == 0)
	{
		PC422_Send[13] = (short)(IR_ShiChang_x>>8) & 0xFF;
		PC422_Send[14] = IR_ShiChang_x & 0xFF;
	}	
	else
	{
		PC422_Send[13] = (short)(CCD_ShiChang_x>>8) & 0xFF;
		PC422_Send[14] = CCD_ShiChang_x & 0xFF;
	}
	for(i=3; i<15; i++)
		PC422_Send[15] += PC422_Send[i];
	PC422_Send[16] = 0xCF;
	/*
	printf("Send to SIFU:");
	for(i=0; i<17; i++)
		printf("0x%02x ",PC422_Send[i]);
	printf("\n");
	*/
	if(ADD_CA_COM_MSG(PC422_Send, 17, p_ComMSG_CA, &pLen) < 0)
		return -1;
	/*
	printf("Send to SIFU:");
	for(i=0; i<pLen; i++)
		printf("0x%02x ",p_ComMSG_CA[i]);
	printf("\n");
	*/
	write(UART1_fd, p_ComMSG_CA, pLen);
}



//**********************************************************************
#define BUF_MAX2 64              //接收长度
#define BUF_MAX1 64              //接收长度

SqQueue         exPayLoadRXFrameBuff2;
SqQueue         exPayLoadRXFrameBuff1;
unsigned char   dataBuff_R2[BUF_MAX2];
unsigned char   dataBuff_R1[BUF_MAX1];
unsigned char   PL_exPayLoadRXFrameBuff2[2*BUF_MAX2];
unsigned char   PL_exPayLoadRXFrameBuff1[2*BUF_MAX1];
//**********************************************************************


void PayLoad_Init1(void) 
{
	exPayLoadRXFrameBuff1.base=PL_exPayLoadRXFrameBuff1;
	QueueInit(&exPayLoadRXFrameBuff1, 2*BUF_MAX1);
} 

void PayLoad_Init2(void)
{
	exPayLoadRXFrameBuff2.base=PL_exPayLoadRXFrameBuff2;
	QueueInit(&exPayLoadRXFrameBuff2, 2*BUF_MAX2);
}

//***************跟踪目标相关参数****************************
extern unsigned char mubiaodiushi;
extern unsigned char PAL_SDI_Track;
extern unsigned char Track_auto_flag ;  //跟踪手动标志位
extern unsigned char Track_auto_statue; //跟踪手动状态
extern int COMPRESS_continue;           //持续压缩标志位，用于跳出不同分辨率的压缩模式
extern unsigned char SDI_BeginTrack_flag;
extern unsigned char PAL_BeginTrack_flag;
extern int socketCon;
unsigned char socket_buf_SDI[] = {0x68,0x01,0x68,0x01,0x00,0x01,0x16};
unsigned char socket_buf_PAL[] = {0x68,0x01,0x68,0x01,0x01,0x01,0x16};
extern int quitflag;


void PayLoad_TaskRx1(int uartname)
{
 	unsigned char   Uart1Buff[BUF_MAX1];
	unsigned char   dataBuff[BUF_MAX1];
	unsigned char   data_recv_catched = 0;
	unsigned char   plFrameHead[2] = {0x7e,0x16};        
	unsigned char   plFrameTail = 0;             
	unsigned short  crcvalue = 0,checksum = 0;
	int uart1_read_len=0;
 	uart1_read_len= read(uartname, Uart1Buff,sizeof(Uart1Buff)); 
	QueuePushBack(&exPayLoadRXFrameBuff1,(unsigned char *)Uart1Buff,uart1_read_len);

    while (QueueLength(&exPayLoadRXFrameBuff1)>= BUF_MAX1)     
    {        
		uart1_read_len=  QueueLength(&exPayLoadRXFrameBuff1); 
        if (FrameCompare(&exPayLoadRXFrameBuff1,plFrameHead,2,&plFrameTail,0,uart1_read_len) )
        {
			QueuePopFront(&exPayLoadRXFrameBuff1,dataBuff_R1,BUF_MAX1,1);
			data_recv_catched = 1;
		}		
        else
        {
			QueuePopFront(&exPayLoadRXFrameBuff1,0,1,1);
        }
		
    }
	if(data_recv_catched)
	{
		PayLoad1_Decode((unsigned char*)dataBuff_R1, uartname);
	}
}


void PayLoad_TaskRx2(int uartname)
{
	unsigned char   Uart2Buff[BUF_MAX2];
	unsigned char   dataBuff2[BUF_MAX2];
	unsigned char   data_recv_catched = 0;
	unsigned char   plFrameHead[2] = {0xeb,0x90};        
	unsigned char   plFrameTail = 0;                
	//unsigned short  crcvalue2 = 0,checksum2 = 0;
	//int kk;
	int uart2_read_len=0;
   	 uart2_read_len= read(uartname, Uart2Buff,sizeof(Uart2Buff)); 
	QueuePushBack(&exPayLoadRXFrameBuff2,(unsigned char *)Uart2Buff,uart2_read_len);
    while(QueueLength(&exPayLoadRXFrameBuff2) >= BUF_MAX2)     
    {	
    	//printf("hello UART2    ******************\n");
		uart2_read_len=  QueueLength(&exPayLoadRXFrameBuff2); 	
       // if(FrameCompare(&exPayLoadRXFrameBuff2, plFrameHead, 2, &plFrameTail, 0, uart2_read_len))
	   if(QueueCompare(&exPayLoadRXFrameBuff2, 0, plFrameHead, 2))
	   {     
	     //	printf("hello UART2    ******************\n");
			QueuePopFront(&exPayLoadRXFrameBuff2,dataBuff2,BUF_MAX2,0); 
		    int kk;
			unsigned short  crcvalue2 = 0,checksum2 = 0;
			for(kk=0; kk<BUF_MAX2-1; kk++)
			{
				crcvalue2 += dataBuff2[kk];
			}
			crcvalue2 = crcvalue2 & 0x00ff;
			checksum2 = dataBuff2[BUF_MAX2-1];
			/*
			int i;
			printf("接收到的指令为：\n");
			for(i=0; i<64; i++)
				printf(" 0x%0x ",dataBuff2[i]);
			printf(" \n");
			*/
            if (checksum2 != crcvalue2) 
            {
            	printf("hello UART2   *********** checksum2 error******\n");
		  	    QueuePopFront(&exPayLoadRXFrameBuff2,0,2,1);
           		continue; 
            } 			
			
			QueuePopFront(&exPayLoadRXFrameBuff2,dataBuff_R2,BUF_MAX2,1);
			data_recv_catched = 1;
		}		
        else
			QueuePopFront(&exPayLoadRXFrameBuff2,0,1,1);
    }
	if(data_recv_catched)
	{
		PayLoad2_Decode((unsigned char*)dataBuff_R2, uartname);
	}
}



void PayLoad1_Decode(unsigned char *pBuff,int uartname)
{	

}

void PayLoad2_Decode(unsigned char *pBuff,int uartname)
{

}







