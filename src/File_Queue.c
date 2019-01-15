#include "File_Queue.h"


char wenjianxinname[256];
char filename[256][256];
short len_findinfo ;

void  create_file(char *filename)
{
	int fd;
	fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
	close(fd);
}  

int trave_dir(char* path, int depth)
{
    DIR *d;
    struct dirent *file;
    struct stat sb;    
    if(!(d = opendir(path)))
    {
        printf("error opendir %s!!!/n",path);
        return -1;
    }
    while((file = readdir(d)) != NULL)
    {
        if(strncmp(file->d_name, ".", 1) == 0)
            continue;
        strcpy(filename[len_findinfo++], file->d_name);
        if(stat(file->d_name, &sb) >= 0 && S_ISDIR(sb.st_mode) && depth <= 1)
        {
            trave_dir(file->d_name, depth + 1);
        }
    }
    closedir(d);
    return 0;
}


int EmptyDir(char *destDir)  
{  
    DIR *dp;  
    struct dirent *entry;  
    struct stat statbuf;  
    if ((dp = opendir(destDir)) == NULL)  
    {  
        fprintf(stderr, "cannot open directory: %s\n", destDir);  
        return -1;  
    }  
    chdir (destDir);  
    while ((entry = readdir(dp)) != NULL)  
    {  
        lstat(entry->d_name, &statbuf);  
        if(S_ISREG(statbuf.st_mode))  
        {  
            remove(entry->d_name);  
        }  
    }  
    return 0;  
} 


char *new_264 (int h264_jpg)
{
	char pinjie264[200];
	char pinjie2[200];
	int file_exist=0;
	char shanroot[200]="/camera_insmod/";
	char shanroot3[200]="/camera_insmod/";
	char shanroot1[200]="/root/";
	DIR *directory_pointer;
	struct dirent *entry;
	if((directory_pointer=opendir("/camera_insmod/"))==NULL)
	{  
		printf("Error open\n");  
	}
	else
	{  
		while((entry=readdir(directory_pointer))!=NULL)
		{  
			if(entry->d_name[0]=='.')  
				continue;  	
			int size = strlen(entry->d_name);  
			if(strcmp(( entry->d_name+ (size - 4) ) , ".txt")!= 0)
			{
				continue;
			}
			file_exist = 1;
	
			//printf("%s  size=%d\n",entry->d_name,size);  
			memset(pinjie264,0,sizeof(pinjie264)); 
			
			sprintf(pinjie264, "%d", atoi(entry->d_name));
			//printf("entry->d_name = %s \n", entry->d_name);
			if(h264_jpg==1)
			{
				strcat(pinjie264, ".264");
			}
			else if(h264_jpg==2)
			{
				strcat(pinjie264, ".jpg");           //修改前.jpg
			}
			strcat(shanroot1, pinjie264);
			strncpy(wenjianxinname,shanroot1,sizeof(shanroot1));
			
			strcat(shanroot, entry->d_name);
			remove(shanroot);
			sprintf(pinjie2, "%d", atoi(entry->d_name)+1);
			strcat(pinjie2, ".txt");
			strcat(shanroot3, pinjie2);
			create_file(shanroot3);		
			closedir(directory_pointer);        //导致打开文件太多超过系统预置的1024   ，必须时刻关闭
		}
	}
	if(file_exist==0)
	{
		create_file("0.txt");
		EmptyDir("/root/"); 
		sprintf(pinjie264, "%d", 0);
		if(h264_jpg==1)
		{
			strcat(pinjie264, ".264");
		}
		else if(h264_jpg==2)
		{
			strcat(pinjie264, ".jpg");
		}
		strcat(shanroot1, pinjie264);
		strncpy(wenjianxinname,shanroot1,sizeof(shanroot1));
	
		strcat(shanroot, "0.txt");
		remove(shanroot);
		
		sprintf(pinjie2, "%d", 1);
		strcat(pinjie2, ".txt");
		strcat(shanroot3, pinjie2);
		create_file(shanroot3);
		printf("eeeeeeee\n");
	}
	return wenjianxinname;
}

short EncQueueInit(EncSqQueue* Q,unsigned int maxLength)
{
	if (!Q->base)
	{
		return 0;
	}
	Q->front = Q->rear = 0;
	Q->maxLength = maxLength;
	return 1;
}

unsigned int  EncQueueLength(EncSqQueue *Q)
{
	if((Q->maxLength) >0)
	{
		return (Q->rear - Q->front + Q->maxLength)%(Q->maxLength);
	}
	return 1;
}

short EncQueuePushBack(EncSqQueue *Q,unsigned char *pSrc, unsigned int num)
{
	unsigned int  i;
	if((Q->maxLength) >0)
	{
		if ((Q->rear + num)%(Q->maxLength) == Q->front)
		{
			return 0;
		}
		for (i = 0; i < num; i++)
		{
			Q->base[Q->rear] = (pSrc[i] & 0x00FF);       //remove the highbyte
			Q->rear = (Q->rear + 1) %(Q->maxLength);
		}
	}
	return 1;
}

short EncQueuePopFront(EncSqQueue *Q, unsigned char *pDst, unsigned int num, signed short bEraseFlag)
{
	unsigned int i;
	if (EncQueueLength(Q) < num)
	{
		return 0;
	}
	if((Q->maxLength) >0)
	{
		if (pDst)
		{
			for (i = 0; i < num; i++)
			{
				pDst[i] = Q->base[(Q->front+i)% (Q->maxLength)];
			}	
		}
		else
		{
			 printf("dst = 0\n");
		}
		if(bEraseFlag)
		{
			Q->front = (Q->front + num) % (Q->maxLength);
		}
	}
	return 1;
}


short PicQueuePushBack(PicSqQueue *Q,unsigned char *pSrc, unsigned int num)
{
  	unsigned int  i;
  	if((Q->maxLength) >0)
  	{
  		if ((Q->rear + num)%(Q->maxLength) == Q->front)
  		{
  			return 0;
  		}
  		for (i = 0; i < num; i++)
  		{
  			Q->base[Q->rear] = (pSrc[i] & 0x00FF);//remove the highbyte
  			Q->rear = (Q->rear + 1) %(Q->maxLength);
  		}
  	}
  	return 1;
}

short PicQueuePopFront(PicSqQueue *Q, unsigned char *pDst, unsigned int num,signed short bEraseFlag)
{
	unsigned int i;
	if (PicQueueLength(Q) < num)
	{
		return 0;
	}
	if((Q->maxLength) >0)
	{
		if (pDst)
		{
			for (i = 0; i < num; i++)
			{
				pDst[i] = Q->base[(Q->front+i)% (Q->maxLength)];
			}
		}
		else
		{
			 printf("dst = 0\n");
		}
		if(bEraseFlag)
		{
			Q->front = (Q->front + num) % (Q->maxLength);
		}
	}
	return 1;
}

unsigned int  PicQueueLength(PicSqQueue *Q)
{
	if((Q->maxLength) >0)
	{
		return (Q->rear - Q->front + Q->maxLength)%(Q->maxLength);
	}
	return 1;
}

short PicQueueInit(PicSqQueue* Q,unsigned int maxLength)
{
	if (!Q->base)
	{
		return 0;
	}
	Q->front = Q->rear = 0;
	Q->maxLength = maxLength;
	return 1;
}

short QueueInit(SqQueue* Q,signed short maxLength)
{
	if (!Q->base)
	{
		return 0;
	}
	Q->front = Q->rear = 0;
	Q->maxLength = maxLength;
	return 1;
}

short QueueLength(SqQueue *Q)
{
	if((Q->maxLength) >0)
	{
		return (Q->rear - Q->front + Q->maxLength)%(Q->maxLength);
	}
	return 1;
}


short QueueCompare(SqQueue *Q, signed short pos,unsigned char *pDst, signed short num)
{
	short i;
	if (QueueLength(Q) < (pos + num))
	{
		return 0;
	}
	if (!pDst)
	{
		return 0;
	}
	if((Q->maxLength) >0)
	{
		for (i = 0; i < num; i++)
		{
			if (Q->base[(Q->front + pos + i) % (Q->maxLength)] != pDst[i])
			{
				return 0;
			}
		}
	}
	return 1;
}

short QueuePushBack(SqQueue *Q, unsigned char *pSrc, signed short num)
{
	short i;
	if((Q->maxLength) >0)
	{
		if ((Q->rear + num)%(Q->maxLength) == Q->front)
		{
			return 0;
		}
		for (i = 0; i < num; i++)
		{
			Q->base[Q->rear] = (pSrc[i] & 0x00FF);     //remove the highbyte
			Q->rear = (Q->rear + 1) %(Q->maxLength);
		}
	}
	return 1;
}

short QueuePopFront(SqQueue *Q, unsigned char *pDst, signed short num,signed short bEraseFlag)
{
	short i;
	if (QueueLength(Q) < num)
	{
		return 0;
	}
	if((Q->maxLength) >0)
	{
		if (pDst)
		{
			for (i = 0; i < num; i++)
			{
				pDst[i] = Q->base[(Q->front+i)% (Q->maxLength)];
			}	
		}
		if(bEraseFlag)
		{
			Q->front = (Q->front + num) % (Q->maxLength);
		}
	}
	return 1;
}


short FrameCompare(SqQueue *Q, unsigned char *pHeadSrc, signed short headnum,unsigned char *pTailSrc, signed short tailnum,signed short framelength)
{
	if (QueueLength(Q) < framelength)
	{
		return 0;
	}
	if ((QueueCompare(Q, 0, pHeadSrc, headnum)) && (QueueCompare(Q, (framelength - tailnum), pTailSrc, tailnum)))
	{
		return 1;
	}
	return 0;
}

