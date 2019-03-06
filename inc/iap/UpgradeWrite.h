/*
 * UpgradeWrite.h
 *
 *  Created on: 2018年1月9日
 *      Author: tykj
 */

#ifndef UPGRADEWRITE_H_
#define UPGRADEWRITE_H_

#include <stdio.h>

#define     SDWAY       0
#define     COMWAY      1
#define    PROGRAMWAY   COMWAY

#if PROGRAMWAY
#define rcSfile "/etc/init.d/rcS"
#else
#define rcSfile "/etc/default/rcS"
#endif
#define APNFile "/etc/ppp/gprs-connect-chat"
/********************************************
 * ******修改升级文件路径  *************/
#define wr_file_path	"/usr/sbin/wr"
#define MPU_1_FILE_PWD "/opt/ct100d-1"
#define MPU_2_FILE_PWD "/opt/ct100d-2"
//#define MCU_FILE_PWD "/ty/ct3000/bin/mcu/CT3000-MCU"
#define VCU_FILE_PWD "/ty/ct3000/bin/vcu/CT3000-VCU"

#define BOOT_SPL_PWD	"/opt/u-boot-spl.bin"
#define BOOT_KERNEL_PWD	"/opt/uImage"
#define BOOT_UBI_PWD	"/opt/ubi.img"
#define ITC_CT100D1_PWD "/ty/ct100d/bin/ct100d-1"
#define ITC_CT100D2_PWD "/ty/ct100d/bin/ct100d-2"

extern char Current_Version[10]; /*current ITC program version*/
extern int rw_right(const char *param, const char *cmd);
extern int updata_IAPWrite(unsigned char* ch);
//extern int updata_IAPWrite(char *ch);
extern int Updata_APNWrite(unsigned char *ch);
extern int Updata_ReadVcuData(FILE *fp, char *ch);

extern int update_SystemCmd1(const char *cmd, const char *param1);
extern int update_SystemCmd2(const char *cmd, const char *param1, const char *param2);
extern int Updata_CheckFile(FILE *fp,char *fpbuf,long size);
extern int up_ITC();
extern int systemcmd( const char *cmd);
#define CAN_VCU_75F_DATA_SIZE (14)  // 一次从文件内读取数据长度
#endif /* UPGRADEWRITE_H_ */
