/*
 * TCPsocket.c
 *
 *  Created on: 2017年11月9日
 *      Author: tykj-guozhiyue
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include<arpa/inet.h>

#include "TCPsocket.h"
#include "gsm.h"
#include "api.h"
#include "Lz77.h"
#include "iap.h"
#include "Time.h"
#include "message_process.h"
#include "Msg_task.h"

int client_sockfd = 0; //socket 句柄
char StickBuf[BUFLEN] = { 0 }; //粘包处理
u16 GPRS_flag = 0;

/*-----------------------------------------------------
 Name    :Switch_IP()
 Funciton:切换IP ，标记状态
 ------------------------------------------------------*/
void Switch_IP(u8 IpState) {
	u8 IP_SwitchFlag = 0;
	if (SYS_SET2.SET_IP_CUT == 0x00) /*auto switch ip*/
	{
		IpState = 0X00; //置终端的IP切换功能
		IP_SwitchFlag = 1;
	} else if (SYS_SET2.SET_IP_CUT == 0x01) {
		IpState = 0X01; //置终端的默认IP
		IP_SwitchFlag = 0;
	} else if (SYS_SET2.SET_IP_CUT == 0x02) {
		IpState = 0X02; //置终端的备用IP
		IP_SwitchFlag = 0;
	}
	if (IP_SwitchFlag == 1) {
	}
}
void IPconvert(u8 *centerIP, u8 *IP, u16 *port) {
	//printf("port:%d\n",port);
	sprintf((char*) IP, "%d.%d.%d.%d", centerIP[1], centerIP[2], centerIP[3], centerIP[4]);
	*port = centerIP[5];
	*port <<= 8;
	*port |= centerIP[6];
	//printf("IP:%s,port:%d\n",IP,port);
}
/******************************************************
 FUNC:打开TCP链接，并创建收发线程
 ******************************************************/
int TCP_Connect() {
	Socket_Flag = 0;
	int ret = 0;

	u8 IPaddr1[16] = { 0 }; //默认IP
	u8 IPaddr2[16] = { 0 }; //备用IP
	u16 port1 = 0, port2 = 0;
	IPconvert(SYS_SET2.SET_IP_DEFAULT, IPaddr1, &port1);
	IPconvert(SYS_SET2.SET_IP_BACKUP, IPaddr2, &port2);
//	u8 IPaddr1[16] = "192.168.9.99";   //默认IP192.168.9.99:5500
//	u8 IPaddr2[16] = "192.168.9.99";  //备用IP
//	u16 port1 = 5500, port2 = 5500;
	printf("IPaddr1:%s,port1:%d\n", IPaddr1, port1);
	printf("IPaddr2:%s,port2:%d\n", IPaddr2, port2);

	if (SYS_SET2.SET_IP_CUT == 0X00) { //默认切换
		ret = OpenTCPConnect(IPaddr1, port1);
		if (ret < 0) {
			Switch_IP(SYS_SET2.SET_IP_CUT);
			ret = OpenTCPConnect(IPaddr2, port2);
		}
	} else if (SYS_SET2.SET_IP_CUT == 0X01) { //默认IP
		ret = OpenTCPConnect(IPaddr1, port1);
	} else if (SYS_SET2.SET_IP_CUT == 0X02) { //备用IP
		ret = OpenTCPConnect(IPaddr2, port2);
	}
	return ret;
}
int OpenTCPConnect(u8 *IPaddr, uint16_t port) {
	int rc1, rc2, count, rt = 0;
	//重链接备用IP逻辑
	for (count = 0; count < SYS_SET2.SET_IP_CUTMAX; count++) {
		gsm_PthParam.sta = 1;
		int ret = TCPSocketConnect(IPaddr, port); //打开socket链接
		if (ret > 0) { //创建收发线程
			if (SockRecv_PthParam.id > 0 || SockSend_PthParam.id > 0) {
				TCPSocketClose();
			}
			printf_gsm("build socket success!\n");
			pthread_attr_init(&SockRecv_PthParam.attr); //创建线程属性分离状态，线程中设置异步结束信号
			pthread_attr_setdetachstate(&SockRecv_PthParam.attr, PTHREAD_CREATE_JOINABLE);
			rc1 = pthread_create(&SockRecv_PthParam.id, &SockRecv_PthParam.attr, TCPSocketRecvpThread, NULL);
			if (rc1 != 0) {
				printf_gsm("ERROR; return code from pthread_create() is %d\n", rc1);
				rt = -1;
				break;
			}
			pthread_attr_init(&SockSend_PthParam.attr);
			pthread_attr_setdetachstate(&SockSend_PthParam.attr, PTHREAD_CREATE_JOINABLE);
			rc2 = pthread_create(&SockSend_PthParam.id, &SockSend_PthParam.attr, TCPSocketSendpThread, NULL);
			if (rc2 != 0) {
				printf_gsm("ERROR; return code from pthread_create() is %d\n", rc2);
				rt = -1;
				break;
			}
			//Socket_Flag = 0; //链接成功，重连标志位清零
			rt = 1;
			break;
		} else {
			printf_gsm("build socket failed!\n");
			//Socket_Flag = 1; //创建socket失败，需要重新创建
			rt = -1;
			sleep(1);
		}
	}
	return rt;
}

/***********************************************************
 NAME:TCPSocketConnect
 FUNC:创建socket链接
 PARA：
 char  *IPaddr:ip地址
 uint16_t port:端口号
 返回值：成功返回1，失败返回-1
 * **********************************************************/
int TCPSocketConnect(u8 *IPaddr, uint16_t port) {
	printf("in function TCPSocketConnect IP:%s,port:%d!\n", IPaddr, port);
	if (client_sockfd > 0) {
		close(client_sockfd);
		client_sockfd = 0;
	}
	if ((client_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		close(client_sockfd);
		client_sockfd = -1;
		if (debug_value & 0x01) {
			perror("socket");
		}
		return -1;
	} else
	printf_gsm("client_socket:%d\n", client_sockfd);

	bzero(&remote_addr, sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = inet_addr((char*) IPaddr);
	remote_addr.sin_port = htons(port);
	if (inet_aton((char*) IPaddr, (struct in_addr *) &remote_addr.sin_addr.s_addr) == 0) {
		if (debug_value & 0x01) {
			perror((char*) IPaddr);
		}
		close(client_sockfd);
		client_sockfd = -1;
		return -1;
	} else
	printf_gsm("IPaddr:%s;port:%d !\n", IPaddr, port);

	struct timeval timeout;
	timeout.tv_sec = 5;  //秒
	timeout.tv_usec = 0;  //微秒
	if (setsockopt(client_sockfd, SOL_SOCKET, SO_RCVTIMEO | SO_SNDTIMEO, &timeout, sizeof(timeout)) == -1) {
		perror("setsockopt failed:");
		close(client_sockfd);
		client_sockfd = 0;
		return -1;
	}
	if (connect(client_sockfd, (struct sockaddr *) &remote_addr, sizeof(struct sockaddr)) < 0) {
		printf_gsm("connect failed!\n");
		if (debug_value & 0x01) {
			perror("connect");
		}
		close(client_sockfd);
		client_sockfd = -1;
		return -1;
	} else
	printf_gsm("connected to server!\n");
	fcntl(client_sockfd, F_SETFL, fcntl(client_sockfd, F_GETFL, 0) | O_NONBLOCK);
	return 1;
}
/*****************************************************************
 NAME:TCPSocketClose
 FUNC:关闭socket
 PARA:  void
 *****************************************************************/
int TCPSocketClose(void) {
	if (client_sockfd <= 0)
		return 0;
	int err = 0;
	printf_gsm("sending cancel info. \n");
	SockSend_PthParam.flag = FALSE;
	SockRecv_PthParam.flag = FALSE;
	usleep(500 * 1000);
	if (SockRecv_PthParam.id > 0) {
		if ((err = pthread_cancel(SockRecv_PthParam.id)) != 0) { //向线程发送结束取消信号，成功返回0
			printf_gsm("line :%04d; error - %s\n", __LINE__, strerror(err));
		}
		if ((err = pthread_join(SockRecv_PthParam.id, NULL)) != 0) { //回收线程资源
			printf_gsm("line :%04d; error - %s\n", __LINE__, strerror(err));
		}
	}
	if (SockSend_PthParam.id > 0) {
		if ((err = pthread_cancel(SockSend_PthParam.id)) != 0) {
			printf_gsm("line :%04d; error - %s\n", __LINE__, strerror(err));
		}
		if ((err = pthread_join(SockSend_PthParam.id, NULL)) != 0) {
			printf_gsm("line :%04d; error - %s\n", __LINE__, strerror(err)); //return 0;
		}
	}

	printf_gsm("thread has been canceled.\n");
	close(SockSend_PthParam.id);
	close(SockRecv_PthParam.id);
	close(client_sockfd);
	SockSend_PthParam.id = 0;
	SockRecv_PthParam.id = 0;
	client_sockfd = 0;
	printf_gsm("client_sockfd:%d\n", client_sockfd);
	return 0;
}

int TCPSocketRestart(void) {
	GPRS_flag = 0;
	TCPSocketClose();
	int ret = TCP_Connect();
	return ret;
}
/*****************************************************
 NAME:Get_IMEI
 FUNC:获取IMEI号，供编辑完整消息格式体使用
 PARA:
 char *strIMEI:保存获取到到IMEI号
 Return: 0成功。1失败
 *****************************************************/
int Get_IMEI(u8 *strIMEI) {
	if (strlen((char*) gsm_info_cur.imei) != 0) {
		strIMEI[0] = 0X0F;
		memcpy(strIMEI + 1, gsm_info_cur.imei, 8);
		//strIMEI[9] = '\0';
		printf_gsm("\n Get_IMEI:");
		if (debug_value & 0x01) {
			api_PrintfHex(strIMEI, 9);
		}
//		printf("\n");
	} else {
		memset(strIMEI, 0, 9);
		return 1;
	}
	return 0;
}
/******************************************************
 NAME:TCPSocketpThread
 FUNC :socket 线程接收数据
 * ****************************************************/
int rTimeOut() {
	/*把可读文件描述符的集合清空*/
	FD_ZERO(&rfds);
	/*把标准输入的文件描述符加入到集合中*/
//	FD_SET(0, &rfds);
	rmaxfd = 0;
	/*把当前连接的文件描述符加入到集合中*/
	FD_SET(client_sockfd, &rfds);
	/*找出文件描述符集合中最大的文件描述符*/
	if (rmaxfd < client_sockfd)
		rmaxfd = client_sockfd;
	/*设置超时时间*/
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	retval = select(rmaxfd + 1, &rfds, NULL, NULL, &tv);
	return retval;
}
int sTimeOut() {
	int ret = -1;
	/*把可读文件描述符的集合清空*/
	FD_ZERO(&wrds);
	/*把标准输入的文件描述符加入到集合中*/
//  FD_SET(0, &wrds);
	smaxfd = 0;
	/*把当前连接的文件描述符加入到集合中*/
	FD_SET(client_sockfd, &wrds);
	/*找出文件描述符集合中最大的文件描述符*/
	if (smaxfd < client_sockfd)
		smaxfd = client_sockfd;
	/*设置超时时间*/
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	ret = select(smaxfd + 1, NULL, &wrds, NULL, &tv);
	return ret;
}
void *TCPSocketRecvpThread(void *argv) {
	printf_gsm("############## %s start ##############\n", __FUNCTION__);
//	Get_Now_Time();
	int ret = 0;
//	int errno;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); //设置异步属性
	SockRecv_PthParam.flag = TRUE;
	while (SockRecv_PthParam.flag) {
		ret = rTimeOut();
		if (ret == -1) {
			printf_gsm("select error,socket need exist and restart!\n");
			Socket_Flag = 1; //置位重新链接socket
			break;
		} else if (ret == 0) {
//			printf("no data send !\n\n");
			usleep(100 * 1000);
			continue;
		} else {
			if (FD_ISSET(client_sockfd, &rfds)) /*服务器发来了消息*/
			{
				/******接收消息*******/
				bzero(RecvBuf, BUFLEN);
				slen = recv(client_sockfd, RecvBuf, BUFLEN, 0);
				if (slen > 0) {
					printf_gsm("the Info len: %d ;Info from server is:", slen);
					if (debug_value & 0x01) {
						api_PrintfHex((u8*) RecvBuf, slen);
					}
					//粘包处理//消息处理
					StickPackageHandle(RecvBuf, slen);
				} else {
					if (slen < 0 && (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)) {
						continue;
					} else {
						if (debug_value & 0x01) {
							perror("recv");
						}
						printf_gsm("The server disconnects, the client program needs exit and restart ! \n");
						Socket_Flag = 1; //置位重新链接socket
						sleep(5);
						break;
					}
				}
			}
		}
	}
	printf_gsm("############## %s exit ##############\n", __FUNCTION__);
	return NULL;
}
/***************************************************
 NAME:TCPSocketSend
 FUNC:发送socket数据
 PARA:
 char* buf:要发送的字符串
 int   len:字符串的长度
 **************************************************/
void *TCPSocketSendpThread(void *argv) {
	printf_gsm("############## %s start ##############\n", __FUNCTION__);
//	Get_Now_Time();
	char *p; //信息出对指针
	u8 buf[BUFLEN] = { 0 };
	u8 strIMEI[9] = { 0 }; //IMEI号拼串用
	u8 str1[BUFLEN] = { 0 };
	uint16_t nbuf = 0;
	int type = -1; //出队类型，0=sqlite3,1=queue.
	int ret = 0;
	int count = 0;
	u32 start_time = api_GetSysmSecs();
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL); //设置异步属性
	SockSend_PthParam.flag = TRUE;
	while (SockSend_PthParam.flag) {
		if (sqlte_PassThrough_count > 0) { //优先取数据库数据
			GPRS_flag = 1; //有数据发送
			p = (char*) malloc(sizeof(QUE_TDF_QUEUE_MSG));
			que_sqlite_out((u8*) p, PassThrough_table,sqlte_PassThrough_count,PassThrough_key_sn);
			que_sqlite_del(PassThrough_table,sqlte_PassThrough_count,PassThrough_key_sn); //出库后删除该记录
			type = 0;
			nbuf = Msg_Spell(p, buf); //出队后添加数据压缩功能
			free(p);
		} else if (PassThrough_count > 0) { //数据库空时取队列数据
			GPRS_flag = 1; //有数据发送
			p = (char*) malloc(sizeof(QUE_TDF_QUEUE_MSG));
			PassThrough_out(p);
			type = 1;
			nbuf = Msg_Spell(p, buf); ///出队后添加数据压缩功能
			free(p);
		} else {
			GPRS_flag = 0; //有数据发送
			usleep(10 * 1000); //没有数据时300s发心跳

			if (api_DiffSysMSecs(start_time) >= 300 * 1000) {
				printf_gsm("send heart ! \n");
				send(client_sockfd, "hello", 6, 0); //MSG_WAITALL,MSG_DONTWAIT
				start_time = api_GetSysmSecs();
			}
			continue;
		}
		Get_IMEI(strIMEI); //获取IMEI号
		if (strIMEI[0] == 0) {
			continue;
		}
		memcpy(str1, strIMEI, 9); //IMEI号
		memcpy(str1 + 9, buf, nbuf); //队列中消息
		str1[nbuf + 9] = api_CheckSum(str1, nbuf + 9); //添加CRC校验码
		uint16_t mlen = Msg_Convert(str1, nbuf + 10); //转义并添加7E头尾

		ret = sTimeOut();
//		if (ret == -1)
		if (ret <= 0) {
			perror("select");
			printf_gsm("send select error,socket need exist and restart  ret:%d, client_sockfd:%d!\n", ret, client_sockfd);
			Socket_Flag = 1; //置位重新链接socket
			break;
//		} else if (ret == 0)
//		{
//			printf("send timeout !!\n\n");
//			usleep(1000 * 1000);
//			continue;
		} else {
			if (client_sockfd > 0) { //MSG_NOSIGNAL
				int len = send(client_sockfd, str1, mlen, 0);
				if (len > 0) {
					if (type == 1) {
						PassThrough_delete(); //出队后删除
						type = -1;
					} else if (type == 0) {
						type = -1;
					}
					printf_gsm("Send %d byte  count: %d tiao info , info type:%02X !\n", len, count++, str1[10]);
//					api_PrintfHex(str1, mlen);
					if (debug_value & 0x01) {
						Get_Now_Time();
					}
					if (SYS_SET2.RESESTER_NET == 2) {
						usleep(20 * 1000);
					} else {
						usleep(100 * 1000);
					}
				} else {
					GPRS_flag = 2; //有数据发送
//					System.DaySummary.GPRSErrCount++; //当天发送GPRS信息失败次数
					printf_gsm("send data error socket_flag=1,socket need to restart ! \n");
					Socket_Flag = 1; //置位重新链接socket
					break;
				}
				start_time = api_GetSysmSecs();
			} else {
				printf_gsm("client_sockfd error socket_flag=1,socket need to restart ! \n");
				Socket_Flag = 1; //置位重新链接socket
				break;
			}
			memset(str1, 0X00, sizeof(str1));
			memset(buf, 0X00, sizeof(buf));
		}
		pthread_testcancel();
	}
	printf_gsm("############## %s exit ##############\n", __FUNCTION__);
	return NULL;
}
/******************************************************
 NAME:StickPackageHandle
 FUNC :粘包处理
 PARA  :
 char*Buf:需要进行粘包处理到串
 int buflen:串的长度
 *******************************************************/
int nStickBuf = 0;
void StickPackageHandle(char *Buf, int buflen) {
//api_PrintfHex((u8*)Buf,buflen);
	int i = 0;
	int nbuf = -1;
	bool red = FALSE; //记录是否有舍弃非7E 开头的串
	memcpy(StickBuf + nStickBuf, Buf, buflen);
	nStickBuf += buflen;
	while (nStickBuf > 0) {
		if (StickBuf[0] == 0X7E) { //7E开头
			for (i = 1; i < nStickBuf; i++) {
				if (StickBuf[i] == 0X7E) { //7E结尾
					nbuf = i; //记录7E 结尾出现的位置
					break;
				} else
					nbuf = -1; //7E 开头没有7E 结尾
			}
		} else { //不是7E开头，找到7E开头，7E 前舍弃保留后边的，没有直接舍弃
			for (i = 0; i < nStickBuf; i++) {
				if (StickBuf[i] == 0X7E) { //找到7E开头的子串，保留后边到，舍弃前边的
					int nlast = nStickBuf - i; //7E后串的长度
					char *str = (char*) malloc((nlast + 1) * sizeof(char));
					memcpy(str, StickBuf + i, nlast);
					memset(StickBuf, 0, nStickBuf);
					memcpy(StickBuf, str, nlast);
					StickBuf[nlast] = '\0';
					free(str);
					str = NULL;
					red = TRUE;
					break;
				} else
					red = FALSE;
			}
			if (red == TRUE)
				continue;
			else {
				memset(StickBuf, 0, nStickBuf);
				nStickBuf = 0;
				break;
			}
		}
		if (nbuf != -1) { //至少有一包7E开头7E结尾
			int nlast = nStickBuf - nbuf - 1; //截取7E**7E包后串的剩余长度
			u8 *strDeconvert = (u8*) malloc((nStickBuf - 2) * sizeof(char)); //用于存放转义后的串
			uint16_t nlen = 0; //转义后串的长度
			if (nlast == 0) { //没有粘包，直接处理
				nlen = Msg_DeConvert(strDeconvert, (u8*) StickBuf, nStickBuf); //数据转义还原
				servMsgHandle(strDeconvert, nlen); //数据处理
				//api_PrintfHex(strDeconvert, nlen);    //打印十六进制串，测试用
				memset(StickBuf, 0, nStickBuf);
				nStickBuf = nlast;
				break;
			} else if (nlast > 0) { //有粘包
//				printf("nbuf:%d , nlast:%d\n", nbuf, nlast);
				char * str1 = (char*) malloc((nbuf + 1 + 1) * sizeof(char));
				char * str2 = (char*) malloc((nlast + 1) * sizeof(char));
				memcpy(str1, StickBuf, nbuf + 1); //str1需要转义还原并作相应数据处理
				str1[nbuf + 1] = '\0';
				nlen = Msg_DeConvert(strDeconvert, (u8*) str1, nbuf + 1); //数据转义还原
				servMsgHandle(strDeconvert, nlen); //数据处理
				//api_PrintfHex(strDeconvert, nlen);    //打印十六进制串，测试用
				memcpy(str2, StickBuf + (nbuf + 1), nlast); //7E结尾后的串存入StickBuf
				str2[nlast] = '\0';
//				printf("str2:%s\n\n", str2);
				memset(StickBuf, 0, nStickBuf);
				memcpy(StickBuf, str2, nlast); //等待下一次循环处理
				StickBuf[nlast] = '\0';
				free(str1);
				free(str2);
				str1 = NULL;
				str2 = NULL;
				nStickBuf = nlast;
			}
			free(strDeconvert);
			strDeconvert = NULL;
		} else
			break;
	}

	return;
}
/******************************************************
 NAME:Msg_Convert
 FUNC :转义：7D->7D01；7D->7E02,并在开头结尾添加7E
 PARA:
 u8 *data:传入的字符串
 uint16_t len:串长度
 Return:转义后字符串的长度
 *****************************************************/
uint16_t Msg_Convert(u8 *data, uint16_t len) {
	u8 *rdata = (u8*) malloc(BUFLEN * sizeof(char));
	uint16_t index = 0, i;
	rdata[index++] = 0X7E;
	for (i = 0; i < len; i++) {
		if (data[i] == 0x7E) {
			rdata[index++] = 0x7D;
			rdata[index++] = 0x02;
		} else if (data[i] == 0x7D) {
			rdata[index++] = 0x7D;
			rdata[index++] = 0x01;
		} else {
			rdata[index++] = data[i];
		}
	}
	rdata[index++] = 0x00;
	rdata[index++] = 0x7E;
	memcpy(data, rdata, index);
	free(rdata);
	rdata = NULL;
	return index;
}
/****************************************************
 NAME:Msg_EscapeRestore
 FUNC:转义还原：7D01->7D；7D02->7E
 PARA:
 u8 *rdata:转义还原后的串
 u8 *data :原始需要转义的串
 uint16_t len:原始字符串长度
 Return:转义还原后的串的长度
 ****************************************************/
uint16_t Msg_DeConvert(u8 *rdata, u8 *data, uint16_t len) {
	uint16_t index, n, i;
	index = 0;
	n = 0;
	if (data[0] == 0x7E)
		n++;
	for (i = 0; i < len; i++) {
		if (data[n] == 0x7D) {
			if (data[n + 1] == 0x01) {
				rdata[index++] = 0x7D;
				n++;
			} else if (data[n + 1] == 0x02) {
				rdata[index++] = 0x7E;
				n++;
			} else
				rdata[index++] = 0x7D;
		} else if (data[n] == 0x7E) {
			break;
		} else {
			rdata[index++] = data[n];
		}
		n++;
	}
	return index;
}

/*******************************************************
 NAME: Read_Para_set
 FUNC: 开机读取配置参数信息
 PARA: void
 Return:
 *******************************************************/
int Read_Para_set(void) {
	InitE5();
	return 0;
}

/*******************************************************
 NAME: servMsgHandle
 FUNC: 中心下行message分流处理
 PARA:u8 *Msg:需要解码的串
 u16 len:需解码串的长度
 Return:
 *******************************************************/
int servMsgHandle(u8 *Msg, u16 len) {
//	printf("servMsgHandle:");
//	api_PrintfHex(Msg, len);         //打印十六进制串，测试用
	int offest = 0;
	u8 sLen[2] = { 0 };
	u8 ssLen[4] = { 0 };
	u8 sCRC = 0;
	u8 cSum = 0;
	RecMessage sRecMsg;
	memcpy(&sRecMsg.MsgType, Msg + offest, 1); //消息类型
	//sRecMsg.MsgType[1] = '\0';
	offest += 1;
	memcpy(sRecMsg.Attribute, Msg + offest, 2); //消息属性
	offest += 2;
	memcpy(sLen, Msg + offest, 2); //消息体长度十六进制形式
	offest += 2;
	api_HexToAsc(sLen, ssLen, 2); //先转成文本，下一步十六进制串转十进制
	sRecMsg.MsgLen = (HexStr2Int((char*) ssLen, 4)) / 8; //消息体长度
	printf_gsm("MsgLen:%d\n", sRecMsg.MsgLen);
	if (sRecMsg.MsgLen > 1500) {
		printf_gsm("Message length error !\n");
		return -1;
	} else {
		memcpy(sRecMsg.data, Msg + offest, sRecMsg.MsgLen); //消息体
		sRecMsg.data[sRecMsg.MsgLen] = '\0';
	}
	offest += sRecMsg.MsgLen;
	memcpy(&sCRC, Msg + offest, 1); //消息体校验码
	offest += 1;
	cSum = api_CheckSum(Msg, len - 2); //计算结果校验码
	if (sCRC != cSum) { //如果结算结果于消息体中校验码不匹配，作出错处理
		printf_gsm("CRC error!\n");
		return -1;
	}
	//中心下发信息分流处理
	switch (sRecMsg.MsgType)
	{

	default:
		printf_gsm("Message type error !\n");
		break;
	}

	return 0;
}
/************************************************************
 NAME:Msg_Spell
 FUNC:拼接消息串
 PARA:
 message_format_t:入队消息结构体
 char* SpellStr:拼接后到串
 Return:拼接串长度
 *************************************************************/
int Msg_Spell(void* Msg, u8* SpellStr) {
//	printf("Msg_Spell\n");
//	api_PrintfHex(Msg,sizeof(QUE_TDF_QUEUE_MSG));
	memcpy(SpellStr, Msg, 12); //前12字节包括类型、版本号、属性、时间
	u8 asclen[2] = { 0 };
	memcpy(asclen, Msg + 12, 2); //长度值需要换成十进制使用
	u8 len[4] = { 0 };
	api_HexToAsc(asclen, len, 2); //先转成文本格式
	int n = HexStr2Int((char*) len, 4); //十六进制串转成十进制

	u8 buf1[1520] = { 0 };
	u8 buf2[1520] = { 0 };
	memcpy(buf1, Msg + 14, n);
//printf("Message.data len:%d,data:",n);
//api_PrintfHex(buf1, n);
	ulong bclen = 0;
	lz77compress(buf1, n, buf2, &bclen);
//printf("bclen:%ld\n", bclen);
	int m = (bclen + 7) >> 3;
//printf("m=%d \n", m);
	sprintf((char*) len, "%04X", (unsigned int) bclen);
	api_AscToHex(len, asclen, 4);
//printf("asclen:");
//api_PrintfHex(asclen,2);
//api_PrintfHex(buf2, m);
	memcpy(SpellStr + 12, asclen, 2); //长度值拼入串
	memcpy(SpellStr + 14, buf2, m); //消息体拼入串

	int Msg_len = 14 + m;
//printf("\n Spellstr:");
//api_PrintfHex(SpellStr, Msg_len);
	return Msg_len;
}
