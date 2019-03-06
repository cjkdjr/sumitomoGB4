/*************************************
 *gps.c
 *
 *日期：2017-12-19
 ************************************ */

#include "serial.h"
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>

#include "general.h"
#include  "gps_4g.h"
#include "api.h"
#include "gpio.h"
#include "gsm.h"
#include "Time.h"
#include "alarm.h"
#include "sys_manage.h"
#include "Msg_task.h"

GPS_INFO GPSbuf;	// gps数据解析后保存的结构体
int gps_fd;

static char GPS_RtcSetFlg; // 设置RTC时间标志位 0本次开机未设置 1本次开机已设置
unsigned long gps_timestart, gps_timeend, gps_time_nolocate/*, gps_timeantcheck*/;
unsigned char gps_timeadjustflag = 0/*,gps_havesetflag,  gps_antcheckflag = 0*/;

#define BSP_GPS_PWR_PIN "10"
#define BSP_GPS_ANT_PIN "171"

void gps_antenna_monitor()
{
	int ret = 0;
	ret= 1 - gpio_GetValue(BSP_GPS_ANT_PIN);
	if(ret==0)
	{
		if( !AlarmIsBitH(AlarmIdGPSAnt) ) {
//			printf("IO_GpsantennaCheck:gps antenna ABNORMAL!\n");
		}
		AlarmSetBit(AlarmIdGPSAnt);
//		AlarmingSetBit(AlarmIdGPSAnt);
	}
	else
	{
		if( AlarmIsBitH(AlarmIdGPSAnt) ) {
//			printf("IO_GpsantennaCheck:gps antenna NORMAL!\n");
		}
		AlarmClrBit(AlarmIdGPSAnt);
//		AlarmingClrBit(AlarmIdGPSAnt);
	}
}

int gps_start() { //成功返回1，失败返回0
	int cnt = 3;
	while (cnt--) {
		if (gsm_SendATCmd(GSMFd, "AT+CGPS=1,1\r", "OK", api_FindStr, 5) == 0) {
			break;
		}
		GSM_ATFaiCount++;
		sleep(1);
	}
	if (cnt < 0)
		return 0;
	return 1;
}
int gps_stop() {
	if (GSMFd > 0)
		  gsm_SendATCmd(GSMFd, "AT+CGPS=0,1\r", "OK", api_FindStr, 5);
	return 0;
}

void gps_close() {
	if (gps_fd > 0) {
		close(gps_fd);
	}
}
int gps_open() { //成功返回1，失败返回0
	gps_close();
	int i = 30;
	int ret = 1;
	while ((gps_fd = open("/dev/ttyUSB1", O_RDWR | O_NOCTTY)) < 0) {
		printf("open ttyUSB1 error!\n");
		sleep(1);
		if ((i--) <= 0) {
			ret = 0;
			close(gps_fd);
			break;
		}
	}
	printf("open ttyUSB1 succ!\n");
	serial_Init(gps_fd, 115200, 8, 1, 'N');
	tcflush(gps_fd, TCIFLUSH);
	return ret;
}

int gps_poweron() {
	gpio_SetModeValue(BSP_GPS_PWR_PIN, "high");
	printf("gps_power on!\n");
	System.DaySummary.GPSOnCount++;
	return 1;
}

int gps_poweroff() {
	gpio_SetModeValue(BSP_GPS_PWR_PIN, "low");
	printf("gps_poweroff  quit!\n");
	return 0;
}

/*
 设置日期/时间
 linux系统设置系统时钟用date命令 ，保存硬件时间用hwclock命令
 格式为：date 062920502008.10 和 hwclock -w
 解释：系统时间设置为2008年6月29日20时50分10秒，然后用命令：hwclock -w保存 硬件时间和RTC时间一致
 */

const int Month_Days_Accu_C[13] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
const int Month_Days_Accu_L[13] = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };
#define SecsPerDay      (3600*24)

int Time_Regulate(char *time) {
	u32 Tmp_Year = 0xFFFF, Tmp_Month = 0xFF, Tmp_Date = 0xFF;
	u32 LeapY, ComY, TotSeconds, TotDays;
	u32 Tmp_HH = 0xFF, Tmp_MM = 0xFF, Tmp_SS = 0xFF;

	//32-bit counter at Second Unit--> 4*1024*1024(s) --> 49710(day) --> 136(year)
	Tmp_Year = 2000 + time[0];
	Tmp_Month = time[1];
	Tmp_Date = time[2];
	Tmp_HH = time[3];
	Tmp_MM = time[4];
	Tmp_SS = time[5];

	if (Tmp_Year == 2000) {
		LeapY = 0;
	} else {
		LeapY = (Tmp_Year - 2000 - 1) / 4 + 1;
	}

	ComY = (Tmp_Year - 2000) - (LeapY);

	if (Tmp_Year % 4) {
		TotDays = LeapY * 366 + ComY * 365 + Month_Days_Accu_C[Tmp_Month - 1] + (Tmp_Date - 1);
	} else {
		TotDays = LeapY * 366 + ComY * 365 + Month_Days_Accu_L[Tmp_Month - 1] + (Tmp_Date - 1);
	}

	TotSeconds = TotDays * SecsPerDay + (Tmp_HH * 3600 + Tmp_MM * 60 + Tmp_SS);

	return TotSeconds;
}

/****************************************
 * function name	:GPS_SetRTC   设置RTC&SYSTEM时间
 * describe				:每5分钟，取一次系统时间， 与gps时间进行比较。如果误差大于30秒。用gps时间校正
 *错误累加， 将错误次数，放到诊断信息的最后（校验和之前）
 * System.DaySummary.SysTimeResetCount:错误累加， 错误计数器
 *****************************************/

void GPS_SetRTC(void) {
	char Temp1[6];
	char Temp2[6];
//	char Temp3[6];
	DateTime Syst;
//	DateTime Rtct;
	if ((GPSbuf.status == 1) && (GPS_RtcSetFlg == 0)) {
		printf("GPS_SetRTC:case 1\n");
		GPS_RtcSetFlg = 1;
		api_SetSystemTime((u8 *) GPSbuf.utch);
		api_SetRtcTime((u8 *) GPSbuf.utch);
	}

	//system time check
	SystemTimeRead(&Syst);
//	RTCTimeRead(&Rtct);

	Temp1[0] = Syst.Year;
	Temp1[1] = Syst.Month;
	Temp1[2] = Syst.Day;
	Temp1[3] = Syst.Hour;
	Temp1[4] = Syst.Min;
	Temp1[5] = Syst.Sec;

	Temp2[0] = GPSbuf.utch[0];
	Temp2[1] = GPSbuf.utch[1];
	Temp2[2] = GPSbuf.utch[2];
	Temp2[3] = GPSbuf.utch[3];
	Temp2[4] = GPSbuf.utch[4];
	Temp2[5] = GPSbuf.utch[5];

	int ret1 = Time_Regulate(Temp1);
	int ret2 = Time_Regulate(Temp2);

	if ((ret1 - ret2 > 60 * 2) || (ret1 - ret2 < -60 * 2)) {
		printf("GPS_SetSystime:case 2\n");
		printf("GPStime: ");
		api_PrintfHex((u8 *) GPSbuf.utch, sizeof(GPSbuf.utch));
		printf("\n");
		api_SetSystemTime((u8 *) GPSbuf.utch);
		if (System.DaySummary.SysTimeResetCount < 255) {
			System.DaySummary.SysTimeResetCount++;
		}
		printf("GPS_SetSystime_Rest_count = %d\n", System.DaySummary.SysTimeResetCount);
	}
}


/*******************************************************************
 * function name	:gps_data_save
 *describe				:Save GPS valid location data
 *
 * ******************************************************************/
void gps_data_save(void) {

	System.GPS.Time.Year = GPSbuf.utch[0]; //date
	System.GPS.Time.Month = GPSbuf.utch[1];
	System.GPS.Time.Day = GPSbuf.utch[2];
	System.GPS.Time.Hour = GPSbuf.utch[3];
	System.GPS.Time.Min = GPSbuf.utch[4];
	System.GPS.Time.Sec = GPSbuf.utch[5];
	System.GPS.Position.Longitude = GPSbuf.longitude; //经度 百万分之一度
	System.GPS.Position.Latitude = GPSbuf.latitude; //纬度 百万分之一度
	System.GPS.Position.Altitude = GPSbuf.height; //高度，可能是负值，-30000~+30000米
	System.GPS.Flags = GPSbuf.status; //定位状态 0未定位 1已定位״̬
	System.GPS.StarNum = GPSbuf.starnum; //可视星数 1字节
	System.GPS.Azimuth = GPSbuf.direction; //地面航向 0~360度
	System.GPS.Speed = GPSbuf.speed; //速度 0.1Km/h=(knots*1.85*10)
	System.GPS.HoriPrec = GPSbuf.hdop; //水平精度因子，(0.5~99.9)*10
#if 0
	printf("time:[%d][%d][%d] [%d][%d][%d]  ", GPSbuf.utch[0], GPSbuf.utch[1], GPSbuf.utch[2], GPSbuf.utch[3], GPSbuf.utch[4], GPSbuf.utch[5]);
	printf("direction:[%d]==", GPSbuf.direction);
	printf("speed:[%d]==", GPSbuf.speed);
	printf("longitude:[%d]==", GPSbuf.longitude);
	printf("latitude:[%d]==\n", GPSbuf.latitude);
#endif

	if (GPSbuf.status == 1 && GPSbuf.latitude != 0 && GPSbuf.longitude != 0) {
		System.GPS.Position.Valid = 1;
		System.GPS.Valid = 1;
		memcpy(&GPSRight, &System.GPS, sizeof(GPSRight));
		if (System.GPS.Time.Valid == 1) {
			GPS_SetRTC();
		}
		GPSbuf.GPSNEW = 1;
		gps_time_nolocate = gps_timestart;
//		printf("GPS OK\n");
	} else {
		System.GPS.Time.Valid = 0;
		System.GPS.Position.Valid = 0;
		System.GPS.Valid = 0;
		GPSbuf.GPSNEW = 0;
	}
}

/*
 *功能：		将字符数组按照逗号进行分隔，找到指定第几个逗号后的字符数组下标
 *输入：		第几个逗号num，要查询的字符数组
 *返回值：	没有找到逗号返回0，找到指定num个逗号后的字符数组下标
 */
int gps_getCommaIndex(int num, unsigned char *str) {
	int i = 0, j = 0;
	const char *p = (const char*) (char*) str;
	int len = strlen(p);
	for (i = 0; i < len; i++) {
		if (',' == str[i]) {
			j++;
		}
		if (j == num) {
			return i + 1;
		}
	}
	return 0;
}

/****************************************
 * 名称:gps_time_chek
 * 功能:gps定位时间检测,过滤时间跳变现象
 * 返回值:0数据正确  -1:数据不正确
 */
static unsigned char num = 0;
time_t _t[gps_num];

u8 gps_time_chek(u8 *time) {
	unsigned char gps_time_error_flg = 0;
	int i, j, a[10] = { 0 };
	GPS_Time_Check[num].tm_year = time[0] + 100;	//年
	GPS_Time_Check[num].tm_mon = time[1] - 1;	//月
	GPS_Time_Check[num].tm_mday = time[2];	//日
	GPS_Time_Check[num].tm_hour = time[3];	//时
	GPS_Time_Check[num].tm_min = time[4];	//分
	GPS_Time_Check[num].tm_sec = time[5];	//秒
	_t[num] = mktime(&GPS_Time_Check[num]);
	//取十个gps时间点,初次定位时间点不足10个时,时间做无效处理不可设置系统时间
	if (num < 9) {
		num++;
		gps_time_error_flg++;
		return -1;
	} else if (num == 9){	//达到连续十个时间点时,时间点差,其中任意两个连续点差值大于2s,定位时间做无效处理
		for (i = 0; i < 9; i++) {
			a[i] = _t[num - i] - _t[num - (i + 1)];
			if (a[i] > 2) {
//				printf("System.GPS.Time.Valid = 0,diff = %d\n",a[i] );
				gps_time_error_flg++;
			}
		}
		//时间点移位
		for (j = 0; j < 9; j++) {
			_t[j] = _t[j + 1];
		}
	}
	//时间点无异常,gps时间有限
	if (gps_time_error_flg == 0) {
//		printf("System.GPS.Time.Valid =1   ok !!!\n");
		return 0;
	} else {
		return -1;
	}
}
/*
 *功能：		将含有RMC包头的数据进行解析，将获得的数据放到全局变量中保存
 *输入：		void
 *返回值：	void
 *样例： 		$GPRMC,091400.000,A,3958.9870,N,11620.3278,E,000.0,000.0,120302,005.6,W*62
 */
void ParseRMCDate(void) {
	int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
	unsigned int mon[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }; // mon[n]对应的第n月有多少天
	int index; // 通过逗号分隔符获取字符串后的下标
	int state;
	float ftmp;
	long lTmp;
	//------------------------------------------------
	// 数据状态 有效定位A 无效定位V
	index = gps_getCommaIndex(2, GPS_GPRMC_BUF);
	if ((GPS_GPRMC_BUF[index + 0] != ',') && (GPS_GPRMC_BUF[index + 0] != '*') && (GPS_GPRMC_BUF[index + 0] != ' ') && (index != 0)) {
		state = GPS_GPRMC_BUF[index + 0];
		if (state == 'A') {
			GPSbuf.status = 1;
		}
		if (state == 'V') {
			GPSbuf.status = 0;
		}
	}

	if (GPSbuf.status == 1) {

		//	日期时间 hhmmss.sss(时分秒)091400.000 ddmmyy(日月年)120302
		index = gps_getCommaIndex(1, GPS_GPRMC_BUF);
		if ((GPS_GPRMC_BUF[index] != ',') && (GPS_GPRMC_BUF[index] != '*') && (GPS_GPRMC_BUF[index] != ' ') && (index != 0)) {
			hour = (GPS_GPRMC_BUF[index + 0] - '0') * 10 + (GPS_GPRMC_BUF[index + 1] - '0');
			minute = (GPS_GPRMC_BUF[index + 2] - '0') * 10 + (GPS_GPRMC_BUF[index + 3] - '0');
			second = (GPS_GPRMC_BUF[index + 4] - '0') * 10 + (GPS_GPRMC_BUF[index + 5] - '0');
		}
		index = gps_getCommaIndex(9, GPS_GPRMC_BUF);
		if ((GPS_GPRMC_BUF[index + 0] != ',') && (GPS_GPRMC_BUF[index + 0] != '*') && (GPS_GPRMC_BUF[index + 0] != ' ') && (index != 0)) {
			day = (GPS_GPRMC_BUF[index + 0] - '0') * 10 + (GPS_GPRMC_BUF[index + 1] - '0');
			month = (GPS_GPRMC_BUF[index + 2] - '0') * 10 + (GPS_GPRMC_BUF[index + 3] - '0');
			year = (GPS_GPRMC_BUF[index + 4] - '0') * 10 + (GPS_GPRMC_BUF[index + 5] - '0');
		}
		// 闰年
		if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
			mon[2] = 29;
		}
		// 时间的数据符合逻辑
		if (second >= 0 && second < 60 && minute >= 0 && minute < 60 && hour >= 0 && hour < 60 && day >= 1 && day <= mon[month] && month >= 1 && month <= 12)
		{
			if ((SYS_SET2.Time_Zone_Config >> 15) == 0) {	// 转换成东八区
//				printf("SYS_SET.Time_Zone_Config:%d; %d\n",SYS_SET.Time_Zone_Config, SYS_SET.Time_Zone_Config/60);
				hour += ((SYS_SET2.Time_Zone_Config & 0x7FFF) / 60);
				if (hour >= 24) {
					hour -= 24;
					day++;
					if (day > mon[month]) {
						day -= mon[month];
						month++;
						if (month > 12) {
							month -= 12;
							year++;
							if (year > 99) {
								year = 00;
								System.GPS.Time.Valid = 0;
							}
						}
					}
				}
			} else if ((SYS_SET2.Time_Zone_Config >> 15) == 1) {
				hour -= ((SYS_SET2.Time_Zone_Config & 0x7FFF) / 60);
				if (hour < 0) {
					hour += 24;
					day--;
					if (day < 1) {
						day += mon[month];
						month--;
						if (month < 1) {
							month += 12;
							year--;
							if (year > 99) {
								year = 00;
								System.GPS.Time.Valid = 0;
							}
						}
					}
				}
			}
			GPSbuf.utch[0] = year;		// 年
			GPSbuf.utch[1] = month;		// 月
			GPSbuf.utch[2] = day;			// 日
			GPSbuf.utch[3] = hour;		// 时
			GPSbuf.utch[4] = minute;	// 分
			GPSbuf.utch[5] = second;	// 秒
			 if( gps_time_chek((u8 *) GPSbuf.utch)==0)
			    {
			        System.GPS.Time.Valid = 1;
			    }else {
			      System.GPS.Time.Valid = 0;
			    }
		} else {
			System.GPS.Time.Valid = 0;
		}

		// 纬度dddmm.mmmm(度分格式)3958.9870,度分秒格式3958.9870 / 100 取整数部分39度，
		// 3958.9870 - 39 * 100取整数部分58分，剩下的0.9870 * 60 取整数部分59秒
		index = gps_getCommaIndex(3, GPS_GPRMC_BUF);
		if (index != 0) {
			ftmp = atof((const char *) GPS_GPRMC_BUF + index);
			// dddmm.mmmm转换成0.0001分
			GPSbuf.latitude = (long) (ftmp / 100);
			GPSbuf.latitude *= 600000;
			lTmp = (long) (ftmp * 10000);
			lTmp = lTmp % 1000000;
			GPSbuf.latitude += lTmp;
		} else {
			GPSbuf.latitude = 0;
		}
		//printf("GPSbuf:latitude=%d\n", GPSbuf.latitude);
		//-------------------------------------------------------------
		// 纬度半球 北纬是0 南纬是1
		index = gps_getCommaIndex(4, GPS_GPRMC_BUF);
		if ((GPS_GPRMC_BUF[index] != ',') && (GPS_GPRMC_BUF[index] != '*') && (GPS_GPRMC_BUF[index] != ' ') && (index != 0)) {
			state = GPS_GPRMC_BUF[index];
			if (state == 'N')
				GPSbuf.NS = 0;
			if (state == 'S')
				GPSbuf.NS = 1;
		}
		//printf("GPSbuf:NS=%02X\n",GPSbuf.NS);
		//--------------------------------------------
		// 经度dddmm.mmmm(度分格式)11620.3278,度分秒格式11620.3278 / 100 取整数部分116度，11620.3278 - 116 * 100取整数部分20分
		// 剩下的0.3278 * 60 取整数部分19秒
		index = gps_getCommaIndex(5, GPS_GPRMC_BUF);
		if (index != 0) {
			ftmp = atof((const char *) GPS_GPRMC_BUF + index);
			// dddmm.mmmm转换成0.0001分
			GPSbuf.longitude = (long) (ftmp / 100);
			GPSbuf.longitude *= 600000;
			lTmp = (long) (ftmp * 10000);
			lTmp = lTmp % 1000000;
			GPSbuf.longitude += lTmp;
		} else {
			GPSbuf.longitude = 0;
		}
		//printf("GPSbuf:longitude=%d\n", GPSbuf.longitude);

		// 经度半球 东经是0 西经是1
		index = gps_getCommaIndex(6, GPS_GPRMC_BUF);
		if ((GPS_GPRMC_BUF[index + 0] != ',') && (GPS_GPRMC_BUF[index + 0] != '*') && (GPS_GPRMC_BUF[index + 0] != ' ') && (index != 0)) {
			state = GPS_GPRMC_BUF[index + 0];
			if (state == 'E')
				GPSbuf.EW = 0;
			if (state == 'W')
				GPSbuf.EW = 1;
		}
		//printf("GPSbuf:EW=%02X\n",GPSbuf.EW);


		// 速度 单位节 （海里/小时）000.0到999.9 转换为公里/小时需要乘以1.85
		index = gps_getCommaIndex(7, GPS_GPRMC_BUF);
		if (index != 0) {
			ftmp = atof((const char *) GPS_GPRMC_BUF + index);
			GPSbuf.speed = (int) (ftmp * 1852) / 1000;	// 海里转换为公里
		} else {
			GPSbuf.speed = 0.0;
		}

		//printf("GPSbuf:speed=%d[0.1Km]\n", GPSbuf.speed);
		// 地面航向 000.0到999.9度
		index = gps_getCommaIndex(8, GPS_GPRMC_BUF);
		if (index != 0) {
			GPSbuf.direction = atof((const char *) GPS_GPRMC_BUF + index);
		} else {
			GPSbuf.direction = 0.0;
		}
	}
}

/*
 *功能：		将含有GGA包头的数据进行解析，将获得的数据放到全局变量中保存
 *输入：		void
 *返回值：	void
 *样例：		$GPGGA,091400,3958.9870,N,11620.3278,E,1,03,1.9,114.2,M,-8.3,M,,*5E
 */
void ParseGGADate(void) {
	int index;
	float fTmp;

	// 使用卫星数量03 从00到12
	index = gps_getCommaIndex(7, GPS_GPGGA_BUF);
	if (index != 0) {
		GPSbuf.starnum = atoi((const char *) GPS_GPGGA_BUF + index);
	} else {
		GPSbuf.starnum = 0;
	}

//	printf("GPSbuf:starnum=%d\n", GPSbuf.starnum);


	// 水平精度因子1.9 从0.5到99.9
	index = gps_getCommaIndex(8, GPS_GPGGA_BUF);
	if (index != 0) {
		fTmp = atof((const char *) GPS_GPGGA_BUF + index);
		GPSbuf.hdop = (int) (fTmp * 10);
	} else {
		GPSbuf.hdop = 0.0;
	}

	// 高度 天线离海平面的高度 114.2 （-9999.9到9999.9）
	index = gps_getCommaIndex(9, GPS_GPGGA_BUF);
	if (index != 0) {
		if (GPS_GPGGA_BUF[index] == '-') {
			GPSbuf.height = atoi((const char *) GPS_GPGGA_BUF + index + 1);
			GPSbuf.height = GPSbuf.height | 0x4000;
		} else {
			GPSbuf.height = atoi((const char *) GPS_GPGGA_BUF + index);
		}
	} else {
		GPSbuf.height = 0.0;
	}
}

/*******************************************************************************
 * Function Name  : Fun_api_HexToAsc
 * Description    : 将ASCII码数值转换为数值
 * Input          :
 ch:转换字符
 * Output         : 输出转换后的数值
 * Return         : None
 *******************************************************************************/
u8 Fun_api_HexToAsc(u8 ch) {
	if (ch >= '0' && ch <= '9')
		return (ch - 0x30);
	else if (ch >= 'A' && ch <= 'F')
		return (ch - 0x37);
	else if (ch >= 'a' && ch <= 'f')
		return (ch - 0x57);
	else
		return (ch);
}

/*
 *功能：		将16进制字符串转换为10进制数
 *输入：		16进制字符串str
 *返回值：	转换的结果10进制数值
 */
int HexToOct(char *str) {
	if (str == NULL) {
		return 0;
	}
	int value = 0;
	while (1) {
		if ((*str >= '0') && (*str <= '9')) {
			value = value * 16 + (*str - '0');
		} else if ((*str >= 'A') && (*str <= 'F')) {
			value = value * 16 + (*str - 'A') + 10;
		} else if ((*str >= 'a') && (*str <= 'f')) {
			value = value * 16 + (*str - 'a') + 10;
		} else {
			break;
		}
		str++;
	}
	return value;
}

/*
 *功能：		检验从串口获得的一条数据，是否包含指定的包头，校验位是否符合，如果有并且符合就解析，解析后对应的全局标志位置1
 *输入：		void
 *返回值：	void
 */
int RMCFlag = 0, GGAFlag = 0;
void CheckDateHead(unsigned char * nmeaStr) {
	char *prmc = NULL;
	char *pgga = NULL;
	char *prmcEnd = NULL;
	char *pggaEnd = NULL;

	prmc = strstr((char *) nmeaStr, "GPRMC");
	pgga = strstr((char *) nmeaStr, "GPGGA");

	if (prmc != NULL) {
		prmcEnd = strchr((const char*) prmc, '*');
		if (prmcEnd != NULL) {
			GSM_GpsFailCount = 0;
			memset(GPS_GPRMC_BUF, 0, 100 * sizeof(char));
			memcpy(GPS_GPRMC_BUF, prmc,  prmcEnd - prmc);
			printf_gps("gprmc_buf:%s\n", GPS_GPRMC_BUF);
			ParseRMCDate();	// 解析RMC的数据
			RMCFlag = 1;
			gps_timeend = gps_timestart;
		}
	}
	if (pgga != NULL) {
		pggaEnd = strchr((const char*) pgga, '*');
		if (pggaEnd != NULL) {
			GSM_GpsFailCount = 0;
			memset(GPS_GPGGA_BUF, 0, 100 * sizeof(char));
			memcpy(GPS_GPGGA_BUF, pgga, pggaEnd - pgga);
//			printf("gpgga_buf:%s\n", GPS_GPGGA_BUF);
			ParseGGADate();	// 解析GGA的数据
			GGAFlag = 1;
			gps_timeend = gps_timestart;
		}
	}
}

int gps_receive() {
	unsigned char nmeaStr[500];
	int timeoutRet;
	int readRet;

	if (system_state == sys_state_save) {
		gps_stop();
		gps_close();
	} else {
		//gps ant check
		gps_antenna_monitor();

		gps_timestart = api_GetSysSecs();
		if (gps_timeadjustflag == 0) {
			gps_timeadjustflag = 1;
			gps_timeend = gps_timestart;
			gps_time_nolocate = gps_timestart;
		}

		if (gps_timestart > gps_timeend) {
			if (gps_timestart - gps_timeend > 2) {
				System.GPS.Time.Valid = 0;
				System.GPS.Position.Valid = 0;
				System.GPS.Valid = 0;
				AlarmSetBit(AlarmIdGPSCom);    //模块异常报警
			}
			if (gps_timestart - gps_timeend >= 30) { // 30秒未收到准确GPS数据可认为GPS启动失败 //but lbs located
				gps_timeadjustflag = 0;
				gps_timeend = gps_timestart;
				printf_gps("GPS had 30 s no  recv ,need to restart!\n ");
				return 0;
			}
		}
		// 超时1s去读串口数据
		timeoutRet = api_TimeOut(gps_fd, 1, 1);
		if (timeoutRet < 0) {
			printf("gps recv api_timeout err\n");
		}else if (0 == timeoutRet) {
			printf("gps recv timeout\n");
		}else {
			usleep(10 * 1000);
			memset(nmeaStr, 0, sizeof(nmeaStr));
			readRet = read(gps_fd, nmeaStr, sizeof(nmeaStr));
			if (readRet <= 0) {
				printf("gps recv read err\n");
			}else {
//				printf("%s\n", nmeaStr);
				// 解析读到的数据
				CheckDateHead(nmeaStr);
				gps_timeend = gps_timestart;
			}
		}

		if ((RMCFlag == 1) && (GGAFlag == 1)) {
			gps_data_save();
			RMCFlag = 0;
			GGAFlag = 0;
			AlarmClrBit(AlarmIdGPSCom);
		}
	}
	return 1;
}
