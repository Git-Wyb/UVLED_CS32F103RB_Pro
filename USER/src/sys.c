#include "sys.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_misc.h"

u16 time_ms = 0;
u16 time_10ms = 0;
u16 time_keyscan = 0;
BaseFlagStu Flag0 = {0};
KEYSTU KeySta = {0};
KEYSTU KeyStaNow = {0};
KEYSTU KeyStaFlag = {0};
u8 keysta_last = 0;
NUMSTU UV_Time[4] = {0};
CH_STU PHY_CH[CHNUM] = {0};
u16 time_buzzer_on = 0;
u16 time_buzzer_off = 0;
u8 buzzer_num = 0;
u16 Adc_Value_Buff[5][7] = {0};
u16 time_adc_conv = 50;
u16 uvled_time = 0;
CH_UVON Timer_Uvon[4] = {0};
BaseFlagStu UVCh_Check = {0};
Proj_Stu Set_Mode = {0};
u8 time_adc_wait = 0;

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
}

