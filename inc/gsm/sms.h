/*
 * sms.h
 *
 *  Created on: 2018年1月10日
 *      Author: tykj
 */

#ifndef SMS_H_
#define SMS_H_

#include "general.h"

extern int  GSM_RevQueHandle(unsigned char *data,int length);
extern int SMS_ListRead(int fd,int state);
extern void Receive_Message_Process(RecMessage *RecM);
extern int SMS_MessageDelete(int fd,int index);

#endif /* SMS_H_ */
