#ifndef  _LOCK_H_H_
#define  _LOCK_H_H_

// #include "mb90340.h"
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/netlink.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>
#include <fcntl.h>
#include <errno.h>
#include "candriver.h"
#include "general.h"
#include"api.h"

/*#include "CAN.h"
 #include "EXTI.h"
 #include "GPIO.h"
 #include "I2C.h"
 #include "TIMER.h"
 #include "USART.h"

 #include "AT1024.h"
 #include "DS1307.h"
 #include "GPS.h"
 #include "GSM.h"

 #include "alarm.h"
 #include "candata.h"
 //#include "DecoderToTerminal.h"
 #include "gpsdata.h"
 #include "handle.h"
 #include "initialset.h"

 #include "OVERDAYDATA.h"
 #include "realtime.h"
 #include "SAVEPARAMETER.h"
 #include "SENDQUEUE.h"
 #include "timelapse.h" */

extern u8 lock_level; // 当前锁车级别，每次变化只在KEYON时执行 0-默认(解车) 1-锁1级 2-锁2级

#define lock_Lock_Imme_Data_EveryLen  ((u8)(4))//每层立即锁条件的长度
typedef union {
	u8 data[12];
	struct {
		u8 b10 :4;
		u8 b14 :1;
		u8 b15 :3;
		u8 data1[3];
		u8 b20 :4;
		u8 b24 :1;
		u8 b25 :3;
		u8 data2[3];
		u8 b30 :4;
		u8 b34 :1;
		u8 b35 :3;
		u8 data3[3];
	} bit;
} lock_ImmeLockbit;

extern lock_ImmeLockbit lock_Lock_Imme_Data; //立即锁条件
/*
 立即锁条件的格式：
 ------------------------------------------------------------------------------------------------------------
 项目         |长度(字节) |   位置       |   单位   |  备注                                                 |
 ------------------------------------------------------------------------------------------------------------
 锁车级别     |           |  p1.7-p1.5   |          |  1-一级锁车；2-二级锁车；3-三级锁车；其余无效         |
 --------------           -----------------------------------------------------------------------------------
 锁车确认     |    1      |   p1.4       |          |  1-需要确认；0-不需要确认                             |
 --------------           -----------------------------------------------------------------------------------
 解车密码基数 |           |  p.3-p.0     |          |   根据此密码基数计算出解车密码                        |
 ------------------------------------------------------------------------------------------------------------
 信息对照码   |   3       |              |          |    设置本条件时的信息对照码                           |
 ------------------------------------------------------------------------------------------------------------
 */
#define lock_Lock_Imme_Data_Ram   lock_Lock_Imme_Data.data
#define lock_Lock_Imme_Data_Len   lock_Lock_Imme_Data_EveryLen*3

#define lock_Lock_A_Imme_Level lock_Lock_Imme_Data.bit.b15   //A立即锁锁车级别
#define lock_Lock_A_Imme_Conf  lock_Lock_Imme_Data.bit.b14   //A立即锁解车确认
#define lock_Lock_A_Imme_Code  lock_Lock_Imme_Data.bit.b10   //A立即锁解车密码基数
#define lock_Lock_A_Imme_DZM   lock_Lock_Imme_Data.bit.data1 //A立即锁信息对照码
#define lock_Lock_B_Imme_Level lock_Lock_Imme_Data.bit.b25   //B立即锁锁车级别
#define lock_Lock_B_Imme_Conf  lock_Lock_Imme_Data.bit.b24   //B立即锁解车确认
#define lock_Lock_B_Imme_Code  lock_Lock_Imme_Data.bit.b20   //B立即锁解车密码基数
#define lock_Lock_B_Imme_DZM   lock_Lock_Imme_Data.bit.data2 //B立即锁信息对照码
#define lock_Lock_C_Imme_Level lock_Lock_Imme_Data.bit.b35   //C立即锁锁车级别
#define lock_Lock_C_Imme_Conf  lock_Lock_Imme_Data.bit.b34   //C立即锁解车确认
#define lock_Lock_C_Imme_Code  lock_Lock_Imme_Data.bit.b30   //C立即锁解车密码基数
#define lock_Lock_C_Imme_DZM   lock_Lock_Imme_Data.bit.data3 //C立即锁信息对照码
typedef union {
	u8 data[18];
	struct {
		u8 b10 :4;
		u8 b14 :1;
		u8 b15 :3;
		u8 hr1[2];
		u8 data1[3];
		u8 b20 :4;
		u8 b24 :1;
		u8 b25 :3;
		u8 hr2[2];
		u8 data2[3];
		u8 b30 :4;
		u8 b34 :1;
		u8 b35 :3;
		u8 hr3[2];
		u8 data3[3];
	} bit;
} lock_HourLockbit;

extern lock_HourLockbit lock_Lock_Hour_Data; //工作小时锁条件
#define lock_Lock_Hour_Data_EveryLen  ((u8)(6))//每层工作小时锁条件的长度
/*
 工作小时锁条件的格式：
 ----------------------------------------------------------------------------------------------------------------
 项目             |长度(字节) |   位置       |   单位   |  备注                                                 |
 ----------------------------------------------------------------------------------------------------------------
 锁车级别         |           |  p1.7-p1.5   |          |  1-一级锁车；2-二级锁车；3-三级锁车；其余无效         |
 --------------               -----------------------------------------------------------------------------------
 锁车确认         |    1      |   p1.4       |          |  1-需要确认；0-不需要确认                             |
 --------------               -----------------------------------------------------------------------------------
 解车密码基数     |           |  p.3-p.0     |          |   根据此密码基数计算出解车密码                        |
 ----------------------------------------------------------------------------------------------------------------
 预定锁车工作小时 |   2       |              |  Hour    |    设置预定的工作小时数                               |
 ----------------------------------------------------------------------------------------------------------------
 信息对照码       |   3       |              |          |    设置本条件时的信息对照码                           |
 ----------------------------------------------------------------------------------------------------------------
 */
#define lock_Lock_Hour_Data_Ram   lock_Lock_Hour_Data.data
#define lock_Lock_Hour_Data_Len   lock_Lock_Hour_Data_EveryLen*3

#define lock_Lock_A_Hour_Level lock_Lock_Hour_Data.bit.b15 //A工作小时锁锁车级别
#define lock_Lock_A_Hour_Conf  lock_Lock_Hour_Data.bit.b14 //A立即锁解车确认
#define lock_Lock_A_Hour_Code  lock_Lock_Hour_Data.bit.b10 //A工作小时锁解车密码基数
#define lock_Lock_A_Hour_Set   ((lock_Lock_Hour_Data.bit.hr1[0]<<8)|lock_Lock_Hour_Data.bit.hr1[1])//A预定锁车工作小时
#define lock_Lock_A_Hour_DZM   lock_Lock_Hour_Data.bit.data1//A预定锁信息对照码
#define lock_Lock_B_Hour_Level lock_Lock_Hour_Data.bit.b25 //B工作小时锁锁车级别
#define lock_Lock_B_Hour_Conf  lock_Lock_Hour_Data.bit.b24 //B立即锁解车确认
#define lock_Lock_B_Hour_Code  lock_Lock_Hour_Data.bit.b20 //B工作小时锁解车密码基数
#define lock_Lock_B_Hour_Set   ((lock_Lock_Hour_Data.bit.hr2[0]<<8)|lock_Lock_Hour_Data.bit.hr2[1])//B预定锁车工作小时
#define lock_Lock_B_Hour_DZM   lock_Lock_Hour_Data.bit.data2//B预定锁信息对照码
#define lock_Lock_C_Hour_Level lock_Lock_Hour_Data.bit.b35 //C工作小时锁锁车级别
#define lock_Lock_C_Hour_Conf  lock_Lock_Hour_Data.bit.b34 //C立即锁解车确认
#define lock_Lock_C_Hour_Code  lock_Lock_Hour_Data.bit.b30 //C工作小时锁解车密码基数
#define lock_Lock_C_Hour_Set   ((lock_Lock_Hour_Data.bit.hr3[0]<<8)|lock_Lock_Hour_Data.bit.hr3[1])//C预定锁车工作小时
#define lock_Lock_C_Hour_DZM   lock_Lock_Hour_Data.bit.data3//C预定锁信息对照码
typedef union {
	u8 data[48];
	struct {
		u8 b10 :4;
		u8 b14 :1;
		u8 b15 :3;
		u8 pi1[12];
		u8 data1[3];
		u8 b20 :4;
		u8 b24 :1;
		u8 b25 :3;
		u8 pi2[12];
		u8 data2[3];
		u8 b30 :4;
		u8 b34 :1;
		u8 b35 :3;
		u8 pi3[12];
		u8 data3[3];
	} bit;
} lock_PosiLockbit;

extern lock_PosiLockbit lock_Lock_Posi_Data; //位置锁条件
#define lock_Lock_Posi_Data_EveryLen  ((u8)(16))//每层位置锁条件的长度
/*
 位置锁条件的格式：
 ----------------------------------------------------------------------------------------------------------------
 项目             |长度(字节) |   位置       |   单位   |  备注                                                 |
 ----------------------------------------------------------------------------------------------------------------
 锁车级别         |           |  p1.7-p1.5   |          |  1-一级锁车；2-二级锁车；3-三级锁车；其余无效         |
 --------------               -----------------------------------------------------------------------------------
 锁车确认         |    1      |   p1.4       |          |  1-需要确认；0-不需要确认                             |
 --------------               -----------------------------------------------------------------------------------
 解车密码基数     |           |  p.3-p.0     |          |   根据此密码基数计算出解车密码                        |
 ----------------------------------------------------------------------------------------------------------------
 中心点纬度       |           |   4字节      |0.000001度|  以度为单位的纬度值乘以10的6次方                      |
 ------------------           -----------------------------------------------------------------------------------
 中心点经度       |   12      |   4字节      |0.000001度|  以度为单位的纬度值乘以10的6次方                      |
 ------------------           -----------------------------------------------------------------------------------
 锁车范围         |           |   4字节      |   m      |   位置锁锁车范围半径                                  |
 ----------------------------------------------------------------------------------------------------------------
 信息对照码       |   3       |              |          |    设置本条件时的信息对照码                           |
 ----------------------------------------------------------------------------------------------------------------
 */
#define lock_Lock_Posi_Data_Ram   lock_Lock_Posi_Data.data
#define lock_Lock_Posi_Data_Len   lock_Lock_Posi_Data_EveryLen*3

#define lock_Lock_A_Posi_Level lock_Lock_Posi_Data.bit.b15 //A位置锁锁车级别
#define lock_Lock_A_Posi_Conf  lock_Lock_Posi_Data.bit.b14 //A立即锁解车确认
#define lock_Lock_A_Posi_Code  lock_Lock_Posi_Data.bit.b10 //A位置锁解车密码基数
#define lock_Lock_A_Posi_Set   lock_Lock_Posi_Data.bit.pi1 //A位置锁设定范围
#define lock_Lock_A_Posi_DZM   lock_Lock_Posi_Data.bit.data1; //A位置锁信息对照码
#define lock_Lock_B_Posi_Level lock_Lock_Posi_Data.bit.b25 //B位置锁锁车级别
#define lock_Lock_B_Posi_Conf  lock_Lock_Posi_Data.bit.b24 //B立即锁解车确认
#define lock_Lock_B_Posi_Code  lock_Lock_Posi_Data.bit.b20 //B位置锁解车密码基数
#define lock_Lock_B_Posi_Set   lock_Lock_Posi_Data.bit.pi2 //B位置锁设定范围
#define lock_Lock_B_Posi_DZM   lock_Lock_Posi_Data.bit.data2; //B位置锁信息对照码
#define lock_Lock_C_Posi_Level lock_Lock_Posi_Data.bit.b35 //C位置锁锁车级别
#define lock_Lock_C_Posi_Conf  lock_Lock_Posi_Data.bit.b34 //C立即锁解车确认
#define lock_Lock_C_Posi_Code  lock_Lock_Posi_Data.bit.b30 //C位置锁解车密码基数
#define lock_Lock_C_Posi_Set   lock_Lock_Posi_Data.bit.pi3 //C位置锁设定范围
#define lock_Lock_C_Posi_DZM   lock_Lock_Posi_Data.bit.data3; //C位置锁信息对照码
typedef union {
	u8 data[30];
	struct {
		u8 b10 :4;
		u8 b14 :1;
		u8 b15 :3;
		u8 te1[6];
		u8 data1[3];
		u8 b20 :4;
		u8 b24 :1;
		u8 b25 :3;
		u8 te2[6];
		u8 data2[3];
		u8 b30 :4;
		u8 b34 :1;
		u8 b35 :3;
		u8 te3[6];
		u8 data3[3];
	} bit;
} lock_TimeLockbit;

extern lock_TimeLockbit lock_Lock_Time_Data; //指定时间点锁条件
#define lock_Lock_Time_Data_EveryLen  ((u8)(10))//每层时间点锁条件的长度
/*
 指定时间点锁条件的格式：
 ----------------------------------------------------------------------------------------------------------------
 项目             |长度(字节) |   位置       |   单位   |  备注                                                 |
 ----------------------------------------------------------------------------------------------------------------
 锁车级别         |           |  p1.7-p1.5   |          |  1-一级锁车；2-二级锁车；3-三级锁车；其余无效         |
 --------------               -----------------------------------------------------------------------------------
 锁车确认         |    1      |   p1.4       |          |  1-需要确认；0-不需要确认                             |
 --------------               -----------------------------------------------------------------------------------
 解车密码基数     |           |  p.3-p.0     |          |   根据此密码基数计算出解车密码                        |
 ----------------------------------------------------------------------------------------------------------------
 指定时间点锁设定 |   6       |              |          |    YY-MM-DD-hh-mm-ss，指定锁车的时间点（HEX）         |
 ----------------------------------------------------------------------------------------------------------------
 信息对照码       |   3       |              |          |    设置本条件时的信息对照码                           |
 ----------------------------------------------------------------------------------------------------------------
 */
#define lock_Lock_Time_Data_Ram   lock_Lock_Time_Data.data
#define lock_Lock_Time_Data_Len   lock_Lock_Time_Data_EveryLen*3

#define lock_Lock_A_Time_Level lock_Lock_Time_Data.bit.b15 //A指定时间点锁锁车级别
#define lock_Lock_A_Time_Conf  lock_Lock_Time_Data.bit.b14 //A立即锁解车确认
#define lock_Lock_A_Time_Code  lock_Lock_Time_Data.bit.b10 //A指定时间点锁解车密码基数
#define lock_Lock_A_Time_Set   lock_Lock_Time_Data.bit.te1 //A指定时间点锁设定
#define lock_Lock_A_Time_DZM   lock_Lock_Time_Data.bit.data1 //A指定时间点锁信息对照码
#define lock_Lock_B_Time_Level lock_Lock_Time_Data.bit.b25 //B指定时间点锁锁车级别
#define lock_Lock_B_Time_Conf  lock_Lock_Time_Data.bit.b24 //B立即锁解车确认
#define lock_Lock_B_Time_Code  lock_Lock_Time_Data.bit.b20 //B指定时间点锁解车密码基数
#define lock_Lock_B_Time_Set   lock_Lock_Time_Data.bit.te2 //B指定时间点锁设定
#define lock_Lock_B_Time_DZM   lock_Lock_Time_Data.bit.data2 //B指定时间点锁信息对照码
#define lock_Lock_C_Time_Level lock_Lock_Time_Data.bit.b35 //C指定时间点锁锁车级别
#define lock_Lock_C_Time_Conf  lock_Lock_Time_Data.bit.b34 //C立即锁解车确认
#define lock_Lock_C_Time_Code  lock_Lock_Time_Data.bit.b30 //C指定时间点锁解车密码基数
#define lock_Lock_C_Time_Set   lock_Lock_Time_Data.bit.te3 //C指定时间点锁设定
#define lock_Lock_C_Time_DZM   lock_Lock_Time_Data.bit.data3 //C指定时间点锁信息对照码
/******** /\
 typedef union
 {
 u8 data[45];
 struct
 {
 u8 b10:4;
 u8 b14:1;
 u8 b15:3;
 u8 te1[11];
 u8 data1[3];
 u8 b20:4;
 u8 b24:1;
 u8 b25:3;
 u8 te2[11];
 u8 data2[3];
 u8 b30:4;
 u8 b34:1;
 u8 b35:3;
 u8 te3[11];
 u8 data3[3];
 }bit;
 }lock_CycTLockbit;
 extern lock_CycTLockbit lock_Lock_CycT_Data;//时间段循环锁条件
 #define lock_Lock_CycT_Data_EveryLen  ((u8)(15))//每层时间段循环条件的长度

 */
/*
 时间段循环锁条件的格式：
 ----------------------------------------------------------------------------------------------------------------
 项目             |长度(字节) |   位置       |   单位   |  备注                                                 |
 ----------------------------------------------------------------------------------------------------------------
 锁车级别         |           |  p1.7-p1.5   |          |  1-一级锁车；2-二级锁车；3-三级锁车；其余无效         |
 --------------               -----------------------------------------------------------------------------------
 锁车确认         |    1      |   p1.4       |          |  1-需要确认；0-不需要确认                             |
 --------------               -----------------------------------------------------------------------------------
 解车密码基数     |           |  p.3-p.0     |          |   根据此密码基数计算出解车密码                        |
 ----------------------------------------------------------------------------------------------------------------
 起始年           |           |   1字节      |          |  取值范围[0，255]                                     |
 ------------------           -----------------------------------------------------------------------------------
 结束年           |           |   1字节      |          |  取值范围[0，255]                                     |
 ------------------           -----------------------------------------------------------------------------------
 循环起始月       |           |   1字节      |          |  取值范围[1，12]                                      |
 ------------------           -----------------------------------------------------------------------------------
 循环结束月       |           |   1字节      |          |  取值范围[1，12]                                      |
 ------------------           -----------------------------------------------------------------------------------
 循环起始日       |    11     |   1字节      |          |  取值范围[1，31]                                      |
 ------------------           -----------------------------------------------------------------------------------
 循环结束日       |           |   1字节      |          |  取值范围[1，31]                                      |
 ------------------           -----------------------------------------------------------------------------------
 循环起始时       |           |   1字节      |          |  取值范围[0，23]                                      |
 ------------------           -----------------------------------------------------------------------------------
 循环结束时       |           |   1字节      |          |  取值范围[1，24]                                      |
 ------------------           -----------------------------------------------------------------------------------
 锁车失效日期     |           |   3字节      |          |  YY-MM-DD，“无效置0xFFFFFF”                         |
 ----------------------------------------------------------------------------------------------------------------
 信息对照码       |   3       |              |          |    设置本条件时的信息对照码                           |
 ----------------------------------------------------------------------------------------------------------------
 */
#if 0
#define lock_Lock_A_CycT_Level lock_Lock_CycT_Data.bit.b15 //A时间段循环锁锁车级别
#define lock_Lock_A_CycT_Conf  lock_Lock_CycT_Data.bit.b14 //A立即锁解车确认
#define lock_Lock_A_CycT_Code  lock_Lock_CycT_Data.bit.b10 //A时间段循环锁解车密码基数
#define lock_Lock_A_CycT_Set   lock_Lock_CycT_Data.bit.te1 //A时间段循环锁设定
#define lock_Lock_A_CycT_DZM   lock_Lock_CycT_Data.bit.data1 //A时间段循环锁信息对照码
#define lock_Lock_B_CycT_Level lock_Lock_CycT_Data.bit.b25 //B时间段循环锁锁车级别
#define lock_Lock_B_CycT_Conf  lock_Lock_CycT_Data.bit.b24 //B立即锁解车确认
#define lock_Lock_B_CycT_Code  lock_Lock_CycT_Data.bit.b20 //B时间段循环锁解车密码基数
#define lock_Lock_B_CycT_Set   lock_Lock_CycT_Data.bit.te2 //B时间段循环锁设定
#define lock_Lock_B_CycT_DZM   lock_Lock_CycT_Data.bit.data2 //B时间段循环锁信息对照码
#define lock_Lock_C_CycT_Level lock_Lock_CycT_Data.bit.b35 //C时间段循环锁锁车级别
#define lock_Lock_C_CycT_Conf  lock_Lock_CycT_Data.bit.b34 //C立即锁解车确认
#define lock_Lock_C_CycT_Code  lock_Lock_CycT_Data.bit.b30 //C时间段循环锁解车密码基数
#define lock_Lock_C_CycT_Set   lock_Lock_CycT_Data.bit.te3 //C时间段循环锁设定
#define lock_Lock_C_CycT_DZM   lock_Lock_CycT_Data.bit.data3 //C时间段循环锁信息对照码
typedef union
{
	u8 data[42];
	struct
	{
		u8 b10:4;
		u8 b14:1;
		u8 b15:3;
		u8 te1[10];
		u8 data1[3];
		u8 b20:4;
		u8 b24:1;
		u8 b25:3;
		u8 te2[10];
		u8 data2[3];
		u8 b30:4;
		u8 b34:1;
		u8 b35:3;
		u8 te3[10];
		u8 data3[3];
	}bit;
}lock_CycHLockbit;
extern lock_CycHLockbit lock_Lock_CycH_Data; //日工作时间循环锁条件
#define lock_Lock_CycH_Data_EveryLen  ((u8)(14))//每层日工作时间循环锁条件的长度
#endif
/*
 日工作时间循环锁条件的格式：
 ----------------------------------------------------------------------------------------------------------------
 项目             |长度(字节) |   位置       |   单位   |  备注                                                 |
 ----------------------------------------------------------------------------------------------------------------
 锁车级别         |           |  p1.7-p1.5   |          |  1-一级锁车；2-二级锁车；3-三级锁车；其余无效         |
 --------------               -----------------------------------------------------------------------------------
 锁车确认         |    1      |   p1.4       |          |  1-需要确认；0-不需要确认                             |
 --------------               -----------------------------------------------------------------------------------
 解车密码基数     |           |  p.3-p.0     |          |   根据此密码基数计算出解车密码                        |
 ----------------------------------------------------------------------------------------------------------------
 起始年           |           |   1字节      |          |  取值范围[0，255]                                     |
 ------------------           -----------------------------------------------------------------------------------
 结束年           |           |   1字节      |          |  取值范围[0，255]                                     |
 ------------------           -----------------------------------------------------------------------------------
 循环起始月       |           |   1字节      |          |  取值范围[1，12]                                      |
 ------------------           -----------------------------------------------------------------------------------
 循环结束月       |           |   1字节      |          |  取值范围[1，12]                                      |
 ------------------           -----------------------------------------------------------------------------------
 循环起始日       |    10     |   1字节      |          |  取值范围[1，31]                                      |
 ------------------           -----------------------------------------------------------------------------------
 循环结束日       |           |   1字节      |          |  取值范围[1，31]                                      |
 ------------------           -----------------------------------------------------------------------------------
 设置日工作时间   |           |   1字节      |   Hour   |  设置车辆当天最大工作时间                             |
 ------------------           -----------------------------------------------------------------------------------
 锁车失效日期     |           |   3字节      |          |  YY-MM-DD，“无效置0xFFFFFF”                         |
 ----------------------------------------------------------------------------------------------------------------
 信息对照码       |   3       |              |          |    设置本条件时的信息对照码                           |
 ----------------------------------------------------------------------------------------------------------------
 */
/*
 #define lock_Lock_A_CycH_Level lock_Lock_CycH_Data.bit.b15 //A日工作时间循环锁锁车级别
 #define lock_Lock_A_CycH_Conf  lock_Lock_CycH_Data.bit.b14 //A立即锁解车确认
 #define lock_Lock_A_CycH_Code  lock_Lock_CycH_Data.bit.b10 //A日工作时间循环锁解车密码基数
 #define lock_Lock_A_CycH_Set   lock_Lock_CycH_Data.bit.te1 //A日工作时间循环锁设定
 #define lock_Lock_A_CycH_DZM   lock_Lock_CycH_Data.bit.data1 //A日工作时间循环锁信息对照码
 #define lock_Lock_B_CycH_Level lock_Lock_CycH_Data.bit.b25 //B日工作时间循环锁锁车级别
 #define lock_Lock_B_CycH_Conf  lock_Lock_CycH_Data.bit.b24 //B立即锁解车确认
 #define lock_Lock_B_CycH_Code  lock_Lock_CycH_Data.bit.b20 //B日工作时间循环锁解车密码基数
 #define lock_Lock_B_CycH_Set   lock_Lock_CycH_Data.bit.te2 //B日工作时间循环锁设定
 #define lock_Lock_B_CycH_DZM   lock_Lock_CycH_Data.bit.data2 //B日工作时间循环锁信息对照码
 #define lock_Lock_C_CycH_Level lock_Lock_CycH_Data.bit.b35 //C日工作时间循环锁锁车级别
 #define lock_Lock_C_CycH_Conf  lock_Lock_CycH_Data.bit.b34 //C立即锁解车确认
 #define lock_Lock_C_CycH_Code  lock_Lock_CycH_Data.bit.b30 //C日工作时间循环锁解车密码基数
 #define lock_Lock_C_CycH_Set   lock_Lock_CycH_Data.bit.te3 //C日工作时间循环锁设定
 #define lock_Lock_C_CycH_DZM   lock_Lock_CycH_Data.bit.data3 //C日工作时间循环锁信息对照码
 / ***********************/

typedef union {
	u8 data[162];
	struct {
		u8 b10 :4;
		u8 b14 :1;
		u8 b15 :3;
		u8 te1[50];
		u8 data1[3];
		u8 b20 :4;
		u8 b24 :1;
		u8 b25 :3;
		u8 te2[50];
		u8 data2[3];
		u8 b30 :4;
		u8 b34 :1;
		u8 b35 :3;
		u8 te3[50];
		u8 data3[3];
	} bit;
} lock_DateLockbit;

extern lock_DateLockbit lock_Lock_Date_Data; //指定日期组锁锁条件
#define lock_Lock_Date_Data_EveryLen  ((u8)(54))//每层指定日期组锁条件的长度
/*
 指定日期组锁条件的格式：
 ----------------------------------------------------------------------------------------------------------------------------------
 项目             |长度(字节) |   位置       |   单位   |  备注                                                                   |
 ----------------------------------------------------------------------------------------------------------------------------------
 锁车级别         |           |  p1.7-p1.5   |          |  1-一级锁车；2-二级锁车；3-三级锁车；其余无效                           |
 --------------               -----------------------------------------------------------------------------------------------------
 锁车确认         |    1      |   p1.4       |          |  1-需要确认；0-不需要确认                                               |
 --------------               -----------------------------------------------------------------------------------------------------
 解车密码基数     |           |  p.3-p.0     |          |   根据此密码基数计算出解车密码                                          |
 ----------------------------------------------------------------------------------------------------------------------------------
 起始年           |           |   1字节      |          |  取值范围[0，255]                                                       |
 ------------------           -----------------------------------------------------------------------------------------------------
 起始月           |           |   1字节      |          |  取值范围[1，12]                                                        |
 ------------------           -----------------------------------------------------------------------------------------------------
 第1个月的锁车日  |           |   1字节      |          |  取值范围[1，31]，0xFF表示未设置该月锁车日，0xFE表示该月已执行锁车动作。|
 ------------------           -----------------------------------------------------------------------------------------------------
 第2个月的锁车日  |           |   1字节      |          |  取值范围[1，31]，0xFF表示未设置该月锁车日，0xFE表示该月已执行锁车动作。|
 ------------------           -----------------------------------------------------------------------------------------------------
 第3个月的锁车日  |    50     |   1字节      |          |  取值范围[1，31]，0xFF表示未设置该月锁车日，0xFE表示该月已执行锁车动作。|
 ------------------           -----------------------------------------------------------------------------------------------------
 ........       |           |   1字节      |          |  .................................                                      |
 ------------------           -----------------------------------------------------------------------------------------------------
 第47个月的锁车日 |           |   1字节      |          |  取值范围[1，31]，0xFF表示未设置该月锁车日，0xFE表示该月已执行锁车动作。|
 ------------------           -----------------------------------------------------------------------------------------------------
 第48个月的锁车日 |           |   1字节      |          |  取值范围[1，31]，0xFF表示未设置该月锁车日，0xFE表示该月已执行锁车动作。|
 ----------------------------------------------------------------------------------------------------------------------------------
 信息对照码       |   3       |              |          |    设置本条件时的信息对照码                                             |
 ----------------------------------------------------------------------------------------------------------------------------------
 */
#define lock_Lock_Date_Data_Ram   lock_Lock_Date_Data.data
#define lock_Lock_Date_Data_Len   lock_Lock_Date_Data_EveryLen*3

#define lock_Lock_A_Date_Level lock_Lock_Date_Data.bit.b15 //A指定日期组锁锁车级别
#define lock_Lock_A_Date_Conf  lock_Lock_Date_Data.bit.b14 //A立即锁解车确认
#define lock_Lock_A_Date_Code  lock_Lock_Date_Data.bit.b10 //A指定日期组锁解车密码基数
#define lock_Lock_A_Date_Set   lock_Lock_Date_Data.bit.te1 //A指定日期组锁设定
#define lock_Lock_A_Date_DZM   lock_Lock_Date_Data.bit.data1 //A指定日期组锁信息对照码
#define lock_Lock_B_Date_Level lock_Lock_Date_Data.bit.b25 //B指定日期组锁锁车级别
#define lock_Lock_B_Date_Conf  lock_Lock_Date_Data.bit.b24 //B立即锁解车确认
#define lock_Lock_B_Date_Code  lock_Lock_Date_Data.bit.b20 //B指定日期组锁解车密码基数
#define lock_Lock_B_Date_Set   lock_Lock_Date_Data.bit.te2 //B指定日期组锁设定
#define lock_Lock_B_Date_DZM   lock_Lock_Date_Data.bit.data2 //B指定日期组锁信息对照码
#define lock_Lock_C_Date_Level lock_Lock_Date_Data.bit.b35 //C指定日期组锁锁车级别
#define lock_Lock_C_Date_Conf  lock_Lock_Date_Data.bit.b34 //C立即锁解车确认
#define lock_Lock_C_Date_Code  lock_Lock_Date_Data.bit.b30 //C指定日期组锁解车密码基数
#define lock_Lock_C_Date_Set   lock_Lock_Date_Data.bit.te3 //C指定日期组锁设定
#define lock_Lock_C_Date_DZM   lock_Lock_Date_Data.bit.data3 //C指定日期组锁信息对照码
typedef struct {
	u8 leve10 :3;
	u8 state10 :1;
	u8 leve11 :3;
	u8 state11 :1;

	u8 leve20 :3;
	u8 state20 :1;
	u8 leve21 :3;
	u8 state21 :1;

	u8 leve30 :3;
	u8 state30 :1;
	u8 leve31 :3;
	u8 state31 :1;

	u8 leve40 :3;
	u8 state40 :1;
	u8 leve41 :3;
	u8 state41 :1;
} lock_u32;

typedef union {
	u8 data[16];
	struct {
		lock_u32 A_St;
		lock_u32 B_St;
		lock_u32 C_St;
		lock_u32 F_St;
	} St;
} lockSignSetbit;

extern lockSignSetbit lock_LockCar_State; //锁车状态字

#define lock_LockCar_State_EveryLen  ((u8)(4))//每层锁车状态字的长度
/*
 锁车状态字的格式：
 ----------------------------------------------------------------------------------------------------------------------------------
 项目             |长度(字节) |   位置       |   单位   |  备注                                                                   |
 ----------------------------------------------------------------------------------------------------------------------------------
 A/B/C层锁车状态字|           |  p4.7        |          |  “立即锁”标志：1--锁车；0--解车                                       |
 -----------------------------------------------------------------------------------------------------
 |           |  p4.6-p4.4   |          |  “立即锁”级别：1--锁一级；2--锁二级；3--锁三级；其它--无效            |
 -----------------------------------------------------------------------------------------------------
 |           |  p4.3        |          |  “预定工作小时锁”标志：1--锁车；0--解车                               |
 -----------------------------------------------------------------------------------------------------
 |           |  p4.2-p4.0   |          |  “预定工作小时锁”级别：1--锁一级；2--锁二级；3--锁三级；其它--无效    |
 -----------------------------------------------------------------------------------------------------
 |           |  p3.7        |          |  “位置锁”标志：1--锁车；0--解车                                       |
 -----------------------------------------------------------------------------------------------------
 |           |  p3.6-p3.4   |          |  “位置锁”级别：1--锁一级；2--锁二级；3--锁三级；其它--无效            |
 -----------------------------------------------------------------------------------------------------
 |    4      |  p3.3        |          |  “指定时间点锁”标志：1--锁车；0--解车                                 |
 -----------------------------------------------------------------------------------------------------
 |           |  p3.2-p3.0   |          |  “指定时间点锁”级别：1--锁一级；2--锁二级；3--锁三级；其它--无效      |
 -----------------------------------------------------------------------------------------------------
 |           |  p2.7        |          |  “时间段循环锁”标志：1--锁车；0--解车                                 |
 -----------------------------------------------------------------------------------------------------
 |           |  p2.6-p2.4   |          |  “时间段循环锁”级别：1--锁一级；2--锁二级；3--锁三级；其它--无效      |
 -----------------------------------------------------------------------------------------------------
 |           |  p2.3        |          |  “日工作时间循环锁”标志：1--锁车；0--解车                             |
 -----------------------------------------------------------------------------------------------------
 |           |  p2.2-p2.0   |          |  “日工作时间循环锁”级别：1--锁一级；2--锁二级；3--锁三级；其它--无效  |
 -----------------------------------------------------------------------------------------------------
 |           |  p1.7        |          |  “指定日期组锁”标志：1--锁车；0--解车                                 |
 -----------------------------------------------------------------------------------------------------
 |           |  p1.6-p1.4   |          |  “指定日期组锁”级别：1--锁一级；2--锁二级；3--锁三级；其它--无效      |
 ----------------------------------------------------------------------------------------------------------------------------------
 防拆锁锁车状态字 |           |  p4.7        |          |  “GSM天线防拆锁”标志：1--锁车；0--解车                                |
 -----------------------------------------------------------------------------------------------------
 |           |  p4.6-p4.4   |          |  “GSM天线防拆锁”级别：1--锁一级；2--锁二级；3--锁三级；其它--无效     |
 -----------------------------------------------------------------------------------------------------
 |           |  p4.3        |          |  “GPS天线防拆锁”标志：1--锁车；0--解车                                |
 -----------------------------------------------------------------------------------------------------
 |           |  p4.2-p4.0   |          |  “GPS天线防拆锁”级别：1--锁一级；2--锁二级；3--锁三级；其它--无效     |
 -----------------------------------------------------------------------------------------------------
 |           |  p3.7        |          |  “SIM卡防拆锁”标志：1--锁车；0--解车                                  |
 -----------------------------------------------------------------------------------------------------
 |           |  p3.6-p3.4   |          |  “SIM卡防拆锁”级别：1--锁一级；2--锁二级；3--锁三级；其它--无效       |
 -----------------------------------------------------------------------------------------------------
 |    4      |  p3.3        |          |  “防拆终端防拆锁”标志：1--锁车；0--解车                               |
 -----------------------------------------------------------------------------------------------------
 |           |  p3.2-p3.0   |          |  “防拆终端防拆锁”级别：1--锁一级；2--锁二级；3--锁三级；其它--无效    |
 -----------------------------------------------------------------------------------------------------
 |           |  p2.7        |          |  “终端外壳防拆锁”标志：1--锁车；0--解车                               |
 -----------------------------------------------------------------------------------------------------
 |           |  p2.6-p2.4   |          |  “终端外壳防拆锁”级别：1--锁一级；2--锁二级；3--锁三级；其它--无效    |
 ----------------------------------------------------------------------------------------------------------------------------------
 */
#define lock_LockCar_State_Ram   lock_LockCar_State.data
#define lock_LockCar_State_Len   lock_LockCar_State_EveryLen*4

#define lock_LockCar_State_A_Imme_St      lock_LockCar_State.St.A_St.state11 //立即锁状态
#define lock_LockCar_State_A_Imme_Level   lock_LockCar_State.St.A_St.leve11  //立即锁级别
#define lock_LockCar_State_A_Hour_St      lock_LockCar_State.St.A_St.state10 //工作小时锁状态
#define lock_LockCar_State_A_Hour_Level   lock_LockCar_State.St.A_St.leve10  //工作小时锁级别
#define lock_LockCar_State_A_Posi_St      lock_LockCar_State.St.A_St.state21 //位置锁状态
#define lock_LockCar_State_A_Posi_Level   lock_LockCar_State.St.A_St.leve21  //位置锁级别
#define lock_LockCar_State_A_Time_St      lock_LockCar_State.St.A_St.state20 //指定时间点锁状态
#define lock_LockCar_State_A_Time_Level   lock_LockCar_State.St.A_St.leve20  //指定时间点锁级别
#define lock_LockCar_State_A_CycT_St      lock_LockCar_State.St.A_St.state31 //时间段循环锁状态
#define lock_LockCar_State_A_CycT_Level   lock_LockCar_State.St.A_St.leve31  //时间段循环锁级别
#define lock_LockCar_State_A_CycH_St      lock_LockCar_State.St.A_St.state30 //日工作时间循环锁状态
#define lock_LockCar_State_A_CycH_Level   lock_LockCar_State.St.A_St.leve30  //日工作时间循环锁级别
#define lock_LockCar_State_A_Date_St      lock_LockCar_State.St.A_St.state41 //指定日期组锁状态
#define lock_LockCar_State_A_Date_Level   lock_LockCar_State.St.A_St.leve41  //指定日期组锁级别
#define lock_LockCar_State_B_Imme_St      lock_LockCar_State.St.B_St.state11 //立即锁状态
#define lock_LockCar_State_B_Imme_Level   lock_LockCar_State.St.B_St.leve11  //立即锁级别
#define lock_LockCar_State_B_Hour_St      lock_LockCar_State.St.B_St.state10 //工作小时锁状态
#define lock_LockCar_State_B_Hour_Level   lock_LockCar_State.St.B_St.leve10  //工作小时锁级别
#define lock_LockCar_State_B_Posi_St      lock_LockCar_State.St.B_St.state21 //位置锁状态
#define lock_LockCar_State_B_Posi_Level   lock_LockCar_State.St.B_St.leve21  //位置锁级别
#define lock_LockCar_State_B_Time_St      lock_LockCar_State.St.B_St.state20 //指定时间点锁状态
#define lock_LockCar_State_B_Time_Level   lock_LockCar_State.St.B_St.leve20  //指定时间点锁级别
#define lock_LockCar_State_B_CycT_St      lock_LockCar_State.St.B_St.state31 //时间段循环锁状态
#define lock_LockCar_State_B_CycT_Level   lock_LockCar_State.St.B_St.leve31  //时间段循环锁级别
#define lock_LockCar_State_B_CycH_St      lock_LockCar_State.St.B_St.state30 //日工作时间循环锁状态
#define lock_LockCar_State_B_CycH_Level   lock_LockCar_State.St.B_St.leve30  //日工作时间循环锁级别
#define lock_LockCar_State_B_Date_St      lock_LockCar_State.St.B_St.state41 //指定日期组锁状态
#define lock_LockCar_State_B_Date_Level   lock_LockCar_State.St.B_St.leve41  //指定日期组锁级别
#define lock_LockCar_State_C_Imme_St      lock_LockCar_State.St.C_St.state11 //立即锁状态
#define lock_LockCar_State_C_Imme_Level   lock_LockCar_State.St.C_St.leve11  //立即锁级别
#define lock_LockCar_State_C_Hour_St      lock_LockCar_State.St.C_St.state10 //工作小时锁状态
#define lock_LockCar_State_C_Hour_Level   lock_LockCar_State.St.C_St.leve10  //工作小时锁级别
#define lock_LockCar_State_C_Posi_St      lock_LockCar_State.St.C_St.state21 //位置锁状态
#define lock_LockCar_State_C_Posi_Level   lock_LockCar_State.St.C_St.leve21  //位置锁级别
#define lock_LockCar_State_C_Time_St      lock_LockCar_State.St.C_St.state20 //指定时间点锁状态
#define lock_LockCar_State_C_Time_Level   lock_LockCar_State.St.C_St.leve20  //指定时间点锁级别
#define lock_LockCar_State_C_CycT_St      lock_LockCar_State.St.C_St.state31 //时间段循环锁状态
#define lock_LockCar_State_C_CycT_Level   lock_LockCar_State.St.C_St.leve31  //时间段循环锁级别
#define lock_LockCar_State_C_CycH_St      lock_LockCar_State.St.C_St.state30 //日工作时间循环锁状态
#define lock_LockCar_State_C_CycH_Level   lock_LockCar_State.St.C_St.leve30  //日工作时间循环锁级别
#define lock_LockCar_State_C_Date_St      lock_LockCar_State.St.C_St.state41 //指定日期组锁状态
#define lock_LockCar_State_C_Date_Level   lock_LockCar_State.St.C_St.leve41  //指定日期组锁级别
#define lock_LockCar_State_F_GSMAnt_St         lock_LockCar_State.St.F_St.state11 //GSM天线防拆锁状态
#define lock_LockCar_State_F_GSMAnt_Level      lock_LockCar_State.St.F_St.leve11  //GSM天线防拆锁级别
#define lock_LockCar_State_F_GPSAnt_St         lock_LockCar_State.St.F_St.state10 //GPS天线防拆锁状态
#define lock_LockCar_State_F_GPSAnt_Level      lock_LockCar_State.St.F_St.leve10  //GPS天线防拆锁级别
#define lock_LockCar_State_F_SIM_St            lock_LockCar_State.St.F_St.state21 //SIM卡锁状态
#define lock_LockCar_State_F_SIM_Level         lock_LockCar_State.St.F_St.leve21  //SIM卡锁级别
#define lock_LockCar_State_F_FTerminal_St      lock_LockCar_State.St.F_St.state20 //防拆终端锁状态
#define lock_LockCar_State_F_FTerminal_Level   lock_LockCar_State.St.F_St.leve20  //防拆终端锁级别
#define lock_LockCar_State_F_Box_St            lock_LockCar_State.St.F_St.state31 //终端外壳防拆锁状态
#define lock_LockCar_State_F_Box_Level         lock_LockCar_State.St.F_St.leve31  //终端外壳防拆锁级别
#define lock_LockCar_Applyt_Len  lock_LockCar_State_Len
extern u8 lock_LockCar_Applyt[16]; //锁车申请

/* Exported types ------------------------------------------------------------*/

#define  lock_MimaUnlock_Imme     LockSign01.bit.b7
#define  lock_MimaUnlock_Hour     LockSign01.bit.b6
#define  lock_keyon               LockSign01.bit.b5 // 用于锁车判断的keyon状态 0-默认 1-keyon
#define  lock_ApplytSendSign     LockSign00.bit.b7
#define  lock_MimaUnlock_Posi    LockSign00.bit.b6
#define  lock_MimaUnlock_Time    LockSign00.bit.b5
#define  lock_MimaUnlock_Date    LockSign00.bit.b4

#define  lock_MimaUnlock_GsmAnt  LockSign00.bit.b3
#define  lock_MimaUnlock_GpsAnt  LockSign00.bit.b2
#define  lock_MimaUnlock_Sim     LockSign00.bit.b1
#define  lock_MimaUnlock_Xilie   LockSign00.bit.b0

/*******************************sys_task.h define********************************/
#define FLASH_PAGE_SIZE         264    //AT45DB081
#define FLASH_BLOCK_SIZE        0x100000
 uint8_t  MAIN_Buffer_Temp[2500];
#define SAVE_VER 0x11 // 参数版本
/* Private define ------------------------------------------------------------*/
//#define  SYS_CANSET_REC_MAX      50
//#define  SYS_CANSET_REC_LEN      7
//#define  SYS_CANSET_REC_DATAMAX  (SYS_CANSET_REC_MAX * SYS_CANSET_REC_LEN)
//#define  SYS_CANSET_QUE_MAX      20
//#define  SYS_CANSET_QUE_LEN      19
//#define  SYS_CANSET_QUE_DATAMAX  (SYS_CANSET_QUE_MAX * SYS_CANSET_QUE_LEN)
//
//#define  SYS_SET_DATAMAX        234
//
//#define  SYS_SET_EE_ADDR       (FR_TEST_ADDR + 1)
//#define  SYS_SET_DATA_LEN      (SYS_SET_DATAMAX + 2 + 3)	//+2 IMU设置
//#define  SYS_SET_EE_MAX        (SYS_SET_EE_ADDR + SYS_SET_DATA_LEN)
//
//#define  SYS_PARA_EE_ADDR      (SYS_SET_EE_MAX + 1)
//#define  SYS_PARA_DATA_LEN     (100 + GPS_Data_MAX + GSM_Data_MAX + 2 + MSG_EVENT_SET_MAX + 20)//系统参数 + GPS参数 + GSM参数 + CRC2  //+MSG_EVENT_SET_MAX事件信息标志位
//																																											// +20 106505 增加转数油耗保存
//#define  SYS_PARA_EE_MAX       (SYS_PARA_EE_ADDR + SYS_PARA_DATA_LEN)
//
//#define  FORK_LOCK_EE_ADDR     (SYS_PARA_EE_MAX + 1)
//#define  FORK_LOCK_DATA_LEN    (83 + 2)//系统参数 + CRC2
//#define  FORK_LOCK_EE_MAX      (FORK_LOCK_EE_ADDR + FORK_LOCK_DATA_LEN)
//
//#define  FORK_PARA_EE_ADDR     (FORK_LOCK_EE_MAX + 1)
//#define  FORK_PARA_DATA_LEN    (238 + 2)//系统参数 + CRC2
//#define  FORK_PARA_EE_MAX      (FORK_PARA_EE_ADDR + FORK_PARA_DATA_LEN)
//
//#define  FORK_SET_EE_ADDR      (FORK_PARA_EE_MAX + 1)
//#define  FORK_SET_DATA_LEN     (34 + 2)//系统参数 + CRC2
//#define  FORK_SET_EE_MAX       (FORK_SET_EE_ADDR + FORK_SET_DATA_LEN)
//
//#define  DEER_DATA_EE_ADDR     (FORK_SET_EE_MAX+1)                           //保存E2的块位置
//#define  DEER_DATA_DATA_LEN    ((uint16_t)(280 + 2))                         //数据长度10个故障
//#define  DEER_DATA_EE_MAX      ((long)(DEER_DATA_EE_ADDR+DEER_DATA_DATA_LEN)) //最大使用地址
//
//#define  IMU_DATA_EE_ADDR      (DEER_DATA_EE_MAX+1)                          //保存IMU参数
//#define  IMU_DATA_DATA_LEN     (128)
//#define  IMU_DATA_EE_MAX       (IMU_DATA_EE_ADDR + IMU_DATA_DATA_LEN)
//
//#define SYS_PARAM_MAX_EE DEER_DATA_EE_MAX

// 运行参数过多，需要使用DATAFLASH保存
#define Sys_CanSetPar_Sign	(0x01+SAVE_VER)
#define Sys_CanSetParLen   (1+MSG_PGN_SET_DATAMAX)
#define  SYS_CANSET_EE_ADDR    (264 + 1)
#define  SYS_CANSET_EE_MAX     (SYS_CANSET_EE_ADDR + Sys_CanSetParLen + 3)

#define Sys_EventTransSetPar_Sign	(Sys_CanSetPar_Sign+1)
#define Sys_EventTransSetParLen   (1+MSG_EVENT_TRANS_SET_DATAMAX+1+MSG_EVENT_SET_DATAMAX)
#define  SYS_EVEN_EE_ADDR      (SYS_CANSET_EE_MAX + 1)
#define  SYS_EVEN_EE_MAX       (SYS_EVEN_EE_ADDR + Sys_EventTransSetParLen + 3)

#define DTU_STAT_SET_SIGN (Sys_EventTransSetPar_Sign + 1)
#define DTU_STAT_SET_LEN  (sizeof(stat_set))
#define DTU_STAT_SET_ADDR (SYS_EVEN_EE_MAX + 1)
#define DTU_STAT_SET_MAX  (DTU_STAT_SET_ADDR + DTU_STAT_SET_LEN + 3)

#define DTU_TX_SET_SIGN (DTU_STAT_SET_SIGN + 1)
#define DTU_TX_SET_LEN  (1+sizeof(dtu_tx_set))
#define DTU_TX_SET_ADDR (DTU_STAT_SET_MAX + 1)
#define DTU_TX_SET_MAX  (DTU_TX_SET_ADDR + DTU_TX_SET_LEN + 3)

#define DTU_STAT_PARA_LEN  (2 + sizeof(stat_val))
#define DTU_STAT_PARA_ADDR (DTU_TX_SET_MAX + 1)
#define DTU_STAT_PARA_MAX  (DTU_STAT_PARA_ADDR + DTU_STAT_PARA_LEN + DTU_STAT_PARA_LEN/(sizeof(MAIN_Buffer_Temp)-2)*5)

#define LockCarPar_Sign    (DTU_TX_SET_SIGN + 1)
#define save_LockCarDataLen  287
#define save_LockCarDataAddress (DTU_STAT_PARA_MAX + 1)
#define save_LockCarDataMax  (save_LockCarDataAddress+save_LockCarDataLen+3)

//#define lock_total_work_hours  0      // 当前总工作小时，单位小时
 extern u32 lock_total_work_hours; // 当前总工作小时，单位小时
#define Lock_file   "/ty/ct100d/bin/lock.ty"           //定时透传设置信息保存文件
 extern  u8 lock_state;//锁车状态
/***************************************************************/

extern void LockCarJudge(void);
extern void LockCarSendReport(u8 *dzm);

extern u8 Lock_HandleMiMaUnlock(u8 *data);
extern void Lock_TakeHandShakeMima(u8 *mima, u8 *imei, u8 *imsi);
//extern int gps_format_jwd_mem(u8 *mem);

void SaveLockCarData(void);
void TakeLockCarData(void);
/**********************************************/

#endif
