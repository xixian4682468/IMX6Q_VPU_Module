#ifndef _FILE_QUEUE_H_
#define _FILE_QUEUE_H_

#include <stdio.h>
#include <signal.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <malloc.h>
#include <asm/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>


typedef struct
{
	unsigned char* base;
	short front;	
	short rear;		
	short maxLength;
}SqQueue;

typedef struct                          
{
	unsigned char* base;
	unsigned int  front;
	unsigned int  rear;
	unsigned int  maxLength;
}EncSqQueue;

typedef struct                            //图像队列
{
	unsigned char* base;
	unsigned int  front;
	unsigned int  rear;
	unsigned int  maxLength;
}PicSqQueue;

extern char filename[256][256];
extern short len_findinfo ;


void create_file(char *filename);
int trave_dir(char* path, int depth);
int EmptyDir(char *destDir) ;
char *new_264 (int h264_jpg);

short EncQueueInit(EncSqQueue* Q,unsigned int maxLength);
unsigned int EncQueueLength(EncSqQueue *Q);
short EncQueuePushBack(EncSqQueue *Q,unsigned char *pSrc, unsigned int num);
short EncQueuePopFront(EncSqQueue *Q, unsigned char *pDst, unsigned int num, signed short bEraseFlag);

short PicQueuePushBack(PicSqQueue *Q,unsigned char *pSrc, unsigned int num);
short PicQueuePopFront(PicSqQueue *Q, unsigned char *pDst, unsigned int num,signed short bEraseFlag);
unsigned int  PicQueueLength(PicSqQueue *Q);
short PicQueueInit(PicSqQueue* Q,unsigned int maxLength);


short QueueInit(SqQueue* Q,signed short maxLength);
short QueueLength(SqQueue *Q);
short QueuePopFront(SqQueue *Q, unsigned char *pDst, signed short num,signed short bEraseFlag);
short QueuePushBack(SqQueue *Q, unsigned char *pSrc, signed short num);


short QueueCompare(SqQueue *Q, signed short pos,unsigned char *pDst, signed short num);
short FrameCompare(SqQueue *Q, unsigned char *pHeadSrc, signed short headnum,unsigned char *pTailSrc, signed short tailnum,signed short framelength);


#endif
