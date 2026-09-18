#ifndef __SYS_H__
#define __SYS_H__

#include <stdio.h>
#include <string.h>
#include "cs32f10x.h"
#include "cs32f10x_conf.h"

typedef signed char s8;
typedef unsigned char u8;
typedef signed short int s16;
typedef unsigned short int u16;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;

#define CHNUM 4
#define TIME_COUNT_MAX 9990 //9990*100=999s

#define CHECK0      0x00
#define CHECK1      0x01
#define CHECK2      0x02
#define CHECK_NONE  0x03

typedef enum
{
    RUN_MODE = 0,
    SETTING_MODE,
    MENU_MODE,
    HOUR_MODE,
    MENU_MODE_SET
}MODE;

typedef struct{
    u8 setval;
}Setting_Stu;

typedef struct{
    Setting_Stu ext_input;
    Setting_Stu trg_pri;
    Setting_Stu buzzer_sw;
    Setting_Stu opl_not;
    Setting_Stu more_led;
    Setting_Stu usart_baud;
}Proj_Stu;

typedef union{
    u8 Flag;
    struct
    {
        u8 b0 : 1;
        u8 b1 : 1;
        u8 b2 : 1;
        u8 b3 : 1;
        u8 b4 : 1;
        u8 b5 : 1;
        u8 b6 : 1;
        u8 b7 : 1;
    };
}BaseFlagStu;

typedef union{
    u8 Byte;
    struct
    {
        u8 Up : 1;
        u8 Down : 1;
        u8 Bk : 1;
        u8 Fw : 1;
        u8 Set : 1;
        u8 Mode : 1;
        u8 Ch : 1;
        u8 Uvon : 1;
    };
}KEYSTU;

typedef struct{
    u8 hundreds;
    u8 point0;
    u8 decde;
    u8 point1;
    u8 unit;
    u8 point2;
}NUMSTU;

typedef struct{
    NUMSTU Cumulative_Time;
    u8 Uvch;
    u16 Time; //max=9990//9990*100ms=999s
    u8 Level;
    u8 Uvon;
    u8 Error_Connect;
    u8 Option;
    u8 Ready;
    u16 Current;
    u8 Error_Curr;
}CH_STU;

typedef enum{
    NO_ERROR = 0,
    Err_Eo1 = 12,
    Err_FE2 = 32,
    Err_Ld1 = 41,
    Err_Ld2 = 42,
    Err_Ld3 = 43,
    Err_Ld4 = 44
}ERROR_CODE;

typedef struct{
    u16 uvontimer;
    u8  uvoff_flag;
}CH_UVON;

#define flag_buzzer_sw   Flag0.b0
#define flag_rx_done     Flag0.b1
#define flag_adc_ok      Flag0.b2
#define flag_rx_head     Flag0.b3
#define flag_error_over  Flag0.b4
#define flag_error_under Flag0.b5
#define flag_adc_en      Flag0.b6
#define flag_keyup_long  Flag0.b7

#define flag_keydown_long  Flag1.b0
#define flag_keybk_long    Flag1.b1
#define flag_keyfw_long    Flag1.b2

extern BaseFlagStu Flag0;
extern BaseFlagStu Flag1;
extern KEYSTU KeySta;
extern u16 time_ms;
extern u16 time_100ms;
extern u16 time_keyscan;
extern u8 keysta_last;
extern KEYSTU KeyStaNow;
extern KEYSTU KeyStaFlag;
extern NUMSTU UV_Time[4];
extern CH_STU PHY_CH[CHNUM];
extern u8 buzzer_num;
extern u16 time_buzzer_on;
extern u16 time_buzzer_off;
extern u16 Adc_Value_Buff[5][7];
extern u16 time_adc_conv;
extern u16 uvled_time;
extern CH_UVON Timer_Uvon[4];
extern BaseFlagStu UVCh_Check;
extern Proj_Stu Set_Mode;
extern u8 time_adc_wait;
extern u8 time_keysta_del;
extern CH_STU SEL_CH;
extern u8 run_ch;
extern NUMSTU Time_stu;

void Init_uvch(void);
void Initial_poweron_state(void);
void Initial_UVLed_CheckIn(void);

#endif
