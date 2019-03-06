/*
 * sys_manage.c
 *
 *  Created on: 2017年12月4日
 *      Author: tykj
 */

#include <linux/watchdog.h>
#include <sys/reboot.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>

#include "alarm.h"
#include "sys_manage.h"
#include "e2p.h"
#include "gsm.h"
#include "TCPsocket.h"
#include "Time.h"
#include "api.h"
#include "iap.h"
#include "message_process.h"
#include "gpio.h"
#include "gps_4g.h"
#include "UpgradeWrite.h"
#include "iap_queue.h"
#include "general.h"
#include "gpio.h"
#include "candriver.h"
#include "Msg_task.h"

struct FifoQueue Queue;

unsigned char ct100D_overday_flag = 0;
u8 photo_overday_flag = 0;
u8 ACC_off_Pos_Change_flag = 0;
u8 mount_usrfs = 0; //挂载用户文件系统标志位

int led_handle(u8 num, u8 state);

sys_state system_state = sys_state_start;

time_t ACC_On_time = 0;
weakup_t weakup_param;

#define BSP_LED1_PIN "203"
#define BSP_LED2_PIN "204"
#define BSP_LED3_PIN "205"
#define BSP_LED4_PIN "206"

void BSP_LED_Off(int num)
{
	switch (num)
	{
	case 1:
		gpio_SetModeValue(BSP_LED1_PIN, "high");
		break;
	case 2:
		gpio_SetModeValue(BSP_LED2_PIN, "high");
		break;
	case 3:
		gpio_SetModeValue(BSP_LED3_PIN, "high");
		break;
	case 4:
		gpio_SetModeValue(BSP_LED4_PIN, "high");
		break;
	default:
		break;
	}
}

void BSP_LED_On(int num)
{
	switch (num)
	{
	case 1:
		gpio_SetModeValue(BSP_LED1_PIN, "high");
		break;
	case 2:
		gpio_SetModeValue(BSP_LED2_PIN, "high");
		break;
	case 3:
		gpio_SetModeValue(BSP_LED3_PIN, "high");
		break;
	case 4:
		gpio_SetModeValue(BSP_LED4_PIN, "high");
		break;
	default:
		break;
	}
}
#define BSP_ACC_PIN "174"
#define BSP_RTC_C_PIN "225"
#define BSP_BAT_AD_EN_PIN "140"

void Li_BAT_AD_enable(void)
{

	gpio_SetModeValue(BSP_BAT_AD_EN_PIN, "high");

}

void Li_BAT_AD_disable(void)
{

	gpio_SetModeValue(BSP_BAT_AD_EN_PIN, "low");

}
#define  BSP_VBAT_DIR "/sys/bus/iio/devices/iio:device0/in_voltage7_raw"
#define BSP_VBack_DIR "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
#define BSP_AD1_DIR "/sys/bus/iio/devices/iio:device0/in_voltage1_raw"
#define BSP_AD2_DIR "/sys/bus/iio/devices/iio:device0/in_voltage3_raw"
#define BSP_AD3_DIR "/sys/bus/iio/devices/iio:device0/in_voltage5_raw"
#define BSP_AD4_DIR "/sys/bus/iio/devices/iio:device0/in_voltage4_raw"

int BSP_ADC_VALUE_READ(int num)
{

	FILE *fd = NULL;
	char a[10];
//    int x,y;
	int x;
//    int adc;

	switch (num)
	{
	case 1:
		fd = fopen(BSP_VBAT_DIR, "r+");
		//        y=11;
		break;

	case 2:

		fd = fopen(BSP_VBack_DIR, "r+");
		Li_BAT_AD_enable();
		usleep(100 * 1000);
		//        y=8;
		break;

	case 3:
		fd = fopen(BSP_AD1_DIR, "r+");
//        y=11;
		break;

	case 4:
		fd = fopen(BSP_AD2_DIR, "r+");
//        y=11;
		break;

	case 5:
		fd = fopen(BSP_AD3_DIR, "r+");
//        y=11;
		break;
	case 6:
		fd = fopen(BSP_AD4_DIR, "r+");
		break;
	default:
		return 0;
	}

	fscanf(fd, "%s", a);

	if (strlen(a) == 4)
	{
		x = ((int) a[0] - 48) * 1000 + ((int) a[1] - 48) * 100 + ((int) a[2] - 48) * 10 + ((int) a[3] - 48);
	} else if (strlen(a) == 3)
	{
		x = ((int) a[0] - 48) * 100 + ((int) a[1] - 48) * 10 + ((int) a[2] - 48);
	} else if (strlen(a) == 2)
	{
		x = ((int) a[0] - 48) * 10 + ((int) a[1] - 48);
	} else
	{
		x = ((int) a[0] - 48);
	}

//    adc=x*2500/4096*y;

	Li_BAT_AD_disable();
	fclose(fd);

	return x;
}

#define BSP_SW1_DIR "/sys/bus/iio/devices/iio:device0/in_voltage2_raw"

int BSP_SW_STATE_READ(int num)
{
#if 0
	static int fd_dev;
	char value;

	switch (num)
	{

		case 1:
		fd_dev = open(BSP_SW1_DIR, O_RDWR);
		if(fd_dev < 0)
		{
			perror(BSP_SW1_DIR);
			goto fail;
		}
		read(fd_dev, &value, 1);

		value -= '0';
		break;

		case 2:
		fd_dev = open(BSP_SW2_DIR, O_RDWR);
		if(fd_dev < 0)
		{
			perror(BSP_SW2_DIR);
			goto fail;
		}
		read(fd_dev, &value, 1);
		break;

		default:
		return 0;

		fail:
		if(fd_dev)
		close(fd_dev);
	}
	return value;
#endif
	return 0;
}

void BSP_LED_Toggle(int num)
{
	switch (num)
	{
	case 1:
		if (gpio_GetValue(BSP_LED1_PIN)) {
			gpio_SetModeValue(BSP_LED1_PIN, "low");
		}else {
			gpio_SetModeValue(BSP_LED1_PIN, "high");
		}
		break;
	case 2:
		if (gpio_GetValue(BSP_LED2_PIN)) {
			gpio_SetModeValue(BSP_LED2_PIN, "low");
		}else {
			gpio_SetModeValue(BSP_LED2_PIN, "high");
		}
		break;
	case 3:
		if (gpio_GetValue(BSP_LED3_PIN)) {
			gpio_SetModeValue(BSP_LED3_PIN, "low");
		}else {
			gpio_SetModeValue(BSP_LED3_PIN, "high");
		}
		break;
	case 4:
		if (gpio_GetValue(BSP_LED4_PIN)) {
			gpio_SetModeValue(BSP_LED4_PIN, "low");
		}else {
			gpio_SetModeValue(BSP_LED4_PIN, "high");
		}
		break;
	default:
		break;
	}
}

/**************************************************
 * 车钥匙状态检测
 **************************************************/
uint8_t check_acc_start(void) {
	uint8_t i;
	uint8_t count = 0;

	for (i = 0; i < 10; i++) {
		if (gpio_GetValue(BSP_ACC_PIN) == 0) {
			if (count >= 5)
				break;
			else
				count++;
		}
		usleep(20 * 1000); //20ms
	}

	if (i >= 10) {
		System.AccState = 0;
		//printf_gsm("AccState:%d\n",System.AccState);
		return 0;
	} else {
		if (System.AccState == 0) {
			System.DaySummary.AccOnCount++; //当天车钥匙接通次数
		}
		System.AccState = 1;
		//printf_gsm("AccState:%d\n",System.AccState);
		return 1;
	}

}
/***********************************************
 *清除当天终端数据
 *
 ***********************************************/
int ClearDaySummary(void) {
	memset(&System.DaySummary, 0x00, sizeof(System.DaySummary));

	return (1);
}
/**********************************************
 * 过天状态检测
 *
 * *********************************************/
const char run_nian[13] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
const char ping_nian[13] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
int msg_OverdayHandle(void) {
	DateTime timeNow = { 0 };

	GetNowTime(&timeNow);
	if (timeNow.Valid == 0) {
//		printf_gsm("msg_OverdayHandle:timeNow.Valid == 0\n");
		return -1;
	}
	if (((timeNow.Year % 4 == 0) && (timeNow.Day > run_nian[timeNow.Month - 1])) || ((timeNow.Year % 4 != 0) && (timeNow.Day > ping_nian[timeNow.Month - 1]))) {
		return -1;
	} else if (0 <= timeNow.Hour && timeNow.Hour < 24 && 0 <= timeNow.Min && timeNow.Min < 60 && 0 <= timeNow.Sec && timeNow.Sec < 60 && timeNow.Month >= 1 && timeNow.Month <= 12) {
		//记录历史时间，用于对比是否过天
		if ((System.Record.Year == 0) && (System.Record.Month == 0) && (System.Record.Day == 0)) {
			GetNowTime(&(System.Record));
			//         msg_OverdayInit();
			return 0;
		}
		//判断过天  过天清除部分数据
		if ((System.Record.Year != timeNow.Year) || (System.Record.Month != timeNow.Month) || (System.Record.Day != timeNow.Day)) {
//			printf_sys("System.Record.time = %d-%d-%d %d:%d:%d \n", System.Record.Year, System.Record.Month, System.Record.Day, System.Record.Hour, System.Record.Min, System.Record.Sec);
//			printf_sys("timeNow = %d-%d-%d %d:%d:%d \n", timeNow.Year, timeNow.Month, timeNow.Day, timeNow.Hour, timeNow.Min, timeNow.Sec);
//			printf_sys("!!!!!!!!!overday!!!!!!!!!!!! \n");
			ct100D_overday_flag = 1;
			photo_overday_flag = 1;
			GetNowTime(&(System.Record));
			save_total_param();
			//ClearDaySummary();
			return 1;
		}
		//更新记录时间
		if ((System.Record.Hour < 12) && (timeNow.Hour >= 12)) {
			GetNowTime(&(System.Record));
			return 3;
		}
	}
	return 0;
}
/*
 * 函数功能：	获取系统看门狗复位寄存器的值
 * 参数：		void
 * 返回值：		是看门狗复位返回1，不是看门狗复位返回0，函数出错返回-1
 * */
#define NUC970_RSTSTS_GET 1
int GetWDTREST(void) {
	int fd = -1;
	char * dev_name = "/dev/nuc970_sys";
	int value;

	fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
	if (-1 == fd) {
		printf("Cannot open '%s': %d, %s/n", dev_name, errno, strerror(errno));
		return -1;
	}

	ioctl(fd, NUC970_RSTSTS_GET, &value);
	close(fd);
	// 是由看门狗复位的
	if ((value & 0x20) != 0) {
		return 1;
	} else {
		return 0;
	}
}
/*
 读e2p中系统参数
 */
void init_total_param(void) {
	u32 E2p_addr = 0X26; //起始地址
	int offset = 0; //地址偏移量
	u8 CheckSum = 0;
	u16 length = 0;
//	e2p_read_page(0X22, (uint8_t*) &Alarming.terminal, 2); //报警标志位ing
//	e2p_read_page(0X24, (uint8_t*) &Alarmed.terminal, 2); //报警标志位ed
//	printf_sys("Alarming.terminal:%2x Alarmed.terminal:%2x \n", Alarming.terminal, Alarmed.terminal);

//	e2p_read_page(0x26, (uint8_t*) &gsm_info_cur.status, 1); //gsm当前状态
//	if (gsm_info_cur.status == 0xFF)
//		gsm_info_cur.status = 0;
//	printf("gsm_info_cur.status:%d\n", gsm_info_cur.status);

    length = sizeof(StayMinute);
    e2p_read_page(E2p_addr + offset, (uint8_t*) &StayMinute, length); //休眠三小时发送待机信息
    offset += length;
    e2p_read_page(E2p_addr + offset, &CheckSum, 1); 
    offset += 1;
    if (CheckSum != api_CheckSum((uint8_t*) &StayMinute, length))
        memset(&StayMinute, 0, length);

	length = sizeof(gsm_info_cur);
	e2p_read_page(E2p_addr + offset, (uint8_t*) &gsm_info_cur, length); //gsm当前状态
	offset += length;
	e2p_read_page(E2p_addr + offset, &CheckSum, 1); //
	offset += 1;
	if (CheckSum != api_CheckSum((uint8_t*) &gsm_info_cur, length))
		memset(&weakup_param, 0, length);
//	printf("gsm_info_cur.status:%d,checksum:%d,gsm_info_cur length:%d,E2p_addr:%4x\n", gsm_info_cur.status, CheckSum, length, E2p_addr);
//	printf("%d\n", gsm_info_cur.csq);
//	printf("%04x\n", gsm_info_cur.cid);
//	printf("%04x\n", gsm_info_cur.lac);
//	api_PrintfHex(gsm_info_cur.imei, 8);
//	api_PrintfHex((uint8_t*) &gsm_info_cur, length);

	length = sizeof(weakup_param);
//	printf("weakup_param length:%d\n", length);
	e2p_read_page(E2p_addr + offset, (uint8_t*) &weakup_param, length); //唤醒参数
	offset += length;
	e2p_read_page(E2p_addr + offset, &CheckSum, 1); //唤醒参数校验码
	offset += 1;
	if (CheckSum != api_CheckSum((uint8_t*) &weakup_param, length))
		memset(&weakup_param, 0, length);
//	api_PrintfHex((uint8_t*) &weakup_param, length);

	length = sizeof(System.DaySummary);
	e2p_read_page(E2p_addr + offset, (uint8_t*) &System.DaySummary, length); //过天统计参数
	offset += length;
	e2p_read_page(E2p_addr + offset, &CheckSum, 1); //过天统计参数校验码
	offset += 1;
	if (CheckSum != api_CheckSum((uint8_t*) &System.DaySummary, length))
		memset(&System.DaySummary, 0, length);

	length = sizeof(SYS_SET2);
	e2p_read_page(E2p_addr + offset, (uint8_t*) &SYS_SET2, length); //终端设置参数
	offset += length;
	e2p_read_page(E2p_addr + offset, &CheckSum, 1); //终端设置参数校验码
	offset += 1;
//	printf("SYS_SET.Stagnation_sTime:%d;SYS_SET.Stagnation_R = %d\n", SYS_SET.Stagnation_sTime,SYS_SET.Stagnation_R);
	if (CheckSum != api_CheckSum((uint8_t*) &SYS_SET2, length))
		memset(&SYS_SET2, 0, length);

	length = sizeof(System.Record);
	e2p_read_page(E2p_addr + offset, (uint8_t*) &System.Record, length); //过天使用
	offset += length;
	e2p_read_page(E2p_addr + offset, &CheckSum, 1); //过天记录参数校验码
	offset += 1;
	if (CheckSum != api_CheckSum((uint8_t*) &System.Record, length))
		GetNowTime(&(System.Record));
}
/*
 写系统参数到e2p中
 */
void save_total_param(void) {
//	e2p_write_page(0x22, (uint8_t*) &Alarming.terminal, 2); //报警标志位
//	e2p_write_page(0x24, (uint8_t*) &Alarmed.terminal, 2);
//	e2p_write_page(0x26, (uint8_t*) &gsm_info_cur.status, 1); //gsm当前状态参数1字节
//	printf("gsm_info_cur.status:%d\n",gsm_info_cur.status);
	u32 E2p_addr = 0X26; //起始地址
	int offset = 0; //地址偏移量
	u8 CheckSum = 0;
	u16 length = 0;
	
	length = sizeof(StayMinute);
    e2p_write_page(E2p_addr + offset, (uint8_t*) &StayMinute, length);
    offset += length;
    CheckSum = api_CheckSum((uint8_t*) &StayMinute, length);
    e2p_write_page(E2p_addr + offset, &CheckSum, 1); 
    offset += 1;


	length = sizeof(gsm_info_cur);
	e2p_write_page(E2p_addr + offset, (uint8_t*) &gsm_info_cur, length); //gsm参数20Bytes
	offset += length;
	CheckSum = api_CheckSum((uint8_t*) &gsm_info_cur, length);
	e2p_write_page(E2p_addr + offset, &CheckSum, 1); //唤醒参数校验码
	offset += 1;
//	printf("gsm_info_cur.status:%d,checksum:%d,gsm_info_cur length:%d,E2p_addr:%4x\n", gsm_info_cur.status, CheckSum, length, E2p_addr);
//	printf("%d\n", gsm_info_cur.csq);
//	printf("%04x\n", gsm_info_cur.cid);
//	printf("%04x\n", gsm_info_cur.lac);
//	api_PrintfHex(gsm_info_cur.imei, 8);
//	api_PrintfHex((uint8_t*) &gsm_info_cur, length);

	length = sizeof(weakup_param);
//	printf("weakup_param length:%d\n", length);
	e2p_write_page(E2p_addr + offset, (uint8_t*) &weakup_param, length); //唤醒参数14bytes
	offset += length;
	CheckSum = api_CheckSum((uint8_t*) &weakup_param, length);
	e2p_write_page(E2p_addr + offset, &CheckSum, 1); //唤醒参数校验码
	offset += 1;
//	api_PrintfHex((uint8_t*) &weakup_param, length);

	length = sizeof(System.DaySummary);
	e2p_write_page(E2p_addr + offset, (uint8_t*) &System.DaySummary, length); //过天统计参数40字节
	offset += length;
	CheckSum = api_CheckSum((uint8_t*) &System.DaySummary, length);
	e2p_write_page(E2p_addr + offset, &CheckSum, 1); //过天统计参数校验码
	offset += 1;

	length = sizeof(SYS_SET2);
	e2p_write_page(E2p_addr + offset, (uint8_t*) &SYS_SET2, length); //系统参数152字节
	offset += length;
	CheckSum = api_CheckSum((uint8_t*) &SYS_SET2, length);
	e2p_write_page(E2p_addr + offset, &CheckSum, 1); //系统参数校验码
	offset += 1;

	length = sizeof(System.Record);
	e2p_write_page(E2p_addr + offset, (uint8_t*) &System.Record, length); //过天时间7Bytes
	offset += length;
	CheckSum = api_CheckSum((uint8_t*) &System.Record, length);
	e2p_write_page(E2p_addr + offset, &CheckSum, 1); //
	offset += 1;
}

/*****************************************************以下为测试部分*********************************************************/
//void clean_E2P()
//{
//	memset(&photo_info, 0, sizeof(photo_info));
//	save_total_param();
//}

void sys_init(void) {
//	DateTime T = { 0 };
//	ACC_On_time = RTCTimeRead(&T);
//	printf("ACC_On_time(RTC):%d/%d/%d-%d:%d:%d\n",T.Year, T.Month, T.Day,T.Hour, T.Min, T.Sec);
	init_total_param(); //读e2p中系统参数
	system_state = sys_state_start;
}

//判断是否在一个圆形区域中
//c 圆心 r 半径， p 指定点
//r 单位是坐标。
u8 In_Circular(uint32_t weidu1, uint32_t jingdu1, uint32_t weidu2, uint32_t jingdu2, u16 R) {
	u32 pl_dist = 0;

	pl_dist = Fun_Distance_jw(weidu1, jingdu1, weidu2, jingdu2); //判断点到圆心的距离

	if (pl_dist <= R) {                           //如果在园内
		return 1;                                //返回成功
	} else {
//		printf_sys("pl_dist = %d\n", pl_dist);
		return 0;                                //否则返回失败
	}
}

void power_off(void) {
	int i, ret = 0;
	u32 L = 0;
	time_t timep0 = 0;
	time_t timep1 = 0;
	time(&timep0);
	struct tm timep2 = *gmtime(&timep0);
	struct tm timep3;
	printf_sys("cur time(LTC):  %s\r\n", asctime(&timep2));
	if (weakup_param.time_interval_set != 20) {
		weakup_param.time_interval_set = 20;
		printf_sys("use the default weakup time interval 20 min\r\n");
	}
//	printf("the weakup time interval %d min\n", weakup_param.time_interval_set);
	if (System.GPS.Valid == 1) {
//		L = Fun_Distance_jw(System.GPS.Position.Latitude, System.GPS.Position.Longitude, weakup_param.latitude, weakup_param.longitude);
		printf_sys("L=%ld\n", (long int ) L);
	}
	/*	printf("System.GPS.Position.Latitude:%ld\n", (u32) System.GPS.Position.Latitude);
	 printf("System.GPS.Position.Longitude:%ld\n", (u32) System.GPS.Position.Longitude);
	 printf("weakup_param.latitude:%ld\n", (u32) weakup_param.latitude);
	 printf("weakup_param.longitude:%ld\n", (u32) weakup_param.longitude);

	 //判断时间间隔是否大于180分钟，距离是否大于200m
	 if ( (ACC_On_time - weakup_param.time) > SYS_SET.SET_OFF_SENDTIME * 60 || L >= 200 || System.DaySummary.WakeupCount == 1 ) {
	 weakup_param.time = timep;
	 printf("Line:%d,weakup_param.time:%ld\n", __LINE__, (u32) weakup_param.time);
	 if ( System.GPS.Valid == 1 ) {
	 weakup_param.latitude = System.GPS.Position.Latitude;
	 weakup_param.longitude = System.GPS.Position.Longitude;
	 }
	 }
	 */
//	if (System.GPS.Valid == 1 && L >= SYS_SET.SET_OFF_RANGE) {
	if (System.GPS.Valid == 1) {
//		weakup_param.latitude = System.GPS.Position.Latitude;
//		weakup_param.longitude = System.GPS.Position.Longitude;
	}
	timep1 = timep0 + (weakup_param.time_interval_set * 60); //时间间隔设置单位是分钟
//	timep1 = timep0 + (5* 60); //时间间隔设置单位是分钟
	timep1 = (timep1 / 60) * 60 + 60;
//	printf("wakeup time: %s\n", asctime(gmtime(&timep1)));
	//闹钟支持的最小分辨率为分钟，为了保证唤醒时能够确定唤醒源，EEPROM保存的唤醒时间需要和闹钟设置值一致
	if (timep1 <= timep0) {
		timep1 = timep0 + (weakup_param.time_interval_set * 60); //时间间隔设置单位是分钟
		timep1 = (timep1 / 60) * 60 + 60;
	}
	timep3 = *gmtime(&timep1);
//	struct tm *setTime = gmtime(&timep1);
//	printf("wakeup time: %s\n", asctime(gmtime(&timep1)));
	//写e2p保存统计参数
	save_total_param();
	sleep(1);

	rtc_set_alarm(&timep3);
	for (i = 0; i < 3; i++) {
		usleep(100);
		ret = gpio_GetValue(BSP_RTC_C_PIN);
		if (ret == 0) { //闹钟已响，需要关闭闹钟重新设置
			printf_sys("set RTC alarm failed %d times\n", i);
			usleep(100);
			rtc_set_alarm(&timep3);
		} else
			break;
	}
}
/*
 *
 *工作状态控制
 */
time_t KeyOnTimeLast;
u16 PowerOffTimeLen = 90;
int CtrlWorkSta(void) {
	if (System.AccState == 0 && System.GPS.Valid == 1) {
//		u32 L = Fun_Distance_jw(System.GPS.Position.Latitude, System.GPS.Position.Longitude, weakup_param.latitude, weakup_param.longitude);
//		if (L >= SYS_SET.SET_OFF_RANGE) { //距离大于200米，需要立即生成一条位置信息
//			ACC_off_Pos_Change_flag = 1;
//		}
	}
	time_t nowTime = api_GetSysSecs();
	if ( System.AccState == 1 || (IAP.Update_Enable == 0x01) || ((System.GPS.Valid == 0) && (!IsWaitGpsOver()))) {
		main_WorkSta = WORK_RUN;
		KeyOnTimeLast = nowTime;
		PowerOffTimeLen = 30;
//		printf_gsm("CtrlWorkSta KEYON KeyOnTimeLast=%ld---------------\n", KeyOnTimeLast);
	} else {
//		if (rngbuf_count > 0) {
//			if (sqlite_count == 0 && rngbuf_count < 20)
//				PowerOffTimeLen = 30;
//			else
//				PowerOffTimeLen = 60;
//		}
		if (KeyOnTimeLast > nowTime) {
			KeyOnTimeLast = nowTime;
//			printf("KeyOnTimeLast:%ld\n", KeyOnTimeLast);
		} else if (nowTime - KeyOnTimeLast > PowerOffTimeLen) {
//			printf(" nowTime - KeyOnTimeLast=%ld\n", nowTime - KeyOnTimeLast);
			main_WorkSta = WORK_GSM_OFF;
		}
	}
	return (0);
}
uint16_t ad_cc[8];
/**************
 * 模拟量采集
 * ****************/
u8 SYS_ANALOG1[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
u8 SYS_ANALOG2[8];
u8 SYS_SWITCH[2];

/********************
 * 模拟量对应：
 * 模拟量1----电源电压
 * 模拟量2----后备电池电压
 * 模拟量3----硬件AD1(CAN1_H)
 * 模拟量4----硬件AD3(外接传感器)
 * *******************/
void ai_detection(void) {

//协议模拟量1-电源电压
	ad_cc[0] = BSP_ADC_VALUE_READ(1);
	SYS_ANALOG1[0] = ad_cc[0] >> 8;
	SYS_ANALOG1[1] = ad_cc[0];
//协议模拟量2-后备电池电压
	ad_cc[1] = BSP_ADC_VALUE_READ(2);
	SYS_ANALOG1[2] = ad_cc[1] >> 8;
	SYS_ANALOG1[3] = ad_cc[1];
	//协议模拟量3-硬件ad4(此处标注硬件ad量以原理图为准)
	ad_cc[2] = BSP_ADC_VALUE_READ(6);
	SYS_ANALOG1[4] = ad_cc[2] >> 8;
	SYS_ANALOG1[5] = ad_cc[2];
	//协议模拟量4-硬件ad3（传感器下排气AD值）
	ad_cc[3] = BSP_ADC_VALUE_READ(4);
	SYS_ANALOG1[6] = ad_cc[3] >> 8;
	SYS_ANALOG1[7] = ad_cc[3];
	if(ad_cc[3] > 1000){
		System.EngineState = 1;
	}
	else{
		System.EngineState = 0;
	}
//    printf("ad:%d %d %d %d %d\n",SYS_ANALOG1[0],SYS_ANALOG1[1],SYS_ANALOG1[2],SYS_ANALOG1[3],SYS_ANALOG1[4]);
//    printf("ad3:%d %d\n", SYS_ANALOG1[6] ,SYS_ANALOG1[7]);
}
/*****************
 * 开关量采集
 * ******************/
void di_detection(void) {
	uint8_t channel = 0x01;
	if (System.AccState == 1) //acc
	                {
		SYS_SWITCH[0] = 0x00;
	} else {
		SYS_SWITCH[0] = 0x80;
	}
	if (BSP_SW_STATE_READ(1)) //sw1
	                {
		SYS_SWITCH[1] |= channel << 7;
	}
	if (BSP_SW_STATE_READ(2)) //sw2
	                {
		SYS_SWITCH[1] |= channel << 6;
	}
	//    if(BSP_Charge_STATE_READ())					//charge
	//    {   SYS_SWITCH[1] |= channel <<7;    }
//    if(BSP_SW_STATE_READ(3))
//    {   SYS_SWITCH[1] |= channel <<4;    }   //sw3
//    if(BSP_SW_STATE_READ(4))
//    {   SYS_SWITCH[1]  |= channel <<3;    }   //sw4
}

int sys_poll() {
	//车钥匙状态检测
	check_acc_start();
//	pid_t status;
	time_t start = 0;
	time_t trun = 0;
	DateTime nowRtc;
	CtrlWorkSta();
	int ret = 1;
	static uint32_t ad_start_time = 0;
	u32 ad_now_time = 0;
	ad_now_time = api_GetSysmSecs();
	if ((ad_now_time - ad_start_time) >= 1000) { //1s
		ad_start_time = ad_now_time;
		//AD 检测
		ai_detection();
		//SW检测
		di_detection();
	}

	switch (system_state)
	{
	case sys_state_start:
		sys_PthParam.sta = 1;
		led_contrl();
		start = SystemTimeRead(&nowRtc);
		trun = api_GetSysSecs();
		Sys_Start_Time = start - trun;

		if (System.AccState == 1) {
			printf_sys("checked acc on !!,ready to enable GPS GSM moudle \n");
			GSMEnable();
		} else if (System.AccState == 0) {
		}
		if (mount_usrfs == 0) {
			mount_usrfs = 1;
			sleep(3);
			//初始化队列和数据库
			que_sqlite_open();
			Read_Para_set();
		}
		system_state = sys_state_run;
		break;
	case sys_state_run:
		sys_PthParam.sta = 1;
		led_contrl();
		if (IAP.Update_Enable == 0) {
			//过天检测
			msg_OverdayHandle();
		}
//		Stagnation_Func(); //开机滞留点判断
		if ((System.AccState == 0) /*&& (key_off_EC_sendflg == 0)*/) {
		}

		break;
	case sys_state_stop:
		sys_PthParam.sta = 1;
		can_PthParam.flag = FALSE;
		sleep(1);
		printf_sys("sys_state_stop:\n")
		;
		if (System.AccState == 1) {
			printf_sys("checked acc on !!,ready to enable GPS GSM moudle \n");
			system_state = sys_state_start;
			break;
		}

		GSMSleep();
		//保存信息队列，直接写入数据库
		queue_save();

		//车钥匙状态检测
		check_acc_start();
		if (System.AccState == 1) {
			printf_sys("checked acc on !!,ready to enable GPS GSM moudle \n");
			system_state = sys_state_start;
			break;
		}

		power_off(); //设置唤醒闹钟
		System.DaySummary.RSTResetartCount++; //当天正常关机次数
		ret = update_SystemCmd1("./etc/init.d/mount-opt", "stop");
		if (ret == 0) {
			mount_usrfs = 0;
		} else {
			usleep(100 * 1000);
		}
		pthread_mutex_destroy(&InterStaQuemutex);
		pthread_mutex_destroy(&can_vcumutex);
		ioctl(wtd_fd, WDIOC_KEEPALIVE, 0); //喂硬狗
		system("poweroff");
		sleep(10);
		break;

	case sys_state_save:
		sys_PthParam.sta = 1;
		can_PthParam.flag = FALSE;
		sleep(1);
//		mpu6050_disable();
		CAN_close();
//			sleep(1);
		//写e2p保存统计参数
		save_total_param();
		//保存信息队列，直接写入数据库
		queue_save();
		update_SystemCmd1("/etc/init.d/mount-opt", "stop");
		pthread_mutex_destroy(&InterStaQuemutex);
		pthread_mutex_destroy(&can_vcumutex);
		ioctl(wtd_fd, WDIOC_KEEPALIVE, 0); //喂硬狗
		//重启
		sync();
		return reboot(RB_AUTOBOOT);

		break;
	default:
		system_state = sys_state_run;
	}
	return 0;
}
/**************************
 *name: led_contrl()
 *LED与模块单元及模块运行状态对应
 * LED1:GPS-BLUE
 * LED2 :GSM-YELLOW
 * LED3:CAN-GREEN
 * LED4:POWER-RED
 *
 * ******************/
u8 led[5] = { 0, 0, 0, 0, 0 };
void led_contrl() {
//GPS 运行状态判断
	if (gps_fd < 0) {
		led[1] = 0;
	} else if (System.GPS.Valid == 1) { //定位有效
		led[1] = 2;
	} else {
		led[1] = 1;
	}
	led_handle(1, led[1]);
// GSM 运行状态判断
	if (GSMFd < 0) {
		led[2] = 0;
	} else if ((GSM_Sock_PPPFlg == 1) && (GPRS_flag == 1)) //有GPRS数据发送
	                {
		led[2] = 2;
	} else if (GPRS_flag == 2) {
		led[2] = 4;
	} else if (GSM_Sock_PPPFlg == 1) {
		led[2] = 3;
	} else {
		led[2] = 1;
	}
	led_handle(2, led[2]);
//CAN 运行状态判断
//	if (( AlarmIsBitH(AlarmIdCan1Trans) == 1) && (AlarmIsBitH(AlarmIdCan0Trans) == 1)) {
//		led[3] = 0;
//	} else if (( AlarmIsBitH(AlarmIdCan1Trans) == 0) && (AlarmIsBitH(AlarmIdCan0Trans) == 0)) {
//		led[3] = 2;
//	} else {
//		led[3] = 1;
//	}
	led_handle(3, led[3]);
//POWER
	if (System.AccState == 1) //key_on
	                {
		led[4] = 1;
	} else {
		led[4] = 0;
	}
	led_handle(4, led[4]);

}
u32 led1_start = 0;
u32 led2_start = 0;
u32 led3_start = 0;
u32 led4_start = 0;
u32 led_now = 0;
/**********************
 * LED：开机默认状态
 * ************************/
void default_led_contrl() {
	led1_start = api_GetSysmSecs();
	led2_start = api_GetSysmSecs();
	led3_start = api_GetSysmSecs();
	led4_start = api_GetSysmSecs();
	BSP_LED_Off(1);
	BSP_LED_Off(2);
	BSP_LED_Off(3);
	BSP_LED_On(4);
}
/**********************
 * LED：关机状态led off
 * ************************/
void led_contrl_off() {
	BSP_LED_Off(1);
	BSP_LED_Off(2);
	BSP_LED_Off(3);
	BSP_LED_Off(4);
}
/******************************************
 * led闪烁状态控制
 *
 ***********************************/
int led_handle(u8 num, u8 state) {
	switch (num)
	{
	case 1:
		if (state == 0) {
			BSP_LED_Off(num);
		} else if (state == 1) {
			led_now = api_GetSysmSecs();
			if ((led_now - led1_start) >= 100) {
				BSP_LED_Toggle(num);
				led1_start = led_now;
			}
		} else {
			BSP_LED_On(num);
		}
		break;
	case 2:
		if (state == 0) {
			BSP_LED_Off(num);
		} else if (state == 1) {
			led_now = api_GetSysmSecs();
			if ((led_now - led2_start) >= 100) {
				BSP_LED_Toggle(num);
				led2_start = led_now;
			}
		} else if (state == 2) {
			BSP_LED_On(num);
		} else if (state == 3) {
			led_now = api_GetSysmSecs();
			if ((led_now - led2_start) >= 500) {
				BSP_LED_Toggle(num);
				led2_start = led_now;
			}
		} else if (state == 4) {
			int i;
			for (i = 0; i < 4; i++) {
				led_now = api_GetSysmSecs();
				if ((led_now - led2_start) >= 100) {
					BSP_LED_Toggle(num);
					led2_start = led_now;
				}
			}
		}
		break;
	case 3:
		if (state == 0) {
			BSP_LED_Off(num);
		} else if (state == 1) {
			led_now = api_GetSysmSecs();
			if ((led_now - led3_start) >= 500) {
				BSP_LED_Toggle(num);
				led3_start = led_now;
			}
		} else if (state == 2) {
			BSP_LED_On(num);
		}
		break;
	case 4:
		if (state == 1) {
			led_now = api_GetSysmSecs();
			if ((led_now - led4_start) >= 100) {
				BSP_LED_Toggle(num);
				led4_start = led_now;
			}
		} else {
			BSP_LED_Off(num);
		}
		break;
	}

	return 0;
}

void *sys_Pthread(void *argv) {
	printf_sys("############## %s start ##############\n", __FUNCTION__);
	sys_PthParam.sta = 1;
//	dtu_uart_init();
	sys_PthParam.flag = TRUE;
	gpio_SetModeValue("225", "high");  //rtc_c
	gpio_SetModeValue("140", "low");     //Li_BAT_AD_EN
	gpio_SetModeValue(BSP_LED1_PIN, "high");
	gpio_SetModeValue(BSP_LED2_PIN, "high");
	gpio_SetModeValue(BSP_LED3_PIN, "high");
	gpio_SetModeValue(BSP_LED4_PIN, "high");
	while (sys_PthParam.flag) {
		sys_PthParam.sta = 1;
		sys_poll();
		usleep(100 * 1000);				 //延时100ms
	}
	return NULL;
}
