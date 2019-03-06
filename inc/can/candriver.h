#ifndef __CANDRIVER_H_
#define  __CANDRIVER_H_

#include <linux/can.h>

//CANA-主控CAN(can2，spi转can);CANB-ECM(CAN0)；CANC-can1
extern int CANA_fd,CANB_fd,CANC_fd;

extern int CAN_Open();
extern int CAN_close();
extern int can_do_restart(const char *name);
extern int CAN_Read(int fd,struct can_frame* CanRxBuf,int CanBufNum);
extern int can_write(int fd,struct can_frame* CanTxBuf);

#endif
