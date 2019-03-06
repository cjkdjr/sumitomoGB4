
#ifndef  _GPSDEAL_H__
#define _GPSDEAL_H__

#include "general.h"
typedef enum {
	GPS_GSM = 0,
	GPS_QIANXUN,
	GPS_QIANXUN_110 ,
	GPS_QIANXUN_120,
} gps_type;

typedef enum {
	gps_state_off, gps_state_run, gps_state_on,
} gps_state;
gps_state gps_cur_state;

typedef struct{
	u8 utch[6];					//HEX,YYMMDDhhmmss,GMT+8
	u8 status;						//定位状态 0未定位 1已定位
	u8 starnum;				//可视星数 1字节
	u8 NS:1;						//1南0北
	u32 latitude:31;			//纬度 百万分之一度
	u8 EW:1;						//0东1西
	u32  longitude:31;		//经度 百万分之一度
	u16 speed;					//速度 0.1Km/h=(knots*1.85*10)
    u16 direction;				//地面航向 0~360度
	u8 GPSNEW:1;			//1-发回GPS数据是新值；0-发回GPS数据是旧值
	u16 height:14;			//高度，可能是负值，-30000~+30000米  含正负指示
	u16 hdop;					//水平精度因子，(0.5~99.9)*10
	u8 ptnstatus;				//天线状态
	u8 status_qx;				//千寻定位状态
}GPS_INFO;

extern int gps_fd;

unsigned char GPS_GPRMC_BUF[100];
unsigned char GPS_GPGGA_BUF[100];

extern void* gps_Pthread(void * arg);
extern  int gps_getCommaIndex(int num, unsigned char *str);

#endif






