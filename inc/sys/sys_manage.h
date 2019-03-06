/*
 * sys_manage.h
 *
 *  Created on: 2017年12月4日
 *      Author: tykj
 */

#ifndef SYS_MANAGE_H_
#define SYS_MANAGE_H_
#include "general.h"

#define mount_pwd "/etc/init.d/mount-opt"
extern uint16_t ad_cc[8];
typedef enum {
	sys_state_start, sys_state_run, sys_state_rtc_wake, sys_state_gsm_wake, sys_state_stop, sys_state_save,
} sys_state;

extern u8 photo_overday_flag; //拍照过天检测标志位
extern sys_state system_state;

extern time_t KeyOnTimeLast; // 本次运行最后关机时间
#define IsWaitGpsOver() (KeyOnTimeLast > 90) //每次唤醒运行时间
enum {
	WORK_RUN = 0, WORK_ON_OFF, WORK_GSM_OFF, WORK_MCU_OFF, WORK_POWER_OFF
} main_WorkSta;

typedef struct weakup {
	uint32_t time; //存储首次关机时间，用于计算唤醒是否满180分钟
	uint16_t time_interval_set; //设定的时间间隔
	//休眠时工作区域，用于唤醒以后判断设备是否被移动
	uint32_t latitude; //经度
	uint32_t longitude; //纬度
//	uint32_t	T;              //存储首次关机时间，用于计算唤醒是否满180分钟
} weakup_t;

extern u8 SYS_ANALOG1[8];
extern u8 SYS_ANALOG2[8]; //模拟量
extern u8 SYS_SWITCH[2]; //开关量
extern u8 ACC_off_Pos_Change_flag;//用于唤醒时有定位时且超过200米，生成D4
extern void led_contrl();
extern unsigned char mount_usrfs;
extern unsigned char ct100D_overday_flag;
extern void save_total_param(void);
extern void sys_init(void); //系统参数初始化
extern void *sys_Pthread(void *argv); //sys线程入口函数
extern void default_led_contrl();

#endif /* SYS_MANAGE_H_ */
