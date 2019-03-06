/*********************
 *name:candata.h
 *auth:guozhiyue
 *date:2018.12.20
 *********************/
#ifndef __CANDATA_H_
#define  __CANDATA_H_

#include <stdint.h>
#include <time.h>

#include "general.h"

extern u16 revolution_speed;

/**********************CAN queue define*****************/
/*linux kernel*/
#define CAND_TX_BUF_NUM				16
#define CAND_RX_BUF_NUM				16	//C_CAN driver in linux define C_CAN_MSG_OBJ_RX_NUM    is  16

typedef struct {
	uint32_t TIME:31; // 复位开始的时间 ms
	uint32_t ide:1;   //0-标准帧 1-扩展帧
	uint32_t ID :29;
	uint32_t SOURCE :3;
	uint8_t DATA[8];
} CanMsg;
#define   CANA_RX_MAX     1000           //CAN最大接收缓冲区
#define   CANB_RX_MAX     1000           //CAN最大接收缓冲区
#define   CANC_RX_MAX     500             //CAN最大接收缓冲区

extern u16 CANA_RX_Save_Index;       //CANA接收队列保存指针
extern u16 CANA_RX_Read_Index;      //CANA接收队列读取指针
extern u16 CANB_RX_Save_Index;       //CANB接收队列保存指针
extern u16 CANB_RX_Read_Index;      //CANB接收队列读取指针
extern u16 CANC_RX_Save_Index;       //CANC接收队列保存指针
extern u16 CANC_RX_Read_Index;      //CANC接收队列读取指针
extern CanMsg CANA_RX_Data[CANA_RX_MAX];    //CANA接收队列大小
extern CanMsg CANB_RX_Data[CANB_RX_MAX];    //CANB接收队列大小
extern CanMsg CANC_RX_Data[CANC_RX_MAX];    //CANC接收队列大小

//can A\B\C recv and send return result flag
extern int CANDA_RecvSta, CANDB_RecvSta, CANDC_RecvSta, CANDA_SendSta, CANDB_SendSta, CANDC_SendSta;
/*****************************ECM数据*******************************/
extern u8  cand_CoolantT;//冷却液温度
extern u8  cand_FuelT;//燃料温度
extern u8  cand_BarPress;//大气压(Barometric press)
extern u16 cand_AmbAirT;//吸气温度(Ambient air Temp)
extern u8 cand_Eng_DPFI_P; //Engine Diesel Particulate Filter Inlet Pressurey引擎柴油微粒过滤器入口压力
extern u8 cand_Eng_InM1_T; //Engine Intake Manifold 1 Temperature发动机进气歧管1温度
extern u8 cand_Eng_AirIn_P; //Engine Air Inlet Pressure发动机进气压力
extern u16 cand_ActExM_P;//Actual Exhaust Manifold Pressure实际排气歧管压力
extern u16 cand_ActInM_P;//Actual Intake Manifold Pressure实际进气歧管压力
extern u8 cand_TargetIn_P;//Target Intake Pressure目标进气压力
extern u8 cand_TargetInthr_rate;//Target Intake throttle rate目标进气门节流率
extern u8 cand_LOP_SW_Port;//LOP SW Port
extern u16  cand_CommonRail_P;//Engine Injector Metering Rail 1 Pressure共轨压力
extern u8  cand_EGRValTar_pos;//EGR Valve Target PositionEGR阀目标位置
extern u8 cand_EGRVaTarPos_MAP;//EGR Valve Target Position(Calculated value by MAP)EGR阀目标位置(MAP计算值)
extern u8 cand_Ex_gas;//Exhaust gas lambda废气λ
extern u8 cand_EGRVaAct_pos;//EGR Valve Actual PositionEGR阀实际位置
extern u16 cand_Tar_NOx;//Target NOx (Final value)目标NOx(最终值)
extern u16 cand_EngineSpeed;//发动机转速
extern u8 cand_EngStartMod;//Engine Starter Mode发动机起动器模式
extern u16 cand_FuelRate;//Fuel rate燃料消耗率
extern u8 cand_Throttle_positon;//Throuttle positon节流正电子
extern u16 cand_After1ExGas_T1;//Aftertreatment 1 Exhaust Gas Temperature 1后处理废气温度
extern u16 cand_After1DPfiltInGas_T ;//Aftertreatment 1 Diesel Particulate Filter Intake Gas Temperature后处理1柴油机微粒过滤器进气温度
extern u8 cand_Engload_ratio;//Engine load ratio引擎负载电压
extern u8 cand_Englimitload_ratio;//Engine limited load ratio发动机限载比
extern u16 cand_EngDesOpera_speed;//Engine's Desired Operating Speed发动机所需的运行速度
extern u8 cand_AccelerationDet_status;//Acceleration Detection Status 1加速度检测状态1\2
extern u8 cand_EngGloag_ratio;//Engine gross load ratio(Smoke limit)发动机总负载比(限烟)
extern u16 cand_TarEng_speed;//Target Engine Speed目标发动机转速
extern u16 cand_TarInjection_Amount;//Target Injection Amount目标注入量
extern u16 cand_BasicInjection_Timing;//Basic Injection Timing基本喷油正时
//extern u8 cand_AccelerationDet_status;//Acceleration Detection Status 1加速度检测状态1\2
extern u8 cand_LoadDetection_status;//Load Detection Status 1\2负载检测状态1\2
extern u32 cand_totalfuel_used;// total fuel used

extern u16 cand_BatteryVoltage;//电源电压

/***************************** ID **********************************/
#define cand_CoolFuelT_ID          ((unsigned long)((0x18FEEE00<<3)))//发动机冷却液、燃料温度接收ID
#define cand_Air_PT_ID                  ((unsigned long)((0x18FEF500<<3)))//大气压力、吸气温度接收接收ID
#define cand_PressT_ID                 ((unsigned long)((0x18FEF600<<3)))//(Intake/Exhaust Conditions 1)增压后进气温度、增压后进气压力接收ID
#define cand_YMRDatalog10_ID ((unsigned long)((0x18FF0000<<3)))//YMR Dataloging 10
#define cand_DigtStaInOut_ID    ((unsigned long)((0x18FF1100<<3))) //State of digital In/Out
#define cand_ComPress_ID           ((unsigned long)((0x18FEDB00<<3)))//Engine Injector Metering Rail 1 Pressure共轨压力接收ID
#define cand_YMRDatalog1_ID    ((unsigned long)((0x18FF2800<<3)))//YMR Dataloging 1
#define cand_RPM_REC_ID            ((unsigned long)((0x0CF00400<<3)))//发动机转速接收ID
#define cand_FuelRate_ID              ((unsigned long)((0x18FEF200<<3)))//Fuel Economy (Liquid)Fuel rate
#define cand_InGas2T_ID               ((unsigned long)((0x18FDB200<<3)))//Aftertreatment 1 Intake Gas 2
#define cand_EngLoad_ID              ((unsigned long)((0x0CFF1700<<3)))//Engine load发动机负载
#define cand_YMRDatalog9_ID     ((unsigned long)((0x18FF1900<<3)))//YMR Dataloging 9
#define cand_FuelConsump_ID    ((unsigned long)((0x18FEE900<<3)))//Fuel Consumption (Liquid)


#define cand_Hour_Meter               ((unsigned long)((0x1CFF1128)))//Hour Meter
/************************CAN data set********************/
#define CANID_CTRLA_RECV  ((u32)0x18FF45E4) //CAN busA Recv ID
#define CANID_CTRLA_SEND ((u32)0x18FF46EC) //CAN busA Send ID

extern u8 cand_command_reset;//can reset value

typedef enum {
	// GetData PrmID
	PrmID_Ver = 1,
	PrmID_CtlrTPartsNum_1,
	PrmID_CtlrTPartsNum_2,
	PrmID_CtlrT,
	PrmID_GpsPos,
	PrmID_RtcTim,
	PrmID_CtlrTCfg,
	PrmID_SerialNumRequest,
	PrmID_SerialNumRequestAgain,
	PrmID_ImmobiPass,
	PrmID_MainteTimeFromWeb_1,
	PrmID_MainteTimeFromWeb_2,
	PrmID_MainteTimeFromWeb_3,
	PrmID_MainteTimeFromWeb_4,
	PrmID_MainteTimeFromWeb_5,
	PrmID_MainteIntervalFromWeb_1,
	PrmID_MainteIntervalFromWeb_2,
	PrmID_MainteIntervalFromWeb_3,
	PrmID_MainteIntervalFromWeb_4,
	PrmID_MainteIntervalFromWeb_5,
	PrmID_ParameterConfig,
	PrmID_EngineDeviceTestRequest,
	PrmID_ImmobiPassAnswer,
	// SetData PrmID(31~367)
	PrmID_McnInfo_1 = 31,
	PrmID_McnInfo_2,
	PrmID_MainCtlrPartsNum_1,
	PrmID_MainCtlrPartsNum_2,
	PrmID_MainCtlrSerialNum_1,
	PrmID_MainCtlrSerialNum_2,
	PrmID_SubCtlrFirmwareVer,
	PrmID_SubCtlrSerialNum_1,
	PrmID_SubCtlrSerialNum_2,
	PrmID_MonitorNum,
	PrmID_MonitorSerialNum,
//	PrmID_MachineInfoReserve_1,
//	PrmID_MachineInfoReserve_1,
//	PrmID_MachineInfoReserve_1,
//	PrmID_MachineInfoReserve_1,
//	PrmID_MachineInfoReserve_1,
//	PrmID_MachineInfoReserve_1,
//	PrmID_MachineInfoReserve_1,
//	PrmID_MachineInfoReserve_1,
	PrmID_BeginOfHistory = 52,
	PrmID_MainCtlrSts,
	PrmID_MainteTime_1,
	PrmID_MainteTime_2,
	PrmID_MainteTime_3,
	PrmID_MainteTime_4,
	PrmID_MainteTime_5,
	PrmID_Work_1,
	PrmID_Work_2,
	PrmID_UpperWork_1,
	PrmID_UpperWork_2,
	PrmID_UpperWork_3,
	PrmID_UpperWork_4,
	PrmID_UpperWork_5,
	PrmID_UpperWork_6,
	PrmID_TravelWork_1,
	PrmID_TravelWork_2,
	PrmID_TravelWork_3,
	PrmID_WorkMode_1,
	PrmID_WorkMode_2,
	PrmID_WorkMode_3,
	PrmID_WorkMode_4,
	PrmID_WorkMode_5,
	PrmID_Breaker,
	PrmID_Crusher,
	PrmID_OptionLine_1,
	PrmID_OptionLine_2,
	PrmID_QuickCoupler,
	PrmID_PumpBackup_1,
	PrmID_PumpBackup_2,
	PrmID_OverheatPowerReduction_1,
	PrmID_OverheatPowerReduction_2,
	PrmID_OverheatPowerReduction_3,
	PrmID_Solenoid_1,
	PrmID_Solenoid_2,
	PrmID_Solenoid_3,
	PrmID_Solenoid_4,
	PrmID_Solenoid_5,
	PrmID_Solenoid_6,
	PrmID_Solenoid_7,
	PrmID_Solenoid_8,
	PrmID_Solenoid_9,
	PrmID_Solenoid_10,
	PrmID_ATSReGen_1,
	PrmID_ATSReGen_2,
	PrmID_ATSReGen_3,
	PrmID_ATSReGen_4,
	PrmID_ATSReGen_5,
	PrmID_ATSReGen_6,
	PrmID_ATSReGen_7,
	PrmID_ATSReGen_8,
	PrmID_ATSReGen_9,
	PrmID_ATSReGen_10,
	PrmID_ATSReGen_11,
	PrmID_HotShutdown,
	PrmID_AirConditioner_1,
	PrmID_AirConditioner_2,
	PrmID_AirConditioner_3,
	PrmID_AirConditioner_4,
	PrmID_AirConditioner_5,
	PrmID_AirConditioner_6,
	PrmID_AirConditioner_7,
	PrmID_AirConditioner_8,
	PrmID_KeyOn,
	PrmID_EngineOn,
	PrmID_AutoIdle,
	PrmID_IdleStop,
	PrmID_WorkOthers_1,
	PrmID_WorkOthers_2,
	PrmID_WorkOthers_3,
	PrmID_Camera_1,
	PrmID_Camera_2,
	PrmID_Wiper_1,
	PrmID_Wiper_2,
	PrmID_Window,
	PrmID_Door,
	PrmID_WorkHistoryReserve_1,
	PrmID_WorkHistoryReserve_2,
	PrmID_WorkHistoryReserve_3,
	PrmID_WorkHistoryReserve_4,
	PrmID_WorkHistoryReserve_5,
	PrmID_WorkHistoryReserve_6,
	PrmID_WorkHistoryReserve_7,
	PrmID_WorkHistoryReserve_8,
	PrmID_WorkHistoryReserve_9,
	PrmID_WorkHistoryReserve_10,
	PrmID_WorkHistoryReserve_11,
	PrmID_MaxMin_1,
	PrmID_MaxMin_2,
	PrmID_MaxMin_3,
	PrmID_MaxMin_4,
	PrmID_MaxMin_5,
	PrmID_MaxMin_6,
	PrmID_MaxMin_7,
	PrmID_MaxMin_8,
	PrmID_MaxMin_9,
	PrmID_MaxMin_10,
	PrmID_MaxMin_11,
	PrmID_MaxMin_12,
	PrmID_MaxMin_13,
	PrmID_MaxMin_14,
	PrmID_P1PressDst_1,
	PrmID_P1PressDst_2,
	PrmID_P1PressDst_3,
	PrmID_P1PressDst_4,
	PrmID_P2PressDst_1,
	PrmID_P2PressDst_2,
	PrmID_P2PressDst_3,
	PrmID_P2PressDst_4,
	PrmID_N1PressDst_1,
	PrmID_N1PressDst_2,
	PrmID_N1PressDst_3,
	PrmID_N1PressDst_4,
	PrmID_N2PressDst_1,
	PrmID_N2PressDst_2,
	PrmID_N2PressDst_3,
	PrmID_N2PressDst_4,
	PrmID_P1P2PressDst_1,
	PrmID_P1P2PressDst_2,
	PrmID_P1P2PressDst_3,
	PrmID_P1P2PressDst_4,
	PrmID_TravelP1PressDst_1,
	PrmID_TravelP1PressDst_2,
	PrmID_TravelP1PressDst_3,
	PrmID_TravelP1PressDst_4,
	PrmID_TravelP2PressDst_1,
	PrmID_TravelP2PressDst_2,
	PrmID_TravelP2PressDst_3,
	PrmID_TravelP2PressDst_4,
	PrmID_ArmCylBottomPressDst_1,
	PrmID_ArmCylBottomPressDst_2,
	PrmID_ArmCylBottomPressDst_3,
	PrmID_ArmCylBottomPressDst_4,
	PrmID_ArmCylRodPressDst_1,
	PrmID_ArmCylRodPressDst_2,
	PrmID_ArmCylRodPressDst_3,
	PrmID_ArmCylRodPressDst_4,
	PrmID_BoomCylBottomPressDst_1,
	PrmID_BoomCylBottomPressDst_2,
	PrmID_BoomCylBottomPressDst_3,
	PrmID_BoomCylBottomPressDst_4,
	PrmID_BoomCylRodPressDst_1,
	PrmID_BoomCylRodPressDst_2,
	PrmID_BoomCylRodPressDst_3,
	PrmID_BoomCylRodPressDst_4,
	PrmID_HydOilTempDst_1,
	PrmID_HydOilTempDst_2,
	PrmID_HydOilTempDst_3,
	PrmID_HydOilTempDst_4,
	PrmID_HydOilFilterPressDst_1,
	PrmID_HydOilFilterPressDst_2,
	PrmID_HydOilFilterPressDst_3,
	PrmID_HydOilFilterPressDst_4,
	PrmID_TravelOprTimeDst_1,
	PrmID_TravelOprTimeDst_2,
	PrmID_TravelOprTimeDst_3,
	PrmID_TravelOprTimeMax_1,
	PrmID_TravelOprTimeMax_2,
	PrmID_EngineActualSpeedDst_1,
	PrmID_EngineActualSpeedDst_2,
	PrmID_EngineActualSpeedDst_3,
	PrmID_EngineActualSpeedDst_4,
	PrmID_CoolantTempDst_1,
	PrmID_CoolantTempDst_2,
	PrmID_CoolantTempDst_3,
	PrmID_CoolantTempDst_4,
	PrmID_CoolDownTimeDst_1,
	PrmID_CoolDownTimeDst_2,
	PrmID_FuelTempDst_1,
	PrmID_FuelTempDst_2,
	PrmID_FuelTempDst_3,
	PrmID_FuelTempDst_4,
	PrmID_InletAirTempDst_1,
	PrmID_InletAirTempDst_2,
	PrmID_InletAirTempDst_3,
	PrmID_InletAirTempDst_4,
	PrmID_BoostTempDst_1,
	PrmID_BoostTempDst_2,
	PrmID_BoostTempDst_3,
	PrmID_BoostTempDst_4,
	PrmID_BaroPressDst_1,
	PrmID_BaroPressDst_2,
	PrmID_BaroPressDst_3,
	PrmID_BaroPressDst_4,
	PrmID_EngineOilPressDst_1,
	PrmID_EngineOilPressDst_2,
	PrmID_EngineOilPressDst_3,
	PrmID_EngineOilPressDst_4,
	PrmID_EngineOilPressRiseTimeDst_1,
	PrmID_EngineOilPressRiseTimeDst_2,
	PrmID_EngineOilPressRiseTimeDst_3,
	PrmID_BoostPressDst_1,
	PrmID_BoostPressDst_2,
	PrmID_BoostPressDst_3,
	PrmID_BoostPressDst_4,
	PrmID_EngineLoadRatioDst_1,
	PrmID_EngineLoadRatioDst_2,
	PrmID_EngineLoadRatioDst_3,
	PrmID_EngineLoadRatioDst_4,
	PrmID_EngineLoadRatioSPDst_1,
	PrmID_EngineLoadRatioSPDst_2,
	PrmID_EngineLoadRatioSPDst_3,
	PrmID_EngineLoadRatioSPDst_4,
	PrmID_EngineLoadRatioHDst_1,
	PrmID_EngineLoadRatioHDst_2,
	PrmID_EngineLoadRatioHDst_3,
	PrmID_EngineLoadRatioHDst_4,
	PrmID_EngineLoadRatioADst_1,
	PrmID_EngineLoadRatioADst_2,
	PrmID_EngineLoadRatioADst_3,
	PrmID_EngineLoadRatioADst_4,
	PrmID_SupplyPumpPressDst_1,
	PrmID_SupplyPumpPressDst_2,
	PrmID_SupplyPumpPressDst_3,
	PrmID_SupplyPumpPressDst_4,
	PrmID_DOCInTempDst_1,
	PrmID_DOCInTempDst_2,
	PrmID_DOCInTempDst_3,
	PrmID_DOCInTempDst_4,
	PrmID_DOCOutTempDst_1,
	PrmID_DOCOutTempDst_2,
	PrmID_DOCOutTempDst_3,
	PrmID_DOCOutTempDst_4,
	PrmID_EGR_1InTempDst_1,
	PrmID_EGR_1InTempDst_2,
	PrmID_EGR_1InTempDst_3,
	PrmID_EGR_1InTempDst_4,
	PrmID_EGR_1OutTempDst_1,
	PrmID_EGR_1OutTempDst_2,
	PrmID_EGR_1OutTempDst_3,
	PrmID_EGR_1OutTempDst_4,
	PrmID_EGR_2InTempDst_1,
	PrmID_EGR_2InTempDst_2,
	PrmID_EGR_2InTempDst_3,
	PrmID_EGR_2InTempDst_4,
	PrmID_EGR_2OutTempDst_1,
	PrmID_EGR_2OutTempDst_2,
	PrmID_EGR_2OutTempDst_3,
	PrmID_EGR_2OutTempDst_4,
	PrmID_InterCoolerTempDst_1,
	PrmID_InterCoolerTempDst_2,
	PrmID_InterCoolerTempDst_3,
	PrmID_InterCoolerTempDst_4,
	PrmID_ManifoldTempDst_1,
	PrmID_ManifoldTempDst_2,
	PrmID_ManifoldTempDst_3,
	PrmID_ManifoldTempDst_4,
	PrmID_CommonRailPressDst_1,
	PrmID_CommonRailPressDst_2,
	PrmID_CommonRailPressDst_3,
	PrmID_CommonRailPressDst_4,
	PrmID_CommonRailDiffPressDst_1,
	PrmID_CommonRailDiffPressDst_2,
	PrmID_CommonRailDiffPressDst_3,
	PrmID_CommonRailDiffPressDst_4,
	PrmID_DPDDiffPressDst_1,
	PrmID_DPDDiffPressDst_2,
	PrmID_DPDDiffPressDst_3,
	PrmID_DPDDiffPressDst_4,
	PrmID_AirconditionerBlowerDst_1,
	PrmID_AirconditionerBlowerDst_2,
	PrmID_AirconditionerBlowerDst_3,
	PrmID_AirconditionerTargetTempDst_1,
	PrmID_AirconditionerTargetTempDst_2,
	PrmID_AirconditionerTargetTempDst_3,
	PrmID_AirconditionerTargetTempDst_4,
	PrmID_DistributionReserve_1,
	PrmID_DistributionReserve_2,
	PrmID_DistributionReserve_3,
	PrmID_DistributionReserve_4,
	PrmID_DistributionReserve_5,
	PrmID_DistributionReserve_6,
	PrmID_DistributionReserve_7,
	PrmID_DistributionReserve_8,
	PrmID_DistributionReserve_9,
	PrmID_DistributionReserve_10,
	PrmID_DistributionReserve_11,
	PrmID_DistributionReserve_12,
	PrmID_DistributionReserve_13,
	PrmID_DistributionReserve_14,
	PrmID_DistributionReserve_15,
	PrmID_DistributionReserve_16,
	PrmID_DistributionReserve_17,
	PrmID_DistributionReserve_18,
	PrmID_DistributionReserve_19,
	PrmID_DistributionReserve_20,
	PrmID_DistributionReserve_21,
	PrmID_DistributionReserve_22,
	PrmID_DistributionReserve_23,
	PrmID_DistributionReserve_24,
	PrmID_DistributionReserve_25,
	PrmID_DistributionReserve_26,
	PrmID_DistributionReserve_27,
	PrmID_DistributionReserve_28,
	PrmID_DistributionReserve_29,
	PrmID_DistributionReserve_30,
	PrmID_DistributionReserve_31,
	PrmID_DistributionReserve_32,
	PrmID_DistributionReserve_33,
	PrmID_DistributionReserve_34,
	PrmID_EndOfHistory,
	PrmID_MachineParameter_1,
	PrmID_MachineParameter_2,
	PrmID_MachineParameter_3,
	PrmID_MachineParameter_4,
	PrmID_MachineParameter_5,
	PrmID_MachineParameter_6,
	PrmID_MachineParameterReserve_1,
	PrmID_MachineParameterReserve_2,
	PrmID_MachineParameterReserve_3,
	PrmID_MachineParameterReserve_4,
	PrmID_Maintenance,
	PrmID_MachineTouch,
	PrmID_ATSReGenStartNotified,
	PrmID_ATSReGenFinishNotified,
	PrmID_EngineDeviceTestResult,
	PrmID_ConfSerialNum,
	PrmID_InquireAboutImmobiPass,
	PrmID_END
} TDF_ENUM_PRMID;
extern TDF_ENUM_PRMID TDF_PrmID;

typedef int (*PrmID_RXQ_FUN)(u8 *);
PrmID_RXQ_FUN PrmID_rxq_fun[PrmID_END];

typedef struct {
	u8 Ver[6];
	u8 CtlrTPartsNum[12];
	u8 CtlrT[6];
	u8 GpsPos[6];
	u8 RtcTim[6];
	u8 CtlrTCfg[6];
	u8 SerialNumRequest[6];
	u8 ImmobiPass[6];
	u8 MainteTimeFromWeb[30];
	u8 MainteIntervalFromWeb[30];
	u8 ParameterConfig[6];
	u8 EngineDeviceTestRequest[6];
	u8 ImmobiPassAnswer[6];
} TDF_GET_DATA;
extern TDF_GET_DATA TdfGetData;

typedef struct {
	struct {
		u8 Model :6;
		u8 Done :1;
		u8 Reserve1 :1;
		u8 Territory :6;
		u8 Reserve2 :2;
		u8 Brand :3;
		u8 AttType :4;
		u8 Reserve3 :1;
		u8 FirstOptionLine :4;
		u8 ReliefPress :2;
		u8 Reserve4 :2;
		u8 OperationType :3;
		u8 SecondOptionLine :2;
		u8 QuickCoupler :2;
		u8 Reserve5 :1;
		u8 OverloadWarning :3;
		u8 AntiInterference :2;
		u8 FreeSwing :2;
		u8 Reserve6 :1;

		u8 OnePedalTravel :2;
		u8 TravelAlarm :2;
		u8 FuelPumpAutoStop :2;
		u8 ElevatorCab :2;
		u8 LiftingMagnet :3;
		u8 ApplicationMachine :5;
		u8 Fan :3;
		u8 ICT :3;
		u8 Reserve7 :2;
		u8 BucketPositionSensor :3;
		u8 GCHC :2;
		u8 Reserve8 :3;
	} McnInfo;
	u8 MainCtltPartsNum[12];
	u8 MainCtltSerialNum[11];
	u8 SubCtlrFirmwareVer[1];
	u8 SubCtlrSerialNum[8];
	u8 MonitorNum[4];
	u8 MonitorSerialNum[6];
	struct {
		u32 HourMeter :24;
		u32 FuelLv :8;
		u8 MachineLock :2;
		u8 ManualLock :2;
		u8 BackMonitorLock :4;
		u8 Operation :1;
		u8 OperationMode :7;
	} MainCtlrSts;
	u8 MainteTimeFromWeb[30];
	u8 Work[9];
	u8 UpperWork[33];
	u8 TravelWork[18];
	u8 WorkMode[30];
	u8 Breaker[6];
	u8 Crusher[6];
	u8 OptionLine[12];
	u8 QuickCoupler[6];
	u8 PumpBackup[12];
	u8 OverheatPowerReduction[15];
	u8 Solenoid[60];
	u8 ATSReGen[66];
	u8 HotShutdown[6];
	u8 AirConditioner[48];
	u8 KeyOn[6];
	u8 EngineOn[6];
	u8 AutoIdle[6];
	u8 IdleStop[6];
	u8 WorkOthers[18];
	u8 Camera[12];
	u8 Wiper[12];
	u8 Window[6];
	u8 Door[6];
	//u8 WorkHistoryReserve[66];
	u8 MaxMin[84];
	u8 P1PressDst_1[6];
	u8 P1PressDst_2[6];
	u8 P1PressDst_3[6];
	u8 P1PressDst_4[6];
	u8 P2PressDst_1[6];
	u8 P2PressDst_2[6];
	u8 P2PressDst_3[6];
	u8 P2PressDst_4[6];
	u8 N1PressDst_1[6];
	u8 N1PressDst_2[6];
	u8 N1PressDst_3[6];
	u8 N1PressDst_4[6];
	u8 N2PressDst_1[6];
	u8 N2PressDst_2[6];
	u8 N2PressDst_3[6];
	u8 N2PressDst_4[3];
	u8 P1P2PressDst_1[6];
	u8 P1P2PressDst_2[6];
	u8 P1P2PressDst_3[6];
	u8 P1P2PressDst_4[3];
	u8 TravelP1PressDst_1[6];
	u8 TravelP1PressDst_2[6];
	u8 TravelP1PressDst_3[6];
	u8 TravelP1PressDst_4[3];
	u8 TravelP2PressDst_1[6];
	u8 TravelP2PressDst_2[6];
	u8 TravelP2PressDst_3[6];
	u8 TravelP2PressDst_4[3];
	u8 ArmCylBottomPressDst_1[6];
	u8 ArmCylBottomPressDst_2[6];
	u8 ArmCylBottomPressDst_3[6];
	u8 ArmCylBottomPressDst_4[3];
	u8 ArmCylRodPressDst_1[6];
	u8 ArmCylRodPressDst_2[6];
	u8 ArmCylRodPressDst_3[6];
	u8 ArmCylRodPressDst_4[3];
	u8 BoomCylBottomPressDst_1[6];
	u8 BoomCylBottomPressDst_2[6];
	u8 BoomCylBottomPressDst_3[6];
	u8 BoomCylBottomPressDst_4[3];
	u8 BoomCylRodPressDst_1[6];
	u8 BoomCylRodPressDst_2[6];
	u8 BoomCylRodPressDst_3[6];
	u8 BoomCylRodPressDst_4[3];
	u8 HydOilTempDst_1[6];
	u8 HydOilTempDst_2[6];
	u8 HydOilTempDst_3[6];
	u8 HydOilTempDst_4[3];
	u8 HydOilFilterPressDst_1[6];
	u8 HydOilFilterPressDst_2[6];
	u8 HydOilFilterPressDst_3[6];
	u8 HydOilFilterPressDst_4[3];
	u8 TravelOprTimeDst_1[6];
	u8 TravelOprTimeDst_2[6];
	u8 TravelOprTimeDst_3[2];
	u8 TravelOprTimeMax_1[6];
	u8 TravelOprTimeMax_2[4];
	u8 EngineActualSpeedDst_1[6];
	u8 EngineActualSpeedDst_2[6];
	u8 EngineActualSpeedDst_3[6];
	u8 EngineActualSpeedDst_4[3];
	u8 CoolantTempDst_1[6];
	u8 CoolantTempDst_2[6];
	u8 CoolantTempDst_3[6];
	u8 CoolantTempDst_4[3];
	u8 CoolDownTimeDst_1[6];
	u8 CoolDownTimeDst_2[6];
	u8 FuelTempDst_1[6];
	u8 FuelTempDst_2[6];
	u8 FuelTempDst_3[6];
	u8 FuelTempDst_4[3];
	u8 InletAirTempDst_1[6];
	u8 InletAirTempDst_2[6];
	u8 InletAirTempDst_3[6];
	u8 InletAirTempDst_4[3];
	u8 BoostTempDst_1[6];
	u8 BoostTempDst_2[6];
	u8 BoostTempDst_3[6];
	u8 BoostTempDst_4[3];
	u8 BaroPressDst_1[6];
	u8 BaroPressDst_2[6];
	u8 BaroPressDst_3[6];
	u8 BaroPressDst_4[3];
	u8 EngineOilPressDst_1[6];
	u8 EngineOilPressDst_2[6];
	u8 EngineOilPressDst_3[6];
	u8 EngineOilPressDst_4[3];
	u8 EngineOilPressRiseTimeDst_1[6];
	u8 EngineOilPressRiseTimeDst_2[6];
	u8 EngineOilPressRiseTimeDst_3[2];
	u8 BoostPressDst_1[6];
	u8 BoostPressDst_2[6];
	u8 BoostPressDst_3[6];
	u8 BoostPressDst_4[3];
	u8 EngineLoadRatioDst_1[6];
	u8 EngineLoadRatioDst_2[6];
	u8 EngineLoadRatioDst_3[6];
	u8 EngineLoadRatioDst_4[3];
	u8 EngineLoadRatioSPDst_1[6];
	u8 EngineLoadRatioSPDst_2[6];
	u8 EngineLoadRatioSPDst_3[6];
	u8 EngineLoadRatioSPDst_4[3];
	u8 EngineLoadRatioHDst_1[6];
	u8 EngineLoadRatioHDst_2[6];
	u8 EngineLoadRatioHDst_3[6];
	u8 EngineLoadRatioHDst_4[3];
	u8 EngineLoadRatioADst_1[6];
	u8 EngineLoadRatioADst_2[6];
	u8 EngineLoadRatioADst_3[6];
	u8 EngineLoadRatioADst_4[3];
	u8 SupplyPumpPressDst_1[6];
	u8 SupplyPumpPressDst_2[6];
	u8 SupplyPumpPressDst_3[6];
	u8 SupplyPumpPressDst_4[3];
	u8 DOCInTempDst_1[6];
	u8 DOCInTempDst_2[6];
	u8 DOCInTempDst_3[6];
	u8 DOCInTempDst_4[3];
	u8 DOCOutTempDst_1[6];
	u8 DOCOutTempDst_2[6];
	u8 DOCOutTempDst_3[6];
	u8 DOCOutTempDst_4[3];
	u8 EGR_1InTempDst_1[6];
	u8 EGR_1InTempDst_2[6];
	u8 EGR_1InTempDst_3[6];
	u8 EGR_1InTempDst_4[3];
	u8 EGR_1OutTempDst_1[6];
	u8 EGR_1OutTempDst_2[6];
	u8 EGR_1OutTempDst_3[6];
	u8 EGR_1OutTempDst_4[3];
	u8 EGR_2InTempDst_1[6];
	u8 EGR_2InTempDst_2[6];
	u8 EGR_2InTempDst_3[6];
	u8 EGR_2InTempDst_4[3];
	u8 EGR_2OutTempDst_1[6];
	u8 EGR_2OutTempDst_2[6];
	u8 EGR_2OutTempDst_3[6];
	u8 EGR_2OutTempDst_4[3];
	u8 InterCoolerTempDst_1[6];
	u8 InterCoolerTempDst_2[6];
	u8 InterCoolerTempDst_3[6];
	u8 InterCoolerTempDst_4[3];
	u8 ManifoldTempDst_1[6];
	u8 ManifoldTempDst_2[6];
	u8 ManifoldTempDst_3[6];
	u8 ManifoldTempDst_4[3];
	u8 CommonRailPressDst_1[6];
	u8 CommonRailPressDst_2[6];
	u8 CommonRailPressDst_3[6];
	u8 CommonRailPressDst_4[3];
	u8 CommonRailDiffPressDst_1[6];
	u8 CommonRailDiffPressDst_2[6];
	u8 CommonRailDiffPressDst_3[6];
	u8 CommonRailDiffPressDst_4[3];
	u8 DPDDiffPressDst_1[6];
	u8 DPDDiffPressDst_2[6];
	u8 DPDDiffPressDst_3[6];
	u8 DPDDiffPressDst_4[3];
	u8 AirconditionerBlowerDst_1[6];
	u8 AirconditionerBlowerDst_2[6];
	u8 AirconditionerBlowerDst_3[6];
	u8 AirconditionerTargetTempDst_1[6];
	u8 AirconditionerTargetTempDst_2[6];
	u8 AirconditionerTargetTempDst_3[6];
	u8 AirconditionerTargetTempDst_4[3];
//	u8 DistributionReserve_1[6];
//	u8 DistributionReserve_2[6];
//	u8 DistributionReserve_3[6];
//	u8 DistributionReserve_4[6];
//	u8 DistributionReserve_5[6];
//	u8 DistributionReserve_6[6];
//	u8 DistributionReserve_7[6];
//	u8 DistributionReserve_8[6];
//	u8 DistributionReserve_9[6];
//	u8 DistributionReserve_10[6];
//	u8 DistributionReserve_11[6];
//	u8 DistributionReserve_12[6];
//	u8 DistributionReserve_13[6];
//	u8 DistributionReserve_14[6];
//	u8 DistributionReserve_15[6];
//	u8 DistributionReserve_16[6];
//	u8 DistributionReserve_17[6];
//	u8 DistributionReserve_18[6];
//	u8 DistributionReserve_19[6];
//	u8 DistributionReserve_20[6];
//	u8 DistributionReserve_21[6];
//	u8 DistributionReserve_22[6];
//	u8 DistributionReserve_23[6];
//	u8 DistributionReserve_24[6];
//	u8 DistributionReserve_25[6];
//	u8 DistributionReserve_26[6];
//	u8 DistributionReserve_27[6];
//	u8 DistributionReserve_28[6];
//	u8 DistributionReserve_29[6];
//	u8 DistributionReserve_30[6];
//	u8 DistributionReserve_31[6];
//	u8 DistributionReserve_32[6];
//	u8 DistributionReserve_33[6];
//	u8 DistributionReserve_34[6];
//	u8 EndOfHistory[6]
	u8 MachineParameter_1[6];
	u8 MachineParameter_2[6];
	u8 MachineParameter_3[6];
	u8 MachineParameter_4[6];
	u8 MachineParameter_5[2];
	u8 MachineParameter_6[3];
//	u8 MachineParameterReserve_1[6];
//	u8 MachineParameterReserve_2[6];
//	u8 MachineParameterReserve_3[6];
//	u8 MachineParameterReserve_4[6];
	u8 Maintenance[6];
	u8 MachineTouch[6];
	u8 ATSReGenStartNotified;
	u8 ATSReGenFinishNotified;
	u8 EngineDeviceTestResult;
	u8 ConfSerialNum[4];
	u8 InquireAboutImmobiPass[3];



} TDF_SET_DATA;
extern TDF_SET_DATA TdfSetData;

/*********************FUNC*****************************/
extern int gps_format_jwd_mem(u8 *mem);//经纬度格式拼串

extern void *CanData_sockA(void *argv);
extern void *CanData_SockRecvB(void *argv);
extern void *CanData_SockRecvC(void *argv);
extern void *CanData_SockSendB(void *argv);
extern void *CanData_SockSendC(void *argv);
extern int CanA_Data_Get(CanMsg *msg);
extern int CanB_Data_Get(CanMsg *msg);
extern int CanC_Data_Get(CanMsg *msg);

extern void PrmID_rxq_device_init(void);
/*********************FUNC*****************************/

#endif
