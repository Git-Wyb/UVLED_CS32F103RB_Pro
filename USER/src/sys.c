#include "sys.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_misc.h"
#include "TM1639.h"
#include "Application.h"
#include "gpio.h"

u16 time_ms = 0;
u16 time_100ms = 0;
u16 time_keyscan = 0;
BaseFlagStu Flag0 = {0};
BaseFlagStu Flag1 = {0};
KEYSTU KeySta = {0};
KEYSTU KeyStaNow = {0};
KEYSTU KeyStaFlag = {0};
u8 keysta_last = 0;
NUMSTU UV_Time[4] = {0};
CH_STU PHY_CH[CHNUM] = {0};
CH_STU SEL_CH = {0};
u16 time_buzzer_on = 0;
u16 time_buzzer_off = 0;
u8 buzzer_num = 0;
u16 Adc_Value_Buff[5][7] = {0};
u16 time_adc_conv = 50;
u16 uvled_time = 0;
BaseFlagStu UVCh_Check = {0};
Proj_Stu Set_Mode = {0};
u16 time_adc_wait = 0;
u8 time_keysta_del = 0;
u8 run_ch = 1;
NUMSTU Time_stu = {0};
CH_UVON Time_uvch[CHNUM] = {0};
u16 time_allch = 0;
u8 uvonch_last = 0;
BuzStu Buzzer_Set = {0};
u16 time_display_cnt = 0;

NUMSTU display_soft_version = { //Ver0.01
    .hundreds = 0,
    .point0 = 1,
    .decde = 0,
    .point1 = 0,
    .unit = 3,
    .point2 = 0
};

Proj_Stu Initial_ModeSet = {
    .ext_input.setval = 0,  //LEU
    .trg_pri.setval = 0,    //EIN
    .buzzer_sw.setval = 0,  //OFF
    .opl_not.setval = 0,    //OFF
    .more_led.setval = 0,   //OFF
    .usart_baud.setval = 1  //192000
};

//system_clock = 72MHz.
void RCC_Configuration(void)
{
    //SystemInit();
}

void Init_uvch(void)
{
    PHY_CH[0].Uvch = 1;
    PHY_CH[1].Uvch = 2;
    PHY_CH[2].Uvch = 3;
    PHY_CH[3].Uvch = 4;
    //SEL_CH.Uvch = 1;
    //run_ch = 1;
    //PHY_CH[0].Option = 1;
}

void Initial_poweron_state(void)
{
    Set_Mode = Initial_ModeSet;
    Init_uvch();
    TM1639_Display_UVLED_Char(DISPLAY_888);
    PHY_Light_ModeLED(LED_ALL_ON);
    wait_ms(1000);
    PHY_Light_ModeLED(LED_ALL_OFF);
    TM1639_Display_UVLED_Char(DISPLAY_VER);
    wait_ms(1000);
    TM1639_DisplayUVtime(display_soft_version);
    wait_ms(1000);
    Initial_UVLed_CheckIn();

    //PHY_UVLed_Select(run_ch);
}

void Initial_UVLed_CheckIn(void)
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
            PHY_CH[ch].Error_Connect = 1;
        }
        else if(run_ch == 1) run_ch = ch+1;
    }
    if(_check_uvled_connect_err() == 0x0F) 
    {
        TM1639_Display_UVLED_Char(DISPLAY_Ld3);
        PHY_Light_ErrorLED(0,1);
        PHY_Light_ErrorLED(1,1);
        PHY_Light_ErrorLED(2,1);
        PHY_Light_ErrorLED(3,1);
    }
    else PHY_UVLed_Select(run_ch);
}
