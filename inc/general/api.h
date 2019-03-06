#ifndef __API_H__
#define __API_H__

#include <stdint.h>

#include "general.h"

/*------------------------------
 *	字符串处理函数
 * ------------------------------*/
extern s32 		api_HexToAsc(unsigned char* strasc, unsigned char* strhex, int length); // 普通字符串转成16进制字符串
extern u16 	api_CheckCrc( u16 CRC, void *Data, u32 Size); // 生成crc校验码
extern u8 		api_CheckSum(u8 *buf, int nword); // 求和校验
extern s32 		api_AscToHex(unsigned char* strhex, unsigned char* strasc, int length); // 十六进制字符串转成普通字符串
extern s32 		api_FindStr(char * askStr, char *checkStr);	// 寻找字符串中是否包含子串
extern void 	api_PrintfHex(u8 *data,int len); // 打印十六进制字符串
/*------------------------------
 *	时间处理函数
 * ------------------------------*/
extern void api_SetSystemTime(u8 *time); // 设置系统时间
extern void api_SetRtcTime(u8 *time); // 设置RTC时间
extern time_t api_GetSysSecs(void); // 获取从系统启动到现在的秒数
extern time_t api_GetSysmSecs(void);
extern time_t api_DiffSysSecs(time_t secLast);
extern time_t api_GetSysMSecs(void);
extern time_t api_DiffSysMSecs(time_t msecLast);
extern int api_TimeOut(int fd, int flag, int timeOutSec);
extern u32 GetNowTime(DateTime *t);

#endif

