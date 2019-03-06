/*
 * Msg_task.c
 *
 *  Created on: 2019年1月24日
 *      Author: tykj
 */

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "api.h"
#include "general.h"
#include "message_process.h"
#include "candata.h"
#include "canhandle.h"
#include "Msg_task.h"

SYS_SET_Struct2 SYS_SET2;
u8 cand_serial_num[3];
u32 cand_event_cyc_get_timer[MSG_EVENT_TRANS_SET_MAX]; // 和数据eventset逐一对应
TDF_BAGS_NUM cand_event_bags_num[MSG_EVENT_TRANS_SET_MAX];
u8 Serial_Num;

u8 CenterRecv_Num[20] = { 0X07, 0X01, 0X46, 0X98, 0X09, 0X13, 0X01, 0XF2 }; //"1064899031102";
u8 CenterSend_Num[20] = { 0X07, 0X01, 0X46, 0X98, 0X09, 0X13, 0X01, 0XF2 }; //1064899031102;
u8 MessageCenterNum[13] = { 0x07, 0X68, 0X31, 0X08, 0X10, 0X00, 0X65, 0XF9 }; //"8613800100569"

char CenterDefaultIP[7] = { 0x06, 0X6F, 0X0B, 0X04, 0X0C, 0x3A, 0xBC };/*111.11.4.12:15036*/
char DefaultAPN[21] = { 0X05, 0X63, 0X6D, 0X69, 0X6F, 0X74 };/*cmiot*/
char CenterBackIP[7] = { 0x06, 0X6F, 0X0B, 0X04, 0X0C, 0x3A, 0xBC };/*111.11.4.12:15036*/
char BackAPN[15] = { 0x05, 0x63, 0x6D, 0x6E, 0x65, 0x74 }; /*CMNET*/
char IAPDefaultIP[7] = { 0x06, 0XDA, 0XCF, 0X42, 0X6B, 0x0F, 0xF8 };/*218.207.66.107:4088*/

void MsgMake_32(u8 *serialnum);
void MsgMake_34(u8 *serialnum, u8 *data);
void MsgMake_36(u8 *serialnum, u8 *data);
void MsgMake_38(u8 *serialnum, u8 TypeVer, u32 ParOpt, u8 ReplyType);
void MsgMake_42(u8 *serialnum, u16 ArrayNum, u8 ReplyType);

u16 bytes_cpy(u8 *dest, u8 *src, u16 len) {
	u16 i;

	for (i = 0; i < len; i++) {
		dest[i] = src[i];
	}

	return (i);
}
u32 bytes_dword(u8 *buf) {
	u32 tmp;

	tmp = buf[0];
	tmp <<= 8;
	tmp |= buf[1];
	tmp <<= 8;
	tmp |= buf[2];
	tmp <<= 8;
	tmp |= buf[3];

	return (tmp);
}
/**
 *拷贝内存
 *dest目的地址
 *src 源地址
 *length 拷贝长度
 *offest 偏移量
 */
void cpymem(void *dest, u8 *src, int length, int *offest) {
	int i = 0;
	for (; i < length; i++) {
		//dest[i] = src[*offest+length-1-i];
		memcpy(dest + i, src + (*offest + length - i - 1), 1);
	}
	*offest += length;
}
void copyobj(u8 *dest, void *src, int length, int *offset) {
	int i;
	for (i = 0; i < length; i++) {
		memcpy(dest + i/* + *offset*/, src + (length - i - 1), 1);
	}
	*offset += length;
}
/**
 * 保存参数到本地
 * data 数据
 * type 数据类型
 */
void save2flash(char *file, u8 *data, u32 length) {
	int fd;
	ssize_t w_length = 0;
	fd = open(file, O_CREAT | O_RDWR | O_TRUNC, 0777);
	if (fd < 0) {
		//TODO  plog
		printf_msg("open file error!\n");
		if (debug_value & 0x80) {
			perror("open file");
		}
	} else {
		w_length = write(fd, data, length);
		if (w_length == length) {
			printf_msg("write success %d\n", w_length);

		} else {
			printf_msg("write error\n");
			//TODO  plog
		}

	}
	close(fd);
}

/*********************************************************
 初始化E5参数，中心没有设置时候且本地也没有保存参数情况下设置
 *********************************************************/
void InitE5() {
	if (SYS_SET2.SET_REC_NUMMBER[0] == 0 || SYS_SET2.SET_REC_NUMMBER[0] == 0XFF) {
		memcpy((char*) SYS_SET2.SET_REC_NUMMBER, (char*) CenterRecv_Num, 8);
		//printfHexData(SYS_SET.SET_REC_NUMMBER,8);
	}
	if (SYS_SET2.SET_SEND_NUMMBER[0] == 0 || SYS_SET2.SET_SEND_NUMMBER[0] == 0XFF)
		memcpy((char*) SYS_SET2.SET_SEND_NUMMBER, (char*) CenterSend_Num, 8);
	if (SYS_SET2.SET_IP_DEFAULT[0] == 0 || SYS_SET2.SET_IP_DEFAULT[0] == 0XFF)
		memcpy((char*) SYS_SET2.SET_IP_DEFAULT, CenterDefaultIP, 7);
	if (SYS_SET2.SET_APN_DEFAULT[0] == 0 || SYS_SET2.SET_APN_DEFAULT[0] == 0XFF)
		memcpy((char*) SYS_SET2.SET_APN_DEFAULT, DefaultAPN, 6);
	if (SYS_SET2.SET_IP_BACKUP[0] == 0 || SYS_SET2.SET_IP_BACKUP[0] == 0XFF)
		memcpy((char*) SYS_SET2.SET_IP_BACKUP, CenterBackIP, 7);
	if (SYS_SET2.SET_APN_BACKUP[0] == 0 || SYS_SET2.SET_APN_BACKUP[0] == 0XFF)
		memcpy((char*) SYS_SET2.SET_APN_BACKUP, BackAPN, 6);
	if (SYS_SET2.SET_IP_IAP[0] == 0 || SYS_SET2.SET_IP_IAP[0] == 0XFF)
		memcpy((char*) SYS_SET2.SET_IP_IAP, IAPDefaultIP, 7);
	if (SYS_SET2.SET_APN_IAP[0] == 0 || SYS_SET2.SET_APN_IAP[0] == 0XFF)
		memcpy((char*) SYS_SET2.SET_APN_IAP, DefaultAPN, 6);
	if (SYS_SET2.SET_CENTER_NUMMBER[0] == 0
			|| SYS_SET2.SET_CENTER_NUMMBER[0] == 0XFF)
		memcpy((char*) SYS_SET2.SET_CENTER_NUMMBER, (char*) MessageCenterNum, 8); // sms center of cmcc.heb
	if (SYS_SET2.SET_IP_CUT != 1)
		SYS_SET2.SET_IP_CUT = 0X00;
	if (SYS_SET2.SET_IP_CUTMAX <= 0 || SYS_SET2.SET_IP_CUTMAX > 9)
		SYS_SET2.SET_IP_CUTMAX = 9;
	if (SYS_SET2.RESESTER_NET == 0 || SYS_SET2.RESESTER_NET == 0XFF)
		SYS_SET2.RESESTER_NET = 2; //0自动，1：2G，2：4G
	if (SYS_SET2.Time_Zone_Config == 0 || SYS_SET2.Time_Zone_Config == 0XFF)
		SYS_SET2.Time_Zone_Config = 480;   //默认东八区480分钟
}
/**
 * 从本地读取数据,开机读取配置文件使用
 * u8 *file:文件路径+名
 * u8 *data:读取数据返回串
 */
int getData(char *file, u8 *data) {
	ssize_t length = 0;
	FILE * fp = fopen(file, "rb");
	if (fp != NULL) {
		fseek(fp, 0L, SEEK_END);
		length = ftell(fp);
		fseek(fp, 0L, SEEK_SET);

		if (length == fread(data, sizeof(u8), length, fp)) {
			data[length] = '\0';
			printf_msg("read %s success len:%d\n", file, length);
			//printfHexData(data,length);
		} else {
			printf_msg("read error\n");
			//TODO  plog
		}
	} else {
		printf_msg("there is no %s file!\n", file);
		return 0;
	}
	return length;
}
/*
 版本号赋值
 */
void cpyVersion(u8 *des) {
	//版本号
	des[0] = Version.Protocol >> 8;
	des[1] = Version.Protocol;
	des[2] = Version.Code;
	return;
}
/*
 信息生成时间+流水号赋值
 */
u32 cpyInfoTime(u8 *des) {
	struct tm *p;
//	struct timeval tv;
//	u16 tmp_sec;
	time_t now_time;
	u32 sys_run_msec;
	u16 tmp_u16;
	//信息生成时间:获取当前时间 = 系统运行时间+ key on时间（RTC时间）
	u32 Sys_Run_Time = api_GetSysSecs();
	now_time = Sys_Start_Time + Sys_Run_Time;
	p = localtime(&now_time);
	des[0] = p->tm_year - 100;
	des[1] = p->tm_mon + 1;
	des[2] = p->tm_mday;
	des[3] = p->tm_hour;
	des[4] = p->tm_min;
	tmp_u16 = (p->tm_sec) * 1000;
	sys_run_msec = api_GetSysmSecs();
	tmp_u16 += sys_run_msec % 1000;
	des[5] = tmp_u16 >> 8;
	des[6] = tmp_u16;
	//信息生成时间
//	gettimeofday(&tv, NULL);
//	p = localtime(&tv.tv_sec);
//	des[0] = p->tm_year - 100;
//	des[1] = p->tm_mon + 1;
//	des[2] = p->tm_mday;
//	des[3] = p->tm_hour;
//	des[4] = p->tm_min;
//	tmp_sec = p->tm_sec * 1000 + tv.tv_usec / 1000;
//	des[5] = tmp_sec >> 8;
//	des[6] = tmp_sec;
	if (Serial_Num < 0xFF) {
		Serial_Num++;
	} else {
		Serial_Num = 0;
	}
	des[7] = Serial_Num;
	return sys_run_msec;
}

void MsgMake_22(QUE_TDF_QUEUE_MSG *Msg_22) {
//	u16 index = 0;

	memset(Msg_22, 0, sizeof(QUE_TDF_QUEUE_MSG));
	// 信息类型
	Msg_22->MsgType = 0x22;
	//版本号
	cpyVersion(Msg_22->Version);
	// 信息生成时间
	cpyInfoTime(Msg_22->Time);
	//属性标识  高字节在前
	Msg_22->Attribute[0] = 0x40;
	Msg_22->Attribute[1] = 0;

	memcpy(Msg_22->data, &sumitomo_parm_grp_0001, sizeof(sumitomo_parm_grp_0001_t));

#if 0
	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_22->length, strlen((char*) len));
#endif
}


//-----------------------------------------交互类信息-----------------------------------------------//

/*
 终端通用应答消息
 */
QUE_TDF_QUEUE_MSG Msg_60;
void RT_ger_reply_msg(u8 *serialnum, u8 msgType, u8 result) {
	memset(&Msg_60, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	//信息类型
	Msg_60.MsgType = 0x60;
	//版本号
	cpyVersion(Msg_60.Version);
	//信息生成时间
	cpyInfoTime(Msg_60.Time);
	//属性标识  高字节在前
	Msg_60.Attribute[0] = 0x40;
	Msg_60.Attribute[1] = 0;
	//对照码
	Msg_60.data[index++] = serialnum[0];
	Msg_60.data[index++] = serialnum[1];
	Msg_60.data[index++] = serialnum[2];
	Msg_60.data[index++] = msgType; //应答信息类型
	Msg_60.data[index++] = result; //数据处理指示
	Msg_60.data[index++] = 0x00; //预留
	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_60.length, strlen((char*) len));
	//信息入队列
	printf_msg("<Make_60_message>message ready in queue %d bytes!!! \n", index + 16);
	InterSta_in(&Msg_60);
	return;
}
/*
 中心通用应答消息
 */
void MsgDecode_61(u8 *data) {

}
/*
 decode 0x31 初期设定回复信息
 */
void MsgDecode_31(u8 *serialnum, u8 *data) {
	if (data[0] == 0x31) {
		if (data[1] == 0x00) {
			//TODO make 0x32 reply
			MsgMake_32(serialnum);
		} else {
			//TODO make 0x30 resend
		}
	}
}
QUE_TDF_QUEUE_MSG Msg_32;
void MsgMake_32(u8 *serialnum) {
	memset(&Msg_32, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	//信息类型
	Msg_32.MsgType = 0x30;
	//版本号
	cpyVersion(Msg_32.Version);
	//信息生成时间
	cpyInfoTime(Msg_32.Time);
	//属性标识  高字节在前
	Msg_32.Attribute[0] = 0x40;
	Msg_32.Attribute[1] = 0;

	Msg_32.data[index++] = serialnum[0];
	Msg_32.data[index++] = serialnum[1];
	Msg_32.data[index++] = serialnum[2];
	Msg_32.data[index++] = 0x31;
	Msg_32.data[index++] = 0x00;

	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_32.length, strlen((char*) len));
	//信息入队列
	printf_msg("<Make_32_message>message ready in queue %d bytes!!! \n", index + 16);
	InterSta_in(&Msg_32);
}
/*
 设置/回叫初期设定状态信息
 */
void MsgDecode_33(u8 *serialnum, u8 *data) {
	if (data[0] == 0x00) { //TODO 设置初期设定状态

	} else if (data[0] == 0x01) { //TODO回叫初期设定状态

	}
	MsgMake_34(serialnum, &data[1]);
}
/*
 0x34 回叫初期设定状态回复信息
 */
QUE_TDF_QUEUE_MSG Msg_34;
void MsgMake_34(u8 *serialnum, u8 *data) {
	memset(&Msg_34, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	//信息类型
	Msg_34.MsgType = 0x34;
	//版本号
	cpyVersion(Msg_34.Version);
	//信息生成时间
	cpyInfoTime(Msg_34.Time);
	//属性标识  高字节在前
	Msg_34.Attribute[0] = 0x40;
	Msg_34.Attribute[1] = 0;

	Msg_34.data[index++] = serialnum[0];
	Msg_34.data[index++] = serialnum[1];
	Msg_34.data[index++] = serialnum[2];
	Msg_34.data[index++] = 0x33;
	Msg_34.data[index++] = data[0];

	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_34.length, strlen((char*) len));
	//信息入队列
	printf_msg("<Make_34_message>message ready in queue %d bytes!!! \n", index + 16);
	InterSta_in(&Msg_34);
}
/*
 0x35 设置/回叫配对参数信息,接收到此信息需要先回复一条终端通用应答
 */
void MsgDecode_35(u8 *serialnum, u8 *data) {
	if (data[0] == 0x00) { //设置

	} else if (data[0] == 0x01) { //回叫

	}
	//TODO reply 36
	MsgMake_36(serialnum, data);
}
/*
 0x36 回叫配对参数回复信息
 */
QUE_TDF_QUEUE_MSG Msg_36;
void MsgMake_36(u8 *serialnum, u8 *data) {
	memset(&Msg_36, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	//信息类型
	Msg_36.MsgType = 0x36;
	//版本号
	cpyVersion(Msg_36.Version);
	//信息生成时间
	cpyInfoTime(Msg_36.Time);
	//属性标识  高字节在前
	Msg_36.Attribute[0] = 0x40;
	Msg_36.Attribute[1] = 0;

	Msg_36.data[index++] = serialnum[0];
	Msg_36.data[index++] = serialnum[1];
	Msg_36.data[index++] = serialnum[2];
	Msg_36.data[index++] = 0x35;
	memcpy(Msg_36.data + index, data, 7);
	index += 7;

	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_36.length, strlen((char*) len));
	//信息入队列
	printf_msg("<Make_36_message>message ready in queue %d bytes!!! \n", index + 16);
	InterSta_in(&Msg_36);
}
/*
 0x37 设置终端参数信息
 */
int MsgDecode_37(u8 *serialnum, u8 *data, int Len) {
	int index = 0;
	u8 len = 0;
	u16 tmp_u16 = 0;
	ParamOpera.dword = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];
	if (data[0] == 0x01) { //参数组01
		if (ParamOpera.bit.b23 == 1) {
			memcpy(SYS_SET1.CtrlT_serialnum, data + index, 4);
			index += 4;
		}
		if (ParamOpera.bit.b15 == 1) {
			memcpy(&SYS_SET1.SET_FAULTINFO_STATUS, data + index, 1);
			index += 1;
		}
		if (ParamOpera.bit.b14 == 1) {
			memcpy(&SYS_SET1.SET_SECURACT_NOTIFIC_STATUS, data + index, 1);
			index += 1;
		}
		if (ParamOpera.bit.b13 == 1) {
			memcpy(&SYS_SET1.SET_MAINTENANCE_NOTIFIC_STATUS, data + index, 1);
			index += 1;
		}
		if (ParamOpera.bit.b12 == 1) {
			memcpy(&SYS_SET1.SET_FUELMARGIN_NOTIFIC_STATUS, data + index, 1);
			index += 1;
		}
		if (ParamOpera.bit.b11 == 1) {
			memcpy(&SYS_SET1.SET_LOG_STATUS, data + index, 1);
			index += 1;
		}
		if (ParamOpera.bit.b10 == 1) {
			memcpy(&SYS_SET1.SET_T_DIAGNOSTIC_STATUS, data + index, 1);
			index += 1;
		}
		if (ParamOpera.bit.b8 == 1) {
			memcpy(SYS_SET1.SET_T_VER, data + index, 2);
			index += 2;
		}
		if (ParamOpera.bit.b6 == 1) {
			memcpy(SYS_SET1.SET_T_PARTCODE, data + index, 12);
			index += 12;
		}
		if (ParamOpera.bit.b5 == 1) {
			memcpy(SYS_SET1.SET_IP_BACKUP, data + index, 7);
			index += 7;
		}
		if (ParamOpera.bit.b4 == 1) {
			memcpy(SYS_SET1.SET_IP_IAP, data + index, 7);
			index += 7;
		}
		if (ParamOpera.bit.b3 == 1) {
			memcpy(SYS_SET1.SET_GSM_ANTENNA_LOCK, data + index, 2);
			index += 2;
		}
		if (ParamOpera.bit.b2 == 1) {
			memcpy(SYS_SET1.SET_GPS_ANTENNA_LOCK, data + index, 2);
			index += 2;
		}
		if (ParamOpera.bit.b1 == 1) {
			memcpy(SYS_SET1.SET_SIM_LOCK, data + index, 2);
			index += 2;
		}
		if (ParamOpera.bit.b0 == 1) {
			memcpy(&SYS_SET1.SET_SIM_LOGO, data + index, Len - index);
		}
	} else if (data[0] == 0x02) { //参数组02
		if (ParamOpera.bit.b31 == 1) {
			memcpy(SYS_SET2.SET_IP_DEFAULT, data + index, 7);
			index += 7;
		}
		if (ParamOpera.bit.b30 == 1) {
			len = data[index++];
			memcpy(SYS_SET2.SET_APN_DEFAULT, data + index, len);
			index += len;
		}
		if (ParamOpera.bit.b29 == 1) {
			memcpy(SYS_SET2.SET_IP_BACKUP, data + index, 7);
			index += 7;
		}
		if (ParamOpera.bit.b28 == 1) {
			len = data[index++];
			memcpy(SYS_SET2.SET_APN_BACKUP, data + index, len);
			index += len;
		}
		if (ParamOpera.bit.b27 == 1) {
			memcpy(SYS_SET2.SET_IP_IAP, data + index, 7);
			index += 7;
		}
		if (ParamOpera.bit.b26 == 1) {
			len = data[index++];
			memcpy(SYS_SET2.SET_APN_IAP, data + index, len);
			index += len;
		}
		if (ParamOpera.bit.b25 == 1) {
			memcpy(&SYS_SET2.SET_OWNERSHIP_DISPLAY, data + index, 1);
			index += 1;
		}
		if (ParamOpera.bit.b24 == 1) {
			tmp_u16 = data[index++] << 8;
			tmp_u16 |= data[index++];
			SYS_SET2.SET_SIMPLE_INTERVAL = tmp_u16;
		}
		if (ParamOpera.bit.b23 == 1) {
			tmp_u16 = data[index++] << 8;
			tmp_u16 |= data[index++];
			SYS_SET2.SET_SIMPLE_CYCLE = tmp_u16;
		}
		if (ParamOpera.bit.b22 == 1) {
			tmp_u16 = data[index++] << 8;
			tmp_u16 |= data[index++];
			SYS_SET2.SET_SIMPTIME_AFTER_ESC = tmp_u16;
		}
		if (ParamOpera.bit.b21 == 1) {
			tmp_u16 = data[index++] << 8;
			tmp_u16 |= data[index++];
			SYS_SET2.SET_SIMPTIME_BEFORE_SESC = tmp_u16;
		}
		if (ParamOpera.bit.b20 == 1) {
			tmp_u16 = data[index++] << 8;
			tmp_u16 |= data[index++];
			SYS_SET2.SET_SIMPTIME_UST = tmp_u16;
		}
		if (ParamOpera.bit.b12 == 1) {
			SYS_SET2.SET_IP_CUT = data[index++];
		}
		if (ParamOpera.bit.b11 == 1) {
			tmp_u16 = data[index++] << 8;
			tmp_u16 |= data[index++];
			SYS_SET2.SET_IP_CUTMAX = tmp_u16;
		}
		if (ParamOpera.bit.b10 == 1) {
			tmp_u16 = data[index++] << 8;
			tmp_u16 |= data[index++];
			SYS_SET2.SET_AUTOSMS_CYCLE = tmp_u16;
		}
	}
	MsgMake_38(serialnum, data[0], ParamOpera.dword, 0x37);
	return 0;
}
/*
 0x38终端参数回复信息
 */
QUE_TDF_QUEUE_MSG Msg_38;
void MsgMake_38(u8 *serialnum, u8 TypeVer, u32 ParOpt, u8 ReplyType) {
	memset(&Msg_38, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	u8 L = 0;
	//信息类型
	Msg_38.MsgType = 0x38;
	//版本号
	cpyVersion(Msg_38.Version);
	//信息生成时间
	cpyInfoTime(Msg_38.Time);
	//属性标识  高字节在前
	Msg_38.Attribute[0] = 0x40;
	Msg_38.Attribute[1] = 0;

	Msg_38.data[index++] = serialnum[0];
	Msg_38.data[index++] = serialnum[1];
	Msg_38.data[index++] = serialnum[2];
	Msg_38.data[index++] = ReplyType;
	Msg_38.data[index++] = TypeVer;
	dwordbit pa;
	pa.dword = ParOpt;
	if (TypeVer == 0x01) {
		if (pa.bit.b23 == 1) {
			memcpy(Msg_38.data + index, SYS_SET1.CtrlT_serialnum, 4);
			index += 4;
		}
		if (pa.bit.b15 == 1) {
			memcpy(Msg_38.data + index, &SYS_SET1.SET_FAULTINFO_STATUS, 1);
			index += 1;
		}
		if (pa.bit.b14 == 1) {
			memcpy(Msg_38.data + index, &SYS_SET1.SET_SECURACT_NOTIFIC_STATUS, 1);
			index += 1;
		}
		if (pa.bit.b13 == 1) {
			memcpy(Msg_38.data + index, &SYS_SET1.SET_MAINTENANCE_NOTIFIC_STATUS, 1);
			index += 1;
		}
		if (pa.bit.b12 == 1) {
			memcpy(Msg_38.data + index, &SYS_SET1.SET_FUELMARGIN_NOTIFIC_STATUS, 1);
			index += 1;
		}
		if (pa.bit.b11 == 1) {
			memcpy(Msg_38.data + index, &SYS_SET1.SET_LOG_STATUS, 1);
			index += 1;
		}
		if (pa.bit.b10 == 1) {
			memcpy(Msg_38.data + index, &SYS_SET1.SET_T_DIAGNOSTIC_STATUS, 1);
			index += 1;
		}
		if (pa.bit.b8 == 1) {
			memcpy(Msg_38.data + index, SYS_SET1.SET_T_VER, 2);
			index += 2;
		}
		if (pa.bit.b6 == 1) {
			memcpy(Msg_38.data + index, SYS_SET1.SET_T_PARTCODE, 12);
			index += 12;
		}
		if (pa.bit.b5 == 1) {
			memcpy(Msg_38.data + index, SYS_SET1.SET_IP_BACKUP, 7);
			index += 7;
		}
		if (pa.bit.b4 == 1) {
			memcpy(Msg_38.data + index, SYS_SET1.SET_IP_IAP, 7);
			index += 7;
		}
		if (pa.bit.b3 == 1) {
			memcpy(Msg_38.data + index, SYS_SET1.SET_GSM_ANTENNA_LOCK, 2);
			index += 2;
		}
		if (pa.bit.b2 == 1) {
			memcpy(Msg_38.data + index, SYS_SET1.SET_GPS_ANTENNA_LOCK, 2);
			index += 2;
		}
		if (pa.bit.b1 == 1) {
			memcpy(Msg_38.data + index, SYS_SET1.SET_SIM_LOCK, 2);
			index += 2;
		}
		if (pa.bit.b0 == 1) {
			memcpy(Msg_38.data + index, &SYS_SET1.SET_SIM_LOGO, (SYS_SET1.SET_SIM_LOGO[1] + 2));
		}
	} else if (TypeVer == 0x02) {
		if (pa.bit.b31 == 1) {
			memcpy(Msg_38.data + index, SYS_SET2.SET_IP_DEFAULT, 7);
			index += 7;
		}
		if (pa.bit.b30 == 1) {
			L = SYS_SET2.SET_APN_DEFAULT[0] + 1;
			memcpy(Msg_38.data + index, SYS_SET2.SET_APN_DEFAULT, L);
			index += L;
		}
		if (pa.bit.b29 == 1) {
			memcpy(Msg_38.data + index, SYS_SET2.SET_IP_BACKUP, 7);
			index += 7;
		}
		if (pa.bit.b28 == 1) {
			L = SYS_SET2.SET_APN_BACKUP[0];
			memcpy(Msg_38.data + index, SYS_SET2.SET_APN_BACKUP, L);
			index += L;
		}
		if (pa.bit.b27 == 1) {
			memcpy(Msg_38.data + index, SYS_SET2.SET_IP_IAP, 7);
			index += 7;
		}
		if (ParamOpera.bit.b26 == 1) {
			L = SYS_SET2.SET_APN_IAP[0];
			memcpy(Msg_38.data + index, SYS_SET2.SET_APN_IAP, L);
			index += L;
		}
		if (pa.bit.b25 == 1) {
			memcpy(Msg_38.data + index, &SYS_SET2.SET_OWNERSHIP_DISPLAY, 1);
			index += 1;
		}
		if (pa.bit.b24 == 1) {
			Msg_38.data[index++] = SYS_SET2.SET_SIMPLE_INTERVAL >> 8;
			Msg_38.data[index++] = SYS_SET2.SET_SIMPLE_INTERVAL;
		}
		if (pa.bit.b23 == 1) {
			Msg_38.data[index++] = SYS_SET2.SET_SIMPLE_CYCLE >> 8;
			Msg_38.data[index++] = SYS_SET2.SET_SIMPLE_CYCLE;
		}
		if (pa.bit.b22 == 1) {
			Msg_38.data[index++] = SYS_SET2.SET_SIMPTIME_AFTER_ESC >> 8;
			Msg_38.data[index++] = SYS_SET2.SET_SIMPTIME_AFTER_ESC;
		}
		if (pa.bit.b21 == 1) {
			Msg_38.data[index++] = SYS_SET2.SET_SIMPTIME_BEFORE_SESC >> 8;
			Msg_38.data[index++] = SYS_SET2.SET_SIMPTIME_BEFORE_SESC;
		}
		if (pa.bit.b20 == 1) {
			Msg_38.data[index++] = SYS_SET2.SET_SIMPTIME_UST >> 8;
			Msg_38.data[index++] = SYS_SET2.SET_SIMPTIME_UST;
		}
		if (pa.bit.b12 == 1) {
			Msg_38.data[index++] = SYS_SET2.SET_IP_CUT;
		}
		if (pa.bit.b11 == 1) {
			Msg_38.data[index++] = SYS_SET2.SET_IP_CUTMAX >> 8;
			Msg_38.data[index++] = SYS_SET2.SET_IP_CUTMAX;
		}
		if (pa.bit.b10 == 1) {
			Msg_38.data[index++] = SYS_SET2.SET_AUTOSMS_CYCLE >> 8;
			Msg_38.data[index++] = SYS_SET2.SET_AUTOSMS_CYCLE;
		}
	}

	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_38.length, strlen((char*) len));
	//信息入队列
	printf_msg("<Make_38_message>message ready in queue %d bytes!!! \n", index + 16);
	InterSta_in(&Msg_38);
}
/*
 0x39 回叫终端参数信息
 */
void MsgDecode_39(u8 *serialnum, u8 *data) {
	ParamOpera.dword = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];
	MsgMake_38(serialnum, data[0], ParamOpera.dword, 0x39);
	return;
}
/*
 0x41 远程操作设置信息
 */
void MsgDecode_41(u8 *serialnum, u8 *data, int Len) {
	u16 tmp_u16, ArrayNum, ArrayLen;
	u8 index = 0;
	tmp_u16 = data[0] << 8;
	tmp_u16 |= data[1];
	ArrayNum = tmp_u16;
	tmp_u16 = data[3] << 8;
	tmp_u16 |= data[4];
	ArrayLen = tmp_u16;
	switch (ArrayNum)
	{
	case 0x0002:
		if (28 == ArrayLen) {
			memcpy(&parm_grp_0002.MainteTimeFromWeb_1, data + index, 6);
			index += 6;
			memcpy(&parm_grp_0002.MainteTimeFromWeb_2, data + index, 6);
			index += 6;
			memcpy(&parm_grp_0002.MainteTimeFromWeb_3, data + index, 6);
			index += 6;
			memcpy(&parm_grp_0002.MainteTimeFromWeb_4, data + index, 6);
			index += 6;
			memcpy(&parm_grp_0002.MainteTimeFromWeb_5, data + index, 4); //4个字节
			index += 4;
		} else { //TODO 信息长度错误处理

		}
		break;
	case 0x0003:
		if (28 == ArrayLen) {
			memcpy(&parm_grp_0003.MainteTimeFromWeb_1, data + index, 6);
			index += 6;
			memcpy(&parm_grp_0003.MainteTimeFromWeb_2, data + index, 6);
			index += 6;
			memcpy(&parm_grp_0003.MainteTimeFromWeb_3, data + index, 6);
			index += 6;
			memcpy(&parm_grp_0003.MainteTimeFromWeb_4, data + index, 6);
			index += 6;
			memcpy(&parm_grp_0003.MainteTimeFromWeb_5, data + index, 4);
			index += 4;
		} else { //TODO 信息长度错误处理

		}
		break;
	case 0x0006:
		if (4 == ArrayLen) {
			memcpy(&parm_grp_0006.ParameterConfig, data + index, 3);
			index += 3;
			memcpy(&parm_grp_0006.response_type, data + index, 1);
			index += 1;
			//TODO setdata
		} else {
			//TODO 信息长度错误处理
		}
		break;
	case 0x0008:
		if (2 == ArrayNum) {
			memcpy(&parm_grp_0008.Machine_Reset_Request, data + index, 1);
			index += 1;
			memcpy(&parm_grp_0008.response_type, data + index, 1);
			index += 1;
			//TODO set data
		} else {
			//TODO 信息长度错误处理
		}
		break;
	case 0x0009:
		if (5 == ArrayNum) {
			memcpy(&parm_grp_0009.EngineDeviceTestRequest, data + index, 3);
			index += 3;
			memcpy(&parm_grp_0009.response_type, data + index, 1);
			index += 1;
			//TODO set data
		} else {
			//TODO 信息长度错误处理
		}
		break;
	default:
		//TODO 信息错误处理
		break;
	}
	//x41 reply 0x42
	MsgMake_42(serialnum, ArrayNum, 0x41);
}
/*
 0x42 远程操作设置回复信息
 */
QUE_TDF_QUEUE_MSG Msg_42;
void MsgMake_42(u8 *serialnum, u16 ArrayNum, u8 ReplyType) {
	memset(&Msg_42, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	//信息类型
	Msg_42.MsgType = 0x42;
	//版本号
	cpyVersion(Msg_42.Version);
	//信息生成时间
	cpyInfoTime(Msg_42.Time);
	//属性标识  高字节在前
	Msg_42.Attribute[0] = 0x40;
	Msg_42.Attribute[1] = 0;

	Msg_42.data[index++] = serialnum[0];
	Msg_42.data[index++] = serialnum[1];
	Msg_42.data[index++] = serialnum[2];
	Msg_42.data[index++] = ReplyType;
	switch (ArrayNum)
	{
	case 0x0002:
		memcpy(Msg_42.data + index, parm_grp_0002.MainteTimeFromWeb_1, 6);
		index += 6;
		memcpy(Msg_42.data + index, parm_grp_0002.MainteTimeFromWeb_2, 6);
		index += 6;
		memcpy(Msg_42.data + index, parm_grp_0002.MainteTimeFromWeb_3, 6);
		index += 6;
		memcpy(Msg_42.data + index, parm_grp_0002.MainteTimeFromWeb_4, 6);
		index += 6;
		memcpy(Msg_42.data + index, parm_grp_0002.MainteTimeFromWeb_5, 4); //4个字节
		index += 4;
		break;
	case 0x0003:
		memcpy(Msg_42.data + index, &parm_grp_0003.MainteTimeFromWeb_1, 6);
		index += 6;
		memcpy(Msg_42.data + index, &parm_grp_0003.MainteTimeFromWeb_2, 6);
		index += 6;
		memcpy(Msg_42.data + index, &parm_grp_0003.MainteTimeFromWeb_3, 6);
		index += 6;
		memcpy(Msg_42.data + index, &parm_grp_0003.MainteTimeFromWeb_4, 6);
		index += 6;
		memcpy(Msg_42.data + index, &parm_grp_0003.MainteTimeFromWeb_5, 4);
		index += 4;
		break;
	case 0x0006:
		memcpy(Msg_42.data + index, &parm_grp_0006.ParameterConfig, 3);
		index += 3;
		memcpy(Msg_42.data + index, &parm_grp_0006.response_type, 1);
		index += 1;
		break;
	case 0x0008:
		memcpy(Msg_42.data + index, &parm_grp_0008.Machine_Reset_Request, 1);
		index += 1;
		memcpy(Msg_42.data + index, &parm_grp_0008.response_type, 1);
		index += 1;
		break;
	case 0x0009:
		memcpy(Msg_42.data + index, &parm_grp_0009.EngineDeviceTestRequest, 3);
		index += 3;
		memcpy(Msg_42.data + index, &parm_grp_0009.response_type, 1);
		index += 1;
		break;
	default:
		break;
	}

	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_42.length, strlen((char*) len));
	//信息入队列
	printf_msg("<Make_42_message>message ready in queue %d bytes!!! \n", index + 16);
	InterSta_in(&Msg_42);
}
/*
 0x43 回叫指定参数组信息
 */
void MsgDecode_43(u8 *serialnum, u8 *data) {

}
/*
 0x44 回叫指定参数组回复信息
 */
QUE_TDF_QUEUE_MSG Msg_44;
void MsgMake_44(u8 *serialnum, u8 *data) {
	memset(&Msg_44, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	//信息类型
	Msg_44.MsgType = 0x44;
	//版本号
	cpyVersion(Msg_44.Version);
	//信息生成时间
	cpyInfoTime(Msg_44.Time);
	//属性标识  高字节在前
	Msg_44.Attribute[0] = 0x40;
	Msg_44.Attribute[1] = 0;

	Msg_44.data[index++] = serialnum[0];
	Msg_44.data[index++] = serialnum[1];
	Msg_44.data[index++] = serialnum[2];
	Msg_44.data[index++] = 0x43;

	u8 j = 0;
	u8 PramCount = data[j++];
	Msg_44.data[index++] = PramCount;
	u8 i;
	u16 tmp_u16 = 0;
	for (i = 0; i < PramCount; i++) {
		Msg_44.data[index++] = data[j]; //参数组编号2byte
		tmp_u16 = data[j++] << 8;
		Msg_44.data[index++] = data[j];
		tmp_u16 |= data[j++];
		switch (tmp_u16)
		{
		case 0x0002: {
			tmp_u16 = sizeof(sumitomo_parm_grp_0002_t); //参数组长度2byte
			Msg_44.data[index++] = tmp_u16 >> 8;
			Msg_44.data[index++] = tmp_u16;
			memcpy(Msg_44.data + index, &parm_grp_0002, tmp_u16);
		}
			break;
		case 0x0003: {
			tmp_u16 = sizeof(sumitomo_parm_grp_0003_t); //参数组长度2byte
			Msg_44.data[index++] = tmp_u16 >> 8;
			Msg_44.data[index++] = tmp_u16;
			memcpy(Msg_44.data + index, &parm_grp_0003, tmp_u16);
		}
			break;
		case 0x0004: {
			tmp_u16 = sizeof(sumitomo_parm_grp_0004_t); //参数组长度2byte
			Msg_44.data[index++] = tmp_u16 >> 8;
			Msg_44.data[index++] = tmp_u16;
			memcpy(Msg_44.data + index, &parm_grp_0004, tmp_u16);
		}
			break;
		case 0x0005: {
			tmp_u16 = sizeof(sumitomo_parm_grp_0005_t); //参数组长度2byte
			Msg_44.data[index++] = tmp_u16 >> 8;
			Msg_44.data[index++] = tmp_u16;
			memcpy(Msg_44.data + index, &parm_grp_0005, tmp_u16);
		}
			break;
		case 0x0007: {
			tmp_u16 = sizeof(sumitomo_parm_grp_0007_t); //参数组长度2byte
			Msg_44.data[index++] = tmp_u16 >> 8;
			Msg_44.data[index++] = tmp_u16;
			memcpy(Msg_44.data + index, &parm_grp_0007, tmp_u16);
		}
			break;
		default:
			break;
		}
	}

	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_44.length, strlen((char*) len));
	//信息入队列
	printf_msg("<Make_44_message>message ready in queue %d bytes!!! \n", index + 16);
	InterSta_in(&Msg_44);
}
/*
 0x45 设置/回叫产权显示状态信息
 */
void MsgDecode_45(u8 *serialnum, u8 *data) {

}
/*
 0x46 设置/回叫产权显示状态回复信息
 */
QUE_TDF_QUEUE_MSG Msg_46;
void MsgDecode_46(u8 *serialnum, u8 *data) {
	memset(&Msg_46, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	//信息类型
	Msg_46.MsgType = 0x46;
	//版本号
	cpyVersion(Msg_46.Version);
	//信息生成时间
	cpyInfoTime(Msg_46.Time);
	//属性标识  高字节在前
	Msg_46.Attribute[0] = 0x40;
	Msg_46.Attribute[1] = 0;

	Msg_46.data[index++] = serialnum[0];
	Msg_46.data[index++] = serialnum[1];
	Msg_46.data[index++] = serialnum[2];
	Msg_46.data[index++] = 0x45;
//TODO 产权显示状态
	//Msg_46.data[index++] =
	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_46.length, strlen((char*) len));
	//信息入队列
	printf_msg("<Make_46_message>message ready in queue %d bytes!!! \n", index + 16);
	InterSta_in(&Msg_46);
}

//------------------------------------------------实时类-----------------------------------------------//
QUE_TDF_QUEUE_MSG Msg_RealTime;
/*
 0x10 故障信息
 */
void MsgDecode_10() {
	memset(&Msg_RealTime, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	//信息类型
	Msg_RealTime.MsgType = 0x10;
	//版本号
	cpyVersion(Msg_RealTime.Version);
	//信息生成时间
	cpyInfoTime(Msg_RealTime.Time);
	//属性标识  高字节在前 ?TODO 是否需要回执通用应答？
	Msg_RealTime.Attribute[0] = 0x40;
	Msg_RealTime.Attribute[1] = 0;

	gps_format_jwd_mem(Msg_RealTime.data + index); //定位数据21byte
	index += 21;
//TODO 发动机工作时间3byte
//TODO 故障数据时间 4byte
//TODO 故障信息个数 1byte
//TODO 故障内容 3*N byte
	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_RealTime.length, strlen((char*) len));
	//信息入队列
	printf_msg("<Make_10_message>message ready in queue %d bytes!!! \n", index + 16);
	InterSta_in(&Msg_RealTime);
}
/*
 0x12 防盜動作通知信息
 */
void MsgDecode_12() {
	memset(&Msg_RealTime, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	//信息类型
	Msg_RealTime.MsgType = 0x12;
	//版本号
	cpyVersion(Msg_RealTime.Version);
	//信息生成时间
	cpyInfoTime(Msg_RealTime.Time);
	//属性标识  高字节在前 ?TODO 是否需要回执通用应答？
	Msg_RealTime.Attribute[0] = 0x40;
	Msg_RealTime.Attribute[1] = 0;

	gps_format_jwd_mem(Msg_RealTime.data + index); //定位数据21byte
	index += 21;
//TODO hour meter
//TODO Machine Lock

	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_RealTime.length, strlen((char*) len));
	//信息入队列
	printf_msg("<Make_12_message>message ready in queue %d bytes!!! \n", index + 16);
	InterSta_in(&Msg_RealTime);
}
/*
 0x14 维修通知信息
 当机种、发送地、发动机编号、ECM部件编号、控制器A部件编号、控制器T部件编号发生变化时，立即形成该信息
 */
void MsgDecode_14() {
	memset(&Msg_RealTime, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	//信息类型
	Msg_RealTime.MsgType = 0x14;
	//版本号
	cpyVersion(Msg_RealTime.Version);
	//信息生成时间
	cpyInfoTime(Msg_RealTime.Time);
	//属性标识  高字节在前 ?TODO 是否需要回执通用应答？
	Msg_RealTime.Attribute[0] = 0x40;
	Msg_RealTime.Attribute[1] = 0;

	Msg_RealTime.data[index++] = 0x00;
	Msg_RealTime.data[index++] = 0x04;
	u16 tmp_u16 = 0;
	tmp_u16 = sizeof(sumitomo_parm_grp_0004_t);
	Msg_RealTime.data[index++] = tmp_u16 >> 8;
	Msg_RealTime.data[index++] = tmp_u16;
	memcpy(Msg_RealTime.data + index, &parm_grp_0004, tmp_u16);
	index += tmp_u16;

	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_RealTime.length, strlen((char*) len));
	//信息入队列
	printf_msg("<Make_14_message>message ready in queue %d bytes!!! \n", index + 16);
	InterSta_in(&Msg_RealTime);
}
/*
 0x16 燃油余量通知信息
 当然后柱状表显示剩余2格时，立即形成该信息
 关于再次发送：燃油柱状表到4格以上后，再降到2格时，实施再次形成该信息
 */
void MsgDecode_16() {
	memset(&Msg_RealTime, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	//信息类型
	Msg_RealTime.MsgType = 0x16;
	//版本号
	cpyVersion(Msg_RealTime.Version);
	//信息生成时间
	cpyInfoTime(Msg_RealTime.Time);
	//属性标识  高字节在前 ?TODO 是否需要回执通用应答？
	Msg_RealTime.Attribute[0] = 0x40;
	Msg_RealTime.Attribute[1] = 0;

	gps_format_jwd_mem(Msg_RealTime.data + index); //定位数据21byte
	index += 21;
//TODO hour meter
//TODO 燃油位

	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_RealTime.length, strlen((char*) len));
	//信息入队列
	printf_msg("<Make_16_message>message ready in queue %d bytes!!! \n", index + 16);
	InterSta_in(&Msg_RealTime);
}
/*
 0x1A 全复位通知信息

 */
void MsgDecode_1A() {
	memset(&Msg_RealTime, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	//信息类型
	Msg_RealTime.MsgType = 0x1A;
	//版本号
	cpyVersion(Msg_RealTime.Version);
	//信息生成时间
	cpyInfoTime(Msg_RealTime.Time);
	//属性标识  高字节在前 ?TODO 是否需要回执通用应答？
	Msg_RealTime.Attribute[0] = 0x40;
	Msg_RealTime.Attribute[1] = 0;

	gps_format_jwd_mem(Msg_RealTime.data + index); //定位数据21byte
	index += 21;
//TODO hour meter
//TODO 全复位类型

	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_RealTime.length, strlen((char*) len));
	//信息入队列
	printf_msg("<Make_1A_message>message ready in queue %d bytes!!! \n", index + 16);
	InterSta_in(&Msg_RealTime);
}
/*
 0x1C 初期配对完成信息
 当Controller A与Controller T初次匹配成功时，立即发起该信息
 */
void MsgDecode_1C() {
	memset(&Msg_RealTime, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	//信息类型
	Msg_RealTime.MsgType = 0x1C;
	//版本号
	cpyVersion(Msg_RealTime.Version);
	//信息生成时间
	cpyInfoTime(Msg_RealTime.Time);
	//属性标识  高字节在前 ?TODO 是否需要回执通用应答？
	Msg_RealTime.Attribute[0] = 0x40;
	Msg_RealTime.Attribute[1] = 0;

	gps_format_jwd_mem(Msg_RealTime.data + index); //定位数据21byte
	index += 21;
//TODO hour meter
//TODO 匹配码

	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_RealTime.length, strlen((char*) len));
	//信息入队列
	printf_msg("<Make_1C_message>message ready in queue %d bytes!!! \n", index + 16);
	InterSta_in(&Msg_RealTime);
}
/*
 0x1E 配对未通过通知信息
 当Controller A与Controller T匹配未通过时，立即发起该信息
 */
void MsgDecode_1E() {
	memset(&Msg_RealTime, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	//信息类型
	Msg_RealTime.MsgType = 0x1E;
	//版本号
	cpyVersion(Msg_RealTime.Version);
	//信息生成时间
	cpyInfoTime(Msg_RealTime.Time);
	//属性标识  高字节在前 ?TODO 是否需要回执通用应答？
	Msg_RealTime.Attribute[0] = 0x40;
	Msg_RealTime.Attribute[1] = 0;

	gps_format_jwd_mem(Msg_RealTime.data + index); //定位数据21byte
	index += 21;
//TODO hour meter 3 byte
//TODO 配对未通过状态 1byte
//TODO Ctrl T匹配码 4byte
//TODO Ctrl A匹配码4byte

	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_RealTime.length, strlen((char*) len));
	//信息入队列
	printf_msg("<Make_1E_message>message ready in queue %d bytes!!! \n", index + 16);
	InterSta_in(&Msg_RealTime);
}
/*
 0xA0 配对未通过通知信息
 */
void MsgDecode_A0() {
	memset(&Msg_RealTime, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	//信息类型
	Msg_RealTime.MsgType = 0xA0;
	//版本号
	cpyVersion(Msg_RealTime.Version);
	//信息生成时间
	cpyInfoTime(Msg_RealTime.Time);
	//属性标识  高字节在前 ?TODO 是否需要回执通用应答？
	Msg_RealTime.Attribute[0] = 0x40;
	Msg_RealTime.Attribute[1] = 0;

	gps_format_jwd_mem(Msg_RealTime.data + index); //定位数据21byte
	index += 21;
//TODO hour meter 3 byte
//TODO 保养通知类型/保养对象配件类型 1byte
//TODO 保养对象配件到剩余时间2byte
//TODO 保养对象配件的保养间隔2byte

	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_RealTime.length, strlen((char*) len));
	//信息入队列
	printf_msg("<Make_A0_message>message ready in queue %d bytes!!! \n", index + 16);
	InterSta_in(&Msg_RealTime);
}
/*
 0xA2 机器touch通知信息
 */
void MsgDecode_A2() {
	memset(&Msg_RealTime, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	//信息类型
	Msg_RealTime.MsgType = 0xA2;
	//版本号
	cpyVersion(Msg_RealTime.Version);
	//信息生成时间
	cpyInfoTime(Msg_RealTime.Time);
	//属性标识  高字节在前 ?TODO 是否需要回执通用应答？
	Msg_RealTime.Attribute[0] = 0x40;
	Msg_RealTime.Attribute[1] = 0;

	gps_format_jwd_mem(Msg_RealTime.data + index); //定位数据21byte
	index += 21;
//TODO hour meter 3 byte

	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_RealTime.length, strlen((char*) len));
	//信息入队列
	printf_msg("<Make_A2_message>message ready in queue %d bytes!!! \n", index + 16);
	InterSta_in(&Msg_RealTime);
}
/*
 0xA4 ATS自動再生間隔通知信息
 */
void MsgDecode_A4() {
	memset(&Msg_RealTime, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	//信息类型
	Msg_RealTime.MsgType = 0xA4;
	//版本号
	cpyVersion(Msg_RealTime.Version);
	//信息生成时间
	cpyInfoTime(Msg_RealTime.Time);
	//属性标识  高字节在前 ?TODO 是否需要回执通用应答？
	Msg_RealTime.Attribute[0] = 0x40;
	Msg_RealTime.Attribute[1] = 0;

	gps_format_jwd_mem(Msg_RealTime.data + index); //定位数据21byte
	index += 21;
//TODO ATS再生结束 Hour Meter 3 byte
//TODO ATS再生结束种类 1byte
//TODO ATS再生开始 Hour Meter 3byte
//TODO ATS再生开始种类 1byte
//TODO ATS再生間隔時間 2byte

	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_RealTime.length, strlen((char*) len));
	//信息入队列
	printf_msg("<Make_A4_message>message ready in queue %d bytes!!! \n", index + 16);
	InterSta_in(&Msg_RealTime);
}
/*
 0xA6 DOC后温度升温时间通知信息
 */
void MsgDecode_A6() {
	memset(&Msg_RealTime, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	//信息类型
	Msg_RealTime.MsgType = 0xA6;
	//版本号
	cpyVersion(Msg_RealTime.Version);
	//信息生成时间
	cpyInfoTime(Msg_RealTime.Time);
	//属性标识  高字节在前 ?TODO 是否需要回执通用应答？
	Msg_RealTime.Attribute[0] = 0x40;
	Msg_RealTime.Attribute[1] = 0;

	gps_format_jwd_mem(Msg_RealTime.data + index); //定位数据21byte
	index += 21;
//TODO Hour Meter 3 byte
//TODO 进气温度 1byte
//TODO 升温时间 2byte

	//信息长度
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_RealTime.length, strlen((char*) len));
	//信息入队列
	printf_msg("<Make_A6_message>message ready in queue %d bytes!!! \n", index + 16);
	InterSta_in(&Msg_RealTime);
}

//----------------------------------------------------透传协议数据类信息-------------------------------------------------//
TDF_PGN_SET hand_pgn_set[MSG_PGN_SET_MAX]; //0XC5
TDF_PGN_SET hand_BTpgn_set[MSG_PGN_SET_MAX]; //BT

u8 dtu_tx_set_num = 0; // 发送单元设置个数
u8 msg_pgn_set_num = 0; //PGN参数ID个数b
u8 msg_pgn_BTset_num = 0; //BTPGN参数ID个数b

u8 msg_event_set_num = 0; //事件设置个数
u8 msg_event_trans_set_num = 0; //事件透传设置PGN个数
TDF_EVENT_SET msg_event_set[MSG_EVENT_SET_MAX]; //【0xB5】事件设置信息

u32 MSG_TIMER_01, MSG_TIMER_KEYOFF, MSG_PGN_QUE_TIMER, MSG_TIMER_E2_1, MSG_TIMER_E2_2;
u32 msg_event_timer[MSG_EVENT_SET_MAX]; //事件实际发生持续时间
u8 msg_event_make_flag[MSG_EVENT_SET_MAX]; //事件信息生成标志位 1已生成 0未生成 事件不满足条件时清0
u8 msg_event_make_num; //事件发生个数标志位 有事件发生此标志位+1 发送完事件透传信息后清0
u32 cand_pgn_cyc_get_timer[MSG_PGN_SET_MAX]; // 和数据pgnset逐一对应
TDF_BAGS_NUM cand_pgn_bags_num[MSG_PGN_SET_MAX];
TDF_PGN_SET cand_event_set[MSG_EVENT_TRANS_SET_MAX]; //【0xB9】事件透传设置信息

DIAGNOSIS_SET diagnosis_set; //实时诊断透传时间设置

u8 cand_pgn_event_flag = 0xff;
u8 cand_pgn_event_can1[600][16];
u16 cand_pgn_event_index1;
u16 cand_pgn_event_num1;
u8 cand_pgn_event_can2[600][16]; // 事件开始后接收缓存
u16 cand_pgn_event_index2;
u16 cand_pgn_event_num2;
u8 cand_pgn_event_can3[600][16]; // 未采集完成前，又发生新事件，缓存地址
u16 cand_pgn_event_index3;
u16 cand_pgn_event_num3;

//拐点判断
GPSCanMsg GPScache[3];
GPS_struct GPS_Now; //当前GPS 点信息
GPS_struct GPS_Last; //上一个GPS 点信息

u8 GPS_inflect_flag = 0; //是否时拐点，0：不是；1：是
u8 IsGPSCanMsg = 0; //是否时GPS 点，0不是，1是，用于缓存GPS 点信息
u8 IsGPSSendFlag = 0; //0不发，1，发送
u16 gpsd_turn_speed_min = 10; // 转弯判断最小车速 10km/hr
u16 gpsd_turn_direct_min = 15; // 转弯判断最小变化角度10度

//--------------------------------------------------------------------------
/**
 * @brief  读取指定格式数据
 * @param  buf :写入缓冲区
 * @param  data:数据
 * @param  type:类型，1：8位，2：16位，3：24位，4：32位
 * @retval 结果
 */
u32 Sys_Read_Buffer(u8 *buf, u8 type) {
	u32 a = 0;

	if (type == 1) {
		a = buf[0];
	} else if (type == 2) {
		a = buf[0];
		a = a << 8 | buf[1];
	} else if (type == 3) {
		a = buf[0];
		a = a << 8 | buf[1];
		a = a << 8 | buf[2];
	} else if (type == 4) {
		a = buf[0];
		a = a << 8 | buf[1];
		a = a << 8 | buf[2];
		a = a << 8 | buf[3];
	}
	return a;
}
/**
 * @brief  读取指定格式数据
 * @param  buf :写入缓冲区
 * @param  data:数据
 * @param  type:类型，1：8位，2：16位，3：24位，4：32位
 * @retval 结果
 */
u32 Can_Read_Buffer(u8 *buf, u8 type) {
	u32 a = 0;

	if (type == 1) {
		a = buf[0];
	} else if (type == 2) {
		a = buf[1];
		a = a << 8 | buf[0];
	} else if (type == 3) {
		a = buf[2];
		a = a << 8 | buf[1];
		a = a << 8 | buf[0];
	} else if (type == 4) {
		a = buf[3];
		a = a << 8 | buf[2];
		a = a << 8 | buf[1];
		a = a << 8 | buf[0];
	}
	return a;
}

/*
 * 【0xB4】数据收集数据汇报
 * */
void MsgMake_B4()
{

}

/**
 * data 数据
 * len 数据长度
 * isWebData 数据来源，1网络，0本地。来自网络，会执行将data串保存到本地,
 * 	保存到本地的经过拼串，包含所有当前内存中D5设置参数
 * 返回值 事件设置个数
 *
 */
 int MsgDecode_B5(u8 *data, u32 len, u8 isWebData) {
	//printfHexData(data, len);
	u8 i, n, num, state, number;
	int index;
	number = 0;
	index = 0;

	n = data[index++];
	if (n > 0 && n <= MSG_EVENT_SET_MAX) {
		for (i = 0; i < n; i++) {
			state = data[index] >> 7;
			num = data[index] & 0x7F;
			num = num - 1;
			if (state == 1) { //设置相应位置
				*(u8 *) &msg_event_set[num].event_state = data[index++];
				msg_event_set[num].event_type = data[index++];

				if (msg_event_set[num].event_type == 0) {
//					*(u32 *) &msg_event_set[num].pgn_id = bytes_dword(data + index);
//					index += 4;
					cpymem(&(msg_event_set[num].pgn_id), data, 4, &index);
					*(u8 *) &msg_event_set[num].attribute = data[index++];
					msg_event_set[num].param_set = bytes_dword(data + index); //参数属性
					index += 4;
					msg_event_set[num].cyc_get = bytes_dword(data + index); //信息时间
					index += 4;
					if (msg_event_set[num].pgn_id.da_id == 1) {
						msg_event_set[num].pgn_id_da.id_len = (data[index] & 0xE0) >> 5;
						msg_event_set[num].pgn_id_da.id_start = (data[index++] & 0x1C) >> 2;
						bytes_cpy(msg_event_set[num].pgn_id_da.id_da, data + index, 7);
						index += 7;
					}
				} else if (msg_event_set[num].event_type == 1) {
//					*(u32 *) &msg_event_set[num].pgn_id = bytes_dword(data + index);
//					index += 4;
					cpymem(&(msg_event_set[num].pgn_id), data, 4, &index);
					*(u8 *) &msg_event_set[num].attribute = data[index++];
					msg_event_set[num].cyc_get = bytes_dword(data + index);
					index += 4;
					if (msg_event_set[num].pgn_id.da_id == 1) {
						msg_event_set[num].pgn_id_da.id_len = (data[index] & 0xE0) >> 5;
						msg_event_set[num].pgn_id_da.id_start = (data[index++] & 0x1C) >> 2;
						bytes_cpy(msg_event_set[num].pgn_id_da.id_da, data + index, 7);
						index += 7;
					}
				}
				number++;
			} else { //清除相应位置 数据
				memset(&(msg_event_set[num]), 0x00, sizeof(TDF_EVENT_SET));
				number--;
			}
		}
		msg_event_set_num = number;
		if (isWebData) //来自网络的数据，保存到本地
		{
			save2flash(B5_file, data, len);
		}
	} else if (n == 0) { //表示回叫当前终端事件设置
	} else if (n == 0xFF) {
		msg_event_set_num = 0;
		memset(msg_event_set, 0x00, sizeof(msg_event_set));
		if (isWebData) //来自网络的数据，保存到本地
		{
			save2flash(B5_file, data, len);
		}
	}

	return n;
}
/*
 * 	事件设置回复信息
 * 	num:事件设置个数
 * 	无返回值
 */
QUE_TDF_QUEUE_MSG Msg_B6;
void MsgMake_B6(u8 num) {
	int index = 0;
	int i;
	memset(&Msg_B6, 0, sizeof(QUE_TDF_QUEUE_MSG));

	//信息类型
	Msg_B6.MsgType = 0xB6;
	//版本号
	cpyVersion(Msg_B6.Version);
	//信息生成时间
	cpyInfoTime(Msg_B6.Time);
	//属性标识  高字节在前
	Msg_B6.Attribute[0] = 0x40;
	Msg_B6.Attribute[1] = 0;

	Msg_B6.data[index++] = msg_event_set_num;
	if ((msg_event_set_num >= 0) && (msg_event_set_num <= MSG_PGN_SET_MAX)) {
		for (i = 0; i < msg_event_set_num; i++) {
			if (*(u8 *) &msg_event_set[i].event_state != 0) {
				Msg_B6.data[index++] = *(u8 *) &msg_event_set[i].event_state;
				if (msg_event_set[i].event_state.state == 1) {
					Msg_B6.data[index++] = msg_event_set[i].event_type;
					if (msg_event_set[i].event_type == 0) { //PGN参数阀值事件
						copyobj(Msg_B6.data + index, &(msg_event_set[i].pgn_id), 4, &index); //加入PGN ID
						memcpy(Msg_B6.data + index, &(msg_event_set[i].attribute), 1); //加入属性
						index += 1;
						copyobj(Msg_B6.data + index, &(msg_event_set[i].param_set), 4, &index); //加入参数设定
						copyobj(Msg_B6.data + index, &(msg_event_set[i].cyc_get), 4, &index); //加入事件发生时间
						if (msg_event_set[i].pgn_id.da_id == 1) { //有数据ID
							memcpy(Msg_B6.data + index, &(msg_event_set[i].pgn_id_da), 1);
							index += 1;
							memcpy(Msg_B6.data + index, msg_event_set[i].pgn_id_da.id_da, 7);
							index += 7;
						}
					} else if (msg_event_set[i].event_type == 1) { //PGN参数开关事件
						copyobj(Msg_B6.data + index, &(msg_event_set[i].pgn_id), 4, &index); //加入PGN ID
						memcpy(Msg_B6.data + index, &(msg_event_set[i].attribute), 1); //加入属性
						index += 1;
						copyobj(Msg_B6.data + index, &(msg_event_set[i].cyc_get), 4, &index); //加入事件发生时间
						if (msg_event_set[i].pgn_id.da_id == 1) { //有数据ID
							memcpy(Msg_B6.data + index, &(msg_event_set[i].pgn_id_da), 1);
							index += 1;
							memcpy(Msg_B6.data + index, msg_event_set[i].pgn_id_da.id_da, 7);
							index += 7;
						}
					}
				}
			}
		}
	}
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_B6.length, strlen((char*) len));
	printf_msg("<MsgMake_B6>message ready in queue %d bytes!!! \n", index + 16);
	PassThrough_in(&Msg_B6); //信息入队列
//	if (debug_value & 0x80) {
//		printfHexData(Message.data, index);
//	}
}

/*
 * 功能：检测事件发生
 */
int Msg_Event_Check(CanMsg candata) {
	u8 i, a, b, c, d, index;
	u32 threshold[MSG_EVENT_SET_MAX];
	u8 onoff = 0xFF;
	c = 0;
	d = 0;
//printf("Msg_Event_Check:%X,",candata.ID);Fun_Puth("",candata.DATA,8);
//Fun_Puth("",(u8 *)&msg_event_set,(5*sizeof(TDF_EVENT_SET)));
	for (i = 0; i < MSG_EVENT_SET_MAX; i++) {
		if (msg_event_set[i].event_state.state == 1) {
			if (msg_event_set[i].event_type == 0) { //PGN参数阀值事件
				if (candata.ide == msg_event_set[i].pgn_id.ide) { //PGN ID的帧格式*/
					if (candata.ID == (u32) msg_event_set[i].pgn_id.id) {
						if (msg_event_set[i].pgn_id.da_id == 0) { //无数据ID
							a = msg_event_set[i].attribute.b2_0; //参数在PGN中所处的起始位置，取值范围[0-7]
							if (msg_event_set[i].attribute.b4 == 1) { //高字节在前
								threshold[i] = Sys_Read_Buffer((u8 *) &candata.DATA[a], msg_event_set[i].attribute.b7_5);
								d = 1;
//								printfHexData(temp, 4);
							} else {
								threshold[i] = Can_Read_Buffer((u8 *) &candata.DATA[a], msg_event_set[i].attribute.b7_5);
								d = 1;
//								printfHexData(temp, 4);
							}
						} else //有数据ID
						{
							for (a = msg_event_set[i].pgn_id_da.id_start, b = 0; a < msg_event_set[i].pgn_id_da.id_len; a++) {
								if (candata.DATA[a] != msg_event_set[i].pgn_id_da.id_da[b++]) {
									printf_msg("Msg_Event_Check1:da_id.id_da unequal!\n");
									c = 0xFF;
									break;
								}
							}
							if (c != 0xFF) { //数据ID相等
								a = msg_event_set[i].attribute.b2_0;
								if (msg_event_set[i].attribute.b4 == 1) { //高字节在前
									threshold[i] = Sys_Read_Buffer((u8 *) &candata.DATA[a], msg_event_set[i].attribute.b7_5);
									d = 1;
//									printfHexData(temp, 4);
								} else {
									threshold[i] = Can_Read_Buffer((u8 *) &candata.DATA[a], msg_event_set[i].attribute.b7_5);
									d = 1;
//									puts("\n");
								}
							}
						}
					}
				}
				//事件发生条件判断
				if (d == 1) {
					//  当参数大于“参数设定”时事件发生条件满足
					if (msg_event_set[i].attribute.b3 == 0) {
						if (threshold[i] > msg_event_set[i].param_set) {
							return i;
						} else {
							msg_event_make_flag[i] = 0;
							msg_event_timer[i] = api_GetSysmSecs(); //时间判定条件代明确
						}
					} else {
						//当参数小于“参数设置”时事件发生条件满足
						if (threshold[i] < msg_event_set[i].param_set) {
							return i;
						} else {
							msg_event_make_flag[i] = 0;
							msg_event_timer[i] = api_GetSysmSecs();
						}
					}
				}
			} else //PGN参数开关事件
			{
				if (candata.ide == msg_event_set[i].pgn_id.ide) { //PGN ID的帧格式*/
					if (candata.ID == (u32) msg_event_set[i].pgn_id.id) {
						if (msg_event_set[i].pgn_id.da_id == 0) { //无数据ID
							index = msg_event_set[i].attribute.b2_0;
							onoff = candata.DATA[index];
						} else //有数据ID
						{
							for (a = msg_event_set[i].pgn_id_da.id_start, b = 0; a < msg_event_set[i].pgn_id_da.id_len; a++) {
								if (candata.DATA[a] != msg_event_set[i].pgn_id_da.id_da[b++]) {
									printf_msg("Msg_Event_Check2:da_id.id_da unequal!\n");
									c = 0xFF;
									break;
								}
							}
							if (c != 0xFF) { //数据ID相等
								index = msg_event_set[i].attribute.b2_0;
								onoff = candata.DATA[index];
							}
						}
					}
				}
				//事件发生条件判断
				if (onoff == msg_event_set[i].attribute.b3) {
					return i;
				} else {
					msg_event_make_flag[i] = 0;
					msg_event_timer[i] = api_GetSysmSecs();
				}
			}
		}
	}
	return -1;
}

/*
 * 功能： 事件信息的生成
 * 参数：candata：CAN数据
 * 描述：can数据出can 队列 ，生成透传信息
 * 	1判断是否为需要采集的ID
 * 	2判断采集频率
 * 	3 判断采集长度及时长
 * 	4信息入队
 */
QUE_TDF_QUEUE_MSG Msg_B8;
void MsgMake_B8(CanMsg candata) {
	u16 index = 0;
	int n = -1;
	n = Msg_Event_Check(candata);
	//printf("n:%d,msg_event_make_flag[n]:%d\n",n,msg_event_make_flag[n]);
	if (msg_event_make_flag[n] == 1)
		return;
	if (n >= 0) {
		if (msg_event_timer[n] == 0 || msg_event_timer[n] > api_GetSysmSecs()) {
			msg_event_timer[n] = api_GetSysmSecs();
		}
		if (api_DiffSysMSecs(msg_event_timer[n]) > msg_event_set[n].cyc_get) {
			msg_event_make_flag[n] = 1;
			msg_event_make_num++;
//			msg_event_timer[n] = api_GetSysMSecs();
			memset(&Msg_B8, 0x00, sizeof(QUE_TDF_QUEUE_MSG));
			Msg_B8.MsgType = 0xB8;
			//版本号
			cpyVersion(Msg_B8.Version);
			//信息生成时间
			cpyInfoTime(Msg_B8.Time);
			//属性标识  高字节在前
			Msg_B8.Attribute[0] = 0x40;
			Msg_B8.Attribute[1] = 0;
			u32 SysRun_Time = api_GetSysSecs();
			Msg_B8.data[index++] = SysRun_Time >> 24;
			Msg_B8.data[index++] = SysRun_Time >> 16;
			Msg_B8.data[index++] = SysRun_Time >> 8;
			Msg_B8.data[index++] = SysRun_Time;
//			printf("event_type:%d\n", msg_event_set[n].event_type);
			if (msg_event_set[n].event_type == 0) { //PGN参数阀值事件
				memcpy(Msg_B8.data + index, &msg_event_set[msg_event_set[n].event_state.num - 1].pgn_id, 4);
				index += 4;
				Msg_B8.data[index++] = *(u8 *) &msg_event_set[msg_event_set[n].event_state.num - 1].attribute;
				memcpy(Msg_B8.data + index, &msg_event_set[msg_event_set[n].event_state.num - 1].param_set, 4);
				index += 4;
				memcpy(Msg_B8.data + index, &msg_event_set[msg_event_set[n].event_state.num - 1].cyc_get, 4);
				index += 4;
				if (msg_event_set[msg_event_set[n].event_state.num - 1].pgn_id.da_id == 1) { //有数据ID
					Msg_B8.data[index] = msg_event_set[msg_event_set[n].event_state.num - 1].pgn_id_da.id_len << 5;
					Msg_B8.data[index++] |= msg_event_set[msg_event_set[n].event_state.num - 1].pgn_id_da.id_start << 2;
					memcpy(Msg_B8.data + index, &msg_event_set[msg_event_set[n].event_state.num - 1].pgn_id_da.id_da, 7);
					index += 7;
				}
			} else if (msg_event_set[n].event_type == 1) { //PGN参数开关事件
				memcpy(Msg_B8.data + index, &msg_event_set[msg_event_set[n].event_state.num - 1].pgn_id, 4);
				index += 4;
				Msg_B8.data[index++] = *(u8 *) &msg_event_set[msg_event_set[n].event_state.num - 1].attribute;
				memcpy(Msg_B8.data + index, &msg_event_set[msg_event_set[n].event_state.num - 1].cyc_get, 4);
				index += 4;
				if (msg_event_set[msg_event_set[n].event_state.num - 1].pgn_id.da_id == 1) { //有数据ID
					Msg_B8.data[index] = msg_event_set[msg_event_set[n].event_state.num - 1].pgn_id_da.id_len << 5;
					Msg_B8.data[index++] |= msg_event_set[msg_event_set[n].event_state.num - 1].pgn_id_da.id_start << 2;
					memcpy(Msg_B8.data + index, &msg_event_set[msg_event_set[n].event_state.num - 1].pgn_id_da.id_da, 7);
					index += 7;
				}
			}
			//Message.length = index;
			//printfHexData(Message.data,index);
			u8 len[4] = { 0 };
			sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
			api_HexToAsc(len, Msg_B8.length, strlen((char*) len));
			//信息入队列
			printf_msg("<MsgMake_B8>message ready in queue!!! \n");
			PassThrough_in(&Msg_B8);
		}
	}
}

/**
 * data 数据
 * len 数据长度
 * isWebData 数据来源，1网络，0本地。来自网络，会执行将data串保存到本地,
 * 返回值 事件设置个数
 *
 */
int MsgDecode_B9(u8 *data, u32 len, u8 isWebData) {
	if (debug_value & 0x80) {
		api_PrintfHex(data, len);
	}
	int i;
	u8 n;
	int offest = 0;
//	memcpy(MSG_SetID, data, 3);
//	offest += 3; //信息对照码

	n = data[offest++];
	if (n == 0) { //回叫终端事件透传参数
		msg_event_trans_set_num = n;
	}
	if (n == 0xFF) { //清除事件参数
		memset(cand_event_set, 0x00, sizeof(cand_event_set));
		msg_event_trans_set_num = 0;
		if (isWebData) //来自网络的数据，保存到本地
		{
			save2flash(B9_file, data, len);
		}

	} else if ((n >= 1) && (n <= EVENT_PT_SET_MAX)) { //设置终端
		for (i = 0; i < n; i++) {

			cpymem(&(cand_event_set[i].mode), data, 1, &offest);

			cpymem(&(cand_event_set[i].pgn_id), data, 4, &offest);
			cpymem(&(cand_event_set[i].cyc_get), data, 3, &offest);

			if (cand_event_set[i].mode.bags) { //多包
				cpymem(&(cand_event_set[i].pgn_id0), data, 4, &offest);
				cpymem(&(cand_event_set[i].pgn_id1), data, 4, &offest);
			}
			if (cand_event_set[i].mode.da_id) { //有数据ID
				cpymem(&(cand_event_set[i].pgn_id_da), data, 1, &offest);
				memcpy(cand_event_set[i].pgn_id_da.id_da, data + offest, 7);
				offest += 7;
			}

		}
		msg_event_trans_set_num = n;
		if (isWebData) //来自网络的数据，保存到本地
		{
			save2flash(B9_file, data, len);
		}
	}
	return n;
}
/**
 * @brief  事件透传ID设置回复信息
 * @retval None
 */
QUE_TDF_QUEUE_MSG Msg_BA;
void MsgMake_BA(u8 *serialnum, u8 num) {
	int index = 0;
	int j;
	memset(&Msg_BA, 0, sizeof(QUE_TDF_QUEUE_MSG));

	Msg_BA.MsgType = 0xBA;
	//版本号
	cpyVersion(Msg_BA.Version);
	//信息生成时间
	cpyInfoTime(Msg_BA.Time);
	//属性标识  高字节在前
	Msg_BA.Attribute[0] = 0x40;
	Msg_BA.Attribute[1] = 0;

	Msg_BA.data[index++] = serialnum[0];
	Msg_BA.data[index++] = serialnum[1];
	Msg_BA.data[index++] = serialnum[2];
	Msg_BA.data[index++] = msg_event_trans_set_num;
	if (num >= 0 && num <= MSG_EVENT_TRANS_SET_MAX) {
		for (j = 0; j < msg_event_trans_set_num; j++) {
			Msg_BA.data[index++] = *(u8 *) &cand_event_set[j].mode;
			copyobj(Msg_BA.data + index, &(cand_event_set[j].pgn_id), 4, &index);
			copyobj(Msg_BA.data + index, &(cand_event_set[j].cyc_get), 3, &index);
//			 if(cand_event_set[j].mode.req == 1)	//请求  暂时不包含请求项
//			 {
//				 memcpy( Message.data+index,&cand_event_set[j].pgn_id_req,4);	index += 4;
//				 Message.data[index++] =  *(u8 *)&cand_event_set[j].].pgn_da_req.len;
//				 memcpy( Message.data+index,&cand_event_set[j].pgn_da_req.da,8);	index += 8;
//			 }
			if (cand_event_set[j].mode.bags == 1) { //多包
				copyobj(Msg_BA.data + index, &(cand_event_set[j].pgn_id0), 4, &index);
				copyobj(Msg_BA.data + index, &(cand_event_set[j].pgn_id1), 4, &index);
			}
			if (cand_event_set[j].mode.da_id == 1) { //有数据id
				copyobj(Msg_BA.data + index, &(cand_event_set[j].pgn_id_da), 1, &index);
				memcpy(Msg_BA.data + index, &cand_event_set[j].pgn_id_da, 7);
				index += 7;
			}
		}
	}
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_BA.length, strlen((char*) len));
	printf_msg("<MsgMake_BA>message ready in queue %d bytes!!! \n", index + 16);
	PassThrough_in(&Msg_BA); //信息入队列
	if (debug_value & 0x80) {
		api_PrintfHex(Msg_BA.data, index);
	}
	memset(&Msg_BA, 0, sizeof(QUE_TDF_QUEUE_MSG));
}

int8_t Msg_EventTrans_Check(CanMsg candata) {
	uint8_t i, a, b, c = 0;
	uint32_t id2;
//printf("Msg_Candata_Check:%X,",candata.ID);Fun_Puth("",candata.DATA,8);
//Fun_Puth("",(u8 *)&hand_pgn_set,(msg_pgn_set_num*sizeof(TDF_PGN_SET)));
	for (i = 0; i < msg_event_trans_set_num; i++) {
		if (i > MSG_EVENT_TRANS_SET_MAX)
			break;
		if (candata.SOURCE == cand_event_set[i].pgn_id.can) { //PGN参数来源
			if ((candata.ID & 0x00FFFFFF) == CAN_ID_PGN_GB) { //多包广播包
				id2 = candata.DATA[6];
				id2 = id2 << 8 | candata.DATA[5];
				id2 = id2 << 8 | candata.DATA[7];
				if (id2 == cand_event_set[i].pgn_id.id) {
					cand_event_bags_num[i].total_num = candata.DATA[2];
					cand_event_bags_num[i].total_num = cand_event_bags_num[i].total_num << 8 | candata.DATA[1];
					cand_event_bags_num[i].today_num = 1;
					c = 1;
//					printf("Msg_EventTrans_Check:PGN_GB\n");
				}
			} else if ((candata.ID & 0x00FFFFFF) == CAN_ID_PGN_DATA) { //多包数据包
				if ((candata.DATA[0] >= cand_event_bags_num[i].today_num) && (candata.DATA[0] <= cand_event_bags_num[i].total_num)) {
					return i;
				} else {
					cand_event_bags_num[i].total_num = 0;
					cand_event_bags_num[i].today_num = 0;
				}
			}
			if (candata.ide == cand_event_set[i].mode.ide) { //PGN ID的帧格式*/
				if (candata.ID == cand_event_set[i].pgn_id.id) {
					if (cand_event_set[i].mode.da_id == 1) { //	设置有数据ID
						for (a = cand_event_set[i].pgn_id_da.id_start, b = 0; a < cand_event_set[i].pgn_id_da.id_len; a++) {
							if (candata.DATA[a] != cand_event_set[i].pgn_id_da.id_da[b++]) {
								printf_msg("Msg_EventTrans_Check:da_id.id_da unequal!\n");
								c = 0xFF;
								break;
							}
						}
						if (c != 0xFF) {
							c = 1;
						}
					} else {
						c = 1;
					}
				}
			}
			if (c == 1) {
				c = 0;
				if (cand_event_set[i].cyc_get != 0) {
					if (cand_event_cyc_get_timer[i] == 0 || cand_event_cyc_get_timer[i] > api_GetSysmSecs())
						cand_event_cyc_get_timer[i] = api_GetSysmSecs();
					if (api_DiffSysMSecs(cand_event_cyc_get_timer[i]) > cand_event_set[i].cyc_get) {
						cand_event_cyc_get_timer[i] = api_GetSysmSecs();
						return i;
					}
				} else {
					return i;
				}
			}
		}
	}
	return -1;
}
/**
 * @brief  事件透传信息生成
 * @param  candata：CAN数据
 * @retval None
 */
static u16 msg_e2_index = 0;
static u16 msg_e3_index = 0;
QUE_TDF_QUEUE_MSG Msg_BC;
void MsgMake_BC(CanMsg candata) {
	uint8_t i, j = 0;
	u16 index = 0;
	u16 a = 0, b;

	u8 n = Msg_EventTrans_Check(candata);
	if (n >= 0) {
		//	printf("EventTransMsg:%d,%X,",n,candata.ID);Fun_Puth("",candata.DATA,8);
		if (msg_event_make_num == 0) {
			if (cand_pgn_event_index1 < 600) {
				cand_pgn_event_can1[cand_pgn_event_index1][j++] = candata.TIME >> 24;
				cand_pgn_event_can1[cand_pgn_event_index1][j++] = candata.TIME >> 16;
				cand_pgn_event_can1[cand_pgn_event_index1][j++] = candata.TIME >> 8;
				cand_pgn_event_can1[cand_pgn_event_index1][j++] = candata.TIME;

				cand_pgn_event_can1[cand_pgn_event_index1][j++] = candata.ID >> 24;
				cand_pgn_event_can1[cand_pgn_event_index1][j++] = candata.ID >> 16;
				cand_pgn_event_can1[cand_pgn_event_index1][j++] = candata.ID >> 8;
				cand_pgn_event_can1[cand_pgn_event_index1][j++] = candata.ID;

				for (i = 0; i < 8; i++)
					cand_pgn_event_can1[cand_pgn_event_index1][j++] = candata.DATA[i];
				cand_pgn_event_index1++;
				if (cand_pgn_event_num1 < 600)
					cand_pgn_event_num1++;
			} else {
				cand_pgn_event_index1 = 0;
			}
		} else //有事件生成，执行生成事件透传信息逻辑
		{
			if (msg_event_make_num == 1) {
				//事件1发生后采集的PGN
				if ((MSG_TIMER_E2_1 == 0) || (MSG_TIMER_E2_1 > api_GetSysmSecs()))
					MSG_TIMER_E2_1 = api_GetSysmSecs();
				if (cand_pgn_event_index2 < 600) {
					cand_pgn_event_can2[cand_pgn_event_index2][j++] = candata.TIME >> 24;
					cand_pgn_event_can2[cand_pgn_event_index2][j++] = candata.TIME >> 16;
					cand_pgn_event_can2[cand_pgn_event_index2][j++] = candata.TIME >> 8;
					cand_pgn_event_can2[cand_pgn_event_index2][j++] = candata.TIME;

					cand_pgn_event_can2[cand_pgn_event_index2][j++] = candata.ID >> 24;
					cand_pgn_event_can2[cand_pgn_event_index2][j++] = candata.ID >> 16;
					cand_pgn_event_can2[cand_pgn_event_index2][j++] = candata.ID >> 8;
					cand_pgn_event_can2[cand_pgn_event_index2][j++] = candata.ID;

					for (i = 0; i < 8; i++)
						cand_pgn_event_can2[cand_pgn_event_index2][j++] = candata.DATA[i];
					cand_pgn_event_index2++;
					if (cand_pgn_event_num2 < 600)
						cand_pgn_event_num2++;
				}
			} else //事件大于1个
			{
				if ((MSG_TIMER_E2_2 == 0) || (MSG_TIMER_E2_2 > api_GetSysmSecs()))
					MSG_TIMER_E2_2 = api_GetSysmSecs();
				if (cand_pgn_event_index3 < 600) {
					cand_pgn_event_can3[cand_pgn_event_index3][j++] = candata.TIME >> 24;
					cand_pgn_event_can3[cand_pgn_event_index3][j++] = candata.TIME >> 16;
					cand_pgn_event_can3[cand_pgn_event_index3][j++] = candata.TIME >> 8;
					cand_pgn_event_can3[cand_pgn_event_index3][j++] = candata.TIME;

					cand_pgn_event_can3[cand_pgn_event_index3][j++] = candata.ID >> 24;
					cand_pgn_event_can3[cand_pgn_event_index3][j++] = candata.ID >> 16;
					cand_pgn_event_can3[cand_pgn_event_index3][j++] = candata.ID >> 8;
					cand_pgn_event_can3[cand_pgn_event_index3][j++] = candata.ID;

					for (i = 0; i < 8; i++)
						cand_pgn_event_can3[cand_pgn_event_index3][j++] = candata.DATA[i];
					cand_pgn_event_index3++;
					if (cand_pgn_event_num3 < 600)
						cand_pgn_event_num3++;
				}
			}
		}
	}
//事件1发生前采集的PGN 生成信息并发送
	if ((msg_event_make_num >= 1) && (cand_pgn_event_flag != 1)) { //有事件发生并且事件1之前的PGN参数没有发送完毕，发送事件1之前的PGN参数
		a = 0;
		if (cand_pgn_event_num1 >= 600) {
			a = cand_pgn_event_index1;
		}

//		puts("cand_pgn_event_index1:");
//		putcha(cand_pgn_event_index1>>8);//输出单字节十六进制函数
//		putcha(cand_pgn_event_index1);
//		puts("\n");
//		printf("cand_pgn_event_index1: %d\n", cand_pgn_event_index1);
		while (cand_pgn_event_num1) {
//			puts("cand_pgn_event_num1:");
//			putcha(cand_pgn_event_num1>>8);
//			putcha(cand_pgn_event_num1);
//			puts("\n");
//			printf("cand_pgn_event_num1: %d\n", cand_pgn_event_num1);
			if (cand_pgn_event_num1 < 60)
				b = cand_pgn_event_num1;
			else
				b = 60;
			cand_pgn_event_num1 -= b;
			memset(&Msg_BC, 0, sizeof(QUE_TDF_QUEUE_MSG));
			index = 0;
			Msg_BC.MsgType = 0xBC;
			//版本号
			cpyVersion(Msg_BC.Version);
			//信息生成时间
			cpyInfoTime(Msg_BC.Time);
			//属性标识  高字节在前
			Msg_BC.Attribute[0] = 0x40;
			Msg_BC.Attribute[1] = 0;
			Msg_BC.data[index++] = b;
			if (a + b <= 600) {
				memcpy(Msg_BC.data + index, (u8 *) &cand_pgn_event_can1[a][0], 16 * b);
				index += 16 * b;
				a += b;
			} else {
				memcpy(Msg_BC.data + index, (u8 *) &cand_pgn_event_can1[a][0], 16 * (600 - a));
				index += 16 * (600 - a);
				b = a + b - 600;
				a = 0;
				memcpy(Msg_BC.data + index, (u8 *) &cand_pgn_event_can1[a][0], 16 * b);
				index += 16 * b;
				a += b;
			}
			//Message.length = index;
			u8 len[4] = { 0 };
			sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
			api_HexToAsc(len, Msg_BC.length, strlen((char*) len));
			//信息入队列
			printf_msg("<MsgMake_BC>message before event 1 ready in queue!!! \n");
			PassThrough_in(&Msg_BC);
		}
		cand_pgn_event_flag = 1;
	}
	if ((msg_event_make_num >= 1) && (cand_pgn_event_flag == 1)) { //事件1之前的数据发送完毕，发送事件1之后事件2之前的数据
		if (MSG_TIMER_E2_1 == 0)
			MSG_TIMER_E2_1 = api_GetSysmSecs();
		if (cand_pgn_event_num2 >= 60 || (api_DiffSysMSecs(MSG_TIMER_E2_1) > 60 * 1000)) {
			MSG_TIMER_E2_1 = api_GetSysmSecs();
			if ((cand_pgn_event_num2 < 60) && (cand_pgn_event_num2 != 0)) {
				memset(&Msg_BC, 0, sizeof(QUE_TDF_QUEUE_MSG));
				index = 0;
				Msg_BC.MsgType = 0xBC;
				//版本号
				cpyVersion(Msg_BC.Version);
				//信息生成时间
				cpyInfoTime(Msg_BC.Time);
				//属性标识  高字节在前
				Msg_BC.Attribute[0] = 0x40;
				Msg_BC.Attribute[1] = 0;
				Msg_BC.data[index++] = cand_pgn_event_num2;
				memcpy(Msg_BC.data + index, (u8 *) &cand_pgn_event_can2[msg_e2_index][0], 16 * cand_pgn_event_num2);
				index += 16 * cand_pgn_event_num2;
				printf_msg("event Que_In2 <60\n");
				//Message.length = index;
				u8 len[4] = { 0 };
				sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
				api_HexToAsc(len, Msg_BC.length, strlen((char*) len));
				//信息入队列
				PassThrough_in(&Msg_BC);
				msg_e2_index += cand_pgn_event_num2;
				cand_pgn_event_num2 = 0;
			} else if (cand_pgn_event_num2 >= 60) {
				memset(&Msg_BC, 0, sizeof(QUE_TDF_QUEUE_MSG));
				index = 0;
				Msg_BC.MsgType = 0xBC;
				//版本号
				cpyVersion(Msg_BC.Version);
				//信息生成时间
				cpyInfoTime(Msg_BC.Time);
				//属性标识  高字节在前
				Msg_BC.Attribute[0] = 0x40;
				Msg_BC.Attribute[1] = 0;
				Msg_BC.data[index++] = 60;
				memcpy(Msg_BC.data + index, (u8 *) &cand_pgn_event_can2[msg_e2_index][0], 16 * 60);
				index += 16 * 60;
				printf_msg("event Que_In2 >60\n");
				//Message.length = index;
				u8 len[4] = { 0 };
				sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
				api_HexToAsc(len, Msg_BC.length, strlen((char*) len));
				//信息入队列
				printf_msg("<MsgMake_BC>message between event 1 and 2 ready in queue!!! \n");
				PassThrough_in(&Msg_BC);
				msg_e2_index += 60;
				cand_pgn_event_num2 -= 60;
			}
		}
	}

	if ((cand_pgn_event_index2 >= 600) && (cand_pgn_event_num2 == 0)) { //事件1前后600包数据都完成发送
		memset(&cand_pgn_event_can1, 0, sizeof(cand_pgn_event_can1));
		cand_pgn_event_index1 = 0;
		cand_pgn_event_num1 = 0;
		memset(&cand_pgn_event_can2, 0, sizeof(cand_pgn_event_can2));
		cand_pgn_event_index2 = 0;
		cand_pgn_event_num2 = 0;
		msg_e2_index = 0;
		msg_event_make_num--;
	}

	if ((msg_event_make_num >= 2) && (cand_pgn_event_num2 == 0)) {
		if (MSG_TIMER_E2_2 == 0)
			MSG_TIMER_E2_2 = api_GetSysmSecs();
		if (cand_pgn_event_num3 >= 60 || (api_DiffSysMSecs(MSG_TIMER_E2_2) > 60 * 1000)) {
			MSG_TIMER_E2_2 = api_GetSysmSecs();
			if ((cand_pgn_event_num3 < 60) && (cand_pgn_event_num3 != 0)) {
				memset(&Msg_BC, 0, sizeof(QUE_TDF_QUEUE_MSG));
				index = 0;
				Msg_BC.MsgType = 0xBC;
				//版本号
				cpyVersion(Msg_BC.Version);
				//信息生成时间
				cpyInfoTime(Msg_BC.Time);
				//属性标识  高字节在前
				Msg_BC.Attribute[0] = 0x40;
				Msg_BC.Attribute[1] = 0;
				Msg_BC.data[index++] = cand_pgn_event_num3;
				memcpy(Msg_BC.data + index, (u8 *) &cand_pgn_event_can3[msg_e3_index][0], 16 * cand_pgn_event_num3);
				index += 16 * cand_pgn_event_num3;
				//Message.length = index;
				u8 len[4] = { 0 };
				sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
				api_HexToAsc(len, Msg_BC.length, strlen((char*) len));

				msg_e3_index += cand_pgn_event_num3;
				//puts("event Que_In3 <60\n");
				printf_msg("<MsgMake_BC>message event Que_In3 <60 ready in queue!!! \n");
				PassThrough_in(&Msg_BC); //保存信息到队列中
//				OSMutexPost(MAINTEMP_mutex);
				cand_pgn_event_num3 = 0;
			} else if (cand_pgn_event_num3 >= 60) {
				memset(&Msg_BC, 0, sizeof(QUE_TDF_QUEUE_MSG));
				index = 0;
				Msg_BC.MsgType = 0xBC;
				//版本号
				cpyVersion(Msg_BC.Version);
				//信息生成时间
				cpyInfoTime(Msg_BC.Time);
				//属性标识  高字节在前
				Msg_BC.Attribute[0] = 0x40;
				Msg_BC.Attribute[1] = 0;
				Msg_BC.data[index++] = 60;
				memcpy(Msg_BC.data + index, (u8 *) &cand_pgn_event_can3[msg_e3_index][0], 16 * 60);
				index += 16 * 60;
				//Message.length = index;
				u8 len[4] = { 0 };
				sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
				api_HexToAsc(len, Msg_BC.length, strlen((char*) len));

				//puts("event Que_In3 >60\n");
				printf_msg("<MsgMake_BC>message event Que_In3 >60 ready in queue!!! \n");
				PassThrough_in(&Msg_BC); //保存信息到队列中
				msg_e3_index += 60;
				cand_pgn_event_num3 -= 60;
			}
		}
	}
	if ((cand_pgn_event_index3 >= 600) && (cand_pgn_event_num3 == 0)) { //事件2后600包数据都完成发送
//		Fun_Clrram((u8 *)&cand_pgn_event_can1,sizeof(cand_pgn_event_can1));
		memset((u8 *) &cand_pgn_event_can1, 0, sizeof(cand_pgn_event_can1));
		cand_pgn_event_index1 = 0;
		cand_pgn_event_num1 = 0;
//		Fun_Clrram((u8 *)&cand_pgn_event_can2,sizeof(cand_pgn_event_can2));
		memset((u8 *) &cand_pgn_event_can2, 0, sizeof(cand_pgn_event_can2));
		cand_pgn_event_index2 = 0;
		cand_pgn_event_num2 = 0;
//		Fun_Clrram((u8 *)&cand_pgn_event_can3,sizeof(cand_pgn_event_can3));
		memset((u8 *) &cand_pgn_event_can3, 0, sizeof(cand_pgn_event_can3));
		cand_pgn_event_index3 = 0;
		cand_pgn_event_num3 = 0;
		msg_e3_index = 0;
		msg_event_make_num = 0;
	}
}

/*
 *实时诊断透传时间设置
 *
 */
void MsgDecode_C3(u8 *data) {
	u16 tmp_u16 = 0;
	diagnosis_set.BeginSet = data[0];
	tmp_u16 = data[1];
	tmp_u16 = (tmp_u16 << 8) | data[2];
	diagnosis_set.KeepTime = tmp_u16;
}
/*
 * 实时诊断透传时间设置/回叫回复
 */
QUE_TDF_QUEUE_MSG Msg_C4;
void MsgMake_C4(u8 *serialnum) {
	memset(&Msg_42, 0, sizeof(QUE_TDF_QUEUE_MSG));
	u16 index = 0;
	//信息类型
	Msg_C4.MsgType = 0xC4;
	//版本号
	cpyVersion(Msg_C4.Version);
	//信息生成时间
	cpyInfoTime(Msg_C4.Time);
	//属性标识  高字节在前
	Msg_C4.Attribute[0] = 0x40;
	Msg_C4.Attribute[1] = 0;

	Msg_C4.data[index++] = serialnum[0];
	Msg_C4.data[index++] = serialnum[1];
	Msg_C4.data[index++] = serialnum[2];
	Msg_C4.data[index++] = diagnosis_set.BeginSet;
	Msg_C4.data[index++] = diagnosis_set.KeepTime >> 8;
	Msg_C4.data[index++] = diagnosis_set.KeepTime;

	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_C4.length, strlen((char*) len));
	//信息入队列
	printf_msg("<MsgMake_C4>message ready in queue %d bytes!!! \n", index + 16);
	PassThrough_in(&Msg_C4);
}

/**
 * data 数据
 * len 数据长度
 * isWebData 数据来源，1网络，0本地。来自网络，会执行将data串保存到本地
 * 返回值 事件设置个数
 */
int MsgDecode_C5(u8 *data, u32 len, u8 isWebData) {
	int offest = 0;
//	printfHexData(data, len);
//	u8 MSG_SetID[3];
//	memcpy(MSG_SetID, data, 3);
//	offest += 3; //信息对照码
	u8 set_num = data[offest++]; //PGN参数ID个数包
	//cpymem(&set_num, data, 1, &offest);
//	printf("set_num:%d\n", set_num);
	if ((set_num > 0) && (set_num <= MSG_PGN_SET_MAX)) { //设置终端
		int i;
		for (i = 0; i < set_num; i++) {
			cpymem(&(hand_pgn_set[i].mode), data, 1, &offest);
			//printfHexData((void*)&(hand_pgn_set[i].mode),1);
			cpymem(&(hand_pgn_set[i].pgn_id), data, 4, &offest);
			//printfHexData((void*)&(hand_pgn_set[i].pgn_id),4);
			cpymem(&(hand_pgn_set[i].cyc_get), data, 3, &offest);
			//printfHexData((void*)&(hand_pgn_set[i].cyc_get),3);
			if (hand_pgn_set[i].mode.bags) { //多包
				cpymem(&(hand_pgn_set[i].pgn_id0), data, 4, &offest);
				//	printfHexData((void*)&(hand_pgn_set[i].pgn_id0),4);
				cpymem(&(hand_pgn_set[i].pgn_id1), data, 4, &offest);
				//	printfHexData((void*)&(hand_pgn_set[i].pgn_id1),4);
			}
			if (hand_pgn_set[i].mode.da_id) { //有数据ID
				cpymem(&(hand_pgn_set[i].pgn_id_da), data, 1, &offest);
				//	printfHexData((void*)&(hand_pgn_set[i].pgn_id_da),1);
				memcpy(hand_pgn_set[i].pgn_id_da.id_da, data + offest, 7);
				offest += 7;
//				cpymem(&(hand_pgn_set[i].pgn_id_da.id_da), data, 7, &offest);
				//	printfHexData((void*)&(hand_pgn_set[i].pgn_id_da.id_da),7);
			}
		}
		msg_pgn_set_num = set_num;
		if (isWebData) //来自网络的数据，保存到本地
		{
			save2flash(C5_file, data, len); //保存参数
		}
	} else if (set_num == 0xFF) { //清除透传参数
		memset(hand_pgn_set, 0x00, sizeof(hand_pgn_set)); //清除旧数据
		msg_pgn_set_num = 0;
		if (isWebData) //来自网络的数据，保存到本地
		{
			save2flash(C5_file, data, len); //保存参数
		}
	} else if (set_num == 0xFE) { //设定所有参数
		msg_pgn_set_num = 0xFE;
//		save2flash(D1_file, data, len); //保存参数
	} else if (set_num == 0) { //回叫终端定时透传参数,不做任何设置

	}
//	printf("msg_pgn_set_num:%d,set_num:%d\n", msg_pgn_set_num, set_num);
	return set_num;
}
/*
 * name:MsgMake_C6
 * 	透传信息设置回复
 * 	无返回值
 */
QUE_TDF_QUEUE_MSG Msg_C6;
void MsgMake_C6(u8 *SerialNum, u8 num) {
	int index = 0;
	int j;
	memset(&Msg_C6, 0, sizeof(QUE_TDF_QUEUE_MSG));

	Msg_C6.MsgType = 0xC6;
	//信息类型
	Msg_C6.MsgType = 0xC4;
	//版本号
	cpyVersion(Msg_C6.Version);
	//信息生成时间
	cpyInfoTime(Msg_C6.Time);
	//属性标识  高字节在前
	Msg_C6.Attribute[0] = 0x40;
	Msg_C6.Attribute[1] = 0;
	//信息对照码
	Msg_C6.data[index++] = SerialNum[0];
	Msg_C6.data[index++] = SerialNum[1];
	Msg_C6.data[index++] = SerialNum[2];
	Msg_C6.data[index++] = msg_pgn_set_num;
	if ((num >= 0) && (num <= MSG_PGN_SET_MAX)) {
		for (j = 0; j < msg_pgn_set_num; j++) {
			Msg_C6.data[index++] = *(u8 *) &hand_pgn_set[j].mode;
			//	printfHexData(Message.data,5);
			copyobj(Msg_C6.data + index, &(hand_pgn_set[j].pgn_id), 4, &index); //加入PGN ID
			//	printfHexData(Message.data,9);
			copyobj(Msg_C6.data + index, &(hand_pgn_set[j].cyc_get), 3, &index);
			//	printfHexData(Message.data,12);
			if (hand_pgn_set[j].mode.bags == 1) { //多包
				copyobj(Msg_C6.data + index, &(hand_pgn_set[j].pgn_id0), 4, &index);
				//		printfHexData(Message.data,16);
				copyobj(Msg_C6.data + index, &(hand_pgn_set[j].pgn_id1), 4, &index);
				//		printfHexData(Message.data,20);
			}
			if (hand_pgn_set[j].mode.da_id == 1) { //有数据id
				Msg_C6.data[index] = hand_pgn_set[j].pgn_id_da.id_len << 5;
				Msg_C6.data[index++] |= hand_pgn_set[j].pgn_id_da.id_start << 2;
//				if (num == 0) {
				memcpy(Msg_C6.data + index, &(hand_pgn_set[j].pgn_id_da.id_da), 7);
				index += 7;
////					printf("AAAAAAAAAAAA\n");
//				} else {
//					copyobj(Message.data + index, &(hand_pgn_set[j].pgn_id_da.id_da), 7, &index);
//				}
			}
		}
	}
	u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
	api_HexToAsc(len, Msg_C6.length, strlen((char*) len));
	printf("<MsgMake_C6>message ready in queue %d bytes!!! \n", index + 16);
	PassThrough_in(&Msg_C6); //信息入队列
	if (debug_value & 0x80) {
		api_PrintfHex(Msg_C6.data, index);
	}
}

u8 GPS_inflect_Judge(GPS_struct *GPS_now) {
	u16 tmp_last = 0;
	int dir_diff = 0;
	if (GPS_Last.Valid == 0) {
		GPS_Last.Speed = GPS_now->Speed;
		GPS_Last.Azimuth = GPS_now->Azimuth;
		GPS_Last.Valid = 1;
		return 1;
	}

	if (GPS_now->Speed > gpsd_turn_speed_min) {  //速度大于10，角度差大于10
//	if (GPS_now->Speed >= 0) {
		tmp_last = GPS_Last.Azimuth;
		if (tmp_last > GPS_now->Azimuth) {
			dir_diff = tmp_last - GPS_now->Azimuth;
		} else {
			dir_diff = GPS_now->Azimuth - tmp_last;
		}
		if (dir_diff > 180) {
			dir_diff = 360 - dir_diff;
		}

		// 判断转向结果
		if (dir_diff > gpsd_turn_direct_min) { // 转向超15度
			printf("GPS_Last->Speed;%d,GPS_Last->Azimuth:%d;;GPS_now->Speed;%d,GPS_now->Azimuth:%d\n", GPS_Last.Speed, GPS_Last.Azimuth, GPS_now->Speed, GPS_now->Azimuth);
			memcpy(&GPS_Last, &GPS_Now, sizeof(GPS_struct));
			return 1;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}
/*
 *  name:Msg_Candata_Check
 * function: 对1多包广播包、2多包数据包 进行处理
 * 					记录数据采集时间，判断时间是否满足数据采集频率
 */
int8_t Msg_Candata_Check(CanMsg candata) {
	uint8_t i, a, b, c, d;
	uint32_t id2 = 0;

	if (msg_pgn_set_num == 0xFE)
		return 1;
	//	printf("Msg_Candata_Check:%X,%d,%d,",candata.ID,candata.SOURCE,candata.ide);printfHexData(candata.DATA,8);
	//Fun_Puth("",(u8 *)&hand_pgn_set,(msg_pgn_set_num*sizeof(TDF_PGN_SET)));
	for (i = 0; i < msg_pgn_set_num; i++) {
		c = 0;
		if (i > MSG_PGN_SET_MAX)
			break;
		if (candata.SOURCE == hand_pgn_set[i].pgn_id.can) {	//PGN参数来源
			if ((candata.ID & 0x00FFFF00) == CAN_ID_PGN_GB) 	//多包广播包
			{
				id2 = candata.DATA[6];
				id2 = id2 << 8 | candata.DATA[5];
				id2 = id2 << 8 | candata.DATA[7];
				if (id2 == (hand_pgn_set[i].pgn_id.id & 0x00FFFFFF)) {
					cand_pgn_bags_num[i].total_num = candata.DATA[3];
					cand_pgn_bags_num[i].today_num = 1;
					c = 2;
					if (id2 == (0x18FF244A & 0x00FFFFFF)) { 	//判断是否是GPS 信息
						IsGPSCanMsg = 1;
						//缓存GPS 广播ID 包
						memcpy(&GPScache[0], &GPScache[1], sizeof(GPSCanMsg)); //顺序递增缓存GPS PGN
						memcpy(&GPScache[1], &GPScache[2], sizeof(GPSCanMsg));
						memcpy(&GPScache[2].GPS_inflect[0], &candata, sizeof(CanMsg)); 	//最新GPS 信息广播包
						if (GPScache[2].GPS_inflect[0].DATA[3] == 0x04) {
							GPScache[2].HaveSend = 0;
						} else {
							GPScache[2].HaveSend = 1;
						}
						if (GPS_inflect_Judge(&GPS_Now) == 1) { //判断是否是拐点需要补传
							c = 3;
							GPS_inflect_flag = 1;
							IsGPSSendFlag = 1; 	//拐点发送
						} else {
							GPS_inflect_flag = 0;
							IsGPSSendFlag = 0; 	//不是拐点暂定不发
						}
					} else {
						IsGPSCanMsg = 0;
						GPS_inflect_flag = 0;
					}
				} else {
					cand_pgn_bags_num[i].total_num = 0;
					cand_pgn_bags_num[i].today_num = 0;
				}

			} else if ((candata.ID & 0x00FFFF00) == CAN_ID_PGN_DATA)	//多包数据包
			{
				GPS_inflect_flag = 0;	//Make D4,补传拐点保证只传一次
				if ((candata.DATA[0] == cand_pgn_bags_num[i].today_num) && (candata.DATA[0] <= cand_pgn_bags_num[i].total_num)) {
					cand_pgn_bags_num[i].today_num++;
					if (IsGPSCanMsg == 1) {	//缓存GPS数据ID包
						d = candata.DATA[0];
						memcpy(&GPScache[2].GPS_inflect[d], &candata, sizeof(CanMsg));
						if (IsGPSSendFlag == 0) {
							return -1;
						}
					}
					return i;
				} else {
					cand_pgn_bags_num[i].total_num = 0;
					cand_pgn_bags_num[i].today_num = 0;
				}
			}
			if (candata.ide == hand_pgn_set[i].mode.ide) {	//PGN ID的帧格式*/}
				if (candata.ID == hand_pgn_set[i].pgn_id.id) {
					if (hand_pgn_set[i].mode.da_id == 1) {	//	设置有数据ID
						for (a = hand_pgn_set[i].pgn_id_da.id_start, b = 0; b < hand_pgn_set[i].pgn_id_da.id_len; b++) {
							if (candata.DATA[a++] != hand_pgn_set[i].pgn_id_da.id_da[b]) {
								c = 0xFF;
								break;
							}
						}
						if (c != 0xFF) {
							c = 1;
						}
					} else {
						c = 1;
					}
				}
			}
			if ((c == 1) || (c == 2)) {
				if (hand_pgn_set[i].cyc_get != 0) {
					if (cand_pgn_cyc_get_timer[i] == 0 || cand_pgn_cyc_get_timer[i] > candata.TIME)
						cand_pgn_cyc_get_timer[i] = candata.TIME;
					if (candata.TIME - cand_pgn_cyc_get_timer[i] > hand_pgn_set[i].cyc_get) {
						cand_pgn_cyc_get_timer[i] = candata.TIME;
						if (id2 == (0x18FF244A & 0x00FFFFFF)) { //判断是否是GPS 信息
							memcpy(&GPS_Last, &GPS_Now, sizeof(GPS_struct));
							GPScache[0].HaveSend = 1;
							GPScache[1].HaveSend = 1;
							GPScache[2].HaveSend = 1;
							IsGPSSendFlag = 1; 	//GPS定时发送
							printf("GPS upload!\n");
						}
						return i;
					} else if (IsGPSCanMsg == 1) {
						return -1;
					} else {
						if (c == 2) {
							cand_pgn_bags_num[i].total_num = 0;
							cand_pgn_bags_num[i].today_num = 0;
						}
					}
				} else {
					return i;
				}
			} else if (c == 3) {
				GPScache[2].HaveSend = 1;
				return i;
			}
		}
	}
	return -1;
}
/*
 【0xC8】实时诊断透传数据汇报
 */
QUE_TDF_QUEUE_MSG Msg_C8;
u32 MSG_C8_lastTime;
u16 MSG_C8_Index = 0;
u8 pgn_sum = 0;
void MsgMake_C8(CanMsg candata) {
	uint32_t t;
//	u32 Sys_Run_mSec;
	u8 j;
	int n = Msg_Candata_Check(candata);
	u32 can_source_id = ((candata.SOURCE << 29) | (candata.ID & 0x1FFFFFFF));
	if (n > 0) {
		if (MSG_C8_Index == 0) {
			memset(&Msg_C8, 0, sizeof(QUE_TDF_QUEUE_MSG));
			msg_strat_time_ms = api_GetSysmSecs();
			//信息类型
			Msg_C8.MsgType = 0xC8;
			//版本号
			cpyVersion(Msg_C8.Version);
			//信息生成时间
			cpyInfoTime(Msg_C8.Time);

			MSG_C8_lastTime = candata.TIME - candata.TIME % 1000;
			t = Sys_Start_Time + candata.TIME / 1000;
			//属性标识  高字节在前 ?TODO 是否需要回执通用应答？
			Msg_C8.Attribute[0] = 0x40;
			Msg_C8.Attribute[1] = 0;

			MSG_C8_Index = 1;
		}
		t = candata.TIME - MSG_C8_lastTime;
		Msg_C8.data[MSG_C8_Index++] = t >> 8;
		Msg_C8.data[MSG_C8_Index++] = t;

		Msg_C8.data[MSG_C8_Index++] = can_source_id >> 24;
		Msg_C8.data[MSG_C8_Index++] = can_source_id >> 16;
		Msg_C8.data[MSG_C8_Index++] = can_source_id >> 8;
		Msg_C8.data[MSG_C8_Index++] = can_source_id;

		for (j = 0; j < 8; j++) {
			Msg_C8.data[MSG_C8_Index++] = candata.DATA[j];
		}
		pgn_sum++;
	}
	//满足条件信息入队列
	if ((MSG_C8_Index >= 1024) || ((api_DiffSysMSecs(msg_strat_time_ms) >= 60 * 1000) && (msg_strat_time_ms != 0))) {
		if (pgn_sum > 0 && MSG_C8_Index > 0) {
			Msg_C8.data[0] = pgn_sum;
			u8 len[4] = { 0 };
			sprintf((char*) len, "%04X", (unsigned int) MSG_C8_Index); //消息体长度
			api_HexToAsc(len, Msg_C8.length, strlen((char*) len));
			//信息入队列
			printf_msg("<Make_B4_message>message ready in queue %d bytes!!! \n", MSG_C8_Index + 14);
			PassThrough_in(&Msg_C8);
			//			if (debug_value & 0x04) {
			//				printfHexData(D4_Msg.data, MSG_D4_Index);
			//			}
			memset(&Msg_C8, 0x00, sizeof(QUE_TDF_QUEUE_MSG));
			MSG_C8_Index = 0;
			pgn_sum = 0;
		}
	}
}
