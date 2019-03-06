/*
 * Program:     serial.c
 * Author:      Paul Dean
 * Version:     0.0.3
 * Date:        2002-02-19
 * Description: To provide underlying serial port function,
 *              for high level applications.
 *
*/

#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include "serial.h"

static int SetBaud(int fd, int baud);
static int SetOtherOpt(int fd, int dataBits, int stopBits, char parity);

/*
*	函数功能: 设置串口波特率
*	参数：	  fd:		串口的文件描述符
*			  		  baud: 要设置的波特率数值
*	返回值：  成功返回0，失败返回-1
*/
static int SetBaud(int fd, int baud)
{
	int   i = 0;
	int   status = 0;
	struct termios options;
	int baudConst[8] = { B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300};
	int baudValue[8] = {115200, 38400,  19200,  9600,  4800,  2400,  1200,  300};

	tcgetattr(fd, &options);
	for ( i= 0;  i < sizeof(baudValue) / sizeof(int);  i++) {
		if  (baud == baudValue[i]) {
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&options, baudConst[i]);
			cfsetospeed(&options, baudConst[i]);
			status = tcsetattr(fd, TCSANOW, &options);
			if  (status != 0) {
				perror("set baud tcsetattr");
				return -1;
			}
			tcflush(fd,TCIOFLUSH);
		}
	}
	return 0;
}

/*
*	函数功能: 设置串口数据位，停止位和效验位
*	参数：	  fd:			  串口的文件描述符
*					  dataBits: 数据位，7或8
*			  		  stopBits: 停止位，1或2
*			  		  parity:	  校验位，'N'、'O'、'E'、'S'
*	返回值：  成功返回0，失败返回-1
*/
static int SetOtherOpt(int fd, int dataBits, int stopBits, char parity)
{
	struct termios options;

	tcgetattr(fd, &options);
	// 设置数据位
	options.c_cflag &= ~CSIZE;
	switch (dataBits) {
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			printf("不支持的数据位\n");
			return -1;
	}
	// 设置校验位
	switch (parity) {
		// 无校验
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB;
			options.c_iflag &= ~INPCK;
			break;
		// 奇校验
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB);
			options.c_iflag |= INPCK;
			break;
		// 偶校验
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;
			break;
		case 'S':
		case 's':  /*as no parity*/
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
		default:
			printf("不支持的校验位\n");
			return -1;
	}
	// 停止位
	switch (stopBits) {
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
		   break;
		default:
			printf("不支持的停止位\n");
			return -1;
	}
	/* Set input parity option */
	if (parity != 'n')
		options.c_iflag |= INPCK;

	/*如果设置，但IGNCR没有设置，接收到的回车符向应用程序发送时会变换成换行符。*/
	options.c_iflag &= ~ICRNL;
	/*如果不是开发终端之类的，只是串口传输数据，而不需要串口来处理，那么使用原始模式(Raw Mode)方式来通讯，设置方式如下*/
	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
	options.c_oflag  &= ~OPOST;   /*Output*/

	tcflush(fd,TCIFLUSH);
	/* set timeout about 1 second(unit:100ms)*/
	options.c_cc[VTIME] = 1;
	/* Update the options and do it NOW */
	options.c_cc[VMIN] = 0;
	if (tcsetattr(fd, TCSANOW, &options) != 0) {
		perror("set other options tcsetattr");
		return -1;
	}
	return 0;
}

int serial_Init(int fd, int baud, int dataBits, int stopBits, char parity) {
	if (SetBaud(fd, baud) < 0) {
		return -1;
	}
	if (SetOtherOpt(fd, dataBits, stopBits, parity) < 0) {
		return -1;
	}
	return 0;
}

