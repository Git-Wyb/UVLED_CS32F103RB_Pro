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

void Send_Logo(void);

u8 pcnt = 0;
void test_chuvled_out(void);
int main(void)
{
    Init_Gpio();
    Init_Timer1(); //f=50k
    Init_Timer3();
    //Init_Adc();
    Set_Mode.buzzer_sw.setval = 1;
    Bueezr_Config(200,0,0);
    //Init_Usart1(9600);
    Init_TM1639();
    Initial_poweron_state();
    //while(1);
    //test_chuvled_out();
    while(1)
    {
        if(time_keyscan == 0)
        {
            time_keyscan = 20;
            TM1639_Read_Key();
            Key_Handle();
            PHY_UVLed_Connect_Check();
        }
        //uvled_current_detection();
        PHY_UvLed_WaitOff();
        uvled_current_error_handle();
        Send_Logo();
    }
}


void Send_Logo(void)
{
    if(flag_rx_done == 1)
    {
        flag_rx_done = 0;
        printf("\r\n2026.09.23,Soft Version V0.03\r\n");
        //printf("ADC Cover_Value    = %d(mV)\r\n",CalVal.Cover_Value);
    }
}

u8 test_on = 0;
void test_chuvled_out(void)
{
    TM1639_Display_UVLED_Char(DISPLAY_ALL);
    while(1)
    {
        if(time_keyscan == 0)
        {
            time_keyscan = 20;
            TM1639_Read_Key();
            Key_Uvon_Scan();
            Key_Mode_Scan();
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
        if(KeyStaFlag.Mode)
        {
            _clear_KeyStaFlag();
            run_ch = 1;
            PHY_UVLed_Select(run_ch);
            return;
        }
    }
}
