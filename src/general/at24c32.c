#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include "e2p.h"
#include "alarm.h"
#include "general.h"
#include "gpio.h"

#define WRITE_ADDRESS 0xa0
#define READ_ADDRESS  0xa1
#define ADDR_LENGTH 2
#define PAGE_SIZE	32

#define BSP_EE_WP "172"

/*******************************************************************************
*
* e2p_write_enable -
*
* This routine to tunoff the device inadvertent write protection.
* It will disable all programming modes.
*
* RETURNS:
* None
*/
void
e2p_write_enable(void)
{
	gpio_SetModeValue(BSP_EE_WP, "low");
}

/*******************************************************************************
*
* e2p_write_disable -
*
* This routine to protect the device against inadvertent writes.
* It will enables all programming modes.
*
* RETURNS:
* None
*/
void e2p_write_disable(void)
{
	gpio_SetModeValue(BSP_EE_WP, "high");
}
#define E2P_port "/dev/i2c-0"

uint32_t e2p_write_page(uint32_t address, uint8_t *data, uint32_t leg)
{
	int fd;
	uint8_t au8TempData[2];
	uint32_t leg_str;
	uint32_t address_cur = address;
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg	i2cmsg[2];

	fd = open(E2P_port, O_RDWR);
    if(fd < 0) {

    	if ( !AlarmIsBitH(AlarmIdE2) )
    	{
//    		printf("<%s>:e2  write error, Alarm Set!!!!!!!!!!\n", __FUNCTION__);
    	}
    	AlarmSetBit(AlarmIdE2);
//    	AlarmingSetBit(AlarmIdE2);
//		perror("open");
		return -1;
    }

	AlarmClrBit(AlarmIdE2);
//	AlarmingClrBit(AlarmIdE2);
	e2p_write_enable();

	while(leg > 0)
	{
		leg_str = PAGE_SIZE - (address_cur & (PAGE_SIZE - 1));
		if (leg < leg_str) leg_str = leg;

		/* Send address */
		if(ADDR_LENGTH == 1)
		{
			au8TempData[0] = address_cur & 0x00FFu;
		}
		else
		{
			au8TempData[0] = (address_cur & 0xFF00u) >> 8;
			au8TempData[1] = address_cur & 0x00FFu;
		}

		//start an write cycle
		msg_rdwr.msgs = i2cmsg;
		msg_rdwr.nmsgs = 2;

		i2cmsg[0].addr = 0xA0 >> 1;
		i2cmsg[0].flags = 0;
		i2cmsg[0].len = 2;
		i2cmsg[0].buf = au8TempData;

		i2cmsg[1].addr = 0xA0 >> 1;
		i2cmsg[1].flags = I2C_M_NOSTART | I2C_M_STOP;
		i2cmsg[1].len = leg_str;
		i2cmsg[1].buf = data;

		ioctl(fd, I2C_RDWR, &msg_rdwr);

		//record src address
		data += leg_str;
		//record dst address
		address_cur += leg_str;
		//record length
		leg -= leg_str;

		usleep(10*1000);    //self-timed write cycle 5ms max
	}

    e2p_write_disable();

    close(fd);
	return address_cur - (uint32_t)address;
}
uint32_t e2p_read_page(uint32_t address, uint8_t *data, uint32_t leg)
{
	int fd;
    uint8_t au8TempData[2];
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg	i2cmsg[2];

	fd = open(E2P_port, O_RDWR);
    if(fd < 0) {

    	if ( !AlarmIsBitH(AlarmIdE2) )
    	{
//    		printf("<%s>:e2  read error, Alarm Set!!!!!!!!!!\n", __FUNCTION__);
    	}
    	AlarmSetBit(AlarmIdE2);

//		perror(E2P_port);
		return -1;
    }


	AlarmClrBit(AlarmIdE2);
//	AlarmingClrBit(AlarmIdE2);
	/* Send address */
	if(ADDR_LENGTH == 1)
	{
		au8TempData[0] = address & 0x00FFu;
	}
	else
	{
		au8TempData[0] = (address & 0xFF00u) >> 8;
		au8TempData[1] = address & 0x00FFu;
	}

	msg_rdwr.msgs = i2cmsg;
	msg_rdwr.nmsgs = 2;

	i2cmsg[0].addr = 0xA0 >> 1;
	i2cmsg[0].flags = 0;
	i2cmsg[0].len = 2;
	i2cmsg[0].buf = au8TempData;

	i2cmsg[1].addr = 0xA0 >> 1;
	i2cmsg[1].flags = I2C_M_RD | I2C_M_STOP;
	i2cmsg[1].len = leg;
	i2cmsg[1].buf = data;

	ioctl(fd, I2C_RDWR, &msg_rdwr);

    close(fd);
    return 0;
}

//int e2p_read(int argc, char *argv[])
//{
//    uint32_t addr, leg;
//
//	if (argc == 3)
//	{
//        //数据地址
//        addr = uc_strtol(argv[1],NULL,16);
//        //数据长度
//        leg = uc_strtol(argv[2],NULL,16);
//
//		//set xmodemReceive callbanck
//		gxmodemChan.mem_read = &e2p_read_page;
//
//		xmodemTransmit(addr, leg);
//
//	}
//	else
//	{
//		uc_printf("add a param like e2pr addr(HEX) leg(HEX)\r\n");
//	}
//
//	return 0;
//}
//CMD_USER_EXPORT("e2pr",e2p_read,"read e2p data addr(HEX) leg(DEC)");
//
//int e2p_write(int argc, char *argv[])
//{
//    uint32_t addr, leg;
//
//	if (argc == 3)
//	{
//        //数据地址
//        addr = uc_strtol(argv[1],NULL,16);
//		//数据长度
//		leg = uc_strtol(argv[2],NULL,16);
//
//		//set xmodemReceive callbanck
//		gxmodemChan.mem_write = &e2p_write_page;
//
//		xmodemReceive(addr, leg);
//
//	}
//	else
//	{
//		uc_printf("add a param like e2pw addr(HEX) leg(HEX)\r\n");
//	}
//	return 0;
//}
//CMD_USER_EXPORT("e2pw",e2p_write,"write e2p data addr(HEX) leg(DEC)");

int e2p_read(int argc, char *argv[])
{
    uint32_t addr, leg;
    uint8_t data[16];
    uint8_t leg_str,leg_str_a = 0;
    uint8_t i;

	if (argc == 3)
	{
        //数据地址
        addr = strtol(argv[1],NULL,16);
        //数据长度
        leg = strtol(argv[2],NULL,10);

//        printf("\n      | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");

        while(leg > 0)
        {
            //判断分页
//            printf("%03x0H | ",(addr>>4));
            //前端补零
            leg_str = addr%16;
            for(i=0; i<leg_str; i++)
//                printf("   ");

            if(leg <= (16-leg_str))
            {
                leg_str_a = leg;
                leg = 0;
            }
            else
            {
                leg_str_a = 16-leg_str;
                leg = leg - leg_str_a;
            }

            e2p_read_page(addr,&data[0],leg_str_a);

			addr = addr +  leg_str_a;

			//数据打印
//			for(i=0; i<leg_str_a; i++)
//				printf("%02x ",data[i]);
//			printf("\n");

			//连续发送数据造成串口缓存阻塞
			usleep(100*1000);
        }

	}
	else
	{
//		printf("add a param like e2pr addr(HEX) leg(DEC)\r\n");
	}
	return 0;
}

int e2p_write(int argc, char *argv[])
{
    uint32_t addr = 0;
    uint8_t data = 0;

	if (argc == 3)
	{
        //数据地址
        addr = strtol(argv[1],NULL,16);
        //数据
        data = strtol(argv[2],NULL,16);

        e2p_write_page(addr, &data, 1);
//		printf("write data : %x--%x\r\n",addr,data);
	}
	else
	{
//		printf("add a param like e2pr addr(HEX) data(HEX)\n");
	}
	return 0;
}
