/*********************************************
 * filename	:time.c
 *describe	:read system time&system run time
 *
 **********************************************/

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>

#include "Time.h"
#include "alarm.h"
#include "api.h"

#define RTC_port "/dev/rtc1"

int rtc_clr_alarm(void) {
	int fd = -1;
	int retval = -1;

	fd = open(RTC_port, O_RDONLY);
	if (fd < 0) {
		printf( "open rtc port faild!\n");
		close(fd);
		return -1;
	}

	retval = ioctl(fd, RTC_AIE_ON);
	close(fd);
	if (retval < 0) {
		printf( "ioctl  RTC_AIE_ON faild!\n");
		return -1;
	}
	return 0;
}

void rtc_set_alarm(struct tm *rtc_tm) {
	int retval = 0;

	int fd = open(RTC_port, O_RDONLY);
	if (fd < 0) {
		printf( "open rtc faild!!!\n");
		goto end;
	}

	retval = ioctl(fd, RTC_ALM_SET, rtc_tm);
	if (retval < 0) {
		printf( "ioctl RTC_ALM_SET  faild!!!");
		goto end;
	}
	/* Enable alarm interrupts */
	usleep(1000);
	retval = ioctl(fd, RTC_AIE_ON);
	if (retval < 0) {
		printf( "ioctl  RTC_AIE_ON faild!!!\n");
		goto end;
	}
	end: close(fd);
}

/****************************************************
 *funcname :SystemTimeRead
 *describe		:read system time
 *
 ******************************************************/
ulong SystemTimeRead(DateTime *Time) {

	struct tm p = { 0 };
	time_t timep;
	time(&timep);
	localtime_r(&timep, &p);
// printf("\n\nsystem time read:%02d-%02d-%02d %02d:%02d:%02d\n", p.tm_year-100, p.tm_mon+1, p.tm_mday, p.tm_hour, p.tm_min, p.tm_sec);
	Time->Year = p.tm_year - 100;
	;
	Time->Month = p.tm_mon + 1;
	Time->Day = p.tm_mday;
	Time->Hour = p.tm_hour;
	Time->Min = p.tm_min;
	Time->Sec = p.tm_sec;
	if ((Time->Year > 99 || Time->Year < 0) || (Time->Month > 12 || Time->Month <= 0) || (Time->Day > 31 || Time->Day <= 0) || ((Time->Hour > 24) || (Time->Hour < 0)) || ((Time->Min > 60) || (Time->Min < 0)) || ((Time->Sec > 60) || (Time->Sec < 0))) {
		Time->Valid = 0;
	} else {
		Time->Valid = 1;
	}

	return timep;
}
/****************************************************
 * 设置rtc 时间
 *
 *****************************************************/
u8 RTCTimeWrite_ok_flag = 0;  // 0:error  1:ok
u8 RTCTimeRead_ok_flag = 0;
unsigned int RTCTimeWrite(u8 *time) {
	int RTC_Fd = open(RTC_port, O_RDONLY);
	if (RTC_Fd < 0) {
		printf( "RTCTimeWrite : open rtc faild!!!\n");
		goto end;
	}
	int retval = -1;
	struct rtc_time rtc_tm;
	rtc_tm.tm_year = time[0] + 2000 - 1900;
	rtc_tm.tm_mon = time[1] - 1;
	rtc_tm.tm_mday = time[2];
	rtc_tm.tm_hour = time[3];
	rtc_tm.tm_min = time[4];
	rtc_tm.tm_sec = time[5];

	retval = ioctl(RTC_Fd, RTC_SET_TIME, &rtc_tm);
	if (retval < 0) {
		if (!AlarmIsBitH(AlarmIdRTC)) {
			printf( "<%s>: RTC write error, Alarm Set!!!!!!!!!!\n", __FUNCTION__);
		}
		AlarmSetBit(AlarmIdRTC);

		RTCTimeWrite_ok_flag = 0;
		printf( "ioctl RTC_SET_TIME  faild!!!");
//		System.DaySummary.RTCSetErrCount++;
	} else {
		RTCTimeWrite_ok_flag = 1;
		if ((RTCTimeRead_ok_flag == 1) && (RTCTimeWrite_ok_flag == 1)) {
			if (AlarmIsBitH(AlarmIdRTC)) {
				printf( "<%s>: RTC write ok, Alarm Clr!!!!!!!!!!\n", __FUNCTION__);
			}
			AlarmClrBit(AlarmIdRTC);
		}

	}
	end: close(RTC_Fd);
	return 0;
}

void RtcDevOpen() {
	int fd = open(RTC_port, O_RDONLY);
	if (fd < 0) {
		printf( "open rtc faild!!!\n");
	} else {
		printf( "RTC_fd:%d!\n", fd);
	}
	close(fd);

}
/***************************************************
 * funcname	:RTCTimeRead
 * describe		:read RTC time and save
 ***************************************************/
#if 0
u8 ct3000_SysDefaultTime[] = {0x0f, 0x01, 0x01, 0x01, 0x01, 0x01};
ulong RTCTimeRead(DateTime *Time)
{
	int RTC_Fd = 0;
	struct tm ptm;
	time_t sTm = 0;
	struct rtc_time rtc_tm;
	u8 *rtcbuf = NULL;
	/* Read the RTC time/date */
	int ret1 = -1;
	RTC_Fd = open(RTC_port, O_RDONLY);
	if ( RTC_Fd < 0 ) {
		perror("open");
		printf("open rtc faild!!!\n");
		goto end;
	}
	ret1 = ioctl(RTC_Fd, RTC_RD_TIME, &rtc_tm);
	if ( ret1 < 0 ) {
		RTCTimeRead_ok_flag = 0;
		if ( !AlarmIsBitH(AlarmIdRTC) ) {
			printf("<%s>: RTC read  error, Alarm Set!!!!!!!!!!\n", __FUNCTION__);
		}
		AlarmSetBit(AlarmIdRTC);
		printf("ioctl RTC_RD_TIME  faild!!!");
	}
	else {
		RTCTimeRead_ok_flag = 1;
		if ( (RTCTimeRead_ok_flag == 1) && (RTCTimeWrite_ok_flag == 1) ) {
			if ( AlarmIsBitH(AlarmIdRTC) ) {
				printf("<%s>: RTC write ok, Alarm Clr!!!!!!!!!!\n", __FUNCTION__);
			}
			AlarmClrBit(AlarmIdRTC);
		}
	}

	if( rtc_tm.tm_year + 1900 > 2000 ) {
		System.RTCTime.Year = (rtc_tm.tm_year + 1900) - 2000;
		System.RTCTime.Month = rtc_tm.tm_mon+1;
		System.RTCTime.Day = rtc_tm.tm_mday;
		System.RTCTime.Hour = rtc_tm.tm_hour;
		System.RTCTime.Min = rtc_tm.tm_min;
		System.RTCTime.Sec = rtc_tm.tm_sec;
		System.RTCTime.Valid = 1;

		Time->Year = System.RTCTime.Year;
		Time->Month = System.RTCTime.Month;
		Time->Day = System.RTCTime.Day;
		Time->Hour = System.RTCTime.Hour;
		Time->Min = System.RTCTime.Min;
		Time->Sec = System.RTCTime.Sec;
		Time->Valid = System.RTCTime.Valid = 1;
		printf("RTCTimeRead: %02d-%02d-%02d  %02d:%02d:%02d\n", Time->Year, Time->Month, Time->Day, Time->Hour, Time->Min, Time->Sec);
		System.RTCStoreTime.Year = System.RTCTime.Year;
		System.RTCStoreTime.Month = System.RTCTime.Month;
		System.RTCStoreTime.Day = System.RTCTime.Day;
		System.RTCStoreTime.Hour = System.RTCTime.Hour;
		System.RTCStoreTime.Min = System.RTCTime.Min;
		System.RTCStoreTime.Sec = System.RTCTime.Sec;
		System.RTCStoreTime.Valid = 1;
	}
	else {
		if( System.RTCStoreTime.Year <= 0 ) {
			printf("[time] time before 2000,set ct3000 default :");
			hprintf( ct3000_SysDefaultTime, sizeof(ct3000_SysDefaultTime));
			api_SetSystemTime(ct3000_SysDefaultTime);
			api_SetRtcTime(ct3000_SysDefaultTime);
			RTCTimeWrite(ct3000_SysDefaultTime);
			Time->Valid = System.RTCTime.Valid = 0;
		}
		else {
			rtcbuf[0] = System.RTCTime.Year = System.RTCStoreTime.Year;
			rtcbuf[1] = System.RTCTime.Month = System.RTCStoreTime.Month;
			rtcbuf[2] = System.RTCTime.Day = System.RTCStoreTime.Day;
			rtcbuf[3] = System.RTCTime.Hour = System.RTCStoreTime.Hour;
			rtcbuf[4] = System.RTCTime.Min = System.RTCStoreTime.Min;
			rtcbuf[5] = System.RTCTime.Sec = System.RTCStoreTime.Sec;
			printf("[general] time abnormal,set storetime:");
			hprintf( rtcbuf, sizeof(rtcbuf));
//			printf("\n");
			api_SetSystemTime(rtcbuf);
			api_SetRtcTime(rtcbuf);
			Time->Valid = System.RTCTime.Valid = 0;
		}
	}
	printf("RTCTimeRead: %02d-%02d-%02d  %02d:%02d:%02d\n",
			Time->Year, Time->Month, Time->Day,Time->Hour, Time->Min, Time->Sec);

	ptm.tm_year = Time->Year+100;
	ptm.tm_mon = Time->Month;
	ptm.tm_mday = Time->Day;
	ptm.tm_hour = Time->Hour;
	ptm.tm_min = Time->Min;
	ptm.tm_sec = Time->Sec;
	sTm = mktime(&ptm);
	end :close(RTC_Fd);
	RTC_Fd = 0;
	return sTm;

}
#endif
time_t Get_Now_Time() {
	struct tm *p;
	time_t timep;
	time(&timep);
	p = localtime(&timep);
	printf( "%d/%d/%d ", p->tm_year - 100, p->tm_mon + 1, p->tm_mday);
	printf( "%d:%d:%d \n", p->tm_hour, p->tm_min, p->tm_sec);
	return timep;
}
