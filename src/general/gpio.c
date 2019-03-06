#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "gpio.h"

/*
*	函数功能: 导出或删除相应的gpio
*	参数：	  gpioNum:		gpio编号的字符串
*			  		  exportState: 导出gpio "export"，删除gpio "unexport"
*	返回值：  成功返回0，失败返回-1
*/
static int CtrlExportState(char *gpioNum, char *exportState) {
	int fd = -1;
	int writeRet = -1;
	char exportPath[64] = {0};
	
	sprintf(exportPath, "/sys/class/gpio/%s", exportState);
	fd = open(exportPath, O_WRONLY);
	if (fd < 0){
		perror("open export");
		return -1;
	}
	writeRet = write(fd, gpioNum, strlen(gpioNum));
	close(fd);
	if (writeRet < 0) {
		perror("write export");
		return -1;
	}
	return 0;
}

/*
*	函数功能: 设置gpio的方向，输入还是输出
*	参数：	  gpioNum:		gpio编号的字符串
*			 		  direction:		输入"in", 输出"out", 输出高电平"high", 输出低电平"low"
* 	返回值：  成功返回0，失败返回-1
*/
static int SetDirection(char *gpioNum, char *direction) {
	int fd = -1;
	int writeRet = -1;
	char directionPath[64] = {0};
	
	sprintf(directionPath, "/sys/class/gpio/gpio%s/direction", gpioNum);
	fd = open(directionPath, O_WRONLY);
	if (fd < 0){
		perror("open direction");
		return -1;
	}
	writeRet = write(fd, direction, strlen(direction));
	close(fd);
	if (writeRet < 0) {
		perror("write direction");
		return -1;
	}
	return 0;
}

/*
*	函数功能: gpio是输出的时候设置他的电平值
*	参数：	  gpioNum:		gpio编号的字符串
*			  		  value:		    设置高电平"1", 低电平"0"
*	返回值：  成功返回0，失败返回-1
*/
static int SetValue(char *gpioNum, char *value) {
	int fd = -1;
	int writeRet = -1;
	char valuePath[64] = {0};
	
	sprintf(valuePath, "/sys/class/gpio/gpio%s/value", gpioNum);
	fd = open(valuePath, O_WRONLY);
	if (fd < 0) {
		perror("open value");
		return -1;
	}
	writeRet = write(fd, value, strlen(value));
	close(fd);
	if (writeRet < 0) {
		perror("write value");
		return -1;
	}
	return 0;
}

/*
*	函数功能: 获取gpio的电平
*	参数：	  gpioNum:		gpio编号的字符串
*	返回值：  高电平返回1，低电平返回0，函数失败返回-1
*/
static int GetValue(char *gpioNum) {
	int fd = -1;
	int readRet = -1;
	char value;
	char valuePath[64] = {0};

	sprintf(valuePath, "/sys/class/gpio/gpio%s/value", gpioNum);
	fd = open(valuePath, O_RDONLY);
	if (fd < 0) {
		perror("open value");
		return -1;
	}
	readRet = read(fd, &value, sizeof(value));
	close(fd);
	if (readRet < 0) {
		perror("read value");
		return -1;
	}
	if (value == '0') {
		return 0;
	} else {
		return 1;
	}
}

/*
*	函数功能: 设置gpio电平
*	参数：	  gpioNum:		gpio编号的字符串
*					  mode:     输入"in", 输出高电平"high",输出低电平"low"
*	返回值：  成功返回0，失败返回-1
*/
int gpio_SetModeValue(char *gpioNum, char *mode) {
	char gpioPath[64] = {0};

	// 1. 如果没有就导出该gpio文件
	sprintf(gpioPath, "/sys/class/gpio/gpio%s", gpioNum);
	if (access(gpioPath, F_OK) == -1) {
		if (CtrlExportState(gpioNum, "export") < 0) {
				return -1;
			}
	}
	// 2. 设置gpio方向和值
	if (SetDirection(gpioNum, mode) < 0) {
		return -1;
	}
	return 0;
}

/*
*	函数功能: 获取gpio的电平
*	参数：	  gpioNum:		gpio编号的字符串
*	返回值：  高电平返回1，低电平返回0，函数失败返回-1
*/
int gpio_GetValue(char *gpioNum) {
	char gpioPath[64] = {0};

	// 1. 如果没有就导出该gpio文件
	sprintf(gpioPath, "/sys/class/gpio/gpio%s", gpioNum);
	if (access(gpioPath, F_OK) == -1) {
		if (CtrlExportState(gpioNum, "export") < 0) {
				return -1;
			}
	}

	// 2. 设置gpio方向和值
	return GetValue(gpioNum);
}

/*
*	函数功能: 产生一个脉冲信号
*	参数：	  gpioNum:	gpio编号的字符串
*			 		  level:			负脉冲：NEGATIVE, 正脉冲：POSITIVE
*					  msec:		脉冲持续的毫秒数
*	返回值：  成功返回0，失败返回-1
*/
int gpio_ProducePulse(char *gpioNum, bool level, int msec) {
	char gpioPath[64] = {0};

	// 1. 如果没有就导出该gpio文件
	sprintf(gpioPath, "/sys/class/gpio/gpio%s", gpioNum);
	if (access(gpioPath, F_OK) == -1) {
		if (CtrlExportState(gpioNum, "export") < 0) {
				return -1;
			}
	}
	// 2. 产生脉冲信号
	if(NEGATIVE == level) {
		// 负脉冲
		if (SetDirection(gpioNum, "high") < 0) {
			return -1;
		}
		usleep(2);
		if (SetValue(gpioNum, "0") < 0) {
			return -1;
		}
		usleep(msec * 1000);
		if (SetValue(gpioNum, "1") < 0) {
			return -1;
		}
	} else{
		// 正脉冲
		if (SetDirection(gpioNum, "low") < 0) {
			return -1;
		}
		usleep(2);
		if (SetValue(gpioNum, "1") < 0) {
			return -1;
		}
		usleep(msec * 1000);
		if (SetValue(gpioNum, "0") < 0) {
			return -1;
		}
	}
	return 0;
}
