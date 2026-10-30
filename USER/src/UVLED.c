#include "UVLED.h"
#include "gpio.h"
#include "timer.h"
#include "adc.h"
#include "Application.h"
#include "eeprom.h"

u8 _check_uvled_current_err(void)
{
    u8 i = 0;
    i = PHY_CH[0].Error_Curr | (PHY_CH[1].Error_Curr<<1) | (PHY_CH[2].Error_Curr<<2) | (PHY_CH[3].Error_Curr<<3);
    return i;
}

u8 _check_uvled_error(void)
{
    u8 i = 0;
    i = (_check_uvled_connect_err() << 4) | _check_uvled_current_err() | Err_Emg;
    return i;
}

u8 _check_err_extout(void)
{
    if(_check_poweron_allch_con_err() || _check_uvled_errLd123()) return 1;
    else return 0;
}

u8 _check_uvled_errLd123(void)
{
    return (PHY_CH[0].Error_Connect>=2 || PHY_CH[1].Error_Connect>=2 || PHY_CH[2].Error_Connect>=2 || PHY_CH[3].Error_Connect>=2
           || _check_uvled_current_err() || Err_Emg);
}

u8 _check_poweron_allch_con_err(void)
{
    return (PHY_CH[0].Error_Connect==1 && PHY_CH[1].Error_Connect==1 && PHY_CH[2].Error_Connect==1 && PHY_CH[3].Error_Connect==1);
}

u8 _check_uvled_ch_error(u8 ch)
{
    u8 i = 0;
    if(ch > CHNUM-1) return 1;
    i = PHY_CH[ch].Error_Connect | (PHY_CH[ch].Error_Curr) | Err_Emg;
    return i;
}

u8 _check_uvled_connect_err(void)
{
    u8 i = 0;
    if((PHY_CH[0].Error_Connect==1) && (PHY_CH[1].Error_Connect==1) && (PHY_CH[2].Error_Connect==1) && (PHY_CH[3].Error_Connect==1))
        i = 0x0F;
    return i;
}

u8 _check_uvled_connect_Ld3(void)
{
    u8 i = 0;
    for(i = 0; i < CHNUM; i++)
    {
        if(PHY_CH[i].Error_Connect == ERR_Ld3) return 1;
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

u8 _check_uvled_ready(void)
{
    u8 i = 0;
    i = PHY_CH[0].Ready | (PHY_CH[1].Ready<<1) | (PHY_CH[2].Ready<<2) | (PHY_CH[3].Ready<<3);
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
            PHY_CH[i].SetTime = time;;
            SEL_CH.SetTime = time;
        }
    }
}

void _set_uvled_timer(u8 ch, u32 timer)
{
    if(ch >= CHNUM) return;
    PHY_CH[ch].SetTime = timer;
}

u8 _get_uvled_level(void)
{
    u8 ui = 0;
    for(ui = 0; ui < CHNUM; ui++)
    {
        if(PHY_CH[ui].Option)   return PHY_CH[ui].SetLevel;
    }
    return 0;
}

u32 _get_uvled_time(void)
{
    u8 ui = 0;
    for(ui = 0; ui < CHNUM; ui++)
    {
        if(PHY_CH[ui].Option)   return PHY_CH[ui].SetTime;
    }
    return 0;
}

void uvled_ready_check(void)
{
    u8 ch = 0;
    for(ch = 0; ch < CHNUM; ch++)
    {
        if(Err_Emg==0 && rs_mode==RUN_MODE && PHY_CH[ch].Error_Connect==0 && PHY_CH[ch].Error_Curr==0 && PHY_CH[ch].Uvon==0)
        {
            PHY_CH[ch].Ready = 1;
        }
        else PHY_CH[ch].Ready = 0;
       
        ext_ch_ready_output(ch,PHY_CH[ch].Ready);
    }
    
    EXT_UVON_OUT(!_check_uvled_sta());
    
    EXT_ERROR_OUT(!_check_err_extout());
}

void ext_ch_ready_output(u8 ch,u8 sta)
{
    switch(ch)
    {
        case 0:
            EXT_RYD1_OUT(!sta);
            break;
        
        case 1:
            EXT_RYD2_OUT(!sta);
            break;
        
        case 2:
            EXT_RYD3_OUT(!sta);
            break;
        
        case 3:
            EXT_RYD4_OUT(!sta);
            break;
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
        if(PHY_CH[channel].Ready == 1 && PHY_CH[channel].Uvon == 0 && PHY_CH[channel].SetLevel != 0) //UV LED ON
        {
            PHY_CH[channel].Ready = 0;
            PHY_CH[channel].Uvledon.timer = PHY_CH[channel].SetTime;
            UV_LED_PwmSet(channel, PHY_CH[channel].SetLevel);
            PHY_CH[channel].Uvledon.flag_off = 0;
            PHY_Uvled_PwmSwitch(channel,1);
            Bueezr_Config(250,0,0);
            CH_LED_switch(channel,0,1);
            uvonch_last = ch;
            Time_uvch[channel].timer = 0;
            PHY_CH[channel].Ontime = 0;
            time_allch = 0;
            flag_single_chon = 1;
            //TM1639_LED_switch(LED_LEVEL,0);
            //TM1639_LED_switch(LED_TIME,0);
        }
        else if(PHY_CH[channel].Uvon == 1 && PHY_CH[channel].flag_ext_uvon == 0) //UV LED OFF
        {
            PHY_CH[channel].Uvledon.timer = 0;
            PHY_CH[channel].Uvledon.flag_off = 0;
            PHY_Uvled_PwmSwitch(channel,0);
            CH_LED_switch(channel,0,0);
            Bueezr_Config(1000,0,0);
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
                if(PHY_CH[ci].Ready == 1 && PHY_CH[ci].Uvon == 0 && PHY_CH[ci].SetLevel != 0)
                {
                    flag_allch_on = 1;
                    PHY_CH[ci].Ready = 0;
                    PHY_CH[ci].Uvledon.timer = PHY_CH[ci].SetTime;
                    UV_LED_PwmSet(ci, PHY_CH[ci].SetLevel);
                    PHY_CH[ci].Uvledon.flag_off = 0;
                    flag_single_chon = 0;
                    Bueezr_Config(250,0,0);
                    PHY_Uvled_PwmSwitch(ci,1);
                    CH_LED_switch(ci,0,1);
                    uvonch_last = ch;
                    Time_uvch[ci].timer = 0;
                    PHY_CH[ci].Ontime = 0;
                    time_allch = 0;
                    TM1639_LED_switch(LED_LEVEL,0);
                    TM1639_LED_switch(LED_TIME,0);
                }
            }
        }
        else if(flag_extall_on == 0)
        {
            PHY_UvLed_Off();
        }
    }
}

void PHY_Ext_UvLed_switch(u8 ch)
{
    u8 ci = 0,channel = 0;
    if(ch < 1) return;
    channel = ch - 1;
    
    if(_check_uvled_sta() == 0 && _check_uvled_errLd123() == 0) uvonch_last = 0;
    /* If a single channel is selected to be turned on, then only that single channel can be selected to be turned off. 
       Selecting all channels is not possible. */
    if(channel < CHNUM)// && uvonch_last <= CHNUM) 
    {
        if(PHY_CH[channel].Ready == 1 && PHY_CH[channel].Uvon == 0 && PHY_CH[channel].SetLevel != 0) //UV LED ON
        {
            PHY_CH[channel].Ready = 0;
            PHY_CH[channel].Uvledon.timer = PHY_CH[channel].SetTime;
            UV_LED_PwmSet(channel, PHY_CH[channel].SetLevel);
            PHY_CH[channel].Uvledon.flag_off = 0;
            PHY_CH[channel].flag_ext_uvon = 1;
            PHY_Uvled_PwmSwitch(channel,1);
            Bueezr_Config(250,0,0);
            CH_LED_switch(channel,0,1);
            uvonch_last = ch;
            Time_uvch[channel].timer = 0;
            PHY_CH[channel].Ontime = 0;
            time_allch = 0;
            flag_single_chon = 1;
            //TM1639_LED_switch(LED_LEVEL,0);
            //TM1639_LED_switch(LED_TIME,0);
        }
        else if(PHY_CH[channel].Uvon == 1 && PHY_CH[channel].flag_ext_uvon == 1) //UV LED OFF
        {
            PHY_CH[channel].Uvledon.timer = 0;
            PHY_CH[channel].Uvledon.flag_off = 0;
            PHY_CH[channel].flag_ext_type = 0;
            PHY_CH[channel].flag_ext_uvon = 0;
            PHY_Uvled_PwmSwitch(channel,0);
            CH_LED_switch(channel,0,0);
            Bueezr_Config(1000,0,0);
        }
    }
    /* If all the channels are selected and turned on, then only the option of turning off all channels is available.
       Selecting a single channel is not possible. */
    else if(channel >= CHNUM && /*(uvonch_last == 0 || uvonch_last > CHNUM) &&*/ _check_uvled_errLd123() == 0) //ALL
    {
        if(_check_uvled_sta() == 0)
        {
            for(ci = 0; ci < CHNUM; ci++)
            {
                if(PHY_CH[ci].Ready == 1 && PHY_CH[ci].Uvon == 0 && PHY_CH[ci].SetLevel != 0)
                {
                    flag_allch_on = 1;
                    if(Set_Mode.ext_input.setval == 0) flag_extlevel_on = 1;
                    else flag_extlevel_on = 0;
                    flag_extall_on = 1;
                    flag_single_chon = 0;
                    PHY_CH[ci].Ready = 0;
                    PHY_CH[ci].Uvledon.timer = PHY_CH[ci].SetTime;
                    UV_LED_PwmSet(ci, PHY_CH[ci].SetLevel);
                    PHY_CH[ci].Uvledon.flag_off = 0;
                    PHY_CH[ci].flag_ext_uvon = 1;
                    Bueezr_Config(250,0,0);
                    PHY_Uvled_PwmSwitch(ci,1);
                    CH_LED_switch(ci,0,1);
                    uvonch_last = ch;
                    Time_uvch[ci].timer = 0;
                    PHY_CH[ci].Ontime = 0;
                    time_allch = 0;
                    TM1639_LED_switch(LED_LEVEL,0);
                    TM1639_LED_switch(LED_TIME,0);
                }
            }
        }
        else if(flag_extall_on)
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
        PHY_CH[ch].flag_ext_type = 0;
        PHY_CH[ch].flag_ext_uvon = 0;
        time_allch = 0;
        uvonch_last = 0;
        flag_allch_on = 0;
        flag_extall_on = 0;
        flag_single_chon = 0;
        flag_extlevel_on = 0;
        PHY_Uvled_PwmSwitch(ch,0);
        PHY_Light_UvonLED(ch,0);
        if(_check_uvled_sta() == 0) Bueezr_Config(1000,0,0);
    }
}

void PHY_Ext_UvLed_Off(void)
{
    u8 ch = 0;
    for(ch = 0; ch < CHNUM; ch++)
    {
        if(Set_Mode.trg_pri.setval || PHY_CH[ch].SetTime == 0) //SiG Signal priority
        {
            PHY_CH[ch].Uvledon.timer = 0;
            PHY_CH[ch].Uvledon.flag_off = 0;
            PHY_CH[ch].flag_ext_type = 0;
            PHY_CH[ch].flag_ext_uvon = 0;
            flag_extlevel_on = 0;
            PHY_Uvled_PwmSwitch(ch,0);
            PHY_Light_UvonLED(ch,0);
        }
        if(_check_uvled_sta() == 0)
        {
            time_allch = 0;
            uvonch_last = 0;
            flag_allch_on = 0;
            flag_single_chon = 0;
            flag_extall_on = 0;
            flag_extlevel_on = 0;
            Bueezr_Config(1000,0,0);
        }
    }
}

void PHY_PowerOff_UvLed_Off(void)
{
    u8 ch = 0;
    for(ch = 0; ch < CHNUM; ch++)
    { 
        PHY_CH[ch].Uvledon.timer = 0;
        PHY_CH[ch].Uvledon.flag_off = 0;
        PHY_CH[ch].flag_ext_type = 0;
        PHY_CH[ch].flag_ext_uvon = 0;
        time_allch = 0;
        uvonch_last = 0;
        flag_allch_on = 0;
        flag_single_chon = 0;
        flag_extall_on = 0;
        flag_extlevel_on = 0;
        PHY_Light_UvonLED(ch,0);
        PHY_Uvled_PwmSwitch(ch,0);
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
            display_uvled_time(PHY_CH[ch].Ontime,1);
            if(_check_uvled_sta() == 0) Bueezr_Config(1000,0,0);
        }
    }
    if(_check_uvled_sta() == 0)
    {
        if(flag_extall_on) _clear_ExtInFlag();
        flag_extall_on = 0;
        flag_single_chon = 0;
        flag_extlevel_on = 0;
        _UVLED_CurrCheck_Disable();
        eeprom_save_parameter(SAVE_CUMUTIME);
    }
}

void PHY_Uvled_PwmSwitch(u8 ch, u8 onoff)
{
    if(ch > CHNUM-1) return;
    if(onoff)
    {/*
        if(flag_test_mode && PHY_CH[ch].Level == 100) 
        {
            timer1_channel_gpiomode(ch,1,PHY_CH[ch].Level);
        }
        else*/
        timer1_channel_gpiomode(ch,0,0);
        tim_pwm_output_enable_ctrl(TIM1,ch,1);
        PHY_CH[ch].undercurr_cnt = 0;
        PHY_CH[ch].overcurr_cnt = 0;
        _UVLED_CurrCheck_Enable();
        PHY_CH[ch].Uvon = 1;
    }
    else
    {
        PHY_CH[ch].undercurr_cnt = 0;
        PHY_CH[ch].overcurr_cnt = 0;
        if(flag_test_mode == 0) 
        {
            if(PHY_CH[ch].Uvon) PHY_CH[ch].flag_update_cumutime = 1;
        }
        Time_uvch[ch].timer = 0;
        PHY_CH[ch].Uvon = 0;
        timer1_channel_gpiomode(ch,1,0);
    }
}

void PHY_UVLed_Connect_Check(void)
{
    u8 ch = 0,chnum = CHNUM;
    
    if(Err_Emg) return;
    UVCh_Check.b0 = CH1_CHECK1_IN;
    UVCh_Check.b1 = CH1_CHECK2_IN;
    UVCh_Check.b2 = CH2_CHECK1_IN;
    UVCh_Check.b3 = CH2_CHECK2_IN;
    if(Set_Mode.H_4VH.setval == 0)
    {
        UVCh_Check.b4 = CH3_CHECK1_IN;
        UVCh_Check.b5 = CH3_CHECK2_IN;
        UVCh_Check.b6 = CH4_CHECK1_IN;
        UVCh_Check.b7 = CH4_CHECK2_IN;
    }
    else
    {
        chnum = 2;
        PHY_CH[2].Error_Connect = 1;
        PHY_CH[3].Error_Connect = 1;
    }

    for(ch = 0; ch < chnum; ch++)
    {
        if((((UVCh_Check.Flag >> (ch*2)) & CHECK_NONE) == CHECK_NONE) && PHY_CH[ch].Error_Connect == 0)
        {
            PHY_CH[ch].Error_Connect = ERR_Ld3;
            PHY_CH[ch].ErrCode = ERR_Ld3;
            CH_LED_switch(ch,PHY_CH[ch].Error_Connect,1);
            PHY_Uvled_PwmSwitch(ch,0);
            Bueezr_Config(250,100,3);
            if(ch == run_ch-1) TM1639_Display_UVLED_Char(DISPLAY_Ld3);
            flag_tx_error = 1;
        }
    }
}

void PHY_UVLed_Sel_opt(u8 ch)
{
    switch(ch)
    {
        case 1:
            PHY_CH[0].Option = 1;
            PHY_CH[1].Option = 0;
            PHY_CH[2].Option = 0;
            PHY_CH[3].Option = 0;
            SEL_CH.Uvch = 1;
            break;
        
        case 2:
            PHY_CH[0].Option = 0;
            PHY_CH[1].Option = 1;
            PHY_CH[2].Option = 0;
            PHY_CH[3].Option = 0;
            SEL_CH.Uvch = 2;
            break;
        
        case 3:
            PHY_CH[0].Option = 0;
            PHY_CH[1].Option = 0;
            PHY_CH[2].Option = 1;
            PHY_CH[3].Option = 0;
            SEL_CH.Uvch = 3;
            break;
        
        case 4:
            PHY_CH[0].Option = 0;
            PHY_CH[1].Option = 0;
            PHY_CH[2].Option = 0;
            PHY_CH[3].Option = 1;
            SEL_CH.Uvch = 4;
            break;
        
        case 5:
            PHY_CH[0].Option = 1;
            PHY_CH[1].Option = 1;
            PHY_CH[2].Option = 1;
            PHY_CH[3].Option = 1;
            SEL_CH.Uvch = 5;
            break;
    }
}

void PHY_UVLed_Select(u8 ch,u8 d_sw)
{
    u8 ci = 0;
    if(ch < 1) ch = 1;
    if(ch < 5)
    {
        if(PHY_CH[ch-1].Error_Connect || PHY_CH[ch-1].Error_Curr)
        {
            //if(PHY_CH[ch-1].Error_Connect == 1)      TM1639_Display_UVLED_Char(DISPLAY_BLANK);
            //else if(PHY_CH[ch-1].Error_Connect == 2) TM1639_Display_UVLED_Char(DISPLAY_Ld3);
            //else if(PHY_CH[ch-1].Error_Curr == ERR_Ld2)    TM1639_Display_UVLED_Char(DISPLAY_Ld2);
            //else if(PHY_CH[ch-1].Error_Curr == ERR_Ld1)    TM1639_Display_UVLED_Char(DISPLAY_Ld1);
            if(d_sw) TM1639_Display_UVLED_Char((DISPLAY_ENUM)(ch+15));
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
            if(d_sw) TM1639_Display_UVLED_Char((DISPLAY_ENUM)(ch+15));
            break;
        
        case 2:
            PHY_CH[0].Option = 0;
            PHY_CH[1].Option = 1;
            PHY_CH[2].Option = 0;
            PHY_CH[3].Option = 0;
            SEL_CH.Uvch = 2;
            if(d_sw) TM1639_Display_UVLED_Char((DISPLAY_ENUM)(ch+15));
            break;
        
        case 3:
            PHY_CH[0].Option = 0;
            PHY_CH[1].Option = 0;
            PHY_CH[2].Option = 1;
            PHY_CH[3].Option = 0;
            SEL_CH.Uvch = 3;
            if(d_sw) TM1639_Display_UVLED_Char((DISPLAY_ENUM)(ch+15));
            break;
        
        case 4:
            PHY_CH[0].Option = 0;
            PHY_CH[1].Option = 0;
            PHY_CH[2].Option = 0;
            PHY_CH[3].Option = 1;
            SEL_CH.Uvch = 4;
            if(d_sw) TM1639_Display_UVLED_Char((DISPLAY_ENUM)(ch+15));
            break;
        
        case 5:
            for(ci = 0; ci < CHNUM; ci++)
            {
                if(PHY_CH[ci].Error_Connect == 0) PHY_CH[ci].Option = 1;
                else PHY_CH[ci].Option = 0;
                if(PHY_CH[ci].Error_Connect > 1)
                {
                    if(PHY_CH[ci].Error_Connect == ERR_Ld3) TM1639_Display_UVLED_Char(DISPLAY_Ld3);
                    else if(PHY_CH[ci].Error_Connect == ERR_Ld4) TM1639_Display_UVLED_Char(DISPLAY_Ld4);
                    PHY_CH[0].Option = 0;
                    PHY_CH[1].Option = 0;
                    PHY_CH[2].Option = 0;
                    PHY_CH[3].Option = 0;
                    return;
                }
                else if(PHY_CH[ci].Error_Curr)
                {
                    if(PHY_CH[ci].Error_Curr == ERR_Ld2)        TM1639_Display_UVLED_Char(DISPLAY_Ld2);
                    else if(PHY_CH[ci].Error_Curr == ERR_Ld1)   TM1639_Display_UVLED_Char(DISPLAY_Ld1);
                    PHY_CH[0].Option = 0;
                    PHY_CH[1].Option = 0;
                    PHY_CH[2].Option = 0;
                    PHY_CH[3].Option = 0;
                    return;
                }
            }
            if(_check_uvled_connect_err() == 0x0F) TM1639_Display_UVLED_Char(DISPLAY_Ld3);
            else if(d_sw) TM1639_Display_UVLED_Char(DISPLAY_ALL);
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
