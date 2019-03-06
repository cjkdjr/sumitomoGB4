/*
 * Msg_task.h
 *
 *  Created on: 2019年1月24日
 *      Author: tykj
 */

#ifndef MSG_TASK_H_
#define MSG_TASK_H_

#include <stdint.h>

#include "candata.h"
#include "general.h"


//配置参数保存目录
#define D1_file   "/opt/D1.ty"           //定时透传设置信息保存文件
#define D5_file   "/opt/D5.ty"           //事件设置信息保存文件
#define D9_file   "/opt/D9.ty"           //事件透传设置信息保存文件
#define E5_file   "/opt/E5.ty"           //终端参数设置信息保存文件
#define E9_file   "/opt/E9.ty"           //参数统计设置信息保存文件
#define F1_file   "/opt/F1.ty"           //发送单元设置信息保存文件
#define F3_file   "/opt/F3.ty"           //定时拍照设置信息

/******************【0x37】终端参数设置信息***************************/
typedef struct {
	u8 CtrlT_serialnum[4];                       //控制器T序列号
//	u8 SET_REC_NUMMBER[11];			 //设置中心接收号
//	u8 SET_SEND_NUMMBER[11]; 		 //设置中心发送号
//	u8 SET_RECV_IP[7];                     //中心接收IP地址
//	u8 SET_SEND_IP[7];                    //中心发送IP地址
//	u16 SET_GPRS_LINKTIME;           //GPRS下载询问周期
//	u8 SET_MSG_SENDTYPE;             //信息传输方式
//	u8 SET_TIMEINFO_STATUS;      //定时信息状态
	u8 SET_FAULTINFO_STATUS;    //故障信息状态
	u8 SET_SECURACT_NOTIFIC_STATUS;//防盗动作通知信息状态
	u8 SET_MAINTENANCE_NOTIFIC_STATUS;//维修通知信息状态
	u8 SET_FUELMARGIN_NOTIFIC_STATUS;//燃料余量通知信息状态
	u8 SET_LOG_STATUS;            //日志信息状态
	u8 SET_T_DIAGNOSTIC_STATUS;//终端终端信息状态
//	u8 SET_LOCK_CONFIRM;   //锁车确认
	u8 SET_T_VER[2];                     //终端版本号
//	u8 SET_PRESSDISTINFO_STATUS;//压力分布信息状态
	u8 SET_T_PARTCODE[12];           //控制器T部件编码
	u8 SET_IP_BACKUP[7];             //中心备用接收/发送IP地址
	u8 SET_IP_IAP[7];                //远程升级接收/发送ip地址
	u8 SET_GSM_ANTENNA_LOCK[2]; //GSM天线缩设置
	u8 SET_GPS_ANTENNA_LOCK[2];//GPS天线锁设置
	u8 SET_SIM_LOCK[2];                    //SIM卡锁设置
	u8 SET_SIM_LOGO[22];                     //SIM卡标识设置
} SYS_SET_Struct1;
typedef struct {
	u8 SET_REC_NUMMBER[11];			 //设置中心接收号
	u8 SET_SEND_NUMMBER[11]; 		 //设置中心发送号
	u8 SET_IP_DEFAULT[7];            //中心默认接收IP地址
	u8 SET_APN_DEFAULT[15];          //中心默认GPRS接入点
	u8 SET_IP_BACKUP[7];             //中心备用接收/发送IP地址
	u8 SET_APN_BACKUP[15];           //中心备用GPRS接入点
	u8 SET_IP_IAP[7];                //远程升级接收/发送ip地址
	u8 SET_APN_IAP[15];              //远程升级GPRS接入点
	u8 SET_CENTER_NUMMBER[11];       //短信中心号码设置
	u8 SET_OWNERSHIP_DISPLAY;//所有权显示设定
	u16 SET_SIMPLE_INTERVAL;  //取样间隔 默认50ms
	u16 SET_SIMPLE_CYCLE;          //取样周期 默认5（0.1s）
	u16 SET_SIMPTIME_AFTER_ESC;//目标发动机转数变更后取样开始时间 默认5s
	u16 SET_SIMPTIME_BEFORE_SESC;//目标发动机转数变更前可以取样时间 默认0.5s
	u16 SET_SIMPTIME_UST;//上部机体(upper)、回转(swing)、行走(travel)OFF后取样开始时间 默认5s
//	u16 SET_ON_SENDTIME;  //开机定时信息间隔 默认30min
//	u16 SET_OFF_SENDTIME;  //关机定时信息间隔 默认180min
//	u16 SET_OFF_RANGE;//关机报警距离 默认200m
	u8 SET_IP_CUT;        //ip地址切换功能
	u16 SET_IP_CUTMAX; //IP链接错误上限次数
	u16 SET_AUTOSMS_CYCLE;//自动短信投递周期

	u8 RESESTER_NET;             	 //注册网络模式0自动 1 ：2G    2：4G
	u16  Time_Zone_Config;    //时区配置，默认东八区480分钟
} SYS_SET_Struct2;
SYS_SET_Struct1 SYS_SET1;
extern SYS_SET_Struct2 SYS_SET2;
dwordbit ParamOpera; //参数选项


//配置参数保存目录
#define B5_file   "/opt/B5.ty"           //事件设置信息保存文件
#define B9_file   "/opt/B9.ty"           //事件透传设置信息保存文件
#define C5_file   "/opt/C5.ty"           //实时诊断透传参数设置
/******************【0xC3】实时诊断透传参数设置信息 ***************************/
typedef struct{
	u8 BeginSet;
	u16 KeepTime;
}DIAGNOSIS_SET;

extern DIAGNOSIS_SET diagnosis_set;
/******************【0xC5】实时诊断透传参数设置信息 ***************************/
typedef struct {
	u32 id :29; // PGN ID 占剩余29位（高字节在前），如帧格式为0，低11位有效
	u32 can :3; // bit31-29 0-CAN0；1-CAN1；2-CAN2；3-CAN3；7-终端自身；其它-暂未定义
} TDF_PGN_SET_ID;
typedef struct {
	u8 reserve :4;
	u8 da_id :1; // bit4 数据ID?  0-无 1-有
	u8 bags :1; // bit5 多包?  0-无 1-有
	u8 req :1; // bit6 请求?  0-无 1-有
	u8 ide :1; // bit7 帧格式 0-标准帧 1-扩展帧
} TDF_PGN_SET_ID_TYPE;

// 多包广播ID
// 多包数据ID
typedef struct {
	u32 id :29; // PGN ID 占剩余29位（高字节在前），如帧格式为0，低11位有效
	u32 reserve :2;
	u32 ide :1; // bit31 0-标准帧；1-扩展帧
} TDF_PGN_SET_ID1;
typedef struct {
	u8 reserve :2;
	u8 id_start :3; // bit4-2 数据ID所占数据域的起始字节数，取值范围为0-7
	u8 id_len :3;   // bit7-5 数据ID长度，取值范围1-7
	u8 id_da[7];   // 数据id内容
} TDF_PGN_SET_ID_DA;
typedef struct {
	TDF_PGN_SET_ID_TYPE mode;
	TDF_PGN_SET_ID pgn_id;
	u32 cyc_get;   //采集频率
	TDF_PGN_SET_ID1 pgn_id0; // 广播包id
	TDF_PGN_SET_ID1 pgn_id1; // 数据包id
	TDF_PGN_SET_ID_DA pgn_id_da; // 数据id格式 数据

} TDF_PGN_SET; //PGN参数ID包

#define MSG_PGN_SET_MAX 200 //定时透传pgn最大包数
extern TDF_PGN_SET hand_pgn_set[MSG_PGN_SET_MAX];  //【0xC5】定时透传设置信息
extern TDF_PGN_SET hand_BTpgn_set[MSG_PGN_SET_MAX];
extern u8 msg_pgn_set_num;	//PGN参数ID个数
extern u8 msg_pgn_BTset_num; //BTPGN参数ID个数b

typedef struct {
	u8 total_num: 4; // 总包数最大16包
	u8 today_num: 4;
}TDF_BAGS_NUM;

extern TDF_BAGS_NUM cand_pgn_bags_num[MSG_PGN_SET_MAX];
extern u32 cand_pgn_cyc_get_timer[MSG_PGN_SET_MAX]; // 和数据pgnset逐一对应

typedef struct{
		u8 HaveSend;
		CanMsg GPS_inflect[5];
}GPSCanMsg;
extern GPSCanMsg GPScache[3];

#define   CAN_ID_PGN_GB       (0x00ECFF00L) // 1939 CAN广播包ID
#define   CAN_ID_PGN_DATA     (0x00EBFF00L) // 1939 CAN数据包ID
/******************【0xB5】事件设置信息  ***************************/
typedef struct {
	u8 num :7; // 事件设置所处位置，取值范围[1~100]
	u8 state :1; // 1-需要设置，0-清除设置
} TDF_EVENT_SET_STATE;
typedef struct {
	u32 id :29; // PGN ID 占剩余29位（高字节在前），如帧格式为0，低11位有效
	u32 reserve :1;
	u32 da_id :1; // 0无数据id 1有数据id
	u32 ide :1; // 0-标准帧 1-扩展帧
} TDF_EVENT_SET_ID;
typedef struct {
	u8 b2_0 :3; // p.2-p.0：参数在PGN中所处的起始位置，取值范围[0-7]
	u8 b3 :1; // p.3：0-当参数大于“参数设定”时事件发生条件满足，1-当参数小于“参数设置”时事件发生条件满足
	u8 b4 :1; // p.4：0-参数高字节在后，1-参数高字节在前
	u8 b7_5 :3; // p.7-p.5：描述事件参与的参数的长度，取值范围[1-4]
	//当是PGN参数开关事件时   p.7-p.5：开关参数在PGN中所处的字节位置，取值范围[0-7]
} TDF_EVENT_SET_ATTRI; // 事件参数属性
typedef struct
{
	u8 event_type; // 事件种类，决定其属性   0：阀值事件；1：开关事件
	TDF_EVENT_SET_STATE event_state;	//设置状态,位置序号
	TDF_EVENT_SET_ID pgn_id;
	TDF_EVENT_SET_ATTRI attribute; // 不同事件属性不同
	u32 param_set; // 阀值事件，参数值设置
	u32 cyc_get; // 事件持续时间设置
	TDF_PGN_SET_ID_DA pgn_id_da; // 数据id格式 数据
}TDF_EVENT_SET;

#define MSG_EVENT_SET_MAX 100 //事件设置最大个数
#define EVENT_PT_SET_MAX 30 //事件透传设置最大包数
extern TDF_EVENT_SET msg_event_set[MSG_EVENT_SET_MAX];//【0xB5】事件设置信息
extern u8 msg_event_set_num;	//事件设置个数
/******************【0xB9】事件设置信息  ***************************/
#define MSG_EVENT_TRANS_SET_MAX 30
extern TDF_BAGS_NUM cand_event_bags_num[MSG_EVENT_TRANS_SET_MAX];
extern u32 cand_event_cyc_get_timer[MSG_EVENT_TRANS_SET_MAX]; // 和数据eventset逐一对应
extern u8 msg_event_set_num;	//事件设置个数
extern TDF_PGN_SET cand_event_set[MSG_EVENT_TRANS_SET_MAX]; //【0xB9】事件透传设置信息

/******************数据处理函数************************************/
void copyobj(u8 *dest, void *src, int length, int *offset);
void msgDecode(u16 type, u8 *data, void *result);
void cpymem(void *dest, u8 *src, int length, int *offest);
void strGet(u8 *dest, u8 *src, int length, int *offset);
/****************** 参数保存读取函数 ********************************/
extern void save2flash(char *file, u8 *data, u32 length);       //参数保存
extern int getData(char *file, u8 *data);                      //参数读取
/******************协议处理函数************************************/
extern void MsgDecode_31(u8 *serialnum, u8 *data);
extern void MsgMake_32(u8 *serialnum);
extern void MsgMake_22(QUE_TDF_QUEUE_MSG *Msg_22);
extern void InitE5();
#endif /* MSG_TASK_H_ */
