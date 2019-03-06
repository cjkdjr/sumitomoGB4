/*
 * sqlite.c
 *
 *  Created on: 2017年11月18日
 *      Author: tykj-guozhiyue
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sqlite.h"
#include "general.h"

sqlite3 *que_db; // 数据库文件open后对应的句柄，关机前注意关闭以免丢失数据
int InterSta_key_sn; // 最小的key_sn
int PassThrough_key_sn; // 最小的key_sn

int CreatTable(char *Info_Table) {
	int rc = -1;
	char sqlStr[500] = { 0 };

	bool IsTab = isTableExist(Info_Table);
	if (IsTab == FALSE) {
		printf("Table not exist, create!\n");
		sqlite3_stmt *vm;
		const char *tail;
		// 创建数表
		// 准备语句对象
		sprintf(sqlStr, "create table %s (key_sn INTEGER PRIMARY KEY AUTOINCREMENT, p_index INTEGER, msg_da VARCHAR(1500))", Info_Table);
		printf("sqlStr %s\n", sqlStr);
		rc = sqlite3_prepare_v2(que_db, sqlStr, -1, &vm, &tail);
		if (rc != SQLITE_OK) {
			printf("prepare_v2 err: %s\n", sqlite3_errmsg(que_db));
			sqlite3_close(que_db);
			return (rc);
		}
		// 执行上述sql语句
		rc = sqlite3_step(vm);	    //执行上述sql语句
		if (rc != SQLITE_DONE) {
			printf("step err: %s\n", sqlite3_errmsg(que_db));
			sqlite3_close(que_db);
			return (rc);
		}
		sqlite3_finalize(vm);	 //销毁vm这个对象
	}
	return 0;
}
/*
 open db @restart
 */
int sql_init_db() {
	int rc1,rc2;
	// 打开数据库文件
        rc1 = sqlite3_open(QUE_SQLITE3_FILE, &que_db);
	if (rc1) {
		printf("open err: %s\n", sqlite3_errmsg(que_db));
		sqlite3_close(que_db);
		return (rc1);
	}
	rc1 = CreatTable(InterSta_table);
	rc2 = CreatTable(PassThrough_table);
	return (rc1 & rc2);
}

/*
 close db @poweroff
 */
void sql_free_db(void) {
	sqlite3_close(que_db);
	que_db = NULL;
}
/***************************************************************
 NAME:isTableExist
 FUNC:判断表是否存在
 PARA:
 u8 *Table_Name:表名
 Return:存在返回TURE，失败返回FALSE
 * ************************************************************/
bool isTableExist(char *tableName) {
	char sql_str[256];
	bool exist = FALSE;
	sqlite3_stmt *vm = NULL;
	const char *tail;

	snprintf(sql_str, sizeof(sql_str), "select name FROM sqlite_master where type ='table' and name = '%s';", tableName);
	printf("check is table exist: %s\n", sql_str);

	int rc = sqlite3_prepare_v2(que_db, sql_str, -1, &vm, &tail);
	if (rc == SQLITE_OK) {
		int temp = sqlite3_step(vm);
		if (temp == SQLITE_ROW) {
			exist = TRUE;
		}
	}
	sqlite3_finalize(vm);
	return exist;
}

/*
 save a msg to db, p_index = index_in
 */
int sql_insert(sqlite3 *db, int index_in, u8 *msg_da, char *tableName) {
	char sql_str[256] = { 0 };
	sqlite3_stmt *vm = NULL;
	const char *tail;
	int rc;

	sprintf(sql_str, "insert into %s (key_sn, p_index, msg_da) values (NULL, ?, ?);", tableName);
	printf("que_insert: %s\n", sql_str);
	rc = sqlite3_prepare_v2(db, sql_str, -1, &vm, &tail);
	if (rc != SQLITE_OK) {
		printf("que_insert prepare_v2 err:%s\n", sqlite3_errmsg(db));
		goto end;
	}

	// 第1个参数赋值
	rc = sqlite3_bind_int(vm, 1, index_in);
	if (rc != SQLITE_OK) {
		printf("bind 1 err: %s\n", sqlite3_errmsg(db));
		goto end;
	}

	// 第2个参数赋值
	rc = sqlite3_bind_text(vm, 2, (const char*) msg_da, 1500, SQLITE_STATIC);
	if (rc != SQLITE_OK) {
		printf("bind 2 err: %s\n", sqlite3_errmsg(db));
		goto end;
	}

	rc = sqlite3_step(vm); //执行上述sql语句
	if (rc != SQLITE_DONE) {
		printf("step err: %s\n", sqlite3_errmsg(db));
		goto end;
	}
	//复位有sqlite3_bind_text绑定的语句，这样可以重新再用pstmt语句，这是为了提高运行速度。
	rc = sqlite3_reset(vm);
	if (rc != SQLITE_DONE && rc != SQLITE_ROW && rc != SQLITE_OK) {
		printf("reset err: %s\n", sqlite3_errmsg(db));
		goto end;
	}

	sqlite3_finalize(vm); //销毁sql语句对象
	return (0);

	end: sqlite3_finalize(vm); //销毁sql语句对象
	sql_free_db();
	return (1);
}

/*
 ret 0无信息 1有信息 其它有错误
 */
int sql_select(sqlite3 *db, int index_out, u8 *msg_da, char *tableName, int que_key_sn) {
	int rc;
	char sql_str[256];
	sqlite3_stmt *vm = NULL;
	const char *tail;

	//snprintf(sql_str, sizeof(sql_str), "select * from tb_msg where p_index=%d limit 1;", index_out);
	sprintf(sql_str, "select * from %s order by key_sn limit 1;", tableName);
//	printf("que_select: %s\n", sql_str);

	rc = sqlite3_prepare_v2(db, sql_str, -1, &vm, &tail);
	if (rc != SQLITE_OK) {
		printf("que_select sqlite3_prepare_v2 rtn=%d: %s\n", rc, sqlite3_errmsg(db));
		goto end;
	}

	rc = sqlite3_step(vm); //ִ������sql���
	if ((rc != SQLITE_DONE) && (rc != SQLITE_ROW)) {
		printf("que_select sqlite3_step rtn=%d: %s\n", rc, sqlite3_errmsg(db));
		goto end;
	}

	que_key_sn = sqlite3_column_int(vm, 0);
	printf("que_select que_key_sn=%d\n", que_key_sn);
	if (index_out == 1) {
		const void * pFileContent = sqlite3_column_blob(vm, 2);
		int len = sqlite3_column_bytes(vm, 2);
//	printf("len=%d\n",len);
//	printf("\n que_sqlite_out\n");
//	hprintf( (unsigned char*)pFileContent, len);
		if (len > 0) {
			if (msg_da != NULL) {
				memcpy(msg_da, pFileContent, len);
			}
			sqlite3_finalize(vm);
			return (1);
		}
	}
	sqlite3_finalize(vm);
	return (0);

	end: sqlite3_finalize(vm); //销毁sql语句对象
	sql_free_db();
	return (-1);
}

/*
 查找最早与最后两条信息，并提取p_index值
 ret 数据总条数
 */
int sql_get_count(sqlite3 *db, char *tableName) {
	int rc;
	char sql_str[256];

	sprintf(sql_str, "select count(*) from %s;", tableName);

	/* sqlite3_stmt *vm = NULL;
	 const char *tail;

	 rc = sqlite3_prepare_v2 (db, sql_str, -1, &vm, &tail);
	 if (rc != SQLITE_OK)
	 {
	 fprintf(stderr, "que_get_count sqlite3_prepare_v2 rtn=%d: %s\n", rc, sqlite3_errmsg(db));
	 sqlite3_finalize(vm); // free *vm
	 return (0);
	 }

	 rc = sqlite3_step(vm);//ִ������sql���
	 if ((rc != SQLITE_DONE) && (rc != SQLITE_ROW))
	 {
	 fprintf(stderr, "que_get_count sqlite3_step rtn=%d: %s\n", rc, sqlite3_errmsg(db));
	 sqlite3_finalize(vm); // free *vm
	 return (0);
	 }

	 rc = sqlite3_column_int(vm, 0);
	 printf("que_get_count %d\n", rc);
	 return (rc); */

	char **dbResult;
	char *errmsg = NULL;
	int nRow, nColumn;
	//int index;
	//int i, j;

	rc = sqlite3_get_table(db, sql_str, &dbResult, &nRow, &nColumn, &errmsg);
	if (rc == SQLITE_OK) {
		if (nRow >= 1) {
			/* index = nColumn;
			 printf("------��ѯ��%d����¼��-------%d\n",nRow,nColumn);
			 for(i = 0; i<nRow; i++)
			 {
			 printf("----��%d����¼----\n",i+1);
			 //sprintf(sql_update,"update memdata set flag ='1' where keyNum = %s",dbResult[index]);
			 for(j=0 ; j<nColumn ;j++)
			 {
			 printf("�ֶ�����%s  �ֶ�ֵ��%s\n",dbResult[j],dbResult[index]);
			 index++;
			 }
			 } */
			rc = atoi(dbResult[1]);
			//printf("que_get_count %s=%d\n", dbResult[1], rc);
			printf("que_get_count %d\n", rc);
			sqlite3_free_table(dbResult);
			return (rc);
		}
	}
	sqlite3_free_table(dbResult);
	return (0);
}

/*
 delete from tb_msg where p_index=
 */
int sql_delete(sqlite3 *db, int index_del, char *tableName) {
	char sql_str[100];
//	sqlite3_stmt *vm = NULL;
//	const char *tail;
	int rc;
	char *zErrMsg = 0;
	//snprintf(sql_str, sizeof(sql_str), "delete from tb_msg where p_index=%d;", index_del);
	sprintf(sql_str, "delete from %s where key_sn=%d;", tableName, index_del);
	printf("que_delete: %s\n", sql_str);
	rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
	printf("zErrMsg=%s\n", zErrMsg);
	return rc;
}

