/*
 * Up_SystemFile.c
 *	系统文件更新操作相关函数
 *  Created on: 2018年1月9日
 *    Author: tykj
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "Up_SystemFile.h"
#include "general.h"

#define mtd_erase_cmd "flash_erase"
#define mtd_write_cmd "nandwrite"


int cmd_erase(char* cmd,char *partition,int addr,int block_count)
{
	int ret = -1;
	char cmdbuf[100];
	memset(cmdbuf,0x00,sizeof(cmdbuf));
	sprintf(cmdbuf,"%s %s %d %d",cmd,partition,addr,block_count);

	ret = system(cmdbuf);
	if ((ret < 0) || (ret == 127))
	{
		if (debug_value & 0x40) {
			perror(cmdbuf);
		}
		return -1;
	}
	else
	{
		return 0;
	}
}
int cmd_write(char* cmd,char* param,char *partition,char* file)
{
	int ret=-1;
	char cmdbuf[100];
	memset(cmdbuf,0x00,sizeof(cmdbuf));
	sprintf(cmdbuf,"%s %s %s %s",cmd,param,partition,file);
	ret = system(cmdbuf);
	if ((ret < 0) || (ret == 127))
	{
		if (debug_value & 0x40) {
			perror(cmdbuf);
		}
		return -1;
	}
	else
	{
		return 0;
	}
}

/******************************************************
 * 含备份的镜像，执行更新时，优先更新备份文件
 *
 */
int up_spl()
{
	int ret =-1;
	ret = cmd_erase(mtd_erase_cmd,spl_path,0,1);
	ret = cmd_write(mtd_write_cmd,"-p -a",spl_path,spl_file);
	return ret;
}

int up_spl_bak1()
{
	int ret =-1;
	ret = cmd_erase(mtd_erase_cmd,spl_path,0x20000,1);
	ret  = cmd_write(mtd_write_cmd,"-p -a -s 0x20000",spl_path,spl_file);
	return ret;
}
int up_spl_bak2()
{
	int ret =-1;
	ret = cmd_erase(mtd_erase_cmd,spl_path,0x40000,1);
	ret  = cmd_write(mtd_write_cmd,"-p -a -s 0x40000",spl_path,spl_file);

	return ret;
}
int up_spl_bak3()
{
	int ret =-1;
	ret = cmd_erase(mtd_erase_cmd,spl_path,0x60000,1);
	ret  = cmd_write(mtd_write_cmd,"-p -a -s 0x60000",spl_path, spl_file);

	return ret;
}

int up_kernel()
{
	int ret =-1;
	   ret = cmd_erase(mtd_erase_cmd,kernel_path,0,20);
		ret = cmd_write(mtd_write_cmd,"-p -a",kernel_path,kernel_file);
	return ret;
}

int up_kernel_bak1()
{
	int ret =-1;
	ret = cmd_erase(mtd_erase_cmd,kernel_path,0x280000,20);
	if(!ret)
	{
		ret = cmd_write(mtd_write_cmd,"-p -a -s 0x280000",kernel_path,kernel_file);
	}
	return ret;
}
int up_ubi()
{
	int ret =-1;
	ret = cmd_erase(mtd_erase_cmd,ubi_path,0,0);
	ret = cmd_write(mtd_write_cmd,"-p -a",ubi_path,ubi_file);

	return ret;
}


