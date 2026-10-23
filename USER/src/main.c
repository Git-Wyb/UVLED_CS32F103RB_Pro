#include "sys.h"
#include "gpio.h"
#include "timer.h"
#include "usart.h"
#include "adc.h"
#include "iic.h"
#include "eeprom.h"
#include "Application.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_misc.h"
#include "TM1639.h"
#include "UVLED.h"
#include "string.h"
void Test_Mode_Handle(void);
void Send_Logo(void);
void TestMode_Key_Handle(void);
void TestMode_current_error_handle(void);
void test_run_mode(void);
void test_chuvled_out(void);
u8 pcnt = 0;

int main(void)
{
    Init_FWDT();
    Init_Gpio();
    Init_Timer1(); //f=50k
    Init_Timer3();
    Init_Adc();
    Init_TM1639();
    SoftI2C_Init();
    Test_Mode_Handle();
    Initial_poweron_state();
    Init_Usart1(19200);
    //while(1);

    while(1)
    {
        FWDT_Clear();
        if(time_keyscan == 0)
        {
            time_keyscan = 20;
            TM1639_Read_Key();
            Key_Handle();
            PHY_UVLed_Connect_Check();
        }
        PHY_UvLed_WaitOff();
        uvled_current_error_handle();
        dispaly_uvled_real_time(run_ch);
        PHY_PowerOff_Handle();
        Send_Logo();
    }
}

void Send_Logo(void)
{
    if(flag_rx_done == 1)
    {
        flag_rx_done = 0;
        printf("\r\nHOYA UV-LED Soft Version V0.07 2026.10.23\r\n");
    }
}

void display_test_mode(u8 set);
void test_setp_set(void);
void test_232(void);
void test_ext_trg(void);
void test_eeprom(void);
u8 test_step = 0;
u8 testcnt = 0;
u8 test_on = 0;
u8 test_232_txdata[] = "(Test 232)";
u8 test_eeprom_data[10] = {1,2,3,4,5,6,7,8,9,10};
u8 read_eeprom_data[10] = {0};

void Test_Mode_Handle(void)
{
    TM1639_Read_Key();
    if(KeySta.Set == 1 && KeySta.Bk == 1) 
    {
        flag_test_mode = 1;
        TM1639_Display_UVLED_Char(DISPLAY_BLANK);
    }
    else flag_test_mode = 0;
    
    while(flag_test_mode)
    {
        FWDT_Clear();
        if(time_keyscan == 0)
        {
            time_keyscan = 20;
            TM1639_Read_Key();
            Key_Up_Scan();
            Key_Down_Scan();
            Key_Set_Scan();
            //Key_Bk_Scan();
            //Key_Fw_Scan();
            //Key_Uvon_Scan();
            //Key_Ch_Scan();
            //Key_Mode_Scan();
            test_setp_set();
        }
        
        switch(test_step)
        {
            case 1:
                test_ext_trg();
            break;
            
            case 2:
                test_232();
                break;
            
            case 3:
                test_eeprom();
                break;
            
            case 4:
                test_chuvled_out();
                break;
            
            case 5:
                test_run_mode();
                break;
            
            default:
                break;
        }
    }
}

void test_ext_trg(void)
{
    TM1639_Display_UVLED_Char(DISPLAY_on);
    while(1)
    {
        FWDT_Clear();
        if(time_keyscan == 0)
        {
            time_keyscan = 20;
            TM1639_Read_Key();
            Key_Set_Scan();
             if(KeyStaFlag.Set)
            {
                _clear_KeyStaFlag();
                display_test_mode(testcnt);
                pcnt = 0;
                test_on = 0;
                test_step = 0;
                flag_test_232 = 0;
                return;
            }
        }
        Test_EXT_TRG_Scan();
    }
}

void test_eeprom(void)
{
    TM1639_Display_UVLED_Char(DISPLAY_on);
    ZD24C08A_PageWrite(0,test_eeprom_data,10);
    wait_ms(100);
    ZD24C08A_Read(0,read_eeprom_data,10);
    wait_ms(300);
    if(memcmp(read_eeprom_data,test_eeprom_data,10) == 0)
    {
        TM1639_Display_UVLED_Char(DISPLAY_oPL);
    }
    else TM1639_Display_UVLED_Char(DISPLAY_ERR);
    while(1)
    {
        FWDT_Clear();
        if(time_keyscan == 0)
        {
            time_keyscan = 20;
            TM1639_Read_Key();
            Key_Set_Scan();
            if(KeyStaFlag.Set)
            {
                _clear_KeyStaFlag();
                display_test_mode(testcnt);
                pcnt = 0;
                test_on = 0;
                test_step = 0;
                flag_test_232 = 0;
                return;
            }
        }
    }
}

void test_setp_set(void)
{
    if(KeyStaFlag.Up)
    {
        _clear_KeyStaFlag();
        testcnt++;
        if(testcnt > 5) testcnt = 1;
        display_test_mode(testcnt);
        
    }
    else if(KeyStaFlag.Down)
    {
        _clear_KeyStaFlag();
        if(testcnt) testcnt--;
        if(testcnt == 0) testcnt = 5;
        display_test_mode(testcnt);
    }
    else if(KeyStaFlag.Set)
    {
        _clear_KeyStaFlag();
        test_step = testcnt;
    }
}

void display_test_mode(u8 set)
{
    switch(set)
    {
        case 1:
            TM1639_Display_UVLED_Char(DISPLAY_Ein);
            break;
        
        case 2:
            TM1639_Display_UVLED_Char(DISPLAY_232);
            break;
        
        case 3:
            TM1639_Display_UVLED_Char(DISPLAY_FE2);
            break;
        
        case 4:
            TM1639_Display_UVLED_Char(DISPLAY_ALL);
            break;
        
        case 5:
            TM1639_Display_UVLED_Char(DISPLAY_RUN);
            break;
    }
}

void test_232(void)
{
    TM1639_Display_UVLED_Char(DISPLAY_on);
    usart1_send_data(test_232_txdata,10);
    time_keyscan = 500;
    while(1)
    {
        FWDT_Clear();
        if(time_keyscan == 0)
        {
            time_keyscan = 20;
            TM1639_Read_Key();
            Key_Set_Scan();
            if(KeyStaFlag.Set)
            {
                _clear_KeyStaFlag();
                display_test_mode(testcnt);
                pcnt = 0;
                test_on = 0;
                test_step = 0;
                flag_test_232 = 0;
                return;
            }    
        }
        
        if(time_keyscan == 0)
        {
            flag_test_232 = 0;
            if(strncmp((const char*)test_232_txdata,(const char*)test232_rxbuf,10) == 0)
            {
                TM1639_Display_UVLED_Char(DISPLAY_oPL);
            }
            else
            {
                TM1639_Display_UVLED_Char(DISPLAY_ERR);
            }
            PHY_CH[0].pCH.CumulTime = 0;
            PHY_CH[1].pCH.CumulTime = 0;
            PHY_CH[2].pCH.CumulTime = 0;
            PHY_CH[3].pCH.CumulTime = 0;
            PHY_CH[0].flag_update_cumutime = 1;
            PHY_CH[1].flag_update_cumutime = 1;
            PHY_CH[2].flag_update_cumutime = 1;
            PHY_CH[3].flag_update_cumutime = 1;
            eeprom_save_parameter(SAVE_CUMUTIME);
        }
    }
}

void test_chuvled_out(void)
{
    TM1639_Display_UVLED_Char(DISPLAY_on);
    while(1)
    {
        FWDT_Clear();
        if(time_keyscan == 0)
        {
            time_keyscan = 20;
            TM1639_Read_Key();
            Key_Set_Scan();
            Key_Uvon_Scan();
            if(KeyStaFlag.Set)
            {
                _clear_KeyStaFlag();
                display_test_mode(testcnt);
                pcnt = 0;
                test_on = 0;
                test_step = 0;
                PHY_Uvled_PwmSwitch(0,0);
                CH_LED_switch(0,0,0);
                PHY_Uvled_PwmSwitch(1,0);
                CH_LED_switch(1,0,0);
                PHY_Uvled_PwmSwitch(2,0);
                CH_LED_switch(2,0,0);
                PHY_Uvled_PwmSwitch(3,0);
                CH_LED_switch(3,0,0);
                return;
            }
        }
        
        if(time_adc_conv == 0 && test_on == 1)
        {
            time_adc_conv = 2000;
            if(pcnt < 100) pcnt++;
            UV_LED_PwmSet(UVLED_CH_ALL,pcnt);
            TM1639_DisplayNum(pcnt);
            if(pcnt == 100) test_on = 2;
        }
        if(KeyStaFlag.Uvon)
        {
            _clear_KeyStaFlag();
            time_adc_conv = 0;
            test_on++;
            if(test_on == 1)
            {
                PHY_Uvled_PwmSwitch(0,1);
                CH_LED_switch(0,0,1);
                PHY_Uvled_PwmSwitch(1,1);
                CH_LED_switch(1,0,1);
                PHY_Uvled_PwmSwitch(2,1);
                CH_LED_switch(2,0,1);
                PHY_Uvled_PwmSwitch(3,1);
                CH_LED_switch(3,0,1);
            }
            else if(test_on >= 2)
            {
                test_on = 0;
                pcnt = 0;
                PHY_Uvled_PwmSwitch(0,0);
                CH_LED_switch(0,0,0);
                PHY_Uvled_PwmSwitch(1,0);
                CH_LED_switch(1,0,0);
                PHY_Uvled_PwmSwitch(2,0);
                CH_LED_switch(2,0,0);
                PHY_Uvled_PwmSwitch(3,0);
                CH_LED_switch(3,0,0);
            }
        }
    }
}

void test_run_mode(void)
{
    TM1639_Display_UVLED_Char(DISPLAY_RUN);
    while(1)
    {
        FWDT_Clear();
        if(time_keyscan == 0)
        {
            time_keyscan = 20;
            TM1639_Read_Key();
            TestMode_Key_Handle();
            PHY_UVLed_Connect_Check();
        }
        PHY_UvLed_WaitOff();
        TestMode_current_error_handle();
    }
}

void TestMode_Key_Handle(void)
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
    dispaly_uvled_real_time(run_ch);
    PHY_CH_Error_Display(run_ch-1);
}    

void TestMode_current_error_handle(void)
{
    u8 ch = 0;
    for(ch = 0; ch < 4; ch++)
    {
        if(PHY_CH[ch].Error_Curr == ERR_Ld1 && PHY_CH[ch].flag_error_curr == 0) //overcurrent
        {
            PHY_CH[ch].flag_error_curr = 1;
            PHY_CH[ch].Uvledon.timer = 0;
            PHY_CH[ch].Uvledon.flag_off = 0;
            TM1639_Display_UVLED_Char(DISPLAY_Ld1); //high
            PHY_Uvled_PwmSwitch(ch,0);
            CH_LED_switch(ch,PHY_CH[ch].Error_Curr,1);
            Bueezr_Config(200,100,3);
            PHY_CH[ch].Error_Curr = 0;
            PHY_CH[ch].flag_error_curr = 0;
        }
        else if(PHY_CH[ch].Error_Curr == ERR_Ld2 && PHY_CH[ch].flag_error_curr == 0) //undercurrent
        {
            PHY_CH[ch].flag_error_curr = 1;
            PHY_CH[ch].Uvledon.timer = 0;
            PHY_CH[ch].Uvledon.flag_off = 0;
            TM1639_Display_UVLED_Char(DISPLAY_Ld2); //low
            PHY_Uvled_PwmSwitch(ch,0);
            CH_LED_switch(ch,PHY_CH[ch].Error_Curr,1);
            Bueezr_Config(200,100,3);
            PHY_CH[ch].Error_Curr = 0;
            PHY_CH[ch].flag_error_curr = 0;
        }
    }
    if(run_ch > CHNUM) //ALL CH
    {
        if(_check_uvled_current_err() && flag_allch_on == 1)
        {
            flag_allch_on = 0;
            uvonch_last = 0;
            PHY_UvLed_Off();
        }
    }
    if(_check_uvled_sta() == 0) _UVLED_CurrCheck_Disable();
}
