/*
 * bluetooth.c
 *
 *  Created on: 2018年1月13日
 *      Author: tykj
 */

#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "api.h"
#include "bluetooth.h"
#include "rfcomm.h"
#include "general.h"
#include "gsm.h"
#include "TCPsocket.h"
#include "message_process.h"
#include "api.h"
#include "Time.h"

u8 enable_bt = 0;
#define enable_bt_pwd "/etc/enable_bt.sh"
#define bt_server_pwd  "/etc/bt_server_init.sh"
#define BT_PSW_KEY_VALUE  0x95D7   //公用密码，用于认证码演算

//ascii 对照码  G-47  R-52  S-53  E-45
#define BT_Rx_Time_Type1   0x47
#define BT_Rx_Time_Type2   0x52
#define BT_Rx_Res_Type1	0x53
#define BT_Rx_Res_Type2	0x45

#define BT_TX_MAX  1500
#define BT_RX_MAX 1500
u8 BT_CMD_Buffer[BT_TX_MAX]; //蓝牙发送缓存
u8 BT_Rx_key[BT_RX_MAX]; //蓝牙接收缓存

u8 KEY_CODE[2] = { 0 };
u16 BT_Random_NUM; //随机数
u8 BTstate_flag = 0; //蓝牙状态标志位：0：等待链接 1；已链接
u8 Identity_Step = 0; //身份认证标志位： 0: 未认证身份\r认证失败  1：终端发送认证信息  2：收到头盔端认证  3：认证成功

int socket_bt();
int bluetooth_snd(void);
int BT_tx_Identity();
int BT_Rx_Identity(u8 *data, u32 len);
//int SleepFlag = 0;

#define HCIDEVDOWN 	_IOW('H', 202, int)
#define BTPROTO_HCI 1
#define SCAN_DISABLED		0x00
#define HCISETSCAN _IOW('H', 221, int)
struct hci_dev_req {
	u16 dev_id;
	u32 dev_opt;
};

void *bt_Pthread(void *argv) {
	printf_bt("############## %s start ##############\n", __FUNCTION__);
//	system("hciconfig hci0 name 222");
	memcpy(BTidentity_Info.BThead, "TY0001-", 7);
	BTidentity_Info.BT_pwd_key[0] = 0x95;
	BTidentity_Info.BT_pwd_key[1] = 0xD7;
	while (1) {
		if (access("/tmp/bt_start", F_OK) != -1)
			socket_bt();
		else {
			sleep(1);
		}
		sleep(1);
	}
	printf_bt("############## %s exit ##############\n", __FUNCTION__);
	return 0;
}
/*
 蓝牙发送D4信息不需要压缩，所以单独拼串
 蓝牙数据处理函数
 */
int BtMsg_spell(void* Msg, u8* SpellStr) {
//	printf("\n Spellstr:");
//	api_PrintfHex(Msg, 14);
	memcpy(SpellStr, Msg, 12); //前12字节包括类型、版本号、属性、时间
	u8 asclen[2] = { 0 };
	memcpy(asclen, Msg + 12, 2); //长度值需要换成十进制使用
	u8 len[4] = { 0 };
	api_HexToAsc(asclen, len, 2); //先转成文本格式
	int n = HexStr2Int((char*) len, 4); //十六进制串转成十进制

	u8 buf1[1520] = { 0 };

	memcpy(buf1, Msg + 14, n);
	int m = n * 8;
	sprintf((char*) len, "%04X", (unsigned int) m);
	api_AscToHex(len, asclen, 4);
	//printf("asclen:");
	//api_PrintfHex(asclen,2);
	//api_PrintfHex(buf2, m);
	memcpy(SpellStr + 12, asclen, 2); //长度值拼入串
	memcpy(SpellStr + 14, buf1, n); //消息体拼入串

	int Msg_len = 14 + n;
	//printf("\n Spellstr:");
	//api_PrintfHex(SpellStr, Msg_len);
	return Msg_len;
}

/**
 *read_timeout - 读超时检测函数, 不包含读操作
 *@fd: 文件描述符
 *@waitSec: 等待超时秒数, 0表示不检测超时
 *成功(未超时)返回0, 失败返回-1, 超时返回-1 并且 errno = ETIMEDOUT
 **/

int read_timeout(int fd, long waitSec, int flag) {
	int returnValue = 0;
	if (waitSec > 0) {
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(fd, &readSet); //添加

		if (flag == 0) {
			struct timeval waitTime;
			waitTime.tv_sec = waitSec;
			waitTime.tv_usec = 0; //将微秒设置为0(不进行设置),如果设置了,时间会更加精确
			do {
				returnValue = select(fd + 1, &readSet, NULL, NULL, &waitTime);
			} while (returnValue < 0 && errno == EINTR); //等待被(信号)打断的情况, 重启select
		} else if (flag == 1) {
			struct timeval waitTime1;
			waitTime1.tv_sec = 0;
			waitTime1.tv_usec = 50 * 1000; //将微秒设置为0(不进行设置),如果设置了,时间会更加精确
			do {
				returnValue = select(fd + 1, &readSet, NULL, NULL, &waitTime1);
			} while (returnValue < 0 && errno == EINTR); //等待被(信号)打断的情况, 重启select
		} else
			return -1;

		if (returnValue < 1) //在waitTime时间段中一个事件也没到达，超时
		                {
			printf_bt("return value:%d \n", returnValue);
			returnValue = -1; //返回-1
			errno = ETIMEDOUT;
		} else if (returnValue == 1) //在waitTime时间段中有事件产生
			returnValue = 0; //返回0,表示成功
		// 如果(returnValue == -1) 并且 (errno != EINTR), 则直接返回-1(returnValue)
	}

	return returnValue;
}

/**
 *write_timeout - 读超时检测函数, 不包含读操作
 *@fd: 文件描述符
 *@waitSec: 等待超时秒数, 0表示不检测超时
 *成功(未超时)返回0, 失败返回-1, 超时返回-1 并且 errno = ETIMEDOUT
 **/
int write_timeout(int fd, long waitSec) {
	int returnValue = 0;
	if (waitSec > 0) {
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(fd, &readSet); //添加

		struct timeval waitTime;
		waitTime.tv_sec = waitSec;
		waitTime.tv_usec = 0; //将微秒设置为0(不进行设置),如果设置了,时间会更加精确
		do {
			returnValue = select(fd + 1, NULL, &readSet, NULL, &waitTime);
		} while (returnValue < 0 && errno == EINTR); //等待被(信号)打断的情况, 重启select

		if (returnValue < 1) //在waitTime时间段中一个事件也没到达，超时
		                {
			returnValue = -1; //返回-1
			errno = ETIMEDOUT;
		} else if (returnValue == 1) //在waitTime时间段中有事件产生
			returnValue = 0; //返回0,表示成功
		// 如果(returnValue == -1) 并且 (errno != EINTR), 则直接返回-1(returnValue)
	}

	return returnValue;
}

int s, ctl;
int socket_bt() {

	struct sockaddr_rc loc_addr = { 0 };
	int result;

	int ret = -1;

	ret = access("/etc/rcS.d/S69wifi", F_OK);
	if (ret == 0) {
		system("wr enable");
		system("ifconfig wlan0 down");
		system("mkdir -p /lib/modules/3.10.101");
		system("rmmod bcmdhd");
		remove("/etc/rcS.d/S69wifi");
		system("wr disable");
	}
	/*创建socket */
	printf_bt("Creating socket...\n");
	usleep(1000);
	s = socket(PF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	if (s < 0) {
		if (debug_value & 0x10) {
			perror("create socket error");
		}
		return 1;
	} else {
		printf_bt("create socket success!\n");
	}

	loc_addr.rc_family = AF_BLUETOOTH;
	loc_addr.rc_bdaddr = *BDADDR_ANY;
	loc_addr.rc_channel = (uint8_t) 1;

	/*绑定socket*/
	printf_bt("Binding socket...\n");
	result = bind(s, (struct sockaddr *) &loc_addr, sizeof(loc_addr));
	if (result < 0) {
		if (debug_value & 0x10) {
			perror("bind socket error:");
		}
		return 1;
	} else {
		printf_bt("bind success!\n");
	}

	printf_bt("Listen... \n");
	result = listen(s, 5);
	if (result < 0) {
		printf_bt("error:%d\n:", result);
		if (debug_value & 0x10) {
			perror("listen error:");
		}
		close(s);
		s = -1;
		return 1;
	} else {
		printf_bt("requested!\n");
		bluetooth_snd();
	}
	return 0;
}

uint8_t gprs_send[1700] = { 0 };
uint8_t buf_spell[1700] = { 0 };
uint8_t buf_send[1700] = { 0 };
int s;

int bluetooth_snd(void) {
	int nbuf = 0;
	u8 strIMEI[9] = { 0 }; //IMEI号拼串用

//		int id;
//	uint16_t i, leg_str;
	struct sockaddr_rc rem_addr = { 0 };

	char buf[1024] = { 0 }; //,*addr;
	u8 Rx_buf[1700] = { 0 };
	int client, bytes_read;
	socklen_t opt = sizeof(rem_addr);
	time_t timep;
	int authed, conned;

	int ret;
	BTstate_flag = 0;
	//	struct timeval timeout = {3,0};
	while (1) {
		/*Accept*/
		printf_bt("Accepting...\n");
		conned = 0;
		authed = 0;
		Identity_Step = 0;

		client = accept(s, (struct sockaddr *) &rem_addr, &opt);
		printf_bt("accept  client =%d \n", client);
		if (client < 0) {
			if (debug_value & 0x10) {
				perror("accept\n");
			}
			conned = 0;
		} else {
			printf_bt("accept OK!\n");
//			ba2str(&rem_addr.rc_bdaddr, buf);
//			printf_bt("accepted connection from %s \n", buf);
			conned = 10;

		}
//		usleep(50 * 1000);
		//		setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(struct timeval));
		while (conned) {
			Identity_Step = 0;
			int ret_index;
			ret_index = BT_tx_Identity();
			if (ret_index == 0) {
				printf_bt("Wait for get IMEI  !\n");
				sleep(1);
				continue;
			}
//					write(client,"TYKJ:0x95D7",9);
			write(client, BT_CMD_Buffer, ret_index);
			printf_bt("BT_tx_Identity:");
			api_PrintfHex(BT_CMD_Buffer, ret_index);
			Identity_Step++;
			printf_bt("wait for authentication in 5S\n");
			authed = 0;

			ret = read_timeout(client, 5, 0);
			// 身份认证
			if (ret == 0) {
				memset(Rx_buf, 0, sizeof(Rx_buf));
				bytes_read = read(client, Rx_buf, sizeof(Rx_buf));
				Identity_Step++;
				printf_bt("Rx_buf:%d\n", bytes_read);
				if (debug_value & 0x10) {
					api_PrintfHex(Rx_buf, bytes_read);
				}
				if (bytes_read > 0) {
					printf_bt("read:%c%c%c%c %02x%02x %02x %02x%02x\n", Rx_buf[0], Rx_buf[1], Rx_buf[2], Rx_buf[3], Rx_buf[4], Rx_buf[5], Rx_buf[6], Rx_buf[7], Rx_buf[8]);
					if (Identity_Step <= 2) {
						//身份校验
						BT_Rx_Identity(Rx_buf, bytes_read);
					}
					u8 BT_buf[6] = { 0 };
					memcpy(BT_buf, "TYKJ-", 5);
					if (Identity_Step == 3) {
						printf_bt("authentication success\n");
						BT_buf[5] = 0;
						write(client, BT_buf, 6);
						authed = 10;
						BTstate_flag = 1;
						break;
					} else {
						BT_buf[5] = 1;
						write(client, BT_buf, 6);
						conned--;
					}
				}
			} else {
				printf_bt("authentication timeout %d\n", conned);
				conned--;
//				usleep(950 * 1000);
			}

		}
		while (authed) {
//			usleep(10 * 1000);
			//信息出队列
			int msglen = 0;
			memset(gprs_send, 0x00, 1700);
			if (BTrngbuf_count > 0)
				msglen = BTqueue_out(&gprs_send[0]);
			else {
//			        printf("BTrngbuf_count = %d\n",BTrngbuf_count);
				usleep(10 * 1000);
				continue;
			}
			if (msglen == 0) {
//			       printf("msglen= %d\n",msglen);
				BTqueue_delete();
				continue;
			}
			if (gprs_send[0] != 0xD4) {
//			        printf("gprs_send[0] = %x\n",gprs_send[0]);
				BTqueue_delete();
				continue;
			}
			//发送前添加压缩转义
			memset(buf_spell, 0X00, 1700);
			memset(buf_send, 0X00, 1700);
			nbuf = BtMsg_spell(&gprs_send, buf_spell); //数据拼串
			if (strIMEI[0] == 0) {
				Get_IMEI(strIMEI); //获取IMEI号
				if (strIMEI[0] == 0)
					continue;
			}
			memcpy(buf_send, strIMEI, 9); //IMEI号
			memcpy(buf_send + 9, buf_spell, nbuf); //队列中消息
			buf_send[nbuf + 9] = api_CheckSum(buf_send, nbuf + 9); //添加CRC校验码
			nbuf = Msg_Convert(buf_send, nbuf + 10); //转义并添加7E头尾
//			printf("\nBT msg:");
//			api_PrintfHex(buf_send, nbuf);
			ret = write_timeout(client, 1); //添加写超时判断
			if (ret == 0) {
				int len = write(client, &buf_send, nbuf);
				authed = 10;
				if (len > 0) {
					BTqueue_delete();
					printf_bt("bluetooth write %d byte!\n", len);
					Get_Now_Time();
					//plog(LOG_DEBUG,buf_send);
//					usleep(300 * 1000);
				} else {
//					close(client);
					printf_bt("send data error ,bluetooth disconnected, and wait for reconnect ! \n");
					break;
				}
			} else {
				printf_bt(" write timeout %d\n", authed);
				authed -= 5;
				if (authed <= 0) {
					printf_bt("disconnect:");
					time(&timep);
					printf_bt("time: %s\n", asctime(gmtime(&timep)));
					break;
				}
			}

			ret = read_timeout(client, 1, 1);
			if (ret == 0) {
				memset(buf, 0, sizeof(buf));
				bytes_read = read(client, buf, sizeof(buf));
				if (bytes_read > 0) {
					authed = 10;
					printf_bt("read:%s\n", buf);
				} else {
					if (bytes_read < 0 && (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)) {
						continue;
					} else {
						printf_bt("The server disconnects, the client program needs exit and restart ! \n");
						authed = 0;
						break;
					}
				}
			} else {
				printf_bt("recive timeout %d\n", authed);
				authed--;
				if (authed == 0) {
					printf_bt("disconnect\n");
					time(&timep);
					printf_bt("time: %s\n", asctime(gmtime(&timep)));
				}
			}
		}
		close(client);
		client = 0;
		sleep(1);
	}
	close(s);
	s = -1;
	return 0;
}

/******************************************************************
 * 蓝牙身份认证
 * 依据协议：终端蓝牙通信协议_V0.04 修订1212
 * 日期		  ：2018-1-19
 * ***************************************************************/
/*******************
 * 蓝牙身份认证1:获取随机数
 *******************/

u16 CAND_uwGetPsw(void) {
	u16 uwPsw;
	uwPsw = BT_Random_NUM ^ BT_PSW_KEY_VALUE;
	uwPsw >>= 6;

	KEY_CODE[0] = uwPsw >> 8;
	KEY_CODE[1] = uwPsw;
	printf("KEY_CODE:");
	api_PrintfHex(KEY_CODE, 2);
	return (uwPsw);
}
u8 BT_Tx_Time_Standard = 0;
/**************************
 * 终端蓝牙发送身份认证信息
 * *********************/
int BT_tx_Identity() {
	int index = 0;
//	int i = 0;
	memset(BT_CMD_Buffer, 0x00, BT_TX_MAX);
	srand(time(NULL));
	BT_Random_NUM = rand();
	DateTime time;
	GetNowTime(&time);
//	BT_CMD_Buffer[index++] = 'T';
//	BT_CMD_Buffer[index++] = 'Y';
//	BT_CMD_Buffer[index++] = '0';
//	BT_CMD_Buffer[index++] = '0';
//	BT_CMD_Buffer[index++] = '0';
//	BT_CMD_Buffer[index++] = '1';
//	BT_CMD_Buffer[index++] = '-';
	memcpy(BT_CMD_Buffer, BTidentity_Info.BThead, 7);
	index += 7;
	BT_CMD_Buffer[index++] = BT_Random_NUM >> 8;
	BT_CMD_Buffer[index++] = BT_Random_NUM;
//	memcpy(BT_CMD_Buffer,BTidentity_Info.BT_pwd_key,2);
//	index += 2;
	CAND_uwGetPsw();
	BT_CMD_Buffer[index++] = 0X0F;
	if (gsm_info_cur.imei[0] == 0)
		return 0;
	memcpy(BT_CMD_Buffer + index, gsm_info_cur.imei, 8);
//	for(i = 0; i < index + 8; i++) {
//		printf("buff %02X", BT_CMD_Buffer[i]);
//	}
//	printf("\n");
//	if (System.GPS.Time.Valid != 0) //gps 定位有效
//	                {
//		BT_CMD_Buffer[index++] = 'G';
//		BT_Tx_Time_Standard = 1;
//	} else {
//		BT_CMD_Buffer[index++] = 'R';
//		BT_Tx_Time_Standard = 0;
//	}
//	BT_CMD_Buffer[index++] = time.Year;
//	BT_CMD_Buffer[index++] = time.Month;
//	BT_CMD_Buffer[index++] = time.Day;
//	BT_CMD_Buffer[index++] = time.Hour;
//	BT_CMD_Buffer[index++] = time.Min;
//	BT_CMD_Buffer[index++] = time.Sec;

	return index + 8;
}
/***************************
 * 蓝牙应答认证
 * *************************/
int BT_Rx_Identity(u8 *data, u32 len) {
	memset(BT_Rx_key, 0x00, 9);
	u16 passwd = 0;
	u8 imeiHighTmp = 0;
	u8 imeiLowTmp = 0;
	u16 imeiTmp = 0;
	int index = 0;
//	char Temp[6];
	//ascii 对照码  G-47  R-52  S-53  E-45
//	u8 BT_G_R[4]= {0x47,0x52,0x53,0x45};
	int i = 0;
	if (len < 9) {
		printf_bt("bluetooth Identity answer error!! \n");
		return -1;
	}
	///int rx_index
	//密码数据
	BT_Rx_key[0] = data[index++];
	BT_Rx_key[1] = data[index++];
	BT_Rx_key[2] = data[index++];
	BT_Rx_key[3] = data[index++];
	BT_Rx_key[4] = data[index++];
	BT_Rx_key[5] = data[index++];
	BT_Rx_key[6] = data[index++];
	BT_Rx_key[7] = data[index++];
	BT_Rx_key[8] = data[index];

	if (((BT_Rx_key[0] == 'K') && (BT_Rx_key[1] == 'E') && (BT_Rx_key[2] == 'Y') && (BT_Rx_key[3] == '-')) == 0) {
		printf_bt("product info error\n");
		return -1;
	}
	if (debug_value & 0x10)
		api_PrintfHex(KEY_CODE, 2);
	if (((BT_Rx_key[4] == KEY_CODE[0]) && (BT_Rx_key[5] == KEY_CODE[1])) == 0) {
		printf_bt(" BT_Rx_Identity KEY_CODE error !!   \n");
		return -1;
	}
	if (BT_Rx_key[6] == 0XA1) {
		printf_bt("super authority:\n");
		passwd = BT_Random_NUM + ((KEY_CODE[0] << 8) + KEY_CODE[1]);
		printf_bt("password:%04x\n", passwd);
		if (passwd == ((BT_Rx_key[7] << 8) + BT_Rx_key[8])) {
			Identity_Step++;
			printf_bt(" super authority BT_Rx_Identity ok !!   \n");
		} else {
			Identity_Step = 0;
			printf_bt("super passwd is discorrect\n");
			return -1;
		}
	} else {
		imeiHighTmp += 0X0F;
		imeiLowTmp ^= 0x0F;
		for (i = 0; i < 8; i++) {
			imeiHighTmp += gsm_info_cur.imei[i];
			imeiLowTmp ^= gsm_info_cur.imei[i];
		}
		imeiTmp = (imeiHighTmp << 8) + imeiLowTmp;
		if (imeiTmp == ((BT_Rx_key[7] << 8) + BT_Rx_key[8])) {
			Identity_Step++;
			printf_bt("other authority BT_Rx_Identity ok !!   \n");
		} else {
			Identity_Step = 0;
			printf_bt("authority passwd is discorrect\n");
			return -1;
		}
	}
	return 0;
}
