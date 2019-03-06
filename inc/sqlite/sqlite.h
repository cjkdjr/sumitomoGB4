/*
 * sqlite.h
 *
 *  Created on: 2017年11月18日
 *      Author: tykj
 */

#ifndef SQLITE_H_
#define SQLITE_H_

#include "sqlite3.h"
#include "general.h"

//#define QUE_SQLITE3_FILE "/ty/cty/save/que_sqlite3.db"
#define QUE_SQLITE3_FILE "/opt/que.db"
#define InterSta_table "InterSta_table"
#define PassThrough_table "PassThrough_table"

//#define QUE_MSG_SIZE 1516
#define QUE_MSG_CNT_MAX 10000

extern sqlite3 *que_db; // 数据库文件open后对应的句柄，关机前注意关闭以免丢失数据
extern int InterSta_key_sn; // 最小的key_sn
extern int PassThrough_key_sn; // 最小的key_sn

extern int sql_init_db();
extern void sql_free_db(void);
extern bool isTableExist(char *tableName);  //判断数据库中表是否存在
extern int sql_insert(sqlite3 *db, int index_in, u8 *msg_da, char *tableName);
extern int sql_select(sqlite3 *db, int index_out, u8 *msg_da, char *tableName, int que_key_sn);
extern int sql_get_count(sqlite3 *db, char *tableName);
extern int sql_delete(sqlite3 *db, int index_del, char *tableName);

#endif /* SQLITE_H_ */
