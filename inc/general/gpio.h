/*
 * gpio.h
 *
 *  Created on: 2018年10月11日
 *      Author: tykj
 */

#ifndef GPIO_H_
#define GPIO_H_

#include <stdbool.h>

#define POSITIVE true 	// 正脉冲
#define NEGATIVE false	// 负脉冲

/*
*	函数功能: 设置gpio电平
*	参数：	   gpioNum: 	gpio编号的字符串
*					   direction:     输入"in", 输出高电平"high",输出低电平"low"
*	返回值：  成功返回0，失败返回-1
*/
extern int gpio_SetModeValue(char *gpioNum, char *mode);

/*
*	函数功能: 获取gpio的电平
*	参数：	  gpioNum:		gpio编号的字符串
*	返回值：  高电平返回1，低电平返回0，函数失败返回-1
*/
extern int gpio_GetValue(char *gpioNum);

/*
*	函数功能: 产生一个脉冲信号
*	参数：	  gpioNum:		gpio编号的字符串
*			  		  level:			  	负脉冲：NEGATIVE, 正脉冲：POSITIVE
*			  	      msec:		  	脉冲持续的毫秒数
*	返回值：  成功返回0，失败返回-1
*/
extern int gpio_ProducePulse(char *gpioNum, bool level, int msec);

#endif /* GPIO_H_ */
