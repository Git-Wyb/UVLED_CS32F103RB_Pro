#include "UVLED.h"
#include "gpio.h"
#include "timer.h"
#include "adc.h"
#include "Application.h"

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

u8 _check_uvled_error(void)
{
    u8 i = 0;
    i = (_check_uvled_connect_err() << 4) | _check_uvled_current_err();
    return i;
}

u8 _check_uvled_errLd123(void)
{
    return (PHY_CH[0].Error_Connect==2 || PHY_CH[1].Error_Connect==2 || PHY_CH[2].Error_Connect==2 || PHY_CH[3].Error_Connect==2
           || _check_uvled_current_err());
}

u8 _check_uvled_ch_error(u8 ch)
{
    u8 i = 0;
    if(ch > CHNUM-1) return 1;
    i = PHY_CH[ch].Error_Connect | (PHY_CH[ch].Error_Curr<<1);
    return i;
}

u8 _check_uvled_connect_err(void)
{
    u8 i = 0;
    i = PHY_CH[0].Error_Connect | (PHY_CH[1].Error_Connect<<1) | (PHY_CH[2].Error_Connect<<2) | (PHY_CH[3].Error_Connect<<3);
    return i;
}

u8 _check_uvled_connect_Ld3(void)
{
    u8 i = 0;
    for(i = 0; i < CHNUM; i++)
    {
        if(PHY_CH[i].Error_Connect == 2) return 1;
    }
    return 0;
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

void uvled_ready_check(void)
{
    u8 ch = 0;
    for(ch = 0; ch < CHNUM; ch++)
    {
        if(PHY_CH[ch].Option && PHY_CH[ch].Error_Connect == 0 && PHY_CH[ch].Error_Curr == 0 && PHY_CH[ch].Uvon == 0)
        {
            if(PHY_CH[ch].Level) PHY_CH[ch].Ready = 1;
        }
        else PHY_CH[ch].Ready = 0;
    }
}

void PHY_UvLed_switch(u8 ch)
{
    u8 ci = 0,channel = 0;
    if(ch < 1) return;
    channel = ch - 1;
    
    if(_check_uvled_sta() == 0 && _check_uvled_errLd123() == 0) uvonch_last = 0;
    /* If a single channel is selected to be turned on, then only that single channel can be selected to be turned off. 
       Selecting all channels is not possible. */
    if(channel < CHNUM && uvonch_last <= CHNUM) 
    {
        if(PHY_CH[channel].Ready == 1 && PHY_CH[channel].Uvon == 0 && PHY_CH[channel].Level != 0) //UV LED ON
        {
            PHY_CH[channel].Uvon = 1;
            PHY_CH[channel].Ready = 0;
            PHY_CH[channel].Uvledon.timer = PHY_CH[channel].Time;
            PHY_CH[ci].Uvledon.flag_off = 0;
            PHY_Uvled_PwmSwitch(channel,1);
            Bueezr_Config(200,0,0);
            CH_LED_switch(channel,0,1);
            uvonch_last = ch;
            Time_uvch[channel].timer = 0;
            time_allch = 0;
            TM1639_LED_switch(LED_LEVEL,0);
            TM1639_LED_switch(LED_TIME,0);
        }
        else if(PHY_CH[channel].Uvon == 1) //UV LED OFF
        {
            PHY_CH[channel].Uvledon.timer = 0;
            PHY_CH[channel].Uvledon.flag_off = 0;
            PHY_Uvled_PwmSwitch(channel,0);
            CH_LED_switch(channel,0,0);
            PHY_CH[channel].Uvon = 0;
        }
    }
    /* If all the channels are selected and turned on, then only the option of turning off all channels is available.
       Selecting a single channel is not possible. */
    else if(channel >= CHNUM && (uvonch_last == 0 || uvonch_last > CHNUM) && _check_uvled_errLd123() == 0) //ALL
    {
        if(_check_uvled_sta() == 0)
        {
            for(ci = 0; ci < CHNUM; ci++)
            {
                if(PHY_CH[ci].Ready == 1 && PHY_CH[ci].Uvon == 0 && PHY_CH[channel].Level != 0)
                {
                    PHY_CH[ci].Uvon = 1;
                    PHY_CH[ci].Ready = 0;
                    PHY_CH[ci].Uvledon.timer = PHY_CH[ci].Time;
                    PHY_CH[ci].Uvledon.flag_off = 0;
                    Bueezr_Config(200,0,0);
                    PHY_Uvled_PwmSwitch(ci,1);
                    CH_LED_switch(ci,0,1);
                    uvonch_last = ch;
                    Time_uvch[ci].timer = 0;
                    time_allch = 0;
                    TM1639_LED_switch(LED_LEVEL,0);
                    TM1639_LED_switch(LED_TIME,0);
                }
            }
        }
        else
        {
            PHY_UvLed_Off();
        }
    }
}

void PHY_UvLed_Off(void)
{
    u8 ch = 0;
    for(ch = 0; ch < CHNUM; ch++)
    { 
        PHY_CH[ch].Uvledon.timer = 0;
        PHY_CH[ch].Uvledon.flag_off = 0;
        PHY_Uvled_PwmSwitch(ch,0);
        CH_LED_switch(ch,0,0);
        PHY_CH[ch].Uvon = 0;
    }
}

void PHY_UvLed_WaitOff(void)
{
    u8 ch = 0;
    for(ch = 0; ch < CHNUM; ch++)
    {
        if(PHY_CH[ch].Uvledon.flag_off == 1)
        {
            PHY_CH[ch].Uvledon.flag_off = 0;
            CH_LED_switch(ch,0,0);
            display_uvled_time(Time_uvch[ch].timer);
            if(_check_uvled_sta() == 0) Bueezr_Config(1000,0,0);
        }
    }
}

void PHY_Uvled_PwmSwitch(u8 ch, u8 onoff)
{
    if(ch > CHNUM-1) return;
    if(onoff)
    {
        if(PHY_CH[ch].Level == 100)
        {
            timer1_channel_gpiomode(ch,1,PHY_CH[ch].Level);
        }
        else
        {
            timer1_channel_gpiomode(ch,0,0);
            tim_pwm_output_enable_ctrl(TIM1,ch,1);
        }
        PHY_CH[ch].undercurr_cnt = 0;
        PHY_CH[ch].overcurr_cnt = 0;
        _UVLED_CurrCheck_Enable();
    }
    else
    {
        PHY_CH[ch].undercurr_cnt = 0;
        PHY_CH[ch].overcurr_cnt = 0;
        timer1_channel_gpiomode(ch,1,0);
    }
}

void PHY_UVLed_Connect_Check(void)
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
        if((((UVCh_Check.Flag >> (ch*2)) & CHECK_NONE) == CHECK_NONE) && PHY_CH[ch].Error_Connect == 0)
        {
            PHY_CH[ch].Error_Connect = 2;
            CH_LED_switch(ch,PHY_CH[ch].Error_Connect,1);
            PHY_Uvled_PwmSwitch(ch,0);
            PHY_CH[ch].Uvon = 0;
            if(ch == run_ch-1) TM1639_Display_UVLED_Char(DISPLAY_Ld3);
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
                PHY_CH[ch].Uvledon.timer = 0;
                PHY_CH[ch].Uvledon.flag_off = 0;
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
        if(PHY_CH[ch-1].Error_Connect || PHY_CH[ch-1].Error_Curr)
        {
            if(PHY_CH[ch-1].Error_Connect == 1)      TM1639_Display_UVLED_Char(DISPLAY_BLANK);
            else if(PHY_CH[ch-1].Error_Connect == 2) TM1639_Display_UVLED_Char(DISPLAY_Ld3);
            else if(PHY_CH[ch-1].Error_Curr == 1)    TM1639_Display_UVLED_Char(DISPLAY_Ld2);
            else if(PHY_CH[ch-1].Error_Curr == 2)    TM1639_Display_UVLED_Char(DISPLAY_Ld1);
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
                else if(PHY_CH[ci].Error_Curr)
                {
                    if(PHY_CH[ci].Error_Curr == 1)        TM1639_Display_UVLED_Char(DISPLAY_Ld2);
                    else if(PHY_CH[ci].Error_Curr == 2)   TM1639_Display_UVLED_Char(DISPLAY_Ld1);
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

