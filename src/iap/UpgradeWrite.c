/*
 * UpgradeWrite.c
 *
 *  Created on: 2018年1月9日
 *      Author: tykj
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "api.h"
#include "UpgradeWrite.h"
#include "iap.h"

IAP_Con IAP;
IAPResult IAPRet;

int systemcmd(const char *cmd) {

	int ret;
	int i;

	for (i = 0; i < 3; i++) {
		ret = system(cmd);

		if ((ret < 0) || (ret == 127)) {
			if (debug_value & 0x40) {
				perror(cmd);
			}

		} else {
			return (0);
		}
	}
	return -1;
}

/*
 NAME:   rw_right
 FUNC:   set root filesystem rw/ro  跟文件系统权限操作
 rw:"/path enable"
 ro:"/path disable"

 */
int rw_right(const char *param, const char *cmd) {
	char cmdBuf[256];
	int ret;
	int i;

	for (i = 0; i < 3; i++) {
		sprintf(cmdBuf, "%s %s", param, cmd);
		ret = system(cmdBuf);
		printf_iap("rw_enable: %s, ret=%d\n", cmdBuf, ret);
		if ((ret < 0) || (ret == 127)) {
			if (debug_value & 0x40) {
				perror(cmdBuf);
			}

		} else {
			return (0);
		}
	}
	return -1;
}

/*
 NAME:   update_SystemCmd1
 FUNC:   NO
 INPUT:  *cmd
 *param1IAP_Con  IAP;
 OUTPUT: NO
 RETURN:
 0 write file not need
 1 write file ok
 History:
 V1.0 wangyin 2014-7-8 10:59:48
 - initial version
 */
int update_SystemCmd1(const char *cmd, const char *param1) {
	char cmdBuf[256];
	int ret;
	int i;

	for (i = 0; i < 3; i++) {
		system("wr enable");
		sleep(1);
		sprintf(cmdBuf, "%s %s\n", cmd, param1);
		ret = system(cmdBuf);
		printf_iap("update_SystemCmd1: %s, ret=%d\n", cmdBuf, ret);
		if ((ret < 0) || (ret == 127)) {
			if (debug_value & 0x40) {
				perror(cmdBuf);
			}
		} else {
			return (0);
		}
	}

	return (-1);
}
/*
 NAME:   update_SystemCmd2
 FUNC:   NO
 INPUT:  *cmd
 *param1
 *param2
 OUTPUT: NO
 RETURN:
 0 write file not need
 1 write file ok
 History:
 V1.0 wangyin 2014-7-8 10:59:48
 - initial version
 */
int update_SystemCmd2(const char *cmd, const char *param1, const char *param2) {
	char cmdBuf[256];
	int ret;
	int i;

	for (i = 0; i < 3; i++) {
		system("wr enable");
		sleep(1);
		sprintf(cmdBuf, "%s %s %s\n", cmd, param1, param2);
		ret = system(cmdBuf);
		printf_iap("update_SystemCmd2: %s, ret=%d\n", cmdBuf, ret);
		if ((ret == -1) || (ret == 127)) {
			if (debug_value & 0x40) {
				perror(cmdBuf);
			}
		} else {
			return (0);
		}
	}

	return (-1);
}

long pIndex = 0;
char* char_File = NULL;

/*
 NAME:   update_WriteFile
 FUNC:   delet fileX, write fileX
 INPUT:  *file_path
 *char_File
 IAP.Total_Length
 OUTPUT: NO
 RETURN:
 -1 error
 1 write file ok
 History:
 V1.0 wangyin 2014-8-8 17:07:19
 - use one fun for writefileX
 */
int update_WriteFile(const char *file_path) {
	FILE *fpw = NULL; // fp of write

	system("cd /opt");
	system("wr  enable");

	update_SystemCmd1("rm -rf", file_path);
	sleep(1);
	fpw = fopen(file_path, "wb+");
	if (fpw == NULL) {
		printf_iap(file_path);
		if (debug_value & 0x40) {
			perror(" update_WriteFile fopen");
		}
		return (-1);
	}

	sleep(1);
	fwrite(char_File, IAP.Total_Length, 1, fpw);
	sleep(3);
	fclose(fpw);

	update_SystemCmd1("chmod 777", file_path);

	return (1);
}

/*
 NAME:   update_SaveIapFile
 FUNC:   NO
 INPUT:  NO
 OUTPUT: NO
 RETURN:
 0 write file not need
 1 write file ok
 History:
 V1.0 wangyin 2014-7-8 10:59:48
 - initial version
 - wy!104306.5
 V1.1 wangyin 2014-8-8 17:07:19
 - update ct3000-1\-2 at same time
 */
int update_SaveIapFile(void) {

	FILE *fpr = NULL;
	char *fpbuf = NULL;

	int mpuflg = 0;
//	int mcuflg = 0;
	int vcuflg = 0;
	int boot_splflg = 0;
//	int  boot_ubootflg = 0;
	int boot_kernelflg = 0;
	int boot_ubiflg = 0;
	int common_ubiflg = 0;

	u16 FileCrc = 0;

	if (IAP.Current_Pack != IAP.Total_Pack) {
		printf_iap("update_SaveIapFile Current_Pack:Total_Pack=%02x:%02x\n", IAP.Current_Pack, IAP.Total_Pack);
		return (0);
	}

	IAP.Deal_Total_CRC = api_CheckCrc(0xffff, char_File, IAP.Total_Length);
	printf_iap("updata_IAPWrite:IAP.Deal_Total_CRC=%x\n", IAP.Deal_Total_CRC);
	printf_iap("updata_IAPWrite:IAP.Total_CRC=%x\n", IAP.Total_CRC);
	/*****************************write file***************************************/
	if (IAP.Deal_Total_CRC == IAP.Total_CRC) {
		if (IAP.Updata_Type == 0xFE) {
			update_WriteFile(MPU_2_FILE_PWD);
			update_WriteFile(MPU_1_FILE_PWD);
			mpuflg = 1;
		} else if (IAP.Updata_Type == 0xF1) {
			update_WriteFile(VCU_FILE_PWD);
			vcuflg = 1;
		}
//		else if(IAP.Updata_Type == 0xF2){
//			update_WriteFile(MCU_FILE_PWD);
//			mcuflg=1;
//		}
		else if (IAP.Updata_Type == 0xF3) {
			update_WriteFile(BOOT_SPL_PWD);
			boot_splflg = 1;
		}
//		else if (IAP.Updata_Type == 0xF4) {
//			update_WriteFile(BOOT_UBOOT_PWD);
//			boot_ubootflg = 1;
//
//		}
		else if (IAP.Updata_Type == 0xF5) {
			update_WriteFile(BOOT_KERNEL_PWD);
			boot_kernelflg = 1;
		} else if (IAP.Updata_Type == 0xF6) {
			update_WriteFile(BOOT_UBI_PWD);
			boot_ubiflg = 1;
		} else if (IAP.Updata_Type == 0xF9) {
			update_WriteFile(Temp_Path);	// zjx 2017-8-14
			common_ubiflg = 1;
		} else {
			printf_iap("<%s> :updata type param[%d] err,quite!!!", __FUNCTION__, IAP.Updata_Type);
			return -1;
		}

		free(char_File);
		pIndex = 0;
		if (char_File != NULL) {
			char_File = NULL;
			printf_iap("updata_IAPWrite: char_File = NULL\n");
		}
		sleep(1);
		/*****************************check file***************************************/
		if (mpuflg == 1) {
			fpr = fopen(MPU_1_FILE_PWD, "r+");
			mpuflg = 0;
		}
		if (vcuflg == 1) {
			fpr = fopen(VCU_FILE_PWD, "r+");
			vcuflg = 0;
		}
//		if(mcuflg==1) {
//			fpr = fopen(MCU_FILE_PWD,"r+");
//			mcuflg=0;
//		}
		if (boot_splflg == 1) {
			fpr = fopen(BOOT_SPL_PWD, "r+");
			boot_splflg = 0;
		}
//		if(boot_ubootflg==1) {
//			fpr = fopen(BOOT_UBOOT_PWD,"r+");
//			boot_ubootflg=0;
//		}
		if (boot_kernelflg == 1) {
			fpr = fopen(BOOT_KERNEL_PWD, "r+");
			boot_kernelflg = 0;
		}
		if (boot_ubiflg == 1) {
			fpr = fopen(BOOT_UBI_PWD, "r+");
			boot_ubiflg = 0;
		}
		if (common_ubiflg == 1) {
			fpr = fopen(Temp_Path, "r+");
			common_ubiflg = 0;
		}
		fpbuf = (char*) malloc(IAP.Total_Length * sizeof(char));
		FileCrc = Updata_CheckFile(fpr, fpbuf, IAP.Total_Length);
		fclose(fpr);
		free(fpbuf);
		fpbuf = NULL;

		/*************************resault*******************************************/
		if (FileCrc == IAP.Deal_Total_CRC) {
			printf_iap("updata_IAPWrite:IAP FILE CHECK OK!\n");
			IAP.Deal_Result = IAP_DEAL_OK;
			if (IAP.Updata_Type == 0xFE) {
				printf_iap("IAPRet.ITCExist_Flag=0x01\n");
				IAPRet.ITCExist_Flag = 0x01;
			}
			if (IAP.Updata_Type == 0xF1) {
				printf_iap("IAPRet.VCUExist_Flag=0x01\n");
				IAPRet.VCUExist_Flag = 0x01;
			}
			if (IAP.Updata_Type == 0xF2) {
				printf_iap("IAPRet.MCUExist_Flag=0x01\n");
				IAPRet.MCUExist_Flag = 0x01;
			}
			if (IAP.Updata_Type == 0xF3) {
				printf_iap("<%s> IAPRet.Boot_SplExist_Flag == %d\n", __FUNCTION__, IAPRet.Boot_SplExist_Flag);
				IAPRet.Boot_SplExist_Flag = 0x01;
			}
			if (IAP.Updata_Type == 0xF4) {
				printf_iap("<%s> IAPRet.Boot_UbootExist_Flag == %d\n", __FUNCTION__, IAPRet.Boot_UbootExist_Flag);
				IAPRet.Boot_UbootExist_Flag = 0x01;
			}
			if (IAP.Updata_Type == 0xF5) {
				printf_iap("<%s> IAPRet.Boot_KernelExist_Flag == %d\n", __FUNCTION__, IAPRet.Boot_KernelExist_Flag);
				IAPRet.Boot_KernelExist_Flag = 0x01;
			}
			if (IAP.Updata_Type == 0xF6) {
				printf_iap("<%s> IAPRet.Boot_UbiExist_Flag == %d\n", __FUNCTION__, IAPRet.Boot_UbiExist_Flag);
				IAPRet.Boot_UbiExist_Flag = 0x01;
			}
			if (IAP.Updata_Type == 0xF9) {
				printf_iap("<%s> IAPRet.Common_FileExist_Flag == %d\n", __FUNCTION__, IAPRet.Common_FileExist_Flag);
				IAPRet.Common_FileExist_Flag = 0x01;
			}

			IAPRet.Finish_Result = IAP_DEAL_OK;
		} else {
			printf_iap("updata_IAPWrite:IAP FILE CHECK Err!\n");
			IAPRet.Finish_Result = IAP_CRC_ERROR;
		}
	} else {
		IAPRet.Finish_Result = IAP_CRC_ERROR;
		if (IAP.Updata_Type == 0xFE) {
			IAPRet.ITC_UPFlg = 1;
		}
		if (IAP.Updata_Type == 0xF1) {
			IAPRet.VCU_UPFlg = 1;
		}
		if (IAP.Updata_Type == 0xF2) {
			IAPRet.MCU_UPFlg = 1;
		}
		if (IAP.Updata_Type == 0xF3) {
			IAPRet.Boot_Spl_UPFlag = 1;
		}
		if (IAP.Updata_Type == 0xF4) {
			IAPRet.Boot_Uboot_UPFlag = 1;
		}
		if (IAP.Updata_Type == 0xF5) {
			IAPRet.Boot_Kernel_UPFlag = 1;
		}
		if (IAP.Updata_Type == 0xF6) {
			IAPRet.Boot_Ubi_UPFlag = 1;
		}
		if (IAP.Updata_Type == 0xF9) {
			IAPRet.Common_File_UPFlag = 1;
		}
	}
	printf_iap("updata_IAPWrite:IAPRet.Finish_Result=%02x\n", IAPRet.Finish_Result);
	return 1;
}

/*-----------------------------------------------------
 Name     :Updata_IAPWrite()
 Funciton :write download data
 Input    :none
 Output   :none.
 Author   :GZY-2014/02/19.
 Modify   :[LMM-2014/03/23]
 ------------------------------------------------------*/
int updata_IAPWrite(unsigned char* ch) {
	printf_iap("updata_IAPWrite:start\n");
	if (char_File == NULL) {
		printf_iap("updata_IAPWrite:malloc space\n");
		char_File = (char*) malloc(IAP.Total_Length * sizeof(char));
	}
	printf_iap("updata_IAPWrite:start write buffer\n");
	memcpy(char_File + pIndex, ch, IAP.Current_Length);
	pIndex += IAP.Current_Length;
	printf_iap("updata_IAPWrite:pIndex=%ld\n", pIndex);
	printf_iap("updata_IAPWrite:IAP.Total_Pack=%x\n", IAP.Total_Pack);

	return (update_SaveIapFile());
}

/*-----------------------------------------------------
 Name     :Updata_APNWrite()
 Funciton :
 Input    :none
 Output   :none.
 Author   :GZY-2014/02/19.
 Modify   :[<name>-<data>]
 ------------------------------------------------------*/
int write_file_apn(char* ch, unsigned int nlen) {
	FILE* fout = fopen(APNFile, "wb+");
	if (!fout) {
		printf_iap("write_file_apn: open file failed!\n");
		return -1;
	} else {
		printf_iap("write_file_apn: open file success!\n");
	}
	fwrite(ch, nlen, sizeof(char), fout);
	printf_iap("write_file_apn: fwrite end\n");
	fclose(fout);

	return 0;
}
int str_replace(char *sSrc, char *sMatchStr, unsigned long int flen, char *sReplaceStr) {
	int StringLen;
	int n = 0;

	char caNewString[flen + 100];

	char *FindPos = strstr(sSrc, sMatchStr);
	if ((!FindPos) || (!sMatchStr)) {
		return 0;
	}
//    DBG0_PR("str_replace:flen=%ld\n",flen);

	while (FindPos) {
		//DBG0_PR("FindPos strMatch : %s\n",FindPos);
		memset(caNewString, 0, sizeof(caNewString));
		StringLen = FindPos - sSrc;
//        DBG0_PR("str_replace:StringLen=%ld\n",StringLen);
		strncpy(caNewString, sSrc, StringLen);
		strcat(caNewString, sReplaceStr);
		strcat(caNewString, FindPos + strlen(sMatchStr));
		strcpy(sSrc, (const char *) caNewString);
		n++;
		FindPos = strstr(sSrc, sMatchStr);
	}

	if (strcmp(sMatchStr, "CMNET") == 0) {
		flen = flen - n * 6;
	} else {
		flen = flen + n * 6;
	}
//   DBG0_PR("str_replace:END-flen=%d\n",flen);
	return flen;
}

/*-----------------------------------------------------
 Name     :Updata_ReadVcuData()
 direction:
 char* str = Func(p);
 free(str);
 Input    :none
 Output   :none.
 Author   :GZY-2014/02/19.
 Modify   :[LMM-2014/02/24]
 ------------------------------------------------------*/
int Updata_ReadVcuData(FILE *fp, char *ch) {
	int ret;
	static int i = 0;
	i++;
	if (i % 2000 == 0) {
		printf_iap("Updata_ReadVcuData enter count = %d\n", i);
	}

	if (feof(fp) != 0) {
		printf_iap("Updata_ReadVcuData -> wen jian jie shu!\n");
		return 0;
	}
	ret = (fread(ch, 1, CAN_VCU_75F_DATA_SIZE, fp));
	return ret;
}
/*
 */

/*-----------------------------------------------------
 Name     :Updata_CheckFile()
 direction:Read upfile check crc
 Input    :
 Output   :crc
 Author   :lmm-2014/06/13
 ------------------------------------------------------*/
int Updata_CheckFile(FILE *fp, char *fpbuf, long size) {
	int crc;
	memset(fpbuf, 0, sizeof(*fpbuf));
	while (!feof(fp)) {
		fread(fpbuf, size, 1, fp);
	}

	crc = api_CheckCrc(0xffff, fpbuf, size);
	return crc;
}

/*
 更新根文件系统app  ct3000-1 ct3000-2
 */
int up_ITC() {
	update_SystemCmd1("rm -rf", ITC_CT100D2_PWD);
	update_SystemCmd2("mv -f", MPU_2_FILE_PWD, ITC_CT100D2_PWD);
	update_SystemCmd1("rm -rf", ITC_CT100D1_PWD);
	update_SystemCmd2("mv -f", MPU_1_FILE_PWD, ITC_CT100D1_PWD);
	return 0;
}
