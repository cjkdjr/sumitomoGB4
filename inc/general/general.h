/*
 * general.h
 *
 *  Created on: 2017年12月6日
 *      Author: tykj
 */

#ifndef GENERAL_H_
#define GENERAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>

#define MAIN_VERSION  "Ver:ct_600d_107301"  //版本号打印
#define MAIN_DATE     "Date:20181130-09"

#define TRUE                1
#define FALSE               0

extern unsigned int debug_value;
#define printf_gsm(...)   		if(0x01 & debug_value) {printf(__VA_ARGS__);}
#define printf_gps(...)  			if(0x02 & debug_value) {printf(__VA_ARGS__);}
#define printf_can(...)   		if(0x04 & debug_value) {printf(__VA_ARGS__);}
#define printf_msg(...)  		if(0x08 & debug_value) {printf(__VA_ARGS__);}
#define printf_bt(...)				if(0x10 & debug_value) {printf(__VA_ARGS__);}
#define printf_imu(...)			if(0x20 & debug_value) {printf(__VA_ARGS__);}
#define printf_iap(...)  			if(0x40 & debug_value) {printf(__VA_ARGS__);}
#define printf_sys(...)   			if(0x80 & debug_value) {printf(__VA_ARGS__);}

typedef unsigned long 				ulong;
typedef unsigned char 				u8;
typedef unsigned short int 	u16;
typedef unsigned int 			 	u32;
typedef signed int 						s32;
typedef unsigned long long 	u64;

/* Exported types ------------------------------------------------------------*/
typedef union {
	u8 byte;
	struct {
		u8 b0 :1; // LSB
		u8 b1 :1;
		u8 b2 :1;
		u8 b3 :1;
		u8 b4 :1;
		u8 b5 :1;
		u8 b6 :1;
		u8 b7 :1; // MSB
	} bit;
} bytebit;

typedef union {
	u16 word;
	struct {
		u8 b0 :1; // LSB
		u8 b1 :1;
		u8 b2 :1;
		u8 b3 :1;
		u8 b4 :1;
		u8 b5 :1;
		u8 b6 :1;
		u8 b7 :1;
		u8 b8 :1;
		u8 b9 :1;
		u8 b10 :1;
		u8 b11 :1;
		u8 b12 :1;
		u8 b13 :1;
		u8 b14 :1;
		u8 b15 :1; // MSB
	} bit;
} wordbit;

typedef union {
	u32 dword;
	struct {
		u8 b0 :1; // LSB
		u8 b1 :1;
		u8 b2 :1;
		u8 b3 :1;
		u8 b4 :1;
		u8 b5 :1;
		u8 b6 :1;
		u8 b7 :1;
		u8 b8 :1;
		u8 b9 :1;
		u8 b10 :1;
		u8 b11 :1;
		u8 b12 :1;
		u8 b13 :1;
		u8 b14 :1;
		u8 b15 :1;
		u8 b16 :1;
		u8 b17 :1;
		u8 b18 :1;
		u8 b19 :1;
		u8 b20 :1;
		u8 b21 :1;
		u8 b22 :1;
		u8 b23 :1;
		u8 b24 :1;
		u8 b25 :1;
		u8 b26 :1;
		u8 b27 :1;
		u8 b28 :1;
		u8 b29 :1;
		u8 b30 :1;
		u8 b31 :1; // MSB
	} bit;
} dwordbit;

extern u8 dbg_OutputTypeSw[10];
#define MAIN_TIMER_SEC(X) (X)

//extern u8 hw_dog_restflg;
//版本号
struct Version_con {
	const u16 Protocol;
	const u8 Code;
};
extern struct Version_con Version;

extern u8 Serial_Num;

//struct DateTime 时间值
typedef struct {
	unsigned char Year;
	unsigned char Month;
	unsigned char Day;
	unsigned char Hour;
	unsigned char Min;
	unsigned char Sec;
	unsigned char Valid;
} DateTime;

//struct GSMSet_t
typedef struct
{
    u8  CenterSIM1Number[21];
    u8  CenterSIM2Number[21];
    u8  MessCenNum[21];
    u8  UpVcuIP[7];
    u8  ubUpVcuAPN[21];
    u8  IAPCurVersion[12];
}GSMSet_t;
GSMSet_t gsm;

//struct LockSet_t
typedef struct {
	u16 StopTime;
	u16 StartTime;
	u8 WeekTime[7]; /* 0monday 1tuesday ... 6sunday */

	u32 Latitude;
	u32 Longitude;
	u32 Radius;
} LockSet_t;
//struct Terminal diagnostic information(终端诊断信息38byte)
typedef struct {
//	u8	Valid;                 //标志位，0表示没有参数统计,清零，1表示有参数统计
	u16 RestartCount;          //当天上电复位次数
	u16 WDRestartCount;        //当天清狗复位次数
	u16 SoftRestartCount;      //当天软件复位次数
	u16 RSTResetartCount;      //当天正常关机次数
	u16 AccOnCount;            //当天车钥匙接通次数
	u16 EngOnCount;           //当天发动机启动次数
	u16 ConnCtrlAOKCount; //当天链接控制器A成功次数
	u16 SendNACKCount;      //当天发送NACK次数
	u16 SendCtrlFailCount;   //当天回复控制器A失败次数
	u16 RecvECMDataFailCount; //当天接收ECM数据失败次数
	u16 GSMWorkState;        //当天GSM工作状态
	u8 RTTemperatureMAX; //当天终端最高温度
	u8 RTTemperatureMIN; //当天终端最低温度
	u8 GSMOnCount;             //当天GSM模块开启次数
	u8 GPSOnCount;              //当天GPS模块开启次数
	u8 E2OverflowCount;     //当天E2溢出次数
	u16 MessMakeNum;        //当天信息生成条数
	u16 E2QueInFailCount;   //当天存E2队列错误次数
	u16 E2QueOutFailCount; //当天取E2队列错误次数
	u8 SIMSendNum;            //SIM发送次数
	u8 GPSSetWay;                 //GPS设置方式
	u8 GPSSetCount;             //GPS设置次数
	u16 SIMCheckFailCount; //SIM卡检测当天失败次数
	u16 UselessNum;  		   //当天删除非法短信条数
    u8  SysTimeResetCount; //当天设置系统时间次数
} RTDiagnosticInfo;

//stuct GPS_struct经纬度格式
typedef struct
{
   u32 Longitude;
   u32 Latitude;
   u16 Altitude;
   u8  Valid;
} __attribute__((packed)) GPS_Pos_t;
//stuct GPS_struct
typedef struct
{
    DateTime    Time;
    GPS_Pos_t   Position;
    u8  Flags;
    u8  StarNum;
    u16  Azimuth;
    u16 Speed;
    u16  HoriPrec;  //u8->u16 byzjx 2018-6-29
    u8  Valid;
}GPS_struct;

GPS_struct  GPSRight;

//struct GSMStation
typedef struct {
	u16 GSM_AreaCode;
	u32 GSM_CellID;
} GSMStation;

//DataCollectionParam 数据收集参数格式
typedef struct {
	u16 EngTorqueAvr; //发动机扭矩平均值
	u32 EngTorqueStdDeviationSquar; //发动机扭矩标准偏差平方
	u16 EngRevolutionAvr; //发动机实际转数平均值
	u32 EngRevolutionStdDeviationSquar; //发动机实际转数标准偏差平方
	u16 CoolWaterTemperatureAvr; //冷却水温平均值
	u16 CoolWaterDeviation; //冷却水温偏差
	u16 FurlTemperatureAvr; //燃料温度平均值
	u16 BreatheTemperatureAvr; //吸气温度平均值
	u16 PressureAvr; //大气压平均值
	u16 IntakePressureAfterPressurizationAvr; //增压后进压力平均值
	u32 IntakePressureAfterPressurizationDeviationSquar; //增压后进气压力标准偏差平方
	u16 IntakePressureAfterPressurizationRange; //增压后进气压力范围
	u16 IntakePressureAfterPressurizationDeviation; //增压后进气压力偏差
	u16 IntakePressureAfterTemperatureAvr; //增压后进气温度平均值
	u16 P1PressureAvr; //p1压力平均值
	u32 P1PressureDeviationSquar; //p1压力标准偏差平方
	u16 P2PressureAvr; //p2压力平均值
	u32 P2PressureDeviationSquar; //p2压力标准偏差平方
	u16 PumpCurrentValueAvr; //泵电流值平均值
	u16 HydraulicOilTemperatureAvr; //液压油温平均值
	u16 CommonRailPressureAvr; //共轨压力平均值
	u32 CommonRailRressureStandardDeviationSquared; //共轨压力标准偏差平方
	u16 OilPressureAvr; //机油压力平均值
	u32 OilPressureStandardDeviationSquared; //机油压力标准偏差平方
	u16 PressurizedPumpInletPressureAverage; //加压泵入口压平均值
	u16 EGROpeningAvr; //EGR开度平均值
	u16 CommonRailAvr; //共轨差圧平均值
	u32 CommonRailDiffStandardDeviationSquare; //共轨差圧标准偏差平方
	u16 AirVolume; //吹风风量
	u16 SetTemperature; //设定温度
	u16 ACSetting; //A/C设定
	u16 CommonRailAvr_SH180; //共轨差圧平均值 SH180机型专用参数
	u32 CommonRailDiffStandardDeviationSquare_SH180; //共轨差圧标准偏差平方 SH180机型专用参数
} DataCollectionParam;

typedef struct {
	u8 all;
	u8 sys;
	u8 dog;
	u8 gsm;
	u8 gps;
	u8 can;
	u8 comm;
	u8 mess;
	u8 iap;
	u8 gene;
	u8 cmd;
} PTHREAD_PENDING_TDF;
PTHREAD_PENDING_TDF PthreadPending;

typedef struct
{
    u16 terminal;
}Alarm_flags;

extern Alarm_flags  Alarm,Alarming,Alarmed;

typedef struct {
	u8 AccState;
	u8 BattState;
	u8 EngineState;
	u8 IsFirstAssembly;
	u8 IsReSet;

	u8 GsmCSQ;
	u32 GSM_AreaCode;
	u32 GSM_CellID;

	GPS_struct GPS;
        GSMSet_t    GSMSet;
	u32 CarLockState;

	RTDiagnosticInfo DaySummary;

	DateTime Record;
	u8 OverDaySendFlag;
	u8 OverMonSendFlag;
	u8 HalfDaySendFlag;
	u32 WorkHourChange;
	Alarm_flags Alarm, Alarming1,Alarming2,Alarmed1,Alarmed2;
} System_t;
System_t System;

#define MesssgeDataSize 1038
typedef struct QueueType {
	u8 MsgType; //信息类型
	u8 Version[3]; //版本号
	u8 Time[8]; //信息生成时间
	u8 Attribute[2]; //属性
	u8 length[2]; //消息体长度
	u8 data[MesssgeDataSize]; //消息体
} QUE_TDF_QUEUE_MSG; // 定义类型为队列的一个消息体元素
QUE_TDF_QUEUE_MSG Message;
u16 Socket_Flag;      //socket重新链接标志，置1需要重新链接
u8  Abnormal_power; //上电标志位 1：异常断电;0正常关机；2休眠唤醒状态

extern u32 Sys_Start_Time;

typedef struct{
	char SCA[16];		/*sms center number*/
	char TPA[16];		/*target phone number*/
	char TP_PID;		/*message protocol identifier*/
	char TP_DCS;		/*message code methods*/
	char TP_SCTS[16];	/*timestamping*/
	char TP_UD[161];	/*user information*/
	char index;		    /* wy! user info len */
}PDU_PARAM;

typedef struct
{
    u8 MsgType;	//信息类型
    u8 SerialNum[3];//信息对照码
    u8 Attribute[2];	//属性
    u16 MsgLen;		//长度
    u8 data[1500];	//消息体
}RecMessage;
extern RecMessage RecvMsg;

/*-----------------------------------------------------------------------------------------------------------------------
 *	 线程相关
 *-----------------------------------------------------------------------------------------------------------------------*/
//  线程属性结构体
typedef struct {
	pthread_t id;        				//线程ID
	pthread_attr_t attr; 			//线程属性
	int sta;									//运行状态 0未创建 1运行中，由线程自动置‘1’、‘2’，看门狗线程定时设置为2
	bool flag;          			 		//线程开关
	time_t lastOKtime_t; 		//线程正常的最后时间
	u8 rest_count;  					//线程重启次数
} TDF_PTHREAD_PARAM;
// 线程属性
extern TDF_PTHREAD_PARAM main_PthParam;
extern TDF_PTHREAD_PARAM SockRecv_PthParam;
extern TDF_PTHREAD_PARAM SockSend_PthParam;
extern TDF_PTHREAD_PARAM gps_PthParam;
extern TDF_PTHREAD_PARAM gsm_PthParam;
extern TDF_PTHREAD_PARAM sys_PthParam;
extern TDF_PTHREAD_PARAM can_PthParam;
extern TDF_PTHREAD_PARAM can_PthParamA;
extern TDF_PTHREAD_PARAM can_PthParamRecvB;
extern TDF_PTHREAD_PARAM can_PthParamSendB;
extern TDF_PTHREAD_PARAM can_PthParamRecvC;
extern TDF_PTHREAD_PARAM can_PthParamSendC;
extern TDF_PTHREAD_PARAM iap_PthParam;
extern TDF_PTHREAD_PARAM bt_PthParam;
extern TDF_PTHREAD_PARAM Camera_PthParam;
extern TDF_PTHREAD_PARAM Alarm_PthParam;
extern TDF_PTHREAD_PARAM Message_PthParam;
extern TDF_PTHREAD_PARAM Cand_Msg_PthParam;
extern TDF_PTHREAD_PARAM shell_PthParam;
extern TDF_PTHREAD_PARAM Wdg_PthParam;
// 线程锁
extern pthread_mutex_t CanAmutex;
extern pthread_mutex_t CanBmutex;
extern pthread_mutex_t CanCmutex;
extern pthread_mutex_t InterStaQuemutex;
extern pthread_mutex_t PassThroughQuemutex;;
extern pthread_mutex_t ATCmdMutex;
extern pthread_mutex_t LockSetMutex;
extern pthread_mutex_t can_vcumutex;

/*-----------------------------------------------------------------------------------------------------------------------
 *	 看门狗
 *-----------------------------------------------------------------------------------------------------------------------*/
extern int wtd_fd;

extern unsigned char TimeIndex;
extern long StayMinute;
extern void cpyVersion(u8 *des);//版本号赋值
extern u32 cpyInfoTime(u8 *des);//信息生成时间+流水号赋值

#endif /* GENERAL_H_ */
