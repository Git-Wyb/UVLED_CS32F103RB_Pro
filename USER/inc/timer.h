#ifndef __TIMER_H__
#define __TIMER_H__

#include "sys.h"

#define FREQ_PWM 50 //KHz

typedef enum
{
    UVLED_CH_1 = 0,
    UVLED_CH_2,
    UVLED_CH_3,
    UVLED_CH_4,
    UVLED_CH_ALL
}CH_ENUM;

void Init_Timer1(void);
void Init_Timer2(void);
void Init_Timer3(void);
void Bueezr_Switch(u8 sta);
void wait_ms(u16 ms);
void UV_LED_PwmSet(CH_ENUM channel, u8 pwm);
void UV_LED_Switch(void);
void timer1_channel_gpiomode(u8 chn,u8 type, u8 lev);
void _Bueezr_Handle(void);
void Bueezr_Config(u16 ontime, u16 offtime, u8 num);
u8 Check_UvLed_Sta(void);
void _timer_uvon_scan(void);

#endif
