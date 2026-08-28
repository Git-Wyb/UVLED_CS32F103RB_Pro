#include "Application.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_misc.h"


u8 upcnt = 0;
u8 downcnt = 0;
u8 setcnt = 0;
u8 uvoncnt = 0;
u8 pwmcnt = 0;
u8 uvon = 0;
void Key_Handle(void)
{
    if(KeySta.Up == 1 && KeyStaNow.Up == 0)
    {
        upcnt++;
        if(upcnt > 3)
        {
            upcnt = 0;
            KeyStaNow.Up = 1;
            pwmcnt++;
            if(pwmcnt > 100) pwmcnt = 0;
            UV_LED_PwmSet(TIM_CHANNEL_ALL,pwmcnt);
            TM1639_DisplayNum(pwmcnt);
        }
    }else 
    {
        upcnt = 0;
        if(KeySta.Up == 0) KeyStaNow.Up = 0;
    }
    
    if(KeySta.Down == 1 && KeyStaNow.Down == 0)
    {
        downcnt++;
        if(downcnt > 3)
        {
            downcnt = 0;
            KeyStaNow.Down = 1;
            if(pwmcnt != 0) pwmcnt--;
            if(pwmcnt == 0) pwmcnt = 100;
            UV_LED_PwmSet(TIM_CHANNEL_ALL,pwmcnt);
            TM1639_DisplayNum(pwmcnt);
        }
    }else 
    {
        downcnt = 0;
        if(KeySta.Down == 0) KeyStaNow.Down = 0;
    }
    
    if(KeySta.Set == 1 && KeyStaNow.Set == 0)
    {
        setcnt++;
        if(setcnt > 3)
        {
            setcnt = 0;
            KeyStaNow.Set = 1;
            pwmcnt = 0;
            UV_LED_PwmSet(TIM_CHANNEL_ALL,pwmcnt);
            TM1639_DisplayNum(pwmcnt);
        }
    }else 
    {
        setcnt = 0;
        if(KeySta.Set == 0) KeyStaNow.Set = 0;
    }
    
    if(KeySta.Uvon == 1 && KeyStaNow.Uvon == 0)
    {
        uvoncnt++;
        if(uvoncnt > 3)
        {
            uvoncnt = 0;
            KeyStaNow.Uvon = 1;
            if(uvon == 0) uvon = 1;
            else uvon = 0;
            UV_LED_Switch(TIM_CHANNEL_ALL,uvon);
            TM1639_LED_switch(LED_CH1_GREEN,uvon);
            TM1639_LED_switch(LED_CH2_GREEN,uvon);
            TM1639_LED_switch(LED_CH3_GREEN,uvon);
            TM1639_LED_switch(LED_CH4_GREEN,uvon);
        }
    }else 
    {
        uvoncnt = 0;
        if(KeySta.Uvon == 0) KeyStaNow.Uvon = 0;
    }
}
