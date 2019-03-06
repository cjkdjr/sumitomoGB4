/*
 * UDPclient.c
 *
 *  Created on: 2018年1月10日
 *      Author: tykj
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "UDPclient.h"
#include "general.h"
#include "api.h"
#include "gsm.h"
#include "iap.h"
#include "iap_queue.h"
#include "sms.h"
#include "Msg_task.h"

int GSM_PPPLen = 0;
#define PPP_IP_MAX_LEN 22
unsigned char PPP_IpLinked[7];
struct sockaddr_in UdpServer_addr;
int Gsm_UdpFd = -1;

/*-----------------------------------------------------
 Name    :GSM_UdpConnect()
 Funciton:connect socket
 Input   :ip address
 Output  :none
 Author  :lmm-2014/02/13
 Modify  :[<name>-<data>]
 ------------------------------------------------------*/
int GSM_UdpConnect()
{
	unsigned int port = 0;
	int fd = -1;
	char ipBuff[PPP_IP_MAX_LEN] =
	{ 0 };
	struct timeval timeout =
	{ 20, 0 };

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		if (debug_value & 0x40) {
			perror("GSM:socket");
		}
	}

	int ret = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeout, sizeof(struct timeval));
	if (ret != 0){
		if (debug_value & 0x40) {
			perror("GSM:timeout");
		}
	}

	memcpy(PPP_IpLinked, SYS_SET2.SET_IP_IAP, PPP_IP_MAX_LEN);
	printf_iap("PPP connect_socket link IAP Default IP: ");
	if (debug_value & 0x40) {
		api_PrintfHex(PPP_IpLinked, PPP_IP_MAX_LEN);
	}

//	printf_gsm("\n");

	memset(ipBuff, 0, PPP_IP_MAX_LEN);
	sprintf(ipBuff, "%d.%d.%d.%d", PPP_IpLinked[1], PPP_IpLinked[2], PPP_IpLinked[3], PPP_IpLinked[4]);
	port = PPP_IpLinked[5];
	port <<= 8;
	port |= PPP_IpLinked[6];

	printf_iap("GSM_UdpConnect:IP=%s\n", ipBuff);
	printf_iap("GSM_UdpConnect:PORT=%d\n", port);
	memset(&UdpServer_addr, 0, sizeof(UdpServer_addr));
	UdpServer_addr.sin_family = AF_INET;
	UdpServer_addr.sin_addr.s_addr = inet_addr(ipBuff);
	UdpServer_addr.sin_port = htons(port);

	GSM_PPPLen = sizeof(struct sockaddr_in);
	printf_iap("GSM_UdpConnect:ok\n");
	return fd;
}

/*-----------------------------------------------------
 Name    :GSM_SocketTrans()
 udp链接。发送。接收
 ------------------------------------------------------*/
int endflag = 0;
struct timeval start, end, start1;
int timeuse;
int GSM_SocketTrans(unsigned char* dataMsg, int len)
{
	int sendNum;
	int recvNum;
	unsigned char recedatabuf[1462];

	if (GSM_Sock_PPPFlg <= 0)
	{
		return -2;
	}
	if (Gsm_UdpFd <= 0)
	{
		Gsm_UdpFd = GSM_UdpConnect();
	}
	gettimeofday(&start, NULL);
	if (endflag == 1)
	{
		gettimeofday(&start1, NULL);
		timeuse = 1000000 * (start1.tv_sec - end.tv_sec) + start1.tv_usec - end.tv_usec;
		printf_iap("GSM_SocketTrans:[RecentReceive->ThisSend]-TimeUsed: [%d us]\n", timeuse);
		endflag = 0;
	}
	sendNum = sendto(Gsm_UdpFd, dataMsg, len, 0, (struct sockaddr *) &UdpServer_addr, (socklen_t) GSM_PPPLen);
	if (sendNum <= 0)
	{
		if (debug_value & 0x40) {
			perror("GSM_Sendto");
		}
		printf_iap("GSM_SocketTrans:Send Fail\n");
		return -1;
	} else
	{
		printf_iap("GSM_SocketTrans-Send:\n ");
		if (debug_value & 0x40) {
			api_PrintfHex(dataMsg, sendNum);
		}
//		printf_gsm("\n");
		memset(recedatabuf, 0, sizeof(recedatabuf));
		recvNum = recvfrom(Gsm_UdpFd, (char *) recedatabuf, sizeof(recedatabuf), 0, (struct sockaddr *) &UdpServer_addr, (socklen_t *) &GSM_PPPLen);
		if (recvNum <= 0)
		{

			printf_iap("GPSR send ok no recv in 10s,once more time to send\n");
			return -1;
		} else
		{
			gettimeofday(&end, NULL);
			timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
			endflag = 1;
			printf_iap("GSM_SocketTrans:[Send->Receive]-TimeUsed: [%d us]\n", timeuse);
			if (IAP.Update_Enable == 0x01)
			{
				QueueDestroy(&IAPQueue);
			}
			printf_iap("GSM_SocketTrans-Recv:\n ");
			if (debug_value & 0x40) {
				api_PrintfHex(recedatabuf, recvNum);
			}
//			printf_gsm("\n");
			GSM_RevQueHandle(recedatabuf, recvNum);
			return 0;
		}

	}

}
int GSM_UdpClose()
{
	if (Gsm_UdpFd > 0)
	{
		shutdown(Gsm_UdpFd, SHUT_RDWR);
		close(Gsm_UdpFd);
		Gsm_UdpFd = -1;
		printf_iap("GSM_UdpClose:finished\n");
		return 1;
	} else
		return 0;
}
