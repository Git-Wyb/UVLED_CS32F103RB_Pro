#include "Application.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_misc.h"
#include "UVLED.h"
#include "gpio.h"
#include "usart.h"
#include "eeprom.h"

#define CNTMAX  2
#define SCANMAX 4 /* x10ms */
/*
LEVEL    MENU       MODE
TIME     HOUR
CH1  CH2  CH3  CH4  CH
          ^
          
<<BK     SET        FW>> 

          V         UV ON
*/
u8 *p_param_set[7] = {NULL,&Set_Mode_Cache.ext_input.setval, &Set_Mode_Cache.trg_pri.setval, &Set_Mode_Cache.buzzer_sw.setval,
                           &Set_Mode_Cache.opl_not.setval,   &Set_Mode_Cache.H_4VH.setval,   &Set_Mode_Cache.usart_baud.setval};
u8 upcnt = 0;
u8 downcnt = 0;
u8 setcnt = 0;
u8 uvoncnt = 0;
u8 chlevel = 0;
u8 chcnt = 0;
u8 chnum = 0;
u16 chtime = 0;
u8 uvon = 0;
u8 levelcnt = 0;
u8 menucnt = 0;
u8 modecnt = 0;
u8 timecnt = 0;
u8 hourcnt = 0;
u8 bkcnt = 0;
u8 fwcnt = 0;
u8 moden = 0;
u8 _scnt = 0;
u8 m_mode_cnt = 0;
u8 _bf_cnt = 0;
u8 _set_menu_cnt = 0;
u8 _mcnt = 0;
u8 _hcnt = 0;
u8 _menuset_cnt = 0;
u8 _keyup_cnt = 0;
u8 _keydown_cnt = 0;
u8 _keybk_cnt = 0;
u8 _keyfw_cnt = 0;
u8 fbuff[11] = {0,1,2,3,5,10,15,20,50,100};
u8 trgcnt = 0;
u8 emgcnt = 0;
u8 trg1cnt = 0;
u8 trg2cnt = 0;
u8 trg3cnt = 0;
u8 trg4cnt = 0;
u32 _timecache = 0;

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
    uvled_ready_check();
    PHY_UvLed_Mode_Handle();
    PHY_Ext_Uvled_Handle();
    PHY_CH_Error_Display(run_ch-1);
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
            KeyStaFlag.Up = 1;
        }
    }else 
    {
        upcnt = 0;
        if(KeySta.Up == 0) 
        {
            KeyStaNow.Up = 0;
            KeyStaFlag.Up = 0;
            _keyup_cnt = 0;
            flag_keyup_long = 0;
        }
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
            KeyStaFlag.Down = 1;
        }
    }else 
    {
        downcnt = 0;
        if(KeySta.Down == 0) 
        {
            KeyStaNow.Down = 0;
            KeyStaFlag.Down = 0;
            _keydown_cnt = 0;
            flag_keydown_long = 0;
        }
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
            KeyStaFlag.Set = 1;
        }
    }else 
    {
        setcnt = 0;
        if(KeySta.Set == 0)
        {
            KeyStaNow.Set = 0;
            KeyStaFlag.Set = 0;
            _menuset_cnt = 0;
        }
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
            KeyStaFlag.Bk = 1;
        }
    }else 
    {
        bkcnt = 0;
        if(KeySta.Bk == 0) 
        {
            KeyStaNow.Bk = 0;
            KeyStaFlag.Bk = 0;
            _keybk_cnt = 0;
            flag_keybk_long = 0;
        }
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
            KeyStaFlag.Fw = 1;
        }
    }else 
    {
        fwcnt = 0;
        if(KeySta.Fw == 0) 
        {
            KeyStaNow.Fw = 0;
            KeyStaFlag.Fw = 0;
            _keyfw_cnt = 0;
            flag_keyfw_long = 0;
        }
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
            KeyStaFlag.Uvon = 1;
        }
    }else 
    {
        uvoncnt = 0;
        if(KeySta.Uvon == 0)
        {
            KeyStaNow.Uvon = 0;
            KeyStaFlag.Uvon = 0;
        }
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
            KeyStaFlag.Ch = 1;
        }
    }else 
    {
        chcnt = 0;
        if(KeySta.Ch == 0)
        {
            KeyStaNow.Ch = 0;
            KeyStaFlag.Ch = 0;
        }
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
            KeyStaFlag.Mode = 1;
        }
    }else 
    {
        modecnt = 0;
        if(KeySta.Mode == 0)
        {
            KeyStaNow.Mode = 0;
            KeyStaFlag.Mode = 0;
        }
    }
}

void PHY_UvLed_Mode_Handle(void)
{
    if(KeyStaFlag.Mode)
    {
        _clear_KeyStaFlag();
        if(_check_uvled_sta())  return;
        
        if(rs_mode != SETTING_MODE)
        {
            rs_mode = SETTING_MODE;
            PHY_Mode_Switch(rs_mode);
        }
        else
        {
            rs_mode = RUN_MODE;
            PHY_Mode_Switch(rs_mode);
        }
    }
    switch(rs_mode)
    {
        case RUN_MODE:
            if(_check_poweron_allch_con_err() || Err_Emg) return;
        
            if(KeyStaFlag.Uvon)
            {
                _clear_KeyStaFlag();
                PHY_UvLed_switch(run_ch);
            }
            else if(KeyStaFlag.Ch)
            {
                _clear_KeyStaFlag();
                run_ch++;
                if(run_ch > CHNUM+1) run_ch = 1;
                if(_check_uvled_errLd123() && run_ch==CHNUM+1) run_ch = 1; //Error occurred; selecting "all" is not allowed.
                PHY_UVLed_Select(run_ch,1);
                time_display_refresh = 500;
                //PHY_Light_ModeLED(LED_MODE_OFF);
            }
            else if(KeyStaFlag.Up)
            {
                _clear_KeyStaFlag();
                if(_check_uvled_sta())  return;
                if(_check_uvled_option() && run_ch < CHNUM+1)
                {
                    TM1639_DisplayNum(_get_uvled_level());
                    PHY_Light_ModeLED(LED_LEVEL);
                }
            }
            else if(KeyStaFlag.Down)
            {
                _clear_KeyStaFlag();
                if(_check_uvled_sta())  return;
                if(_check_uvled_option() && run_ch < CHNUM+1)
                {
                    TM1639_DisplayNum(_get_uvled_level());
                    PHY_Light_ModeLED(LED_LEVEL);
                }
            }
            else if(KeyStaFlag.Bk)
            {
                _clear_KeyStaFlag();
                if(_check_uvled_sta())  return;
                if(_check_uvled_option() && run_ch < CHNUM+1)
                {
                    display_uvled_time(_get_uvled_time(),1);
                    PHY_Light_ModeLED(LED_TIME);
                }
            }
            else if(KeyStaFlag.Fw)
            {
                _clear_KeyStaFlag();
                if(_check_uvled_sta())  return;
                if(_check_uvled_option() && run_ch < CHNUM+1)
                {
                    display_uvled_time(_get_uvled_time(),1);
                    PHY_Light_ModeLED(LED_TIME);
                }
            }
            break;
        
        case SETTING_MODE:
            if(_check_poweron_allch_con_err() || Err_Emg)
            {
                if(KeyStaFlag.Set)
                {
                    rs_mode = MH_MODE_SWITCH;
                }
            }
            else if(Set_Mode.opl_not.setval) //OPL ON
            {
                TM1639_Display_UVLED_Char(DISPLAY_oPL);
                if(KeyStaFlag.Set)
                {
                    rs_mode = MH_MODE_SWITCH;
                }
                return;
            }
            else if(KeyStaFlag.Ch)
            {
                _clear_KeyStaFlag();
                run_ch++;
                if(run_ch > CHNUM) run_ch = 1;
                PHY_UVLed_Select(run_ch,1);
                PHY_Light_ModeLED(LED_MODE_OFF);
                PHY_Light_ModeLED(LED_CH_OFF);
            }
            else if(KeyStaFlag.Up)
            {
                if(_check_uvled_option() && run_ch <= CHNUM)
                {
                    _keyup_cnt++;
                    if(_check_uvled_ch_error(run_ch-1)) return;
                    
                    if(flag_keyup_long == 0 || _keyup_cnt >= 15) //300ms
                    {
                        if(flag_keyup_long == 0) _keyup_cnt = 0;
                        else _keyup_cnt = 10;

                        chlevel = _get_uvled_level();
                        if(flag_keyup_long) 
                        {
                            if(chlevel <= 90) chlevel += 10;
                        }
                        else if(chlevel < 100) chlevel++;

                        UV_LED_PwmSet(run_ch-1,chlevel);
                        TM1639_DisplayNum(chlevel);
                        flag_keyup_long = 1;
                    }
                }
            }
            else if(KeyStaFlag.Down)
            {
                if(_check_uvled_option() && run_ch <= CHNUM)
                {
                    _keydown_cnt++;
                    if(_check_uvled_ch_error(run_ch-1)) return;
                    
                    if(flag_keydown_long == 0 || _keydown_cnt >= 15) //300ms
                    {
                        if(flag_keydown_long == 0) _keydown_cnt = 0;
                        else _keydown_cnt = 10;

                        chlevel = _get_uvled_level();
                        if(flag_keydown_long)
                        {
                            if(chlevel >= 10) chlevel -= 10;
                        }
                        else if(chlevel != 0) chlevel--;
                        UV_LED_PwmSet(run_ch-1,chlevel);
                        TM1639_DisplayNum(chlevel);
                        flag_keydown_long = 1;
                    }
                }
            }
            else if(KeyStaFlag.Bk)
            {
                if(_check_uvled_option() && run_ch <= CHNUM)
                {
                    _keybk_cnt++;
                    if(_check_uvled_ch_error(run_ch-1)) return;
                    
                    if(flag_keybk_long == 0 || _keybk_cnt >= 15) //300ms
                    {
                        if(flag_keybk_long == 0) _keybk_cnt = 0;
                        else _keybk_cnt = 10;
                        
                        chtime = _get_uvled_time();
                        if(flag_keybk_long)
                        {
                            if(chtime>990 && chtime<=999) chtime = 1000;
                            else if(chtime < 999) chtime += 10; //1s step
                            else if(chtime < TIME_COUNT_MAX-100) chtime += 100; //10s step
                        }
                        else
                        {
                            if(chtime == 999) chtime = 1000;
                            else if(chtime < 999) chtime++; //100ms step
                            else if(chtime < TIME_COUNT_MAX) chtime += 10; //1s step
                        }
                        
                        PHY_Set_UVLed_Time(chtime); //chtime * 100ms
                        display_uvled_time(chtime,1);
                        flag_keybk_long = 1;
                    }
                }
            }
            else if(KeyStaFlag.Fw)
            {
                if(_check_uvled_option() && run_ch <= CHNUM)
                {
                    if(_check_uvled_ch_error(run_ch-1)) return;
                    
                    _keyfw_cnt++;
                    if(flag_keyfw_long == 0 || _keyfw_cnt >= 15) //300ms
                    {
                        if(flag_keyfw_long == 0) _keyfw_cnt = 0;
                        else _keyfw_cnt = 10;
                        
                        chtime = _get_uvled_time();
                        if(flag_keyfw_long)
                        {
                            if(chtime > 999) chtime -= 100; //10s step
                            else if(chtime > 10) chtime -= 10; //1s step
                        }
                        else
                        {
                            if(chtime > 1000) chtime -= 10; //1s step
                            else if(chtime != 0) chtime--; //100ms step
                        }
                        PHY_Set_UVLed_Time(chtime);
                        display_uvled_time(chtime,1);
                        flag_keyfw_long = 1;
                    }
                }
            }
            else if(KeyStaFlag.Set)
            {
                rs_mode = MH_MODE_SWITCH;
            }
            break;
            
        case MH_MODE_SWITCH:
            if(KeyStaFlag.Set)
            {
                _scnt++;
                if(_scnt >= 40) //800ms
                {
                    _clear_KeyStaFlag();
                    rs_mode = SETTING_MODE;
                }
                if(KeyStaFlag.Bk)
                {
                    _clear_KeyStaFlag();
                    Set_Mode_Cache = Set_Mode;
                    rs_mode = MENU_MODE;
                    PHY_Mode_Switch(rs_mode);
                }
                else if(KeyStaFlag.Fw)
                {
                    _clear_KeyStaFlag();
                    rs_mode = HOUR_MODE;
                    PHY_Mode_Switch(rs_mode);
                }
            }
            else
            {
                _clear_KeyStaFlag();
                rs_mode = SETTING_MODE;
            }
            break;
        
        case MENU_MODE:
            if(KeyStaFlag.Byte != 0)
            {
                _mcnt++;
                if(_mcnt >= 2) //40ms
                {
                    _mcnt = 0;
                    if(KeyStaFlag.Bk && KeyStaFlag.Fw)
                    {
                        _bf_cnt++;
                        if(_bf_cnt >= 75) //3s
                        {
                            _bf_cnt = 0;
                            Set_Mode = Initial_ModeSet;
                            Set_Mode_Cache = Set_Mode;
                            m_mode_cnt = SET_EIN;
                            TM1639_Display_UVLED_Char(DISPLAY_Ein);
                            _clear_KeyStaFlag();
                            eeprom_save_parameter(SAVE_PARAMETER);
                            Bueezr_Config(1000,0,0);
                        }
                    }
                    else if(KeyStaFlag.Bk)
                    {
                        _clear_KeyStaFlag();
                        m_mode_cnt++;
                        if(m_mode_cnt > 6) m_mode_cnt = 1;
                        menuset_handle(rs_mode,m_mode_cnt,0);
                    }
                    else if(KeyStaFlag.Fw)
                    {
                        _clear_KeyStaFlag();
                        if(m_mode_cnt != 0) m_mode_cnt--;
                        if(m_mode_cnt == 0) m_mode_cnt = 6;
                        menuset_handle(rs_mode,m_mode_cnt,0);
                    }
                    else if(KeyStaFlag.Set)
                    {
                        _menuset_cnt++;
                        if(_menuset_cnt < 2)
                        {
                            rs_mode = MENU_MODE_SET;
                            _set_menu_cnt = *p_param_set[m_mode_cnt];
                            menuset_handle(rs_mode,m_mode_cnt,_set_menu_cnt);
                        }
                        if(_menuset_cnt >= 37) //3s
                        {
                            _menuset_cnt = 0;
                            _clear_KeyStaFlag();
                            Set_Mode = Set_Mode_Cache;
                            Init_Usart1();
                            eeprom_save_parameter(SAVE_PARAMETER);
                            Bueezr_Config(1000,0,0);
                            rs_mode = SETTING_MODE;
                            PHY_Mode_Switch(rs_mode);
                        }
                    }
                }
            }else _mcnt = 0;
            break;
        
        case HOUR_MODE:
            if(KeyStaFlag.Byte != 0)
            {
                _hcnt++;
                if(_hcnt >= 2) //40ms
                {
                    _hcnt = 0;
                    if(KeyStaFlag.Bk && KeyStaFlag.Fw)
                    {
                        _bf_cnt++;
                        if(_bf_cnt >= 75) //3s
                        {
                            _bf_cnt = 0;
                            if(run_ch > 0 && run_ch <= CHNUM)
                            {
                                PHY_CH[run_ch-1].pCH.CumulTime = 0;
                                PHY_CH[run_ch-1].flag_update_cumutime = 1;
                                eeprom_save_parameter(SAVE_CUMUTIME);
                                TM1639_Display_UVLED_Char(DISPLAY_000);
                                Bueezr_Config(1000,0,0);
                            }
                            _clear_KeyStaFlag();
                        }
                    }
                    else if(KeyStaFlag.Ch)
                    {
                        _clear_KeyStaFlag();
                        run_ch++;
                        if(run_ch > CHNUM) run_ch = 1;
                        TM1639_Display_UVLED_Char((DISPLAY_ENUM)(run_ch+15));
                    }
                    else if(KeyStaFlag.Bk) //up three positions
                    {
                        _clear_KeyStaFlag();
                        _timecache = PHY_CH[run_ch-1].pCH.CumulTime / 36000; //hour
                        if(_timecache <= 999) TM1639_Display_UVLED_Char(DISPLAY_H00);
                        else
                        {
                            if(_timecache > 50000) 
                            {
                                _timecache = 50000;
                                PHY_CH[run_ch-1].pCH.CumulTime = TIME_CUMUL_MAX;
                            }
                            
                            if(_timecache > 20000)
                            {
                                _timecache = _timecache / 1000;
                                display_cumultime_hour(_timecache,'E');
                            }
                            else
                            {
                                _timecache = _timecache / 1000;
                                display_cumultime_hour(_timecache,'H');
                            }
                        }
                    }
                    else if(KeyStaFlag.Fw) //down three positions
                    {
                        _clear_KeyStaFlag();
                        _timecache = PHY_CH[run_ch-1].pCH.CumulTime / 36000; //hour
                        if(_timecache <= 999) display_cumultime_hour(_timecache,0);
                        else
                        {
                            if(_timecache > 50000) 
                            {
                                _timecache = 50000;
                                PHY_CH[run_ch-1].pCH.CumulTime = TIME_CUMUL_MAX;
                            }
                            _timecache = _timecache % 1000;
                            display_cumultime_hour(_timecache,0);
                        }
                    }
                }
            }else _hcnt = 0;
            break;
        
        case MENU_MODE_SET:
            if(KeyStaFlag.Up)
            {
                _clear_KeyStaFlag();
                if(m_mode_cnt == SET_232 && _set_menu_cnt < 2) _set_menu_cnt++;
                else if(_set_menu_cnt < 1) _set_menu_cnt++;
                menuset_handle(rs_mode,m_mode_cnt,_set_menu_cnt);
            }
            else if(KeyStaFlag.Down)
            {
                _clear_KeyStaFlag();
                if(_set_menu_cnt != 0) _set_menu_cnt--;
                menuset_handle(rs_mode,m_mode_cnt,_set_menu_cnt);
            }
            else if(KeyStaFlag.Set)
            {
                _menuset_cnt++;
                if(_menuset_cnt < 2)
                {
                    rs_mode = MENU_MODE;
                    menuset_handle(rs_mode,m_mode_cnt,0);
                }
                if(_menuset_cnt >= 150) //3s
                {
                    _menuset_cnt = 0;
                    _clear_KeyStaFlag();
                    Set_Mode = Set_Mode_Cache;
                    eeprom_save_parameter(SAVE_PARAMETER);
                    Bueezr_Config(1000,0,0);
                    rs_mode = SETTING_MODE;
                    PHY_Mode_Switch(rs_mode);
                }
            }
            break;
    }
}

void _set_ch_temporary_param(u8 ch)
{
    if(ch < 1) ch = 1;
    run_ch = ch;
    chlevel = PHY_CH[ch-1].SetLevel;
    chtime = PHY_CH[ch-1].SetTime;
}

void PHY_Mode_Switch(MODE mode)
{
    switch(mode)
    {
        case RUN_MODE:
            PHY_Light_ModeLED(LED_RUN_MODE);
            if(Err_Emg)                             TM1639_Display_UVLED_Char(DISPLAY_Eo1);
            else if(_check_poweron_allch_con_err()) TM1639_Display_UVLED_Char(DISPLAY_Ld3);
            else                                    TM1639_Display_UVLED_Char((DISPLAY_ENUM)(run_ch+15));
            break;
        
        case SETTING_MODE:
            PHY_Light_ModeLED(LED_SETTING_MODE);
            if(Err_Emg)                             TM1639_Display_UVLED_Char(DISPLAY_Eo1);
            else if(_check_poweron_allch_con_err()) TM1639_Display_UVLED_Char(DISPLAY_Ld3);
            else
            {
                if(run_ch > CHNUM) run_ch = 1;
                PHY_UVLed_Select(run_ch,1);
                TM1639_Display_UVLED_Char((DISPLAY_ENUM)(run_ch+15));
            }
            break;
        
        case MENU_MODE:
            m_mode_cnt = 1;
            PHY_Light_ModeLED(LED_CH_OFF);
            PHY_Light_ModeLED(LED_MENU);
            TM1639_Display_UVLED_Char(DISPLAY_Ein);
            break;
        
        case HOUR_MODE:
            PHY_Light_ModeLED(LED_CH_OFF);
            PHY_Light_ModeLED(LED_HOUR);
            run_ch = 1;
            TM1639_Display_UVLED_Char((DISPLAY_ENUM)(run_ch+15));
            break;
        
        case MENU_MODE_SET:
            break;
    }
}

void PHY_Light_ModeLED(LED_ENUM mode)
{
    switch(mode)
    {
        case LED_LEVEL:
            TM1639_LED_switch(LED_LEVEL,1);
            TM1639_LED_switch(LED_TIME,0);
            break;
        
        case LED_TIME:
            TM1639_LED_switch(LED_LEVEL,0);
            TM1639_LED_switch(LED_TIME,1);
            break;
        
        case LED_MENU:
            TM1639_LED_switch(LED_LEVEL,0);
            TM1639_LED_switch(LED_TIME,0);
            TM1639_LED_switch(LED_MENU,1);
            TM1639_LED_switch(LED_HOUR,0);
            break;
        
        case LED_HOUR:
            TM1639_LED_switch(LED_LEVEL,0);
            TM1639_LED_switch(LED_TIME,0);
            TM1639_LED_switch(LED_MENU,0);
            TM1639_LED_switch(LED_HOUR,1);
            break;
        
        case LED_ALL_ON:
            TM1639_LED_switch(LED_LEVEL,1);
            TM1639_LED_switch(LED_TIME,1);
            TM1639_LED_switch(LED_MENU,1);
            TM1639_LED_switch(LED_HOUR,1);
            TM1639_LED_switch(LED_CH1_GREEN,1);
            TM1639_LED_switch(LED_CH2_GREEN,1);
            TM1639_LED_switch(LED_CH3_GREEN,1);
            TM1639_LED_switch(LED_CH4_GREEN,1);
            TM1639_LED_switch(LED_CH1_RED,1);
            TM1639_LED_switch(LED_CH2_RED,1);
            TM1639_LED_switch(LED_CH3_RED,1);
            TM1639_LED_switch(LED_CH4_RED,1);
            break;
        
        case LED_MODE_OFF:
            TM1639_LED_switch(LED_LEVEL,0);
            TM1639_LED_switch(LED_TIME,0);
            TM1639_LED_switch(LED_MENU,0);
            TM1639_LED_switch(LED_HOUR,0);
            break;
        
        case LED_CH_OFF:
            TM1639_LED_switch(LED_CH1_GREEN,0);
            TM1639_LED_switch(LED_CH2_GREEN,0);
            TM1639_LED_switch(LED_CH3_GREEN,0);
            TM1639_LED_switch(LED_CH4_GREEN,0);
            break;
        
        case LED_RUN_MODE:
            TM1639_LED_switch(LED_LEVEL,1);
            TM1639_LED_switch(LED_TIME,0);
            TM1639_LED_switch(LED_MENU,0);
            TM1639_LED_switch(LED_HOUR,0);
            TM1639_LED_switch(LED_CH1_GREEN,0);
            TM1639_LED_switch(LED_CH2_GREEN,0);
            TM1639_LED_switch(LED_CH3_GREEN,0);
            TM1639_LED_switch(LED_CH4_GREEN,0);
            break;
        
        case LED_SETTING_MODE:
            TM1639_LED_switch(LED_LEVEL,0);
            TM1639_LED_switch(LED_TIME,0);
            TM1639_LED_switch(LED_MENU,0);
            TM1639_LED_switch(LED_HOUR,0);
            TM1639_LED_switch(LED_CH1_GREEN,0);
            TM1639_LED_switch(LED_CH2_GREEN,0);
            TM1639_LED_switch(LED_CH3_GREEN,0);
            TM1639_LED_switch(LED_CH4_GREEN,0);
            break;
        
        case LED_ALL_OFF:
            TM1639_LED_switch(LED_LEVEL,0);
            TM1639_LED_switch(LED_TIME,0);
            TM1639_LED_switch(LED_MENU,0);
            TM1639_LED_switch(LED_HOUR,0);
            TM1639_LED_switch(LED_CH1_GREEN,0);
            TM1639_LED_switch(LED_CH2_GREEN,0);
            TM1639_LED_switch(LED_CH3_GREEN,0);
            TM1639_LED_switch(LED_CH4_GREEN,0);
            TM1639_LED_switch(LED_CH1_RED,0);
            TM1639_LED_switch(LED_CH2_RED,0);
            TM1639_LED_switch(LED_CH3_RED,0);
            TM1639_LED_switch(LED_CH4_RED,0);
            break;
    }
}

void PHY_Light_ErrorLED(u8 ch, u8 onoff)
{
    if(ch > CHNUM) return;
    if(onoff > 1) onoff = 1;
    
    switch(ch)
    {
        case 0:
            if(onoff)
            {
                TM1639_LED_switch(LED_CH1_GREEN,0);
                TM1639_LED_switch(LED_CH1_RED,1);
            }
            else
            {
                TM1639_LED_switch(LED_CH1_RED,0);
            }
        break;
        
        case 1:
            if(onoff)
            {
                TM1639_LED_switch(LED_CH2_GREEN,0);
                TM1639_LED_switch(LED_CH2_RED,1);
            }
            else
            {
                TM1639_LED_switch(LED_CH2_RED,0);
            }
        break;
        
        case 2:
            if(onoff)
            {
                TM1639_LED_switch(LED_CH3_GREEN,0);
                TM1639_LED_switch(LED_CH3_RED,1);
            }
            else
            {
                TM1639_LED_switch(LED_CH3_RED,0);
            }
        break;
        
        case 3:
            if(onoff)
            {
                TM1639_LED_switch(LED_CH4_GREEN,0);
                TM1639_LED_switch(LED_CH4_RED,1);
            }
            else
            {
                TM1639_LED_switch(LED_CH4_RED,0);
            }
        break;
    }
}

void PHY_Light_UvonLED(u8 ch, u8 onoff)
{
    switch(ch)
    {
        case 0:
            TM1639_LED_switch(LED_CH1_GREEN,onoff);
            break;
        
        case 1:
            TM1639_LED_switch(LED_CH2_GREEN,onoff);
            break;
        
        case 2:
            TM1639_LED_switch(LED_CH3_GREEN,onoff);
            break;
        
        case 3:
            TM1639_LED_switch(LED_CH4_GREEN,onoff);
            break;
    }
    
}

void _clear_KeyStaFlag(void)
{
    KeyStaFlag.Byte = 0;
    _scnt = 0;
    _bf_cnt = 0;
    _mcnt = 0;
    _menuset_cnt = 0;
    _keyup_cnt = 0;
    flag_keyup_long = 0;
    _keydown_cnt = 0;
    flag_keydown_long = 0;
    _keybk_cnt = 0;
    flag_keybk_long = 0;
    _keyfw_cnt = 0;
    flag_keyfw_long = 0;
}

void menuset_handle(u8 type,u8 set,u8 opt)
{
    switch(set)
    {
        case SET_EIN:
            if(type == MENU_MODE) TM1639_Display_UVLED_Char(DISPLAY_Ein);
            else
            {
                if(opt == 0) 
                {
                    TM1639_Display_UVLED_Char(DISPLAY_LEU);
                    Set_Mode_Cache.ext_input.setval = 0;
                }
                else
                {
                    TM1639_Display_UVLED_Char(DISPLAY_PuL);
                    Set_Mode_Cache.ext_input.setval = 1;
                }
            }
            break;
        case SET_PRI:
            if(type == MENU_MODE) TM1639_Display_UVLED_Char(DISPLAY_Pri);
            else
            {
                if(opt == 0) 
                {
                    TM1639_Display_UVLED_Char(DISPLAY_ein);
                    Set_Mode_Cache.trg_pri.setval = 0;
                }
                else
                {
                    TM1639_Display_UVLED_Char(DISPLAY_SiG);
                    Set_Mode_Cache.trg_pri.setval = 1;
                }
            }
            break;
        case SET_BUR:
            if(type == MENU_MODE) TM1639_Display_UVLED_Char(DISPLAY_bur);
            else
            {
                if(opt == 0) 
                {
                    TM1639_Display_UVLED_Char(DISPLAY_oFF);
                    Set_Mode_Cache.buzzer_sw.setval = 0;
                }
                else
                {
                    TM1639_Display_UVLED_Char(DISPLAY_on);
                    Set_Mode_Cache.buzzer_sw.setval = 1;
                }
            }
            break;
        case SET_OPL:
            if(type == MENU_MODE) TM1639_Display_UVLED_Char(DISPLAY_oPL);
            else
            {
                if(opt == 0) 
                {
                    TM1639_Display_UVLED_Char(DISPLAY_oFF);
                    Set_Mode_Cache.opl_not.setval = 0;
                }
                else
                {
                    TM1639_Display_UVLED_Char(DISPLAY_on);
                    Set_Mode_Cache.opl_not.setval = 1;
                }
            }
            break;
        case SET_4UH:
            if(type == MENU_MODE) TM1639_Display_UVLED_Char(DISPLAY_4Uh);
            else
            {
                if(opt == 0) 
                {
                    TM1639_Display_UVLED_Char(DISPLAY_oFF);
                    Set_Mode_Cache.H_4VH.setval = 0;
                }
                else
                {
                    TM1639_Display_UVLED_Char(DISPLAY_on);
                    Set_Mode_Cache.H_4VH.setval = 1;
                }
            }
            break;
        case SET_232:
            if(type == MENU_MODE) TM1639_Display_UVLED_Char(DISPLAY_232);
            else
            {
                if(opt == 0) 
                {
                    TM1639_Display_UVLED_Char(DISPLAY_96);
                    Set_Mode_Cache.usart_baud.setval = 0;
                }
                else if(opt == 1)
                {
                    TM1639_Display_UVLED_Char(DISPLAY_192);
                    Set_Mode_Cache.usart_baud.setval = 1;
                }
                else
                {
                    TM1639_Display_UVLED_Char(DISPLAY_384);
                    Set_Mode_Cache.usart_baud.setval = 2;
                }
            }
            break;
    }
}
    
void display_uvled_time(u16 time,u8 poi)
{
    if(time == 0)
    {
        Time_stu.hundreds = 99; //>9
        Time_stu.point0 = 0;
        Time_stu.decde = 0;
        if(poi) Time_stu.point1 = 1;
        else Time_stu.point1 = 0;
        Time_stu.unit = time;
        Time_stu.point2 = 0;
    }
    else if(time <= 9)
    {
        Time_stu.hundreds = 99; //>9
        Time_stu.point0 = 0;
        Time_stu.decde = 0;
        if(poi) Time_stu.point1 = 1;
        else Time_stu.point1 = 0;
        Time_stu.unit = time;
        Time_stu.point2 = 0;
    }
    else if(time <= 99)
    {
        Time_stu.hundreds = 99; //>9blank
        Time_stu.point0 = 0;
        Time_stu.decde = time / 10;
        if(poi) Time_stu.point1 = 1;
        else Time_stu.point1 = 0;
        Time_stu.unit = time % 10;
        Time_stu.point2 = 0;
    }
    else if(time <= 999)
    {
        Time_stu.hundreds = time / 100;
        Time_stu.point0 = 0;
        time = time - (Time_stu.hundreds*100);
        Time_stu.decde = time / 10;
        if(poi) Time_stu.point1 = 1;
        else Time_stu.point1 = 0;
        time = time - (Time_stu.decde*10);
        Time_stu.unit = time % 10;
        Time_stu.point2 = 0;
    }
    else if(time <= 9990)
    {
        time = time / 10;
        Time_stu.hundreds = time / 100;
        Time_stu.point0 = 0;
        time = time - (Time_stu.hundreds*100);
        Time_stu.decde = time / 10;
        Time_stu.point1 = 0;
        time = time - (Time_stu.decde*10);
        Time_stu.unit = time;
        Time_stu.point2 = 0;
    }
    TM1639_DisplayUVtime(Time_stu,0);
}

void display_cumultime_hour(u16 hour, u8 mchar)
{
    if(hour > 999) hour = 999;
    
    if(hour <= 9)
    {
        Time_stu.hundreds = 0; 
        Time_stu.point0 = 0;
        Time_stu.decde = 0;
        Time_stu.point1 = 0;
        Time_stu.unit = hour;
        Time_stu.point2 = 0;
    }
    else if(hour <= 99)
    {
        Time_stu.hundreds = 0; //>9blank
        Time_stu.point0 = 0;
        Time_stu.decde = hour / 10;
        Time_stu.point1 = 0;
        Time_stu.unit = hour % 10;
        Time_stu.point2 = 0;
    }
    else if(hour <= 999)
    {
        Time_stu.hundreds = hour / 100;
        Time_stu.point0 = 0;
        hour = hour - (Time_stu.hundreds*100);
        Time_stu.decde = hour / 10;
        Time_stu.point1 = 0;
        Time_stu.unit = hour % 10;
        Time_stu.point2 = 0;
    }
    TM1639_DisplayUVtime(Time_stu, mchar);
}

u16 timelast = 0;
void dispaly_uvled_real_time(u8 runch)
{
    if(runch < 1) return;
    if(time_display_refresh == 0)
    {
        if(runch > CHNUM)
        {
            if( _check_uvled_sta() && timelast!=time_allch && uvonch_last>CHNUM && _check_uvled_current_err()==0)
            {
                timelast = time_allch;
                display_uvled_time(time_allch,1);
            }
        }
        else if(PHY_CH[runch-1].Uvon && _check_uvled_ch_error(runch-1) == 0)
        {
            if(timelast != PHY_CH[runch-1].Ontime)
            {
                timelast = PHY_CH[runch-1].Ontime;
                display_uvled_time(PHY_CH[runch-1].Ontime,1);
            }
        }
    }
}

void PHY_CH_Error_Display(u8 ch)
{
    if(_check_uvled_ch_error(ch) && _check_poweron_allch_con_err() == 0)
    {
        if(rs_mode == RUN_MODE || (rs_mode == SETTING_MODE && Set_Mode.opl_not.setval == 0)) //OPL OFF
        {
            if(ch > CHNUM-1) return;
            time_display_cnt++;
            if(time_display_cnt == 35)
            {
                if(Err_Emg) TM1639_Display_UVLED_Char(DISPLAY_Eo1);
                else        TM1639_Display_UVLED_Char((DISPLAY_ENUM)(ch+16));
            }
            else if(time_display_cnt == 70)
            {
                time_display_cnt = 0;
                if(Err_Emg)                                  TM1639_Display_UVLED_Char(DISPLAY_Eo1);
                else if(PHY_CH[ch].Error_Connect == 1)       TM1639_Display_UVLED_Char(DISPLAY_BLANK);
                else if(PHY_CH[ch].Error_Connect == ERR_Ld3) TM1639_Display_UVLED_Char(DISPLAY_Ld3);
                else if(PHY_CH[ch].Error_Connect == ERR_Ld4) TM1639_Display_UVLED_Char(DISPLAY_Ld4);
                else if(PHY_CH[ch].Error_Curr == ERR_Ld1)    TM1639_Display_UVLED_Char(DISPLAY_Ld1);
                else if(PHY_CH[ch].Error_Curr == ERR_Ld2)    TM1639_Display_UVLED_Char(DISPLAY_Ld2);
            }
        }
    }
    else time_display_cnt = 0;
}

void PHY_Ext_Uvled_Handle(void)
{
    if(Err_Emg) return;
    if(ExtIn_Flag.Emg)
    {
        _clear_ExtInFlag();
        Err_Emg = ERR_EO1;
        PHY_CH[0].ErrCode = ERR_EO1;
        PHY_CH[1].ErrCode = ERR_EO1;
        PHY_CH[2].ErrCode = ERR_EO1;
        PHY_CH[3].ErrCode = ERR_EO1;
        PHY_UvLed_Off();
        if(rs_mode == RUN_MODE) TM1639_Display_UVLED_Char(DISPLAY_Eo1);
        flag_tx_error = 1;
    }
    if(rs_mode != RUN_MODE) return;
    if(_check_poweron_allch_con_err()) return;
    
    if(Set_Mode.ext_input.setval) //pulse
    {
        if(ExtIn_Flag.TrgAll)
        {
            _clear_ExtInFlag();
            if(flag_single_chon != 1)
            {
                run_ch = 5;
                PHY_UVLed_Select(run_ch,0);
                uvled_ready_check();
                PHY_Ext_UvLed_switch(run_ch);
            }
        }
        else 
        {
            if(ExtIn_Flag.Trg1 && flag_extall_on == 0)
            {
                _clear_ExtInFlag();
                if(_check_uvled_ch_error(0) == 0)
                {
                    run_ch = 1;
                    PHY_UVLed_Select(run_ch,0);
                    uvled_ready_check();
                    PHY_Ext_UvLed_switch(run_ch);
                }
            }
            
            if(ExtIn_Flag.Trg2 && flag_extall_on == 0)
            {
                _clear_ExtInFlag();
                if(_check_uvled_ch_error(1) == 0)
                {
                    run_ch = 2;
                    PHY_UVLed_Select(run_ch,0);
                    uvled_ready_check();
                    PHY_Ext_UvLed_switch(run_ch);
                }
            }
            
            if(ExtIn_Flag.Trg3 && flag_extall_on == 0)
            {
                _clear_ExtInFlag();
                if(_check_uvled_ch_error(2) == 0)
                {
                    run_ch = 3;
                    PHY_UVLed_Select(run_ch,0);
                    uvled_ready_check();
                    PHY_Ext_UvLed_switch(run_ch);
                }
            }
            
            if(ExtIn_Flag.Trg4 && flag_extall_on == 0)
            {
                _clear_ExtInFlag();
                if(_check_uvled_ch_error(3) == 0)
                {
                    run_ch = 4;
                    PHY_UVLed_Select(run_ch,0);
                    uvled_ready_check();
                    PHY_Ext_UvLed_switch(run_ch);
                }
            }
        }
    }
    else //level
    {
        if(ExtIn_Flag.TrgAll && flag_extlevel_on == 0 && flag_extall_on == 0)
        {
            if(flag_single_chon != 1)
            {
                run_ch = 5;
                PHY_UVLed_Select(run_ch,0);
                uvled_ready_check();
                PHY_Ext_UvLed_switch(run_ch);
            }
        }
        else if(ExtIn_Flag.TrgAll == 0 && flag_extlevel_on == 1 && flag_extall_on == 1)
        {
            PHY_Ext_UvLed_Off();
        }
        else 
        {
            if(ExtIn_Flag.Trg1 && PHY_CH[0].Uvon == 0 && PHY_CH[0].flag_ext_type == 0)
            {
                if(_check_uvled_ch_error(0) == 0)
                {
                    run_ch = 1;
                    PHY_UVLed_Select(run_ch,0);
                    uvled_ready_check();
                    PHY_Ext_UvLed_switch(run_ch);
                    PHY_CH[0].flag_ext_type = 1;
                }
            }
            else if(ExtIn_Flag.Trg1 == 0 && PHY_CH[0].Uvon == 1 && PHY_CH[0].flag_ext_type == 1)
            {
                run_ch = 1;
                if(PHY_CH[0].SetTime == 0) 
                {
                    PHY_Ext_UvLed_switch(run_ch);
                }
                else
                {
                    if(Set_Mode.trg_pri.setval) //signal priority
                    {
                        PHY_Ext_UvLed_switch(run_ch);
                    }
                    else{}//timer priority
                }
            }
            
            if(ExtIn_Flag.Trg2 && PHY_CH[1].Uvon == 0 && PHY_CH[1].flag_ext_type == 0)
            {
                if(_check_uvled_ch_error(1) == 0)
                {
                    run_ch = 2;
                    PHY_UVLed_Select(run_ch,0);
                    uvled_ready_check();
                    PHY_Ext_UvLed_switch(run_ch);
                    PHY_CH[1].flag_ext_type = 1;
                }
            }
            else if(ExtIn_Flag.Trg2 == 0 && PHY_CH[1].Uvon == 1 && PHY_CH[1].flag_ext_type == 1)
            {
                run_ch = 2;
                if(PHY_CH[1].SetTime == 0) PHY_Ext_UvLed_switch(run_ch);
                else
                {
                    if(Set_Mode.trg_pri.setval) //signal priority
                    {
                        PHY_Ext_UvLed_switch(run_ch);
                    }
                }
            }
            
            if(ExtIn_Flag.Trg3 && PHY_CH[2].Uvon == 0 && PHY_CH[2].flag_ext_type == 0)
            {
                if(_check_uvled_ch_error(2) == 0)
                {
                    run_ch = 3;
                    PHY_UVLed_Select(run_ch,0);
                    uvled_ready_check();
                    PHY_Ext_UvLed_switch(run_ch);
                    PHY_CH[2].flag_ext_type = 1;
                }
            }
            else if(ExtIn_Flag.Trg3 == 0 && PHY_CH[2].Uvon == 1 && PHY_CH[2].flag_ext_type == 1)
            {
                run_ch = 3;
                if(PHY_CH[2].SetTime == 0) PHY_Ext_UvLed_switch(run_ch);
                else
                {
                    if(Set_Mode.trg_pri.setval) //signal priority
                    {
                        PHY_Ext_UvLed_switch(run_ch);
                    }
                }
            }
            
            if(ExtIn_Flag.Trg4 && PHY_CH[3].Uvon == 0 && PHY_CH[3].flag_ext_type == 0)
            {
                if(_check_uvled_ch_error(3) == 0)
                {
                    run_ch = 4;
                    PHY_UVLed_Select(run_ch,0);
                    uvled_ready_check();
                    PHY_Ext_UvLed_switch(run_ch);
                    PHY_CH[3].flag_ext_type = 1;
                }
            }
            else if(ExtIn_Flag.Trg4 == 0 && PHY_CH[3].Uvon == 1 && PHY_CH[3].flag_ext_type == 1)
            {
                run_ch = 4;
                if(PHY_CH[3].SetTime == 0) PHY_Ext_UvLed_switch(run_ch);
                else
                {
                    if(Set_Mode.trg_pri.setval) //signal priority
                    {
                        PHY_Ext_UvLed_switch(run_ch);
                    }
                }
            }
        }
    }
}

void EXT_Input_Scan(void)
{
    Ext_Emg_Scan(); /* emergency stop input. */
    if(rs_mode != RUN_MODE) return;
    Ext_TrgAll_Scan(); /* All channels irradiation input. */
    Ext_Trg1_Scan();/* CH1 Individual irradiation input. */
    Ext_Trg2_Scan();/* CH2 Individual irradiation input. */
    Ext_Trg3_Scan();/* CH3 Individual irradiation input. */
    Ext_Trg4_Scan();/* CH4 Individual irradiation input. */
}
   
u8 trgnone_cnt = 0;
u8 emgnone_cnt = 0;
u8 trg1none_cnt = 0;
u8 trg2none_cnt = 0;
u8 trg3none_cnt = 0;
u8 trg4none_cnt = 0;
void Ext_TrgAll_Scan(void)
{
    if(EXT_TRG_IN == 0 && ExtIn_Now.TrgAll == 0)
    {
        trgnone_cnt = 0;
        trgcnt++;
        if(trgcnt >= SCANMAX)
        {
            trgcnt = 0;
            ExtIn_Now.TrgAll = 1;
            ExtIn_Flag.TrgAll = 1;
        }
    }else 
    {
        trgcnt = 0;
        if(EXT_TRG_IN == 1 && ExtIn_Now.TrgAll == 1) 
        {
            trgnone_cnt++;
            if(trgnone_cnt >= SCANMAX)
            {
                trgnone_cnt = 0;
                ExtIn_Now.TrgAll = 0;
                ExtIn_Flag.TrgAll = 0;
            }
        }
        else trgnone_cnt = 0;
    }
}

void Ext_Emg_Scan(void)
{
    if(EXT_EMG_IN == 0 && ExtIn_Now.Emg == 0)
    {
        emgnone_cnt = 0;
        emgcnt++;
        if(emgcnt >= SCANMAX)
        {
            emgcnt = 0;
            ExtIn_Now.Emg = 1;
            ExtIn_Flag.Emg = 1;
        }
    }else 
    {
        emgcnt = 0;
        if(EXT_EMG_IN == 1 && ExtIn_Now.Emg == 1) 
        {
            emgnone_cnt++;
            if(emgnone_cnt >= SCANMAX)
            {
                emgcnt = 0;
                ExtIn_Now.Emg = 0;
                ExtIn_Flag.Emg = 0;
            }
        }
        else emgnone_cnt = 0;
    }
}

void Ext_Trg1_Scan(void)
{
    if(EXT_TRG1_IN == 0 && ExtIn_Now.Trg1 == 0)
    {
        trg1none_cnt = 0;
        trg1cnt++;
        if(trg1cnt >= SCANMAX)
        {
            trg1cnt = 0;
            ExtIn_Now.Trg1 = 1;
            ExtIn_Flag.Trg1 = 1;
        }
    }else 
    {
        trg1cnt = 0;
        if(EXT_TRG1_IN == 1 && ExtIn_Now.Trg1 == 1) 
        {
            trg1none_cnt++;
            if(trg1none_cnt >= SCANMAX)
            {
                trg1none_cnt = 0;
                ExtIn_Now.Trg1 = 0;
                ExtIn_Flag.Trg1 = 0;
            }
        }
        else trg1none_cnt = 0;
    }
}

void Ext_Trg2_Scan(void)
{
    if(EXT_TRG2_IN == 0 && ExtIn_Now.Trg2 == 0)
    {
        trg2none_cnt = 0;
        trg2cnt++;
        if(trg2cnt >= SCANMAX)
        {
            trg2cnt = 0;
            ExtIn_Now.Trg2 = 1;
            ExtIn_Flag.Trg2 = 1;
        }
    }else 
    {
        trg2cnt = 0;
        if(EXT_TRG2_IN == 1 && ExtIn_Now.Trg2 == 1) 
        {
            trg2none_cnt++;
            if(trg2none_cnt >= SCANMAX)
            {
                trg2none_cnt = 0;
                ExtIn_Now.Trg2 = 0;
                ExtIn_Flag.Trg2 = 0;
            }
        }
        else trg2none_cnt = 0;
    }
}

void Ext_Trg3_Scan(void)
{
    if(EXT_TRG3_IN == 0 && ExtIn_Now.Trg3 == 0)
    {
        trg3none_cnt = 0;
        trg3cnt++;
        if(trg3cnt >= SCANMAX)
        {
            trg3cnt = 0;
            ExtIn_Now.Trg3 = 1;
            ExtIn_Flag.Trg3 = 1;
        }
    }else 
    {
        trg3cnt = 0;
        if(EXT_TRG3_IN == 1 && ExtIn_Now.Trg3 == 1) 
        {
            trg3none_cnt++;
            if(trg3none_cnt >= SCANMAX)
            {
                trg3none_cnt = 0;
                ExtIn_Now.Trg3 = 0;
                ExtIn_Flag.Trg3 = 0;
            }
        }
        else trg3none_cnt = 0;
    }
}

void Ext_Trg4_Scan(void)
{
    if(EXT_TRG4_IN == 0 && ExtIn_Now.Trg4 == 0)
    {
        trg4none_cnt = 0;
        trg4cnt++;
        if(trg4cnt >= SCANMAX)
        {
            trg4cnt = 0;
            ExtIn_Now.Trg4 = 1;
            ExtIn_Flag.Trg4 = 1;
        }
    }else 
    {
        trg4cnt = 0;
        if(EXT_TRG4_IN == 1 && ExtIn_Now.Trg4 == 1) 
        {
            trg4none_cnt++;
            if(trg4none_cnt >= SCANMAX)
            {
                trg4none_cnt = 0;
                ExtIn_Now.Trg4 = 0;
                ExtIn_Flag.Trg4 = 0;
            }
        }
        else trg4none_cnt = 0;
    }
}

void _clear_ExtInFlag(void)
{
    ExtIn_Flag.Byte = 0;
    trgnone_cnt = 0;
    emgnone_cnt = 0;
    trg1none_cnt = 0;
    trg2none_cnt = 0;
    trg3none_cnt = 0;
    trg4none_cnt = 0;
}
    
void TRG_Scan(void)
{
    if(EXT_TRG_IN == 0 && ExtIn_Flag.TrgAll == 0)
    {
        trgcnt++;
        if(trgcnt > CNTMAX)
        {
            trgcnt = 0;
            ExtIn_Flag.TrgAll = 1;
            TM1639_DisplayNum(1);
            EXT_UVON_OUT(0);
        }
    }else 
    {
        trgcnt = 0;
        if(EXT_TRG_IN == 1) 
        {
            ExtIn_Flag.TrgAll = 0;
            EXT_UVON_OUT(1);
        }
    }
}

void EMG_Scan(void)
{
    if(EXT_EMG_IN == 0 && ExtIn_Flag.Emg == 0)
    {
        emgcnt++;
        if(emgcnt > CNTMAX)
        {
            emgcnt = 0;
            ExtIn_Flag.Emg = 1;
            TM1639_DisplayNum(2);
            EXT_ERROR_OUT(0);
        }
    }else 
    {
        emgcnt = 0;
        if(EXT_EMG_IN == 1) 
        {
            ExtIn_Flag.Emg = 0;
            EXT_ERROR_OUT(1);
        }
    }
}

void TRG1_Scan(void)
{
    if(EXT_TRG1_IN == 0 && ExtIn_Flag.Trg1 == 0)
    {
        trg1cnt++;
        if(trg1cnt > CNTMAX)
        {
            trg1cnt = 0;
            ExtIn_Flag.Trg1 = 1;
            TM1639_DisplayNum(3);
            EXT_RYD1_OUT(0);
        }
    }else 
    {
        trg1cnt = 0;
        if(EXT_TRG1_IN == 1) 
        {
            ExtIn_Flag.Trg1 = 0;
            EXT_RYD1_OUT(1);
        }
    }
}

void TRG2_Scan(void)
{
    if(EXT_TRG2_IN == 0 && ExtIn_Flag.Trg2 == 0)
    {
        trg2cnt++;
        if(trg2cnt > CNTMAX)
        {
            trg2cnt = 0;
            ExtIn_Flag.Trg2 = 1;
            TM1639_DisplayNum(4);
            EXT_RYD2_OUT(0);
        }
    }else 
    {
        trg2cnt = 0;
        if(EXT_TRG2_IN == 1) 
        {
            ExtIn_Flag.Trg2 = 0;
            EXT_RYD2_OUT(1);
        }
    }
}

void TRG3_Scan(void)
{
    if(EXT_TRG3_IN == 0 && ExtIn_Flag.Trg3 == 0)
    {
        trg3cnt++;
        if(trg3cnt > CNTMAX)
        {
            trg3cnt = 0;
            ExtIn_Flag.Trg3 = 1;
            TM1639_DisplayNum(5);
            EXT_RYD3_OUT(0);
        }
    }else 
    {
        trg3cnt = 0;
        if(EXT_TRG3_IN == 1) 
        {
            ExtIn_Flag.Trg3 = 0;
            EXT_RYD3_OUT(1);
        }
    }
}

void TRG4_Scan(void)
{
    if(EXT_TRG4_IN == 0 && ExtIn_Flag.Trg4 == 0)
    {
        trg4cnt++;
        if(trg4cnt > CNTMAX)
        {
            trg4cnt = 0;
            ExtIn_Flag.Trg4 = 1;
            TM1639_DisplayNum(6);
            EXT_RYD4_OUT(0);
        }
    }else 
    {
        trg4cnt = 0;
        if(EXT_TRG4_IN == 1) 
        {
            ExtIn_Flag.Trg4 = 0;
            EXT_RYD4_OUT(1);
        }
    }
}

void Test_EXT_TRG_Scan(void)
{
    TRG_Scan();
    EMG_Scan();
    TRG1_Scan();
    TRG2_Scan();
    TRG3_Scan();
    TRG4_Scan();
}

void PHY_PowerOff_Handle(void)
{
    if(flag_low_voltage && flag_save_levtime == 0)
    {
        if(_check_uvled_sta() != 0)
        {
            PHY_PowerOff_UvLed_Off();
            eeprom_save_parameter(SAVE_CUMUTIME);
        }
        eeprom_save_parameter(SAVE_LEVTIME);
        flag_save_levtime = 1;
        Poweroff_Release();
        while(1) FWDT_Clear();
    }
} 
