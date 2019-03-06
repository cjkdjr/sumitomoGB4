/*
 * message_process..c
 *
 *  Created on: 2017年11月17日
 *      Author: tykj
 */
#include <stdio.h>
#include <stdlib.h>

#include "message_process.h"
#include "sqlite.h"
#include "bluetooth.h"
#include "api.h"

//message_format_t message_tmp_make;
//uint16_t message_make_sn = 0;
/*********************************************************
 FUNC:十六进制字符串转十进制数
 PARA:char* buf:十六进制字符串
 len:串长度，这里基本固定为4
 Return:返回转换后的十进制数
 **********************************************************/
int HexStr2Int(char *buf, int len) {
	int result = 0;
	int tmp;
	int i;

	//printf("buf len=%d \r\n", len);
	for (i = 0; i < len; i++) {
		if (*buf >= 'A' && *buf <= 'Z')
			tmp = *buf - 'A' + 10;
		else if (*buf >= 'a' && *buf <= 'z')
			tmp = *buf - 'a' + 10;
		else
			tmp = *buf - '0';

		result *= 16;
		result += tmp;
		buf++;
	}

	return result;
}
/*
 Queue_init
 */
void queue_init(void) {
	//统计交会类信息队列初始化
	InterSta_queue.buf = InterStaQue_buffer;
	InterSta_queue.bufSize = InterStaQue_buffer_max * sizeof(QUE_TDF_QUEUE_MSG) + 1;
	rngFlush(&InterSta_queue);
	InterSta_count = 0;

	//透传信息队列初始化
	PassThrough_queue.buf = PassThrough_buffer;
	PassThrough_queue.bufSize = PassThrough_buffer_max * sizeof(QUE_TDF_QUEUE_MSG) + 1;
	rngFlush(&PassThrough_queue);
	PassThrough_count = 0;
}
/*
 Queue_free
 */
void queue_free(void) {
	rngFlush(&InterSta_queue);
	rngFlush(&PassThrough_queue);
}

/***************************************************************
 NAME:InterSta_in
 FUNC:消息入队
 * *************************************************************/
void InterSta_in(void *src) {
//	printf("queue_in\n");
//	api_PrintfHex(src,sizeof(100));
	pthread_mutex_lock(&InterStaQuemutex);

	System.DaySummary.MessMakeNum++;
	if (InterSta_count >= InterStaQue_buffer_max)    //环形缓冲区满后数据存入数据库
	{
		char *p = (char*) malloc(sizeof(QUE_TDF_QUEUE_MSG));

		rngBufGet(&InterSta_queue, p, sizeof(QUE_TDF_QUEUE_MSG));
		InterSta_count--;
		que_sqlite_in((unsigned char*) p, InterSta_table, sqlite_InterSta_count, InterSta_key_sn);

		printf_msg("queue is full move one message to sql,sqlite_count:%d! \n", sqlite_InterSta_count);
		free(p);
	}
	rngBufPut(&InterSta_queue, src, sizeof(QUE_TDF_QUEUE_MSG));
	InterSta_count++;
	printf_msg("rngbuf->write:%d ;rngbuf_count:%d !\n", InterSta_queue.pToBuf, InterSta_count);
	pthread_mutex_unlock(&InterStaQuemutex);
}
/******************************************************************
 NAME:InterSta_out
 FUNC:消息出队
 PARA:void *src:出对信息指针
 Return:队列不空，出对成功返回读取到到字节数，否则返回0
 *****************************************************************/
int InterSta_out(void *src) {
	int result = 0;
	if (InterSta_count) {
		//printf("message in ringbuf %d; rngbuf->read:%d \n", rngbuf_count, message_queue_send.pFromBuf);
		//result = rngBufGetOnly(&message_queue_send, src, sizeof(QUE_TDF_QUEUE_MSG));
		result = rngBufGetOnly(&InterSta_queue, src, sizeof(QUE_TDF_QUEUE_MSG));
	} else {
		//printf("queue_out empty\n");
		result = 0;
	}
	return result;
}
/*********************************************************************
 NAME:InterSta_delete
 FUNC:删除一条记录
 *********************************************************************/
void InterSta_delete(void) {
	pthread_mutex_lock(&InterStaQuemutex);
	if (InterSta_count > 0) {
		printf_msg("queue_delete in ringbuf\n");
		rngDelet(&InterSta_queue, sizeof(QUE_TDF_QUEUE_MSG));
		InterSta_count--;
	}
	pthread_mutex_unlock(&InterStaQuemutex);
}

/***************************************************************
 NAME:PassThrough_in
 FUNC:消息入队
 * *************************************************************/
void PassThrough_in(void *src) {
//	printf("queue_in\n");
//	api_PrintfHex(src,sizeof(100));
	pthread_mutex_lock(&PassThroughQuemutex);

	System.DaySummary.MessMakeNum++;
	if (PassThrough_count >= PassThrough_buffer_max)    //环形缓冲区满后数据存入数据库
	{
		char *p = (char*) malloc(sizeof(QUE_TDF_QUEUE_MSG));

		rngBufGet(&PassThrough_queue, p, sizeof(QUE_TDF_QUEUE_MSG));
		PassThrough_count--;
		que_sqlite_in((unsigned char*) p, PassThrough_table, sqlte_PassThrough_count, PassThrough_key_sn);

		printf_msg("queue is full move one message to sql,sqlite_count:%d! \n", sqlte_PassThrough_count);
		free(p);
	}
	rngBufPut(&PassThrough_queue, src, sizeof(QUE_TDF_QUEUE_MSG));
	PassThrough_count++;
	printf_msg("rngbuf->write:%d ;rngbuf_count:%d !\n", PassThrough_queue.pToBuf, PassThrough_count);
	pthread_mutex_unlock(&PassThroughQuemutex);
}
/******************************************************************
 NAME:PassThrough_out
 FUNC:消息出队
 PARA:void *src:出对信息指针
 Return:队列不空，出对成功返回读取到到字节数，否则返回0
 *****************************************************************/
int PassThrough_out(void *src) {
	int result = 0;
	if (PassThrough_count) {
		//printf("message in ringbuf %d; rngbuf->read:%d \n", rngbuf_count, message_queue_send.pFromBuf);
		//result = rngBufGetOnly(&message_queue_send, src, sizeof(QUE_TDF_QUEUE_MSG));
		result = rngBufGetOnly(&PassThrough_queue, src, sizeof(QUE_TDF_QUEUE_MSG));
	} else {
		//printf("queue_out empty\n");
		result = 0;
	}
	return result;
}
/*********************************************************************
 NAME:PassThrough_delete
 FUNC:删除一条记录
 *********************************************************************/
void PassThrough_delete(void) {
	pthread_mutex_lock(&PassThroughQuemutex);
	if (PassThrough_count > 0) {
		printf_msg("queue_delete in ringbuf\n");
		rngDelet(&PassThrough_queue, sizeof(QUE_TDF_QUEUE_MSG));
		PassThrough_count--;
	}
	pthread_mutex_unlock(&PassThroughQuemutex);
}
/*****************************************************************
 NAME:queue_save
 FUNC:程序退出时保存队列数据到数据库
 * **************************************************************/
void que_sqlite_save(RING *que, char *table_que, int que_count, int table_key) {
	char *p;
	if (que_db == NULL)
		que_sqlite_open();
	p = (char*) malloc(sizeof(QUE_TDF_QUEUE_MSG));
	sqlite3_exec(que_db, "begin;", 0, 0, 0);
	while (que_count > 0) {
		memset(p, 0X00, sizeof(QUE_TDF_QUEUE_MSG));
		rngBufGet(que, p, sizeof(QUE_TDF_QUEUE_MSG));
		que_count--;
		que_sqlite_in((u8*) p, table_que, que_count, table_key);
	}
	sqlite3_exec(que_db, "commit;", 0, 0, 0);
	free(p);
}
int queue_save(void) {
	if (que_db != NULL)
		que_sqlite_open();
	if (InterSta_count > 0) {
		printf_msg("it's going to shutdown soon,  we'll move InterStaInfo message(%d) to sql\n", InterSta_count);
		que_sqlite_save(&InterSta_queue, InterSta_table, sqlite_InterSta_count, InterSta_key_sn);
	}
	if (PassThrough_count > 0) {
		printf_msg("it's going to shutdown soon,  we'll move PassThroughInfo  message(%d) to sql\n", PassThrough_count);
		que_sqlite_save(&PassThrough_queue, PassThrough_table, sqlte_PassThrough_count, PassThrough_key_sn);
	}
	if (que_db != NULL)
		que_sqlite_close();
	return 0;
}

/*****************************************************
 NAME:que_sqlite_open
 FUNC:打开数据库
 ******************************************************/
void que_sqlite_open() {
	sql_init_db();
	InterSta_count = sql_get_count(que_db, InterSta_table);
	sqlte_PassThrough_count = sql_get_count(que_db, PassThrough_table);
}
/*****************************************************
 NAME:que_sqlite_close
 FUNC:打开数据库
 ******************************************************/
void que_sqlite_close(void) {
	printf_msg("close sqlite_db!!\n");
	sql_free_db();
}
/*****************************************************
 NAME:que_sqlite_in
 FUNC:消息队列满后存入数据库
 PARA:
 u8* sdata:入队数据串
 Rrturn:返回数据库是否满状态
 *****************************************************/
int que_sqlite_in(u8 *sdata, char *tableName, int sqlite_count, int que_key_sn) {
	int ret = 0;
	if (que_db == NULL)
		que_sqlite_open();
	if (sqlite_count >= QUE_MSG_CNT_MAX) {
		ret = sql_select(que_db, 0, NULL, tableName, que_key_sn);
		if (ret != 0) {
			printf_msg("sql_select err !\n");
			return ret;
		}
		ret = sql_delete(que_db, que_key_sn, tableName);
		if (ret != 0) {
			printf_msg("sql_delete err !\n");
			return ret;
		}
		ret = sql_insert(que_db, 1, sdata, tableName);
		if (ret != 0) {
			printf_msg("sql_insert err !\n");
			return ret;
		}
		return ret;
	} else {
		ret = sql_insert(que_db, 1, sdata, tableName);
		if (ret != 0) {
			printf_msg("sql_insert err !\n");
			return ret;
		}
		sqlite_count++;
		printf_msg("que_sqlite_in , sqlite_count:%d \n", sqlite_count);
		return ret;
	}
}
/********************************************************
 NAME:que_sqlite_out
 FUNC:数据库中出库一条信息
 PARA:u8* sdata:存放出库信息串
 Return:返回数据库状态
 *******************************************************/
int que_sqlite_out(u8 *sdata, char *tableName, int sqlite_count, int que_key_sn) {
	if (que_db == NULL)
		que_sqlite_open();
	if (sqlite_count == 0) {
		sdata = '\0';
		return (sqlEmpty);
	}
	printf_msg("Table:%s , que_sqlite_out:%d! \n", tableName, sqlite_count);
	sql_select(que_db, 1, (u8*) sdata, tableName, que_key_sn);
	return sqlOK;
}
/*******************************************************
 NAME:que_sqlite_count
 FUNC:获取数据库表中数据条数

 *******************************************************/
int que_sqlite_count(char *tableName) {
	return (sql_get_count(que_db, tableName));
}
/*******************************************************
 NAME:que_sqlite_del
 FUNC:删除一条记录
 Return:返回0 成功，其他失败
 ********************************************************/
int que_sqlite_del(char *tableName, int sqlite_count, int que_key_sn) {
	if (que_db == NULL)
		que_sqlite_open();
	int rc = sql_delete(que_db, que_key_sn, tableName);
	sqlite_count--;
	return rc;
}

//////////////////////BT queue operate/////////////////////////
void BTqueue_init(void) {
	BT_queue_send.buf = BT_send_buffer;
	BT_queue_send.bufSize = BT_buffer_max * sizeof(QUE_TDF_QUEUE_MSG) + 1;
	rngFlush(&BT_queue_send);
	BTrngbuf_count = 0;
}
/******************************************************
 NAME:queue_free
 FUNC:清空队列
 * ****************************************************/
void BTqueue_free(void) {
	rngFlush(&BT_queue_send);
}
/***************************************************************
 NAME:queue_in
 FUNC:消息入队
 * *************************************************************/
void BTqueue_in(void *src) {
//	System.DaySummary.MessMakeNum++;
	if (BTrngbuf_count >= BT_buffer_max)    //环形缓冲区满后数据存入数据库
	{
		char *p = (char*) malloc(sizeof(QUE_TDF_QUEUE_MSG));

		rngBufGet(&BT_queue_send, p, sizeof(QUE_TDF_QUEUE_MSG));
		BTrngbuf_count--;

		printf_msg("BTqueue is full ,remove one message ! \n");
		free(p);
	}
	rngBufPut(&BT_queue_send, src, sizeof(QUE_TDF_QUEUE_MSG));
	BTrngbuf_count++;
	printf_msg("BT_queue_send->write:%d ;BTrngbuf_count:%d !\n", BT_queue_send.pToBuf, BTrngbuf_count);
}
/******************************************************************
 NAME:queue_out
 FUNC:消息出队
 PARA:void *src:出对信息指针
 Return:队列不空，出对成功返回读取到到字节数，否则返回0
 *****************************************************************/
int BTqueue_out(void *src) {
	int result = 0;
	if (BTrngbuf_count) {
//		printf("message in ringbuf %d; rngbuf->read:%d \n", rngbuf_count, message_queue_send.pFromBuf);
		//result = rngBufGetOnly(&message_queue_send, src, sizeof(QUE_TDF_QUEUE_MSG));
		result = rngBufGetOnly(&BT_queue_send, src, sizeof(QUE_TDF_QUEUE_MSG));
	} else {
//		printf("queue_out empty\n");
		result = 0;
	}
	return result;

}
/*********************************************************************
 NAME:queue_delete
 FUNC:删除一条记录
 *********************************************************************/
void BTqueue_delete(void) {
	if (BTrngbuf_count > 0) {
		printf_msg("BTqueue_delete in ringbuf\n");
		rngDelet(&BT_queue_send, sizeof(QUE_TDF_QUEUE_MSG));
		BTrngbuf_count--;
	}
}
static u32 BTstart_time = 0;
void BTqueue_handle(void *src) {
	if (BTstate_flag == 1) {
		BTqueue_in(src);
		BTstart_time = api_GetSysmSecs();
	} else if (BTstate_flag == 0) {
		if (BTstart_time == 0)
			return;
		if (api_DiffSysMSecs(BTstart_time) <= 10 * 1000) {
			BTqueue_in(src);
		} else {
			BTqueue_free();
			BTstart_time = 0;
			BTstate_flag = 0;
		}
	}
}
