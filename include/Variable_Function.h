#ifndef _VARIABLE_FUNCTION_H
#define _VARIABLE_FUNCTION_H



void Send_H264_Frame(char *pH264FrameData,int nH264FrameLen,int channel);//发送h264帧数据,每次发一帧
void Send_JPG_Frame(char *pJPGHeadData,int nJPGHeadLen,char *pJPGFrameData,int nJPGFrameLen,int Frame_NUM,int channel);//发送JPG帧数据,每次发一帧

int Receive_CMD(char *pCMDData,int nCMDLen);

void Send_CMD(char *pCMDData,int nCMDLen);

int QS_IMAGE_TRACK(int Frame_NUM);

int NCC_IMAGE_TRACK(int Frame_NUM);

int Frame_Diff_Method_IMAGE_ALARM(unsigned char sensor);


int IPU_Init(void);


#endif

