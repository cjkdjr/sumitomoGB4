#include "alarm.h"
#include "general.h"

Alarm_flags  Alarm,Alarming,Alarmed;
Alarm_flags  Alarm_e,Alarming_e,Alarmed_e;
u8 alarm_save_flag = 0;
time_t gen_AlarmTimer[16];

#define GEN_ALARM_TIME_LEN 59 // alarming delaytime=60sec
void Alarm_Processing(time_t *nowTimer)
{
    int i;

    for(i=0; i<16; i++)
    {
        if (AlarmIsBitH(i) != AlarmingIsBitH(i))
        {
            if (*nowTimer - gen_AlarmTimer[i] > GEN_ALARM_TIME_LEN)
            {
                if (AlarmIsBitH(i))
                {
                    AlarmingSetBit(i);
                    printf("Alarm_Processing AlarmingSetBit id=%d\n", i);
                }
                else
                {
                    AlarmingClrBit(i);
                    printf("Alarm_Processing AlarmingClrBit id=%d\n", i);
                }
                gen_AlarmTimer[i] = *nowTimer;
            }
        }
        else
        {
            gen_AlarmTimer[i] = *nowTimer;
        }

    }

}

/*
return 0-no alarm
	   1- have alarm
*/
unsigned int Alarm_Check(u16 *AlarmCreate,u16 *AlarmClear)
{
	int i=0;
	u16 channel = 1;


    for(i=0;i<16;i++)
    {
        if(((Alarming.terminal &channel)==channel)&&(Alarmed.terminal &channel)!=channel)
        {
            (*AlarmCreate) |=channel;
        }
        if(((Alarming.terminal &channel)!=channel)&&(Alarmed.terminal &channel)==channel)
        {
            (*AlarmClear) |=channel;
        }
        channel =channel<<1;
    }
    Alarmed.terminal =Alarming.terminal;

	if(((*AlarmCreate) ==0)&&((*AlarmClear) ==0))
    {
		return 0;
	}
	else
	{
		return 1;
	}
}
