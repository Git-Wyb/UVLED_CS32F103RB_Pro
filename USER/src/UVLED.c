#include "UVLED.h"
#include "gpio.h"

void PHY_UvLed_Refresh(void)
{
    //CH_LED_switch(PHY_CH[0].Uvch, PHY_CH[0].Error, PHY_CH[0].Uvon);
    //CH_LED_switch(PHY_CH[1].Uvch, PHY_CH[0].Error, PHY_CH[0].Uvon);
    //CH_LED_switch(PHY_CH[2].Uvch, PHY_CH[0].Error, PHY_CH[0].Uvon);
    //CH_LED_switch(PHY_CH[3].Uvch, PHY_CH[0].Error, PHY_CH[0].Uvon);
}

u8 _check_uvled_current_err(void)
{
    u8 i = 0;
    i = PHY_CH[0].Error_Curr | (PHY_CH[1].Error_Curr<<1) | (PHY_CH[2].Error_Curr<<2) | (PHY_CH[3].Error_Curr<<3);
    return i;
}

u8 _check_uvled_connect_err(void)
{
    u8 i = 0;
    i = PHY_CH[0].Error_Connect | (PHY_CH[1].Error_Connect<<1) | (PHY_CH[2].Error_Connect<<2) | (PHY_CH[3].Error_Connect<<3);
    return i;
}

u8 _check_uvled_sta(void)
{
    u8 i = 0;
    i = PHY_CH[0].Uvon | (PHY_CH[1].Uvon<<1) | (PHY_CH[2].Uvon<<2) | (PHY_CH[3].Uvon<<3);
    return i;
}

u8 _check_uvled_option(void)
{
    u8 i = 0;
    i = PHY_CH[0].Option | (PHY_CH[1].Option<<1) | (PHY_CH[2].Option<<2) | (PHY_CH[3].Option<<3);
    return i;
}

void _clear_uvled_option(void)
{
    u8 i = 0;
    for(i = 0; i < CHNUM; i++) PHY_CH[i].Option = 0;
}

void PHY_Set_UVLed_Time(u32 time)
{
    u8 i = 0;
    for(i = 0; i < CHNUM; i++)
    {
        if(PHY_CH[i].Option) 
        {
            PHY_CH[i].Time = time;;
            SEL_CH.Time = time;
        }
    }
}

u8 _get_uvled_level(void)
{
    u8 ui = 0;
    for(ui = 0; ui < CHNUM; ui++)
    {
        if(PHY_CH[ui].Option)   return PHY_CH[ui].Level;
    }
    return 0;
}

u32 _get_uvled_time(void)
{
    u8 ui = 0;
    for(ui = 0; ui < CHNUM; ui++)
    {
        if(PHY_CH[ui].Option)   return PHY_CH[ui].Time;
    }
    return 0;
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

void PHY_UVLed_Connect_Check(void)
{
    u8 ch = 0;
    UVCh_Check.b0 = CH1_CHECK1_IN;
    UVCh_Check.b1 = CH1_CHECK2_IN;
    UVCh_Check.b2 = 0;//CH2_CHECK1_IN;
    UVCh_Check.b3 = CH2_CHECK2_IN;
    UVCh_Check.b4 = CH3_CHECK1_IN;
    UVCh_Check.b5 = CH3_CHECK2_IN;
    UVCh_Check.b6 = CH4_CHECK1_IN;
    UVCh_Check.b7 = CH4_CHECK2_IN;
    
    for(ch = 0; ch < CHNUM; ch++)
    {
        if((((UVCh_Check.Flag >> (ch*2)) & CHECK_NONE) == CHECK_NONE) && PHY_CH[ch].Error_Connect == 0)
        {
            PHY_CH[ch].Error_Connect = 2;
            PHY_CH[ch].Uvon = 0;
            CH_LED_switch(ch,PHY_CH[ch].Error_Connect,1);
            timer1_channel_gpiomode(ch,1,0);
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
            if(PHY_CH[ch].Error_Connect == 0)
            {
                PHY_CH[ch].Error_Connect = 1;
                PHY_CH[ch].Uvon = 0;
                timer1_channel_gpiomode(ch,1,0);
                Timer_Uvon[ch].uvontimer = 0;
                Timer_Uvon[ch].uvoff_flag = 0;
                CH_LED_switch(ch,PHY_CH[ch].Error_Connect,0);
            }
        }
        else 
        {
            if(PHY_CH[ch].Error_Connect == 1)
            {
                PHY_CH[ch].Error_Connect = 0;
                PHY_CH[ch].Uvon = 0;
                CH_LED_switch(ch,0,0);
                timer1_channel_gpiomode(ch,1,0);
            }
        }
    }
}

void PHY_UVLed_Select(u8 ch)
{
    u8 ci = 0;
    if(ch < 1) ch = 1;
    if(ch < 5)
    {
        if(PHY_CH[ch-1].Error_Connect)
        {
            if(PHY_CH[ch-1].Error_Connect == 1) TM1639_Display_UVLED_Char(DISPLAY_BLANK);
            else if(PHY_CH[ch-1].Error_Connect == 2) TM1639_Display_UVLED_Char(DISPLAY_Ld3);
            PHY_CH[0].Option = 0;
            PHY_CH[1].Option = 0;
            PHY_CH[2].Option = 0;
            PHY_CH[3].Option = 0;
            return;
        }
    }
    switch(ch)
    {
        case 1:
            PHY_CH[0].Option = 1;
            PHY_CH[1].Option = 0;
            PHY_CH[2].Option = 0;
            PHY_CH[3].Option = 0;
            SEL_CH.Uvch = 1;
            TM1639_Display_UVLED_Char((DISPLAY_ENUM)(ch+15));
            break;
        
        case 2:
            PHY_CH[0].Option = 0;
            PHY_CH[1].Option = 1;
            PHY_CH[2].Option = 0;
            PHY_CH[3].Option = 0;
            SEL_CH.Uvch = 2;
            TM1639_Display_UVLED_Char((DISPLAY_ENUM)(ch+15));
            break;
        
        case 3:
            PHY_CH[0].Option = 0;
            PHY_CH[1].Option = 0;
            PHY_CH[2].Option = 1;
            PHY_CH[3].Option = 0;
            SEL_CH.Uvch = 3;
            TM1639_Display_UVLED_Char((DISPLAY_ENUM)(ch+15));
            break;
        
        case 4:
            PHY_CH[0].Option = 0;
            PHY_CH[1].Option = 0;
            PHY_CH[2].Option = 0;
            PHY_CH[3].Option = 1;
            SEL_CH.Uvch = 4;
            TM1639_Display_UVLED_Char((DISPLAY_ENUM)(ch+15));
            break;
        
        case 5:
            for(ci = 0; ci < CHNUM; ci++)
            {
                if(PHY_CH[ci].Error_Connect == 0) PHY_CH[ci].Option = 1;
                else PHY_CH[ci].Option = 0;
                if(PHY_CH[ci].Error_Connect == 2)
                {
                    TM1639_Display_UVLED_Char(DISPLAY_Ld3);
                    PHY_CH[0].Option = 0;
                    PHY_CH[1].Option = 0;
                    PHY_CH[2].Option = 0;
                    PHY_CH[3].Option = 0;
                    return;
                }
            }
            if(_check_uvled_connect_err() == 0x0F) TM1639_Display_UVLED_Char(DISPLAY_BLANK);
            else TM1639_Display_UVLED_Char(DISPLAY_ALL);
            break;
        
        default:
            PHY_CH[0].Option = 0;
            PHY_CH[1].Option = 0;
            PHY_CH[2].Option = 0;
            PHY_CH[3].Option = 0;
            break;
    }
}

void _uvled_ch_last(u8 ch)
{
    u8 i = 0;
    if(ch < 1) ch = 1;
    for(i = 0; i < CHNUM; i++)
    {
        if(i == ch-1) PHY_CH[i].Option = 1;
        else PHY_CH[i].Option = 0;
    }
}

