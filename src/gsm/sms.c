/*
 * sms.c
 *
 *  Created on: 2018年1月10日
 *      Author: tykj
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "sms.h"
#include "general.h"
#include "TCPsocket.h"
#include "pdu.h"
#include "api.h"
#include "iap.h"

/*-----------------------------------------------------
 Name    :SMS_MessageRead()
 Funciton:send "AT+CMGL=<index>",if receive "ok",
 Input   :1-fd:
 2-index:read message index
 Output  :SMS_Fd
 return	: index 0-no sms, N-first sms index number
 Author  :lmm-2014/02/11
 Modify  :[<name>-<data>]
 ------------------------------------------------------*/
const char CenterSendNum[20] = "10657509110000";
const char CenterSIM1Number[] = "8613400214752"; // bak sim1 of ty
const char CenterSIM2Number[] = "8613722899418"; // bak sim2 of ty
const char GSM_M2M_CENTER1[] = "1064899031102"; // m2m center of ty cmcc.heb
const char GSM_M3M_CENTER1[] = "106550010646"; //liantong

int SMS_ListRead(int fd, int state) {
//	unsigned char buffRecv[1024*3];
//	unsigned char buffSend[512];
	u8 SMSsetData[1500] = { 0 };

	int ret;
	char buffSend[512];
	char buffRecv[1024 * 3];
	int index = -1, j = 0, len = 0, num = 0;
	int overtimes = 0;
	int readbuftimes = 0;
	int readbufret = 0;
	int iRetry;
	char *p;
	RecMessage RecData = { 0 };
	PDU_PARAM pMsg;
	memset(&pMsg, 0, sizeof(pMsg));

	sprintf(buffSend, "AT+CMGL=%d\r", state);
	write(fd, buffSend, strlen(buffSend));
	printf_gsm("AT send: %s\n", buffSend);

	for (iRetry = 0; iRetry < 10; iRetry++) {
		memset(buffRecv, '\0', sizeof(buffRecv));
		usleep(300000);

		ret = read(fd, buffRecv, sizeof(buffRecv));
		if (ret > 0) {
			printf_gsm("SMS_ListRead:buffrecv=%s\n", buffRecv);
			p = buffRecv;
			if ((p = strstr(p, "+CMGL:")) != NULL) {
				index = atoi((const char *) p + strlen("+CMGL:"));
				p = strstr(p, "\r\n");

				if (p != NULL) {
					p += 2;
					len = PDU_DecodeMessage(p, &pMsg);
					printf_gsm("phonenum =%s\n", pMsg.TPA);
					printf_gsm("SMS_ListRead:Judge this message \n");
					if ((0 == memcmp(pMsg.TPA, CenterSendNum, strlen(CenterSendNum))) || (0 == memcmp(pMsg.TPA, CenterSIM1Number, strlen(CenterSIM1Number))) || (0 == memcmp(pMsg.TPA, CenterSIM2Number, strlen(CenterSIM2Number))) || (0 == memcmp(pMsg.TPA, GSM_M3M_CENTER1, strlen(GSM_M3M_CENTER1)))
					                || (0 == memcmp(pMsg.TPA, GSM_M2M_CENTER1, strlen(GSM_M2M_CENTER1)))) {

//                        System.DaySummary.SMSRecvCount++;//lmm
						printf_gsm("SMS_ListRead:Receive effcient message from TY-Center\n");
						printf_gsm("messagelen=%d\n", len);
						printf_gsm("phmessage=");
						if (debug_value & 0x01) {
							api_PrintfHex((unsigned char *) pMsg.TP_UD, len);
						}
//                        printf_gsm("\n");
						num = 0;
						if (pMsg.TP_UD[0] == 0X7E) { //中心设置消息，做分流处理
							uint16_t nlen = 0;    //转义后串的长度
							nlen = Msg_DeConvert(SMSsetData, (u8*) pMsg.TP_UD, len);    //数据转义还原
							servMsgHandle(SMSsetData, nlen);      //数据处理
						} else {      //不是7E开头的默认事3000升级逻辑
							RecData.MsgLen = pMsg.TP_UD[num++];
							RecData.MsgLen = (RecData.MsgLen << 8) | (pMsg.TP_UD[num++]);
							printf_gsm("SMS_ListRead:RecData.length=%02x\n", RecData.MsgLen);
#if 0      /*message check */
							DataCheck = pMsg.TP_UD[len-2];/*1byte-end*/
							if(api_CheckSum(&pMsg.TP_UD[2],RecData.length+2) != DataCheck)
							{
								return 0;
							}
#endif
							RecData.MsgType = pMsg.TP_UD[num++];
							memcpy(&RecData.SerialNum, pMsg.TP_UD + num, 3);
							num += 3;
							for (j = 0; j < len - num; j++) {
								RecData.data[j] = pMsg.TP_UD[j + num];
							}
							Receive_Message_Process(&RecData);
							printf_gsm("SMS_ListRead:clear module buffer\n");

							// waiting uart rx null & delay 3seconds
							readbuftimes = 0;
							overtimes = 0;
							while ((readbuftimes < 3) && (overtimes < 60)) {
								sleep(1);
								readbuftimes++;
								overtimes++;
								readbufret = read(fd, buffRecv, sizeof(buffRecv));
								if (readbufret > 0) {
									readbuftimes = 0;
									printf_gsm("gsm sms list outing1\n");
								}
							}
							return (index);
						}
					} else {
						System.DaySummary.UselessNum++; // wy!104302.3 接收删除非法短信条数
						printf_gsm("SMS_ListRead:This is spam message \n");

						// waiting uart rx null & delay 3seconds
						readbuftimes = 0;
						overtimes = 0;
						while ((readbuftimes < 3) && (overtimes < 60)) {
							sleep(1);
							readbuftimes++;
							overtimes++;
							readbufret = read(fd, buffRecv, sizeof(buffRecv));
							if (readbufret > 0) {
								readbuftimes = 0;
								printf_gsm("gsm sms list outing2\n");
							}
						}
						return (index);
					}
				}
			}
		}
	}

	return (index);
}

/*-----------------------------------------------------
 Name	:GSM_RevQueHandle()
 Funciton  :after receive data from socket,handle data to queue

 Input	:
 Output	:none
 Author	:lmm-2014/02/13
 Modify	:[<name>-<data>]

 ------------------------------------------------------*/
int GSM_RevQueHandle(unsigned char *data, int length) {
	int index;
	RecMessage RecData = { 0 };
//	unsigned char DataCheck=0;

	if (length > 0) {
		index = 0;
		RecData.MsgLen = data[index++];
		RecData.MsgLen = (RecData.MsgLen << 8) | data[index++];
		RecData.MsgLen = RecData.MsgLen - 4;/*[2bytes-length]+[1byte-checksum]+[1byte-end]*/
#if 0
		DataCheck = data[length-2];/*1byte-end*/
		if(api_CheckSum(data+2,RecData.length) != DataCheck)
		{
			return 0;
		}
#endif
		RecData.MsgType = data[index++];
		memcpy(&RecData.SerialNum, data + index, 3);
		index += 3;
		memcpy(RecData.data, data + index, RecData.MsgLen);
		Receive_Message_Process(&RecData);
	}
	return 0;
}

void Receive_Message_Process(RecMessage *RecM) {
	unsigned int MessageType = 0;

	MessageType = RecM->MsgType;
	printf_gsm("<%s> : Handle [%x] message\n", __FUNCTION__, MessageType);

	switch (MessageType)
	{

	case 0x63:
		IAP_63_Parse(RecM);
		break;
	case 0x65:
		IAP_65_Parse(RecM);
		break;
	case 0x67:
		IAP_67_Parse(RecM);
		break;
	case 0x69:
		IAP_69_Parse(RecM);
		break;
	case 0x71:
		IAP_71_Parse(RecM);
		break;
	default:
		printf_gsm("<%s> : Err id type [%d]\n", __FUNCTION__, MessageType)
		;
		break;
	}
}

/*-----------------------------------------------------
 Name    :SMS_MessageDelete()
 Funciton:delete all message that have read.
 Input   :1-<fd>:GSM_Fd
 2-<index>:Integer type;value in range of location nambers
 supported by the associated memory
 Output  :
 Author  :lmm-2014/02/11
 Modify  :[<name>-<data>]
 ------------------------------------------------------*/
int SMS_MessageDelete(int fd, int index) {
//	unsigned char buffSend[512];
//	unsigned char buffRecv[512];
	char buffSend[512];
	char buffRecv[512];
	int i, j, ret;
	char *p;

	for (j = 0; j < 2; j++) //wy!104303.6
	                {
//    	write(fd, buffSend, strlen(buffSend));
		sprintf(buffSend, "AT+CMGD=%d\r", index);
		write(fd, buffSend, strlen(buffSend));
		printf_gsm("AT send: %s\n", buffSend);
		memset(buffRecv, '\0', sizeof(buffRecv));
		for (i = 0; i < 30; i++)   // wy!104303.6
		                {
			usleep(1000000);
			if ((ret = read(fd, buffRecv, sizeof(buffRecv))) > 0) {
				printf_gsm("AT recv: %s\n", buffRecv);
				p = strstr(buffRecv, "OK\r\n");
				if (p != 0) {
					printf_gsm("SMS_MessageDelete: <%d> message ok\n", index);
					return (1);
				} else {
					printf_gsm("SMS_MessageDelete recv_no_ok: ");
					if (debug_value & 0x01) {
						api_PrintfHex((u8 *) buffRecv, ret);
					}
//                    printf_gsm("\n");
				}
			}
		}
		printf_gsm("SMS_MessageDelete: RECEIVE AT TIME OUT\n");
	}
	return 0;
}

