#include "sys.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_misc.h"
#include "cs32f10x_fwdt.h"
#include "TM1639.h"
#include "Application.h"
#include "gpio.h"
#include "eeprom.h"

u16 time_ms = 0;
u8 time_10ms = 0;
u16 time_100ms = 0;
u16 time_keyscan = 0;
BaseFlagStu Flag0 = {0};
BaseFlagStu Flag1 = {0};
BaseFlagStu Flag2 = {0};
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
MODE rs_mode = RUN_MODE;
Proj_Stu Set_Mode = {0};
Proj_Stu Set_Mode_Cache = {0};
u16 time_adc_wait = 0;
u8 time_keysta_del = 0;
u8 run_ch = 0;
NUMSTU Time_stu = {0};
CH_UVON Time_uvch[CHNUM] = {0};
u16 time_allch = 0;
u8 uvonch_last = 0;
BuzStu Buzzer_Set = {0};
u16 time_display_cnt = 0;
EXTINSTU ExtIn_Now = {0};
EXTINSTU ExtIn_Flag = {0};
u8 Err_Emg = 0;
u16 time_display_refresh = 0;

NUMSTU display_soft_version = { //Ver0.06
    .hundreds = 0,
    .point0 = 1,
    .decde = 0,
    .point1 = 0,
    .unit = 7,
    .point2 = 0
};

Proj_Stu Initial_ModeSet = {
    .ext_input.setval = 0,  //External input method; 0:LEU Level; 1:Pul Pulse
    .trg_pri.setval = 0,    //TRG Priority; 0:EIN Time priority; 1:SiG Signal priority
    .buzzer_sw.setval = 0,  //Buzzer; 0:OFF; 1:ON
    .opl_not.setval = 0,    //Attendant exclusion; 0:OFF; 1:ON
    .H_4VH.setval = 0,   //Multi-chip LED; 0:OFF; 1:ON
    .usart_baud.setval = 1  //RS-232;baud 0:9600; 1:19200; 2:38400
};

//system_clock = 72MHz.
void RCC_Configuration(void)
{
    //SystemInit();
}

void Init_FWDT(void) //?????????
{
    fwdt_prescaler_set(32);
    fwdt_reload_counter();
    fwdt_enable();
}

void FWDT_Clear(void)
{
    fwdt_reload_counter();
}

void Init_uvch(void)
{
    PHY_CH[0].Uvch = 1;
    PHY_CH[1].Uvch = 2;
    PHY_CH[2].Uvch = 3;
    PHY_CH[3].Uvch = 4;
}

void Initial_poweron_state(void)
{
    u8 ki = 0;
    eeprom_read_parameter();
    for(ki = 0; ki < CHNUM; ki++)
    {
        if(PHY_CH[ki].SetLevel > 100 || PHY_CH[ki].SetTime > TIME_COUNT_MAX 
           || Set_Mode.ext_input.setval > 2 || Set_Mode.usart_baud.setval > 2
           || Set_Mode.trg_pri.setval > 2 || Set_Mode.buzzer_sw.setval > 2
           || Set_Mode.opl_not.setval > 2 || Set_Mode.H_4VH.setval > 2)
        {
            Set_Mode = Initial_ModeSet;
            Set_Mode_Cache = Initial_ModeSet;
            PHY_CH[0].SetLevel = 0;
            PHY_CH[1].SetLevel = 0;
            PHY_CH[2].SetLevel = 0;
            PHY_CH[3].SetLevel = 0;
            PHY_CH[0].SetTime = 0;
            PHY_CH[1].SetTime = 0;
            PHY_CH[2].SetTime = 0;
            PHY_CH[3].SetTime = 0;
            break;
        }
    }
    Bueezr_Config(200,100,3);
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
    PHY_Light_ModeLED(LED_RUN_MODE);
}

void Initial_UVLed_CheckIn(void)
{
    u8 ch = 0,chnum = CHNUM;
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
        if(((UVCh_Check.Flag >> (ch*2)) & CHECK_NONE) == CHECK_NONE)
        {
            PHY_CH[ch].Error_Connect = 1;
        }
        else if(run_ch == 0) run_ch = ch+1;
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
