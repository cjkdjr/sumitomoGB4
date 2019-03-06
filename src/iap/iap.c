/*
 * iap.c
 *
 *  Created on: 2018年1月9日
 *      Author: tykj
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "iap.h"
#include "UpgradeWrite.h"
#include "iap_queue.h"
#include "gsm.h"
#include "api.h"
#include "Up_SystemFile.h"
#include "message_process.h"

//IAPResult IAPRet;
struct FifoQueue IAPQueue;

IAP_Manage_State_e IAP_Manage_State;
IAP_Con IAP;
IAPResult IAPRet;
int update_iap = 0;

int IAP_Flag65 = 0x00;
int IAP_Flag67 = 0x00;
int IAP_Flag69 = 0x00;
int IAP_Flag64 = 0x00;
int TimFlg = 0;
time_t TimeStart = 0;
time_t TimeEnd;

/*-----------------------------------------------------
 Name    :IAP_Manage_Task()
 Funciton:prase center data start download message
 Input   :none
 Output  :none
 Author  :lmm-2014/02/18、
 Modify  :[<name>-<data>]
 ------------------------------------------------------*/
void* iap_Pthread(void *arg) {
	printf_iap("############## %s start ##############\n", __FUNCTION__);
	iap_PthParam.sta = 1;
	pthread_detach(pthread_self());
	u16 update_failed_flag = 0; //升级失败标志位
	if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) != 0) {
		if (debug_value & 0x40) {
			perror("IAP_Pthread:setcancelstate\n");
		}
	}

	if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0) {
		if (debug_value & 0x40) {
			perror("IAP_Pthread:setcanceltype\n");
		}
	}
	IAP_Manage_State = IAP_Update_IDLE;
	iap_PthParam.flag = TRUE;
	while (iap_PthParam.flag) {
		iap_PthParam.sta = 1;
		while (PthreadPending.iap == 1) {
			PthreadPending.iap = 0;
			iap_PthParam.sta = 1;

			sleep(1);
		}
		switch (IAP_Manage_State)
		{
		case IAP_Update_IDLE:
			IAP.Deal_Pack = 0;
			IAP_MsgFlagClear();
			if (IAPRet.ITC_UPFlg == 1) {
				IAPRet.Finish_Type = 0x01;/*0xFE*/
				IAP_68_Make(IAPRet.Finish_Type);
				IAPRet.ITC_UPFlg = 0;
			}
			if (IAPRet.VCU_UPFlg == 1) {
				IAPRet.Finish_Type = 0xF1;
				IAP_68_Make(IAPRet.Finish_Type);
				IAPRet.VCU_UPFlg = 0;
			}
			if (IAPRet.MCU_UPFlg == 1) {
				IAPRet.Finish_Type = 0xF2;
				IAP_68_Make(IAPRet.Finish_Type);
				IAPRet.MCU_UPFlg = 0;
			}
			if (IAPRet.Boot_Spl_UPFlag == 1) {
				IAPRet.Finish_Type = 0xF3;
				IAP_68_Make(IAPRet.Finish_Type);
				IAPRet.Boot_Spl_UPFlag = 0;
			}
			if (IAPRet.Boot_Uboot_UPFlag == 1) {
				IAPRet.Finish_Type = 0xF4;
				IAP_68_Make(IAPRet.Finish_Type);
				IAPRet.Boot_Uboot_UPFlag = 0;
			}
			if (IAPRet.Boot_Kernel_UPFlag == 1) {
				IAPRet.Finish_Type = 0xF5;
				IAP_68_Make(IAPRet.Finish_Type);
				IAPRet.Boot_Kernel_UPFlag = 0;
			}
			if (IAPRet.Boot_Ubi_UPFlag == 1) {
				IAPRet.Finish_Type = 0xF6;
				IAP_68_Make(IAPRet.Finish_Type);
				IAPRet.Boot_Ubi_UPFlag = 0;
			}
			if (IAPRet.Common_File_UPFlag == 1) {
				IAPRet.Finish_Type = 0xF9;
				IAP_68_Make(IAPRet.Finish_Type);
				IAPRet.Common_File_UPFlag = 0;
			}
			if (IAP.Update_Enable == 0x01) {
//                   if((gsm.TransportWay == GPRS_SEND_MODE)&&(GSM_PPPFlg==1))
				if (GSM_Sock_PPPFlg == 1)  //拨号成功
				                {
					if (QueueInit(&IAPQueue) < 0) {
						printf_iap("IAPQueue: QueueInit error\n");
					}
					IAP_Manage_State = IAP_Update_OPEN;
					printf_iap("IAP_Update_OPEN\n");
				} else {
					printf_iap("IAP_Update_IDLE:Net is not ready,Can not updata!\n");
				}
			}
			break;
		case IAP_Update_OPEN:
			if ((IAP.Updata_Type == 0xFE) || (IAP.Updata_Type == 0xF1) || (IAP.Updata_Type == 0xF2) || (IAP.Updata_Type == 0xF3) || (IAP.Updata_Type == 0xF4) || (IAP.Updata_Type == 0xF5) || (IAP.Updata_Type == 0xF6) || (IAP.Updata_Type == 0xF9)) {
				TimFlg = 0;
				IAP_64_Make();
				IAP_Flag64 = 0x01;
				IAP_Manage_State = IAP_Update_START;
			} else {
				printf_iap("IAP_Update_OPEN:Illegal updata type,Quit iap\n");
				IAP_Manage_State = IAP_Update_END;
			}
			break;
		case IAP_Update_START:
			if ((IAP_Flag65 == 0x01) || (IAP_Flag67 == 0x01) || (IAP_Flag69 == 0x01)) {
				TimFlg = 0;
				IAP_Flag64 = 0x00;
				if (IAP_Flag69 == 0x01) {
					TimFlg = 0;
					IAP_Flag69 = 0x00;
					IAP_Manage_State = IAP_Update_END;
					printf_iap("IAP_Flag69==0x01 ? IAP_Manage_State=IAP_Update_END\n");
				} else {
					IAP_Flag65 = 0x00;
					IAP_Flag67 = 0x00;
					printf_iap("IAP_Update_START:MAKE(66)\n");
					IAP_66_Make();
					if (IAP.Deal_Result == IAP_DEAL_OK) {
						if (IAP.Deal_Pack == IAP.Total_Pack) {
							printf_iap("IAP_Update_START:IAP.Deal_Pack==IAP.Total_Pack\n");
							IAP_Manage_State = IAP_Update_END;
						}
						IAP.Deal_Pack++;
					}
				}
			} else {
				TimeStart = api_GetSysSecs();
				if ((TimFlg == 0) || (TimeEnd > TimeStart)) {
					TimeEnd = TimeStart;
					TimFlg = 1;
				} else {
					if (TimeStart - TimeEnd > 360)   // wy!104302.8.3
					                {

						TimFlg = 0;
						printf_iap("IAP:Receive data timeout!!!\n");
						IAP.Deal_Result = IAP_TIME_OUT;
						IAP_66_Make();
						IAP_Manage_State = IAP_Update_END;
					}
				}
			}
			break;
		case IAP_Update_END:

			if (IAPRet.ITCExist_Flag == 0x01) {
				//获取root filesystem权限
				rw_right( wr_file_path, "enable");
				//替换固件（启动路径/ty/ct3000/bin）
				up_ITC();
				printf_iap("<%s> :ready update ITC CT3000-1\n", __FUNCTION__);
				//设置只读权限
				rw_right( wr_file_path, "disable");
				IAPRet.ITCExist_Flag = 0x00;
				sleep(10);
				IAPRet.ITC_UPFlg = 1;
				IAPRet.Finish_Result = IAP_DEAL_OK;
				printf_iap("<%s> :ITC update finished! gsm go to sleep...\n", __FUNCTION__);
				gsm_cur_state = state_sleep;             // linux goto poweroff
			} else if (IAPRet.Boot_SplExist_Flag == 0x01) {
				IAPRet.Boot_SplExist_Flag = 0x00;
				sleep(1);
				printf_iap("<%s> :ready update spl.bak1\n", __FUNCTION__);
				up_spl_bak1();
				printf_iap("<%s> :spl.bak1 update ok\n", __FUNCTION__);
				printf_iap("<%s> :ready update spl.bak2\n", __FUNCTION__);
				up_spl_bak2();
				printf_iap("<%s> :spl.bak2 update ok\n", __FUNCTION__);
				printf_iap("<%s> :ready update spl.bak3\n", __FUNCTION__);
				up_spl_bak3();
				printf_iap("<%s> :spl.bak3 update ok\n", __FUNCTION__);
				printf_iap("<%s> :ready update spl\n", __FUNCTION__);
				up_spl();
				printf_iap("<%s> :spl update ok!!!!\n", __FUNCTION__);

				IAPRet.Boot_Spl_UPFlag = 1;
				IAPRet.Finish_Result = IAP_DEAL_OK;
				printf_iap("<%s> :SPL update finished! gsm go to sleep...\n", __FUNCTION__);
				gsm_cur_state = state_sleep;             // linux goto poweroff
			}

//				if(IAPRet.Boot_UbootExist_Flag==0x01)
//                {
//                    IAPRet.Boot_UbootExist_Flag=0x00;
//			        sleep(1);
//					printf_iap("<%s> :ready update uboot.img\n",__FUNCTION__);
////					up_uboot();
//					printf_iap("<%s> :uboot.img update ok\n",__FUNCTION__);
//
//
//                    IAPRet.Boot_Uboot_UPFlag=1;
//                    IAPRet.Finish_Result=IAP_DEAL_OK;
//					printf_iap("<%s> :ready inter sleep...\n",__FUNCTION__);
//					gsm_cur_state = state_sleep;             // linux goto poweroff
//		        }
			else if (IAPRet.Boot_KernelExist_Flag == 0x01) {
				IAPRet.Boot_KernelExist_Flag = 0x00;
				sleep(1);
				printf_iap("<%s> :ready update uImage\n", __FUNCTION__);
				up_kernel();
				printf_iap("<%s> :uImage update ok\n", __FUNCTION__);

				IAPRet.Boot_Kernel_UPFlag = 1;
				IAPRet.Finish_Result = IAP_DEAL_OK;
				printf_iap("<%s> :ready inter sleep...\n", __FUNCTION__);
				gsm_cur_state = state_sleep;             // linux goto poweroff
			} else if (IAPRet.Boot_UbiExist_Flag == 0x01) {
				rw_right( wr_file_path, "enable"); //文件系统读写使能
				update_SystemCmd2("cp", nandwrite_path, usr_path);
				update_SystemCmd2("cp", flash_erase_path, usr_path);
				update_SystemCmd2("cp", reboot_path, usr_path);
				IAPRet.Boot_UbiExist_Flag = 0x00;
				sleep(1);
				printf_iap("<%s> :ready update ubi.img\n", __FUNCTION__);
				up_ubi();
				printf_iap("<%s> :ubi.img update ok\n", __FUNCTION__);
				update_SystemCmd1("rm -f", usr_path_e); //delete
				update_SystemCmd1("rm -f", usr_path_w); //deleteb
				update_SystemCmd1("rm -f", usr_path_reboot); //delete
				IAPRet.Boot_Ubi_UPFlag = 1;
				IAPRet.Finish_Result = IAP_DEAL_OK;
				printf_iap("<%s> :ready inter sleep...\n", __FUNCTION__);
				gsm_cur_state = state_sleep;             // linux goto poweroff
			} else if (IAPRet.Common_FileExist_Flag == 0x01) {
				IAPRet.Common_FileExist_Flag = 0x00;
				sleep(1);
				IAPRet.Common_File_UPFlag = 1;
				IAPRet.Finish_Result = IAP_DEAL_OK;
			} else {
				update_failed_flag = 1;
			}
			printf_iap("*********IAP:Quit IAP MODE*************\n")
			;
			QueueDestroy(&IAPQueue);
			IAP.Update_Enable = 0x00;
			if (IAP_Flag64 == 0x01 || update_failed_flag == 1)             //升级不开始或异常中断
			                {
				gsm_cur_state = state_sleep;
			}
			sleep(10);
//                IAP_Manage_State =IAP_Update_IDLE;
			break;
		default:
			IAP_Manage_State = IAP_Update_IDLE;
			break;

		}
		usleep(300 * 1000);

	}
	printf_iap("############## %s exit ##############\n", __FUNCTION__);
	return 0;
}
int IAP_MsgFlagClear() {
	IAP_Flag65 = 0x00;
	IAP_Flag67 = 0x00;
	IAP_Flag69 = 0x00;
	IAP_Flag64 = 0x00;
	return 0;
}

/*-----------------------------------------------------
 Name    :IAP_63_Parse()
 Funciton:prase center data start download message
 Input   :RecMessage
 Output  :none
 --------------------------------------------------------
 Author  :lmm-2014/02/18
 Modify  :[<name>-<data>]
 ------------------------------------------------------*/
int IAP_63_Parse(RecMessage *RecMess) {
	u16 Index = 0;
	u8 i = 0;
	printf_iap("#############################63#############################\n");
	Index = 0;
	IAP.Updata_Type = RecMess->data[Index++];
	printf_iap("IAP_63_Parse:Remote update type : [%02x]\n", IAP.Updata_Type);
	if (IAP.Updata_Type == 0xF1)        //vcu
	                {
		for (i = 0; i < 7; i++) {
			System.GSMSet.UpVcuIP[i] = RecMess->data[Index + i];
		}
		for (i = 0; i < 21; i++) {
			System.GSMSet.ubUpVcuAPN[i] = RecMess->data[Index + i];
		}
		//Updata_APNWrite(System.GSMSet.ubUpVcuAPN);//test cmnet20140402
	}
	IAP.Update_Enable = 0x01;  //升级使能标志位
	printf_iap("IAP_63_Parse:IAP.Update_Enable = 0x%02x\n", IAP.Update_Enable);

	return 0;
}

/*-----------------------------------------------------
 Name    :IAP_64_Make()
 Funciton:slect down load file
 Input   :none
 Output  :none
 Author  :lmm-2014/02/18
 Modify  :[<name>-<data>]
 ------------------------------------------------------*/
int IAP_64_Make() {
	u16 Index = 0;
	u16 ret = 0;
	DateTime time;
	struct QueueIAP message; /*defined in/inc/message.h*/
	memset(&message, 0, sizeof(message));
	message.ID = 0x64;
	Index = 0;
	message.data[Index++] = 0x64;
	message.data[Index++] = Version.Protocol >> 8;
	message.data[Index++] = Version.Protocol;
	message.data[Index++] = Version.Code;

	GetNowTime(&time);
	message.data[Index++] = time.Year;
	message.data[Index++] = time.Month;
	message.data[Index++] = time.Day;
	message.data[Index++] = time.Hour;
	message.data[Index++] = time.Min;
	message.data[Index++] = time.Sec;

	if (IAP.Updata_Type == 0xFE) {
		message.data[Index++] = 0x01;
	} else {
		message.data[Index++] = IAP.Updata_Type;
	}

	ret = Index;
	message.data[Index++] = api_CheckSum(message.data, ret);
	message.data[Index++] = 0x00;

	message.length = Index;
	message.CRC = api_CheckCrc(0xffff, message.data, message.length);

//	GSM_SendGPRS(&message);
	if (QueueOperateOk == QueueIn(&IAPQueue, &message)) {
		printf_iap("message: IAP 64 QueueIn Ok\n");
	} else {
		printf_iap("message: IAP 64 QueueIn error\n");
	}

	return 0;
}

/*-----------------------------------------------------
 Name    :IAP_65_Parse()
 Funciton:prase center data start download message
 Input   :none
 Output  :none
 Author  :lmm-2014/02/18
 Modify  :[<name>-<data>]
 ------------------------------------------------------*/
int IAP_65_Parse(RecMessage *RecMess) {
	u16 Index, val;
	u32 result;
	printf_iap("#############################65#############################\n");
	Index = 0;
	result = RecMess->data[Index++]; /*4 byte IAP.Total_Length*/
	result = (result << 8) | RecMess->data[Index++];
	result = (result << 8) | RecMess->data[Index++];
	result = (result << 8) | RecMess->data[Index++];
	IAP.Total_Length = result;
	printf_iap("65:IAP.Total_Length=%ld\n", (long int)IAP.Total_Length);

	result = RecMess->data[Index++]; /*2 byte IAP.Total_Pack*/
	result = (result << 8) | RecMess->data[Index++];
	IAP.Total_Pack = result;
	printf_iap("65:IAP.Total_Pack=%x\n", IAP.Total_Pack);

	result = RecMess->data[Index++]; /*2 byte IAP.Total_CRC*/
	result = (result << 8) | RecMess->data[Index++];
	IAP.Total_CRC = result;
	printf_iap("65:IAP.Total_CRC=%x\n", IAP.Total_CRC);

	result = RecMess->data[Index++];
	result = (result << 8) | RecMess->data[Index++];
	val = api_CheckCrc(0xffff, RecMess->data, 8);

	if (result != val) {
		IAP.Total_Length = 0;
		IAP.Total_Pack = 0;
		IAP.Total_CRC = 0;
		IAP.Deal_Result = IAP_CRC_ERROR;
	}

	else {
		IAP.Deal_Result = IAP_DEAL_OK; /*this time iap download success*/
	}
	IAP_Flag65 = 0x01;
	printf_iap("65:IAP.Deal_Result=%x\n", IAP.Deal_Result);

	return 0;
}

/*-----------------------------------------------------
 Name    :IAP_66_Make()
 Funciton:make reply message,tell center process[ iap download data] sucess
 Input   :none
 Output  :none
 Author  :lmm-2014/02/18
 Modify  :[<name>-<data>]
 ------------------------------------------------------*/
int IAP_66_Make(void) {
	u16 Index = 0, ret = 0;
	u32 result;
	DateTime time;
	struct QueueIAP message; /*defined in/inc/message.h*/

	memset(&message, 0, sizeof(message));
	message.ID = 0x66;

	Index = 0;
	message.data[Index++] = 0x66;
	message.data[Index++] = Version.Protocol >> 8;
	message.data[Index++] = Version.Protocol;
	message.data[Index++] = Version.Code;

	GetNowTime(&time);
	message.data[Index++] = time.Year;
	message.data[Index++] = time.Month;
	message.data[Index++] = time.Day;
	message.data[Index++] = time.Hour;
	message.data[Index++] = time.Min;
	message.data[Index++] = time.Sec;

	message.data[Index++] = IAP.Total_Pack >> 8;
	message.data[Index++] = IAP.Total_Pack;

	message.data[Index++] = IAP.Deal_Pack >> 8;
	message.data[Index++] = IAP.Deal_Pack;

	message.data[Index++] = IAP.Deal_Result;

	result = api_CheckCrc(0xffff, message.data, Index);
	message.data[Index++] = result >> 8;
	message.data[Index++] = result;
	ret = Index;
	message.data[Index++] = api_CheckSum(message.data, ret);
	message.data[Index++] = 0x00;

	message.length = Index;

	message.CRC = api_CheckCrc(0xffff, message.data, message.length);

	if (QueueOperateOk == QueueIn(&IAPQueue, &message)) {
		printf_iap("message: IAP 66 QueueIn Ok\n");
	} else {
		printf_iap("message: IAP 66 QueueIn error\n");
	}

	return 0;
}

/*-----------------------------------------------------
 Name    :IAP_67_Parse()
 Funciton:prase download content message
 Input   :none
 Output  :none
 Author  :lmm-2014/02/18
 Modify  :[<name>-<data>]
 ------------------------------------------------------*/
int IAP_67_Parse(RecMessage *RecMess) {
	u16 Index, mark, result;
	u8 updatabuf[1500];
	printf_iap("#############################67#############################\n");
	Index = 0;
	result = RecMess->data[Index++];
	result = (result << 8) | RecMess->data[Index++];
	//IAP.Total_Pack =result;
	IAP.Current_Pack = RecMess->data[Index++];
	IAP.Current_Pack = (IAP.Current_Pack << 8) | RecMess->data[Index++];
	printf_iap("67:IAP.Current_Pack=%x\n", IAP.Current_Pack);
	printf_iap("67:IAP.Deal_Pack=%x\n", IAP.Deal_Pack);

	if ((IAP.Current_Pack == IAP.Deal_Pack) && (IAP.Current_Pack <= IAP.Total_Pack)) {
		IAP.Current_Length = RecMess->data[Index++];
		IAP.Current_Length = (IAP.Current_Length << 8) | RecMess->data[Index++];
		printf_iap("67:IAP.Current_Length=%x\n", IAP.Current_Length);
		mark = Index;
		printf_iap("67:mark=%d\n", mark);
		Index += IAP.Current_Length;
		IAP.Current_CRC = RecMess->data[Index++]; /*CRC*/
		IAP.Current_CRC = (IAP.Current_CRC << 8) | RecMess->data[Index++];
		printf_iap("67:IAP.Current_CRC=%x\n", IAP.Current_CRC);
		result = api_CheckCrc(0xffff, (RecMess->data) + mark, IAP.Current_Length);
		printf_iap("67:result_crc=%04x\n", result);

		if (result != IAP.Current_CRC) {
			IAP.Deal_Result = IAP_CRC_ERROR;
		} else {
			IAP.Deal_Result = IAP_DEAL_OK;
		}
	} else {
		IAP.Deal_Result = IAP_PACK_ERROR;
	}
	printf_iap("67:IAP.Deal_Result=%x\n", IAP.Deal_Result);
	if (IAP.Deal_Result == IAP_DEAL_OK) {
		printf_iap("67:clear updatabuf\n");
		memset(updatabuf, 0, sizeof(updatabuf));
		printf_iap("67:copy update \n");
		memcpy(updatabuf, RecMess->data + mark, IAP.Current_Length);
		printf_iap("67:write update to file \n");
		updata_IAPWrite(updatabuf);  //GZY/////
	}

	IAP_Flag67 = 0x01;
	printf_iap("67:This package finished\n");
	return 0;
}

/*-----------------------------------------------------
 Name    :Make_68_Message()
 Funciton:IAP updata state message
 Input   :none
 Output  :none
 Interpretation:
 Author  :lmm-2014/02/18
 Modify  :[<name>-<data>]
 ------------------------------------------------------*/
int IAP_68_Make(u8 val) /*IAP updata state message*/
{
	u16 Index = 0, ret = 0;
	u32 result;
	DateTime time;
	int time_out = 5;

	while (time_out--) {
		if ((state_run == gsm_cur_state) && (IAP.Update_Enable != 0x01)) {
			printf_iap("%s  %d   IAPRet.Finish_Result =0x%0x end\n", __FUNCTION__, __LINE__, IAPRet.Finish_Result);
//						 send_message_creat(IAPRet.Finish_Result);
			break;
		}
		usleep(1000 * 1000);
	}

	struct QueueIAP message;

	memset(&message, 0, sizeof(message));

	Index = 0;
	message.ID = 0x68;
	message.data[Index++] = 0x68;
	message.data[Index++] = Version.Protocol >> 8;
	message.data[Index++] = Version.Protocol;
	message.data[Index++] = Version.Code;

	GetNowTime(&time);
	message.data[Index++] = time.Year;
	message.data[Index++] = time.Month;
	message.data[Index++] = time.Day;
	message.data[Index++] = time.Hour;
	message.data[Index++] = time.Min;
	message.data[Index++] = time.Sec;

	message.data[Index++] = val;
	message.data[Index++] = IAPRet.Finish_Result; /*updata result state*/

	result = api_CheckCrc(0xffff, message.data, Index);
	message.data[Index++] = result >> 8;
	message.data[Index++] = result;
	ret = Index;
	message.data[Index++] = api_CheckSum(message.data, ret);
	message.data[Index++] = 0x00;
	message.length = Index;
	message.CRC = api_CheckCrc(0xffff, message.data, message.length);

//	GSM_SendGPRS(&message);

	printf_iap("68:message.messageID=%02x", message.ID);
	if (QueueOperateOk == QueueIn(&IAPQueue, &message)) {
		printf_iap("message: IAP 68 QueueIn Ok\n");
	} else {
		printf_iap("message: IAP 68 QueueIn error\n");
	}

	/*******************for touchuan start*******************/
	time_t now_time;
		struct tm *p;
		QUE_TDF_QUEUE_MSG Msg;
		memset(&Msg, 0, sizeof(Msg));
		Msg.MsgType = 0x68;
		Msg.Version[0] = Version.Protocol >> 8;
		Msg.Version[1] = Version.Protocol;
		Msg.Version[2] = Version.Code;
		//属性标识  高字节在前
		Msg.Attribute[0] = 0x40;
		Msg.Attribute[1] = 0;
		//获取当前时间 = 系统运行时间+ key on时间（RTC时间）
		time_t  Sys_Run_Time = api_GetSysSecs();
		now_time = Sys_Start_Time + Sys_Run_Time;
		p = localtime(&now_time);
		Msg.Time[0] = p->tm_year - 100;
		Msg.Time[1] = p->tm_mon + 1;
		Msg.Time[2] = p->tm_mday;
		Msg.Time[3] = p->tm_hour;
		Msg.Time[4] = p->tm_min;
		Msg.Time[5] = p->tm_sec;

		int i;
	int	index =0;
		for (i = 0; i <16; i++) {
			Msg.data[index++] = IAPRet.Finish_Result;
		}

		//信息入队列
	     u8 len[4] = { 0 };
		sprintf((char*) len, "%04X", (unsigned int) index); //消息体长度
		api_AscToHex(len, Msg.length, strlen((char*) len));
				//信息入队列
		printf_can("<msg_68_D4_message>message ready in queue %d bytes   IAPRet.Finish_Result=%02x !!! \n", index + 14, IAPRet.Finish_Result);
		PassThrough_in(&Msg);
		/*******************for touchuan  end*******************/


	return 0;
}

/*-----------------------------------------------------
 Name    :Parse_69_Message()
 Funciton:IAP updata download end message
 Input   :none
 Output  :none
 Interpretation:
 Author  :lmm-2014/02/18
 Modify  :[<name>-<data>]
 ------------------------------------------------------*/
int IAP_69_Parse(RecMessage *RecMess) {
	IAP_Flag69 = 0x01;
	printf_iap("#############################69#############################\n");
	return 0;
}

/*-----------------------------------------------------
 Name    :Parse_71_Message()
 Funciton:IAP updata download file set message
 Input   :none
 Output  :none
 Interpretation:
 Author  :zjx-2018/08/11
 Modify  :[<name>-<data>]
 ------------------------------------------------------*/
int IAP_71_Parse(RecMessage *RecMess) {
	int Index = 0;
	iap_set.order_type = RecMess->data[Index++];  //00-更新文件；01-创建文件或路径；02-删除文件或路径；
	iap_set.path_len = RecMess->data[Index++];
	if ((iap_set.path_len <= 64) && (iap_set.order_type == 0)) {
		memset(iap_set.file_path, 0, max_len);
		memcpy(iap_set.file_path, RecMess->data + Index, iap_set.path_len);

		memset(iap_set.whole_path, 0, whole_len);
		memcpy(iap_set.whole_path, RecMess->data + Index, iap_set.path_len);
		Index += iap_set.path_len;
	} else if (iap_set.path_len <= 64) {
		memset(iap_set.temp_path, 0, max_len);
		memcpy(iap_set.temp_path, RecMess->data + Index, iap_set.path_len);

		memset(iap_set.whole_temp_path, 0, whole_len);
		memcpy(iap_set.whole_temp_path, RecMess->data + Index, iap_set.path_len);
		Index += iap_set.path_len;
	}
	iap_set.name_len = RecMess->data[Index++];
	if ((iap_set.name_len <= 64) && (iap_set.order_type == 0)) {
		memset(iap_set.file_name, 0, max_len);
		memcpy(iap_set.file_name, RecMess->data + Index, iap_set.name_len);
		Index += iap_set.name_len;
		strcat(iap_set.whole_path, iap_set.file_name);
	} else if (iap_set.name_len <= 64) {
		memset(iap_set.file_name, 0, max_len);
		memcpy(iap_set.file_name, RecMess->data + Index, iap_set.name_len);
		Index += iap_set.name_len;
		strcat(iap_set.whole_temp_path, iap_set.file_name);
	}
	memcpy(iap_set.property, RecMess->data + Index, 3);
	Index += 3;

	IAP_72_Make(RecMess);
	Order_Handle();
	return 0;
}
/*-----------------------------------------------------
 Name    :IAP_72_Make()
 Funciton: reply to IAP updata download file set message
 Input   :none
 Output  :none
 Interpretation:
 Author  :zjx-2018/08/11
 Modify  :[<name>-<data>]
 ------------------------------------------------------*/
int IAP_72_Make(RecMessage *RecMess) {
	u16 Index = 0, ret = 0, index=0, i;
	DateTime time;
	struct QueueIAP message; /*defined in/inc/message.h*/
	memset(&message, 0, sizeof(message));
	message.ID = 0x72;

	Index = 0;
	message.data[Index++] = 0x72;
	message.data[Index++] = Version.Protocol >> 8;
	message.data[Index++] = Version.Protocol;
	message.data[Index++] = Version.Code;

	GetNowTime(&time);
	message.data[Index++] = time.Year;
	message.data[Index++] = time.Month;
	message.data[Index++] = time.Day;
	message.data[Index++] = time.Hour;
	message.data[Index++] = time.Min;
	message.data[Index++] = time.Sec;

	message.data[Index++] = iap_set.order_type;
	message.data[Index++] = iap_set.path_len;
	index = 2;
	for (i = 0; i < iap_set.path_len; i++) {
		message.data[Index++] = RecMess->data[index++];
	}
	message.data[Index++] = iap_set.name_len;
	index++;
	for (i = 0; i < (iap_set.name_len + 3); i++) {
		message.data[Index++] = RecMess->data[index++];
	}
//--------------------------------------
	ret = Index;
	message.data[Index++] = api_CheckSum(message.data, ret);
	message.data[Index++] = 0x00;
	message.length = Index;
	message.CRC = api_CheckCrc(0xffff, message.data, message.length);

//	GSM_SendGPRS(&message);
	if (QueueOperateOk == QueueIn(&IAPQueue, &message)) {
		printf_iap("QueueIn Ok\n");
	} else {
		printf_iap("QueueIn error\n");
	}

	/*******************for touchuan  start*******************/
	time_t now_time;
	struct tm *p;
	QUE_TDF_QUEUE_MSG Msg_72;
	memset(&Msg_72, 0, sizeof(Msg_72));
	Msg_72.MsgType = 0x72;
	Msg_72.Version[0] = Version.Protocol >> 8;
	Msg_72.Version[1] = Version.Protocol;
	Msg_72.Version[2] = Version.Code;
	//属性标识  高字节在前
	Msg_72.Attribute[0] = 0x40;
	Msg_72.Attribute[1] = 0;
	//获取当前时间 = 系统运行时间+ key on时间（RTC时间）
	time_t  Sys_Run_Time = api_GetSysSecs();
	now_time = Sys_Start_Time + Sys_Run_Time;
	p = localtime(&now_time);
	Msg_72.Time[0] = p->tm_year - 100;
	Msg_72.Time[1] = p->tm_mon + 1;
	Msg_72.Time[2] = p->tm_mday;
	Msg_72.Time[3] = p->tm_hour;
	Msg_72.Time[4] = p->tm_min;
	Msg_72.Time[5] = p->tm_sec;

    int  index_m =0;
	index = 2;
	Msg_72.data[index_m++] = iap_set.order_type;
	Msg_72.data[index_m++] = iap_set.path_len;
	for (i = 0; i < iap_set.path_len; i++) {
		Msg_72.data[index_m++] = RecMess->data[index++];
	}
	Msg_72.data[index_m++] = iap_set.name_len;
	index++;
	for (i = 0; i < (iap_set.name_len + 3); i++) {
		Msg_72.data[index_m++] = RecMess->data[index++];
	}

	//信息入队列
//	 Msg_72.data[0] = 1;
     u8 len[4] = { 0 };
	sprintf((char*) len, "%04X", (unsigned int) index_m); //消息体长度
	api_AscToHex(len, Msg_72.length, strlen((char*) len));
			//信息入队列
	printf_can("<msg_72_D4_message>message ready in queue %d bytes  iap_set.file_name=%s !!! \n", index_m + 14, iap_set.file_name);
	PassThrough_in(&Msg_72);
	/*******************for touchuan  end*******************/
	return 0;
}

int Order_Handle() {
	FILE *fp;

	if (iap_set.order_type == 0) {

		fp = fopen(iap_set.whole_path, "r"); //只供读取
		if (fp == NULL) {
			if (debug_value & 0x40) {
				perror("update fopen");
			}
			IAPRet.Finish_Type = 0xF9;
			IAPRet.Finish_Result = IAP_PROGRAM_ERROR;
			IAP_68_Make(IAPRet.Finish_Type);
			return (-2);
		}
		fclose(fp);
		update_file();

	} else if (iap_set.order_type == 1) {
		create_file();
	} else if (iap_set.order_type == 2) {
		delete_file();
	} else {
		printf_iap("an invalid order_type");
		return -1;
	}
	return 0;
}
/*
 function: update(mv) the file from temp path to execution path
 无论文件成功更新与否，都回复一条68信息作为文件更新结果信息给web；
 */

int update_file() {
	int ret1;
	FILE *fp;

	fp = fopen(Temp_Path, "r"); //只供读取
	if (fp == NULL) {
		perror("update fopen");
		IAPRet.Finish_Type = 0xF9;
		IAPRet.Finish_Result = IAP_FILE_ERROR;
		IAP_68_Make(IAPRet.Finish_Type);

		return 1;
	}
	fclose(fp);

	ret1 = update_SystemCmd2("mv -f", Temp_Path, iap_set.whole_path);
	if (ret1 == 0) {
		IAPRet.Finish_Type = 0xF9;
		IAPRet.Finish_Result = IAP_DEAL_OK;
		IAP_68_Make(IAPRet.Finish_Type);
		printf_iap("<%s> :ready inter sleep...\n", __FUNCTION__);
		update_iap = IAP_Update_END;
		gsm_cur_state = state_sleep;  // linux goto poweroff
		return 0;
	} else {
		IAPRet.Finish_Type = 0xF9;
		IAPRet.Finish_Result = IAP_UNKNOWN;
		IAP_68_Make(IAPRet.Finish_Type);
		return 1;
	}

}

/*
 FUNCTION:1创建路径；2创建文件；3分配权限
 */
int create_file() {
	int ret1, ret2, ret3;
	FILE *fpw = NULL; // fp of write

	ret1 = update_SystemCmd1("mkdir -p", iap_set.temp_path);
	//ret2 = update_SystemCmd1("touch", iap_set.whole_temp_path);
	fpw = fopen(iap_set.whole_temp_path, "r"); //只供读取
	if (fpw != NULL) {
		fclose(fpw);
		IAPRet.Finish_Type = 0xF9;
		IAPRet.Finish_Result = IAP_CRAT_EXIT;
		IAP_68_Make(IAPRet.Finish_Type);
		return 0;
	} else {
		perror("create_file");
	}

	fpw = fopen(iap_set.whole_temp_path, "wb+");
	if (fpw == NULL) {
		printf_iap(iap_set.whole_temp_path);
		perror(" create_file fopen faile");
		ret2 = -1;
	} else {
		ret2 = 0;
	}
	fclose(fpw);

	ret3 = update_SystemCmd2("chmod", iap_set.property, iap_set.whole_temp_path);
	if (ret1 || ret2 || ret3 != 0) {
		IAPRet.Finish_Type = 0xF9;
		IAPRet.Finish_Result = IAP_CRAT_ERROR;
		IAP_68_Make(IAPRet.Finish_Type);
		return 1;
	} else {
		IAPRet.Finish_Type = 0xF9;
		IAPRet.Finish_Result = IAP_DEAL_OK;
		IAP_68_Make(IAPRet.Finish_Type);
		return 0;
	}
}

/*
 function: delete the temp file
 */
int delete_file() {
	int ret = -1;
	ret = update_SystemCmd1("rm -fr", iap_set.whole_temp_path); //delete file
	return ret;
}

