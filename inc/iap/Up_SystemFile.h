/*
 * Up_SystemFile.h
 *
 *  Created on: 2018年1月9日
 *      Author: tykj
 */

#ifndef UP_SYSTEMFILE_H_
#define UP_SYSTEMFILE_H_
#include "UpgradeWrite.h"

#define spl_path		"/dev/mtd0"
#define kernel_path		"/dev/mtd1"
#define ubi_path		"/dev/mtd2"




#define spl_file	BOOT_SPL_PWD
#define uboot_file	BOOT_UBOOT_PWD
#define kernel_file	BOOT_KERNEL_PWD
#define ubi_file	BOOT_UBI_PWD

extern int up_spl();
extern int up_spl_bak1();
extern int up_spl_bak2();
extern int up_spl_bak3();

extern int up_uboot();
extern int up_kernel();
extern int up_kernel_bak1();
extern int up_ubi();


extern int cmd_erase(char* cmd,char *partition,int addr,int block_count);
#endif /* UP_SYSTEMFILE_H_ */
