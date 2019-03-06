/*
 * message_process.h
 *
 *  Created on: 2017年11月17日
 *      Author: tykj
 */

#ifndef MESSAGE_PROCESS_H_
#define MESSAGE_PROCESS_H_

#include "general.h"
#include "rngLib.h"
#include "sqlite.h"
//数据库状态
#define sqlFull 0
#define sqlEmpty 1
#define sqlOK 2
#define sqlerr 3
//队列状态
#define queFull 0
#define queEmpty 1
#define queOK  2

#define InterStaQue_buffer_max 100 //统计交会类信息队列
char InterStaQue_buffer[InterStaQue_buffer_max * sizeof(QUE_TDF_QUEUE_MSG) + 1];
RING InterSta_queue;
int InterSta_count;  //统计交互信息队列中信息条数

#define PassThrough_buffer_max 300 //透传信息队列
char PassThrough_buffer[PassThrough_buffer_max * sizeof(QUE_TDF_QUEUE_MSG) + 1];
RING PassThrough_queue;
int PassThrough_count; //透传信息队列中信息条数

extern void queue_init(void);  //初始化信息队列
extern void queue_free(void);   //消息队列清空

extern void InterSta_in(void *src);     //统计交会类消息入队
extern int  InterSta_out( void *src);//统计交会类消息出队
extern void InterSta_delete(void);  //删除统计交会类一条记录

extern void PassThrough_in(void *src);     //透传信息入队
extern int  PassThrough_out( void *src);//透传信息出队
extern void PassThrough_delete(void);  //删除透传信息一条记录

extern int  queue_save(void);   //消息队列保存到数据库

int sqlite_InterSta_count;//数据库中交互统计信息数据条数
int sqlte_PassThrough_count;//数据库中透传信息数据条数

extern int HexStr2Int(char *buf, int len);

extern void que_sqlite_open();
extern void que_sqlite_close(void);
extern int que_sqlite_in(u8 *sdata, char *tableName,int sqlite_count, int que_key_sn);
extern int que_sqlite_out(u8 *sdata, char *tableName ,int sqlite_count, int que_key_sn);
extern int que_sqlite_del(char *tableName ,int sqlite_count, int que_key_sn);
extern int que_sqlite_count(char *tableName);

int BTrngbuf_count;//队列中数据条数
#define BT_buffer_max 300         //环形缓冲区队列大小
char BT_send_buffer[BT_buffer_max * sizeof(QUE_TDF_QUEUE_MSG)+1];
RING BT_queue_send;

extern void BTqueue_init(void);   //消息队列初始化
extern void BTqueue_free(void);   //消息队列清空
extern int  BTqueue_count(void);  //消息队列长度
extern void BTqueue_in(void *src);     //消息入队
extern int  BTqueue_out(void *src);//消息出队
extern void BTqueue_delete(void);  //删除一条记录
extern void BTqueue_handle(void *src);
#endif /* MESSAGE_PROCESS_H_ */
