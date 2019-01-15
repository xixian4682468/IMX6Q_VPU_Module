#include <stdio.h>  
#include <errno.h>  
#include <sys/stat.h>  
#include <fcntl.h>    /* 文件控制定义*/  
#include <termios.h>  /* PPSIX 终端控制定义*/  
#include <stdlib.h>   
#include <string.h>   
#include <sys/types.h>
#include <unistd.h>  

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "Init_dev.h"

//*********************UART参数设置***********************
int UART3_fd,UART2_fd,UART1_fd; 

//*********************UDP协议参数设置***********************
#define UDP_SERVER_PORT 9201
#define BUFF_LEN 512
#define UDP_SERVER_IP "192.168.0.41"


//*******************TCP网络协议相关参数*********************
//struct sockaddr_in TCP_Server_addr;   //定义sockaddr_in 网络套接字
//struct sockaddr_in TCP_Client_addr;
#define TCP_SERVER_IP "192.168.1.70"

//***********************************************************


int UDP_Init(void)
{
   UDP_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
   if(UDP_sockfd < 0)
   {
		printf("create socket fail!\n");
		return -1;
   }

   memset(&UDP_Server_addr, 0, sizeof(struct sockaddr_in));
   UDP_Server_addr.sin_family = AF_INET;
   UDP_Server_addr.sin_addr.s_addr = inet_addr(UDP_SERVER_IP);	 //注意网络序转换
   UDP_Server_addr.sin_port = htons(UDP_SERVER_PORT);			 //注意网络序转换
   return 0;
}


//***********************************************************************
// sockfd :设备描述符
// PDATA_Buffer :发送数据地址
// DATA_length: 发送数据长度
//***********************************************************************
int UDP_TO_SERVER(int sockfd, unsigned char* PDATA_Buffer, unsigned int DATA_length)
{
	//unsigned char infor[512] = "hello kitty !!!!!";
	//sendto(sockfd, infor, 512, 0, (struct sockaddr*)&UDP_Server_addr, sizeof(struct sockaddr_in));

	sendto(sockfd, PDATA_Buffer, DATA_length, 0, (struct sockaddr*)&UDP_Server_addr, sizeof(struct sockaddr_in));
	return 0;
}

//***********************************************************************
// TCP_Init()：  TCP服务端初始化
//
//***********************************************************************
int TCP_SERVER_Init(int TCP_Server_sockfd,int * TCP_Accept_fd,int TCP_SERVER_PORT)
{
	//定义sockfd
	//int TCP_Server_sockfd;
	struct sockaddr_in TCP_Server_addr;   //定义sockaddr_in 网络套接字
	struct sockaddr_in TCP_Client_addr;
	
	TCP_Server_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	TCP_Server_addr.sin_family = AF_INET;
	TCP_Server_addr.sin_port = htons(TCP_SERVER_PORT);
	TCP_Server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//TCP_Server_addr.sin_addr.s_addr = inet_addr(TCP_SERVER_IP);

	///bind，成功返回0，出错返回-1
	if(bind(TCP_Server_sockfd, (struct sockaddr *)&TCP_Server_addr, sizeof(TCP_Server_addr))==-1)
	{
		perror("TCP bind error !!!\n");
		return -1;
	}

	printf("listen %d port !!!\n", TCP_SERVER_PORT);
	///listen，成功返回0，出错返回-1
	if(listen(TCP_Server_sockfd, 20) == -1)
	{
		perror("TCP listen error !!!\n");
		return -1;
	}

	///客户端套接字
	//struct sockaddr_in TCP_Client_addr;
	socklen_t TCP_Client_addr_length = sizeof(TCP_Client_addr);

	printf("TCP waiting client connnecting!!!\n");
	///成功返回非负描述字，出错返回-1
	*TCP_Accept_fd = accept(TCP_Server_sockfd, (struct sockaddr*)&TCP_Client_addr, &TCP_Client_addr_length);
	if(TCP_Accept_fd < 0)
	{
		perror("TCP Accept error !!! \n");
		return -1;
	}
	//printf("TCP Client connected !!!\n");

	return 0;
}

//***********************************************************************
// sockfd :设备描述符
// PDATA_Buffer :发送数据地址
// DATA_length: 发送数据长度
//***********************************************************************
//int TCP_Send_Client(int sockfd, unsigned char* PDATA_Buffer, unsigned int DATA_length)

int TCP_Send_Client(int sockfd, char* PDATA_Buffer, int DATA_length)
{
	int ret;
	ret = send(sockfd, PDATA_Buffer, DATA_length, 0);
	if(ret < 0)
	{
		printf("TCP Send Client data error !!! \n");
		return -1;
	}
	return 0;
}

//***********************************************************************
// sockfd :设备描述符
// PDATA_Buffer :接收数据地址
// DATA_length: 接收数据空间长度
// len:实际接收数据长度
//***********************************************************************
//int TCP_recv_Client(int sockfd, unsigned char* PDATA_Buffer, unsigned int DATA_length)

int TCP_recv_Client(int sockfd, char* PDATA_Buffer, int DATA_length)
{
	int len;
	len = recv(sockfd, PDATA_Buffer, DATA_length, 0);
	if(len <= 0)
	{
		printf("TCP recv Client data error !!! \n");
		return len;
	}
	return len;
}


int set_serial(int fd,int nSpeed,int nBits,char nEvent,int nStop)
{
    struct termios newttys1,oldttys1;

	/*保存原有串口配置*/  
	if(tcgetattr(fd,&oldttys1)!=0)
	{  
		perror("Setupserial 1");
		return -1;  
	}  
	memset(&newttys1, 0, sizeof(newttys1));/* 先将新串口配置清0 */  
	newttys1.c_cflag |= (CLOCAL|CREAD ); /* CREAD 开启串行数据接收，CLOCAL并打开本地连接模式 */  

	newttys1.c_cflag &=~CSIZE;/* 设置数据位 */
	/* 数据位选择 */  
	switch(nBits)  
	{  
		case 7:  
			newttys1.c_cflag |=CS7;  
			break;  
		case 8:  
			newttys1.c_cflag |=CS8;  
			break;  
	}  
	/* 设置奇偶校验位 */  
	switch( nEvent )  
	{  
		case '0': /* 奇校验 */  
			newttys1.c_cflag |= PARENB;/* 开启奇偶校验 */  
			newttys1.c_iflag |= (INPCK | ISTRIP);/*INPCK打开输入奇偶校验；ISTRIP去除字符的第八个比特  */  
			newttys1.c_cflag |= PARODD;/*启用奇校验(默认为偶校验)*/  
			break;  
		case 'E': /*偶校验*/  
			newttys1.c_cflag |= PARENB; /*开启奇偶校验  */  
			newttys1.c_iflag |= (INPCK | ISTRIP);/*打开输入奇偶校验并去除字符第八个比特*/  
			newttys1.c_cflag &= ~PARODD;/*启用偶校验*/  
			break;  
		case 'N': /*无奇偶校验*/  
			newttys1.c_cflag &= ~PARENB;  
			break;  
	}  
	/* 设置波特率 */  
    switch( nSpeed )  
    {  
        case 2400:  
            cfsetispeed(&newttys1, B2400);  
            cfsetospeed(&newttys1, B2400);  
            break;  
        case 4800:  
            cfsetispeed(&newttys1, B4800);  
            cfsetospeed(&newttys1, B4800);  
            break;  
        case 9600:  
            cfsetispeed(&newttys1, B9600);  
            cfsetospeed(&newttys1, B9600);  
            break;  
        case 115200:  
            cfsetispeed(&newttys1, B115200);  
            cfsetospeed(&newttys1, B115200);  
            break;  
        default:  
            cfsetispeed(&newttys1, B9600);  
            cfsetospeed(&newttys1, B9600);  
            break;  
    }  
    /*设置停止位*/  
    if( nStop == 1)/* 设置停止位；若停止位为1，则清除CSTOPB，若停止位为2，则激活CSTOPB */  
    {  
        newttys1.c_cflag &= ~CSTOPB;/*默认为一位停止位； */  
    }  
    else if( nStop == 2)  
    {  
        newttys1.c_cflag |= CSTOPB;/* CSTOPB表示送两位停止位 */  
    }  
  
    /* 设置最少字符和等待时间，对于接收字符和等待时间没有特别的要求时*/  
    newttys1.c_cc[VTIME] = 0; /* 非规范模式读取时的超时时间；*/  
    newttys1.c_cc[VMIN]  = 0; /* 非规范模式读取时的最小字符数*/  
    tcflush(fd ,TCIFLUSH);    /* tcflush清空终端未完成的输入/输出请求及数据；TCIFLUSH表示清空正收到的数据，且不读取出来 */  
  
    /*激活配置使其生效*/  
    if((tcsetattr(fd, TCSANOW,&newttys1))!=0)  
    {  
        perror("com set error");  
        exit(1);  
    }  
  
    return 0;  
}


int UART1_Init(void)           //串口1初始化
{
	if((UART1_fd = open("/dev/ttymxc1",O_RDWR|O_NOCTTY|O_NDELAY))<0)  
	{   
		perror("Can't Open the /dev/ttymxc1 Serial Port!!!! \n");  
		return -1;
	} 
	//set_serial(UART1_fd, 115200, 8, 'N', 1);
	set_serial(UART1_fd, 19200, 8, 'N', 1);
	return 0;
}


int UART2_Init(void)           //串口2初始化
{
	if((UART2_fd = open("/dev/ttymxc2",O_RDWR|O_NOCTTY|O_NDELAY))<0)    
	{  
		perror("Can't Open the /dev/ttymxc2 Serial Port");  
		return -1;
	} 
	set_serial(UART2_fd, 115200, 8, 'N', 1); 
	return 0;
}


int UART3_Init(void)           //串口3初始化
{
	if((UART3_fd = open("/dev/ttymxc3",O_RDWR|O_NOCTTY|O_NDELAY))<0)    
	{  
		perror("Can't Open the /dev/ttymxc3 Serial Port");  
		return -1;
	} 
	set_serial(UART3_fd, 115200, 8, 'N', 1); 
	return 0;
}


void Uart_Ack(int fd, unsigned char *buf)
{
	int ret;
	unsigned short NUM;
	NUM = sizeof(buf);
	ret = write(fd, buf, NUM);
	if(ret<0)
		printf("Uart_Ack error!!!\n");
}





