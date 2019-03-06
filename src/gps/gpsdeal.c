#include <math.h>
#include <string.h>
#include <unistd.h>

#include "gps_4g.h"
#include "general.h"
#include "gpio.h"
#include "gsm.h"
#include "sys_manage.h"

void gps_run_gsm(void)
{
		int ret = -1;

		if (system_state != sys_state_stop ) {
			switch (gps_cur_state)
			{
			case gps_state_run:
				ret = gps_receive();
				if (ret == 0) {
					GSM_GpsFailCount++;
					gps_cur_state = gps_state_off;
					gps_flag = 3;
				}
				if (gps_flag == 0) {
					gps_cur_state = gps_state_off;
				}
				if (gps_flag == 1) {
						gps_cur_state = gps_state_on;
					}
				break;
			case gps_state_on:
				if (gps_flag == 1 || gps_flag == 3) {
					if (gps_start() < 1) {
						break;
					}
					if (gps_open() < 1) {
						break;
					}
					gps_flag = 2;
					gps_cur_state = gps_state_run;
				}
				break;
			case gps_state_off:
				gps_stop();
				gps_close();
				gps_cur_state = gps_state_on;
				break;
			default:
				break;
			}
		}else{
				gps_stop();
				gps_close();
				gps_poweroff();
				sleep(5);
		}
}

void *gps_Pthread(void *arg) {
	printf("############## %s start ##############\n", __FUNCTION__);
	gps_PthParam.sta = 1;
	gps_PthParam.flag = TRUE;
	gpio_SetModeValue("10", "low");     //gps power en:PA10	//unexport
	gpio_SetModeValue("171", "in");      //GPS_ANT_D:PF11    //unexport

	int ret = GPS_GSM;  //设置默认状态
	if(ret ==GPS_GSM  &&  system_state != sys_state_stop)
	{
		gps_poweron();
		gps_cur_state = gps_state_off;
	}

while (gps_PthParam.flag) {
	gps_PthParam.sta = 1;

	if(GPS_GSM == ret)
	{
		gps_run_gsm();
	}
	if(GPS_QIANXUN== ret)
	{
#ifdef  QX110_MAKE
		RunGPS();

#endif
#ifdef  QX120_MAKE
		gps_run_qx();
#endif
	 }
}
	printf("############## %s exit ##############\n", __FUNCTION__);
	sleep(1);
	return NULL;
}

/*******************************************************************************
 * Function Name  : Fun_Distance_jw
 * Description    : 时间经纬度计算
 * Input          :
 维度1、经度1、维度2、经度2
 * Output         : None
 * Return         : None
 *******************************************************************************/
uint32_t Fun_Distance_jw(uint32_t weidu1, uint32_t jingdu1, uint32_t weidu2, uint32_t jingdu2) {
	float x, y, result;
	float PI = 3.14159265;
	float Earth_R = 6371229;
	unsigned long int distance;

	float wd1 = (float) weidu1 / 600000;
	float jd1 = (float) jingdu1 / 600000;
	float wd2 = (float) weidu2 / 600000;
	float jd2 = (float) jingdu2 / 600000;

	x = (jd2 - jd1) * PI * Earth_R * cos(((wd1 + wd2) / 2) * PI / 180) / 180;
	y = (wd2 - wd1) * PI * Earth_R / 180;

	result = sqrt(x * x + y * y);
	distance = (unsigned long int) result;
	return (distance);
}




