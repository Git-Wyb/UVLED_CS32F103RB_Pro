#include "Application.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_misc.h"
#include "UVLED.h"

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
MODE rs_mode = RUN_MODE;
u8 _scnt = 0;
u8 m_mode_cnt = 0;
u8 _bf_cnt = 0;
u8 _sec_menu_cnt = 0;
u8 _mcnt = 0;
u8 _menuset_cnt = 0;
u8 _keyup_cnt = 0;
u8 _keydown_cnt = 0;
u8 _keybk_cnt = 0;
u8 _keyfw_cnt = 0;
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
    uvled_ready_check();
#if (PRO_MODE == 1)
    PHY_UvLed_Mode_Handle();
#endif
    dispaly_uvled_real_time(run_ch);
    PHY_CH_Error_Display(run_ch-1);
}

#if (PRO_MODE == 0)
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

            if(_check_uvled_sta() == 0)
            {
                pwmcnt++;
                //if(pwmcnt > 9) pwmcnt = 1;
                //UV_LED_PwmSet(UVLED_CH_ALL,fbuff[pwmcnt]);
                if(pwmcnt > 100) pwmcnt = 1;
                if(chnum == 0) UV_LED_PwmSet(0,pwmcnt); //UVLED_CH_ALL
                else if(chnum < 5) UV_LED_PwmSet(chnum-1,pwmcnt);
                else UV_LED_PwmSet(UVLED_CH_ALL,pwmcnt);
                TM1639_DisplayNum(pwmcnt);
                //if(pwmcnt > 15) pwmcnt = 1;
                //TM1639_Display_UVLED_Char(pwmcnt);
                TM1639_LED_switch(LED_LEVEL,1);
                TM1639_LED_switch(LED_TIME,0);
                moden = 2;
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
            KeyStaFlag.Down = 1;
 
            if(_check_uvled_sta() == 0)
            {
                if(pwmcnt != 0) pwmcnt--;
                //if(pwmcnt == 0) pwmcnt = 9;
                //UV_LED_PwmSet(UVLED_CH_ALL,fbuff[pwmcnt]);
                if(pwmcnt == 0) pwmcnt = 100;
                if(chnum == 0) UV_LED_PwmSet(0,pwmcnt); //UVLED_CH_ALL
                else if(chnum < 5) UV_LED_PwmSet(chnum-1,pwmcnt);
                else UV_LED_PwmSet(UVLED_CH_ALL,pwmcnt);
                TM1639_DisplayNum(pwmcnt);
                //TM1639_Display_UVLED_Char(pwmcnt);
                TM1639_LED_switch(LED_LEVEL,1);
                TM1639_LED_switch(LED_TIME,0);
                moden = 2;
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
            KeyStaFlag.Set = 1;
 
            if(_check_uvled_sta() == 0)
            {
                pwmcnt = 0;
                UV_LED_PwmSet(UVLED_CH_ALL,pwmcnt);
                TM1639_DisplayNum(pwmcnt);
                uvled_time = 0;
                PHY_Set_UVLed_Time(0);
                TM1639_LED_switch(LED_LEVEL,0);
                TM1639_LED_switch(LED_TIME,0);
                moden = 0;
            }
        }
    }else 
    {
        setcnt = 0;
        if(KeySta.Set == 0)
        {
            KeyStaNow.Set = 0;
            KeyStaFlag.Set = 0;
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
         
            if(_check_uvled_sta() == 0)
            {
                if(uvled_time == 0) uvled_time = 1000;
                if(uvled_time != 0) uvled_time--;
                TM1639_DisplayNum(uvled_time);
                PHY_Set_UVLed_Time(uvled_time*10); //uvled_time*10 * 100ms
                TM1639_LED_switch(LED_LEVEL,0);
                TM1639_LED_switch(LED_TIME,1);
                moden = 3;
                /*if(uvled_time >= 1000)
                {
                    UV_Time[0].hundreds = (uint8_t)(uvled_time / 1000);
                    UV_Time[0].decde = (uint8_t)((uvled_time / 100) % 10);
                    UV_Time[0].unit = (uint8_t)((uvled_time / 10) % 10);
                    UV_Time[0].point1 = 0;
                }
                else if(uvled_time >= 100)
                {
                    UV_Time[0].hundreds = (uint8_t)((uvled_time / 100) % 10);
                    UV_Time[0].decde = (uint8_t)((uvled_time / 10) % 10);
                    UV_Time[0].unit = (uint8_t)(uvled_time % 10);
                    UV_Time[0].point1 = 1; // 小数点在第1位（中间位）右下角
                }
                else
                {
                    UV_Time[0].hundreds = (uint8_t)(uvled_time / 10);
                    UV_Time[0].decde = (uint8_t)(uvled_time % 10);
                    UV_Time[0].unit = 0x00; // 标记第三位熄灭
                    UV_Time[0].point1 = 1;  // 小数点在第0位（最左位）右下角
                }
                
                TM1639_DisplayUVtime(UV_Time[0]);*/
            }
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
            KeyStaFlag.Fw = 1;
         
            if(_check_uvled_sta() == 0)
            {
                uvled_time++; //s
                if(uvled_time > 999) uvled_time = 0;
                TM1639_DisplayNum(uvled_time);
                PHY_Set_UVLed_Time(uvled_time*10); //uvled_time*10 * 100ms
                TM1639_LED_switch(LED_LEVEL,0);
                TM1639_LED_switch(LED_TIME,1);
                moden = 3;
            }
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
            KeyStaFlag.Uvon = 1;
           
            TM1639_SetBrightness(5);
            if(uvon == 0) uvon = 1;
            else uvon = 0;
            time_adc_conv = 50;
            if(_check_uvled_err() == 0) UV_LED_Switch();
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
            KeyStaFlag.Ch = 1;
          
            if(_check_uvled_sta() == 0)
            {
                chnum++;
                if(chnum > 5) chnum = 1;
                TM1639_Display_UVLED_Char(chnum+15);
                PHY_UVLed_Select(chnum);
                TM1639_LED_switch(LED_LEVEL,0);
                TM1639_LED_switch(LED_TIME,0);
                moden = 1;
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
            KeyStaFlag.Mode = 1;
         
            moden++;
            switch(moden)
            {
                case 1:
                    if(chnum == 0) TM1639_Display_UVLED_Char(1+15);
                    else if(chnum < 5) TM1639_Display_UVLED_Char(chnum+15);
                    else TM1639_Display_UVLED_Char(5+15);
                    TM1639_LED_switch(LED_LEVEL,0);
                    TM1639_LED_switch(LED_TIME,0);
                    break;
                
                case 2:
                    if(chnum < 5) TM1639_DisplayNum(PHY_CH[chnum-1].Level);
                    else TM1639_DisplayNum(PHY_CH[0].Level);
                    TM1639_LED_switch(LED_LEVEL,1);
                    TM1639_LED_switch(LED_TIME,0);
                    break;
                
                default:
                    moden = 0;
                    if(chnum < 5) TM1639_DisplayNum(PHY_CH[chnum-1].Time/10);
                    else TM1639_DisplayNum((PHY_CH[0].Time/10));
                    TM1639_LED_switch(LED_LEVEL,0);
                    TM1639_LED_switch(LED_TIME,1);
                    break;
            }
        }
    }else 
    {
        modecnt = 0;
        if(KeySta.Mode == 0) KeyStaNow.Mode = 0;
    }
}
#else
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
#endif
u8 sd;
void PHY_UvLed_Mode_Handle(void)
{
#if TEST_MODE
    rs_mode = SETTING_MODE;
#endif
    if(_check_poweron_allch_con_err()) return;
    
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
                PHY_UVLed_Select(run_ch);
                PHY_Light_ModeLED(LED_MODE_OFF);
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
                    display_uvled_time(_get_uvled_time());
                    PHY_Light_ModeLED(LED_TIME);
                }
            }
            else if(KeyStaFlag.Fw)
            {
                _clear_KeyStaFlag();
                if(_check_uvled_sta())  return;
                if(_check_uvled_option() && run_ch < CHNUM+1)
                {
                    display_uvled_time(_get_uvled_time());
                    PHY_Light_ModeLED(LED_TIME);
                }
            }
            break;
        
        case SETTING_MODE:
#if TEST_MODE
            if(KeyStaFlag.Uvon)
            {
                _clear_KeyStaFlag();
                PHY_UvLed_switch(run_ch);
            }
#endif
            if(KeyStaFlag.Set)
            {
                _scnt++;
                if(_scnt >= 40) //800ms
                {
                    _clear_KeyStaFlag();
                }
                if(KeyStaFlag.Bk)
                {
                    _clear_KeyStaFlag();
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
            else if(KeyStaFlag.Ch)
            {
                _clear_KeyStaFlag();
                run_ch++;
                if(run_ch > CHNUM) run_ch = 1;
                PHY_UVLed_Select(run_ch);
                PHY_Light_ModeLED(LED_MODE_OFF);
                PHY_Light_ModeLED(LED_CH_OFF);
            }
            else if(KeyStaFlag.Up)
            {
                if(_check_uvled_option() && run_ch <= CHNUM)
                {
                    _keyup_cnt++;
                    if(_check_uvled_ch_error(run_ch-1)) return 0;
                    
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
                    if(_check_uvled_ch_error(run_ch-1)) return 0;
                    
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
                    if(_check_uvled_ch_error(run_ch-1)) return 0;
                    
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
                        display_uvled_time(chtime);
                        flag_keybk_long = 1;
                    }
                }
            }
            else if(KeyStaFlag.Fw)
            {
                if(_check_uvled_option() && run_ch <= CHNUM)
                {
                    _keyfw_cnt++;
                    if(_check_uvled_ch_error(run_ch-1)) return 0;
                    
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
                        display_uvled_time(chtime);
                        flag_keyfw_long = 1;
                    }
                }
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
                            Bueezr_Config(200,0,0);
                            Set_Mode = Initial_ModeSet;
                            _clear_KeyStaFlag();
                        }
                    }
                    else if(KeyStaFlag.Bk)
                    {
                        _clear_KeyStaFlag();
                        m_mode_cnt++;
                        if(m_mode_cnt > 6) m_mode_cnt = 1;
                        menuset_handle(0,m_mode_cnt,0);
                    }
                    else if(KeyStaFlag.Fw)
                    {
                        _clear_KeyStaFlag();
                        if(m_mode_cnt != 0) m_mode_cnt--;
                        if(m_mode_cnt == 0) m_mode_cnt = 6;
                        menuset_handle(0,m_mode_cnt,0);
                    }
                    else if(KeyStaFlag.Set)
                    {
                        _menuset_cnt++;
                        if(_menuset_cnt < 2)
                        {
                            menuset_handle(1,m_mode_cnt,_sec_menu_cnt);
                            rs_mode = MENU_MODE_SET;
                        }
                        if(_menuset_cnt >= 37) //3s
                        {
                            _menuset_cnt = 0;
                            _clear_KeyStaFlag();
                            Bueezr_Config(200,0,0);
                            rs_mode = SETTING_MODE;
                            PHY_Mode_Switch(rs_mode);
                        }
                    }
                }
            }else _mcnt = 0;
            break;
        
        case HOUR_MODE:
            
            break;
        
        case MENU_MODE_SET:
            if(KeyStaFlag.Up)
            {
                _clear_KeyStaFlag();
                if(m_mode_cnt == 6 && _sec_menu_cnt < 2) _sec_menu_cnt++;
                else if(_sec_menu_cnt < 1) _sec_menu_cnt++;
                menuset_handle(1,m_mode_cnt,_sec_menu_cnt);
            }
            else if(KeyStaFlag.Down)
            {
                _clear_KeyStaFlag();
                if(_sec_menu_cnt != 0) _sec_menu_cnt--;
                menuset_handle(1,m_mode_cnt,_sec_menu_cnt);
            }
            else if(KeyStaFlag.Set)
            {
                _menuset_cnt++;
                if(_menuset_cnt < 2)
                {
                    menuset_handle(0,m_mode_cnt,0);
                    rs_mode = MENU_MODE;
                }
                if(_menuset_cnt >= 150) //3s
                {
                    _menuset_cnt = 0;
                    _clear_KeyStaFlag();
                    Bueezr_Config(200,0,0);
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
    chlevel = PHY_CH[ch-1].Level;
    chtime = PHY_CH[ch-1].Time;
}

void PHY_Mode_Switch(MODE mode)
{
    switch(mode)
    {
        case RUN_MODE:
            PHY_Light_ModeLED(LED_RUN_MODE);
            //if(PHY_CH[run_ch-1].Error_Connect == 1)         TM1639_Display_UVLED_Char(DISPLAY_BLANK);
            //else if(PHY_CH[run_ch-1].Error_Connect == 2)    TM1639_Display_UVLED_Char(DISPLAY_Ld3);
            //else if(PHY_CH[run_ch-1].Error_Curr == ERR_Ld1) TM1639_Display_UVLED_Char(DISPLAY_Ld1);
            //else if(PHY_CH[run_ch-1].Error_Curr == ERR_Ld2) TM1639_Display_UVLED_Char(DISPLAY_Ld2);
            //else 
            TM1639_Display_UVLED_Char((DISPLAY_ENUM)(run_ch+15));
            break;
        
        case SETTING_MODE:
            PHY_Light_ModeLED(LED_SETTING_MODE);
            //if(PHY_CH[run_ch-1].Error_Connect == 1)         TM1639_Display_UVLED_Char(DISPLAY_BLANK);
            //else if(PHY_CH[run_ch-1].Error_Connect == 2)    TM1639_Display_UVLED_Char(DISPLAY_Ld3);
            //else if(PHY_CH[run_ch-1].Error_Curr == ERR_Ld1) TM1639_Display_UVLED_Char(DISPLAY_Ld1);
            //else if(PHY_CH[run_ch-1].Error_Curr == ERR_Ld2) TM1639_Display_UVLED_Char(DISPLAY_Ld2);
            //else 
            TM1639_Display_UVLED_Char((DISPLAY_ENUM)(run_ch+15));
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
            TM1639_Display_UVLED_Char(DISPLAY_HOUR);
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
            TM1639_LED_switch(LED_LEVEL,0);
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
        case 1:
            if(type == 0) TM1639_Display_UVLED_Char(DISPLAY_Ein);
            else
            {
                if(opt == 0) 
                {
                    TM1639_Display_UVLED_Char(DISPLAY_LEU);
                    Set_Mode.ext_input.setval = 0;
                }
                else
                {
                    TM1639_Display_UVLED_Char(DISPLAY_PuL);
                    Set_Mode.ext_input.setval = 1;
                }
            }
            break;
        case 2:
            if(type == 0) TM1639_Display_UVLED_Char(DISPLAY_Pri);
            else
            {
                if(opt == 0) 
                {
                    TM1639_Display_UVLED_Char(DISPLAY_ein);
                    Set_Mode.trg_pri.setval = 0;
                }
                else
                {
                    TM1639_Display_UVLED_Char(DISPLAY_SiG);
                    Set_Mode.trg_pri.setval = 1;
                }
            }
            break;
        case 3:
            if(type == 0) TM1639_Display_UVLED_Char(DISPLAY_bur);
            else
            {
                if(opt == 0) 
                {
                    TM1639_Display_UVLED_Char(DISPLAY_oFF);
                    Set_Mode.buzzer_sw.setval = 0;
                }
                else
                {
                    TM1639_Display_UVLED_Char(DISPLAY_on);
                    Set_Mode.buzzer_sw.setval = 1;
                }
            }
            break;
        case 4:
            if(type == 0) TM1639_Display_UVLED_Char(DISPLAY_oPL);
            else
            {
                if(opt == 0) 
                {
                    TM1639_Display_UVLED_Char(DISPLAY_oFF);
                    Set_Mode.opl_not.setval = 0;
                }
                else
                {
                    TM1639_Display_UVLED_Char(DISPLAY_on);
                    Set_Mode.opl_not.setval = 1;
                }
            }
            break;
        case 5:
            if(type == 0) TM1639_Display_UVLED_Char(DISPLAY_4Uh);
            else
            {
                if(opt == 0) 
                {
                    TM1639_Display_UVLED_Char(DISPLAY_oFF);
                    Set_Mode.more_led.setval = 0;
                }
                else
                {
                    TM1639_Display_UVLED_Char(DISPLAY_on);
                    Set_Mode.more_led.setval = 1;
                }
            }
            break;
        case 6:
            if(type == 0) TM1639_Display_UVLED_Char(DISPLAY_232);
            else
            {
                if(opt == 0) 
                {
                    TM1639_Display_UVLED_Char(DISPLAY_96);
                    Set_Mode.usart_baud.setval = 0;
                }
                else if(opt == 1)
                {
                    TM1639_Display_UVLED_Char(DISPLAY_192);
                    Set_Mode.usart_baud.setval = 1;
                }
                else
                {
                    TM1639_Display_UVLED_Char(DISPLAY_384);
                    Set_Mode.usart_baud.setval = 2;
                }
            }
            break;
    }
}
    
void display_uvled_time(u16 time)
{
    if(time == 0)
    {
        Time_stu.hundreds = 99; //>9
        Time_stu.point0 = 0;
        Time_stu.decde = 0;
        Time_stu.point1 = 1;
        Time_stu.unit = time;
    }
    else if(time <= 9)
    {
        Time_stu.hundreds = 99; //>9
        Time_stu.point0 = 0;
        Time_stu.decde = 0;
        Time_stu.point1 = 1;
        Time_stu.unit = time;
    }
    else if(time <= 99)
    {
        Time_stu.hundreds = 99; //>9blank
        Time_stu.point0 = 0;
        Time_stu.decde = time / 10;
        Time_stu.point1 = 1;
        Time_stu.unit = time % 10;
    }
    else if(time <= 999)
    {
        Time_stu.hundreds = time / 100;
        Time_stu.point0 = 0;
        time = time - (Time_stu.hundreds*100);
        Time_stu.decde = time / 10;
        Time_stu.point1 = 1;
        time = time - (Time_stu.decde*10);
        Time_stu.unit = time % 10;
    }
    else if(time <= 9999)
    {
        time = time / 10;
        Time_stu.hundreds = time / 100;
        Time_stu.point0 = 0;
        time = time - (Time_stu.hundreds*100);
        Time_stu.decde = time / 10;
        Time_stu.point1 = 0;
        time = time - (Time_stu.decde*10);
        Time_stu.unit = time;
    }
    TM1639_DisplayUVtime(Time_stu);
}

void dispaly_uvled_real_time(u8 ch)
{
    if(ch < 1) return;
    
    if(ch>CHNUM && _check_uvled_sta() && uvonch_last>CHNUM && _check_uvled_current_err()==0) display_uvled_time(time_allch);
    else if(PHY_CH[ch-1].Uvon && _check_uvled_ch_error(ch-1) == 0) display_uvled_time(Time_uvch[ch-1].timer);
}

void PHY_CH_Error_Display(u8 ch)
{
    if(_check_uvled_ch_error(ch) && rs_mode <= SETTING_MODE && _check_poweron_allch_con_err() == 0)
    {
        if(ch > CHNUM-1) return;
        time_display_cnt++;
        if(time_display_cnt == 35)
        {
            TM1639_Display_UVLED_Char((DISPLAY_ENUM)(ch+16));
        }
        else if(time_display_cnt == 70)
        {
            time_display_cnt = 0;
            if(PHY_CH[ch].Error_Connect == 1)         TM1639_Display_UVLED_Char(DISPLAY_BLANK);
            else if(PHY_CH[ch].Error_Connect == 2)    TM1639_Display_UVLED_Char(DISPLAY_Ld3);
            else if(PHY_CH[ch].Error_Curr == ERR_Ld1) TM1639_Display_UVLED_Char(DISPLAY_Ld1);
            else if(PHY_CH[ch].Error_Curr == ERR_Ld2) TM1639_Display_UVLED_Char(DISPLAY_Ld2);
        }
    }
    else time_display_cnt = 0;
}

