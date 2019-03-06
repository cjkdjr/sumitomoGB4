
#ifndef __ALARM_H__
#define __ALARM_H__
#include <time.h>

typedef unsigned short int  u16;


#define AlarmSetBit(ID) (Alarm.terminal |= ((u16)0x1<<ID))
#define AlarmClrBit(ID) (Alarm.terminal &= (~((u16)0x1<<ID)))
#define AlarmIsBitH(ID) ((Alarm.terminal & ((u16)0x1<<ID)) != 0)

#define AlarmingSetBit(ID) (Alarming.terminal |= ((u16)0x1<<ID))
#define AlarmingClrBit(ID) (Alarming.terminal &= (~((u16)0x1<<ID)))
#define AlarmingIsBitH(ID) ((Alarming.terminal & ((u16)0x1<<ID)) != 0)

#define AlarmIdBattRemove		15
#define AlarmIdCan0Trans			14
#define AlarmIdWireRemove		13
#define AlarmIdGSMAnt       12  //硬件未预留GSM天线检测
#define AlarmIdGPSAnt       11
#define AlarmIdSIMFail      10
#define AlarmIdGSMCom       9
#define AlarmIdGPSCom		8
#define AlarmIdE2		7
#define AlarmIdRTC		6
#define AlarmIdFlash		5
#define AlarmIdGRY	4  //加速度传感器
#define AlarmIdCan1Trans			3
#define AlarmIdGPSLocation   2  //GPS定位异常报警
#define AlarmIdVedio    1    //视频传输异常

extern void Alarm_Processing(time_t *nowTimer);
extern unsigned int Alarm_Check();

#endif
