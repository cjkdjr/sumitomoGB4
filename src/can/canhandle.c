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
#include "Msg_task.h"

sumitomo_parm_grp_0001_t sumitomo_parm_grp_0001;
QUE_TDF_QUEUE_MSG Msg_22_Run;
QUE_TDF_QUEUE_MSG Msg_22_Stay;
QUE_TDF_QUEUE_MSG Msg_22_Off;
long StayMinute = 0;

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
static bool MsgRunArr[48] = { 0 };

// 开机记录定时发送相对于上一个时间点的偏移量
static int TimingOffset = 0;

// 现在的时间点下标
static unsigned char TimeIndex = 0;

// 现在的时间点下标和现在相对于上一个时间点的偏移量
static int CurOffset = 0;

u8 cand_Init_Set; //初期设定状态标志位,0x00-初期设定未完成，0x01-初期设定已完成

void Make_Sumitomo_0001_s(sumitomo_parm_grp_0001_t *sumitomo_0001) {
	//TODO
	//	bit_zone_comm_t bzcommon;	//通用位域结构体

	//	u8 UTC[6];	//GPS时间
	//u8 longitude_and_latitude[12];	//经纬度

	//u8 alternator_signal[2];	//发动机运行情况
	//u8 engine_speed[2];
	//u8	ENGON_OFF;
	//u8	KEYON_OFF;
	//	memcpy(sumitomo_0001->Hour_Meter,TdfSetData.MainCtlrSts.HourMeter, 3);
	sumitomo_0001->Hour_Meter[0] = TdfSetData.MainCtlrSts.HourMeter >> 16;
	sumitomo_0001->Hour_Meter[1] = TdfSetData.MainCtlrSts.HourMeter >> 8;
	sumitomo_0001->Hour_Meter[2] = TdfSetData.MainCtlrSts.HourMeter;

	sumitomo_0001->Manual_Lock = TdfSetData.MainCtlrSts.ManualLock;
	sumitomo_0001->Back_Monitor_Lock = TdfSetData.MainCtlrSts.BackMonitorLock;
	sumitomo_0001->Fuel_Lv = TdfSetData.MainCtlrSts.FuelLv;

}

void Make_Sumitomo_0002_s(sumitomo_parm_grp_0002_t *sumitomo_0002) {
	memcpy(sumitomo_0002->MainteTimeFromWeb_1, TdfSetData.MainteTimeFromWeb, 6);
	memcpy(sumitomo_0002->MainteTimeFromWeb_2, TdfSetData.MainteTimeFromWeb + 6,
			6);
	memcpy(sumitomo_0002->MainteTimeFromWeb_3,
			TdfSetData.MainteTimeFromWeb + 12, 6);
	memcpy(sumitomo_0002->MainteTimeFromWeb_4,
			TdfSetData.MainteTimeFromWeb + 18, 6);
	memcpy(sumitomo_0002->MainteTimeFromWeb_5,
			TdfSetData.MainteTimeFromWeb + 24, 6);
}

void Make_Sumitomo_0003_s(sumitomo_parm_grp_0003_t *sumitomo_0003) {
	memcpy(sumitomo_0003->MainteTimeFromWeb_1, TdfSetData.MainteTimeFromWeb, 6);
	memcpy(sumitomo_0003->MainteTimeFromWeb_2, TdfSetData.MainteTimeFromWeb + 6,
			6);
	memcpy(sumitomo_0003->MainteTimeFromWeb_3,
			TdfSetData.MainteTimeFromWeb + 12, 6);
	memcpy(sumitomo_0003->MainteTimeFromWeb_4,
			TdfSetData.MainteTimeFromWeb + 18, 6);
	memcpy(sumitomo_0003->MainteTimeFromWeb_5,
			TdfSetData.MainteTimeFromWeb + 24, 6);
}

void Make_Sumitomo_0004_s(sumitomo_parm_grp_0004_t *sumitomo_0004) {
	/*todo
	 * 	bit_zone_comm_t bzcommon;	//通用位域结构体

	 u8 canbus_c_1CFF1128[6]; //CAN C定期送信
	 u8 canbus_b_18FEEB00[6];
	 *
	 *
	 */
	memcpy(sumitomo_0004->MainCtlrPartsNum_1, TdfSetData.MainCtltPartsNum, 6);
	memcpy(sumitomo_0004->MainCtlrPartsNum_2, TdfSetData.MainCtltPartsNum + 6,
			6);
	memcpy(sumitomo_0004->MainCtlrSerialNum_1, TdfSetData.MainCtltSerialNum, 6);
	memcpy(sumitomo_0004->MainCtlrSerialNum_2, TdfSetData.MainCtltSerialNum + 6,
			5);
	memcpy(sumitomo_0004->SubCtlrFirmwareVer, TdfSetData.SubCtlrFirmwareVer, 6);
	memcpy(sumitomo_0004->SubCtlrSerialNum_1, TdfSetData.SubCtlrSerialNum, 6);
	memcpy(sumitomo_0004->SubCtlrSerialNum_2, TdfSetData.SubCtlrSerialNum + 6,
			6);
	memcpy(sumitomo_0004->MonitorNum, TdfSetData.MonitorNum, 4);
	memcpy(sumitomo_0004->MonitorSerialNum, TdfSetData.MonitorSerialNum, 6);
	memcpy(sumitomo_0004->CtlrTPartsNum_1, TdfGetData.CtlrTPartsNum, 6);
	memcpy(sumitomo_0004->CtlrTPartsNum_2, TdfGetData.CtlrTPartsNum + 6, 6);
	memcpy(sumitomo_0004->McnInfo_1, &TdfSetData.McnInfo, 6);
	memcpy(sumitomo_0004->McnInfo_2, &TdfSetData.McnInfo + 6, 6);
	gps_format_jwd_mem(sumitomo_0004->longitude_and_latitude);
	//todo memcpy(sumitomo_0004->longitude_and_latitude, , );
}

void Make_Sumitomo_0005_s(sumitomo_parm_grp_0005_t *sumitomo_0005) {
	/*todo
	 * 	bit_zone_comm_t bzcommon;	//通用位域结构体
	 */
	memcpy(sumitomo_0005->MachineParameter_1, TdfSetData.MachineParameter_1, 6);
	memcpy(sumitomo_0005->MachineParameter_2, TdfSetData.MachineParameter_2, 6);
	memcpy(sumitomo_0005->MachineParameter_3, TdfSetData.MachineParameter_3, 6);
	memcpy(sumitomo_0005->MachineParameter_4, TdfSetData.MachineParameter_4, 6);
	memcpy(sumitomo_0005->MachineParameter_5, TdfSetData.MachineParameter_5, 6);
	memcpy(sumitomo_0005->MachineParameter_6, TdfSetData.MachineParameter_6, 6);
}

void Make_Sumitomo_0006_s(sumitomo_parm_grp_0006_t *sumitomo_0006) {
	/*todo
	 * 	bit_zone_comm_t bzcommon;	//通用位域结构体
	 */
	memcpy(sumitomo_0006->ParameterConfig, TdfGetData.ParameterConfig, 6);
	//todo  sumitomo_0006->response_type =
}

//todo void Make_Sumitomo_0007_s(sumitomo_parm_grp_0007_t *sumitomo_0007)

void Make_Sumitomo_0008_s(sumitomo_parm_grp_0008_t *sumitomo_0008) {
	/*todo
	 * 	bit_zone_comm_t bzcommon;	//通用位域结构体
	 */
	sumitomo_0008->Machine_Reset_Request = cand_command_reset;
	sumitomo_0008->response_type = 0x01;	//0x00收到设置立即回复，0x01设置成功
	//todo  sumitomo_0006->response_type =
}

void Make_Sumitomo_0009_s(sumitomo_parm_grp_0009_t *sumitomo_0009) {
	/*todo
	 * 	bit_zone_comm_t bzcommon;	//通用位域结构体
	 */
	memcpy(sumitomo_0009->EngineDeviceTestRequest,
			TdfGetData.EngineDeviceTestRequest, 6);
	//todo  sumitomo_0006->response_type =
}

//todo void Make_Sumitomo_0600_s(sumitomo_parm_grp_0600_t *sumitomo_0600)

//todo void Make_Sumitomo_0800_s(sumitomo_parm_grp_0800_t *sumitomo_0800)

void Make_Sumitomo_0700_s(sumitomo_parm_grp_0700_t *sumitomo_0700) {
	/*todo
	 * 	bit_zone_comm_t bzcommon;	//通用位域结构体
	 */
	memcpy(sumitomo_0700->Work_1, TdfSetData.Work, 6);
	memcpy(sumitomo_0700->Work_2, TdfSetData.Work + 6, 3);
	memcpy(sumitomo_0700->UpperWork_1, TdfSetData.UpperWork, 6);
	memcpy(sumitomo_0700->UpperWork_2, TdfSetData.UpperWork + 6, 6);
	memcpy(sumitomo_0700->UpperWork_3, TdfSetData.UpperWork + 12, 6);
	memcpy(sumitomo_0700->UpperWork_4, TdfSetData.UpperWork + 18, 6);
	memcpy(sumitomo_0700->UpperWork_5, TdfSetData.UpperWork + 24, 6);
	memcpy(sumitomo_0700->UpperWork_6, TdfSetData.UpperWork + 30, 3);
	memcpy(sumitomo_0700->TravelWork_1, TdfSetData.TravelWork, 6);
	memcpy(sumitomo_0700->TravelWork_2, TdfSetData.TravelWork + 6, 6);
	memcpy(sumitomo_0700->TravelWork_3, TdfSetData.TravelWork + 12, 6);
	memcpy(sumitomo_0700->WorkMode_1, TdfSetData.WorkMode, 6);
	memcpy(sumitomo_0700->WorkMode_2, TdfSetData.WorkMode + 6, 6);
	memcpy(sumitomo_0700->WorkMode_3, TdfSetData.WorkMode + 12, 6);
	memcpy(sumitomo_0700->WorkMode_4, TdfSetData.WorkMode + 18, 6);
	memcpy(sumitomo_0700->WorkMode_5, TdfSetData.WorkMode + 24, 6);
	memcpy(sumitomo_0700->Breaker_Operation_Time, TdfSetData.Breaker + 3, 3);
	memcpy(sumitomo_0700->Crusher_Operation_Time, TdfSetData.Crusher + 3, 3);
	memcpy(sumitomo_0700->Option_Line_Operation_Time, TdfSetData.OptionLine, 3);
	memcpy(sumitomo_0700->Option_Line_Operation_Time_2nd,
			TdfSetData.OptionLine + 9, 3);
	memcpy(sumitomo_0700->Overheat_Power_Reduction_Time,
			TdfSetData.OverheatPowerReduction, 3);

}

void Make_Sumitomo_1301_s(sumitomo_parm_grp_1301_t *sumitomo_1301) {
	/*todo
	 * 	bit_zone_comm_t bzcommon;	//通用位域结构体
	 */
	memcpy(sumitomo_1301->Work_1, TdfSetData.Work, 6);
	memcpy(sumitomo_1301->Work_2, TdfSetData.Work + 6, 3);
	memcpy(sumitomo_1301->UpperWork_1, TdfSetData.UpperWork, 6);
	memcpy(sumitomo_1301->UpperWork_2, TdfSetData.UpperWork + 6, 6);
	memcpy(sumitomo_1301->UpperWork_3, TdfSetData.UpperWork + 12, 6);
	memcpy(sumitomo_1301->UpperWork_4, TdfSetData.UpperWork + 18, 6);
	memcpy(sumitomo_1301->UpperWork_5, TdfSetData.UpperWork + 24, 6);
	memcpy(sumitomo_1301->UpperWork_6, TdfSetData.UpperWork + 30, 3);
	memcpy(sumitomo_1301->TravelWork_1, TdfSetData.TravelWork, 6);
	memcpy(sumitomo_1301->TravelWork_2, TdfSetData.TravelWork + 6, 6);
	memcpy(sumitomo_1301->TravelWork_3, TdfSetData.TravelWork + 12, 6);
	memcpy(sumitomo_1301->WorkMode_1, TdfSetData.WorkMode, 6);
	memcpy(sumitomo_1301->WorkMode_2, TdfSetData.WorkMode + 6, 6);
	memcpy(sumitomo_1301->WorkMode_3, TdfSetData.WorkMode + 12, 6);
	memcpy(sumitomo_1301->WorkMode_4, TdfSetData.WorkMode + 18, 6);
	memcpy(sumitomo_1301->WorkMode_5, TdfSetData.WorkMode + 24, 6);
	memcpy(sumitomo_1301->OptionLine_1, TdfSetData.OptionLine, 6);
	memcpy(sumitomo_1301->OptionLine_2, TdfSetData.OptionLine + 6, 6);
}

void Make_Sumitomo_1302_s(sumitomo_parm_grp_1302_t *sumitomo_1302) {
	/*todo
	 * 	bit_zone_comm_t bzcommon;	//通用位域结构体
	 */
	memcpy(sumitomo_1302->QuickCoupler, TdfSetData.QuickCoupler, 6);
	memcpy(sumitomo_1302->PumpBackup_1, TdfSetData.PumpBackup, 6);
	memcpy(sumitomo_1302->PumpBackup_2, TdfSetData.PumpBackup + 6, 6);
	memcpy(sumitomo_1302->OverheatPowerReduction_1,
			TdfSetData.OverheatPowerReduction, 6);
	memcpy(sumitomo_1302->OverheatPowerReduction_2,
			TdfSetData.OverheatPowerReduction + 6, 6);
	memcpy(sumitomo_1302->OverheatPowerReduction_3,
			TdfSetData.OverheatPowerReduction + 12, 3);
	memcpy(sumitomo_1302->Solenoid_1, TdfSetData.Solenoid, 6);
	memcpy(sumitomo_1302->Solenoid_2, TdfSetData.Solenoid + 6, 6);
	memcpy(sumitomo_1302->Solenoid_3, TdfSetData.Solenoid + 12, 6);
	memcpy(sumitomo_1302->Solenoid_4, TdfSetData.Solenoid + 18, 6);
	memcpy(sumitomo_1302->Solenoid_5, TdfSetData.Solenoid + 24, 6);
	memcpy(sumitomo_1302->Solenoid_6, TdfSetData.Solenoid + 30, 6);
	memcpy(sumitomo_1302->Solenoid_7, TdfSetData.Solenoid + 36, 6);
	memcpy(sumitomo_1302->Solenoid_8, TdfSetData.Solenoid + 42, 6);
	memcpy(sumitomo_1302->Solenoid_9, TdfSetData.Solenoid + 48, 6);
	memcpy(sumitomo_1302->Solenoid_10, TdfSetData.Solenoid + 54, 6);
	memcpy(sumitomo_1302->ATSReGen_1, TdfSetData.ATSReGen, 6);
	memcpy(sumitomo_1302->ATSReGen_2, TdfSetData.ATSReGen + 6, 6);
	memcpy(sumitomo_1302->ATSReGen_3, TdfSetData.ATSReGen + 12, 6);
	memcpy(sumitomo_1302->ATSReGen_4, TdfSetData.ATSReGen + 18, 6);
	memcpy(sumitomo_1302->ATSReGen_5, TdfSetData.ATSReGen + 24, 6);
	memcpy(sumitomo_1302->ATSReGen_6, TdfSetData.ATSReGen + 30, 6);
	memcpy(sumitomo_1302->ATSReGen_7, TdfSetData.ATSReGen + 36, 6);
	memcpy(sumitomo_1302->ATSReGen_8, TdfSetData.ATSReGen + 42, 6);
	memcpy(sumitomo_1302->ATSReGen_9, TdfSetData.ATSReGen + 48, 6);
	memcpy(sumitomo_1302->ATSReGen_10, TdfSetData.ATSReGen + 54, 6);
	memcpy(sumitomo_1302->ATSReGen_11, TdfSetData.ATSReGen + 60, 6);
}

void Make_Sumitomo_1303_s(sumitomo_parm_grp_1303_t *sumitomo_1303) {
	/*todo
	 * 	bit_zone_comm_t bzcommon;	//通用位域结构体
	 */
	memcpy(sumitomo_1303->HotShutdown, TdfSetData.HotShutdown, 6);
	memcpy(sumitomo_1303->AirConditioner_1, TdfSetData.AirConditioner, 6);
	memcpy(sumitomo_1303->AirConditioner_2, TdfSetData.AirConditioner + 6, 6);
	memcpy(sumitomo_1303->AirConditioner_3, TdfSetData.AirConditioner + 12, 6);
	memcpy(sumitomo_1303->AirConditioner_4, TdfSetData.AirConditioner + 18, 6);
	memcpy(sumitomo_1303->AirConditioner_5, TdfSetData.AirConditioner + 24, 6);
	memcpy(sumitomo_1303->AirConditioner_6, TdfSetData.AirConditioner + 30, 6);
	memcpy(sumitomo_1303->AirConditioner_7, TdfSetData.AirConditioner + 36, 6);
	memcpy(sumitomo_1303->AirConditioner_8, TdfSetData.AirConditioner + 42, 6);
	memcpy(sumitomo_1303->KeyOn, TdfSetData.KeyOn, 6);
	memcpy(sumitomo_1303->EngineOn, TdfSetData.EngineOn, 6);
	memcpy(sumitomo_1303->AutoIdle, TdfSetData.AutoIdle, 6);
	memcpy(sumitomo_1303->IdleStop, TdfSetData.IdleStop, 6);
	memcpy(sumitomo_1303->WorkOthers_1, TdfSetData.WorkOthers, 6);
	memcpy(sumitomo_1303->WorkOthers_2, TdfSetData.WorkOthers + 6, 6);
	memcpy(sumitomo_1303->WorkOthers_3, TdfSetData.WorkOthers + 12, 6);
	memcpy(sumitomo_1303->Camera_1, TdfSetData.Camera, 6);
	memcpy(sumitomo_1303->Camera_2, TdfSetData.Camera + 6, 6);
	memcpy(sumitomo_1303->Wiper_1, TdfSetData.Wiper, 6);
	memcpy(sumitomo_1303->Wiper_2, TdfSetData.Wiper + 6, 6);
	memcpy(sumitomo_1303->Window, TdfSetData.Window, 6);
	memcpy(sumitomo_1303->Door, TdfSetData.Door, 6);
}

void Make_Sumitomo_1304_s(sumitomo_parm_grp_1304_t *sumitomo_1304) {
	/*todo
	 * 	bit_zone_comm_t bzcommon;	//通用位域结构体
	 */
	memcpy(sumitomo_1304->MaxMin_1, TdfSetData.MaxMin, 6);
	memcpy(sumitomo_1304->MaxMin_2, TdfSetData.MaxMin + 6, 6);
	memcpy(sumitomo_1304->MaxMin_3, TdfSetData.MaxMin + 12, 6);
	memcpy(sumitomo_1304->MaxMin_4, TdfSetData.MaxMin + 18, 6);
	memcpy(sumitomo_1304->MaxMin_5, TdfSetData.MaxMin + 24, 6);
	memcpy(sumitomo_1304->MaxMin_6, TdfSetData.MaxMin + 30, 6);
	memcpy(sumitomo_1304->MaxMin_7, TdfSetData.MaxMin + 36, 6);
	memcpy(sumitomo_1304->MaxMin_8, TdfSetData.MaxMin + 42, 6);
	memcpy(sumitomo_1304->MaxMin_9, TdfSetData.MaxMin + 48, 6);
	memcpy(sumitomo_1304->MaxMin_10, TdfSetData.MaxMin + 54, 6);
	memcpy(sumitomo_1304->MaxMin_11, TdfSetData.MaxMin + 60, 6);
	memcpy(sumitomo_1304->MaxMin_12, TdfSetData.MaxMin + 66, 6);
	memcpy(sumitomo_1304->MaxMin_13, TdfSetData.MaxMin + 72, 6);
	memcpy(sumitomo_1304->MaxMin_14, TdfSetData.MaxMin + 78, 6);
}

void Make_Sumitomo_0901_s(sumitomo_parm_grp_0901_t *sumitomo_0901) {
	memcpy(sumitomo_0901->P1PressDst_1, TdfSetData.P1PressDst_1, 6);
	memcpy(sumitomo_0901->P1PressDst_2, TdfSetData.P1PressDst_2, 6);
	memcpy(sumitomo_0901->P1PressDst_3, TdfSetData.P1PressDst_3, 6);
	memcpy(sumitomo_0901->P1PressDst_4, TdfSetData.P1PressDst_4, 6);
}

void Make_Sumitomo_0902_s(sumitomo_parm_grp_0902_t *sumitomo_0902) {
	memcpy(sumitomo_0902->P2PressDst_1, TdfSetData.P2PressDst_1, 6);
	memcpy(sumitomo_0902->P2PressDst_2, TdfSetData.P2PressDst_2, 6);
	memcpy(sumitomo_0902->P2PressDst_3, TdfSetData.P2PressDst_3, 6);
	memcpy(sumitomo_0902->P2PressDst_4, TdfSetData.P2PressDst_4, 6);
}

void Make_Sumitomo_0903_s(sumitomo_parm_grp_0903_t *sumitomo_0903) {
	memcpy(sumitomo_0903->N1PressDst_1, TdfSetData.N1PressDst_1, 6);
	memcpy(sumitomo_0903->N1PressDst_2, TdfSetData.N1PressDst_2, 6);
	memcpy(sumitomo_0903->N1PressDst_3, TdfSetData.N1PressDst_3, 6);
	memcpy(sumitomo_0903->N1PressDst_4, TdfSetData.N1PressDst_4, 6);
}

void Make_Sumitomo_0904_s(sumitomo_parm_grp_0904_t *sumitomo_0904) {
	memcpy(sumitomo_0904->N2PressDst_1, TdfSetData.N2PressDst_1, 6);
	memcpy(sumitomo_0904->N2PressDst_2, TdfSetData.N2PressDst_2, 6);
	memcpy(sumitomo_0904->N2PressDst_3, TdfSetData.N2PressDst_3, 6);
	memcpy(sumitomo_0904->N2PressDst_4, TdfSetData.N2PressDst_4, 3);
}

void Make_Sumitomo_0905_s(sumitomo_parm_grp_0905_t *sumitomo_0905) {
	memcpy(sumitomo_0905->P1_P2PressDst_1, TdfSetData.P1P2PressDst_1, 6);
	memcpy(sumitomo_0905->P1_P2PressDst_2, TdfSetData.P1P2PressDst_2, 6);
	memcpy(sumitomo_0905->P1_P2PressDst_3, TdfSetData.P1P2PressDst_3, 6);
	memcpy(sumitomo_0905->P1_P2PressDst_4, TdfSetData.P1P2PressDst_4, 3);
}

void Make_Sumitomo_0906_s(sumitomo_parm_grp_0906_t *sumitomo_0906) {
	memcpy(sumitomo_0906->TravelP1PressDst_1, TdfSetData.TravelP1PressDst_1, 6);
	memcpy(sumitomo_0906->TravelP1PressDst_2, TdfSetData.TravelP1PressDst_2, 6);
	memcpy(sumitomo_0906->TravelP1PressDst_3, TdfSetData.TravelP1PressDst_3, 6);
	memcpy(sumitomo_0906->TravelP1PressDst_4, TdfSetData.TravelP1PressDst_4, 3);
}

void Make_Sumitomo_0907_s(sumitomo_parm_grp_0907_t *sumitomo_0907) {
	memcpy(sumitomo_0907->TravelP2PressDst_1, TdfSetData.TravelP2PressDst_1, 6);
	memcpy(sumitomo_0907->TravelP2PressDst_2, TdfSetData.TravelP2PressDst_2, 6);
	memcpy(sumitomo_0907->TravelP2PressDst_3, TdfSetData.TravelP2PressDst_3, 6);
	memcpy(sumitomo_0907->TravelP2PressDst_4, TdfSetData.TravelP2PressDst_4, 4);
}

void Make_Sumitomo_0908_s(sumitomo_parm_grp_0908_t *sumitomo_0908) {
	memcpy(sumitomo_0908->ArmCylBottomPressDst_1,
			TdfSetData.ArmCylBottomPressDst_1, 6);
	memcpy(sumitomo_0908->ArmCylBottomPressDst_2,
			TdfSetData.ArmCylBottomPressDst_2, 6);
	memcpy(sumitomo_0908->ArmCylBottomPressDst_3,
			TdfSetData.ArmCylBottomPressDst_3, 6);
	memcpy(sumitomo_0908->ArmCylBottomPressDst_4,
			TdfSetData.ArmCylBottomPressDst_4, 3);
}

void Make_Sumitomo_0909_s(sumitomo_parm_grp_0909_t *sumitomo_0909) {
	memcpy(sumitomo_0909->ArmCylRodPressDst_1, TdfSetData.ArmCylRodPressDst_1,
			6);
	memcpy(sumitomo_0909->ArmCylRodPressDst_2, TdfSetData.ArmCylRodPressDst_2,
			6);
	memcpy(sumitomo_0909->ArmCylRodPressDst_3, TdfSetData.ArmCylRodPressDst_3,
			6);
	memcpy(sumitomo_0909->ArmCylRodPressDst_4, TdfSetData.ArmCylRodPressDst_4,
			3);
}

void Make_Sumitomo_090A_s(sumitomo_parm_grp_090A_t *sumitomo_090A) {
	memcpy(sumitomo_090A->BoomCylBottomPressDst_1,
			TdfSetData.BoomCylBottomPressDst_1, 6);
	memcpy(sumitomo_090A->BoomCylBottomPressDst_2,
			TdfSetData.BoomCylBottomPressDst_2, 6);
	memcpy(sumitomo_090A->BoomCylBottomPressDst_3,
			TdfSetData.BoomCylBottomPressDst_3, 6);
	memcpy(sumitomo_090A->BoomCylBottomPressDst_4,
			TdfSetData.BoomCylBottomPressDst_4, 3);

}

void Make_Sumitomo_090B_s(sumitomo_parm_grp_090B_t *sumitomo_090B) {
	memcpy(sumitomo_090B->BoomCylRodPressDst_1, TdfSetData.BoomCylRodPressDst_1,
			6);
	memcpy(sumitomo_090B->BoomCylRodPressDst_2, TdfSetData.BoomCylRodPressDst_2,
			6);
	memcpy(sumitomo_090B->BoomCylRodPressDst_3, TdfSetData.BoomCylRodPressDst_3,
			6);
	memcpy(sumitomo_090B->BoomCylRodPressDst_4, TdfSetData.BoomCylRodPressDst_4,
			3);

}

void Make_Sumitomo_090C_s(sumitomo_parm_grp_090C_t *sumitomo_090C) {
	memcpy(sumitomo_090C->HydOilTempDst_1, TdfSetData.HydOilTempDst_1, 6);
	memcpy(sumitomo_090C->HydOilTempDst_2, TdfSetData.HydOilTempDst_2, 6);
	memcpy(sumitomo_090C->HydOilTempDst_3, TdfSetData.HydOilTempDst_3, 6);
	memcpy(sumitomo_090C->HydOilTempDst_4, TdfSetData.HydOilTempDst_4, 3);
}

void Make_Sumitomo_090D_s(sumitomo_parm_grp_090D_t *sumitomo_090D) {
	memcpy(sumitomo_090D->HydOilFilterPressDst_1,
			TdfSetData.HydOilFilterPressDst_1, 6);
	memcpy(sumitomo_090D->HydOilFilterPressDst_2,
			TdfSetData.HydOilFilterPressDst_2, 6);
	memcpy(sumitomo_090D->HydOilFilterPressDst_3,
			TdfSetData.HydOilFilterPressDst_3, 6);
	memcpy(sumitomo_090D->HydOilFilterPressDst_4,
			TdfSetData.HydOilFilterPressDst_4, 3);

}

void Make_Sumitomo_090E_s(sumitomo_parm_grp_090E_t *sumitomo_090E) {
	memcpy(sumitomo_090E->TravelOprTimeDst_1, TdfSetData.TravelOprTimeDst_1, 6);
	memcpy(sumitomo_090E->TravelOprTimeDst_2, TdfSetData.TravelOprTimeDst_2, 6);
	memcpy(sumitomo_090E->TravelOprTimeDst_3, TdfSetData.TravelOprTimeDst_3, 2);
}

void Make_Sumitomo_090F_s(sumitomo_parm_grp_090F_t *sumitomo_090F) {
	memcpy(sumitomo_090F->TravelOprTimeMax_1, TdfSetData.TravelOprTimeMax_1, 6);
	memcpy(sumitomo_090F->TravelOprTimeMax_2, TdfSetData.TravelOprTimeMax_2, 4);
}

void Make_Sumitomo_0910_s(sumitomo_parm_grp_0910_t *sumitomo_0910) {
	memcpy(sumitomo_0910->EngineActualSpeedDst_1,
			TdfSetData.EngineActualSpeedDst_1, 6);
	memcpy(sumitomo_0910->EngineActualSpeedDst_2,
			TdfSetData.EngineActualSpeedDst_2, 6);
	memcpy(sumitomo_0910->EngineActualSpeedDst_3,
			TdfSetData.EngineActualSpeedDst_3, 6);
	memcpy(sumitomo_0910->EngineActualSpeedDst_4,
			TdfSetData.EngineActualSpeedDst_4, 3);
}

void Make_Sumitomo_0911_s(sumitomo_parm_grp_0911_t *sumitomo_0911) {

	memcpy(sumitomo_0911->CoolantTempDst_1, TdfSetData.CoolantTempDst_1, 6);
	memcpy(sumitomo_0911->CoolantTempDst_2, TdfSetData.CoolantTempDst_2, 6);
	memcpy(sumitomo_0911->CoolantTempDst_3, TdfSetData.CoolantTempDst_3, 6);
	memcpy(sumitomo_0911->CoolantTempDst_4, TdfSetData.CoolantTempDst_4, 3);

}

void Make_Sumitomo_0912_s(sumitomo_parm_grp_0912_t *sumitomo_0912) {

	memcpy(sumitomo_0912->CoolDownTimeDst_1, TdfSetData.CoolDownTimeDst_1, 6);
	memcpy(sumitomo_0912->CoolDownTimeDst_2, TdfSetData.CoolDownTimeDst_2, 6);
}

void Make_Sumitomo_0913_s(sumitomo_parm_grp_0913_t *sumitomo_0913) {

	memcpy(sumitomo_0913->FuelTempDst_1, TdfSetData.FuelTempDst_1, 6);
	memcpy(sumitomo_0913->FuelTempDst_2, TdfSetData.FuelTempDst_2, 6);
	memcpy(sumitomo_0913->FuelTempDst_3, TdfSetData.FuelTempDst_3, 6);
	memcpy(sumitomo_0913->FuelTempDst_4, TdfSetData.FuelTempDst_4, 3);

}

void Make_Sumitomo_0914_s(sumitomo_parm_grp_0914_t *sumitomo_0914) {

	memcpy(sumitomo_0914->InletAirTempDst_1, TdfSetData.InletAirTempDst_1, 6);
	memcpy(sumitomo_0914->InletAirTempDst_2, TdfSetData.InletAirTempDst_2, 6);
	memcpy(sumitomo_0914->InletAirTempDst_3, TdfSetData.InletAirTempDst_3, 6);
	memcpy(sumitomo_0914->InletAirTempDst_4, TdfSetData.InletAirTempDst_4, 3);
}

void Make_Sumitomo_0915_s(sumitomo_parm_grp_0915_t *sumitomo_0915) {
	memcpy(sumitomo_0915->BoostTempDst_1, TdfSetData.BoostTempDst_1, 6);
	memcpy(sumitomo_0915->BoostTempDst_2, TdfSetData.BoostTempDst_2, 6);
	memcpy(sumitomo_0915->BoostTempDst_3, TdfSetData.BoostTempDst_3, 6);
	memcpy(sumitomo_0915->BoostTempDst_4, TdfSetData.BoostTempDst_4, 3);
}

void Make_Sumitomo_0916_s(sumitomo_parm_grp_0916_t *sumitomo_0916) {
	memcpy(sumitomo_0916->BaroPressDst_1, TdfSetData.BaroPressDst_1, 6);
	memcpy(sumitomo_0916->BaroPressDst_2, TdfSetData.BaroPressDst_2, 6);
	memcpy(sumitomo_0916->BaroPressDst_3, TdfSetData.BaroPressDst_3, 6);
	memcpy(sumitomo_0916->BaroPressDst_4, TdfSetData.BaroPressDst_4, 3);
}

void Make_Sumitomo_0917_s(sumitomo_parm_grp_0917_t *sumitomo_0917) {
	memcpy(sumitomo_0917->EngineOilPressDst_1, TdfSetData.EngineOilPressDst_1,
			6);
	memcpy(sumitomo_0917->EngineOilPressDst_2, TdfSetData.EngineOilPressDst_2,
			6);
	memcpy(sumitomo_0917->EngineOilPressDst_3, TdfSetData.EngineOilPressDst_3,
			6);
	memcpy(sumitomo_0917->EngineOilPressDst_4, TdfSetData.EngineOilPressDst_4,
			3);

}

void Make_Sumitomo_0918_s(sumitomo_parm_grp_0918_t *sumitomo_0918) {
	memcpy(sumitomo_0918->EngineOilPressRiseTimeDst_1,
			TdfSetData.EngineOilPressRiseTimeDst_1, 6);
	memcpy(sumitomo_0918->EngineOilPressRiseTimeDst_2,
			TdfSetData.EngineOilPressRiseTimeDst_2, 6);
	memcpy(sumitomo_0918->EngineOilPressRiseTimeDst_3,
			TdfSetData.EngineOilPressRiseTimeDst_3, 2);
}

void Make_Sumitomo_0919_s(sumitomo_parm_grp_0919_t *sumitomo_0919) {
	memcpy(sumitomo_0919->BoostPressDst_1, TdfSetData.BoostPressDst_1, 6);
	memcpy(sumitomo_0919->BoostPressDst_2, TdfSetData.BoostPressDst_2, 6);
	memcpy(sumitomo_0919->BoostPressDst_3, TdfSetData.BoostPressDst_3, 6);
	memcpy(sumitomo_0919->BoostPressDst_4, TdfSetData.BoostPressDst_4, 3);
}

void Make_Sumitomo_091A_s(sumitomo_parm_grp_091A_t *sumitomo_091A) {
	memcpy(sumitomo_091A->EngineLoadRatioDst_1,
			TdfSetData.EngineLoadRatioADst_1, 6);
	memcpy(sumitomo_091A->EngineLoadRatioDst_2,
			TdfSetData.EngineLoadRatioADst_2, 6);
	memcpy(sumitomo_091A->EngineLoadRatioDst_3,
			TdfSetData.EngineLoadRatioADst_3, 6);
	memcpy(sumitomo_091A->EngineLoadRatioDst_4,
			TdfSetData.EngineLoadRatioADst_4, 3);
}

void Make_Sumitomo_091B_s(sumitomo_parm_grp_091B_t *sumitomo_091B) {
	memcpy(sumitomo_091B->EngineLoadRatioSPDst_1,
			TdfSetData.EngineLoadRatioSPDst_1, 6);
	memcpy(sumitomo_091B->EngineLoadRatioSPDst_2,
			TdfSetData.EngineLoadRatioSPDst_2, 6);
	memcpy(sumitomo_091B->EngineLoadRatioSPDst_3,
			TdfSetData.EngineLoadRatioSPDst_3, 6);
	memcpy(sumitomo_091B->EngineLoadRatioSPDst_4,
			TdfSetData.EngineLoadRatioSPDst_4, 3);
}

void Make_Sumitomo_091C_s(sumitomo_parm_grp_091C_t *sumitomo_091C) {
	memcpy(sumitomo_091C->EngineLoadRatioHDst_1,
			TdfSetData.EngineLoadRatioHDst_1, 6);
	memcpy(sumitomo_091C->EngineLoadRatioHDst_2,
			TdfSetData.EngineLoadRatioHDst_2, 6);
	memcpy(sumitomo_091C->EngineLoadRatioHDst_3,
			TdfSetData.EngineLoadRatioHDst_3, 6);
	memcpy(sumitomo_091C->EngineLoadRatioHDst_4,
			TdfSetData.EngineLoadRatioHDst_4, 3);
}

void Make_Sumitomo_091D_s(sumitomo_parm_grp_091D_t *sumitomo_091D) {
	memcpy(sumitomo_091D->EngineLoadRatioADst_1,
			TdfSetData.EngineLoadRatioADst_1, 6);
	memcpy(sumitomo_091D->EngineLoadRatioADst_2,
			TdfSetData.EngineLoadRatioADst_2, 6);
	memcpy(sumitomo_091D->EngineLoadRatioADst_3,
			TdfSetData.EngineLoadRatioADst_3, 6);
	memcpy(sumitomo_091D->EngineLoadRatioADst_4,
			TdfSetData.EngineLoadRatioADst_4, 3);
}

void Make_Sumitomo_091E_s(sumitomo_parm_grp_091E_t *sumitomo_091E) {
	memcpy(sumitomo_091E->SupplyPumpPressDst_1, TdfSetData.SupplyPumpPressDst_1,
			6);
	memcpy(sumitomo_091E->SupplyPumpPressDst_2, TdfSetData.SupplyPumpPressDst_2,
			6);
	memcpy(sumitomo_091E->SupplyPumpPressDst_3, TdfSetData.SupplyPumpPressDst_3,
			6);
	memcpy(sumitomo_091E->SupplyPumpPressDst_4, TdfSetData.SupplyPumpPressDst_4,
			3);
}

void Make_Sumitomo_091F_s(sumitomo_parm_grp_091F_t *sumitomo_091F) {
	memcpy(sumitomo_091F->DOCInTempDst_1, TdfSetData.DOCInTempDst_1, 6);
	memcpy(sumitomo_091F->DOCInTempDst_2, TdfSetData.DOCInTempDst_2, 6);
	memcpy(sumitomo_091F->DOCInTempDst_3, TdfSetData.DOCInTempDst_3, 6);
	memcpy(sumitomo_091F->DOCInTempDst_4, TdfSetData.DOCInTempDst_4, 3);
}

void Make_Sumitomo_0920_s(sumitomo_parm_grp_0920_t *sumitomo_0920) {
	memcpy(sumitomo_0920->DOCOutTempDst_1, TdfSetData.DOCOutTempDst_1, 6);
	memcpy(sumitomo_0920->DOCOutTempDst_2, TdfSetData.DOCOutTempDst_2, 6);
	memcpy(sumitomo_0920->DOCOutTempDst_3, TdfSetData.DOCOutTempDst_3, 6);
	memcpy(sumitomo_0920->DOCOutTempDst_4, TdfSetData.DOCOutTempDst_4, 3);
}

void Make_Sumitomo_0921_s(sumitomo_parm_grp_0921_t *sumitomo_0921) {
	memcpy(sumitomo_0921->EGR_1InTempDst_1, TdfSetData.EGR_1InTempDst_1, 6);
	memcpy(sumitomo_0921->EGR_1InTempDst_2, TdfSetData.EGR_1InTempDst_2, 6);
	memcpy(sumitomo_0921->EGR_1InTempDst_3, TdfSetData.EGR_1InTempDst_3, 6);
	memcpy(sumitomo_0921->EGR_1InTempDst_4, TdfSetData.EGR_1InTempDst_4, 3);
}

void Make_Sumitomo_0922_s(sumitomo_parm_grp_0922_t *sumitomo_0922) {
	memcpy(sumitomo_0922->EGR_1OutTempDst_1, TdfSetData.EGR_1OutTempDst_1, 6);
	memcpy(sumitomo_0922->EGR_1OutTempDst_2, TdfSetData.EGR_1OutTempDst_2, 6);
	memcpy(sumitomo_0922->EGR_1OutTempDst_3, TdfSetData.EGR_1OutTempDst_3, 6);
	memcpy(sumitomo_0922->EGR_1OutTempDst_4, TdfSetData.EGR_1OutTempDst_4, 3);
}

void Make_Sumitomo_0923_s(sumitomo_parm_grp_0923_t *sumitomo_0923) {
	memcpy(sumitomo_0923->EGR_2InTempDst_1, TdfSetData.EGR_2InTempDst_1, 6);
	memcpy(sumitomo_0923->EGR_2InTempDst_2, TdfSetData.EGR_2InTempDst_2, 6);
	memcpy(sumitomo_0923->EGR_2InTempDst_3, TdfSetData.EGR_2InTempDst_4, 6);
	memcpy(sumitomo_0923->EGR_2InTempDst_4, TdfSetData.EGR_2InTempDst_4, 3);
}

void Make_Sumitomo_0924_s(sumitomo_parm_grp_0924_t *sumitomo_0924) {
	memcpy(sumitomo_0924->EGR_2OutTempDst_1, TdfSetData.EGR_2OutTempDst_1, 6);
	memcpy(sumitomo_0924->EGR_2OutTempDst_2, TdfSetData.EGR_2OutTempDst_2, 6);
	memcpy(sumitomo_0924->EGR_2OutTempDst_3, TdfSetData.EGR_2OutTempDst_3, 6);
	memcpy(sumitomo_0924->EGR_2OutTempDst_4, TdfSetData.EGR_2OutTempDst_4, 3);
}

void Make_Sumitomo_0925_s(sumitomo_parm_grp_0925_t *sumitomo_0925) {
	memcpy(sumitomo_0925->InterCoolerTempDst_1, TdfSetData.InterCoolerTempDst_1,
			6);
	memcpy(sumitomo_0925->InterCoolerTempDst_2, TdfSetData.InterCoolerTempDst_2,
			6);
	memcpy(sumitomo_0925->InterCoolerTempDst_3, TdfSetData.InterCoolerTempDst_3,
			6);
	memcpy(sumitomo_0925->InterCoolerTempDst_4, TdfSetData.InterCoolerTempDst_4,
			3);

}

void Make_Sumitomo_0926_s(sumitomo_parm_grp_0926_t *sumitomo_0926) {
	memcpy(sumitomo_0926->ManifoldTempDst_1, TdfSetData.ManifoldTempDst_1, 6);
	memcpy(sumitomo_0926->ManifoldTempDst_2, TdfSetData.ManifoldTempDst_2, 6);
	memcpy(sumitomo_0926->ManifoldTempDst_3, TdfSetData.ManifoldTempDst_3, 6);
	memcpy(sumitomo_0926->ManifoldTempDst_4, TdfSetData.ManifoldTempDst_4, 3);

}

void Make_Sumitomo_0927_s(sumitomo_parm_grp_0927_t *sumitomo_0927) {
	memcpy(sumitomo_0927->CommonRailPressDst_1, TdfSetData.CommonRailPressDst_1,
			6);
	memcpy(sumitomo_0927->CommonRailPressDst_2, TdfSetData.CommonRailPressDst_2,
			6);
	memcpy(sumitomo_0927->CommonRailPressDst_3, TdfSetData.CommonRailPressDst_3,
			6);
	memcpy(sumitomo_0927->CommonRailPressDst_4, TdfSetData.CommonRailPressDst_4,
			3);
}

void Make_Sumitomo_0928_s(sumitomo_parm_grp_0928_t *sumitomo_0928) {
	memcpy(sumitomo_0928->CommonRailDiffPressDst_1,
			TdfSetData.CommonRailDiffPressDst_1, 6);
	memcpy(sumitomo_0928->CommonRailDiffPressDst_2,
			TdfSetData.CommonRailDiffPressDst_2, 6);
	memcpy(sumitomo_0928->CommonRailDiffPressDst_3,
			TdfSetData.CommonRailDiffPressDst_3, 6);
	memcpy(sumitomo_0928->CommonRailDiffPressDst_4,
			TdfSetData.CommonRailDiffPressDst_4, 3);
}

void Make_Sumitomo_0929_s(sumitomo_parm_grp_0929_t *sumitomo_0929) {
	memcpy(sumitomo_0929->DPDDiffPressDst_1, TdfSetData.DPDDiffPressDst_1, 6);
	memcpy(sumitomo_0929->DPDDiffPressDst_2, TdfSetData.DPDDiffPressDst_2, 6);
	memcpy(sumitomo_0929->DPDDiffPressDst_3, TdfSetData.DPDDiffPressDst_3, 6);
	memcpy(sumitomo_0929->DPDDiffPressDst_4, TdfSetData.DPDDiffPressDst_4, 3);
}

void Make_Sumitomo_092A_s(sumitomo_parm_grp_092A_t *sumitomo_092A) {
	memcpy(sumitomo_092A->AirconditionerBlowerDst_1,
			TdfSetData.AirconditionerBlowerDst_1, 6);
	memcpy(sumitomo_092A->AirconditionerBlowerDst_2,
			TdfSetData.AirconditionerBlowerDst_2, 6);
	memcpy(sumitomo_092A->AirconditionerBlowerDst_3,
			TdfSetData.AirconditionerBlowerDst_3, 6);
}

void Make_Sumitomo_092B_s(sumitomo_parm_grp_092B_t *sumitomo_092B) {
	memcpy(sumitomo_092B->AirconditionerTargetTempDst_1,
			TdfSetData.AirconditionerTargetTempDst_1, 6);
	memcpy(sumitomo_092B->AirconditionerTargetTempDst_2,
			TdfSetData.AirconditionerTargetTempDst_2, 6);
	memcpy(sumitomo_092B->AirconditionerTargetTempDst_3,
			TdfSetData.AirconditionerTargetTempDst_3, 6);
	memcpy(sumitomo_092B->AirconditionerTargetTempDst_4,
			TdfSetData.AirconditionerTargetTempDst_4, 3);
}

void Make_Sumitomo_0A00_s(sumitomo_parm_grp_0A00_t *sumitomo_0A00) {
	//todo bit_zone_comm_t bzcommon;
	memcpy(sumitomo_0A00->MainteTime_1, TdfSetData.MainteTimeFromWeb, 6);
	memcpy(sumitomo_0A00->MainteTime_2, TdfSetData.MainteTimeFromWeb + 6, 6);
	memcpy(sumitomo_0A00->MainteTime_3, TdfSetData.MainteTimeFromWeb + 12, 6);
	memcpy(sumitomo_0A00->MainteTime_4, TdfSetData.MainteTimeFromWeb + 18, 6);
	memcpy(sumitomo_0A00->MainteTime_5, TdfSetData.MainteTimeFromWeb + 24, 6);
}

void Make_Sumitomo_0B00_s(sumitomo_parm_grp_0B00_t *sumitomo_0B00) {
	//todo bit_zone_comm_t bzcommon;
	memcpy(sumitomo_0B00->Manual_Re_Gen_Notification,
			TdfSetData.MainteTimeFromWeb, 6);

}

/*
 *函数功能:  获取系统启动到现在的时间
 *输入:      void
 *返回值:    系统从开始到现在的时间
 */
int GetSysTime(void) {
	struct timespec sysTime = { 0, 0 };
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
	} else {
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
	} else {
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
 *返回值:    如果时间段切换返回值retValue中最后一个bit位是1，时间偏移量切换返回值reValue中倒数第二个bit位
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
		if ((cand_Init_Set == 0) && ((System.EngineState == 1 && System.AccState == 1) || System.IsReSet == 1)) {
			//GPS初次组装且keyOn状态收到初期设定信息或者GPS 执行全复位，生成初期设定信息，收到<初期设定回复信息>之前，每15分钟重复生成一次该信息
			MsgMake_30();
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
			if (time(NULL) / 60 + 1 >= StayMinute) {
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
		} else {
			EngineRpm = 0;
		}
		// 检测发动机状态
		CheckEngine();
		// 不断更新生成定时信息的数组
		CheckMakeMsg();
		CheckPowerOff();

		usleep(1000);
	}
	printf("############## %s exit ##############\n", __FUNCTION__);
	return 0;
}
