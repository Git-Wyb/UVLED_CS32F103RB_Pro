#include "Application.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_misc.h"

#define CNTMAX  2
/*
LEVEL    MENU       MODE
TIME     HOUR
CH1  CH2  CH3  CH4  CH
          ^
          
<<BK     SET        FW>> 

          V         UV ON
*/

u8 upcnt = 0;
u8 downcnt = 0;
u8 setcnt = 0;
u8 uvoncnt = 0;
u8 pwmcnt = 0;
u8 chcnt = 0;
u8 chnum = 0;
u8 uvon = 0;
u8 levelcnt = 0;
u8 menucnt = 0;
u8 modecnt = 0;
u8 timecnt = 0;
u8 hourcnt = 0;
u8 bkcnt = 0;
u8 fwcnt = 0;
u8 fbuff[11] = {0,1,2,3,5,10,15,20,50,100};

void Key_Handle(void)
{
    Key_Up_Scan();
    Key_Down_Scan();
    Key_Set_Scan();
    Key_Bk_Scan();
    Key_Fw_Scan();
    Key_Uvon_Scan();
    Key_Ch_Scan();
    Key_Mode_Scan();
}

void Key_Up_Scan(void)
{
    if(KeySta.Up == 1 && KeyStaNow.Up == 0)
    {
        upcnt++;
        if(upcnt > CNTMAX)
        {
            upcnt = 0;
            KeyStaNow.Up = 1;
            if(PHY_CH[0].Uvon==0 && PHY_CH[1].Uvon==0 && PHY_CH[2].Uvon==0 && PHY_CH[3].Uvon==0)
            {
                pwmcnt++;
                if(pwmcnt > 9) pwmcnt = 1;
                UV_LED_PwmSet(UVLED_CH_ALL,fbuff[pwmcnt]);
                //if(pwmcnt > 100) pwmcnt = 1;
                //UV_LED_PwmSet(TIM_CHANNEL_ALL,pwmcnt);
                TM1639_DisplayNum(fbuff[pwmcnt]);
                //if(pwmcnt > 15) pwmcnt = 1;
                //TM1639_Display_UVLED_Char(pwmcnt);
            }
        }
    }else 
    {
        upcnt = 0;
        if(KeySta.Up == 0) KeyStaNow.Up = 0;
    }
}

void Key_Down_Scan(void)
{
    if(KeySta.Down == 1 && KeyStaNow.Down == 0)
    {
        downcnt++;
        if(downcnt > CNTMAX)
        {
            downcnt = 0;
            KeyStaNow.Down = 1;
            if(PHY_CH[0].Uvon==0 && PHY_CH[1].Uvon==0 && PHY_CH[2].Uvon==0 && PHY_CH[3].Uvon==0)
            {
                if(pwmcnt != 0) pwmcnt--;
                if(pwmcnt == 0) pwmcnt = 9;
                UV_LED_PwmSet(UVLED_CH_ALL,fbuff[pwmcnt]);
                //if(pwmcnt == 0) pwmcnt = 100;
                //UV_LED_PwmSet(TIM_CHANNEL_ALL,pwmcnt);
                TM1639_DisplayNum(fbuff[pwmcnt]);
                //TM1639_Display_UVLED_Char(pwmcnt);
            }
        }
    }else 
    {
        downcnt = 0;
        if(KeySta.Down == 0) KeyStaNow.Down = 0;
    }
}

void Key_Set_Scan(void)
{
    if(KeySta.Set == 1 && KeyStaNow.Set == 0)
    {
        setcnt++;
        if(setcnt > CNTMAX)
        {
            setcnt = 0;
            KeyStaNow.Set = 1;
            if(PHY_CH[0].Uvon==0 && PHY_CH[1].Uvon==0 && PHY_CH[2].Uvon==0 && PHY_CH[3].Uvon==0)
            {
                pwmcnt = 0;
                UV_LED_PwmSet(UVLED_CH_ALL,pwmcnt);
                TM1639_DisplayNum(pwmcnt);
            }
        }
    }else 
    {
        setcnt = 0;
        if(KeySta.Set == 0) KeyStaNow.Set = 0;
    }
}

void Key_Bk_Scan(void)
{
    if(KeySta.Bk == 1 && KeyStaNow.Bk == 0)
    {
        bkcnt++;
        if(bkcnt > CNTMAX)
        {
            bkcnt = 0;
            KeyStaNow.Bk = 1;
            
        }
    }else 
    {
        bkcnt = 0;
        if(KeySta.Bk == 0) KeyStaNow.Bk = 0;
    }
}

void Key_Fw_Scan(void)
{
    if(KeySta.Fw == 1 && KeyStaNow.Fw == 0)
    {
        fwcnt++;
        if(fwcnt > CNTMAX)
        {
            fwcnt = 0;
            KeyStaNow.Fw = 1;
            
        }
    }else 
    {
        fwcnt = 0;
        if(KeySta.Fw == 0) KeyStaNow.Fw = 0;
    }
}
    
void Key_Uvon_Scan(void)
{
    if(KeySta.Uvon == 1 && KeyStaNow.Uvon == 0)
    {
        uvoncnt++;
        if(uvoncnt > CNTMAX)
        {
            uvoncnt = 0;
            KeyStaNow.Uvon = 1;
            if(uvon == 0) uvon = 1;
            else uvon = 0;
            UV_LED_Switch();
            //TM1639_LED_switch(LED_CH1_GREEN,uvon);
            //TM1639_LED_switch(LED_CH2_GREEN,uvon);
            //TM1639_LED_switch(LED_CH3_GREEN,uvon);
            //TM1639_LED_switch(LED_CH4_GREEN,uvon);
        }
    }else 
    {
        uvoncnt = 0;
        if(KeySta.Uvon == 0) KeyStaNow.Uvon = 0;
    }
}

void Key_Ch_Scan(void)
{
    if(KeySta.Ch == 1 && KeyStaNow.Ch == 0)
    {
        chcnt++;
        if(chcnt > CNTMAX)
        {
            chcnt = 0;
            KeyStaNow.Ch = 1;
            if(PHY_CH[0].Uvon==0 && PHY_CH[1].Uvon==0 && PHY_CH[2].Uvon==0 && PHY_CH[3].Uvon==0)
            {
                chnum++;
                if(chnum > 5) chnum = 1;
                TM1639_Display_UVLED_Char(chnum+15);
                PHY_UVLed_Select(chnum);
                //TM1639_LED_switch(LED_CH1_RED,chnum);
            }
        }
    }else 
    {
        chcnt = 0;
        if(KeySta.Ch == 0) KeyStaNow.Ch = 0;
    }
}

void Key_Mode_Scan(void)
{
    if(KeySta.Mode == 1 && KeyStaNow.Mode == 0)
    {
        modecnt++;
        if(modecnt > CNTMAX)
        {
            modecnt = 0;
            KeyStaNow.Mode = 1;
            
        }
    }else 
    {
        modecnt = 0;
        if(KeySta.Mode == 0) KeyStaNow.Mode = 0;
    }
}
 

