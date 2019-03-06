#ifndef  __TIME_H__
#define  __TIME_H__

#include "general.h"

extern void rtc_set_alarm(struct tm *rtc_tm);
extern void rtc_read_alarm();
extern ulong SystemTimeRead(DateTime *Time);
extern unsigned int RTCTimeWrite(u8 *time);
extern ulong RTCTimeRead(DateTime *Time);
extern void RtcDevOpen();
extern time_t Get_Now_Time();
extern int rtc_clr_alarm(void);

#endif
