#ifndef _INIT_DEV_H_
#define _INIT_DEV_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//***********************************************************
//*********************UDP协议参数设置***********************
int UDP_sockfd;
struct sockaddr_in UDP_Server_addr;

//***********************************************************
//******串口描述符*******************************************
extern int UART3_fd,UART2_fd,UART1_fd; 


int UDP_TO_SERVER(int sockfd, unsigned char* PDATA_Buffer, unsigned int DATA_length);
int UDP_Init(void);


//int TCP_SERVER_Init(int * TCP_Accept_fd,int TCP_SERVER_PORT);
int TCP_SERVER_Init(int TCP_Server_sockfd,int * TCP_Accept_fd,int TCP_SERVER_PORT);

//int TCP_Send_Client(int sockfd, unsigned char* PDATA_Buffer, unsigned int DATA_length);
//int TCP_recv_Client(int sockfd, unsigned char* PDATA_Buffer, unsigned int DATA_length);

int TCP_Send_Client(int sockfd,  char* PDATA_Buffer,  int DATA_length);
int TCP_recv_Client(int sockfd,  char* PDATA_Buffer,  int DATA_length);

int UART3_Init(void);           //串口3初始化
int UART2_Init(void);           //串口2初始化
int UART1_Init(void);           //串口1初始化

void Uart_Ack(int fd, unsigned char *buf);

#endif







