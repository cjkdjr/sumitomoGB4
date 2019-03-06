#ifndef  __GSM_H
#define  __GSM_H

#include <stdint.h>

#include "general.h"
#define PPP_DIAL 1
#define GSM_MODULE_4G 1
extern int gps_flag ;
extern int GSMFd;

typedef enum {
	state_off, state_run, state_on, state_sleep, state_debug,
} gsm_state;

extern int GSM_Sock_PPPFlg;
extern gsm_state gsm_cur_state;
extern int GSM_ATFaiCount;
extern int GSM_GpsFailCount;
#define SMS_SEND_MODE   0x01
#define GPRS_SEND_MODE  0x00

#define GSM_STATUS_CREG     0x01
#define GSM_STATUS_ANT      0x02
#define GSM_STATUS_DEF      0x04
#define GSM_STATUS_SLEEP      0x08
#define GSM_STATUS_PWR      0x10
#define GSM_STATUS_SER      0x20
#define GSM_STATUS_INT      0x40

typedef struct gsm_info {
	uint8_t running;
	uint8_t csq;
	u16 lac;
	u32 cid;
	uint8_t mytel[8];
	u8 imei_len;
	u8 imei[8];
	u8 imei_str[20];
	u8 imsi_len;
	u8 imsi[21];
	uint8_t status;         //<0>网络注册状态<1>天线状态<2>预留<3>预留<4>电源<5>串口<6>初始化
} gsm_info_t;

extern gsm_info_t gsm_info_cur;

extern unsigned char GSM_Csq_FLAG;
extern int gsm_CheckSim(void);
extern void GSMEnable(void);
extern void GSMSleep(void);
extern void GSMDisable(void);
extern int SleepGSM(void);
extern void GSM_init(void);
int gsm_check();
extern int ppp_connect(void);//拨号上网功能
extern void GSMpoll(void);
extern void CloseGSM(void);
extern int gsm_PPPoE(void);

//封装线程入口函数
extern void *gsm_Pthread(void *argv);
extern int gsm_SendATCmd(int fd, char *cmdStr, char * checkStr, int (*fun)(char *, char *),  int timeout);

#endif
