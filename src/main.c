#define __USE_GNU                 //启用CPU_ZERO等相关的宏
#define _GNU_SOURCE

#include  <pthread.h>
#include  <sched.h>
#include  <signal.h>
#include  <getopt.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <math.h>

#include <malloc.h>
#include <mxcfb.h>
#include <mxc_v4l2.h>
#include <ipu.h>

#include <linux/types.h>
#include <assert.h>
#include <dirent.h>

#include <sys/msg.h>  
#include <errno.h>  

#include "File_Queue.h"
#include "Init_dev.h"
#include "NCC_Track.h"
#include "qsImgLib.h"
#include "libavformat/avformat.h"

#include "Communication.h"
#include "Variable_Function.h"
#include "VPU_Camera.h"
#include "KCF_Track.h"


#define MAX_TEXT 512  
struct msg_st  
{  
    long int msg_type;  
    char text[MAX_TEXT];  
};

struct msg_st data;

int msgid = -1;
long int msgtype = 0; //注意1


//****************************************************
//创建线程
//****************************************************
pthread_t camera_tid;
pthread_t UDP_tid;  
pthread_t TCP_SERVER_CMD_tid;  
pthread_t TCP_SERVER_VIDEO_tid;
pthread_t RTMP_tid;                                  //上传压缩视频
pthread_t Track_tid;                                 //压缩图像
pthread_t UART2_tid;
pthread_t UART1_tid;

pthread_t msg_tid;



//********************标志位&变量**************************
//原本默认为手动状态 ：0
unsigned char Track_auto_statue = 0; //跟踪手动状态
unsigned char Track_auto_flag = 1;   //跟踪手动标志位

unsigned char PAL_SDI_Track = 1;     //PAL&SDI 跟踪标志位,     高清SDI =0，标清PAL =1
unsigned char SDI_BeginTrack_flag = 0;
unsigned char PAL_BeginTrack_flag = 0;

unsigned char IR_Track_Begin_Flag = 0;
unsigned char CCD_Track_Begin_Flag = 0;

int Frame_Num = 0;
volatile unsigned char Frame_Capture = 1;
volatile unsigned char Frame_Capture_OK = 0;

extern QSRECT rcInit;

//接收到的鼠标选中框***************************************
unsigned short tar_x1,tar_y1,tar_x2,tar_y2;
unsigned short PAL_Target_x = 0, PAL_Target_y = 0;

unsigned short VPU_Resolution = 576;    //VPU设置图像分辨率
unsigned char VPU_Format = 2;           //VPU压缩格式 H264	

extern unsigned char Frame_NUM_FPGA;
extern unsigned char Alarm_Begin_Flag;
extern unsigned char Current_Cmd_State;

extern int New_JPG_filename ;
extern int New_JPG ;

//管道******************************************************
int pipe_fdname;

//TCP协议***************************************************
int socketCon;
int TCP_Accept_CMD_fd = -1;
int TCP_Accept_VIDEO_fd = -1;
int TCP_Server_CMD_sockfd = -1;
int TCP_Server_VIDEO_sockfd = -1;


#define PORT_VIDEO 9898 //视频服务端口
#define PORT_CMD   9899 //命令服务端口

//串口设备文件描述符****************************************
//extern int UART1_fd,UART2_fd;
extern int quitflag;

//*********************IPU 参数*****************************
extern struct ipu_task task;
extern int fd_ipu ;   // IPU file descriptor
extern int isize ;	  // input size
extern int osize ;	  // output size

extern unsigned char *inbuf ;
extern void *outbuf ;
//*********************************************************

//*********************KCF Track****************************
extern unsigned char gSubImageData_KCF[720*576];
unsigned char KCF_BeginTrack_flag = 0;
unsigned int KCF_Target_x=360, KCF_Target_y=288;

//处理函数
//*********************************************************

void tracking_process(void)
{	
	int x, y;
	cpu_set_t cpuset;
	pthread_t thread = pthread_self();
	int s,core_no = 1;
	CPU_ZERO(&cpuset);
	CPU_SET(core_no,&cpuset);
	s = pthread_setaffinity_np(thread,sizeof(cpuset),&cpuset);

	if(s != 0)
		printf("Set pthread_setaffinity_np failed\n"); 
	
	
	while(1)
	{	
	
	}
}

void camera_process(void)
{
	cpu_set_t cpuset;
	pthread_t thread = pthread_self();
	int s,core_no = 3;
	CPU_ZERO(&cpuset);
	CPU_SET(core_no,&cpuset);
	s = pthread_setaffinity_np(thread,sizeof(cpuset),&cpuset);
	if(s != 0)
		printf("Set pthread_setaffinity_npfailed\n");
	
//**************************************************************************

	pipe_fdname = open("pipe_filename", O_WRONLY);
	//pipe_fdname = open("pipe_filename", O_WRONLY | O_NONBLOCK);
	
	while(1)
	{ 
		usleep(1000);
		// -f <format> 0 - MPEG4, 1 - H.263, 2 - H.264, 7 - MJPG 
		//VPU_Format = 7;
		quitflag = 0;
		New_JPG_filename = 0;
		New_JPG = 0;
		VPU_camera(VPU_Resolution, VPU_Format);
	}

}

void UART2_process(void)
{
	cpu_set_t cpuset;
	pthread_t thread = pthread_self();
	int s,core_no = 0;
	CPU_ZERO(&cpuset);
	CPU_SET(core_no,&cpuset);

	s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
   	if(s != 0)
    	printf("Set pthread_setaffinity_np failed\n");

	PayLoad_Init2();
	for(;;)
	{
		usleep(15000);
		PayLoad_TaskRx2(UART2_fd);
	}
}

void UART1_process(void)
{
	cpu_set_t cpuset;
	pthread_t thread = pthread_self();
	int s,core_no = 0;	
	CPU_ZERO(&cpuset);
	CPU_SET(core_no,&cpuset);

	s = pthread_setaffinity_np(thread,sizeof(cpu_set_t),&cpuset);
   	if(s != 0)
    	printf("Set pthread_setaffinity_np failed\n");
	
	while(1)
	{
		sleep(100);
	}
}

int rtmpStart()
{
    printf("rtmp process Start......\n");
    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;

    //
    AVPacket pkt;
    //Êä³öÂ·Ÿ¶
    const char *out_filename;
    const char *in_filename = NULL;

    uint8_t *buffer = NULL;
    size_t buffer_size;

    int ret, i;
    int videoindex = -1;
    int frame_index = 0;
    int64_t start_time = 0;

    //×¢²á
    av_register_all();
    //Network
    avformat_network_init();

    out_filename = "udp://192.168.1.1:6666";
    printf("addr:%s\n", out_filename);

    //ÊäÈë£šInput£©
    char buf[] = "";
    if ((ret = avformat_open_input(&ifmt_ctx, "pipe_filename"/*in_filename*/, NULL, NULL)) < 0)
    {
        printf( "Could not open input file.\n");
        av_strerror(ret, buf, 1024);
        printf("Couldn't open file: %d(%s)\n", ret, buf);
        goto end;
    }

    //Á÷ÐÅÏ¢
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0)
    {
        printf( "Failed to retrieve input stream information\n");
        goto end;
    }

    for(i=0; i<ifmt_ctx->nb_streams; i++)
    {
        if(ifmt_ctx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            videoindex=i;
            break;
        }
    }
    av_dump_format(ifmt_ctx, 0, in_filename, 0);
    avformat_alloc_output_context2(&ofmt_ctx, NULL, "mpegts", out_filename);//UDP

    if (!ofmt_ctx)
    {
        printf( "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    ofmt = ofmt_ctx->oformat;
    for (i = 0; i < ifmt_ctx->nb_streams; i++)
    {
        //žùŸÝÊäÈëÁ÷ŽŽœšÊä³öÁ÷£šCreate output AVStream according to input AVStream£©
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
        if (!out_stream)
        {
            printf( "Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
                goto end;
        }
        //žŽÖÆAVCodecContextµÄÉèÖÃ£šCopy the settings of AVCodecContext£©
        ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
        if (ret < 0)
        {
            printf( "Failed to copy context from input to output stream codec context\n");
                goto end;
        }
        out_stream->codec->codec_tag = 0;
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    //Dump Format------------------
    av_dump_format(ofmt_ctx, 0, out_filename, 1);
    //Žò¿ªÊä³öURL£šOpen output URL£©
    if (!(ofmt->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            printf( "Could not open output URL '%s'\n", out_filename);
                goto end;
        }
    }
    ofmt_ctx->streams[0]->codec->width = 1;
    ofmt_ctx->streams[0]->codec->height = 1;

    //ÐŽÎÄŒþÍ·£šWrite file header£©
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0)
    {
        printf( "Error occurred when opening output URL\n");
            goto end;
    }

    start_time=av_gettime();

    while (1)
    {
        AVStream *in_stream, *out_stream;
        //»ñÈ¡Ò»žöAVPacket£šGet an AVPacket£©
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
            break;
        //FIX£ºNo PTS (Example: Raw H.264)
        //Simple Write PTS
        if(pkt.pts==AV_NOPTS_VALUE)
        {
            //Write PTS
            AVRational time_base1=ifmt_ctx->streams[videoindex]->time_base;
            //Duration between 2 frames (us)
            int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(ifmt_ctx->streams[videoindex]->r_frame_rate);
            //Parameters
            pkt.pts=(double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
            pkt.dts=pkt.pts;
            pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
        }
        //Important:Delay
        if(pkt.stream_index == videoindex)
        {
            AVRational time_base=ifmt_ctx->streams[videoindex]->time_base;
            AVRational time_base_q={1,AV_TIME_BASE};
            int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
            int64_t now_time = av_gettime() - start_time;
            if (pts_time > now_time)
            {
               // printf("delay:%d\n", pts_time - now_time);
               // av_usleep(pts_time - now_time);           //关键！！！注意
            } 
        }

        in_stream  = ifmt_ctx->streams[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];
        /* copy packet */
        //×ª»»PTS/DTS£šConvert PTS/DTS£©
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, /*(AVRounding)*/(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, /*(AVRounding)*/(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        //Print to Screen
        if(pkt.stream_index == videoindex)
        {
          //  printf("Send %8d video frames to output URL\n",frame_index);
            frame_index++;
        }

		//printf("%d kbs\n", pkt.size / 1024);//发送压缩图像大小
        //ret = av_write_frame(ofmt_ctx, &pkt);
        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);

        if (ret < 0)
        {
            printf( "Error muxing packet\n");
            break;
        }

        av_free_packet(&pkt);

    }
    //ÐŽÎÄŒþÎ²£šWrite file trailer£©
    av_write_trailer(ofmt_ctx);

end:
    avformat_close_input(&ifmt_ctx);
    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
    {
        avio_close(ofmt_ctx->pb);
    }
    avformat_free_context(ofmt_ctx);
    if (ret < 0 && ret != AVERROR_EOF)
    {
        printf( "Error occurred.\n");
        return -1;
    }
}


void socket_process(void)
{  
    int isconnect;

    fd_set readfds,writefds;
	char buffer[MAX_TEXT];
	time_t timet;
	struct timeval tv;

	int msgid = -1;  
	int msgid2 = -1;//*((int *)msg_fd);

	while(1)
	{		
		printf("开始socket\n");  
		/* 创建TCP连接的Socket套接字 */  
		socketCon = socket(AF_INET, SOCK_STREAM, 0);  
		if(socketCon < 0)
		{  
			printf("创建TCP连接套接字失败\n");  
			exit(-1);  
		}  
		/* 填充客户端端口地址信息，以便下面使用此地址和端口监听 */  
		struct sockaddr_in server_addr;  
		bzero(&server_addr,sizeof(struct sockaddr_in));  
		server_addr.sin_family = AF_INET;  
		server_addr.sin_addr.s_addr = inet_addr("192.168.1.1");  
		server_addr.sin_port = htons(6767);  
		printf("连接之前的socketCon:%d",socketCon);  
	
		/* 连接服务器 */  
		int res_con = connect(socketCon,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr));  
		if(res_con != 0)
		{  
			printf("连接失败,等待3秒后继续连接......\n");
			// 关闭套接字	
			close(socketCon); 
			sleep(3);			
			continue;  
		}  
		printf("连接成功\n");  
		//开启新的实时接受数据线程			
		isconnect = 1;
		
		/* 实时发送数据 */  
		while(1)
		{  
			if(isconnect == 0)
				break;
	
			FD_ZERO(&readfds);
			FD_ZERO(&writefds);
			FD_SET(socketCon, &readfds);
			FD_SET(socketCon, &writefds);
			tv.tv_usec = 0;
			tv.tv_sec = 60;
			int ret;
			ret = select(socketCon+1, &readfds, NULL, NULL, &tv);
			switch(ret)
			{
				case 0:
					printf("select timeout!\n");
					break;
				case -1:
					perror("select return failed!\n");
					isconnect = 0;
				default:
					if(FD_ISSET(socketCon, &readfds) > 0)
					{
						memset(buffer, 0, MAX_TEXT);
						time(&timet);
						int readlen;
						readlen = read(socketCon, buffer, MAX_TEXT);
						printf("readlen:%d\n", readlen);
						if(readlen <= 0)
						{
							perror("read data failed!\n");
							isconnect = 0;
							break;
						}
						else
						{	
						//***********************************接收鼠标选中目标***********************************
							printf("%d-->:\n", readlen);
							if(buffer[0] == 0x68)
							{
								printf("buffer[0]:%x\n", buffer[0]);
							}
							unsigned char i;
							for(i=0; i<25; i++)
								printf("0x=%x ", buffer[i]);
							printf("\n");
							if((buffer[0] == 0x68) && (buffer[2] == 0x68) && (buffer[13] == 0x16))
							{
								int x ;

								tar_x1  = (buffer[5]<<8) + buffer[4];
								tar_y1  = (buffer[7]<<8) + buffer[6];
							    tar_x2	= (buffer[9]<<8) + buffer[8];
							    tar_y2	= (buffer[11]<<8) + buffer[10];
								
								printf("tar_x1 = 0x%04x	  ",tar_x1);
								printf("tar_y1 = 0x%04x	  ",tar_y1);
								printf("tar_x2 = 0x%04x   ",tar_x2);
								printf("tar_y2 = 0x%04x   \n",tar_y2);
								if(Track_auto_flag == 1)
								{
									if((tar_x1!=rcInit.left)||(tar_y1!=rcInit.top)||(tar_x2!=rcInit.right)||(tar_y2!=rcInit.bottom))
									{
										if((tar_x1+5<tar_x2)&&(tar_y1+5<tar_y2))
										{
											Track_auto_flag = 0;         //切换成手动模式
											usleep(50000);               //等待跟踪算法结束，保持在手动状态
											
											//PAL************************************************
											if(PAL_SDI_Track == 1)  
											{
												if(PAL_BeginTrack_flag == 0)  //第一次获得跟踪对象
													PAL_BeginTrack_flag = 1;             
												//PAL_Target_x = (tar_x1+tar_x2)/2-600;
												//PAL_Target_y = (tar_y1+tar_y2)/2-252;
												PAL_Target_x = (tar_x1+tar_x2)/2;
												PAL_Target_y = (tar_y1+tar_y2)/2;
											}
											
											//SDI高清********************************************
											if(PAL_SDI_Track == 0) 
											{
												if(SDI_BeginTrack_flag == 0)  //第一次获得跟踪对象		
													SDI_BeginTrack_flag = 1;
												else	
													qsObjectTrackStop();	 //停止跟踪器
												//原始为1920*1080图像目标减半给960*540图像做跟踪
												//rcInit.left = tar_x1/2;	     //选中区域
												//rcInit.right = tar_x2/2;
												//rcInit.top = tar_y1/2;
												//rcInit.bottom = tar_y2/2;
												
												rcInit.left = tar_x1;	     //选中区域
												rcInit.right = tar_x2;
												rcInit.top = tar_y1;
												rcInit.bottom = tar_y2;
											}
											
											Frame_Num = 0;	             //中断跟踪
											printf("*************接收到新的跟踪区域*************\n");
											Track_auto_flag = 1;         //切换成跟踪模式
										}
									}
								}
							}
							
							data.msg_type = 1;
							memcpy(data.text, buffer, readlen);
							msgsnd(msgid, (void*)&data, MAX_TEXT, 0);
						//***********************************接收鼠标选中目标***********************************
						}
					}	
			}
		}
		close(socketCon);  
	}
	//return 0;  
}


void TCP_SERVER_CMD_process(void)
{  

	cpu_set_t cpuset;
	pthread_t thread = pthread_self();
	int s,core_no = 0;	
	CPU_ZERO(&cpuset);
	CPU_SET(core_no,&cpuset);

	s = pthread_setaffinity_np(thread,sizeof(cpu_set_t),&cpuset);
	if(s != 0)
		printf("Set pthread_setaffinity_np failed\n");

    int isconnect;
	
    fd_set readfds,writefds;
	char buffer[MAX_TEXT];
	time_t timet;
	struct timeval tv;

	int msgid = -1;  
	int msgid2 = -1;//*((int *)msg_fd);

	while(1)
	{	
		int ret;
		ret = TCP_SERVER_Init(TCP_Server_CMD_sockfd,&TCP_Accept_CMD_fd, PORT_CMD);
		if(ret < 0)
		{	
			printf("连接失败,等待3秒后继续连接......\n");
			// 关闭套接字	
			//close(TCP_Accept_CMD_fd); 
			//sleep(3);			
			continue;
		}
		
		printf("连接 TCP_Accept_CMD_fd成功......\n");  
		//开启新的实时接受数据线程			
		isconnect = 1;
		
		/* 实时发送数据 */  
		while(1)
		{  
			if(isconnect == 0)
				break;
	
			FD_ZERO(&readfds);
			FD_ZERO(&writefds);
			FD_SET(TCP_Accept_CMD_fd, &readfds);
			FD_SET(TCP_Accept_CMD_fd, &writefds);
			tv.tv_usec = 0;
			tv.tv_sec = 3;
			int ret;
			ret = select(TCP_Accept_CMD_fd+1, &readfds, NULL, NULL, &tv);
			
			switch(ret)
			{
				case 0:
					printf("select timeout!\n");
					break;
				case -1:
					perror("select return failed!\n");
					isconnect = 0;
				default:
					if(FD_ISSET(TCP_Accept_CMD_fd, &readfds) > 0)
					{
						memset(buffer, 0, MAX_TEXT);
						time(&timet);
						int readlen;
						readlen = read(TCP_Accept_CMD_fd, buffer, MAX_TEXT);
						printf("readlen:%d\n", readlen);
						if(readlen <= 0)
						{
							perror("read data failed!\n");
							isconnect = 0;
							break;
						}
						else
						{	
							#if 0
								int ret;
								int i;
								for(i=0; i<readlen; i++)
								{
									printf(" 0x%0x ",buffer[i]);
								}
								
								printf("\n");
								int length  = 0;
								length = buffer[6]+buffer[7]<<8+buffer[8]<<16+buffer[9]<<24;
								//ret = Command_Analysis(TCP_Accept_CMD_fd, buffer+10, length);	
							#endif

							Receive_CMD(buffer, readlen);

							
							data.msg_type = 1;
							memcpy(data.text, buffer, readlen);
							msgsnd(msgid, (void*)&data, MAX_TEXT, 0);
						}
					}	
			}
		}
		close(TCP_Accept_CMD_fd);  
		close(TCP_Server_CMD_sockfd);
	}
	//return 0;  
}

void TCP_SERVER_VIDEO_process(void)
{  
    int isconnect;
	
    fd_set readfds,writefds;
	char buffer[MAX_TEXT];
	time_t timet;
	struct timeval tv;

	int msgid = -1;  
	int msgid2 = -1;//*((int *)msg_fd);

	while(1)
	{	
		int ret;
		ret = TCP_SERVER_Init(TCP_Server_VIDEO_sockfd,&TCP_Accept_VIDEO_fd, PORT_VIDEO);
		if(ret < 0)
		{	
			printf("连接失败,等待3秒后继续连接......\n");
			// 关闭套接字	
			//close(TCP_Accept_VIDEO_fd); 
			//sleep(3);			
			continue;
		}
		
		printf("连接 TCP_Accept_VIDEO_fd......\n");  
		//开启新的实时接受数据线程			
		isconnect = 1;
		
		/* 实时发送数据 */  
		while(1)
		{  
			if(isconnect == 0)
				break;
	
			FD_ZERO(&readfds);
			FD_ZERO(&writefds);
			FD_SET(TCP_Accept_VIDEO_fd, &readfds);
			FD_SET(TCP_Accept_VIDEO_fd, &writefds);
			tv.tv_usec = 0;
			tv.tv_sec = 60;
			int ret;
			ret = select(TCP_Accept_VIDEO_fd+1, &readfds, NULL, NULL, &tv);
			
			switch(ret)
			{
				case 0:
					printf("select timeout!\n");
					break;
				case -1:
					perror("select return failed!\n");
					isconnect = 0;
				default:
					if(FD_ISSET(TCP_Accept_VIDEO_fd, &readfds) > 0)
					{
						memset(buffer, 0, MAX_TEXT);
						time(&timet);
						int readlen;
						readlen = read(TCP_Accept_VIDEO_fd, buffer, MAX_TEXT);
						printf("readlen:%d\n", readlen);
						if(readlen <= 0)
						{
							perror("read data failed!\n");
							isconnect = 0;
							break;
						}
						else
						{	
							int ret;
							ret = Command_Analysis(TCP_Accept_VIDEO_fd, buffer, readlen);
						
							data.msg_type = 1;
							memcpy(data.text, buffer, readlen); 
							msgsnd(msgid, (void*)&data, MAX_TEXT, 0);
						}
					}	
			}
		}
		close(TCP_Accept_VIDEO_fd); 
		close(TCP_Server_VIDEO_sockfd);
	}
	//return 0;  
}

#define MAX_CLIENT_NUM 30
#define MAX_RECV_LEN 1024 
#define SERVER_IP "192.168.1.60" 
#define TCP_VIDEO_SERVER_PORT 9898
#define TCP_CMD_SERVER_PORT 9899
#define BACK_LOG 20 



int TCP_CMD_Process(void)
{
	int sock_fd = -1;
	int ret = -1;
	
	int client_fd[MAX_CLIENT_NUM];

	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;

	socklen_t serv_addr_len = 0;
	socklen_t cli_addr_len = 0; 

	
	char recv_buf[MAX_RECV_LEN]; 
	int new_conn_fd = -1; 
	int i = 0; 
	int max_fd = -1; 
	int num = -1; 

	static int running = 1; 

	while(1)
	{
		int count = 0;
		struct timeval timeout;
		fd_set read_set;
		fd_set write_set; 
		fd_set select_read_set;
		FD_ZERO(&read_set);
		FD_ZERO(&write_set); 
		FD_ZERO(&select_read_set);

		for (i = 0; i < MAX_CLIENT_NUM; i++) 
		{ 
			client_fd[i] = -1; 
		}
		memset(&serv_addr, 0, sizeof(serv_addr));
		memset(&cli_addr, 0, sizeof(cli_addr));
		sock_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (sock_fd < 0)
		{ 
			perror("Fail to socket"); 
			return -1; 
		} 

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(TCP_CMD_SERVER_PORT);
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

		unsigned int value = 1; 

		if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&value, sizeof(value)) < 0) 
		{ 
			perror("Fail to setsockopt"); 
			return -1; 
		}
		serv_addr_len = sizeof(serv_addr); 
		if (bind(sock_fd, (struct sockaddr*)&serv_addr, serv_addr_len) < 0) 
		{ 
			perror("Fail to bind"); 
			return -1;  
		} 
		if (listen(sock_fd, BACK_LOG) < 0) 
		{ 
			perror("Fail to listen"); 
			return -1; 
		} 
		char buf[1024]; 
		max_fd = sock_fd; 
		int len; 
		FD_SET(sock_fd, &read_set); 
	
		printf("start server...\n");

		while (running) 
		{ 

			//if(count >= 3)
				//break;

			timeout.tv_sec = 5; 
			timeout.tv_usec = 0; 
			max_fd = sock_fd; 

			for (i = 0; i < MAX_CLIENT_NUM; i++) 
			{ 
				if (max_fd < client_fd[i]) 
				{ 
					max_fd = client_fd[i]; 
				}
			}

			select_read_set = read_set; 
			ret = select(max_fd + 1, &select_read_set, NULL, NULL, &timeout); 


			if (ret == 0)
			{ 
				//printf("timeout\n");
				count++;
				//printf("count:%d\n", count);
			}
			else if (ret < 0) 
			{ 
				printf("error occur\n"); 
				count = 0;
			} 
			else 
			{ 
				count = 0;
				if (FD_ISSET(sock_fd, &select_read_set)) 
				{ 
					printf("new client comes\n");
					len = sizeof(cli_addr); 
					new_conn_fd = accept(sock_fd, (struct sockaddr*)&cli_addr, &len);
					if (new_conn_fd < 0) 
					{ 
						perror("Fail to accept"); 
						exit(1);
					} 
					else 
					{ 
						for (i = 0; i < MAX_CLIENT_NUM; i++)
						{
							if (client_fd[i] == -1)
							{ 
								client_fd[i] = new_conn_fd; 
								TCP_Accept_CMD_fd = new_conn_fd;//***************
								FD_SET(new_conn_fd, &read_set);
								break; 
							} 
							if (max_fd < new_conn_fd)
							{
								max_fd = new_conn_fd; 
							} 
						}
		 			} 
				} 
				else 
				{ 
					for (i = 0; i < MAX_CLIENT_NUM; i++) 
					{ 
						if (-1 == client_fd[i]) 
						{
		 					continue; 
						} 
						memset(recv_buf, 0, MAX_RECV_LEN); 
						if (FD_ISSET(client_fd[i], &select_read_set)) 
						{ 
							unsigned int Sizeof_MSG = 0;
							num = read(client_fd[i], recv_buf, 4);
							if(num == 4)
							{
								Sizeof_MSG = recv_buf[0] + recv_buf[1]*256 + recv_buf[2]*256*256 + recv_buf[3]*256*256*256;
								//printf("recv_buf[0] = %d recv_buf[1]=%d recv_buf[2]=%d  recv_buf[3]=%d \n",recv_buf[0],recv_buf[1],recv_buf[2],recv_buf[3]);
								//printf("Sizeof_MSG = %d \n",Sizeof_MSG);
								if((Sizeof_MSG <= 1020) && (Sizeof_MSG > 0))
								{
									num = read(client_fd[i], recv_buf+4, Sizeof_MSG-4);
									//printf("num = %d *******\n",num);
									if(num == (Sizeof_MSG-4))
									{
										Receive_CMD(recv_buf,num+4);//********************
										Analysis_Msg();
									}
								}
							}
							if (num < 0)
							{
								printf("Client(%d) left\n", client_fd[i]); 
								FD_CLR(client_fd[i], &read_set);
								close(client_fd[i]);
								client_fd[i] = -1; 
								TCP_Accept_CMD_fd = -1;
							} 
							else if (num > 0) 
							{ 
								//printf("Recieve client(%d) data\n", client_fd[i]); 
								#if 0
								int i;				
								printf("numLen : %d \n date:",num);
								for(i=0; i<num+4; i++)
									printf("%02x", recv_buf[i]);
								printf("\n");
								//Receive_CMD(recv_buf,num);//********************
								//Analysis_Msg();
								#endif
							} 
							if (num == 0)
							{ 
								printf("Client(%d) exit\n", client_fd[i]);
								FD_CLR(client_fd[i], &read_set);
								close(client_fd[i]);
								client_fd[i] = -1; 
								TCP_Accept_CMD_fd = -1;
							} 
						} 
					} 
				} 
			} 
		} 
		close(sock_fd);
	}
	return 0; 
}



int TCP_VIDEO_Process(void)
{
	int sock_fd = -1;
	int ret = -1;
	int client_fd[MAX_CLIENT_NUM];

	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;

	socklen_t serv_addr_len = 0;
	socklen_t cli_addr_len = 0; 

	
	char recv_buf[MAX_RECV_LEN]; 
	int new_conn_fd = -1; 
	int i = 0; 
	int max_fd = -1; 
	int num = -1; 

	static int running = 1; 

	while(1)
	{
		int count = 0;
		struct timeval timeout;
		fd_set read_set;
		fd_set write_set; 
		fd_set select_read_set;
		FD_ZERO(&read_set);
		FD_ZERO(&write_set); 
		FD_ZERO(&select_read_set);

		for (i = 0; i < MAX_CLIENT_NUM; i++) 
		{ 
			client_fd[i] = -1; 
		}
		memset(&serv_addr, 0, sizeof(serv_addr));
		memset(&cli_addr, 0, sizeof(cli_addr));
		sock_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (sock_fd < 0)
		{ 
			perror("Fail to socket"); 
			return -1; 
		} 

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(TCP_VIDEO_SERVER_PORT);
		serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
		unsigned int value = 1; 

		if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&value, sizeof(value)) < 0) 
		{ 
			perror("Fail to setsockopt"); 
			return -1; 
		}
		serv_addr_len = sizeof(serv_addr); 
		if (bind(sock_fd, (struct sockaddr*)&serv_addr, serv_addr_len) < 0) 
		{ 
			perror("Fail to bind"); 
			return -1;  
		} 
		if (listen(sock_fd, BACK_LOG) < 0) 
		{ 
			perror("Fail to listen"); 
			return -1; 
		} 
		char buf[1024]; 
		max_fd = sock_fd; 
		int len; 
		FD_SET(sock_fd, &read_set); 
	
		printf("start server...\n");

		while (running) 
		{ 

			//if(count >= 3)
				//break;

			timeout.tv_sec = 5; 
			timeout.tv_usec = 0; 
			max_fd = sock_fd; 

			for (i = 0; i < MAX_CLIENT_NUM; i++) 
			{ 
				if (max_fd < client_fd[i]) 
				{ 
					max_fd = client_fd[i]; 
				}
			}

			select_read_set = read_set; 
			ret = select(max_fd + 1, &select_read_set, NULL, NULL, &timeout); 


			if (ret == 0)
			{ 
				//printf("timeout\n");
				count++;
				//printf("count:%d\n", count);
			}
			else if (ret < 0) 
			{ 
				printf("error occur\n"); 
				count = 0;
			} 
			else 
			{ 
				count = 0;
				if (FD_ISSET(sock_fd, &select_read_set)) 
				{ 
					//printf("new client comes\n");
					len = sizeof(cli_addr); 
					new_conn_fd = accept(sock_fd, (struct sockaddr*)&cli_addr, &len);
					if (new_conn_fd < 0) 
					{ 
						perror("Fail to accept"); 
						exit(1);
					} 
					else 
					{ 
						for (i = 0; i < MAX_CLIENT_NUM; i++)
						{
							if (client_fd[i] == -1)
							{ 
								client_fd[i] = new_conn_fd; 
								TCP_Accept_VIDEO_fd = new_conn_fd;//***************
								FD_SET(new_conn_fd, &read_set);
								break; 
							} 
							if (max_fd < new_conn_fd)
							{
								max_fd = new_conn_fd; 
							} 
						}
		 			} 
				} 
				else 
				{ 
					for (i = 0; i < MAX_CLIENT_NUM; i++) 
					{ 
						if (-1 == client_fd[i]) 
						{
		 					continue; 
						} 
						memset(recv_buf, 0, MAX_RECV_LEN); 
						if (FD_ISSET(client_fd[i], &select_read_set)) 
						{ 
							num = read(client_fd[i], recv_buf, MAX_RECV_LEN);
							if (num < 0)
							{
								printf("Client(%d) left\n", client_fd[i]); 
								FD_CLR(client_fd[i], &read_set);
								close(client_fd[i]);
								client_fd[i] = -1; 
								TCP_Accept_VIDEO_fd = -1;
							} 
							else if (num > 0) 
							{ 
								recv_buf[num] = '\0'; 
								printf("Recieve client(%d) data\n", client_fd[i]); 
								printf("Data: %s\n\n", recv_buf);
								//Receive_CMD(recv_buf,num);//********************
							} 
							if (num == 0)
							{ 
								printf("Client(%d) exit\n", client_fd[i]);
								FD_CLR(client_fd[i], &read_set);
								close(client_fd[i]);
								client_fd[i] = -1; 
								TCP_Accept_VIDEO_fd = -1;
							} 
						} 
					} 
				} 
			} 
		} 
		close(sock_fd);
	}
	return 0; 
}

int main(int argc, char **argv)
{
	int ret;
	unlink("pipe_filename");
	mkfifo("pipe_filename", 0777);
	
//初始化各个模块设备************************************************		
	if(0)
	if(UDP_Init() < 0)		       //UDP协议初始化
	{
		printf("UDP_Init not success!!!!\n");
		exit(1);
	}
	
	if(UART1_Init() < 0)           //串口1初始化	
	{
		printf("UART1_Init not success!!!!\n");
		exit(1);
	}

	if(UART2_Init() < 0)           //串口2初始化
	{
		printf("UART2_Init not success!!!!\n");
		exit(1);
	}
	
	if(UART3_Init() < 0)           //串口3初始化
	{
		printf("UART3_Init not success!!!!\n");
		exit(1);
	}
	
//*************************************************切换视频*********
#if 1
	int i;
	unsigned char ACK[11] = {0};
	for(i=0; i<11; i++)
	{
		ACK[i] = 2;
	}
	write(UART2_fd, ACK, 11);
#endif
//******************************************************************	

//创建线程**********************************************************
	ret = pthread_create(&UART2_tid, NULL, (void *)UART2_process,NULL); 
	if(ret != 0)
		printf ("Create UART2_tid pthread error!\n");
		
	ret = pthread_create(&UART1_tid, NULL, (void *)UART1_process,NULL); 
	if(ret != 0)
		printf ("Create UART1_tid pthread error!\n");
	
	ret=pthread_create(&camera_tid,NULL,(void *)camera_process,NULL);
	if(ret!=0)
		printf ("Create camera pthread error!\n");
	sleep(2);
	//ret = pthread_create(&TCP_SERVER_CMD_tid, NULL, (void *)TCP_SERVER_CMD_process,NULL); 
	ret = pthread_create(&TCP_SERVER_CMD_tid, NULL, (void *)TCP_CMD_Process,NULL); 
	if(ret != 0)
		printf ("Create TCP_SERVER_CMD_tid pthread error!\n");
		
	//ret = pthread_create(&TCP_SERVER_VIDEO_tid, NULL, (void *)TCP_SERVER_VIDEO_process,NULL); //TCP_SERVER_VIDEO_process   tcp_pro
	ret = pthread_create(&TCP_SERVER_VIDEO_tid, NULL, (void *)TCP_VIDEO_Process,NULL);
	if(ret != 0)
		printf ("Create TCP_SERVER_VIDEO_tid pthread error!\n");

	ret=pthread_create(&Track_tid,NULL,(void *)tracking_process,NULL);
	if(ret!=0)
		printf ("Create tracking_process pthread error!\n");

	ret=pthread_create(&RTMP_tid,NULL,(void *)rtmpStart,NULL);
	if(ret!=0)
		printf ("Create rtmpStart pthread error!\n");
	
	//rtmpStart();
	socket_process();
	
	//for(;;)
	//{
	//	sleep(2);
		//msg_process();	 
	//}

	pthread_join(camera_tid,NULL);
	pthread_join(RTMP_tid,NULL);
	pthread_join(Track_tid,NULL);
	pthread_join(UART2_tid,NULL);
	pthread_join(UART1_tid,NULL);
	pthread_join(TCP_SERVER_CMD_tid,NULL);
	pthread_join(TCP_SERVER_VIDEO_tid,NULL);

	

	/*关闭文件描述符*/ 
	close(UART2_fd);
	close(UART1_fd);
			
	return 0;

}




