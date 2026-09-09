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

u8 beepcnt = 0;
int main(void)
{
    Init_uvch();
    Init_Gpio();
    Init_Timer1(); //f=50k
    Init_Timer3();
    Init_Adc();
    Bueezr_Config(200,0,0);
    //Init_Usart1(9600);
    Init_TM1639();
    TM1639_Display_UVLED_Char(DISPLAY_888);
    //PHY_CH[0].Option = 1;
    //PHY_CH[0].Level = 1;
    //UV_LED_PwmSet(UVLED_CH_1,9);
    //UV_LED_Switch();
    PHY_CH[0].Option = 1;
    while(1)
    {
        //wait_ms(20);
        if(time_keyscan == 0)
        {
            time_keyscan = 20;
            TM1639_Read_Key();
            Key_Handle();
            PHY_UVLed_CheckIn();
        }
        adc_dma_value();
        if(time_adc_conv == 0)
        {
            time_adc_conv = 50;
            uvled_current_detection();
            if(beepcnt++ > 40)
            {
                beepcnt = 0;
                //Bueezr_Config(200,0,0);
            }
        }
        PHY_UVon_ChLed();
    }
}


void Send_Logo(void)
{
    if(flag_rx_done == 1)
    {
        flag_rx_done = 0;
        printf("\r\n2026.09.02,Soft Version V0.00\r\n");
        //printf("ADC Cover_Value    = %d(mV)\r\n",CalVal.Cover_Value);
    }
}
