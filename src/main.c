/*
 * main.c
 *
 *  Created on: 2018年12月25日
 *      Author: tykj
 */

#ifndef MAIN_C_
#define MAIN_C_

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sched.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <signal.h>

#include "api.h"
#include "bluetooth.h"
#include "candriver.h"
#include "candata.h"
#include "canhandle.h"
#include "iap.h"
#include "gpsdeal.h"
#include "gsm.h"
#include "sys_manage.h"
#include "message_process.h"

//版本号自定义 版本更新实时修改
struct Version_con Version = { 0x1073, 0x01 };

unsigned int debug_value = 255;
unsigned char debug_str[10] = { 0 };
unsigned char debug_flag = 0;

TDF_PTHREAD_PARAM main_PthParam; //看门狗线程
TDF_PTHREAD_PARAM gsm_PthParam; //GSM模块线程
TDF_PTHREAD_PARAM SockRecv_PthParam; //GPRS socket RECV线程
TDF_PTHREAD_PARAM SockSend_PthParam; //GPRS socket SEND线程
TDF_PTHREAD_PARAM gps_PthParam;
TDF_PTHREAD_PARAM sys_PthParam;
TDF_PTHREAD_PARAM can_PthParam; //can线程
TDF_PTHREAD_PARAM can_PthParamA; //CanData_sockA
TDF_PTHREAD_PARAM can_PthParamRecvB;
TDF_PTHREAD_PARAM can_PthParamSendB;
TDF_PTHREAD_PARAM can_PthParamRecvC;
TDF_PTHREAD_PARAM can_PthParamSendC;
TDF_PTHREAD_PARAM iap_PthParam;
TDF_PTHREAD_PARAM bt_PthParam;
TDF_PTHREAD_PARAM Camera_PthParam;
TDF_PTHREAD_PARAM Alarm_PthParam;
TDF_PTHREAD_PARAM Message_PthParam;
TDF_PTHREAD_PARAM Cand_Msg_PthParam;
TDF_PTHREAD_PARAM shell_PthParam;
TDF_PTHREAD_PARAM Wdg_PthParam;

pthread_mutex_t CanAmutex;
pthread_mutex_t CanBmutex;
pthread_mutex_t CanCmutex;
pthread_mutex_t InterStaQuemutex;
pthread_mutex_t PassThroughQuemutex;
pthread_mutex_t can_vcumutex;
pthread_mutex_t ATCmdMutex;
pthread_mutex_t LockSetMutex;

u8 hw_dog_restflg = 0;
void main_WatchPthread(TDF_PTHREAD_PARAM *PthParam, int timeLen, void *(*start_routine)(void *), const char *dbgStr) {
	time_t nowtime_t = api_GetSysSecs();

	if (PthParam->sta == 0) {
		pthread_attr_init(&PthParam->attr);
		pthread_attr_setschedpolicy(&PthParam->attr, SCHED_OTHER);
		pthread_attr_setdetachstate(&PthParam->attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&PthParam->id, &PthParam->attr, (*start_routine), NULL);
		PthParam->lastOKtime_t = nowtime_t;
	} else {
		if (PthParam->sta == 1) {
			PthParam->sta = 2; // 监视任务设置任务标志为2，被监视任务自动设置为1
			PthParam->lastOKtime_t = nowtime_t;
		} else {
			if (nowtime_t - PthParam->lastOKtime_t > timeLen) {
				pthread_cancel(PthParam->id);
				close(PthParam->id);
				PthParam->id = 0;
				PthParam->sta = 0;
				PthParam->lastOKtime_t = nowtime_t;
				PthParam->rest_count++;
				printf("------------------errTimer>%d close PthParam-%s\n", timeLen, dbgStr);
			}
		}
	}
}

void *main_PthreadManagement(void *data) {
	main_PthParam.sta = 1;
	printf(" %s start \n", __FUNCTION__);

	if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) != 0) {
		perror("Management_Pthread:setcancelstate\n");
	}

	if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0) {
		perror("Management_Pthread:setcanceltype\n");
	}
	while (main_PthParam.flag) {
		main_PthParam.sta = 1;
//		main_WatchPthread(&can_PthParamA, MAIN_TIMER_SEC(60), CanData_sockA, "CANAsock");
		main_WatchPthread(&can_PthParam, MAIN_TIMER_SEC(60), canh_TransProcess, "CANTransProcess");
//		main_WatchPthread(&can_PthParamRecvB, MAIN_TIMER_SEC(60), CanData_SockRecvB, "CANBrecv");
//		main_WatchPthread(&can_PthParamSendB, MAIN_TIMER_SEC(60), CanData_SockRecvC, "CANCrecv");
//		main_WatchPthread(&can_PthParamRecvC, MAIN_TIMER_SEC(60), CanData_SockSendB, "CANBsend");
//		main_WatchPthread(&can_PthParamSendC, MAIN_TIMER_SEC(60), CanData_SockSendC, "CANCsend");
		main_WatchPthread(&gsm_PthParam, MAIN_TIMER_SEC(60), gsm_Pthread, "gsmSock");
		main_WatchPthread(&gps_PthParam, MAIN_TIMER_SEC(60), gps_Pthread, "gpsSock");
		main_WatchPthread(&sys_PthParam, MAIN_TIMER_SEC(60), sys_Pthread, "sysSock");
		main_WatchPthread(&iap_PthParam, MAIN_TIMER_SEC(60), iap_Pthread, "iapSock");
		main_WatchPthread(&bt_PthParam, MAIN_TIMER_SEC(60), bt_Pthread, "btSock");
		bt_PthParam.sta = 1;
		sleep(1);
	}

	printf("############## %s exit ##############\n", __FUNCTION__);
	return NULL;
}
//需要设置到一些系统参数
void main_RunParamReset(void) {
//	Central_set_Default();
//	Queue_init();
	sys_init(); //读e2p系统参数
	PrmID_rxq_device_init();
	CAN_Open();
//	Sys_CanSet_Default(); //设置PGN默认参数
//	Sys_CanSet_BTDefault();
//	j1939_protocol_init();
//	default_led_contrl();
	queue_init();
//	BTqueue_init();

//	System.DaySummary.RestartCount++; //当天上电复位次数
//	System.DaySummary.Valid = 1;

}

int wtd_fd = 0; //硬件狗
int wtd_ret = 0; //喂狗结果
int main(int argc, char *argv[]) {
	int debugFd;
	printf("***************APP version %04X%02X start!****************\n", Version.Protocol, Version.Code);
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, 0);

	main_RunParamReset(); //init
	system("/etc/init.d/mount-opt start");
	//任务线程管理
	main_PthParam.flag = TRUE;
	struct sched_param ManagementSchedValue;
	pthread_attr_init(&main_PthParam.attr);
	pthread_attr_setschedpolicy(&main_PthParam.attr, SCHED_RR);
	ManagementSchedValue.sched_priority = sched_get_priority_max(SCHED_RR);
	pthread_attr_setschedparam(&main_PthParam.attr, &ManagementSchedValue);
	pthread_attr_setdetachstate(&main_PthParam.attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&main_PthParam.id, &main_PthParam.attr, main_PthreadManagement, 0);
#if 0
	wtd_fd = open("/dev/watchdog", O_RDWR); //打开硬件狗
	if (wtd_fd < 0)
		printf("Open watchdog failed!\n");
	else
		printf("watchdog ID:%d\n", wtd_fd);
#endif
	while (1) {
#if 0
		if (wtd_fd > 0 && hw_dog_restflg != 1) {
			wtd_ret = ioctl(wtd_fd, WDIOC_KEEPALIVE, 0); //喂硬狗
			if (wtd_ret < 0) {
				printf("wtd feed failed!\n");
				close(wtd_fd);
				wtd_fd = 0;
			}
//			else
//				printf("program feed dog!\n");
		} else {
//			printf("wtd will closed!\n");
		}
#endif
		sleep(1);
		//软看门狗任务
		main_WatchPthread(&main_PthParam, MAIN_TIMER_SEC(60), main_PthreadManagement, "wdg");

		if ((access("/tmp/debug", F_OK) != -1) && (debug_flag == 0)) {
			debugFd = open("/tmp/debug", O_RDWR);
			read(debugFd, debug_str, sizeof(debug_str));
			printf("debug_str %s\n", debug_str);
			debug_value = atoi((char *) debug_str);
			printf("*******************************************************debug_value %d\n", debug_value);
			debug_flag = 1;
		} else if ((access("/tmp/debug", F_OK) == -1) && (debug_flag == 1)) {
			debug_flag = 0;
			debug_value = 0;
		}
	}
	return 0;
}
#endif /* MAIN_C_ */
