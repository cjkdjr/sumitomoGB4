/*
 * TCPsocket.h
 *
 *  Created on: 2017年11月9日
 *      Author: tykj-guozhiyue
 */

#ifndef TCPSOCKET_H_
#define TCPSOCKET_H_

#include <netinet/in.h>

#include "general.h"

#define BUFLEN 5000
extern int client_sockfd;                                          //socket 句柄
struct sockaddr_in remote_addr;                                    //服务端网络地址结构体
pthread_t Recv_ID;
pthread_t Send_ID;
int slen;                                                    //接收到一包数据的长度
fd_set rfds;                                                       //读文件描述符
fd_set wrds;                                                     //写文件描述符
struct timeval tv;                                                 //设置超时时间
int retval, rmaxfd, smaxfd;
char RecvBuf[BUFLEN];                                              // 接收数据缓冲区
char SendBuf[BUFLEN];                                              // 发送数据缓冲区



extern u16 GPRS_flag;   //定义：GPRS_flag值，0-没有数据发送；1-数据库或对列有数据send；2-发送失败

extern int Msg_Spell(void* Msg_in, u8* SpellStr);//将出队消息拼串
void Switch_IP(u8 IpState);                                        //切换IP
extern int Get_IMEI(u8 *strIMEI);                                       //获取IMEI号，编辑消息体用
extern int servMsgHandle(u8 *Msg, u16 len);                        //中心下行message分流处理
extern int TCPSocketConnect(u8 * IPaddr, uint16_t port);         //打开socket并建立链接
extern int TCPSocketClose(void);                                   //关闭socket
extern void *TCPSocketRecvpThread(void *argv);                     //socket接收数据线程函数
extern void *TCPSocketSendpThread(void *argv);                     //socket发送数据线程函数
extern int  TCP_Connect();                                         //打开TCP链接
extern int  OpenTCPConnect(u8 *IPaddr, uint16_t port);           //创建开socket链接
extern int  TCPSocketRestart(void);                                //socket重启
extern uint16_t Msg_DeConvert(u8 *rdata, u8 *data, uint16_t len);         //转义还原
extern uint16_t Msg_Convert(u8 *data, uint16_t len);                      //转义
void StickPackageHandle(char *Buf, int buflen);                    //粘包处理
extern int Read_Para_set(void);                                    //开机读取配置参数信息

#endif /* TCPSOCKET_H_ */
