/*********************
 *name:candata.c
 * func:can date recv request  operation
 * auth:guozhiyue
 * date:2018.12.20
 *********************/
#include <stdio.h>
#include <linux/can.h>
#include <unistd.h>
#include <string.h>

#include "api.h"
#include "general.h"
#include "candriver.h"
#include "candata.h"
#include "gps_4g.h"

/****************************ECM数据***************************/
u8 cand_CoolantT = 0xFF; //冷却液温度
u8 cand_FuelT = 0xFF; //燃料温度
u8 cand_BarPress = 0xFF; //大气压
u16 cand_AmbAirT = 0xFFFF; //吸气温度
u8 cand_Eng_DPFI_P = 0xFF; //Engine Diesel Particulate Filter Inlet Pressurey引擎柴油微粒过滤器入口压力
u8 cand_Eng_InM1_T = 0xFF; //Engine Intake Manifold 1 Temperature发动机进气歧管1温度
u8 cand_Eng_AirIn_P = 0xFF; //Engine Air Inlet Pressure发动机进气压力
u16 cand_ActExM_P = 0xFFFF; //Actual Exhaust Manifold Pressure实际排气歧管压力
u16 cand_ActInM_P = 0xFFFF; //Actual Intake Manifold Pressure实际进气歧管压力
u8 cand_TargetIn_P = 0xFF; //Target Intake Pressure目标进气压力
u8 cand_TargetInthr_rate = 0xFF; //Target Intake throttle rate目标进气门节流率
u8 cand_LOP_SW_Port = 0xFF; //LOP SW Port
u16 cand_CommonRail_P = 0xFFFF; //Engine Injector Metering Rail 1 Pressure共轨压力
u8 cand_EGRValTar_pos = 0xFF; //EGR Valve Target PositionEGR阀目标位置
u8 cand_EGRVaTarPos_MAP = 0xFF; //EGR Valve Target Position(Calculated value by MAP)EGR阀目标位置(MAP计算值)
u8 cand_Ex_gas = 0xFF; //Exhaust gas lambda废气λ
u8 cand_EGRVaAct_pos = 0xFF; //EGR Valve Actual PositionEGR阀实际位置
u16 cand_Tar_NOx = 0xFFFF; //Target NOx (Final value)目标NOx(最终值)
u16 cand_EngineSpeed = 0xFFFF; //发动机转速
u8 cand_EngStartMod = 0xFF; //Engine Starter Mode发动机起动器模式
u16 cand_FuelRate = 0xFFFF; //Fuel rate燃料消耗率
u8 cand_Throttle_positon = 0xFF; //Throuttle positon节流正电子
u16 cand_After1ExGas_T1 = 0xFFFF; //Aftertreatment 1 Exhaust Gas Temperature 1后处理废气温度
u16 cand_After1DPfiltInGas_T = 0xFFFF; //Aftertreatment 1 Diesel Particulate Filter Intake Gas Temperature后处理1柴油机微粒过滤器进气温度
u8 cand_Engload_ratio = 0xFF; //Engine load ratio引擎负载电压
u8 cand_Englimitload_ratio = 0xFF; //Engine limited load ratio发动机限载比
u16 cand_EngDesOpera_speed = 0xFFFF; //Engine's Desired Operating Speed发动机所需的运行速度
u8 cand_AccelerationDet_status = 0xFF; //Acceleration Detection Status1\2加速度检测状态1\2
u8 cand_EngGloag_ratio = 0xFF; //Engine gross load ratio(Smoke limit)发动机总负载比(限烟)
u16 cand_TarEng_speed = 0xFFFF; //Target Engine Speed目标发动机转速
u16 cand_TarInjection_Amount = 0xFFFF; //Target Injection Amount目标注入量
u16 cand_BasicInjection_Timing = 0xFFFF; //Basic Injection Timing基本喷油正时
//u8 cand_AccelerationDet_status;//Acceleration Detection Status 1加速度检测状态1\2
u8 cand_LoadDetection_status = 0xFF; //Load Detection Status 1\2负载检测状态1\2
u32 cand_totalfuel_used = 0xFFFFFFFF;// total fuel used

/*****************************************************/
int CANDA_RecvSta = -1, CANDB_RecvSta = -1, CANDC_RecvSta = -1; //接收状态标志位
int CANDA_SendSta = -1, CANDB_SendSta = -1, CANDC_SendSta = -1; //发送
//int cand_Pgn_Set_Num = 18;
u16 CANA_RX_Save_Index = 0;       //CANA接收队列保存指针
u16 CANA_RX_Read_Index = 0;      //CANA接收队列读取指针
u16 CANB_RX_Save_Index = 0;       //CANB接收队列保存指针
u16 CANB_RX_Read_Index = 0;      //CANB接收队列读取指针
u16 CANC_RX_Save_Index = 0;       //CANC接收队列保存指针
u16 CANC_RX_Read_Index = 0;      //CANC接收队列读取指针
CanMsg CANA_RX_Data[CANA_RX_MAX];    //CANA接收队列大小
CanMsg CANB_RX_Data[CANB_RX_MAX];    //CANB接收队列大小
CanMsg CANC_RX_Data[CANC_RX_MAX];    //CANC接收队列大小

struct can_frame canA_RecvBuf;
struct can_frame canA_SendBuf;
struct can_frame canB_RecvBuf[CAND_RX_BUF_NUM];
struct can_frame canC_RecvBuf[CAND_RX_BUF_NUM];

TDF_ENUM_PRMID TDF_PrmID;
TDF_GET_DATA TdfGetData;
TDF_SET_DATA TdfSetData;

u8 cand_command_reset;//can reset value

int cand_RecvHandle(int canx, struct can_frame* candata, int nframe);
//******************************************************************

/**
 * @brief  终端CAN设置参数默认值 透传数据采集
 * @param  None
 * @retval None
 */
int Sys_CanSet_Default(void) {

//	u32 *tmpPtr = NULL;
	//清除设置缓冲区

	return 0;
}
int Sys_CanSet_BTDefault(void) {
//	u32 *tmpPtr = NULL;

	//清除设置缓冲区

	return 0;

}

/**************************GetData decode***************************/
//getdata
int gd_rxq_Ver(u8 *da) {
	memcpy(da, TdfGetData.Ver, 6);
	return (0);
}

int gd_rxq_CtlrTPartsNum_1(u8 *da) {
	memcpy(da, TdfGetData.CtlrTPartsNum, 6);
	return (0);
}

int gd_rxq_CtlrTPartsNum_2(u8 *da) {
	memcpy(da, TdfGetData.CtlrTPartsNum + 6, 6);
	return (0);
}

int gd_rxq_CtlrT(u8 *da) {
	memcpy(da, TdfGetData.CtlrT, 6);
	return (0);
}

int gd_rxq_GpsPos(u8 *da) {
	memcpy(da, TdfGetData.GpsPos, 6);
	return (0);
}

int gd_rxq_RtcTim(u8 *da) {
	memcpy(da, TdfGetData.RtcTim, 6);
	return (0);
}

int gd_rxq_CtlrTCfg(u8 *da) {
	memcpy(da, TdfGetData.CtlrTCfg, 6);
	return (0);
}

int gd_rxq_SerialNumRequest(u8 *da) {
	memcpy(da, TdfGetData.SerialNumRequest, 6);
	return (0);
}

int gd_rxq_ImmobiPass(u8 *da) {
	memcpy(da, TdfGetData.ImmobiPass, 6);
	return (0);
}

int gd_rxq_MainteTimeFromWeb_1(u8 *da) {
	memcpy(da, TdfGetData.MainteTimeFromWeb, 6);
	return (0);
}
int gd_rxq_MainteTimeFromWeb_2(u8 *da) {
	memcpy(da, TdfGetData.MainteTimeFromWeb + 6, 6);
	return (0);
}
int gd_rxq_MainteTimeFromWeb_3(u8 *da) {
	memcpy(da, TdfGetData.MainteTimeFromWeb + 12, 6);
	return (0);
}
int gd_rxq_MainteTimeFromWeb_4(u8 *da) {
	memcpy(da, TdfGetData.MainteTimeFromWeb + 18, 6);
	return (0);
}
int gd_rxq_MainteTimeFromWeb_5(u8 *da) {
	memcpy(da, TdfGetData.MainteTimeFromWeb + 24, 6);
	return (0);
}

int gd_rxq_MainteIntervalFromWeb_1(u8 *da) {
	memcpy(da, TdfGetData.MainteIntervalFromWeb, 6);
	return (0);
}
int gd_rxq_MainteIntervalFromWeb_2(u8 *da) {
	memcpy(da, TdfGetData.MainteIntervalFromWeb + 6, 6);
	return (0);
}
int gd_rxq_MainteIntervalFromWeb_3(u8 *da) {
	memcpy(da, TdfGetData.MainteIntervalFromWeb + 12, 6);
	return (0);
}
int gd_rxq_MainteIntervalFromWeb_4(u8 *da) {
	memcpy(da, TdfGetData.MainteIntervalFromWeb + 18, 6);
	return (0);
}
int gd_rxq_MainteIntervalFromWeb_5(u8 *da) {
	memcpy(da, TdfGetData.MainteIntervalFromWeb + 24, 6);
	return (0);
}

int gd_rxq_ParameterConfig(u8 *da) {
	memcpy(da, TdfGetData.ParameterConfig, 6);
	return (0);
}

int gd_rxq_EngineDeviceTestRequest(u8 *da) {
	memcpy(da, TdfGetData.EngineDeviceTestRequest, 6);
	return (0);
}

int gd_rxq_ImmobiPassAnswer(u8 *da) {
	memcpy(da, TdfGetData.ImmobiPassAnswer, 6);
	return (0);
}
//setdata
int gd_rxq_McnInfo_1(u8 *da) {
	memcpy(&TdfSetData.McnInfo, da, 6);
	return 0;
}

int gd_rxq_McnInfo_2(u8 *da) {
	memcpy(&TdfSetData.McnInfo + 6, da, 6);
	return 0;
}

int gd_rxq_MainCtlrPartsNum_1(u8 *da) {
	memcpy(TdfSetData.MainCtltPartsNum, da, 6);
	return 0;
}
int gd_rxq_MainCtlrPartsNum_2(u8 *da) {
	memcpy(TdfSetData.MainCtltPartsNum + 6, da, 6);
	return 0;
}

int gd_rxq_MainCtlrSerialNum_1(u8 *da) {
	memcpy(TdfSetData.MainCtltSerialNum, da, 6);
	return 0;
}

int gd_rxq_MainCtlrSerialNum_2(u8 *da) {
	memcpy(TdfSetData.MainCtltSerialNum + 6, da, 5);
	return 0;
}

int gd_rxq_SubCtlrFirmwareVer(u8 *da) {
	memcpy(TdfSetData.SubCtlrFirmwareVer, da, 1);
	return 0;
}

int gd_rxq_SubCtlrSerialNum_1(u8 *da) {
	memcpy(TdfSetData.SubCtlrSerialNum, da, 6);
	return 0;
}

int gd_rxq_SubCtlrSerialNum_2(u8 *da) {
	memcpy(TdfSetData.SubCtlrSerialNum + 6, da, 2);
	return 0;
}

int gd_rxq_MonitorNum(u8 *da) {
	memcpy(TdfSetData.MonitorNum, da, 4);
	return 0;
}

int gd_rxq_MonitorSerialNum(u8 *da) {
	memcpy(TdfSetData.MonitorSerialNum, da, 6);
	return 0;
}

int gd_rxq_MainCtlrSts(u8 *da) {
	memcpy(&TdfSetData.MainCtlrSts, da, 6);
	return 0;
}

int gd_rxq_MainteTime_1(u8 *da) {
	memcpy(TdfSetData.MainteTimeFromWeb, da, 6);
	return 0;
}

int gd_rxq_MainteTime_2(u8 *da) {
	memcpy(TdfSetData.MainteTimeFromWeb + 6, da, 6);
	return 0;
}

int gd_rxq_MainteTime_3(u8 *da) {
	memcpy(TdfSetData.MainteTimeFromWeb + 12, da, 6);
	return 0;
}
int gd_rxq_MainteTime_4(u8 *da) {
	memcpy(TdfSetData.MainteTimeFromWeb + 18, da, 6);
	return 0;
}
int gd_rxq_MainteTime_5(u8 *da) {
	memcpy(TdfSetData.MainteTimeFromWeb + 24, da, 6);
	return 0;
}

int gd_rxq_Work_1(u8 *da) {
	memcpy(TdfSetData.Work, da, 6);
	return 0;
}

int gd_rxq_Work_2(u8 *da) {
	memcpy(TdfSetData.Work + 6, da, 3);
	return 0;
}

int gd_rxq_UpperWork_1(u8 *da) {
	memcpy(TdfSetData.UpperWork, da, 6);
	return 0;
}

int gd_rxq_UpperWork_2(u8 *da) {
	memcpy(TdfSetData.UpperWork + 6, da, 6);
	return 0;
}

int gd_rxq_UpperWork_3(u8 *da) {
	memcpy(TdfSetData.UpperWork + 12, da, 6);
	return 0;
}

int gd_rxq_UpperWork_4(u8 *da) {
	memcpy(TdfSetData.UpperWork + 18, da, 6);
	return 0;
}

int gd_rxq_UpperWork_5(u8 *da) {
	memcpy(TdfSetData.UpperWork + 24, da, 6);
	return 0;
}

int gd_rxq_UpperWork_6(u8 *da) {
	memcpy(TdfSetData.UpperWork + 30, da, 3);
	return 0;
}

int gd_rxq_TravelWork_1(u8 *da) {
	memcpy(TdfSetData.TravelWork, da, 6);
	return 0;
}

int gd_rxq_TravelWork_2(u8 *da) {
	memcpy(TdfSetData.TravelWork + 6, da, 6);
	return 0;
}

int gd_rxq_TravelWork_3(u8 *da) {
	memcpy(TdfSetData.TravelWork + 12, da, 6);
	return 0;
}

int gd_rxq_WorkMode_1(u8 *da) {
	memcpy(TdfSetData.WorkMode, da, 6);
	return 0;
}

int gd_rxq_WorkMode_2(u8 *da) {
	memcpy(TdfSetData.WorkMode + 6, da, 6);
	return 0;
}

int gd_rxq_WorkMode_3(u8 *da) {
	memcpy(TdfSetData.WorkMode + 12, da, 6);
	return 0;
}

int gd_rxq_WorkMode_4(u8 *da) {
	memcpy(TdfSetData.WorkMode + 18, da, 6);
	return 0;
}

int gd_rxq_WorkMode_5(u8 *da) {
	memcpy(TdfSetData.WorkMode + 24, da, 6);
	return 0;
}

int gd_rxq_Breaker(u8 *da) {
	memcpy(TdfSetData.Breaker, da, 6);
	return 0;
}

int gd_rxq_Crusher(u8 *da) {
	memcpy(TdfSetData.Crusher, da, 6);
	return 0;
}

int gd_rxq_OptionLine_1(u8 *da) {
	memcpy(TdfSetData.OptionLine, da, 6);
	return 0;
}

int gd_rxq_OptionLine_2(u8 *da) {
	memcpy(TdfSetData.OptionLine + 6, da, 6);
	return 0;
}

int gd_rxq_QuickCoupler(u8 *da) {
	memcpy(TdfSetData.QuickCoupler, da, 6);
	return 0;
}

int gd_rxq_PumpBackup_1(u8 *da) {
	memcpy(TdfSetData.PumpBackup, da, 6);
	return 0;
}

int gd_rxq_PumpBackup_2(u8 *da) {
	memcpy(TdfSetData.PumpBackup + 6, da, 6);
	return 0;
}

int gd_rxq_OverheatPowerReduction_1(u8 *da) {
	memcpy(TdfSetData.OverheatPowerReduction, da, 6);
	return 0;
}

int gd_rxq_OverheatPowerReduction_2(u8 *da) {
	memcpy(TdfSetData.OverheatPowerReduction + 6, da, 6);
	return 0;
}

int gd_rxq_OverheatPowerReduction_3(u8 *da) {
	memcpy(TdfSetData.OverheatPowerReduction + 12, da, 3);
	return 0;
}
int gd_rxq_Solenoid_1(u8 *da) {
	memcpy(TdfSetData.Solenoid, da, 6);
	return 0;
}
int gd_rxq_Solenoid_2(u8 *da) {
	memcpy(TdfSetData.Solenoid + 6, da, 6);
	return 0;
}
int gd_rxq_Solenoid_3(u8 *da) {
	memcpy(TdfSetData.Solenoid + 12, da, 6);
	return 0;
}
int gd_rxq_Solenoid_4(u8 *da) {
	memcpy(TdfSetData.Solenoid + 18, da, 6);
	return 0;
}
int gd_rxq_Solenoid_5(u8 *da) {
	memcpy(TdfSetData.Solenoid + 24, da, 6);
	return 0;
}
int gd_rxq_Solenoid_6(u8 *da) {
	memcpy(TdfSetData.Solenoid + 30, da, 6);
	return 0;
}
int gd_rxq_Solenoid_7(u8 *da) {
	memcpy(TdfSetData.Solenoid + 36, da, 6);
	return 0;
}
int gd_rxq_Solenoid_8(u8 *da) {
	memcpy(TdfSetData.Solenoid + 42, da, 6);
	return 0;
}
int gd_rxq_Solenoid_9(u8 *da) {
	memcpy(TdfSetData.Solenoid + 48, da, 6);
	return 0;
}
int gd_rxq_Solenoid_10(u8 *da) {
	memcpy(TdfSetData.Solenoid + 54, da, 6);
	return 0;
}

int gd_rxq_ATSReGen_1(u8 *da) {
	memcpy(TdfSetData.ATSReGen, da, 6);
	return 0;
}
int gd_rxq_ATSReGen_2(u8 *da) {
	memcpy(TdfSetData.ATSReGen + 6, da, 6);
	return 0;
}
int gd_rxq_ATSReGen_3(u8 *da) {
	memcpy(TdfSetData.ATSReGen + 12, da, 6);
	return 0;
}
int gd_rxq_ATSReGen_4(u8 *da) {
	memcpy(TdfSetData.ATSReGen + 18, da, 6);
	return 0;
}
int gd_rxq_ATSReGen_5(u8 *da) {
	memcpy(TdfSetData.ATSReGen + 24, da, 6);
	return 0;
}
int gd_rxq_ATSReGen_6(u8 *da) {
	memcpy(TdfSetData.ATSReGen + 30, da, 6);
	return 0;
}
int gd_rxq_ATSReGen_7(u8 *da) {
	memcpy(TdfSetData.ATSReGen + 36, da, 6);
	return 0;
}
int gd_rxq_ATSReGen_8(u8 *da) {
	memcpy(TdfSetData.ATSReGen + 42, da, 6);
	return 0;
}
int gd_rxq_ATSReGen_9(u8 *da) {
	memcpy(TdfSetData.ATSReGen + 48, da, 6);
	return 0;
}
int gd_rxq_ATSReGen_10(u8 *da) {
	memcpy(TdfSetData.ATSReGen + 54, da, 6);
	return 0;
}
int gd_rxq_ATSReGen_11(u8 *da) {
	memcpy(TdfSetData.ATSReGen + 60, da, 6);
	return 0;
}

int gd_rxq_HotShutdown(u8 *da) {
	memcpy(TdfSetData.HotShutdown, da, 6);
	return 0;
}

int gd_rxq_AirConditioner_1(u8 *da) {
	memcpy(TdfSetData.AirConditioner, da, 6);
	return 0;
}
int gd_rxq_AirConditioner_2(u8 *da) {
	memcpy(TdfSetData.AirConditioner + 6, da, 6);
	return 0;
}
int gd_rxq_AirConditioner_3(u8 *da) {
	memcpy(TdfSetData.AirConditioner + 12, da, 6);
	return 0;
}
int gd_rxq_AirConditioner_4(u8 *da) {
	memcpy(TdfSetData.AirConditioner + 18, da, 6);
	return 0;
}
int gd_rxq_AirConditioner_5(u8 *da) {
	memcpy(TdfSetData.AirConditioner + 24, da, 6);
	return 0;
}
int gd_rxq_AirConditioner_6(u8 *da) {
	memcpy(TdfSetData.AirConditioner + 30, da, 6);
	return 0;
}
int gd_rxq_AirConditioner_7(u8 *da) {
	memcpy(TdfSetData.AirConditioner + 36, da, 6);
	return 0;
}
int gd_rxq_AirConditioner_8(u8 *da) {
	memcpy(TdfSetData.AirConditioner + 42, da, 6);
	return 0;
}

int gd_rxq_KeyOn(u8 *da) {
	memcpy(TdfSetData.KeyOn, da, 6);
	return 0;
}

int gd_rxq_EngineOn(u8 *da) {
	memcpy(TdfSetData.EngineOn, da, 6);
	return 0;
}

int gd_rxq_AutoIdle(u8 *da) {
	memcpy(TdfSetData.AutoIdle, da, 6);
	return 0;
}

int gd_rxq_IdleStop(u8 *da) {
	memcpy(TdfSetData.IdleStop, da, 6);
	return 0;
}

int gd_rxq_WorkOthers_1(u8 *da) {
	memcpy(TdfSetData.WorkOthers, da, 6);
	return 0;
}

int gd_rxq_WorkOthers_2(u8 *da) {
	memcpy(TdfSetData.WorkOthers + 6, da, 6);
	return 0;
}

int gd_rxq_WorkOthers_3(u8 *da) {
	memcpy(TdfSetData.WorkOthers + 12, da, 6);
	return 0;
}

int gd_rxq_Camera_1(u8 *da) {
	memcpy(TdfSetData.Camera, da, 6);
	return 0;
}

int gd_rxq_Camera_2(u8 *da) {
	memcpy(TdfSetData.Camera + 6, da, 6);
	return 0;
}

int gd_rxq_Wiper_1(u8 *da) {
	memcpy(TdfSetData.Wiper, da, 6);
	return 0;
}

int gd_rxq_Wiper_2(u8 *da) {
	memcpy(TdfSetData.Wiper + 6, da, 6);
	return 0;
}

int gd_rxq_Window(u8 *da) {
	memcpy(TdfSetData.Window, da, 6);
	return 0;
}

int gd_rxq_Door(u8 *da) {
	memcpy(TdfSetData.Door, da, 6);
	return 0;
}

int gd_rxq_MaxMin_1(u8 *da) {
	memcpy(TdfSetData.MaxMin, da, 6);
	return 0;
}

int gd_rxq_MaxMin_2(u8 *da) {
	memcpy(TdfSetData.MaxMin + 6, da, 6);
	return 0;
}

int gd_rxq_MaxMin_3(u8 *da) {
	memcpy(TdfSetData.MaxMin + 12, da, 6);
	return 0;
}

int gd_rxq_MaxMin_4(u8 *da) {
	memcpy(TdfSetData.MaxMin + 18, da, 6);
	return 0;
}

int gd_rxq_MaxMin_5(u8 *da) {
	memcpy(TdfSetData.MaxMin + 24, da, 6);
	return 0;
}

int gd_rxq_MaxMin_6(u8 *da) {
	memcpy(TdfSetData.MaxMin + 30, da, 6);
	return 0;
}

int gd_rxq_MaxMin_7(u8 *da) {
	memcpy(TdfSetData.MaxMin + 36, da, 6);
	return 0;
}

int gd_rxq_MaxMin_8(u8 *da) {
	memcpy(TdfSetData.MaxMin + 42, da, 6);
	return 0;
}

int gd_rxq_MaxMin_9(u8 *da) {
	memcpy(TdfSetData.MaxMin + 48, da, 6);
	return 0;
}

int gd_rxq_MaxMin_10(u8 *da) {
	memcpy(TdfSetData.MaxMin + 54, da, 6);
	return 0;
}

int gd_rxq_MaxMin_11(u8 *da) {
	memcpy(TdfSetData.MaxMin + 60, da, 6);
	return 0;
}

int gd_rxq_MaxMin_12(u8 *da) {
	memcpy(TdfSetData.MaxMin + 66, da, 6);
	return 0;
}

int gd_rxq_MaxMin_13(u8 *da) {
	memcpy(TdfSetData.MaxMin + 72, da, 6);
	return 0;
}

int gd_rxq_MaxMin_14(u8 *da) {
	memcpy(TdfSetData.MaxMin + 78, da, 6);
	return 0;
}

int gd_rxq_P1PressDst_1(u8 *da) {
	memcpy(TdfSetData.P1PressDst_1, da, 6);
	return 0;
}

int gd_rxq_P1PressDst_2(u8 *da) {
	memcpy(TdfSetData.P1PressDst_2, da, 6);
	return 0;
}

int gd_rxq_P1PressDst_3(u8 *da) {
	memcpy(TdfSetData.P1PressDst_3, da, 6);
	return 0;
}

int gd_rxq_P1PressDst_4(u8 *da) {
	memcpy(TdfSetData.P1PressDst_4, da, 6);
	return 0;
}

int gd_rxq_P2PressDst_1(u8 *da) {
	memcpy(TdfSetData.P2PressDst_1, da, 6);
	return 0;
}

int gd_rxq_P2PressDst_2(u8 *da) {
	memcpy(TdfSetData.P2PressDst_2, da, 6);
	return 0;
}

int gd_rxq_P2PressDst_3(u8 *da) {
	memcpy(TdfSetData.P2PressDst_3, da, 6);
	return 0;
}

int gd_rxq_P2PressDst_4(u8 *da) {
	memcpy(TdfSetData.P2PressDst_4, da, 6);
	return 0;
}

int gd_rxq_N1PressDst_1(u8 *da) {
	memcpy(TdfSetData.N1PressDst_1, da, 6);
	return 0;
}

int gd_rxq_N1PressDst_2(u8 *da) {
	memcpy(TdfSetData.N1PressDst_2, da, 6);
	return 0;
}

int gd_rxq_N1PressDst_3(u8 *da) {
	memcpy(TdfSetData.N1PressDst_3, da, 6);
	return 0;
}

int gd_rxq_N1PressDst_4(u8 *da) {
	memcpy(TdfSetData.N1PressDst_4, da, 6);
	return 0;
}

int gd_rxq_N2PressDst_1(u8 *da) {
	memcpy(TdfSetData.N2PressDst_1, da, 6);
	return 0;
}

int gd_rxq_N2PressDst_2(u8 *da) {
	memcpy(TdfSetData.N2PressDst_2, da, 6);
	return 0;
}

int gd_rxq_N2PressDst_3(u8 *da) {
	memcpy(TdfSetData.N2PressDst_3, da, 6);
	return 0;
}

int gd_rxq_N2PressDst_4(u8 *da) {
	memcpy(TdfSetData.N2PressDst_4, da, 3);
	return 0;
}

int gd_rxq_P1P2PressDst_1(u8 *da) {
	memcpy(TdfSetData.P1P2PressDst_1, da, 6);
	return 0;
}

int gd_rxq_P1P2PressDst_2(u8 *da) {
	memcpy(TdfSetData.P1P2PressDst_2, da, 6);
	return 0;
}

int gd_rxq_P1P2PressDst_3(u8 *da) {
	memcpy(TdfSetData.P1P2PressDst_3, da, 6);
	return 0;
}

int gd_rxq_P1P2PressDst_4(u8 *da) {
	memcpy(TdfSetData.P1P2PressDst_4, da, 3);
	return 0;
}

int gd_rxq_TravelP1PressDst_1(u8 *da) {
	memcpy(TdfSetData.TravelP1PressDst_1, da, 6);
	return 0;
}

int gd_rxq_TravelP1PressDst_2(u8 *da) {
	memcpy(TdfSetData.TravelP1PressDst_2, da, 6);
	return 0;
}

int gd_rxq_TravelP1PressDst_3(u8 *da) {
	memcpy(TdfSetData.TravelP1PressDst_3, da, 6);
	return 0;
}

int gd_rxq_TravelP1PressDst_4(u8 *da) {
	memcpy(TdfSetData.TravelP1PressDst_4, da, 3);
	return 0;
}

int gd_rxq_TravelP2PressDst_1(u8 *da) {
	memcpy(TdfSetData.TravelP2PressDst_1, da, 6);
	return 0;
}

int gd_rxq_TravelP2PressDst_2(u8 *da) {
	memcpy(TdfSetData.TravelP2PressDst_2, da, 6);
	return 0;
}

int gd_rxq_TravelP2PressDst_3(u8 *da) {
	memcpy(TdfSetData.TravelP2PressDst_3, da, 6);
	return 0;
}

int gd_rxq_TravelP2PressDst_4(u8 *da) {
	memcpy(TdfSetData.TravelP2PressDst_4, da, 3);
	return 0;
}

int gd_rxq_ArmCylBottomPressDst_1(u8 *da) {
	memcpy(TdfSetData.ArmCylBottomPressDst_1, da, 6);
	return 0;
}

int gd_rxq_ArmCylBottomPressDst_2(u8 *da) {
	memcpy(TdfSetData.ArmCylBottomPressDst_2, da, 6);
	return 0;
}

int gd_rxq_ArmCylBottomPressDst_3(u8 *da) {
	memcpy(TdfSetData.ArmCylBottomPressDst_3, da, 6);
	return 0;
}

int gd_rxq_ArmCylBottomPressDst_4(u8 *da) {
	memcpy(TdfSetData.ArmCylBottomPressDst_4, da, 3);
	return 0;
}

int gd_rxq_BoomCylBottomPressDst_1(u8 *da) {
	memcpy(TdfSetData.BoomCylBottomPressDst_1, da, 6);
	return 0;
}

int gd_rxq_BoomCylBottomPressDst_2(u8 *da) {
	memcpy(TdfSetData.BoomCylBottomPressDst_2, da, 6);
	return 0;
}

int gd_rxq_BoomCylBottomPressDst_3(u8 *da) {
	memcpy(TdfSetData.BoomCylBottomPressDst_3, da, 6);
	return 0;
}

int gd_rxq_BoomCylBottomPressDst_4(u8 *da) {
	memcpy(TdfSetData.BoomCylBottomPressDst_4, da, 3);
	return 0;
}

int gd_rxq_BoomCylRodPressDst_1(u8 *da) {
	memcpy(TdfSetData.BoomCylRodPressDst_1, da, 6);
	return 0;
}

int gd_rxq_BoomCylRodPressDst_2(u8 *da) {
	memcpy(TdfSetData.BoomCylRodPressDst_2, da, 6);
	return 0;
}

int gd_rxq_BoomCylRodPressDst_3(u8 *da) {
	memcpy(TdfSetData.BoomCylRodPressDst_3, da, 6);
	return 0;
}

int gd_rxq_BoomCylRodPressDst_4(u8 *da) {
	memcpy(TdfSetData.BoomCylRodPressDst_4, da, 3);
	return 0;
}

int gd_rxq_HydOilTempDst_1(u8 *da) {
	memcpy(TdfSetData.HydOilTempDst_1, da, 6);
	return 0;
}

int gd_rxq_HydOilTempDst_2(u8 *da) {
	memcpy(TdfSetData.HydOilTempDst_2, da, 6);
	return 0;
}

int gd_rxq_HydOilTempDst_3(u8 *da) {
	memcpy(TdfSetData.HydOilTempDst_3, da, 6);
	return 0;
}

int gd_rxq_HydOilTempDst_4(u8 *da) {
	memcpy(TdfSetData.HydOilTempDst_4, da, 3);
	return 0;
}

int gd_rxq_HydOilFilterPressDst_1(u8 *da) {
	memcpy(TdfSetData.HydOilFilterPressDst_1, da, 6);
	return 0;
}

int gd_rxq_HydOilFilterPressDst_2(u8 *da) {
	memcpy(TdfSetData.HydOilFilterPressDst_2, da, 6);
	return 0;
}

int gd_rxq_HydOilFilterPressDst_3(u8 *da) {
	memcpy(TdfSetData.HydOilFilterPressDst_3, da, 6);
	return 0;
}

int gd_rxq_HydOilFilterPressDst_4(u8 *da) {
	memcpy(TdfSetData.HydOilFilterPressDst_4, da, 3);
	return 0;
}

int gd_rxq_TravelOprTimeDst_1(u8 *da) {
	memcpy(TdfSetData.TravelOprTimeDst_1, da, 6);
	return 0;
}

int gd_rxq_TravelOprTimeDst_2(u8 *da) {
	memcpy(TdfSetData.TravelOprTimeDst_2, da, 6);
	return 0;
}

int gd_rxq_TravelOprTimeDst_3(u8 *da) {
	memcpy(TdfSetData.TravelOprTimeDst_3, da, 2);
	return 0;
}

int gd_rxq_TravelOprTimeMax_1(u8 *da) {
	memcpy(TdfSetData.TravelOprTimeMax_1, da, 6);
	return 0;
}

int gd_rxq_TravelOprTimeMax_2(u8 *da) {
	memcpy(TdfSetData.TravelOprTimeMax_2, da, 4);
	return 0;
}

int gd_rxq_EngineActualSpeedDst_1(u8 *da) {
	memcpy(TdfSetData.EngineActualSpeedDst_1, da, 6);
	return 0;
}

int gd_rxq_EngineActualSpeedDst_2(u8 *da) {
	memcpy(TdfSetData.EngineActualSpeedDst_2, da, 6);
	return 0;
}

int gd_rxq_EngineActualSpeedDst_3(u8 *da) {
	memcpy(TdfSetData.EngineActualSpeedDst_3, da, 6);
	return 0;
}

int gd_rxq_EngineActualSpeedDst_4(u8 *da) {
	memcpy(TdfSetData.EngineActualSpeedDst_4, da, 3);
	return 0;
}

int gd_rxq_CoolantTempDst_1(u8 *da) {
	memcpy(TdfSetData.CoolantTempDst_1, da, 6);
	return 0;
}

int gd_rxq_CoolantTempDst_2(u8 *da) {
	memcpy(TdfSetData.CoolantTempDst_2, da, 6);
	return 0;
}

int gd_rxq_CoolantTempDst_3(u8 *da) {
	memcpy(TdfSetData.CoolantTempDst_3, da, 6);
	return 0;
}

int gd_rxq_CoolantTempDst_4(u8 *da) {
	memcpy(TdfSetData.CoolantTempDst_4, da, 3);
	return 0;
}

int gd_rxq_CoolDownTimeDst_1(u8 *da) {
	memcpy(TdfSetData.CoolDownTimeDst_1, da, 6);
	return 0;
}

int gd_rxq_CoolDownTimeDst_2(u8 *da) {
	memcpy(TdfSetData.CoolDownTimeDst_2, da, 6);
	return 0;
}

int gd_rxq_FuelTempDst_1(u8 *da) {
	memcpy(TdfSetData.FuelTempDst_1, da, 6);
	return 0;
}

int gd_rxq_FuelTempDst_2(u8 *da) {
	memcpy(TdfSetData.FuelTempDst_2, da, 6);
	return 0;
}

int gd_rxq_FuelTempDst_3(u8 *da) {
	memcpy(TdfSetData.FuelTempDst_3, da, 6);
	return 0;
}

int gd_rxq_FuelTempDst_4(u8 *da) {
	memcpy(TdfSetData.FuelTempDst_4, da, 3);
	return 0;
}

int gd_rxq_InletAirTempDst_1(u8 *da) {
	memcpy(TdfSetData.InletAirTempDst_1, da, 6);
	return 0;
}

int gd_rxq_InletAirTempDst_2(u8 *da) {
	memcpy(TdfSetData.InletAirTempDst_2, da, 6);
	return 0;
}

int gd_rxq_InletAirTempDst_3(u8 *da) {
	memcpy(TdfSetData.InletAirTempDst_3, da, 6);
	return 0;
}

int gd_rxq_InletAirTempDst_4(u8 *da) {
	memcpy(TdfSetData.InletAirTempDst_4, da, 3);
	return 0;
}

int gd_rxq_BoostTempDst_1(u8 *da) {
	memcpy(TdfSetData.BoostTempDst_1, da, 6);
	return 0;
}

int gd_rxq_BoostTempDst_2(u8 *da) {
	memcpy(TdfSetData.BoostTempDst_2, da, 6);
	return 0;
}

int gd_rxq_BoostTempDst_3(u8 *da) {
	memcpy(TdfSetData.BoostTempDst_3, da, 6);
	return 0;
}

int gd_rxq_BoostTempDst_4(u8 *da) {
	memcpy(TdfSetData.BoostTempDst_4, da, 3);
	return 0;
}

int gd_rxq_BaroPressDst_1(u8 *da) {
	memcpy(TdfSetData.BaroPressDst_1, da, 6);
	return 0;
}

int gd_rxq_BaroPressDst_2(u8 *da) {
	memcpy(TdfSetData.BaroPressDst_2, da, 6);
	return 0;
}

int gd_rxq_BaroPressDst_3(u8 *da) {
	memcpy(TdfSetData.BaroPressDst_3, da, 6);
	return 0;
}

int gd_rxq_BaroPressDst_4(u8 *da) {
	memcpy(TdfSetData.BaroPressDst_4, da, 3);
	return 0;
}

int gd_rxq_EngineOilPressDst_1(u8 *da) {
	memcpy(TdfSetData.EngineOilPressDst_1, da, 6);
	return 0;
}

int gd_rxq_EngineOilPressDst_2(u8 *da) {
	memcpy(TdfSetData.EngineOilPressDst_2, da, 6);
	return 0;
}

int gd_rxq_EngineOilPressDst_3(u8 *da) {
	memcpy(TdfSetData.EngineOilPressDst_3, da, 6);
	return 0;
}

int gd_rxq_EngineOilPressDst_4(u8 *da) {
	memcpy(TdfSetData.EngineOilPressDst_4, da, 3);
	return 0;
}

int gd_rxq_EngineOilPressRiseTimeDst_1(u8 *da) {
	memcpy(TdfSetData.EngineOilPressRiseTimeDst_1, da, 6);
	return 0;
}

int gd_rxq_EngineOilPressRiseTimeDst_2(u8 *da) {
	memcpy(TdfSetData.EngineOilPressRiseTimeDst_2, da, 6);
	return 0;
}

int gd_rxq_EngineOilPressRiseTimeDst_3(u8 *da) {
	memcpy(TdfSetData.EngineOilPressRiseTimeDst_3, da, 2);
	return 0;
}

int gd_rxq_BoostPressDst_1(u8 *da) {
	memcpy(TdfSetData.BoostPressDst_1, da, 6);
	return 0;
}

int gd_rxq_BoostPressDst_2(u8 *da) {
	memcpy(TdfSetData.BoostPressDst_2, da, 6);
	return 0;
}

int gd_rxq_BoostPressDst_3(u8 *da) {
	memcpy(TdfSetData.BoostPressDst_3, da, 6);
	return 0;
}

int gd_rxq_BoostPressDst_4(u8 *da) {
	memcpy(TdfSetData.BoostPressDst_4, da, 3);
	return 0;
}

int gd_rxq_EngineLoadRatioDst_1(u8 *da) {
	memcpy(TdfSetData.EngineLoadRatioDst_1, da, 6);
	return 0;
}

int gd_rxq_EngineLoadRatioDst_2(u8 *da) {
	memcpy(TdfSetData.EngineLoadRatioDst_2, da, 6);
	return 0;
}

int gd_rxq_EngineLoadRatioDst_3(u8 *da) {
	memcpy(TdfSetData.EngineLoadRatioDst_3, da, 6);
	return 0;
}

int gd_rxq_EngineLoadRatioDst_4(u8 *da) {
	memcpy(TdfSetData.EngineLoadRatioDst_4, da, 3);
	return 0;
}

int gd_rxq_EngineLoadRatioSPDst_1(u8 *da) {
	memcpy(TdfSetData.EngineLoadRatioSPDst_1, da, 6);
	return 0;
}

int gd_rxq_EngineLoadRatioSPDst_2(u8 *da) {
	memcpy(TdfSetData.EngineLoadRatioSPDst_2, da, 6);
	return 0;
}

int gd_rxq_EngineLoadRatioSPDst_3(u8 *da) {
	memcpy(TdfSetData.EngineLoadRatioSPDst_3, da, 6);
	return 0;
}

int gd_rxq_EngineLoadRatioSPDst_4(u8 *da) {
	memcpy(TdfSetData.EngineLoadRatioSPDst_4, da, 3);
	return 0;
}

int gd_rxq_EngineLoadRatioHDst_1(u8 *da) {
	memcpy(TdfSetData.EngineLoadRatioHDst_1, da, 6);
	return 0;
}

int gd_rxq_EngineLoadRatioHDst_2(u8 *da) {
	memcpy(TdfSetData.EngineLoadRatioHDst_2, da, 6);
	return 0;
}

int gd_rxq_EngineLoadRatioHDst_3(u8 *da) {
	memcpy(TdfSetData.EngineLoadRatioHDst_3, da, 6);
	return 0;
}

int gd_rxq_EngineLoadRatioHDst_4(u8 *da) {
	memcpy(TdfSetData.EngineLoadRatioHDst_4, da, 3);
	return 0;
}

int gd_rxq_EngineLoadRatioADst_1(u8 *da) {
	memcpy(TdfSetData.EngineLoadRatioADst_1, da, 6);
	return 0;
}

int gd_rxq_EngineLoadRatioADst_2(u8 *da) {
	memcpy(TdfSetData.EngineLoadRatioADst_2, da, 6);
	return 0;
}

int gd_rxq_EngineLoadRatioADst_3(u8 *da) {
	memcpy(TdfSetData.EngineLoadRatioADst_3, da, 6);
	return 0;
}

int gd_rxq_EngineLoadRatioADst_4(u8 *da) {
	memcpy(TdfSetData.EngineLoadRatioADst_4, da, 3);
	return 0;
}

int gd_rxq_SupplyPumpPressDst_1(u8 *da) {
	memcpy(TdfSetData.SupplyPumpPressDst_1, da, 6);
	return 0;
}

int gd_rxq_SupplyPumpPressDst_2(u8 *da) {
	memcpy(TdfSetData.SupplyPumpPressDst_2, da, 6);
	return 0;
}

int gd_rxq_SupplyPumpPressDst_3(u8 *da) {
	memcpy(TdfSetData.SupplyPumpPressDst_3, da, 6);
	return 0;
}

int gd_rxq_SupplyPumpPressDst_4(u8 *da) {
	memcpy(TdfSetData.SupplyPumpPressDst_4, da, 3);
	return 0;
}

int gd_rxq_DOCInTempDst_1(u8 *da) {
	memcpy(TdfSetData.DOCInTempDst_1, da, 6);
	return 0;
}

int gd_rxq_DOCInTempDst_2(u8 *da) {
	memcpy(TdfSetData.DOCInTempDst_2, da, 6);
	return 0;
}

int gd_rxq_DOCInTempDst_3(u8 *da) {
	memcpy(TdfSetData.DOCInTempDst_3, da, 6);
	return 0;
}

int gd_rxq_DOCInTempDst_4(u8 *da) {
	memcpy(TdfSetData.DOCInTempDst_4, da, 3);
	return 0;
}

int gd_rxq_DOCOutTempDst_1(u8 *da) {
	memcpy(TdfSetData.DOCOutTempDst_1, da, 6);
	return 0;
}

int gd_rxq_DOCOutTempDst_2(u8 *da) {
	memcpy(TdfSetData.DOCOutTempDst_2, da, 6);
	return 0;
}

int gd_rxq_DOCOutTempDst_3(u8 *da) {
	memcpy(TdfSetData.DOCOutTempDst_3, da, 6);
	return 0;
}

int gd_rxq_DOCOutTempDst_4(u8 *da) {
	memcpy(TdfSetData.DOCOutTempDst_4, da, 3);
	return 0;
}

int gd_rxq_EGR_1InTempDst_1(u8 *da) {
	memcpy(TdfSetData.EGR_1InTempDst_1, da, 6);
	return 0;
}

int gd_rxq_EGR_1InTempDst_2(u8 *da) {
	memcpy(TdfSetData.EGR_1InTempDst_2, da, 6);
	return 0;
}

int gd_rxq_EGR_1InTempDst_3(u8 *da) {
	memcpy(TdfSetData.EGR_1InTempDst_3, da, 6);
	return 0;
}

int gd_rxq_EGR_1InTempDst_4(u8 *da) {
	memcpy(TdfSetData.EGR_1InTempDst_4, da, 3);
	return 0;
}

int gd_rxq_EGR_1OutTempDst_1(u8 *da) {
	memcpy(TdfSetData.EGR_1OutTempDst_1, da, 6);
	return 0;
}

int gd_rxq_EGR_1OutTempDst_2(u8 *da) {
	memcpy(TdfSetData.EGR_1OutTempDst_2, da, 6);
	return 0;
}

int gd_rxq_EGR_1OutTempDst_3(u8 *da) {
	memcpy(TdfSetData.EGR_1OutTempDst_3, da, 6);
	return 0;
}

int gd_rxq_EGR_1OutTempDst_4(u8 *da) {
	memcpy(TdfSetData.EGR_1OutTempDst_4, da, 3);
	return 0;
}

int gd_rxq_EGR_2InTempDst_1(u8 *da) {
	memcpy(TdfSetData.EGR_2InTempDst_1, da, 6);
	return 0;
}

int gd_rxq_EGR_2InTempDst_2(u8 *da) {
	memcpy(TdfSetData.EGR_2InTempDst_2, da, 6);
	return 0;
}

int gd_rxq_EGR_2InTempDst_3(u8 *da) {
	memcpy(TdfSetData.EGR_2InTempDst_3, da, 6);
	return 0;
}

int gd_rxq_EGR_2InTempDst_4(u8 *da) {
	memcpy(TdfSetData.EGR_2InTempDst_4, da, 3);
	return 0;
}

int gd_rxq_EGR_2OutTempDst_1(u8 *da) {
	memcpy(TdfSetData.EGR_2OutTempDst_1, da, 6);
	return 0;
}

int gd_rxq_EGR_2OutTempDst_2(u8 *da) {
	memcpy(TdfSetData.EGR_2OutTempDst_2, da, 6);
	return 0;
}

int gd_rxq_EGR_2OutTempDst_3(u8 *da) {
	memcpy(TdfSetData.EGR_2OutTempDst_3, da, 6);
	return 0;
}

int gd_rxq_EGR_2OutTempDst_4(u8 *da) {
	memcpy(TdfSetData.EGR_2OutTempDst_4, da, 3);
	return 0;
}

int gd_rxq_InterCoolerTempDst_1(u8 *da) {
	memcpy(TdfSetData.InterCoolerTempDst_1, da, 6);
	return 0;
}

int gd_rxq_InterCoolerTempDst_2(u8 *da) {
	memcpy(TdfSetData.InterCoolerTempDst_2, da, 6);
	return 0;
}

int gd_rxq_InterCoolerTempDst_3(u8 *da) {
	memcpy(TdfSetData.InterCoolerTempDst_3, da, 6);
	return 0;
}

int gd_rxq_InterCoolerTempDst_4(u8 *da) {
	memcpy(TdfSetData.InterCoolerTempDst_4, da, 3);
	return 0;
}

int gd_rxq_ManifoldTempDst_1(u8 *da) {
	memcpy(TdfSetData.ManifoldTempDst_1, da, 6);
	return 0;
}

int gd_rxq_ManifoldTempDst_2(u8 *da) {
	memcpy(TdfSetData.ManifoldTempDst_2, da, 6);
	return 0;
}

int gd_rxq_ManifoldTempDst_3(u8 *da) {
	memcpy(TdfSetData.ManifoldTempDst_3, da, 6);
	return 0;
}

int gd_rxq_ManifoldTempDst_4(u8 *da) {
	memcpy(TdfSetData.ManifoldTempDst_4, da, 3);
	return 0;
}

int gd_rxq_CommonRailPressDst_1(u8 *da) {
	memcpy(TdfSetData.CommonRailPressDst_1, da, 6);
	return 0;
}

int gd_rxq_CommonRailPressDst_2(u8 *da) {
	memcpy(TdfSetData.CommonRailPressDst_2, da, 6);
	return 0;
}

int gd_rxq_CommonRailPressDst_3(u8 *da) {
	memcpy(TdfSetData.CommonRailPressDst_3, da, 6);
	return 0;
}

int gd_rxq_CommonRailPressDst_4(u8 *da) {
	memcpy(TdfSetData.CommonRailPressDst_4, da, 3);
	return 0;
}

int gd_rxq_CommonRailDiffPressDst_1(u8 *da) {
	memcpy(TdfSetData.CommonRailDiffPressDst_1, da, 6);
	return 0;
}

int gd_rxq_CommonRailDiffPressDst_2(u8 *da) {
	memcpy(TdfSetData.CommonRailDiffPressDst_2, da, 6);
	return 0;
}

int gd_rxq_CommonRailDiffPressDst_3(u8 *da) {
	memcpy(TdfSetData.CommonRailDiffPressDst_3, da, 6);
	return 0;
}

int gd_rxq_CommonRailDiffPressDst_4(u8 *da) {
	memcpy(TdfSetData.CommonRailDiffPressDst_4, da, 3);
	return 0;
}

int gd_rxq_DPDDiffPressDst_1(u8 *da) {
	memcpy(TdfSetData.DPDDiffPressDst_1, da, 6);
	return 0;
}

int gd_rxq_DPDDiffPressDst_2(u8 *da) {
	memcpy(TdfSetData.DPDDiffPressDst_2, da, 6);
	return 0;
}

int gd_rxq_DPDDiffPressDst_3(u8 *da) {
	memcpy(TdfSetData.DPDDiffPressDst_3, da, 6);
	return 0;
}

int gd_rxq_DPDDiffPressDst_4(u8 *da) {
	memcpy(TdfSetData.DPDDiffPressDst_4, da, 3);
	return 0;
}

int gd_rxq_AirconditionerBlowerDst_1(u8 *da) {
	memcpy(TdfSetData.AirconditionerBlowerDst_1, da, 6);
	return 0;
}

int gd_rxq_AirconditionerBlowerDst_2(u8 *da) {
	memcpy(TdfSetData.AirconditionerBlowerDst_2, da, 6);
	return 0;
}

int gd_rxq_AirconditionerBlowerDst_3(u8 *da) {
	memcpy(TdfSetData.AirconditionerBlowerDst_3, da, 6);
	return 0;
}

int gd_rxq_AirconditionerTargetTempDst_1(u8 *da) {
	memcpy(TdfSetData.AirconditionerTargetTempDst_1, da, 6);
	return 0;
}

int gd_rxq_AirconditionerTargetTempDst_2(u8 *da) {
	memcpy(TdfSetData.AirconditionerTargetTempDst_2, da, 6);
	return 0;
}

int gd_rxq_AirconditionerTargetTempDst_3(u8 *da) {
	memcpy(TdfSetData.AirconditionerTargetTempDst_3, da, 6);
	return 0;
}

int gd_rxq_AirconditionerTargetTempDst_4(u8 *da) {
	memcpy(TdfSetData.AirconditionerTargetTempDst_4, da, 3);
	return 0;
}

#if 0
int gd_rxq_DistributionReserve_1(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_1, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_2(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_2, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_3(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_3, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_4(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_4, da, 6);
	return 0;
}
int gd_rxq_DistributionReserve_5(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_5, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_6(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_6, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_7(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_7, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_8(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_8, da, 6);
	return 0;
}
int gd_rxq_DistributionReserve_9(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_9, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_10(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_10, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_11(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_11, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_12(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_12, da, 6);
	return 0;
}
int gd_rxq_DistributionReserve_13(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_13, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_14(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_14, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_15(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_15, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_16(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_16, da, 6);
	return 0;
}
int gd_rxq_DistributionReserve_17(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_17, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_18(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_18, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_19(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_19, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_20(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_20, da, 6);
	return 0;
}
int gd_rxq_DistributionReserve_21(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_21, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_22(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_22, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_23(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_23, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_24(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_24, da, 6);
	return 0;
}
int gd_rxq_DistributionReserve_25(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_25, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_26(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_26, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_27(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_27, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_28(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_28, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_29(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_29, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_30(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_30, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_31(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_31, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_32(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_32, da, 6);
	return 0;
}
int gd_rxq_DistributionReserve_33(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_33, da, 6);
	return 0;
}

int gd_rxq_DistributionReserve_34(u8 *da) {
	memcpy(TdfSetData.DistributionReserve_34, da, 6);
	return 0;
}

int gd_rxq_EndOfHistory(u8 *da) {
	memcpy(TdfSetData.EndOfHistory, da, 6);
	return 0;
}

#endif

int gd_rxq_MachineParameter_1(u8 *da) {
	memcpy(TdfSetData.MachineParameter_1, da, 6);
	return 0;
}
int gd_rxq_MachineParameter_2(u8 *da) {
	memcpy(TdfSetData.MachineParameter_2, da, 6);
	return 0;
}

int gd_rxq_MachineParameter_3(u8 *da) {
	memcpy(TdfSetData.MachineParameter_3, da, 6);
	return 0;
}

int gd_rxq_MachineParameter_4(u8 *da) {
	memcpy(TdfSetData.MachineParameter_4, da, 6);
	return 0;
}
int gd_rxq_MachineParameter_5(u8 *da) {
	memcpy(TdfSetData.MachineParameter_5, da, 2);
	return 0;
}

int gd_rxq_MachineParameter_6(u8 *da) {
	memcpy(TdfSetData.MachineParameter_6, da, 3);
	TdfSetData.MachineParameter_6[2] |= 0xF;
	return 0;
}

//int gd_rxq_MachineParameterReserve_1(u8 *da){
//	memcpy(TdfSetData.MachineParameterReserve_1, da, 6);
//	return 0;
//}
//int gd_rxq_MachineParameterReserve_2(u8 *da){
//	memcpy(TdfSetData.MachineParameterReserve_2, da, 6);
//	return 0;
//}
//
//int gd_rxq_MachineParameterReserve_3(u8 *da){
//	memcpy(TdfSetData.MachineParameterReserve_3, da, 6);
//	return 0;
//}
//
//int gd_rxq_MachineParameterReserve_4(u8 *da){
//	memcpy(TdfSetData.MachineParameterReserve_4, da, 6);
//	return 0;
//}

int gd_rxq_Maintenance(u8 *da) {
	memcpy(TdfSetData.Maintenance, da, 6);
	return 0;
}

int gd_rxq_MachineTouch(u8 *da) {
	memcpy(TdfSetData.MachineTouch, da, 6);
	return 0;
}
int gd_rxq_ATSReGenStartNotified(u8 *da) {
	memcpy(&TdfSetData.ATSReGenStartNotified, da, 1);
	TdfSetData.ATSReGenStartNotified |= 0xf3;
	return 0;
}
int gd_rxq_ATSReGenFinishNotified(u8 *da) {
	memcpy(&TdfSetData.ATSReGenFinishNotified, da, 1);
	TdfSetData.ATSReGenFinishNotified |= 0xf3;
	return 0;
}

int gd_rxq_EngineDeviceTestResult(u8 *da) {
	memcpy(&TdfSetData.EngineDeviceTestResult, da, 1);
	return 0;
}

int gd_rxq_ConfSerialNum(u8 *da) {
	memcpy(TdfSetData.ConfSerialNum, da, 4);
	return 0;
}

int gd_rxq_InquireAboutImmobiPass(u8 *da) {
	memcpy(TdfSetData.InquireAboutImmobiPass, da, 3);
	return 0;
}

void PrmID_rxq_device_init(void) {
	PrmID_rxq_fun[PrmID_Ver] = gd_rxq_Ver;
	PrmID_rxq_fun[PrmID_CtlrTPartsNum_1] = gd_rxq_CtlrTPartsNum_1;
	PrmID_rxq_fun[PrmID_CtlrTPartsNum_2] = gd_rxq_CtlrTPartsNum_2;
	PrmID_rxq_fun[PrmID_CtlrT] = gd_rxq_CtlrT;
	PrmID_rxq_fun[PrmID_GpsPos] = gd_rxq_GpsPos;
	PrmID_rxq_fun[PrmID_RtcTim] = gd_rxq_RtcTim;
	PrmID_rxq_fun[PrmID_CtlrTCfg] = gd_rxq_CtlrTCfg;
	PrmID_rxq_fun[PrmID_SerialNumRequest] = gd_rxq_SerialNumRequest;
	PrmID_rxq_fun[PrmID_SerialNumRequestAgain] = gd_rxq_SerialNumRequest;
	PrmID_rxq_fun[PrmID_ImmobiPass] = gd_rxq_ImmobiPass;
	PrmID_rxq_fun[PrmID_MainteTimeFromWeb_1] = gd_rxq_MainteTimeFromWeb_1;
	PrmID_rxq_fun[PrmID_MainteTimeFromWeb_2] = gd_rxq_MainteTimeFromWeb_2;
	PrmID_rxq_fun[PrmID_MainteTimeFromWeb_3] = gd_rxq_MainteTimeFromWeb_3;
	PrmID_rxq_fun[PrmID_MainteTimeFromWeb_4] = gd_rxq_MainteTimeFromWeb_4;
	PrmID_rxq_fun[PrmID_MainteTimeFromWeb_5] = gd_rxq_MainteTimeFromWeb_5;
	PrmID_rxq_fun[PrmID_MainteIntervalFromWeb_1] = gd_rxq_MainteIntervalFromWeb_1;
	PrmID_rxq_fun[PrmID_MainteIntervalFromWeb_2] = gd_rxq_MainteIntervalFromWeb_2;
	PrmID_rxq_fun[PrmID_MainteIntervalFromWeb_3] = gd_rxq_MainteIntervalFromWeb_3;
	PrmID_rxq_fun[PrmID_MainteIntervalFromWeb_4] = gd_rxq_MainteIntervalFromWeb_4;
	PrmID_rxq_fun[PrmID_MainteIntervalFromWeb_5] = gd_rxq_MainteIntervalFromWeb_5;
	PrmID_rxq_fun[PrmID_ParameterConfig] = gd_rxq_ParameterConfig;
	PrmID_rxq_fun[PrmID_EngineDeviceTestRequest] = gd_rxq_EngineDeviceTestRequest;
	PrmID_rxq_fun[PrmID_ImmobiPassAnswer] = gd_rxq_ImmobiPassAnswer;
	PrmID_rxq_fun[PrmID_McnInfo_1] = gd_rxq_McnInfo_1;
	PrmID_rxq_fun[PrmID_McnInfo_2] = gd_rxq_McnInfo_2;
	PrmID_rxq_fun[PrmID_MainCtlrPartsNum_1] = gd_rxq_MainCtlrPartsNum_1;
	PrmID_rxq_fun[PrmID_MainCtlrPartsNum_2] = gd_rxq_MainCtlrPartsNum_2;
	PrmID_rxq_fun[PrmID_MainCtlrSerialNum_1] = gd_rxq_MainCtlrSerialNum_1;
	PrmID_rxq_fun[PrmID_MainCtlrSerialNum_2] = gd_rxq_MainCtlrSerialNum_2;
	PrmID_rxq_fun[PrmID_SubCtlrFirmwareVer] = gd_rxq_SubCtlrFirmwareVer;
	PrmID_rxq_fun[PrmID_SubCtlrSerialNum_1] = gd_rxq_SubCtlrSerialNum_1;
	PrmID_rxq_fun[PrmID_MonitorNum] = gd_rxq_MonitorNum;
	PrmID_rxq_fun[PrmID_MonitorSerialNum] = gd_rxq_MonitorSerialNum;
	PrmID_rxq_fun[PrmID_MainCtlrSts] = gd_rxq_MainCtlrSts;
	PrmID_rxq_fun[PrmID_MainteTime_1] = gd_rxq_MainteTime_1;
	PrmID_rxq_fun[PrmID_MainteTime_2] = gd_rxq_MainteTime_2;
	PrmID_rxq_fun[PrmID_MainteTime_3] = gd_rxq_MainteTime_3;
	PrmID_rxq_fun[PrmID_MainteTime_4] = gd_rxq_MainteTime_4;
	PrmID_rxq_fun[PrmID_MainteTime_5] = gd_rxq_MainteTime_5;
	PrmID_rxq_fun[PrmID_Work_1] = gd_rxq_Work_1;
	PrmID_rxq_fun[PrmID_Work_2] = gd_rxq_Work_2;
	PrmID_rxq_fun[PrmID_UpperWork_1] = gd_rxq_UpperWork_1;
	PrmID_rxq_fun[PrmID_UpperWork_2] = gd_rxq_UpperWork_2;
	PrmID_rxq_fun[PrmID_UpperWork_3] = gd_rxq_UpperWork_3;
	PrmID_rxq_fun[PrmID_UpperWork_4] = gd_rxq_UpperWork_4;
	PrmID_rxq_fun[PrmID_UpperWork_5] = gd_rxq_UpperWork_5;
	PrmID_rxq_fun[PrmID_UpperWork_6] = gd_rxq_UpperWork_6;
	PrmID_rxq_fun[PrmID_TravelWork_1] = gd_rxq_TravelWork_1;
	PrmID_rxq_fun[PrmID_TravelWork_2] = gd_rxq_TravelWork_2;
	PrmID_rxq_fun[PrmID_TravelWork_3] = gd_rxq_TravelWork_3;
	PrmID_rxq_fun[PrmID_WorkMode_1] = gd_rxq_WorkMode_1;
	PrmID_rxq_fun[PrmID_WorkMode_2] = gd_rxq_WorkMode_2;
	PrmID_rxq_fun[PrmID_WorkMode_3] = gd_rxq_WorkMode_3;
	PrmID_rxq_fun[PrmID_WorkMode_4] = gd_rxq_WorkMode_4;
	PrmID_rxq_fun[PrmID_WorkMode_5] = gd_rxq_WorkMode_5;
	PrmID_rxq_fun[PrmID_Breaker] = gd_rxq_Breaker;
	PrmID_rxq_fun[PrmID_Crusher] = gd_rxq_Crusher;
	PrmID_rxq_fun[PrmID_OptionLine_1] = gd_rxq_OptionLine_1;
	PrmID_rxq_fun[PrmID_OptionLine_2] = gd_rxq_OptionLine_2;
	PrmID_rxq_fun[PrmID_QuickCoupler] = gd_rxq_QuickCoupler;
	PrmID_rxq_fun[PrmID_PumpBackup_1] = gd_rxq_PumpBackup_1;
	PrmID_rxq_fun[PrmID_PumpBackup_1] = gd_rxq_PumpBackup_1;
	PrmID_rxq_fun[PrmID_OverheatPowerReduction_1] = gd_rxq_OverheatPowerReduction_1;
	PrmID_rxq_fun[PrmID_OverheatPowerReduction_2] = gd_rxq_OverheatPowerReduction_2;
	PrmID_rxq_fun[PrmID_OverheatPowerReduction_3] = gd_rxq_OverheatPowerReduction_3;
	PrmID_rxq_fun[PrmID_Solenoid_1] = gd_rxq_Solenoid_1;
	PrmID_rxq_fun[PrmID_Solenoid_2] = gd_rxq_Solenoid_2;
	PrmID_rxq_fun[PrmID_Solenoid_3] = gd_rxq_Solenoid_3;
	PrmID_rxq_fun[PrmID_Solenoid_4] = gd_rxq_Solenoid_4;
	PrmID_rxq_fun[PrmID_Solenoid_5] = gd_rxq_Solenoid_5;
	PrmID_rxq_fun[PrmID_Solenoid_6] = gd_rxq_Solenoid_6;
	PrmID_rxq_fun[PrmID_Solenoid_7] = gd_rxq_Solenoid_7;
	PrmID_rxq_fun[PrmID_Solenoid_8] = gd_rxq_Solenoid_8;
	PrmID_rxq_fun[PrmID_Solenoid_9] = gd_rxq_Solenoid_9;
	PrmID_rxq_fun[PrmID_Solenoid_10] = gd_rxq_Solenoid_10;
	PrmID_rxq_fun[PrmID_HotShutdown] = gd_rxq_HotShutdown;
	PrmID_rxq_fun[PrmID_AirConditioner_1] = gd_rxq_AirConditioner_1;
	PrmID_rxq_fun[PrmID_AirConditioner_2] = gd_rxq_AirConditioner_2;
	PrmID_rxq_fun[PrmID_AirConditioner_3] = gd_rxq_AirConditioner_3;
	PrmID_rxq_fun[PrmID_AirConditioner_4] = gd_rxq_AirConditioner_4;
	PrmID_rxq_fun[PrmID_AirConditioner_5] = gd_rxq_AirConditioner_5;
	PrmID_rxq_fun[PrmID_AirConditioner_6] = gd_rxq_AirConditioner_6;
	PrmID_rxq_fun[PrmID_AirConditioner_7] = gd_rxq_AirConditioner_7;
	PrmID_rxq_fun[PrmID_AirConditioner_8] = gd_rxq_AirConditioner_8;
	PrmID_rxq_fun[PrmID_KeyOn] = gd_rxq_KeyOn;
	PrmID_rxq_fun[PrmID_EngineOn] = gd_rxq_EngineOn;
	PrmID_rxq_fun[PrmID_AutoIdle] = gd_rxq_AutoIdle;
	PrmID_rxq_fun[PrmID_IdleStop] = gd_rxq_IdleStop;
	PrmID_rxq_fun[PrmID_WorkOthers_1] = gd_rxq_WorkOthers_1;
	PrmID_rxq_fun[PrmID_WorkOthers_2] = gd_rxq_WorkOthers_2;
	PrmID_rxq_fun[PrmID_WorkOthers_3] = gd_rxq_WorkOthers_3;
	PrmID_rxq_fun[PrmID_Camera_1] = gd_rxq_Camera_1;
	PrmID_rxq_fun[PrmID_Camera_2] = gd_rxq_Camera_2;
	PrmID_rxq_fun[PrmID_Wiper_1] = gd_rxq_Wiper_1;
	PrmID_rxq_fun[PrmID_Wiper_2] = gd_rxq_Wiper_2;
	PrmID_rxq_fun[PrmID_Window] = gd_rxq_Window;
	PrmID_rxq_fun[PrmID_Door] = gd_rxq_Door;
	PrmID_rxq_fun[PrmID_MaxMin_1] = gd_rxq_MaxMin_1;
	PrmID_rxq_fun[PrmID_MaxMin_2] = gd_rxq_MaxMin_2;
	PrmID_rxq_fun[PrmID_MaxMin_3] = gd_rxq_MaxMin_3;
	PrmID_rxq_fun[PrmID_MaxMin_4] = gd_rxq_MaxMin_4;
	PrmID_rxq_fun[PrmID_MaxMin_5] = gd_rxq_MaxMin_5;
	PrmID_rxq_fun[PrmID_MaxMin_6] = gd_rxq_MaxMin_6;
	PrmID_rxq_fun[PrmID_MaxMin_7] = gd_rxq_MaxMin_7;
	PrmID_rxq_fun[PrmID_MaxMin_8] = gd_rxq_MaxMin_8;
	PrmID_rxq_fun[PrmID_MaxMin_9] = gd_rxq_MaxMin_9;
	PrmID_rxq_fun[PrmID_MaxMin_10] = gd_rxq_MaxMin_10;
	PrmID_rxq_fun[PrmID_MaxMin_11] = gd_rxq_MaxMin_11;
	PrmID_rxq_fun[PrmID_MaxMin_12] = gd_rxq_MaxMin_12;
	PrmID_rxq_fun[PrmID_MaxMin_13] = gd_rxq_MaxMin_13;
	PrmID_rxq_fun[PrmID_MaxMin_14] = gd_rxq_MaxMin_14;
	PrmID_rxq_fun[PrmID_P1PressDst_1] = gd_rxq_P1PressDst_1;
	PrmID_rxq_fun[PrmID_P1PressDst_2] = gd_rxq_P1PressDst_2;
	PrmID_rxq_fun[PrmID_P1PressDst_3] = gd_rxq_P1PressDst_3;
	PrmID_rxq_fun[PrmID_P1PressDst_4] = gd_rxq_P1PressDst_4;
	PrmID_rxq_fun[PrmID_P2PressDst_1] = gd_rxq_P2PressDst_1;
	PrmID_rxq_fun[PrmID_P2PressDst_2] = gd_rxq_P2PressDst_2;
	PrmID_rxq_fun[PrmID_P2PressDst_3] = gd_rxq_P2PressDst_3;
	PrmID_rxq_fun[PrmID_P2PressDst_4] = gd_rxq_P2PressDst_4;
	PrmID_rxq_fun[PrmID_N1PressDst_1] = gd_rxq_N1PressDst_1;
	PrmID_rxq_fun[PrmID_N1PressDst_2] = gd_rxq_N1PressDst_2;
	PrmID_rxq_fun[PrmID_N1PressDst_3] = gd_rxq_N1PressDst_3;
	PrmID_rxq_fun[PrmID_N1PressDst_4] = gd_rxq_N1PressDst_4;
	PrmID_rxq_fun[PrmID_N2PressDst_1] = gd_rxq_N2PressDst_1;
	PrmID_rxq_fun[PrmID_N2PressDst_2] = gd_rxq_N2PressDst_2;
	PrmID_rxq_fun[PrmID_N2PressDst_3] = gd_rxq_N2PressDst_3;
	PrmID_rxq_fun[PrmID_N2PressDst_4] = gd_rxq_N2PressDst_4;
	PrmID_rxq_fun[PrmID_P1P2PressDst_1] = gd_rxq_P1P2PressDst_1;
	PrmID_rxq_fun[PrmID_P1P2PressDst_2] = gd_rxq_P1P2PressDst_2;
	PrmID_rxq_fun[PrmID_P1P2PressDst_3] = gd_rxq_P1P2PressDst_3;
	PrmID_rxq_fun[PrmID_P1P2PressDst_4] = gd_rxq_P1P2PressDst_4;
	PrmID_rxq_fun[PrmID_TravelP1PressDst_1] = gd_rxq_TravelP1PressDst_1;
	PrmID_rxq_fun[PrmID_TravelP1PressDst_2] = gd_rxq_TravelP1PressDst_2;
	PrmID_rxq_fun[PrmID_TravelP1PressDst_3] = gd_rxq_TravelP1PressDst_3;
	PrmID_rxq_fun[PrmID_TravelP1PressDst_4] = gd_rxq_TravelP1PressDst_4;
	PrmID_rxq_fun[PrmID_TravelP2PressDst_1] = gd_rxq_TravelP2PressDst_1;
	PrmID_rxq_fun[PrmID_TravelP2PressDst_2] = gd_rxq_TravelP2PressDst_2;
	PrmID_rxq_fun[PrmID_TravelP2PressDst_3] = gd_rxq_TravelP2PressDst_3;
	PrmID_rxq_fun[PrmID_TravelP2PressDst_4] = gd_rxq_TravelP2PressDst_4;
	PrmID_rxq_fun[PrmID_ArmCylBottomPressDst_1] = gd_rxq_ArmCylBottomPressDst_1;
	PrmID_rxq_fun[PrmID_ArmCylBottomPressDst_2] = gd_rxq_ArmCylBottomPressDst_2;
	PrmID_rxq_fun[PrmID_ArmCylBottomPressDst_3] = gd_rxq_ArmCylBottomPressDst_3;
	PrmID_rxq_fun[PrmID_ArmCylBottomPressDst_4] = gd_rxq_ArmCylBottomPressDst_4;
	PrmID_rxq_fun[PrmID_BoomCylBottomPressDst_1] = gd_rxq_BoomCylBottomPressDst_1;
	PrmID_rxq_fun[PrmID_BoomCylBottomPressDst_2] = gd_rxq_BoomCylBottomPressDst_2;
	PrmID_rxq_fun[PrmID_BoomCylBottomPressDst_3] = gd_rxq_BoomCylBottomPressDst_3;
	PrmID_rxq_fun[PrmID_BoomCylBottomPressDst_4] = gd_rxq_BoomCylBottomPressDst_4;
	PrmID_rxq_fun[PrmID_BoomCylRodPressDst_1] = gd_rxq_BoomCylRodPressDst_1;
	PrmID_rxq_fun[PrmID_BoomCylRodPressDst_2] = gd_rxq_BoomCylRodPressDst_2;
	PrmID_rxq_fun[PrmID_BoomCylRodPressDst_3] = gd_rxq_BoomCylRodPressDst_3;
	PrmID_rxq_fun[PrmID_BoomCylRodPressDst_4] = gd_rxq_BoomCylRodPressDst_4;
	PrmID_rxq_fun[PrmID_HydOilTempDst_1] = gd_rxq_HydOilTempDst_1;
	PrmID_rxq_fun[PrmID_HydOilTempDst_2] = gd_rxq_HydOilTempDst_2;
	PrmID_rxq_fun[PrmID_HydOilTempDst_3] = gd_rxq_HydOilTempDst_3;
	PrmID_rxq_fun[PrmID_HydOilTempDst_4] = gd_rxq_HydOilTempDst_4;
	PrmID_rxq_fun[PrmID_HydOilFilterPressDst_1] = gd_rxq_HydOilFilterPressDst_1;
	PrmID_rxq_fun[PrmID_HydOilFilterPressDst_2] = gd_rxq_HydOilFilterPressDst_2;
	PrmID_rxq_fun[PrmID_HydOilFilterPressDst_3] = gd_rxq_HydOilFilterPressDst_3;
	PrmID_rxq_fun[PrmID_HydOilFilterPressDst_4] = gd_rxq_HydOilFilterPressDst_4;
	PrmID_rxq_fun[PrmID_TravelOprTimeDst_1] = gd_rxq_TravelOprTimeDst_1;
	PrmID_rxq_fun[PrmID_TravelOprTimeDst_2] = gd_rxq_TravelOprTimeDst_2;
	PrmID_rxq_fun[PrmID_TravelOprTimeDst_3] = gd_rxq_TravelOprTimeDst_3;
	PrmID_rxq_fun[PrmID_TravelOprTimeMax_1] = gd_rxq_TravelOprTimeMax_1;
	PrmID_rxq_fun[PrmID_TravelOprTimeMax_2] = gd_rxq_TravelOprTimeMax_2;
	PrmID_rxq_fun[PrmID_EngineActualSpeedDst_1] = gd_rxq_EngineActualSpeedDst_1;
	PrmID_rxq_fun[PrmID_EngineActualSpeedDst_2] = gd_rxq_EngineActualSpeedDst_2;
	PrmID_rxq_fun[PrmID_EngineActualSpeedDst_3] = gd_rxq_EngineActualSpeedDst_3;
	PrmID_rxq_fun[PrmID_EngineActualSpeedDst_4] = gd_rxq_EngineActualSpeedDst_4;
	PrmID_rxq_fun[PrmID_CoolantTempDst_1] = gd_rxq_CoolantTempDst_1;
	PrmID_rxq_fun[PrmID_CoolantTempDst_2] = gd_rxq_CoolantTempDst_2;
	PrmID_rxq_fun[PrmID_CoolantTempDst_3] = gd_rxq_CoolantTempDst_3;
	PrmID_rxq_fun[PrmID_CoolantTempDst_4] = gd_rxq_CoolantTempDst_4;
	PrmID_rxq_fun[PrmID_CoolDownTimeDst_1] = gd_rxq_CoolDownTimeDst_1;
	PrmID_rxq_fun[PrmID_CoolDownTimeDst_2] = gd_rxq_CoolDownTimeDst_2;
	PrmID_rxq_fun[PrmID_FuelTempDst_1] = gd_rxq_FuelTempDst_1;
	PrmID_rxq_fun[PrmID_FuelTempDst_2] = gd_rxq_FuelTempDst_2;
	PrmID_rxq_fun[PrmID_FuelTempDst_3] = gd_rxq_FuelTempDst_3;
	PrmID_rxq_fun[PrmID_FuelTempDst_4] = gd_rxq_FuelTempDst_4;
	PrmID_rxq_fun[PrmID_InletAirTempDst_1] = gd_rxq_InletAirTempDst_1;
	PrmID_rxq_fun[PrmID_InletAirTempDst_2] = gd_rxq_InletAirTempDst_2;
	PrmID_rxq_fun[PrmID_InletAirTempDst_3] = gd_rxq_InletAirTempDst_3;
	PrmID_rxq_fun[PrmID_InletAirTempDst_4] = gd_rxq_InletAirTempDst_4;
	PrmID_rxq_fun[PrmID_BoostTempDst_1] = gd_rxq_BoostTempDst_1;
	PrmID_rxq_fun[PrmID_BoostTempDst_2] = gd_rxq_BoostTempDst_2;
	PrmID_rxq_fun[PrmID_BoostTempDst_3] = gd_rxq_BoostTempDst_3;
	PrmID_rxq_fun[PrmID_BoostTempDst_4] = gd_rxq_BoostTempDst_4;
	PrmID_rxq_fun[PrmID_BaroPressDst_1] = gd_rxq_BaroPressDst_1;
	PrmID_rxq_fun[PrmID_BaroPressDst_2] = gd_rxq_BaroPressDst_2;
	PrmID_rxq_fun[PrmID_BaroPressDst_3] = gd_rxq_BaroPressDst_3;
	PrmID_rxq_fun[PrmID_BaroPressDst_4] = gd_rxq_BaroPressDst_4;
	PrmID_rxq_fun[PrmID_EngineOilPressDst_1] = gd_rxq_EngineOilPressDst_1;
	PrmID_rxq_fun[PrmID_EngineOilPressDst_2] = gd_rxq_EngineOilPressDst_2;
	PrmID_rxq_fun[PrmID_EngineOilPressDst_3] = gd_rxq_EngineOilPressDst_3;
	PrmID_rxq_fun[PrmID_EngineOilPressDst_4] = gd_rxq_EngineOilPressDst_4;
	PrmID_rxq_fun[PrmID_EngineOilPressRiseTimeDst_1] = gd_rxq_EngineOilPressRiseTimeDst_1;
	PrmID_rxq_fun[PrmID_EngineOilPressRiseTimeDst_2] = gd_rxq_EngineOilPressRiseTimeDst_2;
	PrmID_rxq_fun[PrmID_EngineOilPressRiseTimeDst_3] = gd_rxq_EngineOilPressRiseTimeDst_3;
	PrmID_rxq_fun[PrmID_BoostPressDst_1] = gd_rxq_BoostPressDst_1;
	PrmID_rxq_fun[PrmID_BoostPressDst_2] = gd_rxq_BoostPressDst_2;
	PrmID_rxq_fun[PrmID_BoostPressDst_3] = gd_rxq_BoostPressDst_3;
	PrmID_rxq_fun[PrmID_BoostPressDst_4] = gd_rxq_BoostPressDst_4;
	PrmID_rxq_fun[PrmID_EngineLoadRatioDst_1] = gd_rxq_EngineLoadRatioDst_1;
	PrmID_rxq_fun[PrmID_EngineLoadRatioDst_2] = gd_rxq_EngineLoadRatioDst_2;
	PrmID_rxq_fun[PrmID_EngineLoadRatioDst_3] = gd_rxq_EngineLoadRatioDst_3;
	PrmID_rxq_fun[PrmID_EngineLoadRatioDst_4] = gd_rxq_EngineLoadRatioDst_4;
	PrmID_rxq_fun[PrmID_EngineLoadRatioSPDst_1] = gd_rxq_EngineLoadRatioSPDst_1;
	PrmID_rxq_fun[PrmID_EngineLoadRatioSPDst_2] = gd_rxq_EngineLoadRatioSPDst_2;
	PrmID_rxq_fun[PrmID_EngineLoadRatioSPDst_3] = gd_rxq_EngineLoadRatioSPDst_3;
	PrmID_rxq_fun[PrmID_EngineLoadRatioSPDst_4] = gd_rxq_EngineLoadRatioSPDst_4;
	PrmID_rxq_fun[PrmID_EngineLoadRatioHDst_1] = gd_rxq_EngineLoadRatioHDst_1;
	PrmID_rxq_fun[PrmID_EngineLoadRatioHDst_2] = gd_rxq_EngineLoadRatioHDst_2;
	PrmID_rxq_fun[PrmID_EngineLoadRatioHDst_3] = gd_rxq_EngineLoadRatioHDst_3;
	PrmID_rxq_fun[PrmID_EngineLoadRatioHDst_4] = gd_rxq_EngineLoadRatioHDst_4;
	PrmID_rxq_fun[PrmID_EngineLoadRatioADst_1] = gd_rxq_EngineLoadRatioADst_1;
	PrmID_rxq_fun[PrmID_EngineLoadRatioADst_2] = gd_rxq_EngineLoadRatioADst_2;
	PrmID_rxq_fun[PrmID_EngineLoadRatioADst_3] = gd_rxq_EngineLoadRatioADst_3;
	PrmID_rxq_fun[PrmID_EngineLoadRatioADst_4] = gd_rxq_EngineLoadRatioADst_4;
	PrmID_rxq_fun[PrmID_SupplyPumpPressDst_1] = gd_rxq_SupplyPumpPressDst_1;
	PrmID_rxq_fun[PrmID_SupplyPumpPressDst_2] = gd_rxq_SupplyPumpPressDst_2;
	PrmID_rxq_fun[PrmID_SupplyPumpPressDst_3] = gd_rxq_SupplyPumpPressDst_3;
	PrmID_rxq_fun[PrmID_SupplyPumpPressDst_4] = gd_rxq_SupplyPumpPressDst_4;
	PrmID_rxq_fun[PrmID_DOCInTempDst_1] = gd_rxq_DOCInTempDst_1;
	PrmID_rxq_fun[PrmID_DOCInTempDst_2] = gd_rxq_DOCInTempDst_2;
	PrmID_rxq_fun[PrmID_DOCInTempDst_3] = gd_rxq_DOCInTempDst_3;
	PrmID_rxq_fun[PrmID_DOCInTempDst_4] = gd_rxq_DOCInTempDst_4;
	PrmID_rxq_fun[PrmID_DOCOutTempDst_1] = gd_rxq_DOCOutTempDst_1;
	PrmID_rxq_fun[PrmID_DOCOutTempDst_2] = gd_rxq_DOCOutTempDst_2;
	PrmID_rxq_fun[PrmID_DOCOutTempDst_3] = gd_rxq_DOCOutTempDst_3;
	PrmID_rxq_fun[PrmID_DOCOutTempDst_4] = gd_rxq_DOCOutTempDst_4;
	PrmID_rxq_fun[PrmID_EGR_1InTempDst_1] = gd_rxq_EGR_1InTempDst_1;
	PrmID_rxq_fun[PrmID_EGR_1InTempDst_2] = gd_rxq_EGR_1InTempDst_2;
	PrmID_rxq_fun[PrmID_EGR_1InTempDst_3] = gd_rxq_EGR_1InTempDst_3;
	PrmID_rxq_fun[PrmID_EGR_1InTempDst_4] = gd_rxq_EGR_1InTempDst_4;
	PrmID_rxq_fun[PrmID_EGR_1OutTempDst_1] = gd_rxq_EGR_1OutTempDst_1;
	PrmID_rxq_fun[PrmID_EGR_1OutTempDst_2] = gd_rxq_EGR_1OutTempDst_2;
	PrmID_rxq_fun[PrmID_EGR_1OutTempDst_3] = gd_rxq_EGR_1OutTempDst_3;
	PrmID_rxq_fun[PrmID_EGR_1OutTempDst_4] = gd_rxq_EGR_1OutTempDst_4;
	PrmID_rxq_fun[PrmID_EGR_2InTempDst_1] = gd_rxq_EGR_2InTempDst_1;
	PrmID_rxq_fun[PrmID_EGR_2InTempDst_2] = gd_rxq_EGR_2InTempDst_2;
	PrmID_rxq_fun[PrmID_EGR_2InTempDst_3] = gd_rxq_EGR_2InTempDst_3;
	PrmID_rxq_fun[PrmID_EGR_2InTempDst_4] = gd_rxq_EGR_2InTempDst_4;
	PrmID_rxq_fun[PrmID_EGR_2OutTempDst_1] = gd_rxq_EGR_2OutTempDst_1;
	PrmID_rxq_fun[PrmID_EGR_2OutTempDst_2] = gd_rxq_EGR_2OutTempDst_2;
	PrmID_rxq_fun[PrmID_EGR_2OutTempDst_3] = gd_rxq_EGR_2OutTempDst_3;
	PrmID_rxq_fun[PrmID_EGR_2OutTempDst_4] = gd_rxq_EGR_2OutTempDst_4;
	PrmID_rxq_fun[PrmID_InterCoolerTempDst_1] = gd_rxq_InterCoolerTempDst_1;
	PrmID_rxq_fun[PrmID_InterCoolerTempDst_2] = gd_rxq_InterCoolerTempDst_2;
	PrmID_rxq_fun[PrmID_InterCoolerTempDst_3] = gd_rxq_InterCoolerTempDst_3;
	PrmID_rxq_fun[PrmID_InterCoolerTempDst_4] = gd_rxq_InterCoolerTempDst_4;
	PrmID_rxq_fun[PrmID_ManifoldTempDst_1] = gd_rxq_ManifoldTempDst_1;
	PrmID_rxq_fun[PrmID_ManifoldTempDst_2] = gd_rxq_ManifoldTempDst_2;
	PrmID_rxq_fun[PrmID_ManifoldTempDst_3] = gd_rxq_ManifoldTempDst_3;
	PrmID_rxq_fun[PrmID_ManifoldTempDst_4] = gd_rxq_ManifoldTempDst_4;
	PrmID_rxq_fun[PrmID_CommonRailPressDst_1] = gd_rxq_CommonRailPressDst_1;
	PrmID_rxq_fun[PrmID_CommonRailPressDst_2] = gd_rxq_CommonRailPressDst_2;
	PrmID_rxq_fun[PrmID_CommonRailPressDst_3] = gd_rxq_CommonRailPressDst_3;
	PrmID_rxq_fun[PrmID_CommonRailPressDst_4] = gd_rxq_CommonRailPressDst_4;
	PrmID_rxq_fun[PrmID_CommonRailDiffPressDst_1] = gd_rxq_CommonRailDiffPressDst_1;
	PrmID_rxq_fun[PrmID_CommonRailDiffPressDst_2] = gd_rxq_CommonRailDiffPressDst_2;
	PrmID_rxq_fun[PrmID_CommonRailDiffPressDst_3] = gd_rxq_CommonRailDiffPressDst_3;
	PrmID_rxq_fun[PrmID_CommonRailDiffPressDst_4] = gd_rxq_CommonRailDiffPressDst_4;
	PrmID_rxq_fun[PrmID_DPDDiffPressDst_1] = gd_rxq_DPDDiffPressDst_1;
	PrmID_rxq_fun[PrmID_DPDDiffPressDst_2] = gd_rxq_DPDDiffPressDst_2;
	PrmID_rxq_fun[PrmID_DPDDiffPressDst_3] = gd_rxq_DPDDiffPressDst_3;
	PrmID_rxq_fun[PrmID_DPDDiffPressDst_4] = gd_rxq_DPDDiffPressDst_4;
	PrmID_rxq_fun[PrmID_AirconditionerBlowerDst_1] = gd_rxq_AirconditionerBlowerDst_1;
	PrmID_rxq_fun[PrmID_AirconditionerBlowerDst_2] = gd_rxq_AirconditionerBlowerDst_2;
	PrmID_rxq_fun[PrmID_AirconditionerBlowerDst_3] = gd_rxq_AirconditionerBlowerDst_3;
	PrmID_rxq_fun[PrmID_AirconditionerTargetTempDst_1] = gd_rxq_AirconditionerTargetTempDst_1;
	PrmID_rxq_fun[PrmID_AirconditionerTargetTempDst_2] = gd_rxq_AirconditionerTargetTempDst_2;
	PrmID_rxq_fun[PrmID_AirconditionerTargetTempDst_3] = gd_rxq_AirconditionerTargetTempDst_3;
	PrmID_rxq_fun[PrmID_AirconditionerTargetTempDst_4] = gd_rxq_AirconditionerTargetTempDst_4;
//	PrmID_rxq_fun[PrmID_DistributionReserve_1] = gd_rxq_DistributionReserve_1;
//	PrmID_rxq_fun[PrmID_DistributionReserve_2] = gd_rxq_DistributionReserve_2;
//	PrmID_rxq_fun[PrmID_DistributionReserve_3] = gd_rxq_DistributionReserve_3;
//	PrmID_rxq_fun[PrmID_DistributionReserve_4] = gd_rxq_DistributionReserve_4;
//	PrmID_rxq_fun[PrmID_DistributionReserve_5] = gd_rxq_DistributionReserve_5;
//	PrmID_rxq_fun[PrmID_DistributionReserve_6] = gd_rxq_DistributionReserve_6;
//	PrmID_rxq_fun[PrmID_DistributionReserve_7] = gd_rxq_DistributionReserve_7;
//	PrmID_rxq_fun[PrmID_DistributionReserve_8] = gd_rxq_DistributionReserve_8;
//	PrmID_rxq_fun[PrmID_DistributionReserve_9] = gd_rxq_DistributionReserve_9;
//	PrmID_rxq_fun[PrmID_DistributionReserve_10] = gd_rxq_DistributionReserve_10;
//	PrmID_rxq_fun[PrmID_DistributionReserve_11] = gd_rxq_DistributionReserve_11;
//	PrmID_rxq_fun[PrmID_DistributionReserve_12] = gd_rxq_DistributionReserve_12;
//	PrmID_rxq_fun[PrmID_DistributionReserve_13] = gd_rxq_DistributionReserve_13;
//	PrmID_rxq_fun[PrmID_DistributionReserve_14] = gd_rxq_DistributionReserve_14;
//	PrmID_rxq_fun[PrmID_DistributionReserve_15] = gd_rxq_DistributionReserve_15;
//	PrmID_rxq_fun[PrmID_DistributionReserve_16] = gd_rxq_DistributionReserve_16;
//	PrmID_rxq_fun[PrmID_DistributionReserve_17] = gd_rxq_DistributionReserve_17;
//	PrmID_rxq_fun[PrmID_DistributionReserve_18] = gd_rxq_DistributionReserve_18;
//	PrmID_rxq_fun[PrmID_DistributionReserve_19] = gd_rxq_DistributionReserve_19;
//	PrmID_rxq_fun[PrmID_DistributionReserve_20] = gd_rxq_DistributionReserve_20;
//	PrmID_rxq_fun[PrmID_DistributionReserve_21] = gd_rxq_DistributionReserve_21;
//	PrmID_rxq_fun[PrmID_DistributionReserve_22] = gd_rxq_DistributionReserve_22;
//	PrmID_rxq_fun[PrmID_DistributionReserve_23] = gd_rxq_DistributionReserve_23;
//	PrmID_rxq_fun[PrmID_DistributionReserve_24] = gd_rxq_DistributionReserve_24;
//	PrmID_rxq_fun[PrmID_DistributionReserve_25] = gd_rxq_DistributionReserve_25;
//	PrmID_rxq_fun[PrmID_DistributionReserve_26] = gd_rxq_DistributionReserve_26;
//	PrmID_rxq_fun[PrmID_DistributionReserve_27] = gd_rxq_DistributionReserve_27;
//	PrmID_rxq_fun[PrmID_DistributionReserve_28] = gd_rxq_DistributionReserve_28;
//	PrmID_rxq_fun[PrmID_DistributionReserve_29] = gd_rxq_DistributionReserve_29;
//	PrmID_rxq_fun[PrmID_DistributionReserve_30] = gd_rxq_DistributionReserve_30;
//	PrmID_rxq_fun[PrmID_DistributionReserve_31] = gd_rxq_DistributionReserve_31;
//	PrmID_rxq_fun[PrmID_DistributionReserve_32] = gd_rxq_DistributionReserve_32;
//	PrmID_rxq_fun[PrmID_DistributionReserve_33] = gd_rxq_DistributionReserve_33;
//	PrmID_rxq_fun[PrmID_DistributionReserve_34] = gd_rxq_DistributionReserve_34;
//	PrmID_rxq_fun[PrmID_EndOfHistory] = gd_rxq_EndOfHistory;
	PrmID_rxq_fun[PrmID_MachineParameter_1] = gd_rxq_MachineParameter_1;
	PrmID_rxq_fun[PrmID_MachineParameter_2] = gd_rxq_MachineParameter_2;
	PrmID_rxq_fun[PrmID_MachineParameter_3] = gd_rxq_MachineParameter_3;
	PrmID_rxq_fun[PrmID_MachineParameter_4] = gd_rxq_MachineParameter_4;
	PrmID_rxq_fun[PrmID_MachineParameter_5] = gd_rxq_MachineParameter_5;
	PrmID_rxq_fun[PrmID_MachineParameter_6] = gd_rxq_MachineParameter_6;
//	PrmID_rxq_fun[PrmID_MachineParameterReserve_1] = gd_rxq_MachineParameterReserve_1;
//	PrmID_rxq_fun[PrmID_MachineParameterReserve_2] = gd_rxq_MachineParameterReserve_2;
//	PrmID_rxq_fun[PrmID_MachineParameterReserve_3] = gd_rxq_MachineParameterReserve_3;
//	PrmID_rxq_fun[PrmID_MachineParameterReserve_4] = gd_rxq_MachineParameterReserve_4;
	PrmID_rxq_fun[PrmID_Maintenance] = gd_rxq_Maintenance;
	PrmID_rxq_fun[PrmID_MachineTouch] = gd_rxq_MachineTouch;
	PrmID_rxq_fun[PrmID_ATSReGenStartNotified] = gd_rxq_ATSReGenStartNotified;
	PrmID_rxq_fun[PrmID_ATSReGenFinishNotified] = gd_rxq_ATSReGenFinishNotified;
	PrmID_rxq_fun[PrmID_EngineDeviceTestResult] = gd_rxq_EngineDeviceTestResult;
	PrmID_rxq_fun[PrmID_ConfSerialNum] = gd_rxq_ConfSerialNum;
	PrmID_rxq_fun[PrmID_InquireAboutImmobiPass] = gd_rxq_InquireAboutImmobiPass;

}

/************************************************************
 * @brief  CAN信息出队
 *  @param  msg  :CAN信息存储
 * @retval    1：有信息，0：无信息
 *************************************************************/
int CanA_Data_Get(CanMsg *msg) {
	pthread_mutex_lock(&CanAmutex);
	int b = 0;
	if (CANA_RX_Save_Index != CANA_RX_Read_Index) {
		*msg = CANA_RX_Data[CANA_RX_Read_Index];
//		can_msg_deal(msg);
		CANA_RX_Read_Index++;
		if (CANA_RX_Read_Index >= CANA_RX_MAX)
			CANA_RX_Read_Index = 0;
		b = 1;
//      printf("read ok CAN0_RX_Read_Index = %d \n",CAN0_RX_Read_Index);
	}
//    printf("msg.ID: = %x  CAN0_RX_Read_Index:%d\n",msg->ID,CAN0_RX_Read_Index);
	pthread_mutex_unlock(&CanAmutex);
	return b;
}
int CanA_Data_In(struct can_frame* candata, int canx) {
	int j, i = 0;
	ulong Sys_run_time_ms = 0; //zjx-2018-7-19
	pthread_mutex_lock(&CanAmutex);
	Sys_run_time_ms = api_GetSysmSecs();
	CANA_RX_Data[CANA_RX_Save_Index].TIME = Sys_run_time_ms;
	CANA_RX_Data[CANA_RX_Save_Index].ID = candata[i].can_id; /* Standard ID */
	CANA_RX_Data[CANA_RX_Save_Index].SOURCE = canx;
	for (j = 0; j < 8; j++) {
		CANB_RX_Data[CANB_RX_Save_Index].DATA[j] = candata[i].data[j];
	}
	//          printf("canx:%d;CAN1_RX_Data[].ID :%08x\n",canx,CAN1_RX_Data[CAN1_RX_Save_Index].ID);
	//          hprintf( CAN1_RX_Data[CAN1_RX_Save_Index].DATA,8);
	CANA_RX_Save_Index++;
	if (CANA_RX_Save_Index >= CANA_RX_MAX)
		CANA_RX_Save_Index = 0;
	if (CANA_RX_Read_Index == CANA_RX_Save_Index)
		CANA_RX_Read_Index++;
	if (CANA_RX_Read_Index >= CANA_RX_MAX)
		CANA_RX_Read_Index = 0;
	pthread_mutex_unlock(&CanAmutex);
	return 0;
}
int CanB_Data_Get(CanMsg *msg) {
	pthread_mutex_lock(&CanBmutex);

	int b = 0;

	if (CANB_RX_Save_Index != CANB_RX_Read_Index) {
		*msg = CANB_RX_Data[CANB_RX_Read_Index];
//		can_msg_deal(msg);
		CANB_RX_Read_Index++;
		if (CANB_RX_Read_Index >= CANB_RX_MAX)
			CANB_RX_Read_Index = 0;
		b = 1;
//      printf("read ok CAN0_RX_Read_Index = %d \n",CAN0_RX_Read_Index);
	}

//    printf("msg.ID: = %x  CAN0_RX_Read_Index:%d\n",msg->ID,CAN0_RX_Read_Index);
	pthread_mutex_unlock(&CanBmutex);
	return b;
}
int CanB_Data_In(struct can_frame* candata, int canx) {
	int j, i = 0;
	ulong Sys_run_time_ms = 0; //zjx-2018-7-19
	pthread_mutex_lock(&CanBmutex);
	Sys_run_time_ms = api_GetSysmSecs();
	CANB_RX_Data[CANB_RX_Save_Index].TIME = Sys_run_time_ms;
	CANB_RX_Data[CANB_RX_Save_Index].ID = candata[i].can_id; /* Standard ID */
	CANB_RX_Data[CANB_RX_Save_Index].SOURCE = canx;
	for (j = 0; j < 8; j++) {
		CANB_RX_Data[CANB_RX_Save_Index].DATA[j] = candata[i].data[j];
	}
	//          printf("canx:%d;CAN1_RX_Data[].ID :%08x\n",canx,CAN1_RX_Data[CAN1_RX_Save_Index].ID);
	//          hprintf( CAN1_RX_Data[CAN1_RX_Save_Index].DATA,8);
	CANB_RX_Save_Index++;
	if (CANB_RX_Save_Index >= CANB_RX_MAX)
		CANB_RX_Save_Index = 0;
	if (CANB_RX_Read_Index == CANB_RX_Save_Index)
		CANB_RX_Read_Index++;
	if (CANB_RX_Read_Index >= CANB_RX_MAX)
		CANB_RX_Read_Index = 0;
	pthread_mutex_unlock(&CanBmutex);
	return 0;
}
int CanC_Data_Get(CanMsg *msg) {
	pthread_mutex_lock(&CanCmutex);
	int b = 0;
	if (CANC_RX_Save_Index != CANC_RX_Read_Index) {
		*msg = CANC_RX_Data[CANC_RX_Read_Index];
//		can_msg_deal(msg);
		CANC_RX_Read_Index++;
		if (CANC_RX_Read_Index >= CANC_RX_MAX)
			CANC_RX_Read_Index = 0;
		b = 1;
//      printf("read ok CAN0_RX_Read_Index = %d \n",CAN0_RX_Read_Index);
	}
	pthread_mutex_unlock(&CanCmutex);
	return b;
}
int CanC_Data_In(struct can_frame* candata, int canx) {
	int j, i = 0;
	ulong Sys_run_time_ms = 0; //zjx-2018-7-19
	pthread_mutex_lock(&CanCmutex);
	Sys_run_time_ms = api_GetSysmSecs();
	CANC_RX_Data[CANC_RX_Save_Index].TIME = Sys_run_time_ms;
	CANC_RX_Data[CANC_RX_Save_Index].ID = candata[i].can_id; /* Standard ID */
	CANC_RX_Data[CANC_RX_Save_Index].SOURCE = canx;
	for (j = 0; j < 8; j++) {
		CANC_RX_Data[CANC_RX_Save_Index].DATA[j] = candata[i].data[j];
	}
	//          printf("canx:%d;CAN1_RX_Data[].ID :%08x\n",canx,CAN1_RX_Data[CAN1_RX_Save_Index].ID);
	//          hprintf( CAN1_RX_Data[CAN1_RX_Save_Index].DATA,8);

	CANC_RX_Save_Index++;
	if (CANC_RX_Save_Index >= CANC_RX_MAX)
		CANC_RX_Save_Index = 0;
	if (CANC_RX_Read_Index == CANC_RX_Save_Index)
		CANC_RX_Read_Index++;
	if (CANC_RX_Read_Index >= CANC_RX_MAX)
		CANC_RX_Read_Index = 0;
	pthread_mutex_unlock(&CanCmutex);
	return 0;
}
/********************************************************************
 *func  name:cand_RecvHandle
 * function:
 * canx: if can0  :0;
 * 			 if can1 :1;
 *nframe:数据包数
 *can数据入队
 ********************************************************************/
int cand_RecvHandle(int canx, struct can_frame* candata, int nframe) {
	int i = 0;
	u16 tmp_u16;
	u32 tmp_u32;
	switch (canx)
	{
	case 0:	//can0(canB)
		for (i = 0; i < nframe; i++) {
			tmp_u32 = candata[1].can_id << 3;
			switch (tmp_u32)
			{	//ECM
			case cand_CoolFuelT_ID:	//发动机冷却液、燃料温度
				cand_CoolantT = candata[i].data[0];
				cand_FuelT = candata[i].data[1];
				break;
			case cand_Air_PT_ID:	//AMBIENT CONDITIONS
				cand_BarPress = candata[i].data[0];
				tmp_u16 = candata[i].data[4];
				tmp_u16 = (tmp_u16 << 8) | candata[i].data[3];
				cand_AmbAirT = tmp_u16;
				break;
			case cand_PressT_ID:	//Intake/Exhaust Conditions 1
				cand_Eng_DPFI_P = candata[i].data[0];
				cand_Eng_InM1_T = candata[i].data[2];
				cand_Eng_AirIn_P = candata[i].data[3];
				break;
			case cand_YMRDatalog10_ID:
				tmp_u16 = candata[i].data[1];
				tmp_u16 = (tmp_u16 << 8) | candata[i].data[0];
				cand_ActExM_P = tmp_u16;
				tmp_u16 = candata[i].data[3];
				tmp_u16 = (tmp_u16 << 8) | candata[i].data[2];
				cand_ActInM_P = tmp_u16;
				cand_TargetIn_P = candata[i].data[4];
				cand_TargetInthr_rate = candata[i].data[5];
				break;
			case cand_DigtStaInOut_ID:	//State of digital In/Out
				cand_LOP_SW_Port = (candata[i].data[3] & 0x10) >> 4;	//byte3.bit4
				break;
			case cand_ComPress_ID:	//Engine Injector Metering Rail 1 Pressure共轨压力
				tmp_u16 = candata[i].data[3];
				tmp_u16 = (tmp_u16 << 8) | candata[i].data[2];
				cand_CommonRail_P = tmp_u16;
				break;
			case cand_YMRDatalog1_ID:	//YMR Dataloging 1
				cand_EGRValTar_pos = candata[i].data[0];	//EGR Valve Target PositionEGR阀目标位置
				cand_EGRVaTarPos_MAP = candata[i].data[1];	//EGR Valve Target Position(Calculated value by MAP)EGR阀目标位置(MAP计算值)
				cand_Ex_gas = candata[i].data[2];	//Exhaust gas lambda废气λ
				cand_EGRVaAct_pos = candata[i].data[3];	//EGR Valve Actual PositionEGR阀实际位置
				tmp_u16 = candata[i].data[5];
				tmp_u16 = (tmp_u16 << 8) | candata[i].data[4];
				cand_Tar_NOx = tmp_u16;	//Target NOx (Final value)目标NOx(最终值)
				break;
			case cand_RPM_REC_ID:	//ECM发动机转速
				tmp_u16 = candata[i].data[4];
				tmp_u16 = (tmp_u16 << 8) | candata[i].data[3];
				cand_EngineSpeed = tmp_u16;
				cand_EngStartMod = candata[i].data[6] >> 4;	//Engine Starter Mode发动机起动器模式
				break;
			case cand_FuelRate_ID:	//Fuel tate
				tmp_u16 = candata[i].data[1];
				tmp_u16 = (tmp_u16 << 8) | candata[i].data[0];
				cand_FuelRate = tmp_u16;
				cand_Throttle_positon = candata[i].data[6];	//Throttle_positon节流正电子
				break;
			case cand_InGas2T_ID:	//Aftertreatment 1 Intake Gas 2
				tmp_u16 = candata[i].data[1];
				tmp_u16 = (tmp_u16 << 8) | candata[i].data[0];
				cand_After1ExGas_T1 = tmp_u16;
				tmp_u16 = candata[i].data[3];
				tmp_u16 = (tmp_u16 << 8) | candata[i].data[2];
				cand_After1DPfiltInGas_T = tmp_u16;
				break;
			case cand_EngLoad_ID:	//Engine load
				cand_Engload_ratio = candata[i].data[1];
				cand_Englimitload_ratio = candata[i].data[2];
				tmp_u16 = candata[i].data[4];
				tmp_u16 = (tmp_u16 << 8) | candata[i].data[3];
				cand_EngDesOpera_speed = tmp_u16;
				cand_AccelerationDet_status = candata[i].data[5] >> 6;
				cand_EngGloag_ratio = candata[i].data[6];
				break;
			case cand_YMRDatalog9_ID:	//YMR Dataloging 9
				tmp_u16 = candata[i].data[1];
				tmp_u16 = (tmp_u16 << 8) | candata[i].data[0];
				cand_TarEng_speed = tmp_u16;
				tmp_u16 = candata[i].data[3];
				tmp_u16 = (tmp_u16 << 8) | candata[i].data[2];
				cand_TarInjection_Amount = tmp_u16;
				tmp_u16 = candata[i].data[5];
				tmp_u16 = (tmp_u16 << 8) | candata[i].data[4];
				cand_BasicInjection_Timing = tmp_u16;
				cand_LoadDetection_status = candata[i].data[6] >> 4;
				break;
			case cand_FuelConsump_ID://
				cand_totalfuel_used = (candata[i].data[7]  << 24) | (candata[i].data[6]  << 16) | (candata[i].data[5]  << 8) |candata[i].data[4];
				break;
			default:
				break;
			}
			CanB_Data_In(&candata[i], canx);
		}
		break;
	case 1:				//can1(canC)
		for (i = 0; i < nframe; i++) {
			CanC_Data_In(&candata[i], canx);
			if(candata[i].can_id == cand_Hour_Meter){

			}
		}
		break;
	case 2:				//can2(canA)
		for (i = 0; i < nframe; i++) {
			CanA_Data_In(&candata[i], canx);
		}
		break;
	case 7:
		CanB_Data_In(candata, canx);
		break;
	default:
		break;
	}
	return 0;
}
u8 CmdID;
u16 PrmID;
int SetData_rxq(u8 *da) {
	PrmID = ((da[0] & 0x01) << 8) | da[1];
	if (PrmID >= 31 && PrmID <= 367 && PrmID_rxq_fun[PrmID] != NULL) {
		// 发送Ack
		printf("ACK\n");
		da[0] = (0x40 | (da[0] & 0x01)) | (CmdID << 1);
		PrmID_rxq_fun[PrmID](da + 2); // 装载相应的数据到内存
	} else {
		// 发送Nack，发送接收到的数据
		printf("NACK\n");
		da[0] = (0x00 | (da[0] & 0x01)) | (CmdID << 1);
	}
	return 0;
}

int GetData_rxq(u8 *da) {
	PrmID = ((da[0] & 0x01) << 8) | da[1];
	if (PrmID > 0 && PrmID < 31 && PrmID_rxq_fun[PrmID] != NULL) {
		// 发送Ack
		printf("ACK\n");
		da[0] = (0x40 | (da[0] & 0x01)) | (CmdID << 1);
		PrmID_rxq_fun[PrmID](da + 2); // 填充相应的数据到candata
	} else {
		// 发送Nack，发送接收到的数据
		printf("NACK\n");
		da[0] = (0x00 | (da[0] & 0x01)) | (CmdID << 1);
	}
	return 0;
}

/*************************************************
 name:candS_RecvHandle
 func  :can bus A recv data handle
 *************************************************/
int candA_RecvHandle(struct can_frame* candata) {
	CmdID = 0;
	PrmID = 0;
	u8 tmp_u8;
	memset(&canA_SendBuf, 0, sizeof(canA_SendBuf));
	canA_SendBuf.can_id = CANID_CTRLA_SEND;
	canA_SendBuf.can_dlc = 8;
	CmdID = (candata->data[0] >> 4) & 0x0F;
//	printf("CmdID:%d\n", CmdID);
	switch (CmdID)
	{
	case 1:    //Set data
//		printf("Set data:\n");
		SetData_rxq(candata->data);
		break;
	case 2:    //Get data
//		printf("Get data:\n");
		GetData_rxq(candata->data);
		break;
	case 4:    //Set_Fault
//		printf("Set Fault:\n");
		candata->data[0] = (0x40 | (candata->data[0] & 0x01)) | (CmdID << 1);
		break;
	case 5:    //Reset
//		printf("Reset:\n");
		candata->data[0] = (0x40 | (candata->data[0] & 0x01)) | (CmdID << 1);
		tmp_u8 = candata->data[1];
		cand_command_reset = tmp_u8;//can command reset
		switch (tmp_u8)
		{
		case 0:    //FaultReset
			break;
		case 1:    //HistoryReset
			break;
		case 2:    //RentalReset
			break;
		case 3:    //FieldReset
			break;
		case 4:    //AllReset
			System.IsReSet = 1;
			break;
		default:    //NACK
			candata->data[0] = (0x00 | (candata->data[0] & 0x01)) | (CmdID << 1);
			break;
		}
		break;
	case 14:    //Restart
//		printf("Restart:\n");
		candata->data[0] = (0x40 | (candata->data[0] & 0x01)) | (CmdID << 1);
		break;
	default:
		candata->data[0] = (0x00 | (candata->data[0] & 0x01)) | (CmdID << 1);
		break;
	}
	memcpy(&canA_SendBuf.data, &candata->data, 8);
//	printfHexData(canA_SendBuf.data, 8);
//	can_write(CANA_fd, &canA_SendBuf);
	return 0;
}

/****************************************************
 name:CanAData_sock
 func : canA (can2) data Recv and send handle
 ***************************************************/
void *CanData_sockA(void *argv) {
	printf("############## %s start ##############\n", __FUNCTION__);
	int n = 0;
	can_PthParamA.sta = 1;
	can_PthParamA.flag = true;
	while (can_PthParamA.flag) {
		can_PthParamA.sta = 1;
		memset(&canA_RecvBuf, 0, sizeof(canA_RecvBuf));
		n = CAN_Read(CANA_fd, &canA_RecvBuf, 1);
		if (n > 0) {
			//TODO  data handle and send communicate data
			if ((canA_RecvBuf.can_id & 0x1FFFFFFF) == CANID_CTRLA_RECV) {
				candA_RecvHandle(&canA_RecvBuf);
				can_write(CANA_fd, &canA_SendBuf);
			}
			CANDA_RecvSta = 1;
		} else {
			CANDA_RecvSta = -1;
		}
		n = 0;
//		canA_SendBuf.can_id = CANID_CTRLA_SEND;
//		canA_SendBuf.can_dlc = 8;
//		can_write(CANA_fd, &canA_SendBuf);
		usleep(10);
	}
	printf("############## %s exit ##############\n", __FUNCTION__);
	return NULL;
}
/*****************************************************
 *name :CanData_SockRecvB
 *func  :canB(can0)数据读取
 ******************************************************/
void *CanData_SockRecvB(void *argv) {
	printf("############## %s start ##############\n", __FUNCTION__);
	can_PthParamRecvB.sta = 1;
	can_PthParamRecvB.flag = true;
	while (can_PthParamRecvB.flag) {
		can_PthParamRecvB.sta = 1;
		int n = 0;
		memset(canB_RecvBuf, 0, sizeof(canB_RecvBuf));
		n = CAN_Read(CANB_fd, &canB_RecvBuf[0], 1);
		if (n > 0) {
			cand_RecvHandle(0, canB_RecvBuf, n);
			//TODO  data handle and send communicate data
//			if ((canB_RecvBuf[0].can_id & 0x1FFFFFFF) == CANID_CTRLA_RECV) {
//				candA_RecvHandle(&canB_RecvBuf[0]);
//				can_write(CANB_fd, &canA_SendBuf);
//			}
			CANDB_RecvSta = 1;
		} else {
			CANDB_RecvSta = -1;
		}
		usleep(10);
	}
	printf("############## %s exit ##############\n", __FUNCTION__);
	return NULL;
}
/*****************************************************
 *name :CanData_SockRecvC
 *func  :canC(can1)数据读取
 ******************************************************/
void *CanData_SockRecvC(void *argv) {
	printf("############## %s start ##############\n", __FUNCTION__);
	can_PthParamRecvC.sta = 1;
	can_PthParamRecvC.flag = true;
	while (can_PthParamRecvC.flag) {
		can_PthParamRecvC.sta = 1;
		int n = 0;
		memset(canC_RecvBuf, 0, sizeof(canC_RecvBuf));
		n = CAN_Read(CANC_fd, &canC_RecvBuf[0], 1);
		if (n > 0) {
			cand_RecvHandle(1, canC_RecvBuf, n);
			CANDC_RecvSta = 1;
		} else {
			CANDC_RecvSta = -1;
		}
	}
	printf("############## %s exit ##############\n", __FUNCTION__);
	return NULL;
}
/*******************************************
 *name:CanData_Collect
 *function:请求can 数据
 ***********************************************/
void *CanData_SockSendB(void *argv) {
	struct can_frame canbuf;
//	int i = 0;
	printf("############## %s start ##############\n", __FUNCTION__);
	can_PthParamSendB.sta = 1;
	can_PthParamSendB.flag = true;
//	u8 n = 0;
	int ret = 0;
	while (can_PthParamSendB.flag) {
		can_PthParamSendB.sta = 1;
		if (CANB_fd > 0) {
//			while (Can_Tx_Data_Get(&canbuf) && n++ < 20) {
			ret = can_write(CANB_fd, &canbuf);
			if (ret <= 0) {
				CANDB_SendSta = -1;
			} else {
				CANDB_SendSta = 1;
			}
			usleep(1000 * 5);
//			}
//			n = 0;
//			for (i = 0; i < CAND_PGN_SET_MAX_NUM; i++) {
//				canbuf.can_id = CAND_ID_PGN_REQUEST;
//				canbuf.can_dlc = 3;
//				memcpy(canbuf.data, &hand_pgn_set[i].pgn_id, 3);
//				if (hand_pgn_set[i].mode.req == 1) { //是否请求
//					if (hand_pgn_set[i].pgn_id.can == 0) { //来源can0
//						can_write(CAN0_fd, &canbuf);
//						if (ret <= 0) {
//							CAND0_SendSta = -1;
//						} else {
//							CAND0_SendSta = 1;
//						}
//						usleep(1000 * 5);
//					}
//				}
//			}
		}
		usleep(100);
	}
	printf("############## %s exit ##############\n", __FUNCTION__);
	return NULL;
}

void *CanData_SockSendC(void *argv) {
	struct can_frame canbuf;
	int ret = 0;
	printf("############## %s start ##############\n", __FUNCTION__);
	can_PthParamSendC.sta = 1;
	can_PthParamSendC.flag = true;
	while (can_PthParamSendC.flag) {
		can_PthParamSendC.sta = 1;
		if (CANC_fd > 0) {
			//TODO copy data canB need to write to canbuf
			ret = can_write(CANC_fd, &canbuf);
			if (ret <= 0) {
				CANDC_SendSta = -1;
			} else {
				CANDC_SendSta = 1;
			}
			usleep(1000 * 5);
		}
		usleep(1000);
	}
	printf("############## %s exit ##############\n", __FUNCTION__);
	return NULL;
}

/*
 经纬度信息21字节格式
 */
int gps_format_jwd_mem(u8 *mem) {
	u32 tmp_u32;
	u8 tmp_du;
	u32 tmp_fen;
	u16 tmp_u16;
	int index = 0;

	memset(mem, 0, 21); // ??? ????12??
	if (System.GPS.Position.Valid == 1) {   // if GPS is valid
		tmp_u32 = GPSbuf.latitude;
		tmp_du = tmp_u32 / 600000;
		tmp_fen = tmp_u32 - tmp_du * 600000;
		mem[index++] = tmp_du;
		mem[index++] = tmp_fen >> 16;
		if (GPSbuf.NS) {
			mem[index - 1] |= 0x80;
		}
		mem[index - 1] |= 0x40;
		mem[index++] = tmp_fen >> 8;
		mem[index++] = tmp_fen;

		tmp_u32 = GPSbuf.longitude;
		tmp_du = tmp_u32 / 600000;
		tmp_fen = tmp_u32 - tmp_du * 600000;
		mem[index++] = tmp_du;
		mem[index++] = tmp_fen >> 16;
		if (GPSbuf.NS) {
			mem[index - 1] |= 0x80;
		}
		mem[index++] = tmp_fen >> 8;
		mem[index++] = tmp_fen;
//？？
		tmp_u16 = GPSbuf.height;   //??海拔指示正负去要解码确认？？
		mem[index++] = tmp_u16 >> 8;
		mem[index++] = tmp_u16;

		tmp_u16 = GPSbuf.direction;
		mem[index++] = tmp_u16 >> 1;
		tmp_u16 = GPSbuf.speed | (tmp_u16 << 15);
		mem[index++] = tmp_u16 >> 8;
		mem[index++] = tmp_u16;

		tmp_u16 = GPSbuf.starnum;
		tmp_u16 = (tmp_u16 << 10) | GPSbuf.hdop;
		mem[index++] = tmp_u16 >> 8;
		mem[index++] = tmp_u16;

		mem[index++] = GPSbuf.status_qx;

		memcpy(mem + index, GPSbuf.utch, 6);
		index += 6;
	}

	return (index);
}
