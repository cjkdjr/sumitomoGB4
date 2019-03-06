/*
 * canhandle.c
 *
 *  Created on: 2019年1月9日
 *      Author: guozhiyue
 */

#include <unistd.h>

#include "api.h"
#include "general.h"
#include "message_process.h"
#include "candata.h"
#include "canhandle.h"
#include "e2p.h"
#include "Msg_task.h"


sumitomo_parm_grp_0001_t sumitomo_parm_grp_0001;
QUE_TDF_QUEUE_MSG Msg_22_Run;
QUE_TDF_QUEUE_MSG Msg_22_Stay;
QUE_TDF_QUEUE_MSG Msg_22_Off;
long StayMinute = 0;

/*****************************************
 初期设定信息：0x30
 *****************************************/
u32 Sys_Run_Time;        //系统开机后的运行时间---局部变量

QUE_TDF_QUEUE_MSG Msg_30;
u16 MSG_30_Index = 0;
u32 msg_strat_time_ms = 0;
u32 MSG_30_lastTime;
void Make_30_Msg() {
	struct tm *p;
	struct timeval tv;
	u16 tmp_sec;

	//信息类型
	Msg_30.MsgType = 0x30;
	//版本号
	Msg_30.Version[0] = Version.Protocol >> 8;
	Msg_30.Version[1] = Version.Protocol;
	Msg_30.Version[2] = Version.Code;
	//属性标识  高字节在前
	Msg_30.Attribute[0] = 0xC0;
	Msg_30.Attribute[1] = 0;

	//获取当前时间 = 系统运行时间+ key on时间（RTC时间）
//	Sys_Run_Time = api_GetSysSecs();
//	now_time = Sys_Start_Time + Sys_Run_Time;
//	p = localtime(&now_time);
	//获取当前时间，年月日时分+两字节毫秒+一字节流水号
	gettimeofday(&tv, NULL);
	p = localtime(&tv.tv_sec);
	Msg_30.Time[0] = p->tm_year - 100;
	Msg_30.Time[1] = p->tm_mon + 1;
	Msg_30.Time[2] = p->tm_mday;
	Msg_30.Time[3] = p->tm_hour;
	Msg_30.Time[4] = p->tm_min;
//	Msg_30.Time[5] = p->tm_sec;
	tmp_sec = p->tm_sec * 1000 + tv.tv_usec / 1000;
	Msg_30.Time[5] = tmp_sec>>8;
	Msg_30.Time[6] = tmp_sec;
	if(Serial_Num <  0xFF){
		Serial_Num++;
	}else {
		Serial_Num = 0;
	}
	Msg_30.Time[7] = Serial_Num;


	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) MSG_30_Index); //消息体长度
	api_HexToAsc(len, Msg_30.length, strlen((char*) len));
	//信息入队列
	printf_can("<Make_30_message>message ready in queue %d bytes!!! \n", MSG_30_Index + 14);
	InterSta_in(&Msg_30);
	memset(&Msg_30, 0x00, sizeof(QUE_TDF_QUEUE_MSG));
	MSG_30_Index = 0;
}


// 发动机状态判断用到的变量和标志位
static bool EngineState = false;
static bool AlternatorState = false;
static bool EngineRpmState = false;
static int Alternator = 0;
static int AlternatorStart = 0;
static int EngineRpm = 0;
static int EngineRpmStart = 0;

// 关车钥匙发送信息的标志
bool AccState = true;
bool AccFlag = false;
static bool AccMsgFlag = false; // 发送关机信息之后，这个标志位变成true

// 定时半小时发送信息的48个时间段的数组
static bool MsgRunArr[48] = {0};

// 开机记录定时发送相对于上一个时间点的偏移量
static int TimingOffset = 0;

// 现在的时间点下标
unsigned char TimeIndex = 0;

// 现在的时间点下标和现在相对于上一个时间点的偏移量
static int CurOffset = 0;


/*
*函数功能:  获取系统启动到现在的时间
*输入:      void
*返回值:    系统从开始到现在的时间
*/
int GetSysTime(void) {
    struct timespec sysTime = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &sysTime);
    return sysTime.tv_sec;
}

/*
*函数功能:  检测Alternator信号是否持续1min
*输入:      void
*返回值:    满足触发条件返回1，没有满足条件返回0
*/
int CheckAlternator(void) {
    int CurTime = 0;

    // 检测到交流电信号
    if (Alternator >= 10) {
        // 只有从无到有的时候开始计时
        if (AlternatorState == false) {
            AlternatorStart = GetSysTime();
        }
        AlternatorState = true;
        // 只有发动机没有启动的时候统计时间
        if (EngineState == false) {
            CurTime = GetSysTime();
            // 检测到Alternator信号持续一分钟
            if (CurTime - AlternatorStart > 5) {
                return 1;
            }
        }
    }else {
        AlternatorState = false;
    }
    return 0;
}

/*
*函数功能:  检测发动机转数是否大于500转持续1min
*输入:      void
*返回值:    满足触发条件返回1，没有满足条件返回0
*/
int CheckEngineRpm(void) {
    int CurTime = 0;

    // 检测到发动机转数超过500
    if (EngineRpm >= 500) {
        // 只有从无到有的时候才开始计时
        if (EngineRpmState == false) {
            EngineRpmStart = GetSysTime();
        }
        EngineRpmState = true;
        // 只有发动机没有启动的时候统计时间
        if (EngineState == false) {
            CurTime = GetSysTime();
            // 检测到发动机转数超过500持续一分钟
            if (CurTime - EngineRpmStart > 5) {
                return 1;
            }
        }
    }else {
        EngineRpmState = false;
    }
    return 0;
}

/*
*函数功能:  检测发动机状态，给发动机状态全局变量赋值
*输入:      void
*返回值:    void
*/
void CheckEngine(void) {
    if ((Alternator < 10) && (EngineRpm < 500)) {
        EngineState = false;
    }
    if (CheckAlternator() || CheckEngineRpm()) {
        EngineState = true;
    }
}


// 当前时间偏移量和要发送信息的时间偏移量不同时为true，相同时为false
static bool SendRunMsg = false;
/*
*函数功能:  获取当前时间对应的48个时间段下标
*输入:      void
*返回值:    如果时间段切换返回值retValue中最后一个bit位是1，时间偏移量切换返回值reValue中倒数第二个bit位是1
*/
char GetTimeIndex(void) {
    time_t curTime;
    struct tm *tmPtr = NULL;
    int curIndex = 0; // 当前的时间段下标
    char retValue = 0;

    curTime = time(NULL);
    tmPtr = localtime(&curTime);
    curIndex = tmPtr->tm_hour * 2 + (tmPtr->tm_min >= 30 ? 1 : 0);
    CurOffset = (tmPtr->tm_min >= 30 ? (tmPtr->tm_min - 30) : tmPtr->tm_min);
    if (curIndex == 48) {
        curIndex = 0;
    }
    tmPtr = NULL;


    // 当前时间偏移量和要发送信息的时间偏移量从不同变为相同时
    if (CurOffset != TimingOffset) {
        SendRunMsg = true;
    } else if ((CurOffset == TimingOffset) && (SendRunMsg == true)) {
        SendRunMsg = false;
        retValue |= 0x02;
    }

    if (curIndex != TimeIndex) {
        TimeIndex = curIndex;
    	retValue |= 0x01;
    }
    return retValue;
}

/*
*函数功能:  判断哪个时间段生成信息
*输入:      void
*返回值:    void
*/
void CheckMakeMsg(void) {
    int hour = 0;
    int min = 0;

    if (EngineState == true) {
        MsgRunArr[(TimeIndex + 1) % 48] = true;
        printf("MsgRunArr[%d]\n", (TimeIndex + 1) % 48);
        printf("MsgRunArr %d\n", TimeIndex);
        hour = (TimeIndex + 1) / 2;
        min = (TimeIndex + 1) % 2;
        printf("%02d点%02d分生成信息\n", hour, min * 30);
        StayMinute = time(NULL) / 60 + 3 * 60 + (30 - CurOffset);
    }
}

/*
*函数功能:  检测到关车钥匙，生成一条定时信息
*输入:      void
*返回值:    void
*/
void CheckPowerOff(void) {
	// 只有程序运行的时候开过车钥匙，才会执行关机发送信息的逻辑
	if (System.AccState == true) {
		AccFlag = true;
	    // 防止误关车钥匙的操作
		AccMsgFlag = false;
	}

	if (System.AccState == false && (time(NULL) / 60 >= StayMinute)) {
		system("date");
		printf("发送待机三小时的信息\n");
        MsgMake_22(&Msg_22_Stay);
    	//信息入队列
    	InterSta_in(&Msg_22_Stay);
    	StayMinute += 3 * 60;
	}
    // 车钥匙开过AccFlag == true,车钥匙关了以后只发一次AccMsgFlag == false
	 if (System.AccState == false && AccMsgFlag == false && AccFlag == true) {
        // 如果上一条生成的信息还没有发送
		 if (MsgRunArr[TimeIndex] == true) {
			 printf("关车钥匙发送已生成未发送的信息\n");
             InterSta_in(&Msg_22_Run);
             MsgRunArr[TimeIndex] = false;
		 }
#if 0
		 // 关车钥匙的时候还没有切换时间段
		 if (MsgRunArr[TimeIndex + 1] == true) {
			 MsgRunArr[TimeIndex + 1] = false;
		 }
#endif

		 printf("生成发送关车钥匙的信息\n");
		 MsgMake_22(&Msg_22_Off);
		 InterSta_in(&Msg_22_Off);
		 AccMsgFlag = true; // 信息发送过了，就不发送了
    }
}

/*
*函数功能:  开机获取时间相对于48个时间段的偏移量
*输入:      void
*返回值:    void
*/
void GetTimingOffset(void) {
    time_t curTime;
    struct tm *tmPtr = NULL;

    curTime = time(NULL);
    tmPtr = localtime(&curTime);
    TimingOffset = (tmPtr->tm_min >= 30 ? (tmPtr->tm_min - 30) : tmPtr->tm_min);
    tmPtr = NULL;
}

void *canh_TransProcess(void *argc) {
	printf("############## %s start ##############\n", __FUNCTION__);
	can_PthParam.sta = 1;
	sleep(1);
	can_PthParam.flag = TRUE;
	// 上电获取相对于整半点的时间偏差
    GetTimingOffset();
//    printf("TimingOffset %d\n", TimingOffset);
    // 获取应该发送休眠信息的时间
    if (StayMinute == 0) {
        StayMinute = time(NULL) / 60 + 3 * 60;
    }
    memset(&sumitomo_parm_grp_0001, 0, sizeof(sumitomo_parm_grp_0001));

	while (can_PthParam.flag) {
		int ret = -1;
		can_PthParam.sta = 1;
#if 0
		if((System.EngineState == 1 && System.AccState == 1) || System.IsReSet == 1){
			//GPS初次组装且keyOn状态收到初期设定信息或者GPS 执行全复位，生成初期设定信息，收到<初期设定回复信息>之前，每15分钟重复生成一次该信息
			Make_30_Msg();
		}
#endif

        ret = GetTimeIndex();
        // 当前时间偏移量和要发送信息的时间偏移量从不同变为相同时，检测要不要发送运行半小时的信息
        if ((ret & 0x02) != 0) {
            if (MsgRunArr[TimeIndex] == true) {
            	MsgRunArr[TimeIndex] = false;
            	system("date");
            	printf("发送每半小时的信息\n");
                InterSta_in(&Msg_22_Run);
            }
        }
        // 在时间点切换的时候检测要不要生成每半小时的信息，发送待机3小时的信息
        if ((ret & 0x01) != 0) {
        	// 发动机运行状态下每半小时生成信息
        	if (MsgRunArr[TimeIndex] == true && System.AccState == true) {
        		printf("发动机启动，每半小时生成的信息\n");
                MsgMake_22(&Msg_22_Run);
        	}
        	// 发动机待机状态下，每三小时生成发送信息
        	if(time(NULL) / 60 + 1 >= StayMinute) {
				system("date");
				printf("发送待机三小时的信息\n");
                MsgMake_22(&Msg_22_Stay);
            	//信息入队列
            	InterSta_in(&Msg_22_Stay);
            	StayMinute += 3 * 60;
			}
        }
        if (access("/tmp/Engine", F_OK) != -1) {
        	EngineRpm = 600;
        }else {
        	EngineRpm = 0;
        }
        // 检测发动机状态
        CheckEngine();
        // 不断更新生成定时信息的数组
        CheckMakeMsg();
        CheckPowerOff();

		usleep(1000 * 1000);
	}
	printf("############## %s exit ##############\n", __FUNCTION__);
	return 0;
}

