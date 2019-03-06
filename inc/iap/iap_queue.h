/*
 * iap_queue.h
 *
 *  Created on: 2018年1月12日
 *      Author: tykj
 *      文件标识: 定义队列结构及功能函数
 */

#ifndef IAP_QUEUE_H_
#define IAP_QUEUE_H_
#include "general.h"

#if (QUE_DEBUG_MODE == 2)
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
#else
#endif

#define QueueFull 0
#define QueueEmpty 1
#define QueueOperateOk 2

#define QueueSize       300
//#define MesssgeDataSize 1024
//
//typedef struct QueueType{
//    u16 messageID;
//    int length;
//    u8  data[MesssgeDataSize];
//    u16 CRC;
//}QUE_TDF_QUEUE_MSG; // 定义类型为队列的一个消息体元素


/**********远程升级信息格式*********/
typedef struct QueueIAP{
	u8 ID;//信息类型
    int length;//消息体长度
    u8 data[MesssgeDataSize];//消息体
    u16 CRC;
}QUE_TDF_QUEUE_IAP_MSG; // 定义类型为队列的一个消息体元素

typedef struct FifoQueue{
    u16 check;  // 校验
    int front;  // 头
    int rear;   // 尾
    int count;  // 计数
    QUE_TDF_QUEUE_IAP_MSG   data[QueueSize];    // 数据
}QUE_TDF_QUEUE_FIFO;    // 定义类型为先进先出队列，包含QueueSize个消息体元素

extern struct FifoQueue IAPQueue;
extern int QueueInit(struct FifoQueue *Queue);//初始化队列
extern int QueueFree(struct FifoQueue *Queue);
extern int QueueGetMessageNum(struct FifoQueue *Queue);          //获取队列中信息的条数
extern int QueueIn(struct FifoQueue *Queue,struct QueueIAP *sdata);//入队
extern int QueueOut(struct FifoQueue *Queue, struct QueueIAP *sdata);//出队列
extern int QueueDestroy(struct FifoQueue *Queue);//删队列
extern int QueueSave(struct FifoQueue *Queue);//将队列保存到文件
extern int QueueUpload(struct FifoQueue *Queue);//读取文件中的内容放到队列中

extern int GSM_SendGPRS(struct QueueIAP * SDM);


#endif /* IAP_QUEUE_H_ */
