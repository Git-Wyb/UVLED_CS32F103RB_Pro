#include "UVLED.h"
#include "gpio.h"

void PHY_UvLed_Refresh(void)
{
    //CH_LED_switch(PHY_CH[0].Uvch, PHY_CH[0].Error, PHY_CH[0].Uvon);
    //CH_LED_switch(PHY_CH[1].Uvch, PHY_CH[0].Error, PHY_CH[0].Uvon);
    //CH_LED_switch(PHY_CH[2].Uvch, PHY_CH[0].Error, PHY_CH[0].Uvon);
    //CH_LED_switch(PHY_CH[3].Uvch, PHY_CH[0].Error, PHY_CH[0].Uvon);
}

void PHY_Set_UVLed_Time(u16 time)
{
    u8 i = 0;
    for(i = 0; i < CHNUM; i++)
    {
        if(PHY_CH[i].Option) PHY_CH[i].Time = time;
    }
}

void PHY_UVon_ChLed(void)
{
    u8 ch = 0;
    for(ch = 0; ch < CHNUM; ch++)
    {
        if(Timer_Uvon[ch].uvoff_flag == 1)
        {
            Timer_Uvon[ch].uvoff_flag = 0;
            CH_LED_switch(ch,0,0);
        }
    }
}

void PHY_UVLed_CheckIn(void)
{
    u8 ch = 0;
    UVCh_Check.b0 = CH1_CHECK1_IN;
    UVCh_Check.b1 = CH1_CHECK2_IN;
    UVCh_Check.b2 = CH2_CHECK1_IN;
    UVCh_Check.b3 = CH2_CHECK2_IN;
    UVCh_Check.b4 = CH3_CHECK1_IN;
    UVCh_Check.b5 = CH3_CHECK2_IN;
    UVCh_Check.b6 = CH4_CHECK1_IN;
    UVCh_Check.b7 = CH4_CHECK2_IN;
    
    for(ch = 0; ch < CHNUM; ch++)
    {
        if(((UVCh_Check.Flag >> (ch*2)) & CHECK_NONE) == CHECK_NONE)
        {
            if(PHY_CH[ch].Error == 0)
            {
                PHY_CH[ch].Error = 1;
                PHY_CH[ch].Uvon = 0;
                timer1_channel_gpiomode(ch,1,0);
                Timer_Uvon[ch].uvontimer = 0;
                Timer_Uvon[ch].uvoff_flag = 0;
            }
        }
        else 
        {
            if(PHY_CH[ch].Error == 1)
            {
                PHY_CH[ch].Error = 0;
                PHY_CH[ch].Uvon = 0;
                CH_LED_switch(ch,0,0);
                timer1_channel_gpiomode(ch,1,0);
            }
        }
    }
}

void PHY_UVLed_Select(u8 ch)
{
    switch(ch)
    {
        case 1:
            PHY_CH[0].Option = 1;
            PHY_CH[1].Option = 0;
            PHY_CH[2].Option = 0;
            PHY_CH[3].Option = 0;
            break;
        
        case 2:
            PHY_CH[0].Option = 0;
            PHY_CH[1].Option = 1;
            PHY_CH[2].Option = 0;
            PHY_CH[3].Option = 0;
            break;
        
        case 3:
            PHY_CH[0].Option = 0;
            PHY_CH[1].Option = 0;
            PHY_CH[2].Option = 1;
            PHY_CH[3].Option = 0;
            break;
        
        case 4:
            PHY_CH[0].Option = 0;
            PHY_CH[1].Option = 0;
            PHY_CH[2].Option = 0;
            PHY_CH[3].Option = 1;
            break;
        
        case 5:
            PHY_CH[0].Option = 1;
            PHY_CH[1].Option = 1;
            PHY_CH[2].Option = 1;
            PHY_CH[3].Option = 1;
            PHY_CH[0].Level = 0;
            PHY_CH[0].Time = 0;
            PHY_CH[1].Level = 0;
            PHY_CH[1].Time = 0;
            PHY_CH[2].Level = 0;
            PHY_CH[2].Time = 0;
            PHY_CH[3].Level = 0;
            PHY_CH[3].Time = 0;
            break;
        
        default:
            PHY_CH[0].Option = 0;
            PHY_CH[1].Option = 0;
            PHY_CH[2].Option = 0;
            PHY_CH[3].Option = 0;
            break;
    }

}

