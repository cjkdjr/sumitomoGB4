/*
 * canhandle.h
 *
 *  Created on: 2019年1月9日
 *      Author: tykj
 */

#ifndef CANHANDLE_H_
#define CANHANDLE_H_

#include"candata.h"

/*********************参数组定义*******************/
typedef struct bit_zone17_s
{
	unsigned current_updata_flag:1;		//本时段更新标志
	unsigned parm_type:7;		//参数版本控制
}bit_zone_comm_t;

extern u32 msg_strat_time_ms;

/*
	参数组 0x0001
*/
typedef struct sumitomo_parm_grp_0001_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	u8 UTC[6];	//GPS时间
	u8 longitude_and_latitude[12];	//经纬度

	u8 alternator_signal[2];	//发动机运行情况
	u8 engine_speed[2];
	u8	ENGON_OFF;
	u8	KEYON_OFF;

	u8 Hour_Meter[3];	//计时表
	u8 Manual_Lock;
	u8 Back_Monitor_Lock;
	u8 Fuel_Lv;

	u8 voltage_update_flag;	//电压更新标志
	u8 Alternator_voltage[2];
	u8 storage_battery[2];		//车载蓄电池信号
	u8 ENGON[4];
	u8 KEYON[4];
}sumitomo_parm_grp_0001_t;

extern sumitomo_parm_grp_0001_t sumitomo_parm_grp_0001;

/*
	参数组 0x0002 用于“设定保养剩余时间及回复信息”
*/
typedef struct sumitomo_parm_grp_0002_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	u8 MainteTimeFromWeb_1[6];
	u8 MainteTimeFromWeb_2[6];
	u8 MainteTimeFromWeb_3[6];
	u8 MainteTimeFromWeb_4[6];
	u8 MainteTimeFromWeb_5[4]; //4字节有效
}sumitomo_parm_grp_0002_t;
sumitomo_parm_grp_0002_t parm_grp_0002;
/*
	参数组 0x0003 用于“设定保养间隔时间及回复信息”
*/
typedef struct sumitomo_parm_grp_0003_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	u8 MainteTimeFromWeb_1[6];
	u8 MainteTimeFromWeb_2[6];
	u8 MainteTimeFromWeb_3[6];
	u8 MainteTimeFromWeb_4[6];
	u8 MainteTimeFromWeb_5[4];
}sumitomo_parm_grp_0003_t;
sumitomo_parm_grp_0003_t parm_grp_0003;
/*
	参数组 0x0004 用于“初期设定信息”、“回叫初期设定参数信息”、“维修通知信息”
*/
typedef struct sumitomo_parm_grp_0004_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	u8 canbus_c_1CFF1128[6]; //CAN C定期送信
	u8 canbus_b_18FEEB00[6];
	u8 MainCtlrPartsNum_1[6];
	u8 MainCtlrPartsNum_2[6];
	u8 MainCtlrSerialNum_1[6];
	u8 MainCtlrSerialNum_2[6];
	u8 SubCtlrFirmwareVer[6];
	u8 SubCtlrSerialNum_1[6];
	u8 SubCtlrSerialNum_2[6];
	u8 MonitorNum[6];
	u8 MonitorSerialNum[6];
	u8 CtlrTPartsNum_1[6];
	u8 CtlrTPartsNum_2[6];
	u8 McnInfo_1[6];
	u8 McnInfo_2[6];
	u8 longitude_and_latitude[21];	//经纬度
}sumitomo_parm_grp_0004_t;
sumitomo_parm_grp_0004_t parm_grp_0004;
/*
	参数组：0x0005设备参数请求
*/
typedef struct sumitomo_parm_grp_0005_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	u8 MachineParameter_1[6];
	u8 MachineParameter_2[6];
	u8 MachineParameter_3[6];
	u8 MachineParameter_4[6];
	u8 MachineParameter_5[6];
	u8 MachineParameter_6[6];
}sumitomo_parm_grp_0005_t;
sumitomo_parm_grp_0005_t parm_grp_0005;
/*
	参数组：0x0006，设备参数设定 结合“远程操作设置及回复信息”使用，实现远程操作-设备参数设定。
*/
typedef struct sumitomo_parm_grp_0006_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	u8 ParameterConfig[6];
	u8 response_type;
}sumitomo_parm_grp_0006_t;
sumitomo_parm_grp_0006_t parm_grp_0006;
/*
	参数组：0x0007，设备状态请求 用于“回叫指定参数组信息”
*/
typedef struct sumitomo_parm_grp_0007_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	//有些参数不知道从哪里获取 ？
}sumitomo_parm_grp_0007_t;
sumitomo_parm_grp_0007_t parm_grp_0007;
/*
	参数组：0x0008，设备重置设定 结合“远程操作设置及回复信息”使用，实现远程操作-设备重置设定。
*/
typedef struct sumitomo_parm_grp_0008_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	u8 Machine_Reset_Request;	//从 prmid->CtlrT获取
	u8 response_type;
}sumitomo_parm_grp_0008_t;
sumitomo_parm_grp_0008_t parm_grp_0008;
/*
	参数组：0x0009，发动机装置测试设定 结合“远程操作设置及回复信息”使用，实现远程操作-发动机装置测试设定。
*/
typedef struct sumitomo_parm_grp_0009_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	u8 EngineDeviceTestRequest[6];
	u8 response_type;
	//Test Result （6字节，但是协议删除了，不知道还定不定义？）
}sumitomo_parm_grp_0009_t;
sumitomo_parm_grp_0009_t  parm_grp_0009;
/*
	参数组：0x0600
*/
typedef struct sumitomo_parm_grp_0600_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	//油耗，没搜到咋整？
}sumitomo_parm_grp_0600_t;



typedef struct sumitomo_parm_grp_0800_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	//本时段达到KEYON条件（没找到）
	//水温 MIN	---从哪获得？
	//大気圧 MIN
	//燃料温度 MIN
	//吸気温度 MIN	---
	//本时段达到KEYON条件（没找到）
}sumitomo_parm_grp_0800_t;

typedef struct sumitomo_parm_grp_0700_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	u8 Work_1[6];
	u8 Work_2[6];
	u8 UpperWork_1[6];
	u8 UpperWork_2[6];
	u8 UpperWork_3[6];
	u8 UpperWork_4[6];
	u8 UpperWork_5[6];
	u8 UpperWork_6[6];
	u8 TravelWork_1[6];
	u8 TravelWork_2[6];
	u8 TravelWork_3[6];
	u8 WorkMode_1[6];
	u8 WorkMode_2[6];
	u8 WorkMode_3[6];
	u8 WorkMode_4[6];
	u8 WorkMode_5[6];
	u8 Breaker_Operation_Time[3];	//（Breaker中的一部分 3个字节）
	u8 Crusher_Operation_Time[3];
	u8 Option_Line_Operation_Time[3];	//（Crusher中的一部分，3个字节）
	u8 Option_Line_Operation_Time_2nd[3];	//（OptionLine_2中的一部分，3字节）
	u8 Overheat_Power_Reduction_Time[3];	//（OverheatPowerReduction_1中的一部分，3字节）
}sumitomo_parm_grp_0700_t;


typedef struct sumitomo_parm_grp_1301_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	u8 Work_1[6];
	u8 Work_2[6];
	u8 UpperWork_1[6];
	u8 UpperWork_2[6];
	u8 UpperWork_3[6];
	u8 UpperWork_4[6];
	u8 UpperWork_5[6];
	u8 UpperWork_6[6];
	u8 TravelWork_1[6];
	u8 TravelWork_2[6];
	u8 TravelWork_3[6];
	u8 WorkMode_1[6];
	u8 WorkMode_2[6];
	u8 WorkMode_3[6];
	u8 WorkMode_4[6];
	u8 WorkMode_5[6];
	u8 Breaker[6];
	u8 Crusher[6];
	u8 OptionLine_1[6];
	u8 OptionLine_2[6];
}sumitomo_parm_grp_1301_t;


typedef struct sumitomo_parm_grp_1302_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	u8 QuickCoupler[6];
	u8 PumpBackup_1[6];
	u8 PumpBackup_2[6];
	u8 OverheatPowerReduction_1[6];
	u8 OverheatPowerReduction_2[6];
	u8 OverheatPowerReduction_3[6];
	u8 Solenoid_1[6];
	u8 Solenoid_2[6];
	u8 Solenoid_3[6];
	u8 Solenoid_4[6];
	u8 Solenoid_5[6];
	u8 Solenoid_6[6];
	u8 Solenoid_7[6];
	u8 Solenoid_8[6];
	u8 Solenoid_9[6];
	u8 Solenoid_10[6];
	u8 ATSReGen_1[6];
	u8 ATSReGen_2[6];
	u8 ATSReGen_3[6];
	u8 ATSReGen_4[6];
	u8 ATSReGen_5[6];
	u8 ATSReGen_6[6];
	u8 ATSReGen_7[6];
	u8 ATSReGen_8[6];
	u8 ATSReGen_9[6];
	u8 ATSReGen_10[6];
	u8 ATSReGen_11[6];
}sumitomo_parm_grp_1302_t;


typedef struct sumitomo_parm_grp_1303_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	u8 HotShutdown[6];
	u8 AirConditioner_1[6];
	u8 AirConditioner_2[6];
	u8 AirConditioner_3[6];
	u8 AirConditioner_4[6];
	u8 AirConditioner_5[6];
	u8 AirConditioner_6[6];
	u8 AirConditioner_7[6];
	u8 AirConditioner_8[6];
	u8 KeyOn[6];
	u8 EngineOn[6];
	u8 AutoIdle[6];
	u8 IdleStop[6];
	u8 WorkOthers_1[6];
	u8 WorkOthers_2[6];
	u8 WorkOthers_3[6];
	u8 Camera_1[6];
	u8 Camera_2[6];
	u8 Wiper_1[6];
	u8 Wiper_2[6];
	u8 Window[6];
	u8 Door[6];
}sumitomo_parm_grp_1303_t;

/*
	参数组：0x1304
*/
typedef struct sumitomo_parm_grp_1304_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	u8 MaxMin_1[6];
	u8 MaxMin_2[6];
	u8 MaxMin_3[6];
	u8 MaxMin_4[6];
	u8 MaxMin_5[6];
	u8 MaxMin_6[6];
	u8 MaxMin_7[6];
	u8 MaxMin_8[6];
	u8 MaxMin_9[6];
	u8 MaxMin_10[6];
	u8 MaxMin_11[6];
	u8 MaxMin_12[6];
	u8 MaxMin_13[6];
	u8 MaxMin_14[6];
}sumitomo_parm_grp_1304_t;

/*
	参数组：0x0901，P1压力分布
*/
typedef struct sumitomo_parm_grp_0901_s
{
	u8 P1PressDst_1[6];
	u8 P1PressDst_2[6];
	u8 P1PressDst_3[6];
	u8 P1PressDst_4[6];
}sumitomo_parm_grp_0901_t;

/*
	参数组：0x0902，P2压力分布
*/
typedef struct sumitomo_parm_grp_0902_s
{
	u8 P2PressDst_1[6];
	u8 P2PressDst_2[6];
	u8 P2PressDst_3[6];
	u8 P2PressDst_4[6];
}sumitomo_parm_grp_0902_t;

/*
	参数组：0x0903，N1压力分布
*/
typedef struct sumitomo_parm_grp_0903_s
{
	u8 N1PressDst_1[6];
	u8 N1PressDst_2[6];
	u8 N1PressDst_3[6];
	u8 N1PressDst_4[6];
}sumitomo_parm_grp_0903_t;

/*
	参数组：0x0904，N2压力分布
*/
typedef struct sumitomo_parm_grp_0904_s
{
	u8 N2PressDst_1[6];
	u8 N2PressDst_2[6];
	u8 N2PressDst_3[6];
	u8 N2PressDst_4[6];
}sumitomo_parm_grp_0904_t;

/*
	参数组：0x0905，P1+P2压力分布
*/
typedef struct sumitomo_parm_grp_0905_s
{
	u8 P1_P2PressDst_1[6];
	u8 P1_P2PressDst_2[6];
	u8 P1_P2PressDst_3[6];
	u8 P1_P2PressDst_4[6];
}sumitomo_parm_grp_0905_t;

/*
	参数组：0x0906，走行単独中のP1圧力分布
*/
typedef struct sumitomo_parm_grp_0906_s
{
	u8 TravelP1PressDst_1[6];
	u8 TravelP1PressDst_2[6];
	u8 TravelP1PressDst_3[6];
	u8 TravelP1PressDst_4[6];
}sumitomo_parm_grp_0906_t;

/*
	参数组：0x0907，走行単独中のP2圧力分布
*/
typedef struct sumitomo_parm_grp_0907_s
{
	u8 TravelP2PressDst_1[6];
	u8 TravelP2PressDst_2[6];
	u8 TravelP2PressDst_3[6];
	u8 TravelP2PressDst_4[6];
}sumitomo_parm_grp_0907_t;

/*
	参数组：0x0908，ｱｰﾑｼﾘﾝﾀﾞﾎﾞﾄﾑ圧分布
*/
typedef struct sumitomo_parm_grp_0908_s
{
	u8 ArmCylBottomPressDst_1[6];
	u8 ArmCylBottomPressDst_2[6];
	u8 ArmCylBottomPressDst_3[6];
	u8 ArmCylBottomPressDst_4[6];
}sumitomo_parm_grp_0908_t;

/*
	参数组：0x0909，ｱｰﾑｼﾘﾝﾀﾞﾛｯﾄﾞ圧分布
*/
typedef struct sumitomo_parm_grp_0909_s
{
	u8 ArmCylRodPressDst_1[6];
	u8 ArmCylRodPressDst_2[6];
	u8 ArmCylRodPressDst_3[6];
	u8 ArmCylRodPressDst_4[6];
}sumitomo_parm_grp_0909_t;

/*
	参数组：0x090A，ﾌﾞｰﾑｼﾘﾝﾀﾞﾎﾞﾄﾑ圧分布
*/
typedef struct sumitomo_parm_grp_090A_s
{
	u8 BoomCylBottomPressDst_1[6];
	u8 BoomCylBottomPressDst_2[6];
	u8 BoomCylBottomPressDst_3[6];
	u8 BoomCylBottomPressDst_4[6];
}sumitomo_parm_grp_090A_t;

/*
	参数组：0x090B，ﾌﾞｰﾑｼﾘﾝﾀﾞﾛｯﾄﾞ圧分布
*/
typedef struct sumitomo_parm_grp_090B_s
{
	u8 BoomCylRodPressDst_1[6];
	u8 BoomCylRodPressDst_2[6];
	u8 BoomCylRodPressDst_3[6];
	u8 BoomCylRodPressDst_4[6];
}sumitomo_parm_grp_090B_t;

/*
	参数组：0x090C，油温分布
*/
typedef struct sumitomo_parm_grp_090C_s
{
	u8 HydOilTempDst_1[6];
	u8 HydOilTempDst_2[6];
	u8 HydOilTempDst_3[6];
	u8 HydOilTempDst_4[6];
}sumitomo_parm_grp_090C_t;

/*
	参数组：0x090D，作動油ﾌｨﾙﾀ目詰まり圧力分布
*/
typedef struct sumitomo_parm_grp_090D_s
{
	u8 HydOilFilterPressDst_1[6];
	u8 HydOilFilterPressDst_2[6];
	u8 HydOilFilterPressDst_3[6];
	u8 HydOilFilterPressDst_4[6];
}sumitomo_parm_grp_090D_t;

/*
	参数组：0x090E，連続走行時間分布
*/
typedef struct sumitomo_parm_grp_090E_s
{
	u8 TravelOprTimeDst_1[6];
	u8 TravelOprTimeDst_2[6];
	u8 TravelOprTimeDst_3[6];
}sumitomo_parm_grp_090E_t;

/*
	参数组：0x090F，連続走行時間ﾗﾝｷﾝｸﾞ
*/
typedef struct sumitomo_parm_grp_090F_s
{
	u8 TravelOprTimeMax_1[6];
	u8 TravelOprTimeMax_2[6];
}sumitomo_parm_grp_090F_t;

/*
	参数组：0x0910，ｴﾝｼﾞﾝ実回転数分布
*/
typedef struct sumitomo_parm_grp_0910_s
{
	u8 EngineActualSpeedDst_1[6];
	u8 EngineActualSpeedDst_2[6];
	u8 EngineActualSpeedDst_3[6];
	u8 EngineActualSpeedDst_4[6];
}sumitomo_parm_grp_0910_t;

/*
	参数组：0x0911，水温分布
*/
typedef struct sumitomo_parm_grp_0911_s
{
	u8 CoolantTempDst_1[6];
	u8 CoolantTempDst_2[6];
	u8 CoolantTempDst_3[6];
	u8 CoolantTempDst_4[6];
}sumitomo_parm_grp_0911_t;

/*
	参数组：0x0912，ﾎｯﾄｼｬｯﾄﾀﾞｳﾝ分布
*/
typedef struct sumitomo_parm_grp_0912_s
{
	u8 CoolDownTimeDst_1[6];
	u8 CoolDownTimeDst_2[6];
}sumitomo_parm_grp_0912_t;

/*
	参数组：0x0913，燃料温度分布
*/
typedef struct sumitomo_parm_grp_0913_s
{
	u8 FuelTempDst_1[6];
	u8 FuelTempDst_2[6];
	u8 FuelTempDst_3[6];
	u8 FuelTempDst_4[6];
}sumitomo_parm_grp_0913_t;

/*
	参数组：0x0914，吸気温度分布
*/
typedef struct sumitomo_parm_grp_0914_s
{
	u8 InletAirTempDst_1[6];
	u8 InletAirTempDst_2[6];
	u8 InletAirTempDst_3[6];
	u8 InletAirTempDst_4[6];
}sumitomo_parm_grp_0914_t;

/*
	参数组：0x0915，ﾌﾞｰｽﾄ温度分布
*/
typedef struct sumitomo_parm_grp_0915_s
{
	u8 BoostTempDst_1[6];
	u8 BoostTempDst_2[6];
	u8 BoostTempDst_3[6];
	u8 BoostTempDst_4[6];
}sumitomo_parm_grp_0915_t;

/*
	参数组：0x0916，大気圧　時間分布
*/
typedef struct sumitomo_parm_grp_0916_s
{
	u8 BaroPressDst_1[6];
	u8 BaroPressDst_2[6];
	u8 BaroPressDst_3[6];
	u8 BaroPressDst_4[6];
}sumitomo_parm_grp_0916_t;

/*
	参数组：0x0917，ｴﾝｼﾞﾝｵｲﾙ圧　時間分布
*/
typedef struct sumitomo_parm_grp_0917_s
{
	u8 EngineOilPressDst_1[6];
	u8 EngineOilPressDst_2[6];
	u8 EngineOilPressDst_3[6];
	u8 EngineOilPressDst_4[6];
}sumitomo_parm_grp_0917_t;

/*
	参数组：0x0918，ｴﾝｼﾞﾝｵｲﾙ圧立ち上がり時間分布
*/
typedef struct sumitomo_parm_grp_0918_s
{
	u8 EngineOilPressRiseTimeDst_1[6];
	u8 EngineOilPressRiseTimeDst_2[6];
	u8 EngineOilPressRiseTimeDst_3[6];
}sumitomo_parm_grp_0918_t;

/*
	参数组：0x0919，ﾌﾞｰｽﾄ圧分布
*/
typedef struct sumitomo_parm_grp_0919_s
{
	u8 BoostPressDst_1[6];
	u8 BoostPressDst_2[6];
	u8 BoostPressDst_3[6];
	u8 BoostPressDst_4[6];
}sumitomo_parm_grp_0919_t;

/*
	参数组：0x091A，負荷率分布
*/
typedef struct sumitomo_parm_grp_091A_s
{
	u8 EngineLoadRatioDst_1[6];
	u8 EngineLoadRatioDst_2[6];
	u8 EngineLoadRatioDst_3[6];
	u8 EngineLoadRatioDst_4[6];
}sumitomo_parm_grp_091A_t;

/*
	参数组：0x091B，負荷率分布（SPﾓｰﾄﾞ）
*/
typedef struct sumitomo_parm_grp_091B_s
{
	u8 EngineLoadRatioSPDst_1[6];
	u8 EngineLoadRatioSPDst_2[6];
	u8 EngineLoadRatioSPDst_3[6];
	u8 EngineLoadRatioSPDst_4[6];
}sumitomo_parm_grp_091B_t;

/*
	参数组：0x091C，負荷率分布（Hﾓｰﾄﾞ）
*/
typedef struct sumitomo_parm_grp_091C_s
{
	u8 EngineLoadRatioHDst_1[6];
	u8 EngineLoadRatioHDst_2[6];
	u8 EngineLoadRatioHDst_3[6];
	u8 EngineLoadRatioHDst_4[6];
}sumitomo_parm_grp_091C_t;

/*
	参数组：0x091D，負荷率分布（Aﾓｰﾄﾞ）
*/
typedef struct sumitomo_parm_grp_091D_s
{
	u8 EngineLoadRatioADst_1[6];
	u8 EngineLoadRatioADst_2[6];
	u8 EngineLoadRatioADst_3[6];
	u8 EngineLoadRatioADst_4[6];
}sumitomo_parm_grp_091D_t;

/*
	参数组：0x091E，ｻﾌﾟﾗｲﾎﾟﾝﾌﾟ入口圧力　時間分布
*/
typedef struct sumitomo_parm_grp_091E_s
{
	u8 SupplyPumpPressDst_1[6];
	u8 SupplyPumpPressDst_2[6];
	u8 SupplyPumpPressDst_3[6];
	u8 SupplyPumpPressDst_4[6];
}sumitomo_parm_grp_091E_t;

/*
	参数组：0x091F，DOC前温度分布
*/
typedef struct sumitomo_parm_grp_091F_s
{
	u8 DOCInTempDst_1[6];
	u8 DOCInTempDst_2[6];
	u8 DOCInTempDst_3[6];
	u8 DOCInTempDst_4[6];
}sumitomo_parm_grp_091F_t;

/*
	参数组：0x0920，DOC後温度分布
*/
typedef struct sumitomo_parm_grp_0920_s
{
	u8 DOCOutTempDst_1[6];
	u8 DOCOutTempDst_2[6];
	u8 DOCOutTempDst_3[6];
	u8 DOCOutTempDst_4[6];
}sumitomo_parm_grp_0920_t;

/*
	参数组：0x0921，EGR#1入口温度
*/
typedef struct sumitomo_parm_grp_0921_s
{
	u8 EGR_1InTempDst_1[6];
	u8 EGR_1InTempDst_2[6];
	u8 EGR_1InTempDst_3[6];
	u8 EGR_1InTempDst_4[6];
}sumitomo_parm_grp_0921_t;

/*
	参数组：0x0922，EGR#1出口温度
*/
typedef struct sumitomo_parm_grp_0922_s
{
	u8 EGR_1OutTempDst_1[6];
	u8 EGR_1OutTempDst_2[6];
	u8 EGR_1OutTempDst_3[6];
	u8 EGR_1OutTempDst_4[6];
}sumitomo_parm_grp_0922_t;

/*
	参数组：0x0923，EGR#2入口温度
*/
typedef struct sumitomo_parm_grp_0923_s
{
	u8 EGR_2InTempDst_1[6];
	u8 EGR_2InTempDst_2[6];
	u8 EGR_2InTempDst_3[6];
	u8 EGR_2InTempDst_4[6];
}sumitomo_parm_grp_0923_t;

/*
	参数组：0x0924，EGR#2出口温度	(参数组写错了？)
*/
typedef struct sumitomo_parm_grp_0924_s
{
	u8 EGR_2OutTempDst_1[6];
	u8 EGR_2OutTempDst_2[6];
	u8 EGR_2OutTempDst_3[6];
	u8 EGR_2OutTempDst_4[6];
}sumitomo_parm_grp_0924_t;

/*
	参数组：0x0925，ｲﾝﾀｰｸｰﾗ入口温度
*/
typedef struct sumitomo_parm_grp_0925_s
{
	u8 InterCoolerTempDst_1[6];
	u8 InterCoolerTempDst_2[6];
	u8 InterCoolerTempDst_3[6];
	u8 InterCoolerTempDst_4[6];
}sumitomo_parm_grp_0925_t;

/*
	参数组：0x0926，ｲﾝﾃｰｸﾏﾆﾎｰﾙﾄﾞ温度
*/
typedef struct sumitomo_parm_grp_0926_s
{
	u8 ManifoldTempDst_1[6];
	u8 ManifoldTempDst_2[6];
	u8 ManifoldTempDst_3[6];
	u8 ManifoldTempDst_4[6];
}sumitomo_parm_grp_0926_t;

/*
	参数组：0x0927，ｺﾓﾝﾚｰﾙ圧　時間分布
*/
typedef struct sumitomo_parm_grp_0927_s
{
	u8 CommonRailPressDst_1[6];
	u8 CommonRailPressDst_2[6];
	u8 CommonRailPressDst_3[6];
	u8 CommonRailPressDst_4[6];
}sumitomo_parm_grp_0927_t;

/*
	参数组：0x0928，ｺﾓﾝﾚｰﾙ差圧　時間分布
*/
typedef struct sumitomo_parm_grp_0928_s
{
	u8 CommonRailDiffPressDst_1[6];
	u8 CommonRailDiffPressDst_2[6];
	u8 CommonRailDiffPressDst_3[6];
	u8 CommonRailDiffPressDst_4[6];
}sumitomo_parm_grp_0928_t;

/*
	参数组：0x0929，DPD差圧分布
*/
typedef struct sumitomo_parm_grp_0929_s
{
	u8 DPDDiffPressDst_1[6];
	u8 DPDDiffPressDst_2[6];
	u8 DPDDiffPressDst_3[6];
	u8 DPDDiffPressDst_4[6];
}sumitomo_parm_grp_0929_t;

/*
	参数组：0x092A，A/Cﾌﾞﾛﾜ
*/
typedef struct sumitomo_parm_grp_092A_s
{
	u8 AirconditionerBlowerDst_1[6];
	u8 AirconditionerBlowerDst_2[6];
	u8 AirconditionerBlowerDst_3[6];
}sumitomo_parm_grp_092A_t;

/*
	参数组：0x092B，A/Cﾀｰｹﾞｯﾄ温度
*/
typedef struct sumitomo_parm_grp_092B_s
{
	u8 AirconditionerTargetTempDst_1[6];
	u8 AirconditionerTargetTempDst_2[6];
	u8 AirconditionerTargetTempDst_3[6];
	u8 AirconditionerTargetTempDst_4[6];
}sumitomo_parm_grp_092B_t;

/*
	参数组：0x0A00
*/
typedef struct sumitomo_parm_grp_0A00_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	u8 MainteTime_1[6];
	u8 MainteTime_2[6];
	u8 MainteTime_3[6];
	u8 MainteTime_4[6];
	u8 MainteTime_5[6];
}sumitomo_parm_grp_0A00_t;

/*
	参数组：0x0B00
*/
typedef struct sumitomo_parm_grp_0B00_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	u8 Manual_Re_Gen_Notification[2];	//（ATSReGen_10的一部分）
	u8 Auto_Re_Gen_Start[2];	//（ATSReGen_1的一部分）
	u8 Auto_Re_Gen_Finish[2];	//（ATSReGen_2的一部分）
	u8 Manual_Re_Gen_Start[2];	//（ATSReGen_4的一部分）
	u8 Manual_Re_Gen_Finish[2];	//（ATSReGen_5的一部分）
	u8 Manual_Re_Gen_Start_By_Reqest[2];	//ATSReGen_7的一部分
}sumitomo_parm_grp_0B00_t;

/*
	参数组：0x0C00
*/
typedef struct sumitomo_parm_grp_0C00_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	u8 canbus_b_0CF00400[6];
	u8 canbus_b_18FDB200[6];
	//暂无说明？
}sumitomo_parm_grp_0C00_t;

/*
	参数组：0x0D00
*/
typedef struct sumitomo_parm_grp_0D00_s
{
	bit_zone_comm_t bzcommon;	//通用位域结构体

	u8 canbus_b_18FDB200[6];
}sumitomo_parm_grp_0D00_t;

extern void Make_Sumitomo_0001_s(sumitomo_parm_grp_0001_t *sumitomo_0001);
extern void Make_Sumitomo_0002_s(sumitomo_parm_grp_0002_t *sumitomo_0002);
extern void Make_Sumitomo_0003_s(sumitomo_parm_grp_0003_t *sumitomo_0003);
extern void Make_Sumitomo_0004_s(sumitomo_parm_grp_0004_t *sumitomo_0004);
extern void Make_Sumitomo_0005_s(sumitomo_parm_grp_0005_t *sumitomo_0005);
extern void Make_Sumitomo_0006_s(sumitomo_parm_grp_0006_t *sumitomo_0006);
extern void Make_Sumitomo_0008_s(sumitomo_parm_grp_0008_t *sumitomo_0008);
extern void Make_Sumitomo_0009_s(sumitomo_parm_grp_0009_t *sumitomo_0009);

extern u8 cand_Init_Set;//初期设定状态标志位,0x00-初期设定未完成，0x01-初期设定已完成

extern void MsgMake_30();//初期设定信息

extern void *canh_TransProcess(void *argc);


#endif /* CANHANDLE_H_ */
