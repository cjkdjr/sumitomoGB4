#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <sys/sysinfo.h>

#include "alarm.h"
#include "api.h"
#include "iap_queue.h"
#include "gsm.h"
#include "TCPsocket.h"
#include "gpio.h"
#include "gps_4g.h"
#include "serial.h"
#include "sms.h"
#include "UDPclient.h"
#include "iap.h"
#include "sys_manage.h"
#include "general.h"
#include "Msg_task.h"

#define BSP_USB_POWER_PIN "143"
#define BSP_GSM_WAKEUP "36"
#define BSP_SBOARD_PWR_PIN "230"
#define BSP_SBOARD_CLK_PIN "229"
#define BSP_GSM_PWR_PIN "234"
#define BSP_GSM_2V75_PIN "235"

int gps_flag = 0;
int GSMFd;
int swdt_gsm;

uint8_t rcv_cnt[150] = { 0 };
uint8_t rcv_telephone[8] = { 0 };
int GSM_Sock_PPPFlg = 0;
int GSM_ATFaiCount = 0;
int GSM_TranFaiCount = 0;
int GSM_GpsFailCount = 0;
u16 GSM_restart_Count = 0;
unsigned char GSM_Csq_FLAG=0x00;/*0x01:out-groups;0x02:in groups;0x00:can not Obtain*/

typedef enum {
	control_off, control_on, control_sleep,
} gsm_control;

gsm_info_t gsm_info_cur;
gsm_state gsm_cur_state = state_off;
gsm_control gsm_cur_control = control_off;

/*
 * 函数功能：	获取当前CSQ值，数据更新至GSM模块全局变量
 * 参数：			发送AT+CSQ返回的内容ackStr，打印要执行的函数
 * 返回值：		成功返回0，失败返回-1
 * 举例：			AT+CSQ
 * 						+CSQ: 20,99
 * 						OK
 * */
int GetCSQ(char *ackStr, char *funcName) {
	int val;
	char *p;

	printf_gsm("%s\n", ackStr);
	if (strstr(ackStr, "OK")) {
		p = strstr(ackStr, "+CSQ:");
		if (p != NULL) {
			if (*(p + 7) == ',')
				val = (*(p + 6) - '0');
			else {
				val = (*(p + 6) - '0') * 10;
				val = val + (*(p + 7) - '0');
			}
			System.GsmCSQ = val;
			if ((val >= 5) && (val <= 31)) {
				GSM_Csq_FLAG = 0x10;
			}
			if(System.GsmCSQ<=5)
			{
				GSM_Csq_FLAG=0x01;  /*out-groups*/
				printf_gsm("SMS_CheckCSQ:CSQ is out-groups\n");
			}
			if(System.GsmCSQ==99)
			{
				GSM_Csq_FLAG=0x00; /*can not Obtain*/
				printf_gsm("SMS_CheckCSQ:CSQ can not Obtain\n");
			}
			return 0;
		} else {
			return -1;
		}
	} else {
		return -1;
	}

}

/*
 * 函数功能：	获取注册信息，数据更新至全局变量
 * 参数：			发送AT+CPSI返回的内容ackStr，打印要执行的函数名funcName
 * 返回值：		成功返回0，失败返回-1
 * 举例：			AT+CPSI?
 * 						+CPSI: GSM,Online,460-00,0x323a,30892,35 EGSM 900,-81,0,18-128
 * 						OK
 * 						+CPSI: LTE,Online,460-00,0x32C9,83928897,495,EUTRAN-BAND41,40936,5,5,-101,-1005,-714,14
 * 						OK
 * */
int GetCPSI(char *ackStr, char *funcName) {
	int index1, index2;
	int tac_cnt;
	char *p;
	char p_buf[10] = { 0 };
	u32 tmp_cid = 0;

	printf_gsm("%s\n", ackStr);
	if (strstr(ackStr, "OK")) {
		p = strstr(ackStr, "+CPSI:");
		if (p != NULL) {
			index1 = gps_getCommaIndex(3, (unsigned char *)p); //0x323A,50518018,89,EUTRAN-BAND39,38400,5,5,-128,-927,-600,10
			index2 = gps_getCommaIndex(4, (unsigned char *)p);

			if (index2 > index1) {
				//tac
				tac_cnt = index2 - index1 - 2 - 1;
				if (tac_cnt >= 4) {
					memset(p_buf, '\0', sizeof(p_buf));
					strncpy(p_buf, (const char *) p + index1 + 2, tac_cnt);
					gsm_info_cur.lac = strtol(p_buf, NULL, 16);
					System.GSM_AreaCode = gsm_info_cur.lac;

					//cid
					tmp_cid = atol((const char *) (p + index2));
//					gsm_info_cur.cid  = (tmp_cid<<8);
//					gsm_info_cur.cid |= (tmp_cid>>8);
					gsm_info_cur.cid = tmp_cid;
					System.GSM_CellID = gsm_info_cur.cid;
					printf_gsm(" System.GSM_CellID =%4x  \n",(unsigned int)System.GSM_CellID);
					return 0;
				}
			}
		}
	}
	return -1;
}

/*
 * 函数功能：	获取当前CREG值，数据更新至GSM模块全局变量
 * 参数：			发送AT+CREG返回的内容ackStr，打印要执行的函数名funcName
 * 返回值：		成功返回0，失败返回-1
 * 举例：			AT+CREG?
 * 						+CREG: 2,1,"323A","D3E8"
 * 						OK
 * */
int GetCREG(char *ackStr, char *funcName) {
	char *p_reg;

	printf_gsm("%s\n", ackStr);
	if (strstr(ackStr, "OK") != NULL) {
		p_reg = strstr(ackStr, "+CREG:");
		if (p_reg != NULL) {
			//creg status
			if (*(p_reg + 9) - '0')
				gsm_info_cur.status |= GSM_STATUS_CREG;
			else
				gsm_info_cur.status &= ~GSM_STATUS_CREG;
			return 0;
		}
	}
	return -1;
}

/*
 * 函数功能：    获取模块的IMEI号，将转换成16进制存到全局变量
 * 参数：			发送AT+CGSN返回的内容readBuff，打印要执行的函数
 * 返回值：		成功返回0，失败返回-1
 * 举例：			AT+CGSN
 * 						357224020936212
 * 						OK
 * history:2018-10-25 v1.1 添加接收长度判断，防止特殊情况线接收数据过长导致溢出
 * */
int GetIMEI(char *ackStr, char *funcName) {
	int imeiLen = 0;
	char *p_end = NULL;
	char *p = NULL;
	char *p_start = NULL;
	unsigned char IMEIStr[10] = {0};
	unsigned char tmpBuff[20] = {0};

	printf_gsm(" %s \n",ackStr);
	p_end = strstr((const char*)ackStr, "OK");
	if (NULL == p_end) {
		return -1;
	} else {
		for (p = ackStr; p < p_end; p++) {
			if ( *p >= 0x30 && *p < 0x3A ) {
				if(imeiLen == 0){
					p_start = p;
				}
				imeiLen++;
			}
		}
		if(imeiLen == 0) //如果未读到数据
		{
			return -1;
		}
		if(imeiLen > 16||imeiLen < 10)//数据长度异常
		{
	    	return 0;
		}
		memcpy(tmpBuff, p_start, imeiLen);
		memset(gsm_info_cur.imei_str, 0, 20);
		strncpy((char *)gsm_info_cur.imei_str, (char *)tmpBuff, imeiLen);
		printf("imeiLen = %d, imei_str = %s\n", imeiLen, gsm_info_cur.imei_str);
		if (imeiLen % 2 != 0) {
			imeiLen++;
			tmpBuff[imeiLen - 1] = 'F';
			tmpBuff[imeiLen] = '\0';
		}else {
			tmpBuff[imeiLen] = '\0';
		}
		gsm_info_cur.imei_len = imeiLen;
		api_HexToAsc(tmpBuff, IMEIStr, imeiLen);
		memcpy(&gsm_info_cur.imei, IMEIStr, 8);
		api_PrintfHex(IMEIStr, 8);
    	return 0;
	}
}

/*
 * 函数功能：    获取模块的IMSI号，将转换成16进制存到全局变量
 * 参数：			发送AT+CIMI返回的内容readBuff，打印要执行的函数
 * 返回值：		成功返回0，失败返回-1 返回1：sim卡被更换
 * 举例：			AT+CIMI
 * */
int GetIMSI(char *ackStr, char *funcName)
{
	char tmp[512];
	char *p_end = NULL;
	char *p_start = NULL;
	char *p = NULL;
	u8 imsi_len = 0;

	printf_gsm(" %s \n",ackStr);
	p_end = strstr((const char*)ackStr, "OK");
	if (NULL == p_end) {
		return -1;
	} else {
		gsm_info_cur.imsi_len= 0;
		for (p = ackStr; p < p_end; p++)
		{
			if ((*p>='0') && (*p<='9'))
			{
				if (imsi_len == 0)
				{
					p_start = p;
				}
				imsi_len++;
			}
		}
	}


	//
		memcpy(tmp, p_start, imsi_len);
		gsm_info_cur.imsi_len = imsi_len;
		memcpy(&gsm_info_cur.imsi, tmp, imsi_len);
    	return 0;
	}

/*
 * 函数功能：    获取当前CNSMOD值(注册网络模式)，数据更新至GSM模块全局变量
 * 参数：
 * 返回值：		成功返回0，失败返回-1
 * 举例：			AT+CNSMOD?
 *							+CNSMOD: 0,0
 * 						OK
 * */
int GetCNSMOD(char * ackStr, char *funcName) {
	char *p;

	printf_gsm(" %s \n",ackStr);
	if (strstr(ackStr, "OK")) {
		p = strstr(ackStr, "+CNSMOD:");
		if (p != NULL) {
			switch (ackStr[11]) {
				case 56:
					SYS_SET2.RESESTER_NET = 2;    //LTE 4G网络模式
					break;
				default:
					SYS_SET2.RESESTER_NET = 1;    //其他状态为2/3G模式
					break;
			}
			return 0;
		} else {
			return -1;
		}
	} else {
		return -1;
	}
}

/*
 * 函数功能：gsm模块上电
 * 参数：		void
 * 返回值：	成功返回0，失败返回-1
 * */
int PowerOn(void) {
	if (gpio_SetModeValue(BSP_SBOARD_PWR_PIN, "high") < 0) {
		return -1;
	}
	if (gpio_ProducePulse(BSP_SBOARD_CLK_PIN, NEGATIVE, 500) < 0) {
		return -1;
	}
	if (gpio_ProducePulse(BSP_GSM_PWR_PIN, POSITIVE, 500) < 0) {
		return -1;
	}
	if (gpio_SetModeValue(BSP_USB_POWER_PIN, "high") < 0) {
		return -1;
	}
	return 0;
}

/*
 * 函数功能：gsm模块断电
 * 参数：		void
 * 返回值：	成功返回0，失败返回-1
 * */
int gsm_PowerOff(void) {
	if (gpio_ProducePulse(BSP_GSM_PWR_PIN, POSITIVE, 2500) < 0) {
		return -1;
	}
	if (gpio_SetModeValue( BSP_USB_POWER_PIN, "low") < 0) {
		return -1;
	}
	if (gpio_SetModeValue( BSP_SBOARD_PWR_PIN, "low") < 0) {
		return -1;
	}
	if (gpio_ProducePulse(BSP_SBOARD_CLK_PIN, NEGATIVE, 500) < 0) {
		return -1;
	}
	return 0;
}

/*
 * 函数功能：	gsm模块休眠
 * 参数：			void
 * 返回值：		成功返回0，失败返回-1
 * */
int gsm_DevSleep(void) {
	if (gpio_SetModeValue(BSP_GSM_WAKEUP, "low") < 0) {
		return -1;
	}
	if (gpio_SetModeValue(BSP_USB_POWER_PIN, "low") < 0) {
		return -1;
	}
	return 0;
}

/*
 * 函数功能：打开GSM虚拟串口ttyUSB2
 * 参数：		void
 * 返回值：	成功返回0，失败返回-1
 * */
int OpenGSMSerial(void) {
	int cnt = 0;

	while(1) {
		if (0 == access("/dev/ttyUSB2", F_OK)) {
			break;
		}
		printf_gsm(" have no ttyUSB2\n");
		if (10 == cnt) {
			break;
		}
		sleep(1);
		cnt++;
	}
	sleep(2);
	if ((GSMFd = open("/dev/ttyUSB2", O_RDWR | O_NOCTTY)) < 0) {
		printf_gsm("open ttyUSB2 fail\n");
		return -1;
	}
	return 0;
}



/*
 * 函数功能: 发送AT指令，在超时时间内读取回复信息
 * 参数：	  串口的文件描述符fd，发送的AT命令cmdStr，传入到处理回复信息函数的参数checkStr，
 * 				  处理回复信息的回调函数handleAck，超时时间timeout
 * 返回值：  串口中读到了指定的字符串返回0，没有读到返回-1
 * */
int gsm_SendATCmd(int fd, char *cmdStr, char *checkStr, int (*handleAck)(char *, char *), int timeout) {
	int timeoutRet = -1, readRet = -1;
	unsigned char readBuff[100] = {0};

	printf_gsm("%s\n",cmdStr);

	pthread_mutex_lock(&ATCmdMutex);
	tcflush(fd, TCIFLUSH);
	if (write(fd, cmdStr, strlen(cmdStr)) < 0) {
		printf_gsm("write AT cmd fail\n");
		goto fail;
	}
	timeoutRet = api_TimeOut(fd, 1, timeout);
	if (timeoutRet < 0) {
		goto fail;
	}else if (0 == timeoutRet) {
		printf_gsm("read AT cmd timeout\n");
		goto fail;
	}else {
		usleep(100 * 1000);
		readRet = read(fd, readBuff, sizeof(readBuff));
		if (readRet <= 0) {
			perror("at");
			printf_gsm("read AT cmd fail\n");
			goto fail;
		}
		pthread_mutex_unlock(&ATCmdMutex);
		if (handleAck((char *)readBuff, checkStr) == 0) {
			return 0;
		} else{
			return -1;
		}
	}
fail:
	pthread_mutex_unlock(&ATCmdMutex);
	return -1;
}
/*
 * 函数功能: 拨号上网
 * 参数：	  void
 * 返回值：  成功返回0，失败返回-1
 * V1.1 修改APN可配置
 * */
int gsm_PPPoE(void) {
	char readBuf[1024] = { 0 };
	int readRet = 0;
	int retry = 5;

	while (retry--) {
		readRet = read(GSMFd, readBuf, 100);
		if (readRet > 0) {
			if (strstr(readBuf, "PB DONE")) {
				printf_gsm("PB DONE break!!\n");
				usleep(100 * 1000);
				break;
			}
		}
		sleep(1);
		printf_gsm("...\n");
	}

	sleep(1);
	for (retry = 0; retry < 5; retry++) {
#if Kebecol_APN
		if(strcmp((const char *)(Central_set.APN_IP_default),"CMIOT\0")==0 || strcmp((const char *)(Central_set.APN_IP_default),"cmiot\0")==0 )
		{
			if (0 == gsm_SendATCmd(GSMFd, "AT$QCRMCALL=1,1,,,,,\"CMIOT\",,,0\r", "OK",  api_FindStr, 5))
					break;
		}else 	if(strcmp((const char *)(Central_set.APN_IP_default),"CMNET\0")==0 || strcmp((const char *)(Central_set.APN_IP_default),"cmnet\0")==0 )
		{
			if (0 == gsm_SendATCmd(GSMFd, "AT$QCRMCALL=1,1,,,,,\"CMNET\",,,0\r", "OK",  api_FindStr, 5))
					break;
		}else 	if(strcmp((const char *)(Central_set.APN_IP_default),"3GNET\0")==0 || strcmp((const char *)(Central_set.APN_IP_default),"3gnet\0")==0 )
		{
			if (0 == gsm_SendATCmd(GSMFd, "AT$QCRMCALL=1,1,,,,,\"3GNET\",,,0\r", "OK",  api_FindStr, 5))
					break;
		}else{//若无匹配apn 默认为 CMIOT
			if (0 == gsm_SendATCmd(GSMFd, "AT$QCRMCALL=1,1,,,,,\"CMIOT\",,,0\r", "OK",  api_FindStr, 5))
					break;
			printf("default apn \n");
			}
#else
		if(strcmp((const char *)(SYS_SET2.SET_APN_DEFAULT),"CMIOT\0")==0 || strcmp((const char *)(SYS_SET2.SET_APN_DEFAULT),"cmiot\0")==0 )
		{
			if (0 == gsm_SendATCmd(GSMFd, "AT$QCRMCALL=1,1,,,,,\"CMIOT\",,,0\r", "OK",  api_FindStr, 5))
					break;
		}else 	if(strcmp((const char *)(SYS_SET2.SET_APN_DEFAULT),"CMNET\0")==0 || strcmp((const char *)(SYS_SET2.SET_APN_DEFAULT),"cmnet\0")==0 )
		{
			if (0 == gsm_SendATCmd(GSMFd, "AT$QCRMCALL=1,1,,,,,\"CMNET\",,,0\r", "OK",  api_FindStr, 5))
					break;
		}else 	if(strcmp((const char *)(SYS_SET2.SET_APN_DEFAULT),"3GNET\0")==0 || strcmp((const char *)(SYS_SET2.SET_APN_DEFAULT),"3gnet\0")==0 )
		{
			if (0 == gsm_SendATCmd(GSMFd, "AT$QCRMCALL=1,1,,,,,\"3GNET\",,,0\r", "OK",  api_FindStr, 5))
					break;
		}else{//若无匹配apn 默认为 CMIOT
			if (0 == gsm_SendATCmd(GSMFd, "AT$QCRMCALL=1,1,,,,,\"CMIOT\",,,0\r", "OK",  api_FindStr, 5))
					break;
			printf("default apn \n");
			}

#endif
		sleep(1);
	}
	if (5 == retry) {
		return -1;
	}
	system("udhcpc -i wwan0  -q");
	return 0;
}


///**----------------------------------------------------------
// * FUNCTION: ppp close
// * ----------------------------------------------------------*/
int ppp_disconnect() {
	int ret = -1;
	int i = 50;
	GSM_Sock_PPPFlg = 0;
	system("/etc/ppp/gprs-off.sh");
	while (i--) {
		sleep(1);
		if (access("/var/run/ppp0.pid", F_OK) == -1) {
			ret = 1;
			break;
		}
	}

	return ret;
}
///**----------------------------------------------------------
// * FUNCTION: START  PPP DIAL
// * ----------------------------------------------------------*/
int ppp_connect() {
	int ret = -1;
	int i = 10;
	int tmout = 120;
	if (GSM_Sock_PPPFlg == 0) {
		printf_gsm("GSM_PPPON:START PPP DIAL:%d \n", GSM_Sock_PPPFlg);


		//等待网络状态注册成功
		while (tmout--) {
			gsm_PthParam.sta = 1;
			if (-1 == gsm_SendATCmd(GSMFd, "AT+CREG?\r", "GetCREG", GetCREG, 5)) {
				GSM_ATFaiCount++;
				gsm_SendATCmd(GSMFd, "AT+CREG=2\r", "OK", api_FindStr, 5);
				sleep(1);
			} else{
				tmout = 0;
			}
		}

		if(strcmp((const char *)(SYS_SET2.SET_APN_DEFAULT),"CMIOT\0")==0 || strcmp((const char *)(SYS_SET2.SET_APN_DEFAULT),"cmiot\0")==0 ) {
			system("pppd call cmiot-gprs &");
		}else 	if(strcmp((const char *)(SYS_SET2.SET_APN_DEFAULT),"CMMTM\0")==0 || strcmp((const char *)(SYS_SET2.SET_APN_DEFAULT),"cmmtm\0")==0 )
		{
			system("pppd call cmnet-gprs &");
		}else{//若无匹配apn 默认为 CMNET
			system("pppd call hw-gprs &");
			printf_gsm("default apn \n");
			}

		while (i--) {
			sleep(1);
			gsm_PthParam.sta = 1;
			if (access("/var/run/ppp0.pid", F_OK) != -1) {
				printf_gsm("GSM_PPPON:PPP DIAL %d times OK !\n", (10 - i));
				GSM_Sock_PPPFlg = 1;
				ret = 1;
				break;
			} else {
				printf_gsm("GSM_PPPON:PPP DIAL %d times failed!\n", (10 - i));
			}
		}
	} else {
		printf_gsm("ppp_connect failed!\n");
		return -1;
	}
	sleep(2);
	printf_gsm("GSM_Sock_PPPFlg :%d\n", GSM_Sock_PPPFlg);
	return ret;
}
/*
 * 函数功能：    GSM模块初始化拨号上网
 * 参数：			虚拟串口ttyUSB2的配置参数，波特率baud，数据位dataBits，停止位stopBits，奇偶校验位parity
 * 返回值：		拨号上网成功返回0，失败返回-1
 * history: v1.1 by zjx 2018-10-27
 * 初始化完成后添加读短信
 * 出现apn、ip等错误导致的通信异常时，可通过短信设置恢复正常。
 * 相关参数清0：单一因素导致模块重启后，其他错误累计次数也要去清0 ，防止重复重启。
 * */
uint8_t InitGSM(uint32_t baud,  uint8_t dataBits,  uint8_t stopBits, uint8_t parity) {
	gsm_info_cur.status = 0;
	int index = 0;
	int retry = 0;
	char readBuf[1024] = { 0 };
	int readRet = 0;
	retry = 10;
	//相关参数清0
	 GSM_Sock_PPPFlg = 0;
	 GSM_ATFaiCount = 0;
	 GSM_TranFaiCount = 0;
	 GSM_GpsFailCount = 0;
	// 1.上电虚拟出GSM串口ttyUSB2
	if (PowerOn() < 0)
		return -1;
	// 2.打开虚拟串口ttyUSB2
	if (OpenGSMSerial() < 0)
		return -1;
	// 3.初始化串口
	if (serial_Init(GSMFd, baud, dataBits, stopBits, parity) < 0)
		return -1;
	// 4.发送AT指令，初始化配置
	if (-1 == gsm_SendATCmd(GSMFd, "AT\r",  "OK", api_FindStr, 5))
		return -1;
	if (-1 == gsm_SendATCmd(GSMFd, "ATE0\r", "OK", api_FindStr, 5))
		return -1;
	if (-1 == gsm_SendATCmd(GSMFd, "AT+IPR=115200\r", "OK", api_FindStr, 5))
		return -1;
	while (retry--) {
		readRet = read(GSMFd, readBuf, 100);
		if (readRet > 0) {
			if (strstr(readBuf, "PB DONE")) {
				printf_gsm("PB DONE break!!\n");
				usleep(100 * 1000);
				break;
			}
		}
		sleep(1);
		printf_gsm("...\n");
	}
	if (-1 == gsm_SendATCmd(GSMFd, "AT+CGSN\r",  "GetIMEI", GetIMEI, 5))
		return -1;
	if (-1 == gsm_SendATCmd(GSMFd, "AT+CMGF=0\r",  "OK", api_FindStr, 5))
		GSM_ATFaiCount++;
	if (-1 == gsm_SendATCmd(GSMFd, "AT+CREG=2\r", "OK", api_FindStr, 5))
		GSM_ATFaiCount++;
	if (-1 == gsm_SendATCmd(GSMFd, "AT+CNSMOD?\r", "GetCNSMOD", GetCNSMOD, 5))
		GSM_ATFaiCount++;
	if(-1 == gsm_SendATCmd(GSMFd, "AT+CNMP=2\r", "OK", api_FindStr, 5))
		GSM_ATFaiCount++;
	gsm_CheckSim();


	//读短信
		index = SMS_ListRead(GSMFd, 4);
		if (index != -1) {
			sleep(1);
			SMS_MessageDelete(GSMFd, index);
			sleep(1);
		}

	gsm_info_cur.status |= GSM_STATUS_INT;
#if PPP_DIAL
	if (ppp_connect() < 0){
		if(AlarmIsBitH(AlarmIdSIMFail))	{
			GSM_Sock_PPPFlg = 0;
		}else{
			return -1;
		}
	}else{
		GSM_Sock_PPPFlg = 1;
	}

#else
	// 5.发送AT指令上网
	if (-1 == gsm_PPPoE())
	{
		if(AlarmIsBitH(AlarmIdSIMFail))
		{
			GSM_Sock_PPPFlg = 0;
		}else{
			return -1;
		}
	}else{
		GSM_Sock_PPPFlg = 1;
	}

#endif
	// 6.置标志位，GPS和TCP传输需要联网
	return 0;
}

/********************************
 * SIM 卡检测逻辑
 *功能：sim卡检测，设置报警及报警解除逻辑
 *history by zhangjx 20180910
 *返回值发生变化
 ********************************/
u8 gsm_CountCPIN;
u8 gsm_CountIMSI;
int gsm_CheckSim(void) {
	int retCPIN = 0;
	int retIMSI = 0;

	if (gsm_SendATCmd(GSMFd, "AT+CPIN?\r", "READY", api_FindStr, 5) == -1) {
		GSM_ATFaiCount++;
		gsm_CountCPIN++;
	} else {
		retCPIN = 1;
		gsm_CountCPIN = 0;
		GSM_ATFaiCount = 0;
		AlarmClrBit(AlarmIdGSMCom);
	}

	if ( gsm_SendATCmd(GSMFd, "AT+CIMI\r", "OK", GetIMSI, 5)== -1) {
		GSM_ATFaiCount++;
		gsm_CountIMSI++;
	}else {
		retIMSI = 1;
		gsm_CountIMSI = 0;
		GSM_ATFaiCount = 0;
		AlarmClrBit(AlarmIdGSMCom);
	}
	if(gsm_CountIMSI > 0 ||gsm_CountCPIN > 0)
	{
		if((gsm_CountIMSI >= 3*3) ||(gsm_CountCPIN >= 3*3))
		{
			//如果存在gsm模块报警不再设置sim卡报警状态
			if(AlarmIsBitH(AlarmIdGSMCom))
			{
				return 1;
			}
			AlarmSetBit(AlarmIdSIMFail);
		}
	}
	if ((retCPIN == 1) && (retIMSI == 1)) {
		// sim卡异常报警解除
		AlarmClrBit(AlarmIdSIMFail);
	}
	return 0;
}
/***************************************************
 * name :gsm_check
 * func:Module cycle detection for 1:csq;2:creg;3:gsm_ant;4cmgl(rev msg)
 *  //1.at failed 20 times set  GSM module alarm.
 *
 ****************************************************/
int gsm_check() {

	if (GSM_ATFaiCount > 20) {
		printf_gsm("GSM_Check:AT ERROR has 20 TIMES,GSM Restart!!!\n");
		//GSM模块报警
		AlarmSetBit(AlarmIdGSMCom);
		GSM_ATFaiCount = 0;
		gsm_cur_state = state_off;
	}
	if (GSM_TranFaiCount > 5) {
		printf_gsm("GSM_Check:Trans ERROR 2min,GSM Restart!!!\n");
		GSM_TranFaiCount = 0;
		gsm_cur_state = state_off;
	}
	if (GSM_GpsFailCount > 10) {
		printf_gsm("GSM_Check:GPS no data 5min,GPS nead GSM Restart!!!\n");
		GSM_GpsFailCount = 0;
		gsm_cur_state = state_off;
	}
	printf_gsm("GSM_ATFaiCount=%d\n", GSM_ATFaiCount);
	printf_gsm("GSM_TranFaiCount=%d\n", GSM_TranFaiCount);

	if (-1 == gsm_SendATCmd(GSMFd, "AT+CSQ\r", "GetCSQ", GetCSQ, 5)) {
		GSM_ATFaiCount++;
	} else {
		GSM_ATFaiCount = 0;
		AlarmClrBit(AlarmIdGSMCom);
	}
	if (-1 == gsm_SendATCmd(GSMFd, "AT+CPSI?\r", "GetCPSI", GetCPSI, 5)) {
			GSM_ATFaiCount++;
		} else {
			GSM_ATFaiCount = 0;
			AlarmClrBit(AlarmIdGSMCom);
		}
	if (-1 == gsm_SendATCmd(GSMFd, "AT+CREG?\r", "GetCREG", GetCREG, 5)) {
		GSM_ATFaiCount++;
		gsm_SendATCmd(GSMFd, "AT+CREG=2\r", "OK", api_FindStr, 5);
	} else {
		GSM_ATFaiCount = 0;
		AlarmClrBit(AlarmIdGSMCom);
	}
	if (-1 == gsm_SendATCmd(GSMFd, "AT+CGSN\r",  "GetIMEI", GetIMEI, 5)){
		GSM_ATFaiCount++;
	} else {
		GSM_ATFaiCount = 0;
		AlarmClrBit(AlarmIdGSMCom);
	}
	gsm_CheckSim();
	return 0;
}
/***************************************************
 * gsm_cycle_check
 ***************************************************/
void GSM_CycleCheck() {
	int index;
	gsm_check();
	sleep(1);
	//读短信
	index = SMS_ListRead(GSMFd, 4);
	if (index != -1) {
		sleep(1);
		SMS_MessageDelete(GSMFd, index);
		sleep(1);
	}
}

void CloseINT(void) {
	struct ifreq ifr;
	int Csockfd;
	bzero(&ifr, sizeof(struct ifreq));
	Csockfd = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(ifr.ifr_name, "wwan0");
	ioctl(Csockfd, SIOCGIFFLAGS, &ifr);
	ifr.ifr_flags &= ~IFF_UP;
	ioctl(Csockfd, SIOCSIFFLAGS, &ifr);
	usleep(100 * 1000);
	close(Csockfd);
}
/*********************************************************************************************************
 * 作者：孙广田
 * 功能：GSM模块关闭函数
 * 日期：15.05.05
 ********************************************************************************************************/
void CloseGSM(void) {
	struct sysinfo s_info;
	/*first close all virtual ports before closing the physical port.*/
	if (GSM_Sock_PPPFlg == 1)
		CloseINT();
	GSM_Sock_PPPFlg = 0;
	if (GSMFd) {
		int ret = close(GSMFd);
		if (ret == -1) {
			printf_gsm("close error!\n");
			close(GSMFd);
		}
	}

	gsm_PowerOff();

	sysinfo(&s_info);
	printf_gsm("gsm power off[%ld]\n", s_info.uptime);
}

/*********************************************************************************************************
 * 作者：孙广田
 * 功能：GSM模块休眠函数
 * 日期：15.07.18
 ********************************************************************************************************/
int SleepGSM(void) {
	system("ifconfig wwan0 down");
	//关闭基站信息
	if (gsm_SendATCmd(GSMFd, "AT+CREG=0\r", "OK", api_FindStr, 5))
		return 1;
	if (gsm_SendATCmd(GSMFd, "AT+CSCLK=1\r", "OK", api_FindStr, 5))
		return 1;
	gsm_DevSleep();
	if (GSMFd)
		close(GSMFd);

	/* experience showed that some modems need some time before closing
	 * the physical port so a delay may be needed here in some case */
	sleep(1);
	return 0;
}

void GSM_init(void) {
	gsm_cur_state = state_off;
}
int checkNet() {
	int ret = -1;
	int i = 10;
	char buf[1024];
	system("ping 222.223.231.137 -w 2 1> /tmp/check_log &");
	usleep(1000 * 1000);
	FILE *fp = fopen("/tmp/check_log", "r");
	while (!feof(fp)) {
		if (i-- > 0) {
			memset(buf, 0, sizeof(buf));
			fread(buf, sizeof(buf), 1, fp);
			if (strstr(buf, "64 bytes from 222.223.231.137") != NULL) {
				ret = 1;
				break;
			} else {
				ret = 0;
			}
		}
		usleep(100 * 1000);
	}
	fclose(fp);
	return ret;
}
/**********************************
 * 判断网络是否需要重连
 *
 * hidtory:v1.1 2018-10-25 by zjx
 * 网络异常时，如果重连失败，首先重新拨号，如果拨号也不成功才重启。
 ***********************************/

void socket_check(void)
{
	int ret = 0;
	if (Socket_Flag == 1) {	               //循环检测socket是否需要重新链接
		printf_gsm("line:%d,socket_flag = 1 socket restart!\n", __LINE__);
			ret = TCPSocketRestart();
			ret = -1;  //test
			if (ret < 0) {
				if (checkNet() == 1) {
						gsm_cur_state = state_run;	               //重连TCP
				} else {
				#if PPP_DIAL
					ppp_disconnect();
					sleep(1);
					if(ppp_connect < 0)         //重新拨号
					{
						gsm_cur_state = state_off;   //重启GSM模块
					}
					#else

									system("ifconfig wwan0 down");
									gsm_SendATCmd(GSMFd, "AT$QCRMCALL=0,1\r", "OK",  api_FindStr, 5);
									if (-1 == gsm_PPPoE())				//	重连失败重新拨号
									{
										GSM_Sock_PPPFlg = 0;
										gsm_cur_state = state_off;
									}
					#endif
									printf("TCP restart failed!\n");
				}
			} else{

				Socket_Flag = 0;
			}
		}
}


void GSMpoll(void) {
	int ret = 0;
	int rTcp = 0;
	switch (gsm_cur_state)
	{
	case state_on: {
		if (InitGSM(115200, 8, 1, 'N') == 0) {
			sleep(1);
			if (GSM_Sock_PPPFlg == 1){
				rTcp = TCP_Connect();
				if (rTcp < 0) {
					printf_gsm("rtcp<0!!!!!!\n");
					gsm_cur_state = state_off;   //重启GSM模块
				} else
				{
					gps_flag = 1;
					gsm_cur_state = state_run;
				}
			} else {
				gsm_cur_state = state_off;
				printf_gsm("Dial failed!\n");
			}
		} else {
			gsm_cur_state = state_off;	               //开启失败关闭GSM模块
			printf_gsm("OpenGSM error,ready to open again !\n");
		}
		break;
	}
	case state_run: {
		socket_check();
		if (IAP.Update_Enable != 0x01)    //非升级状态                                    /*if at,at check*/
		{
			GSM_CycleCheck();
			if (main_WorkSta == WORK_GSM_OFF) {
				gsm_cur_state = state_sleep;
			}
		}
		else    //升级状态    udp 通信
		{    // 信息出队列
			struct QueueIAP MessageSend;
			ret = QueueOut(&IAPQueue, &MessageSend);
			if (QueueOperateOk == ret) {
				GSM_SendGPRS(&MessageSend);
			}
		}
		break;
	}
	case state_sleep: {
		TCPSocketClose();
		//模块当前状态，防止重复操作
		if (gsm_info_cur.status & GSM_STATUS_SLEEP) {
			//判断模块是否需要重启
			if (gsm_cur_control == control_on) {
				printf_gsm("gsm restart!!!\n");
				gsm_cur_state = state_off;
			}
		} else {
			GSM_UdpClose();
			ppp_disconnect();

			gps_PthParam.flag = FALSE;
			sleep(1);
			gps_poweroff();
			printf_gsm("gsmsleep\n");
			SleepGSM();
			gsm_info_cur.status |= GSM_STATUS_SLEEP;
		}
		GSMSleep();
		if (IAP_Manage_State == IAP_Update_END || IAP_Update_END == update_iap) {
			system_state = sys_state_save;
		} else {
			system_state = sys_state_stop;
		}
		sleep(1);
		break;
	}
	case state_off: {
		if (main_WorkSta == WORK_GSM_OFF) {
			gsm_cur_state = state_sleep;
			break;
		}
		//模块当前状态，防止重复操作
		if (gsm_info_cur.status & GSM_STATUS_PWR) {
			//关闭socket线程，回收资源
			if (client_sockfd > 0) {
				printf_gsm("line:%d,state_off socket close!\n", __LINE__);
				TCPSocketClose();
			}
			ppp_disconnect();
			gps_flag = 0;

			usleep(100*1000);
			CloseGSM();
			gsm_info_cur.status &= ~GSM_STATUS_PWR;
		}

		//判断模块是否需要重启
		if (gsm_cur_control == control_on) {
			printf_gsm("gsm restart!!!\n");
			gsm_cur_state = state_on;
			System.DaySummary.GSMOnCount++;
		}
		break;
	}

	default:
		gsm_cur_state = state_run;
		break;
	}
}

void GSMEnable(void) {
	gsm_cur_state = state_off;
	gsm_cur_control = control_on;
}

void GSMSleep(void) {
	gsm_cur_control = control_sleep;
}

void GSMDisable(void) {
	gsm_cur_control = control_off;
}

void *gsm_Pthread(void *argv) {
	printf("############## %s start ##############\n", __FUNCTION__);
	gsm_PthParam.sta = 1;
	GSMEnable(); //使能GSM模块
	gsm_PthParam.flag = TRUE;
	gpio_SetModeValue("196", "low");
	while (gsm_PthParam.flag) {
		gsm_PthParam.sta = 1;
		GSMpoll();
		usleep(1000 * 1000);
	}
	return NULL;
}
