/*
 * iap.h
 *
 *  Created on: 2018年1月9日
 *      Author: tykj
 */

#ifndef IAP_H_
#define IAP_H_

#include "general.h"

#define  IAP_DEAL_OK            0x00
#define  IAP_CRC_ERROR          0x01
#define  IAP_PACK_ERROR         0x02
#define  IAP_NET_ERROR          0x03
#define  IAP_E2_ERROR           0x04
#define  IAP_PROGRAM_ERROR      0x05
#define  IAP_FILE_ERROR         0x06
#define  IAP_UNKNOWN            0x07
#define  IAP_CRAT_ERROR            0x08
#define  IAP_CRAT_EXIT            0x09
#define  IAP_TIME_OUT           0xFE

#define nandwrite_path  "/usr/sbin/nandwrite"
#define flash_erase_path "/usr/sbin/flash_erase"
#define reboot_path "/sbin/reboot"
#define usr_path "/opt"
#define usr_path_w   "/opt/nandwrite"
#define usr_path_e   "/opt/flash_erase"
#define usr_path_reboot   "/opt/reboot"
typedef struct
{
   u8   Update_Enable;/*00-close;01-open*/
   u8   Updata_Type;

   u32   Total_Length;
   u16   Total_Pack;
   u16   Total_CRC;

   u16   Current_Pack;
   u16   Current_Length;
   u16   Current_CRC;

   u16   Deal_Total_CRC;
   u16   Deal_Pack;
   u16   Deal_Result;
} IAP_Con;
extern IAP_Con IAP;

typedef struct
{
   u8   ITCExist_Flag;
   u8   VCUExist_Flag;
   u8   MCUExist_Flag;/*0x00:no file;0x01:hava data*/
   u8   Boot_SplExist_Flag;
   u8   Boot_UbootExist_Flag;
   u8   Boot_KernelExist_Flag;
   u8   Boot_UbiExist_Flag;
   u8   Common_FileExist_Flag;// ZJX 2017-8-12；通用文件存在标志位；
   u8   ITC_UPFlg;  // wy!104302.6 0默认 1需要发送升级完成(包含成功失败结果)信息
   u8   VCU_UPFlg;  // wy!104302.6 0默认 1需要发送升级完成(包含成功失败结果)信息
   u8   MCU_UPFlg;  // wy!104302.6 0默认 1需要发送升级完成(包含成功失败结果)信息
   u8   Boot_Spl_UPFlag;
   u8   Boot_Uboot_UPFlag;
   u8   Boot_Kernel_UPFlag;
   u8   Boot_Ubi_UPFlag;
   u8   Common_File_UPFlag;
   u8   Finish_Result;  // 对应的结果形式见"IAP_DEAL_OK"等定义
   u8   Finish_Type;
}IAPResult;
extern IAPResult IAPRet;

typedef enum
{
     IAP_Update_IDLE,
     IAP_Update_OPEN,
     IAP_Update_START,
     IAP_Update_END,
} IAP_Manage_State_e;
extern IAP_Manage_State_e IAP_Manage_State;
extern  int update_iap;
extern void *iap_Pthread(void *arg);

extern int IAP_61_Parse(RecMessage *RecMess);
extern int IAP_62_Make(void);
extern int IAP_63_Parse(RecMessage *RecMess);
extern int IAP_64_Make(void);
extern int IAP_65_Parse(RecMessage *RecMess);
extern int IAP_66_Make(void);
extern int IAP_67_Parse(RecMessage *RecMess);
extern int IAP_68_Make(u8 val);
extern int IAP_69_Parse(RecMessage *RecMess);
extern int IAP_MsgFlagClear();
extern int IAP_71_Parse(RecMessage *RecMess);



#define max_len 64
#define whole_len 128
typedef struct
{

	u8 order_type;
	u8 path_len;
	char temp_path[max_len];//临时路径
	char file_path[max_len];//文件路径
	u8 name_len;
	char file_name[max_len];
	char whole_path[whole_len];//完整路径 包含路径+文件名称
	char whole_temp_path[whole_len];
	char property[3]; //文件属性
}IAP_SET_SAVE_TDF;
IAP_SET_SAVE_TDF iap_set;



#define Temp_Path "/opt/tmp.bin"

extern int Order_Handle();
extern int IAP_72_Make();
extern int create_file();
extern int update_file();
extern int delete_file();



#endif /* IAP_H_ */
