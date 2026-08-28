#ifndef __TIMER_H__
#define __TIMER_H__

#include "sys.h"

typedef enum
{
    TIM_CHANNEL_1 = 0,
    TIM_CHANNEL_2,
    TIM_CHANNEL_3,
    TIM_CHANNEL_4,
    TIM_CHANNEL_ALL
}CH_ENUM;


void Init_Timer1(u8 fkhz);
void Init_Timer2(void);
void Init_Timer3(void);
void Bueezr_Switch(enable_state_t sta);
void wait_ms(u16 ms);
void UV_LED_PwmSet(CH_ENUM channel, u8 pwm);
void UV_LED_Switch(CH_ENUM channel, enable_state_t onoff);

#endif
