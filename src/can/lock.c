#include <stdio.h>
#include <linux/can.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

#include "api.h"
#include "candriver.h"
#include "candata.h"
#include "general.h"
#include "gps_4g.h"
#include "lock.h"
#include "sys_manage.h"
#include "Msg_task.h"

u8 lock_RtcTime[6];
u8 lock_level; // 当前锁车级别，每次变化只在KEYON时执行 0-默认(解车) 1-锁1级 2-锁2级
u8 lock_state = 0; //锁车状态
lock_ImmeLockbit lock_Lock_Imme_Data; //立即锁条件
lock_HourLockbit lock_Lock_Hour_Data; //工作小时锁条件
lock_PosiLockbit lock_Lock_Posi_Data; //位置锁条件
lock_TimeLockbit lock_Lock_Time_Data; //指定时间点锁条件
lock_DateLockbit lock_Lock_Date_Data; //指定日期组锁锁条件
lockSignSetbit lock_LockCar_State; //锁车状态字
u8 lock_LockCar_Applyt[16]; //锁车申请
u32 lock_SendApplyt_Timer, lock_lockjudge_timer;

bytebit LockSign00, LockSign01;

u8 lock_save_center_set_flg; // 1保存 其它不保存
u32 lock_total_work_hours; // 当前总工作小时，单位小时
void LockCarJudge(void);
void LockCarSendReport(u8 *dzm);
void LockCarSendApplyt(u8 *dzm);

u8 Lock_HandleMiMaUnlock(u8 *data);
void Lock_TakeHandShakeMima(u8 *mima, u8 *imei, u8 *imsi);

u8 LockImmeLockJudge(u8 num, u8 *LockImmeData, u8 *LockCarState, u8 *LockCarApplyt);
u8 LockHourLockJudge(u8 num, u8 *LockHourData, u8 *LockCarState, u8 *LockCarApplyt, u16 CarHour);
u8 LockPosiLockJudge(u8 num, u8 *LockPosiData, u8 *LockCarState, u8 *LockCarApplyt, u32 wd, u32 jd);
u8 LockTimeLockJudge(u8 num, u8 *LockTimeData, u8 *LockCarState, u8 *LockCarApplyt, u8* Time);
u8 LockDateLockJudge(u8 num, u8 *LockDateData, u8 *LockCarState, u8 *LockCarApplyt, u8 *Time);
u8 LockCmpData(u8 *data1, u8 *data2, u16 Len);

int u32_bytes(uint8_t *buf, uint32_t tmp) {
	buf[0] = tmp >> 24;
	buf[1] = tmp >> 16;
	buf[2] = tmp >> 8;
	buf[3] = tmp;

	return (4);
}
/**
 * @ 根据机型不同执行不同的锁车逻辑
 */
void lock_sta_update(void) {
#if 0 // 自动防拆锁车状态检测	if (time_GsmAntennaLockCarOn == 1 || time_GsmAntennaLockCar == 0	|| lock_MimaUnlock_GsmAnt == 1)	{	//b=0;	lock_LockCar_State_F_GSMAnt_St = 0;	lock_LockCar_State_F_GSMAnt_Level = 0;}	else	{	//b=time_GsmAntennaLockCarSt;	lock_LockCar_State_F_GSMAnt_St = 1;	lock_LockCar_State_F_GSMAnt_Level = time_GsmAntennaLockCarSt;}	if (time_GpsAntennaLockCarOn == 1 || time_GpsAntennaLockCar == 0	|| lock_MimaUnlock_GpsAnt == 1)	{
		//c=0;
		lock_LockCar_State_F_GPSAnt_St = 0;
		lock_LockCar_State_F_GPSAnt_Level = 0;
	}
	else
	{
		//c=time_GpsAntennaLockCarSt;
		lock_LockCar_State_F_GPSAnt_St = 1;
		lock_LockCar_State_F_GPSAnt_Level = time_GpsAntennaLockCarSt;
	}
	if (time_SIMBCLockCarOn == 1 || time_SIMBCLockCar == 0
			|| lock_MimaUnlock_Sim == 1)
	{
		//d=0;
		lock_LockCar_State_F_SIM_St = 0;
		lock_LockCar_State_F_SIM_Level = 0;
	}
	else
	{
		//d=time_SIMBCLockCarSt;
		lock_LockCar_State_F_SIM_St = 1;
		lock_LockCar_State_F_SIM_Level = time_SIMBCLockCarSt;
	}
#endif

	u8 lock_level_tmp = 0;
	u8 tmp;
	int i;
	lock_keyon = System.AccState;
	for (i = 0; i < 16; i++) {
		if (lock_LockCar_State_Ram[i] & 0x80) {
			tmp = (lock_LockCar_State_Ram[i] & 0x70) >> 4;
			if (lock_level_tmp < tmp)
				lock_level_tmp = tmp;
		}
		if (lock_LockCar_State_Ram[i] & 0x08) {
			tmp = (lock_LockCar_State_Ram[i] & 0x07);
			if (lock_level_tmp < tmp)
				lock_level_tmp = tmp;
		}
	}
	if (lock_keyon == 0) { // keyoff时切换锁定状态
		if (lock_level != lock_level_tmp) {
			printf("lock_level=%d->%d\n", lock_level, lock_level_tmp);
			lock_level = lock_level_tmp;
			SaveLockCarData();
		}
	}

}

/**
 * @brief 锁车动作
 * @brief 解车-PD10-高-OUT1-地  锁车-PD10-低-OUT1-悬空
 */
//void lock_action(void) {
//	if (lock_level == 0) {
//		if (!GPIO_ReadOutputDataBit(STD_IO_OUT1)) {
//			GPIO_SetBits(STD_IO_OUT1);
//		}
//	} else if (lock_level == 1) {
//		if (GPIO_ReadOutputDataBit(STD_IO_OUT1)) {
//			GPIO_ResetBits(STD_IO_OUT1);
//		}
//	} else if (lock_level == 2) {
//		if (GPIO_ReadOutputDataBit(STD_IO_OUT1)) {
//			GPIO_ResetBits(STD_IO_OUT1);
//		}
//	}
//}
u8 Sys_RTC_Get(u8 RTCtime[]) {
	struct tm *p;
	time_t timep;
	time(&timep);
	p = localtime(&timep);

	RTCtime[0] = p->tm_year - 100;
	RTCtime[1] = p->tm_mon + 1;
	RTCtime[2] = p->tm_mday;
	RTCtime[3] = p->tm_hour;
	RTCtime[4] = p->tm_min;
	RTCtime[5] = p->tm_sec;
	return 0;
}
void LockCarJudge(void) {
	u8 b;
	u16 lock_CarHour;
	u32 lock_GPS_W, lock_GPS_J;
//	u8 lock_RtcTime[6];
	b = 0;
	if (lock_lockjudge_timer == 0) {
		b = 1;
	}
	if (lock_lockjudge_timer > api_GetSysmSecs()) {
		b = 1;
	}
	if (api_GetSysmSecs() - lock_lockjudge_timer > 10000) {
		b = 1;
	}
	if (lock_MimaUnlock_Imme == 1 || lock_MimaUnlock_Hour == 1 || lock_MimaUnlock_Date == 1 || lock_MimaUnlock_Time == 1 || lock_MimaUnlock_Posi == 1)
		b = 1;
	if (b == 0)
		return;
	lock_lockjudge_timer = api_GetSysmSecs();
	Sys_RTC_Get(lock_RtcTime); // 间隔10秒 更新时间

	for (b = 0; b < lock_LockCar_Applyt_Len; b++) {
		if (lock_LockCar_Applyt[b] != 0)
			b = 50;
	}
	if (b < 20)
		lock_ApplytSendSign = 0;

	LockImmeLockJudge(0, lock_Lock_Imme_Data_Ram, lock_LockCar_State_Ram, lock_LockCar_Applyt);
	LockImmeLockJudge(1, lock_Lock_Imme_Data_Ram, lock_LockCar_State_Ram, lock_LockCar_Applyt);
	LockImmeLockJudge(2, lock_Lock_Imme_Data_Ram, lock_LockCar_State_Ram, lock_LockCar_Applyt);
	lock_MimaUnlock_Imme = 0;

	lock_CarHour = lock_total_work_hours;
	LockHourLockJudge(0, lock_Lock_Hour_Data_Ram, lock_LockCar_State_Ram, lock_LockCar_Applyt, lock_CarHour);
	LockHourLockJudge(1, lock_Lock_Hour_Data_Ram, lock_LockCar_State_Ram, lock_LockCar_Applyt, lock_CarHour);
	LockHourLockJudge(2, lock_Lock_Hour_Data_Ram, lock_LockCar_State_Ram, lock_LockCar_Applyt, lock_CarHour);
	lock_MimaUnlock_Hour = 0;

	if (System.GPS.Valid == 1 || lock_MimaUnlock_Posi == 1) {
		lock_GPS_W = System.GPS.Position.Latitude * 6 / 10; //万分之一分
		lock_GPS_J = System.GPS.Position.Longitude * 6 / 10;

		LockPosiLockJudge(0, lock_Lock_Posi_Data_Ram, lock_LockCar_State_Ram, lock_LockCar_Applyt, lock_GPS_W, lock_GPS_J);
		LockPosiLockJudge(1, lock_Lock_Posi_Data_Ram, lock_LockCar_State_Ram, lock_LockCar_Applyt, lock_GPS_W, lock_GPS_J);
		LockPosiLockJudge(2, lock_Lock_Posi_Data_Ram, lock_LockCar_State_Ram, lock_LockCar_Applyt, lock_GPS_W, lock_GPS_J);
		lock_MimaUnlock_Posi = 0;
	}

	//if(lock_MimaUnlock_Time==1)
	{
		LockTimeLockJudge(0, lock_Lock_Time_Data_Ram, lock_LockCar_State_Ram, lock_LockCar_Applyt, lock_RtcTime);
		LockTimeLockJudge(1, lock_Lock_Time_Data_Ram, lock_LockCar_State_Ram, lock_LockCar_Applyt, lock_RtcTime);
		LockTimeLockJudge(2, lock_Lock_Time_Data_Ram, lock_LockCar_State_Ram, lock_LockCar_Applyt, lock_RtcTime);
		lock_MimaUnlock_Time = 0;
	}
	//if(lock_MimaUnlock_Date==1)
	{
		LockDateLockJudge(0, lock_Lock_Date_Data_Ram, lock_LockCar_State_Ram, lock_LockCar_Applyt, lock_RtcTime);
		LockDateLockJudge(1, lock_Lock_Date_Data_Ram, lock_LockCar_State_Ram, lock_LockCar_Applyt, lock_RtcTime);
		LockDateLockJudge(2, lock_Lock_Date_Data_Ram, lock_LockCar_State_Ram, lock_LockCar_Applyt, lock_RtcTime);
		lock_MimaUnlock_Date = 0;
	}

	if (lock_ApplytSendSign == 1) {
		if (lock_SendApplyt_Timer == 0)
			lock_SendApplyt_Timer = api_GetSysmSecs();
		if (lock_SendApplyt_Timer > api_GetSysmSecs())
			lock_SendApplyt_Timer = api_GetSysmSecs();
		if (api_GetSysmSecs() - lock_SendApplyt_Timer > 3600000) {
			lock_SendApplyt_Timer = api_GetSysmSecs();
			for (b = 0; b < lock_LockCar_Applyt_Len; b++)
				lock_LockCar_Applyt[b] = 0;
			lock_ApplytSendSign = 0;
		}
	} else
		lock_SendApplyt_Timer = 0;

	lock_sta_update();
//	lock_action();

}
/*
int gps_format_jwd_mem(u8 *mem) {
	u32 tmp_u32;
	u8 Du, Fen;
	u16 Miao;
	int index = 0;

	memset(mem, 0, 12); // ??? ????12??
	if (System.GPS.Position.Valid == 1) {   // if GPS is valid
		tmp_u32 = GPSbuf.latitude;
		Du = tmp_u32 / 600000;
		Fen = (tmp_u32 - Du * 600000) / 10000;
		Miao = tmp_u32 - Du * 600000 - Fen * 10000;
		mem[index++] = Du;
		mem[index++] = Fen;
		if (GPSbuf.NS) {
			mem[index - 1] |= 0x80;
		}
//		if (GPS.GPSData.GPSNEW == TRUE)
//		{
		mem[index - 1] |= 0x40;
//		}
		mem[index++] = Miao >> 8;
		mem[index++] = Miao;
		tmp_u32 = GPSbuf.longitude;
		Du = tmp_u32 / 600000;
		Fen = (tmp_u32 - Du * 600000) / 10000;
		mem[index++] = Du;
		mem[index++] = Fen;
		if (GPSbuf.EW) {
			mem[index - 1] |= 0x80;
		}
		mem[index++] = Miao >> 8;
		mem[index++] = Miao;
		tmp_u32 = GPSbuf.starnum;
		tmp_u32 <<= 5;
		tmp_u32 |= GPSbuf.direction;
		tmp_u32 <<= 9;
		tmp_u32 |= GPSbuf.speed;
		tmp_u32 <<= 10;
		tmp_u32 |= System.GsmCSQ;

		index += u32_bytes(mem + index, tmp_u32);
	}

	return (12);
}
*/
/**
 实现: 立即锁判断
 num : 指示层锁车  0 - A层   1 - B层   2 - C层
 LockImmeData : 立即锁条件
 LockCarState : 锁车状态字
 LockCarApplyt: 锁车申请
 Return : 0xFE - 立即锁条件错误（锁车级别）
 0x00 - 解车，无锁车条件
 0x01 - 无动作，解车状态
 0x02 - 无动作，锁车状态
 0x03 - 解车，换锁车条件
 0x05 - 锁车
 0x06 - 发送锁车申请
 0x16 - 已发送锁车申请
 */
u8 LockImmeLockJudge(u8 num, u8 *LockImmeData, u8 *LockCarState, u8 *LockCarApplyt) {
	u8 a, b, c, d;
	u8 *dzm;
	u8 s;

//	printf("num:%d,LockCarState:%02x\n",num,LockCarState[0]);
	a = lock_Lock_Imme_Data_EveryLen * num;
	LockImmeData = LockImmeData + a;
	dzm = LockImmeData + 1;
//	printf("LockImmeData:%02x,a:%02x\n",LockImmeData[0],a);
	a = lock_LockCar_State_EveryLen * num;
	LockCarState = LockCarState + a;
	LockCarApplyt = LockCarApplyt + a;
	s = LockCarApplyt[0] & 0x80; //是否发送锁车申请
	LockCarApplyt[0] = LockCarApplyt[0] & 0x0F;

	//a=lock_Lock_Validate_EveryLen*num;
	//LockValidate=LockValidate+a;

	a = LockImmeData[0] >> 5; //设置条件的锁车级别

	b = LockCarState[0] & 0x80; //锁车状态字中的锁车标志
	c = LockCarState[0] >> 4;
	c = c & 0x07; //锁车状态字中的锁车级别

	//d=LockValidate[0]&0x80;//锁车确认
	d = LockImmeData[0] & 0x10; //锁车确认
//	printf("a:%02x;b:%02x;c%02x;d:%02x\n",a,b,c,d);
	if (a > 3)
		return (0xFE);
	if (a == 0) { //无锁车条件
		if (b != 0) { //已锁车
		              //执行解车
			LockCarState[0] = LockCarState[0] & 0x0F;
			LockCarSendReport(dzm); //发解车报告
//			printf("111111:LockCarState:%02x\n", LockCarState[0]);
			return (0x00);
		}
		return (0x01); //未锁车
	}

	if (b != 0) { //已锁车
		if (a == c)
			return (0x02); //锁车级别相同
		//执行解车
		LockCarState[0] = LockCarState[0] & 0x0F;
		LockCarSendReport(dzm);   //发送解车报告
//		printf("222222:LockCarState:%02x\n", LockCarState[0]);
		return (0x03);
	}

	a = a | 0x08;
	a = a << 4;
//	printf("a:%02x;d:%02x\n",a,d);
	if (d != 0) {   //需确认
	                //发送锁车申请
		lock_ApplytSendSign = 1;
		LockCarApplyt[0] = LockCarApplyt[0] | a;
		if (s != 0)
			return (0x16); //已发送锁车申请
		LockCarSendApplyt(dzm);
//		printf("3333333:LockCarState:%02x\n", LockCarState[0]);
		return (0x06); //发送锁车申请
	}
	//更改锁车状态字
	LockCarState[0] = LockCarState[0] & 0x0F;
	LockCarState[0] = LockCarState[0] | a;
//	printf("444444:LockCarState:%02X\n", LockCarState[0]);
	LockCarSendReport(dzm); //发锁车报告
	//Lock_AddLockNum(num);  // 累计锁车次数
	return (0x05); //发锁车报告

}

/**
 实现: 工作小时锁判断
 num : 指示层锁车  0 - A层   1 - B层   2 - C层
 LockHourData : 工作小时锁条件
 LockCarState : 锁车状态字
 LockCarApplyt: 锁车申请
 CarHour : 车辆工作小时
 Return : 0xFE - 立即锁条件错误（锁车级别）
 0x00 - 解车，无锁车条件
 0x01 - 无动作，解车状态
 0x02 - 解车，锁车条件不符合
 0x03 - 解车，换锁车条件
 0x04 - 无动作，锁车状态
 0x05 - 无动作，解车状态
 0x07 - 锁车
 0x06 - 发送锁车申请
 0x16 - 已发送锁车申请
 */
u8 LockHourLockJudge(u8 num, u8 *LockHourData, u8 *LockCarState, u8 *LockCarApplyt, u16 CarHour) {
	u8 a, b, c, d;
	u16 i;
	u8 *dzm;
	u8 s;

	a = lock_Lock_Hour_Data_EveryLen * num;
	LockHourData = LockHourData + a;
	dzm = LockHourData + 3;

	a = lock_LockCar_State_EveryLen * num;
	LockCarState = LockCarState + a;
	LockCarApplyt = LockCarApplyt + a;
	s = LockCarApplyt[0] & 0x08;  //是否发送锁车申请
	LockCarApplyt[0] = LockCarApplyt[0] & 0xF0;

	//a=lock_Lock_Validate_EveryLen*num;
	//LockValidate=LockValidate+a;

	a = LockHourData[0] >> 5; //设置条件的锁车级别
	i = LockHourData[1];
	i = i << 8;
	i = i | LockHourData[2];

	b = LockCarState[0] & 0x08; //锁车状态字中的锁车标志
	c = LockCarState[0] & 0x07; //锁车状态字中的锁车级别

	//d=LockValidate[0]&0x40;//锁车确认
	d = LockHourData[0] & 0x10; //锁车确认

	if (a > 3)
		return (0xFE);
	if (a == 0) //无锁车条件
	                {
		if (b != 0) //已锁车
		                {
			//执行解车
			LockCarState[0] = LockCarState[0] & 0xF0;
			LockCarSendReport(dzm); //发解车报告
			return (0x00);
		}
		return (0x01); //未锁车
	}

	if (b != 0) //已锁车
	                {
		if (CarHour < i) //锁车条件不符合
		                {
			//执行解车
			LockCarState[0] = LockCarState[0] & 0xF0;
			LockCarSendReport(dzm); //发送解车报告
			return (0x02);
		}
		if (a != c) //锁车级别不相同
		                {
			//执行解车
			LockCarState[0] = LockCarState[0] & 0xF0;
			LockCarSendReport(dzm); //发送解车报告
			return (0x03);
		}
		return (0x04);
	}

	if (CarHour < i)
		return (0x05); //锁车条件不符合

	a = a | 0x08;

	if (d != 0) //需确认
	                {
		//发送锁车申请
		lock_ApplytSendSign = 1;
		LockCarApplyt[0] = LockCarApplyt[0] | a;
		if (s != 0)
			return (0x16); //已发送锁车申请
		LockCarSendApplyt(dzm);
		return (0x06); //发送锁车申请
	}
	//更改锁车状态字
	LockCarState[0] = LockCarState[0] & 0xF0;
	LockCarState[0] = LockCarState[0] | a;
	LockCarSendReport(dzm); //发锁车报告
	//Lock_AddLockNum(num);  // 累计锁车次数
	return (0x07); //发锁车报告

}

/**
 实现: 位置锁判断
 num : 指示层锁车  0 - A层   1 - B层   2 - C层
 LockPosiData : 位置锁条件
 LockCarState : 锁车状态字
 LockCarApplyt: 锁车申请
 wd           : 纬度0.000001度
 jd           : 经度0.000001度
 Return : 0xFE - 锁条件错误（锁车级别）
 0x00 - 解车，无锁车条件
 0x01 - 无动作，解车状态
 0x02 - 解车，锁车条件不符合
 0x03 - 解车，换锁车条件
 0x04 - 无动作，锁车状态
 0x05 - 无动作，解车状态
 0x06 - 发送锁车申请
 0x16 - 已发送锁车申请
 0x07 - 锁车
 */
u8 LockPosiLockJudge(u8 num, u8 *LockPosiData, u8 *LockCarState, u8 *LockCarApplyt, u32 wd, u32 jd) {
	u8 a, b, c, d;
	u32 j1, w1;
	u8 *dzm;
	u8 s;

	a = lock_Lock_Posi_Data_EveryLen * num;
	LockPosiData = LockPosiData + a;
	dzm = LockPosiData + 13;

	a = lock_LockCar_State_EveryLen * num;
	LockCarState = LockCarState + a;
	LockCarApplyt = LockCarApplyt + a;
	s = LockCarApplyt[1] & 0x80;
	LockCarApplyt[1] = LockCarApplyt[1] & 0x0F;

	//a=lock_Lock_Validate_EveryLen*num;
	//LockValidate=LockValidate+a;

	a = LockPosiData[0] >> 5; //设置条件的锁车级别

	b = LockCarState[1] & 0x80; //锁车状态字中的锁车标志
	c = LockCarState[1] >> 4;
	c = c & 0x07; //锁车状态字中的锁车级别

	//d=LockValidate[0]&0x10;//锁车确认
	d = LockPosiData[0] & 0x10; //锁车确认

	if (a > 3)
		return (0xFE);
	if (a == 0) //无锁车条件
	                {
		if (b != 0) //已锁车
		                {
			//执行解车
			LockCarState[1] = LockCarState[1] & 0x0F;
			LockCarSendReport(dzm); //发解车报告
			return (0x00);
		}
		return (0x01); //未锁车
	}

	if (lock_MimaUnlock_Posi == 1)
		return (0xFF);

	w1 = LockPosiData[1];
	w1 = w1 << 8;
	w1 = w1 | LockPosiData[2];
	w1 = w1 << 8;
	w1 = w1 | LockPosiData[3];
	w1 = w1 << 8;
	w1 = w1 | LockPosiData[4];
	w1 = w1 * 6;
	w1 = w1 / 10;

	j1 = LockPosiData[5];
	j1 = j1 << 8;
	j1 = j1 | LockPosiData[6];
	j1 = j1 << 8;
	j1 = j1 | LockPosiData[7];
	j1 = j1 << 8;
	j1 = j1 | LockPosiData[8];
	j1 = j1 * 6;
	j1 = j1 / 10;

	w1 = Fun_Distance_jw(w1, j1, wd, jd);
	j1 = LockPosiData[9];
	j1 = j1 << 8;
	j1 = j1 | LockPosiData[10];
	j1 = j1 << 8;
	j1 = j1 | LockPosiData[11];
	j1 = j1 << 8;
	j1 = j1 | LockPosiData[12];

	if (b != 0) //已锁车
	                {
		if (w1 < j1) //锁车条件不符合
		                {
			//执行解车
			LockCarState[1] = LockCarState[1] & 0x0F;
			LockCarSendReport(dzm); //发送解车报告
			return (0x02);
		}
		if (a != c) //锁车级别不相同
		                {
			//执行解车
			LockCarState[1] = LockCarState[1] & 0x0F;
			LockCarSendReport(dzm); //发送解车报告
			return (0x03);
		}
		return (0x04);
	}

	if (w1 < j1)
		return (0x05); //锁车条件不符合

	a = a | 0x08;
	a = a << 4;

	if (d != 0) //需确认
	                {
		//发送锁车申请
		lock_ApplytSendSign = 1;
		LockCarApplyt[1] = LockCarApplyt[1] | a;
		if (s != 0)
			return (0x16); //已发送锁车申请
		LockCarSendApplyt(dzm);
		return (0x06); //发送锁车申请
	}
	//更改锁车状态字
	LockCarState[1] = LockCarState[1] & 0x0F;
	LockCarState[1] = LockCarState[1] | a;
	LockCarSendReport(dzm); //发锁车报告
	//Lock_AddLockNum(num);  // 累计锁车次数
	return (0x07); //发锁车报告

}

/**
 实现: 指定时间点锁判断
 num : 指示层锁车  0 - A层   1 - B层   2 - C层
 LockTimeData : 工作小时锁条件
 LockCarState : 锁车状态字
 LockCarApplyt: 锁车申请
 Time         : 当前终端时间
 Return : 0xFE - 立即锁条件错误（锁车级别）
 0x00 - 解车，无锁车条件
 0x01 - 无动作，解车状态
 0x02 - 解车，锁车条件不符合
 0x03 - 解车，换锁车条件
 0x04 - 无动作，锁车状态
 0x05 - 无动作，解车状态
 0x06 - 发送锁车申请
 0x16 - 已发送锁车申请
 0x07 - 锁车
 */
u8 LockTimeLockJudge(u8 num, u8 *LockTimeData, u8 *LockCarState, u8 *LockCarApplyt, u8* Time) {
	u8 a, b, c, d;
	u8 e;
	u8 *dzm;
	u8 s;

	a = lock_Lock_Time_Data_EveryLen * num;
	LockTimeData = LockTimeData + a;
	dzm = LockTimeData + 7;

	a = lock_LockCar_State_EveryLen * num;
	LockCarState = LockCarState + a;
	LockCarApplyt = LockCarApplyt + a;
	s = LockCarApplyt[1] & 0x08;
	LockCarApplyt[1] = LockCarApplyt[1] & 0xF0;

	//a=lock_Lock_Validate_EveryLen*num;
	//LockValidate=LockValidate+a;

	a = LockTimeData[0] >> 5; //设置条件的锁车级别

	b = LockCarState[1] & 0x08; //锁车状态字中的锁车标志
	c = LockCarState[1] & 0x07; //锁车状态字中的锁车级别

	d = LockTimeData[0] & 0x10; //锁车确认

	if (a > 3)
		return (0xFE);
	if (a == 0) { //无锁车条件
		if (b != 0) { //已锁车
		              //执行解车
			LockCarState[1] = LockCarState[1] & 0xF0;
			LockCarSendReport(dzm); //发解车报告
			return (0x00);
		}
		return (0x01); //未锁车
	}

	if (lock_MimaUnlock_Time == 1)
		return (0xFF);

	e = LockCmpData(Time, LockTimeData + 1, 6);

	if (b != 0) { //已锁车
		if (e == 2) { //锁车条件不符合
		              //执行解车
			LockCarState[1] = LockCarState[1] & 0xF0;
			LockCarSendReport(dzm); //发送解车报告
			return (0x02);
		}
		if (a != c) { //锁车级别不相同
		              //执行解车
			LockCarState[1] = LockCarState[1] & 0xF0;
			LockCarSendReport(dzm); //发送解车报告
			return (0x03);
		}
		return (0x04);
	}

	if (e == 2)
		return (0x05); //锁车条件不符合

	a = a | 0x08;
	if (d != 0) { //需确认
	              //发送锁车申请
		lock_ApplytSendSign = 1;
		LockCarApplyt[1] = LockCarApplyt[1] | a;
		if (s != 0)
			return (0x16); //已发送锁车申请
		LockCarSendApplyt(dzm);
		return (0x06); //发送锁车申请
	}
	//更改锁车状态字
	LockCarState[1] = LockCarState[1] & 0xF0;
	LockCarState[1] = LockCarState[1] | a;
	LockCarSendReport(dzm); //发锁车报告
	//Lock_AddLockNum(num);  // 累计锁车次数
	return (0x07); //发锁车报告

}

/**
 实现: 指定日期组锁判断
 num : 指示层锁车  0 - A层   1 - B层   2 - C层
 LockDateData : 指定日期组锁条件
 LockCarState : 锁车状态字
 LockCarApplyt: 锁车申请
 Time         : 当前终端时间
 Return : 0xFE - 锁条件错误（锁车级别）
 0x00 - 解车，无锁车条件,锁车条件不满足（还未达到第一个锁车月）
 0x01 - 无动作，解车状态
 0x02 - 解车，锁车条件不符合
 0x03 - 解车，换锁车条件
 0x04 - 无动作，保持上一状态
 0x05 - 无动作，解车状态
 0x06 - 发送锁车申请
 0x16 - 已发送锁车申请
 0x07 - 锁车
 */
u8 LockDateLockJudge(u8 num, u8 *LockDateData, u8 *LockCarState, u8 *LockCarApplyt, u8 *Time) {
	u8 a, b, c;
	u8 e;
	u8 f, lockDate[3];
	u8 *dzm;
	u8 lockpoint;

	a = lock_Lock_Date_Data_EveryLen * num;
	LockDateData = LockDateData + a;
	dzm = LockDateData + 51;

	a = lock_LockCar_State_EveryLen * num;
	LockCarState = LockCarState + a;
	LockCarApplyt = LockCarApplyt + a;
#if 0
	s = LockCarApplyt[3] & 0x80;
#endif
	LockCarApplyt[3] = LockCarApplyt[3] & 0x0F;

	//a=lock_Lock_Validate_EveryLen*num;
	//LockValidate=LockValidate+a;

	a = LockDateData[0] >> 5; //设置条件的锁车级别

	b = LockCarState[3] & 0x80; //锁车状态字中的锁车标志
	c = LockCarState[3] >> 4;
	c = c & 0x07; //锁车状态字中的锁车级别

	//d=LockValidate[0]&0x02;//锁车确认
#if 0
	d = LockDateData[0] & 0x10; //锁车确认
#endif
	if (a > 3)
		return (0xFE);

	if (a == 0) { //无锁车条件
		if (b != 0) { //已锁车
		              //执行解车
			LockCarState[3] = LockCarState[3] & 0x0F;
			LockCarSendReport(dzm); //发解车报告

			return (0x00);
		}
		return (0x01); //未锁车
	}

	e = 0;
	for (f = 3; f < 51; f++) {
		if (LockDateData[f] == 0xFE) {
			if (e != 0) {
				LockDateData[e] = 0xFD; //该月锁车已过期
				SaveLockCarData();
			}
			e = f; //当前存在已锁车的条件
		}
	}
	if (e > 0) {
		if (b == 0) { //之前未锁车》》》》》》》》》》》》》》》
			a = a | 0x08;
			a = a << 4;
			LockCarState[3] = LockCarState[3] & 0x0F;
			LockCarState[3] = LockCarState[3] | a;
			LockCarSendReport(dzm); //发锁车报告
			//Lock_AddLockNum(num);  // 累计锁车次数
			return (0x07);     //发锁车报告;
		} else {
			if (a != c) {    //锁车级别不相同
				//执行解车
				LockCarState[3] = LockCarState[3] & 0x0F;
				LockCarSendReport(dzm);     //发送解车报告
				return (0x03);
			}
		}
	} else {
		if (b != 0) {
			//执行解车
			LockCarState[3] = LockCarState[3] & 0x0F;
			if (lock_MimaUnlock_Date == 1) {
				lockDate[0] = 0xFF;
				lockDate[1] = 0xFF;
				lockDate[2] = 0xFF;
				LockCarSendReport(lockDate);
			} else
				LockCarSendReport(dzm);     //发送解车报告
			return (0x02);
		}
	}

	if (lock_MimaUnlock_Date == 1)
		return (0xFF);

	e = LockCmpData(Time, LockDateData + 1, 2);
	if (e == 2) {    //锁车条件不满足（还未达到第一个锁车月）
		if (b != 0) {    //已锁车
			//执行解车
			LockCarState[3] = LockCarState[3] & 0x0F;
			LockCarSendReport(dzm);     //发解车报告
			return (0x00);
		}
		return (0x01); //未锁车
	}

	for (f = 0; f < 48; f++) {
		lockDate[0] = LockDateData[1];
		lockDate[1] = LockDateData[2] + f;
		while (lockDate[1] > 12) {
			lockDate[1] = lockDate[1] - 12;
			lockDate[0]++;
		}
		lockpoint = 3 + f;
		lockDate[2] = LockDateData[lockpoint];
		e = LockCmpData(Time, lockDate, 2);
		if (e == 0)
			f = 48; //找到当前月
	}

	if (lockDate[2] > 31 || lockDate[2] == 0)
		return (0x04); //当前月锁车条件无效

	e = LockCmpData(Time, lockDate, 3);
	if (e == 2)
		return (0x04); //锁车条件不满足

	a = a | 0x08;
	a = a << 4;
	/*
	 if(d!=0) //需确认
	 {
	 //发送锁车申请
	 lock_ApplytSendSign=1;
	 LockCarApplyt[3]=LockCarApplyt[3]|a;
	 if(s!=0) return(0x16);//已发送锁车申请
	 LockCarSendApplyt(dzm);
	 return(0x06);//发送锁车申请
	 }
	 */
	//更改锁车状态字
	LockCarState[3] = LockCarState[3] & 0x0F;
	LockCarState[3] = LockCarState[3] | a;

	LockDateData[lockpoint] = 0xFE;
	LockCarSendReport(dzm); //发锁车报告
	//Lock_AddLockNum(num);  // 累计锁车次数

	e = 0;
	for (f = 3; f < 51; f++) {
		if (LockDateData[f] == 0xFE) {
			if (e != 0) {
				LockDateData[e] = 0xFD; //该月锁车已过期

				SaveLockCarData();
			}
			e = f; //当前存在已锁车的条件
		}
	}

	return (0x07); //发锁车报告

}

/**
 实现: 锁解车报告:0x88
 dzm[3] : 立即锁信息对照码
 */

void LockCarSendReport(u8 *dzm) {
	u8 a;
	u32 lock_hours;
	u16 len = 0;
	time_t now_time;
	struct tm *p;
	u32 Sys_Run_Time;        //系统开机后的运行时间---局部变量

	QUE_TDF_QUEUE_MSG Lock88_Msg;
	Lock88_Msg.MsgType = 0x88;
	Lock88_Msg.Version[0] = Version.Protocol >> 8;
	Lock88_Msg.Version[1] = Version.Protocol;
	Lock88_Msg.Version[2] = Version.Code;
	//属性标识  高字节在前
	Lock88_Msg.Attribute[0] = 0x40;
	Lock88_Msg.Attribute[1] = 0;
	//获取当前时间 = 系统运行时间+ key on时间（RTC时间）
	Sys_Run_Time = api_GetSysSecs();
	now_time = Sys_Start_Time + Sys_Run_Time;
	p = localtime(&now_time);
	Lock88_Msg.Time[0] = p->tm_year - 100;
	Lock88_Msg.Time[1] = p->tm_mon + 1;
	Lock88_Msg.Time[2] = p->tm_mday;
	Lock88_Msg.Time[3] = p->tm_hour;
	Lock88_Msg.Time[4] = p->tm_min;
	Lock88_Msg.Time[5] = p->tm_sec;
//        b = 12;
//	len = b;
//	b += 2;	//消息体长度 预留2字节

	Lock88_Msg.data[len++] = dzm[0];
	Lock88_Msg.data[len++] = dzm[1];
	Lock88_Msg.data[len++] = dzm[2];
	for (a = 0; a < lock_LockCar_State_Len; a++)
		Lock88_Msg.data[len++] = lock_LockCar_State_Ram[a];
	len += gps_format_jwd_mem(Lock88_Msg.data + len);

	lock_hours = lock_total_work_hours;
	Lock88_Msg.data[len++] = lock_hours >> 16;
	Lock88_Msg.data[len++] = lock_hours >> 8;
	Lock88_Msg.data[len++] = lock_hours;
	u8 len1[4] = { 0 };
	sprintf((char*) len1, "%04X", (unsigned int) len); //消息体长度
	api_AscToHex(len1, Lock88_Msg.length, strlen((char*) len1));
	Lock88_Msg.MsgType = 0x88;
	// queue_in(&Lock88_Msg); //保存信息到队列中
	if (debug_value & 0x80) {
		printf("<88_Msg>message ready in queue %d bytes!!! \n", len + 14);
		api_PrintfHex(Lock88_Msg.data, len);
	}
	SaveLockCarData();
}

/**
 实现: 锁解车申请 :0x86
 dzm[3] : 立即锁信息对照码
 */
void LockCarSendApplyt(u8 *dzm) {
	u8 a;
	u16 len = 0;
	u32 lock_hours;

	time_t now_time;
	struct tm *p;
	u32 Sys_Run_Time;        //系统开机后的运行时间---局部变量

	QUE_TDF_QUEUE_MSG Lock86_Msg;
	Lock86_Msg.MsgType = 0x86;
	Lock86_Msg.Version[0] = Version.Protocol >> 8;
	Lock86_Msg.Version[1] = Version.Protocol;
	Lock86_Msg.Version[2] = Version.Code;
	//属性标识  高字节在前
	Lock86_Msg.Attribute[0] = 0x40;
	Lock86_Msg.Attribute[1] = 0;
	//获取当前时间 = 系统运行时间+ key on时间（RTC时间）
	Sys_Run_Time = api_GetSysSecs();
	now_time = Sys_Start_Time + Sys_Run_Time;
	p = localtime(&now_time);
	Lock86_Msg.Time[0] = p->tm_year - 100;
	Lock86_Msg.Time[1] = p->tm_mon + 1;
	Lock86_Msg.Time[2] = p->tm_mday;
	Lock86_Msg.Time[3] = p->tm_hour;
	Lock86_Msg.Time[4] = p->tm_min;
	Lock86_Msg.Time[5] = p->tm_sec;
	//        b = 12;
	//	len = b;
	//	b += 2;	//消息体长度 预留2字节

	Lock86_Msg.data[len++] = dzm[0];
	Lock86_Msg.data[len++] = dzm[1];
	Lock86_Msg.data[len++] = dzm[2];
	for (a = 0; a < lock_LockCar_Applyt_Len; a++)
		Lock86_Msg.data[len++] = lock_LockCar_Applyt[a];
	len += gps_format_jwd_mem(Lock86_Msg.data + len);

	lock_hours = lock_total_work_hours;
	Lock86_Msg.data[len++] = lock_hours >> 16;
	Lock86_Msg.data[len++] = lock_hours >> 8;
	Lock86_Msg.data[len++] = lock_hours;

	u8 len1[4] = { 0 };
	sprintf((char*) len1, "%04X", (unsigned int) len); //消息体长度
	api_AscToHex(len1, Lock86_Msg.length, strlen((char*) len1));
	Lock86_Msg.MsgType = 0x86;
	// queue_in(&Lock86_Msg); //保存信息到队列中
	if (debug_value & 0x80) {
		printf("<86_Msg>message ready in queue %d bytes!!! \n", len + 14);
		api_PrintfHex(Lock86_Msg.data, len);
	}
	lock_SendApplyt_Timer = api_GetSysmSecs();
}
/*
 实现：比较两个数组的大小
 Len ： 数组长度
 data1>data2 返回1
 data1<data2 返回2
 data1=data2 返回0
 */
u8 LockCmpData(u8 *data1, u8 *data2, u16 Len) {
	u16 a;
	u8 i = 0;
	for (a = 0; a < Len; a++) {
		if (data1[a] > data2[a])
			i = 1;
		if (data1[a] < data2[a])
			i = 2;
		if (i != 0)
			return (i);
	}
	return (i);
}

/*
 mima 终端接收到的密码
 lockdata  终端锁车条件
 lockdatalen  终端锁车条件对应对照码做在位置
 zhishi  锁车密码指示

 renturn 0：密码正确，解车成功
 1：密码错误，解车失败
 */
u8 Lock_TakeMiMa1_Lock(u8 *mima, u8 *lockdata, u8 lockdatalen, u8 zhishi) {
	u8 a, b;
	u8 c2, c3, c4;
	u8 Xx;
	a = lockdata[0] & 0x0F;
	a = a << 2;
	c2 = lockdata[lockdatalen] ^ a;
	c3 = lockdata[lockdatalen + 1] ^ c2;
	c4 = lockdata[lockdatalen + 2] ^ c3;

	// wy!103705 密码方案变更
	Xx = lock_RtcTime[0] * lock_RtcTime[1];
	c2 = c2 ^ Xx;
	c3 = c3 ^ Xx;
	c4 = c4 ^ Xx;

	a = c2 >> 4;
	a = a % 10;
	b = a << 4;
	a = c2 & 0x0F;
	a = a % 10;
	b = b | a;
	if (b != mima[0])
		return (1);

	a = c3 >> 4;
	a = a % 10;
	b = a << 4;
	a = c3 & 0x0F;
	a = a % 10;
	b = b | a;
	if (b != mima[1])
		return (1);

	a = c4 >> 4;
	a = a % 10;
	b = a << 4;
	b = b | zhishi;
	if (b != mima[2])
		return (1);
	lockdata[0] = 0;
	lockdata[lockdatalen] = 0xFF;
	lockdata[lockdatalen + 1] = 0xFF;
	lockdata[lockdatalen + 2] = 0xFF;
	if (zhishi == 0)
		lock_MimaUnlock_Imme = 1;
	if (zhishi == 1)
		lock_MimaUnlock_Hour = 1;
	if (zhishi == 2)
		lock_MimaUnlock_Time = 1;
	if (zhishi == 3)
		lock_MimaUnlock_Posi = 1;
	lock_lockjudge_timer = 0;
	return (0);
}
/*
 mima 终端接收到的密码
 lockdata  终端锁车条件
 lockdatalen  终端锁车条件对应对照码做在位置
 zhishi  锁车密码指示

 renturn 0：密码正确，解车成功
 1：密码错误，解车失败
 */
u8 Lock_TakeMiMa2_Lock(u8 *mima, u8 *lockdata, u8 lockdatalen, u8 zhishi) {
	u8 a, b, s;
	u8 c2, c3, c4;
	u8 y, m;

	a = lockdata[0] & 0x0F;
	a = a << 2;
	c2 = lockdata[lockdatalen] ^ a;
	c3 = lockdata[lockdatalen + 1] ^ c2;
	c4 = lockdata[lockdatalen + 2] ^ c3;

	a = mima[1] & 0x0F;
	a = a * 10;
	b = mima[0] & 0x0F;
	s = a + b;
	if (s == 0 || s > 48)
		return (1);

	y = lockdata[1];
	m = lockdata[2];
	s--;
	if (lockdata[3 + s] == 0xFD || lockdata[3 + s] == 0xFF)
		return (1);

	m = m + s;
	while (m > 12) {
		m = m - 12;
		y++;
	}
	y = y * m;		// wy103701.3 修改计算过程错误
	c2 = c2 ^ y;
	c3 = c3 ^ y;
	c4 = c4 ^ y;

	s++;
	a = c2 >> 4;
	a = a % 10;
	b = a << 4;
	a = s % 10;
	b = b | a;
	if (b != mima[0])
		return (1);

	a = c3 >> 4;
	a = a % 10;
	b = a << 4;
	a = s / 10;
	b = b | a;
	if (b != mima[1])
		return (1);

	a = c4 >> 4;
	a = a % 10;
	b = a << 4;
	b = b | zhishi;
	if (b != mima[2])
		return (1);

	s--;
	if (lockdata[3 + s] == 0xFE)		//已锁车
	                {
		lockdata[3 + s] = 0xFD; // 103701.3
	} else                    // 未锁车
	{
		lockdata[3 + s] = 0xFF; // 103701.3
	}
	lock_MimaUnlock_Date = 1;
	lock_lockjudge_timer = 0;
	return (0);
}

u8 Lock_TakeMiMa3_Bijiao(u8 *a, u8 *mima, u8 zhishi, u8 b, u8 c) {
	u8 mi[3];
	u8 i, j;

	for (i = 0; i < 2; i++) {
		mi[i] = b ^ a[i];
		j = mi[i] >> 4;
		j = j % 10;
		j = j << 4;
		mi[i] = mi[i] & 0x0F;
		mi[i] = mi[i] % 10;
		mi[i] = mi[i] | j;
	}
	mi[2] = b ^ a[2];
	j = mi[2] >> 4;
	j = j % 10;
	j = j << 4;
	mi[2] = j | zhishi;
	if (mi[2] == mima[2] && mi[1] == mima[1] && mi[0] == mima[0])
		return (0);

	for (i = 0; i < 2; i++) {
		mi[i] = c ^ a[i];
		j = mi[i] >> 4;
		j = j % 10;
		j = j << 4;
		mi[i] = mi[i] & 0x0F;
		mi[i] = mi[i] % 10;
		mi[i] = mi[i] | j;
	}
	mi[2] = mi[0] ^ mi[1];
	j = mi[2] >> 4;
	j = j % 10;
	j = j << 4;
	mi[2] = j | zhishi;

	if (mi[2] == mima[2] && mi[1] == mima[1] && mi[0] == mima[0])
		return (2);

	return (1);
}

/*
 mima 终端接收到的密码
 imei 终端对应imei，16进制
 imsi 终端对应imsi，16进制
 time 实时时间
 zhishi  锁车密码指示

 renturn 0：密码正确，解车成功
 1：密码错误，解车失败
 */
#if 0
u8 Lock_TakeMiMa3_Lock(u8 *mima,u8 *imei, u8 *imsi,u8 *time,u8 *SerialNum,u8 zhishi)
{
	u8 i;
	u8 a[3];
	u8 b,c;
	u8 mi[3];

	a[0]=imei[0];
	a[1]=imsi[0];
	for(i=1;i<8;i++)
	{
		a[0]=a[0]^imei[i];
		a[1]=a[1]^imsi[i];
	}
	a[2]=time[0]^time[1];
	//a[2]=a[2]^time[2]; // wy!103705 密码方案变更
	switch(zhishi)
	{
		case 8: //gsm
		b=imei[0]+imsi[0]+a[2];
		c=imei[0]^imsi[0];
		i=Lock_TakeMiMa3_Bijiao(a,mima,zhishi,b,c);
		if(i==0)//当次解车
		{
			lock_MimaUnlock_GsmAnt=1;
			return(0);
		}
		if(i==2) //功能关闭
		{
			time_GsmAntennaLockCarOn=1;
			lock_save_center_set_flg = 1;
			return(0);
		}
		//gps
		b=imei[1]+imsi[1]+a[2];
		c=imei[1]^imsi[1];
		i=Lock_TakeMiMa3_Bijiao(a,mima,zhishi,b,c);
		if(i==0)//当次解车
		{
			lock_MimaUnlock_GpsAnt=1;
			return(0);
		}
		if(i==2) //功能关闭
		{
			time_GpsAntennaLockCarOn=1;
			lock_save_center_set_flg = 1;
			return(0);
		}
		break;
		case 7: //sim
		b=imei[2]+imsi[2]+a[2];
		c=imei[2]^imsi[2];
		i=Lock_TakeMiMa3_Bijiao(a,mima,zhishi,b,c);
		if(i==0)//当次解车
		{
			lock_MimaUnlock_Sim=1;
			return(0);
		}
		if(i==2) //功能关闭
		{
			time_SIMBCLockCarOn=1;
			lock_save_center_set_flg = 1;
			return(0);
		}
		break;
		case 5: //handshake
		for(i=0;i<2;i++)
		{
			mi[i]=SerialNum[i]+a[i];
			b=mi[i]>>4;
			b=b%10;
			b=b<<4;
			mi[i]=mi[i]&0x0F;
			mi[i]=mi[i]%10;
			mi[i]=mi[i]|b;
		}
		mi[2]=SerialNum[2]+SerialNum[3];
		b=mi[2]>>4;
		b=b%10;
		b=b<<4;
		mi[2]=b|zhishi;
		if(mi[2]==mima[2] && mi[1]==mima[1] && mi[0]==mima[0]) //功能关闭
		{
			cand_GetData_Ver[0]=0;
			cand_GetData_Ver[1]=0;
			lock_save_center_set_flg = 1;
			return(0);
		}
		break;
		case 6: //系列号
		b=imei[4]+imsi[4]+a[2];
		c=imei[4]^imsi[4];
		i=Lock_TakeMiMa3_Bijiao(a,mima,zhishi,b,c);
		if(i==0)//当次解车
		{
			lock_MimaUnlock_Xilie=1;
			return(0);
		}
		if(i==2) //功能关闭
		{
			lock_MimaUnlock_Xilie=1;
			cand_DecoderExist_NG=1;
			cand_PiPeiStatus=0;
			lock_save_center_set_flg = 1;
			return(0);
		}
		break;
		default:
		break;
	}
	return(1);
}
#endif

/*
 生成handshake密码
 mima 生成密码出口
 imei 终端对应imei，16进制
 imsi 终端对应imsi，16进制
 SerialNum 终端对应系列号
 */
void Lock_TakeHandShakeMima(u8 *mima, u8 *imei, u8 *imsi) {
	u8 i;
	u8 a[3];
	u8 b;
	u8 B3;

	// wy!103705 密码方案变更
	a[2] = lock_RtcTime[0] ^ lock_RtcTime[1]; // A3=Yx^Mx
	B3 = imei[2] ^ imsi[2] ^ a[2];          // B3=E3^S3^A3

	a[0] = imei[0];
	a[1] = imsi[0];
	for (i = 1; i < 8; i++) {
		a[0] = a[0] ^ imei[i];
		a[1] = a[1] ^ imsi[i];
	}

	for (i = 0; i < 2; i++) {
		mima[i] = B3 ^ a[i]; // wy!103705 密码方案变更 B1^A1 B2^A2
		b = mima[i] >> 4;
		b = b % 10;
		b = b << 4;
		mima[i] = mima[i] & 0x0F;
		mima[i] = mima[i] % 10;
		mima[i] = mima[i] | b;
	}
	mima[2] = B3 ^ a[2]; // wy!103705 密码方案变更 B3^A3
	b = mima[2] >> 4;
	b = b % 10;
	b = b << 4;
	mima[2] = b | 0x05;
}

u8 Lock_HandleMiMaUnlock(u8 *data) {
	u8 a, b;
	u8 *lockdata;
	//u8 time[6];

	a = data[2] & 0x0F;
	switch (a)
	{
	case 0: //立即锁解车密码
		b = lock_Lock_Imme_Data_EveryLen - 3;
		lockdata = lock_Lock_Imme_Data_Ram;
		if (Lock_TakeMiMa1_Lock(data, lockdata, b, a) == 0)
			return (a);
		lockdata = lockdata + lock_Lock_Imme_Data_EveryLen;
		if (Lock_TakeMiMa1_Lock(data, lockdata, b, a) == 0)
			return (a);
		lockdata = lockdata + lock_Lock_Imme_Data_EveryLen;
		if (Lock_TakeMiMa1_Lock(data, lockdata, b, a) == 0)
			return (a);
		break;
	case 1: //工作小时锁解车密码
		b = lock_Lock_Hour_Data_EveryLen - 3;
		lockdata = lock_Lock_Hour_Data_Ram;
		if (Lock_TakeMiMa1_Lock(data, lockdata, b, a) == 0)
			return (a);
		lockdata = lockdata + lock_Lock_Hour_Data_EveryLen;
		if (Lock_TakeMiMa1_Lock(data, lockdata, b, a) == 0)
			return (a);
		lockdata = lockdata + lock_Lock_Hour_Data_EveryLen;
		if (Lock_TakeMiMa1_Lock(data, lockdata, b, a) == 0)
			return (a);
		break;
	case 2: //指定时间锁解车密码
		b = lock_Lock_Time_Data_EveryLen - 3;
		lockdata = lock_Lock_Time_Data_Ram;
		if (Lock_TakeMiMa1_Lock(data, lockdata, b, a) == 0)
			return (a);
		lockdata = lockdata + lock_Lock_Time_Data_EveryLen;
		if (Lock_TakeMiMa1_Lock(data, lockdata, b, a) == 0)
			return (a);
		lockdata = lockdata + lock_Lock_Time_Data_EveryLen;
		if (Lock_TakeMiMa1_Lock(data, lockdata, b, a) == 0)
			return (a);
		break;
	case 3: //位置锁解车密码
		b = lock_Lock_Posi_Data_EveryLen - 3;
		lockdata = lock_Lock_Posi_Data_Ram;
		if (Lock_TakeMiMa1_Lock(data, lockdata, b, a) == 0)
			return (a);
		lockdata = lockdata + lock_Lock_Posi_Data_EveryLen;
		if (Lock_TakeMiMa1_Lock(data, lockdata, b, a) == 0)
			return (a);
		lockdata = lockdata + lock_Lock_Posi_Data_EveryLen;
		if (Lock_TakeMiMa1_Lock(data, lockdata, b, a) == 0)
			return (a);
		break;
	case 4: //循环日期锁锁解车密码
		b = lock_Lock_Date_Data_EveryLen - 3;
		lockdata = lock_Lock_Date_Data_Ram;
		if (Lock_TakeMiMa2_Lock(data, lockdata, b, a) == 0)
			return (a);
		lockdata = lockdata + lock_Lock_Date_Data_EveryLen;
		if (Lock_TakeMiMa2_Lock(data, lockdata, b, a) == 0)
			return (a);
		lockdata = lockdata + lock_Lock_Date_Data_EveryLen;
		if (Lock_TakeMiMa2_Lock(data, lockdata, b, a) == 0)
			return (a);
		break;
	case 5: //Handshake锁解车密码

		//break;
	case 6: //系列号锁解车密码

		//break;
	case 7: //SIM卡锁解车密码

		//break;
	case 8: //天线锁解车密码

		//Sys_RTC_Get(time);
		//if(Lock_TakeMiMa3_Lock(data,IMEI,CIMI,time,cand_GetData_SerialNum,a)==0) return(a);
		break;
	default:

		break;
	}
	return (0xFF);
}

/**
 * @brief  写入指定格式数据
 * @param  buf :写入缓冲区
 * @param  data:数据
 * @param  type:类型，1：8位，2：16位，3：24位，4：32位
 * @retval None
 */
uint16_t Sys_Save_Buffer(uint8_t *buf, uint32_t data, uint8_t type) {
	uint16_t index = 0;

	if (type == 1) {
		buf[index++] = data & 0xFF;
	} else if (type == 2) {
		buf[index++] = data >> 8;
		buf[index++] = data & 0xFF;
	} else if (type == 3) {
		buf[index++] = data >> 16;
		buf[index++] = data >> 8;
		buf[index++] = data & 0xFF;
	} else if (type == 4) {
		buf[index++] = data >> 24;
		buf[index++] = data >> 16;
		buf[index++] = data >> 8;
		buf[index++] = data & 0xFF;
	}
	return index;
}
/**
 * @brief  读取指定格式数据
 * @param  buf :写入缓冲区
 * @param  data:数据
 * @param  type:类型，1：8位，2：16位，3：24位，4：32位
 * @retval 结果
 */
uint32_t sys_Read_Buffer(uint8_t *buf, uint8_t type) {
	uint32_t a = 0;

	if (type == 1) {
		a = buf[0];
	} else if (type == 2) {
		a = buf[0];
		a = a << 8 | buf[1];
	} else if (type == 3) {
		a = buf[0];
		a = a << 8 | buf[1];
		a = a << 8 | buf[2];
	} else if (type == 4) {
		a = buf[0];
		a = a << 8 | buf[1];
		a = a << 8 | buf[2];
		a = a << 8 | buf[3];
	}
	return a;
}

/**
 * @ 保存锁车相关设置及状态
 */
#define PutDownData(to, from, setoff, size) memcpy(to+setoff, from, size)
#define PutDownChar(to, param, setoff) Sys_Save_Buffer(to+setoff, param, 1)
#define PutDownInt(to, param, setoff) Sys_Save_Buffer(to+setoff, param, 2)
void SaveLockCarData(void) {
	u16 a;
	u16 crc;
	u8 *da;
//	u8 err;

	da = (u8 *) malloc(sizeof(u8) * 512);
	printf("Start Save LockCarPar\n");

//	OSMutexPend(MAINTEMP_mutex, 0, &err);
	a = 0;
	da[a++] = LockCarPar_Sign;
	//286字节
	PutDownData(da, lock_Lock_Imme_Data_Ram, a, lock_Lock_Imme_Data_Len);
	a = a + lock_Lock_Imme_Data_Len;			//12
	PutDownData(da, lock_Lock_Hour_Data_Ram, a, lock_Lock_Hour_Data_Len);
	a = a + lock_Lock_Hour_Data_Len;			//18
	PutDownData(da, lock_Lock_Posi_Data_Ram, a, lock_Lock_Posi_Data_Len);
	a = a + lock_Lock_Posi_Data_Len;			//48
	PutDownData(da, lock_Lock_Time_Data_Ram, a, lock_Lock_Time_Data_Len);
	a = a + lock_Lock_Time_Data_Len;			//30
	PutDownData(da, lock_Lock_Date_Data_Ram, a, lock_Lock_Date_Data_Len);
	a = a + lock_Lock_Date_Data_Len;			//162
	PutDownData(da, lock_LockCar_State_Ram, a, lock_LockCar_State_Len);
	a = a + lock_LockCar_State_Len;			//16
	da[a++] = lock_level;

	crc = api_CheckCrc(0xFFFF, da, save_LockCarDataLen + 1);
	PutDownInt(da, crc, a);
	a = a + 2;
	save2flash(Lock_file, da, save_LockCarDataLen + 3);
	if (debug_value & 0x80) {
		printf("lockdata save\n");
		api_PrintfHex(da, save_LockCarDataLen + 3);
	}
}

/**
 * @ 读取锁车相关设置及状态
 */
#define TakeOutData(from, to, setoff, size) memcpy(to, from+setoff, size)
#define TakeOutInt(from, setoff) sys_Read_Buffer(from+setoff, 2)
void TakeLockCarData(void) {
	u16 a;
	u16 crc;
	u8 *da;

	printf("Start Take LockCarPar\n");
	da = (u8 *) malloc(sizeof(u8) * 512);

//	OSMutexPend(MAINTEMP_mutex, 0, &err);

	getData(Lock_file, da); //读文件
	if (da[0] == LockCarPar_Sign) {
		a = api_CheckCrc(0xFFFF, da, save_LockCarDataLen + 1);
		crc = TakeOutInt(da, save_LockCarDataLen + 1);
		if (a == crc) {
			printf("LockCarPar E0 CRC OK\n");
		} else {
			da[0] = 0xFF;
			printf("LockCarPar E0 CRC Err\n");
		}
	} else {
		printf("No LockCarPar E0\n");
	}
	if (da[0] == LockCarPar_Sign) {
		a = 1;
		//286字节
		TakeOutData(da, lock_Lock_Imme_Data_Ram, a, lock_Lock_Imme_Data_Len);
		a = a + lock_Lock_Imme_Data_Len;			//12
		TakeOutData(da, lock_Lock_Hour_Data_Ram, a, lock_Lock_Hour_Data_Len);
		a = a + lock_Lock_Hour_Data_Len;			//18
		TakeOutData(da, lock_Lock_Posi_Data_Ram, a, lock_Lock_Posi_Data_Len);
		a = a + lock_Lock_Posi_Data_Len;			//48
		TakeOutData(da, lock_Lock_Time_Data_Ram, a, lock_Lock_Time_Data_Len);
		a = a + lock_Lock_Time_Data_Len;			//30
		TakeOutData(da, lock_Lock_Date_Data_Ram, a, lock_Lock_Date_Data_Len);
		a = a + lock_Lock_Date_Data_Len;			//162
		TakeOutData(da, lock_LockCar_State_Ram, a, lock_LockCar_State_Len);
		a = a + lock_LockCar_State_Len;			//16
		lock_level = da[a++];

//		OSMutexPost(MAINTEMP_mutex);

		printf("Take LockCarPar0 OK:");
//		puth(da, save_LockCarDataLen + 3);
		printf("\n");

		return;
	}
//	OSMutexPost(MAINTEMP_mutex);

}
