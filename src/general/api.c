#include <ctype.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>

#include "api.h"
#include "general.h"
#include "Time.h"

/*********************************************
 * NAME	:GetNowTime 获取当前时间
 * FUNC		:GPS定位有效时以GPS时间为准；定位无效获取rtc时间
 *
 *********************************************/
unsigned int GetNowTime(DateTime *t)  //rtc+gps
{
	SystemTimeRead(t);
	return 0;
}

void api_PrintfHex(u8 *data, int len) {
	int i;

	for (i = 0; i < len; i++) {
		printf("%02X", data[i]);
	}
	printf("\n");
}

u32 Sys_Start_Time = 0;
void api_SetSystemTime(u8 *time) {
	time_t start = 0;
	time_t trun = 0;
	DateTime nowRtc;
	struct tm nowtime;
	time_t t = 0;

	nowtime.tm_year = time[0] + 100;
	nowtime.tm_mon = time[1] - 1;
	nowtime.tm_mday = time[2];
	nowtime.tm_hour = time[3];
	nowtime.tm_min = time[4];
	nowtime.tm_sec = time[5];
	t = mktime(&nowtime);
	stime(&t);
	start = SystemTimeRead(&nowRtc);
	trun = api_GetSysSecs();
	Sys_Start_Time = start - trun;
}
void api_SetRtcTime(u8 *time) {
	RTCTimeWrite(time);
}

time_t api_GetSysSecs(void) {
	struct timespec timex;

	clock_gettime(CLOCK_MONOTONIC, &timex);

	return (timex.tv_sec);
}
time_t api_DiffSysSecs(time_t secLast) {
	return (api_GetSysSecs() - secLast);
}
/**********************************************
 * 函数名称：api_GetSysmSecs（）
 * 作用：从系统启动这一刻起开始计时，单位毫秒。
 * 原型：clock_gettime  提供纳秒极精确度
 * 参数：	CLOCK_REALTIME:系统实时时间,随系统实时时间改变而改变,即从UTC1970-1-1 0:0:0开始计时,
 * 													中间时刻如果系统时间被用户该成其他,则对应的时间相应改变
 * 			*	CLOCK_MONOTONIC:从系统启动这一刻起开始计时,不受系统时间被用户改变的影响
 * 				CLOCK_PROCESS_CPUTIME_ID:本进程到当前代码系统CPU花费的时间
 *    				CLOCK_THREAD_CPUTIME_ID:本线程到当前代码系统CPU花费的时间
 *    当前指示灯控制逻辑，采用该函数计时
 ********************************************/
time_t api_GetSysmSecs(void) {

	struct timespec timex;
	unsigned long time_mSecs = 0;

	clock_gettime(CLOCK_MONOTONIC, &timex);
	time_mSecs = (1000 * timex.tv_sec + timex.tv_nsec / 1000000);
	//	 printf("CLOCK_MONOTONIC: %d, %d time_mSecs=%d\n", time1.tv_sec, time1.tv_nsec,time_mSecs);
	return (time_mSecs);
}

/*
 *��������ת��������
 */
//time_t mktime(struct tm * timeptr);
/*
 get time  ms
 */
time_t api_GetSysMSecs(void) {
	struct timeval time;
	unsigned long time_msec = 0;
	gettimeofday(&time, NULL);
	time_msec = (1000000 * time.tv_sec + time.tv_usec) / 1000;
	return time_msec;
}
time_t api_DiffSysMSecs(time_t msecLast) {
	return (api_GetSysmSecs() - msecLast);
}

/*
 * 功能描述：在设置的超时时间内检测指定文件描述符是否可读可写，若可读写则立马返回，否则到超时时间后再返回
 * 参数：			要检测的文件描述符fd，要检测是否可读可写的标记flag（检测可读flag = 1，可写flag = 2），超时时间timeOutSec（单位s）
 * 返回值：		函数出错返回-1，超时还不可以读写返回0，文件描述符可读返回1，可写返回2
 * */
int api_TimeOut(int fd, int flag, int timeOutSec) {
	fd_set fdSet;
	int selectRet;

	FD_ZERO(&fdSet);
	FD_SET(fd, &fdSet);
	struct timeval waitTime;
	waitTime.tv_sec = timeOutSec;
	waitTime.tv_usec = 0;

	if (1 == flag) {
		selectRet = select(fd + 1, &fdSet, NULL, NULL, &waitTime);
	} else if (2 == flag) {
		selectRet = select(fd + 1, NULL, &fdSet, NULL, &waitTime);
	} else {
		printf("api_TimeOut flag is wrong!\n");
		return -1;
	}
	if (-1 == selectRet) {
		printf("api_TimeOut select fail!\n");
		return -1;
	} else if (0 == selectRet) {
		printf("api_TimeOut time out!\n");
		return 0;
	} else {
		return flag;
	}
}

/*
 * 函数功能: 看字符串中是否有指定的字符串checkStr
 * 参数：	  待寻找的字符串askStr，指定的字符串checkStr
 * 返回值：  有就返回0，没有返回-1
 * */
s32 api_FindStr(char * askStr, char *checkStr) {
	printf("%s", askStr);
	if (NULL != strstr((const char*) askStr, checkStr)) {
		return 0;
	} else
		return -1;
}

int HexVal(unsigned char ch) {
	if (ch >= '0' && ch <= '9') {
		return ch - '0';
	} else if (ch >= 'A' && ch <= 'F') {
		return 10 + ch - 'A';
	} else if (ch >= 'a' && ch <= 'f') {
		return 10 + ch - 'a';
	} else {
		return -1;
	}
}

unsigned char ValHex(int val) {
	if (val >= 0 && val <= 9) {
		return val + '0';
	} else if (val >= 10 && val <= 15) {
		return val - 10 + 'A';
	} else {
		return '$';
	}
}
/*
 文本串转十六进制串
 */
s32 api_HexToAsc(unsigned char* strhex, unsigned char* strasc, int length) {
	int i;
	int pos = 0;

	if (length % 2 == 1) {
		return -1;
	}
	for (i = 0; i < length; i++) {
		if (!isxdigit(strhex[i])) {
			return -2;
		}
	}

	for (i = 0; i < length; i += 2) {
		unsigned char tmp = (HexVal(strhex[i]) << 4) + HexVal(strhex[i + 1]);
		strasc[pos++] = tmp;
	}
	return pos;
}
/*
 十六进制串转文本串
 */
s32 api_AscToHex(unsigned char* strasc, unsigned char* strhex, int length) {
	int i = 0, pos = 0;
	for (i = 0; i < length; ++i) {
		strhex[pos++] = ValHex(strasc[i] >> 4);
		strhex[pos++] = ValHex(strasc[i] & 0x0F);
	}
	return 0;
}

/*******************************************************
 NAME:
 FUNC:CRC校验码
 * ****************************************************/
u16 api_CheckCrc(u16 CRC, void *Data, u32 Size) {
	u32 i;
	u8 *ptr = (u8 *) Data;
	for (i = 0; i < Size; i++) {
		CRC = (unsigned char) (CRC >> 8) | (CRC << 8);
		CRC ^= *ptr++;
		CRC ^= (unsigned char) (CRC & 0xff) >> 4;
		CRC ^= (CRC << 8) << 4;
		CRC ^= ((CRC & 0xff) << 4) << 1;
	}
	return CRC;
}

u8 api_CheckSum(u8 *buf, int nword) {
	u8 sum;
	int i;

	sum = 0;
	for (i = 0; i < nword; i++) {
		sum += buf[i];
	}

	return sum;
}

