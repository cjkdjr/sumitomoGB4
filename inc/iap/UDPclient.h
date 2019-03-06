/*
 * UDPclient.h
 *
 *  Created on: 2018年1月10日
 *      Author: tykj
 */

#ifndef UDPCLIENT_H_
#define UDPCLIENT_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<errno.h>
#include<sys/types.h>
#include <fcntl.h>

//#define BUFLEN 1500
int sockfd;//socket套接字
struct sockaddr_in address;//处理网络通信的地址

int UDPclient(char *IP, int port);
int UDPrecv();
int UDPsend(unsigned char *buf,int len);
extern int GSM_SocketTrans();
extern int GSM_UdpClose();
#endif /* UDPCLIENT_H_ */
