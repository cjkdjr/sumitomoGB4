/*
 * general.c
 *
 *  Created on: 2019年2月26日
 *      Author: tykj
 */
#include "api.h"
#include "general.h"

/*
 版本号赋值
 */
void cpyVersion(u8 *des) {
	//版本号
	des[0] = Version.Protocol >> 8;
	des[1] = Version.Protocol;
	des[2] = Version.Code;
	return;
}
/*
 信息生成时间+流水号赋值
 */
u32 cpyInfoTime(u8 *des) {
	struct tm *p;
//	struct timeval tv;
//	u16 tmp_sec;
	time_t now_time;
	u32 sys_run_msec;
	u16 tmp_u16;
	//信息生成时间:获取当前时间 = 系统运行时间+ key on时间（RTC时间）
	u32 Sys_Run_Time = api_GetSysSecs();
	now_time = Sys_Start_Time + Sys_Run_Time;
	p = localtime(&now_time);
	des[0] = p->tm_year - 100;
	des[1] = p->tm_mon + 1;
	des[2] = p->tm_mday;
	des[3] = p->tm_hour;
	des[4] = p->tm_min;
	tmp_u16 = (p->tm_sec) * 1000;
	sys_run_msec = api_GetSysmSecs();
	tmp_u16 += sys_run_msec % 1000;
	des[5] = tmp_u16 >> 8;
	des[6] = tmp_u16;
	//信息生成时间
//	gettimeofday(&tv, NULL);
//	p = localtime(&tv.tv_sec);
//	des[0] = p->tm_year - 100;
//	des[1] = p->tm_mon + 1;
//	des[2] = p->tm_mday;
//	des[3] = p->tm_hour;
//	des[4] = p->tm_min;
//	tmp_sec = p->tm_sec * 1000 + tv.tv_usec / 1000;
//	des[5] = tmp_sec >> 8;
//	des[6] = tmp_sec;
	if (Serial_Num < 0xFF) {
		Serial_Num++;
	} else {
		Serial_Num = 0;
	}
	des[7] = Serial_Num;
	return sys_run_msec;
}
