#ifndef _GPS_H
#define _GPS_H

#include <stdint.h>

#include  "gpsdeal.h"

#define gps_num 10

struct  tm GPS_Time_Check[gps_num];
extern GPS_INFO GPSbuf;
extern u8 gps_time_chek(u8 *time);
extern int gps_receive();
extern int HexToOct(char *str);
extern int CheckXOR(unsigned char *str);
extern int gps_start();
extern int gps_open();
extern int gps_poweron();
extern int gps_poweroff();
extern int gps_stop();
extern void gps_close();
extern int gps_getCommaIndex(int num, unsigned char *str);
extern uint32_t Fun_Distance_jw(uint32_t weidu1, uint32_t jingdu1, uint32_t weidu2, uint32_t jingdu2);

#endif
