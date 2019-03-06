/*
 * iap_queue.c
 *
 *  Created on: 2018年1月12日
 *      Author: tykj
 *
 *      1.升级信息队列，由于升级信息不走压缩、转义逻辑故单独开辟队列
 *      2.目前认为升级队列不需要执行关机保存开机读取逻辑
 */
#include <string.h>

#include "iap_queue.h"
#include "api.h"
#include "gsm.h"
#include "UDPclient.h"


int QueueInit(struct FifoQueue *Queue )
{
	Queue->front = 0;
	Queue->rear = 0;
	Queue->count = 0;

	printf_iap("QueueInit:%d,%d,%d\n",Queue->front,Queue->rear,Queue->rear);//lmmtest
	return (0);
}

/*
NAME:	QueueIn
FUNC:	保存一条信息进队列
INPUT:	NO
OUTPUT:	NO
RETURN:
    0 队列满
    1 队列空
    2 队列正常
History:
	V1.0 wuyingqiang 2014-2-26
	- initial version
	V1.1 wangyin 2014-3-3
	- 队列满时，删除旧消息，保存新消息
	- 增加调试输出，显示存储消息的类型
    V1.2 lmm 2014-03-27
    -limit message queue in during IAP.
*/
int QueueIn(struct FifoQueue *Queue ,struct QueueIAP *sdata)
{
	printf_iap("dbg QueueIn front=%d, rear=%d, count=%d, length=%d, messageID=%2x\n",
        Queue->front, Queue->rear, Queue->count, sdata->length, sdata->ID);
	if (debug_value & 0x40) {
		api_PrintfHex(sdata->data, 100);
	}
//    printf_gsm("\n");

        if ((Queue->front == Queue->rear) &&
            (Queue->count == QueueSize))
        {
            Queue->front = (Queue->front + 1) % QueueSize;
            Queue->count = Queue->count - 1;
            Queue->data[Queue->rear] = *sdata;
            Queue->rear = (Queue->rear + 1) % QueueSize;
            Queue->count = Queue->count + 1;
            printf_iap("QueueIn FULL\n");
//			System.DaySummary.MsgQueFullCount++;
            return QueueFull;
        }
        else
        {
            Queue->data[Queue->rear] = *sdata;
            Queue->rear = (Queue->rear + 1) % QueueSize;
            Queue->count = Queue->count + 1;
            printf_iap("QueueIn OK\n");
			System.DaySummary.MessMakeNum++;
            return QueueOperateOk;
	    }


}


/*
NAME:	QueueOut
FUNC:	取一条信息
INPUT:	NO
OUTPUT:	NO
RETURN:
    1 队列空
    2 取成功
History:
	V1.0 wuyingqiang 2014-2-26
	- initial version
	V1.1 wangyin 2014-3-4
	- 出队后，无需移动指针，等待删队函数，清除数据
*/
int QueueOut(struct FifoQueue *Queue ,struct QueueIAP *sdata)
{
//    sem_wait(mutex);

    if ((Queue->front == Queue->rear) &&
        (Queue->count == 0))
    {
 //       sem_post(mutex);

        return QueueEmpty;
    }
    else
    {
        *sdata = Queue->data[Queue->front];
        //Queue->front = (Queue->front+1)%QueueSize;
        //Queue->count = Queue->count - 1;
#if (QUE_DEBUG_MODE == 1)
        printf_gsm("dbg QueueOut front,rear,count,%d,%d,%d\n", Queue->front,
            Queue->rear, Queue->count);
        api_PrintfHex(sdata->data, 100);
        printf_gsm("\n");
#endif
 //       sem_post(mutex);

        return	QueueOperateOk;
    }
}

int QueueGetMessageNum(struct FifoQueue *Queue )
{
    return Queue->count;
}


/*
NAME:	QueueDestroy
FUNC:	删除一条信息
INPUT:	NO
OUTPUT:	NO
RETURN:
    -1 失败，队列无信息
    1 删一条成功
History:
	V1.0 wuyingqiang 2014-2-26
	- initial version
	V1.1 wangyin 2014-3-4
	- 队列空时，无需删除，返回-1
*/
int QueueDestroy(struct FifoQueue *Queue )
{
//    sem_wait(mutex);
    if (Queue->count > 0)
    {
        Queue->front = (Queue->front + 1) % QueueSize;
        Queue->count = Queue->count - 1;
        return (1);
    }
    return (-1);
//    sem_post(mutex);
}

int GSM_SendGPRS(struct QueueIAP * SDM)
{
	int ret= -1;
	int index = 0;
	int MsgLen;
    unsigned char MsgData[1500];
	MsgData[index++]=15;  //IMEI号长度
	memcpy(MsgData+index, gsm_info_cur.imei, 8);
	index += 8;
    MsgLen=SDM->length-2;
    printf_iap("MsgLen=%d \n",MsgLen);

    MsgData[index++]=(MsgLen)>>8;
    MsgData[index++]=(MsgLen);
	memcpy(MsgData+index,SDM->data,MsgLen);
	index += MsgLen;
	if (debug_value & 0x40) {
		api_PrintfHex(MsgData,index); //打印
	}
	MsgData[index] = api_CheckSum((u8 *)MsgData, index);
    index++;
	MsgData[index++] = 0x00;

	printf_iap("GSM_SendGPRS: TakeQueue ok len=%d\n", index);
	printf_iap("GSM_SendGPRS:\n ");
	if (debug_value & 0x40) {
		api_PrintfHex(MsgData, index);
	}

   ret = GSM_SocketTrans(MsgData,index);
	return ret;

}
