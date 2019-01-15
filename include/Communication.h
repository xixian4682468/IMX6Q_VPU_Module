#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_
#include "RunLength_Algorithm.h"


//网络协议传输相关命令定义***********************************
//跟踪器命令功能参数
	//红外跟踪命令相关参数***************
	extern char  IR_TrackerStatus;//0x00——红外停止跟踪，0x01——红外准备跟踪，0x02——红外开始跟踪。
	extern char  IR_Cross;        //0x00——红外不显示十字叉，0x01——红外显示十字叉。
	extern short IR_ShiChang_x;   //红外水平视场，单位0.001度。
	extern short IR_ShiChang_y;   //红外垂直视场，单位0.001度。
	extern short IR_Center_x;     //红外跟踪中心水平坐标。
	extern short IR_Center_y;     //红外跟踪中心垂直坐标。
	extern short IR_BoMen_x0;     //红外波门中心水平坐标。
	extern short IR_BoMen_y0;     //红外波门中心垂直坐标。
	extern short IR_BoMen_W;      //红外波门宽度。
	extern short IR_BoMen_H;      //红外波门高度。
	extern short IR_MuBiao_x0;    //红外目标中心水平坐标。
	extern short IR_MuBiao_y0;    //红外目标中心垂直坐标。
	extern short IR_MuBiao_W;     //红外目标宽度。
	extern short IR_MuBiao_H;     //红外目标高度。
	//************************************
			
	//可见光跟踪命令相关参数**************
	extern char  CCD_TrackerStatus;//0x00——可见光停止跟踪，0x01——可见光准备跟踪，0x02——可见光开始跟踪。
	extern char  CCD_Cross;        //0x00——可见光不显示十字叉，0x01——可见光显示十字叉。
	extern short CCD_ShiChang_x;   //可见光水平视场，单位0.001度。
	extern short CCD_ShiChang_y;   //可见光垂直视场，单位0.001度。
	extern short CCD_Center_x;     //可见光跟踪中心水平坐标。
	extern short CCD_Center_y;     //可见光跟踪中心垂直坐标。
	extern short CCD_BoMen_x0;     //可见光波门中心水平坐标。
	extern short CCD_BoMen_y0;     //可见光波门中心垂直坐标。
	extern short CCD_BoMen_W;      //可见光波门宽度。
	extern short CCD_BoMen_H;      //可见光波门高度。
	extern short CCD_MuBiao_x0;    //可见光目标中心水平坐标。
	extern short CCD_MuBiao_y0;    //可见光目标中心垂直坐标。
	extern short CCD_MuBiao_W;     //可见光目标宽度。
	extern short CCD_MuBiao_H;     //可见光目标高度。
	//************************************

	//读红外跟踪信息**********************
	extern char  IR_mubiao_Status;     //0x00——失锁，0x01——锁定，0xFF——转台要求停止跟踪。
	extern short IR_MuBiao_x0_FeedBack;//红外目标中心水平坐标。
	extern short IR_MuBiao_y0_FeedBack;//红外目标中心垂直坐标。
	extern short IR_MuBiao_W_FeedBack; //红外目标宽度。
	extern short IR_MuBiao_H_FeedBack; //红外目标高度。
	 //************************************

	//读可见光跟踪信息********************
	extern char  CCD_mubiao_Status;     //0x00——失锁，0x01——锁定，0xFF——转台要求停止跟踪。
	extern short CCD_MuBiao_x0_FeedBack;//可见光目标中心水平坐标。
	extern short CCD_MuBiao_y0_FeedBack;//可见光目标中心垂直坐标。
	extern short CCD_MuBiao_W_FeedBack; //可见光目标宽度。
	extern short CCD_MuBiao_H_FeedBack; //可见光目标高度。
	//************************************

//告警器命令功能参数
	extern int startblock;     //起始处理块   起始处理块位置，范围0~180（默认为0）
	extern int endblock;	   //结束处理块   结束处理块位置，范围0~180（默认为180）
	extern int gray_low;	   //灰度门限   （默认为0）
	extern int mComparedot;    //告警能力    范围100~110 （默认104）
	extern int studytimes;	   //学习次数    保留
	extern int distance;       //固定目标的间距  保留
	extern int Probability;    //固定目标出现概率  保留
	extern int isstudy;        //是否学习        保留
	extern int ly;             //ly为俯仰位置，单位0.000001度。
	extern int isIROrCCD;      //1: IR 0: CCD
	extern int filter;	       //滤波器      滤波器设置：0为无滤波 1为中值滤波（默认为0）
	extern int showtypes;      //告警功能设置  0全不开，1运动目标，2静止目标， 4随机点， 5有批号的目标，6运动目标加随机目标（按照位修改）
	extern int BGBkitMethed;   //背景去除  背景去除功能（替代学习功能）0为不去除，1为低效去除，2为中效去除，3为高效去除

//压缩板FPGA命令功能参数
	unsigned char begin_frame;
	unsigned char end_frame;

//***********************************************************
#define IR_Track 0
#define IR_Alarm 1
#define CCD_Track 2
#define CCD_Alarm 3

extern unsigned char Current_Cmd_State;//当前跟踪搜索状态

#define IR_SENSOR 0
#define CCD_SENSOR 1
#define TRACK_STATE 0
#define ALARM_STATE 1

extern unsigned char SENSOR_IR_CCD ;    //红外 可见光
extern unsigned char SENSOR_STATE ;     //跟踪 搜索

//响应
//***********************************************************
int Command_Analysis(int TCP_Accept_fd, unsigned char* buffer, unsigned int len);
int Command_Analysis_SwitchVideo(int bIrOrCCD);

//int Send_Alarm_to_PC(int Frame_NUM,unsigned char m_PossTarNum,unsigned char sensor);
int Send_Alarm_to_PC(int Frame_NUM, TARFEATURE	*g_Candidate,unsigned char m_PossTarNum,unsigned char sensor);


void PayLoad_Init1(void);
void PayLoad_Init2(void);


void PayLoad_TaskRx1(int uartname);
void PayLoad_TaskRx2(int uartname);


void PayLoad1_Decode(unsigned char *pBuff,int uartname);
void PayLoad2_Decode(unsigned char *pBuff,int uartname);



#endif
