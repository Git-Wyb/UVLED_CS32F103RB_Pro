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

#define CHECK_H_1VH  0x01
#define CHECK_H_2VH  0x02
#define CHECK_H_4VH  0x00
#define CHECK_NONE   0x03

typedef enum
{
    RUN_MODE = 0,
    SETTING_MODE,
    MENU_MODE,
    HOUR_MODE,
    MENU_MODE_SET
}MODE;

typedef struct{
    u16 time_on;
    u16 time_off;
    u8  number;
}BuzStu;

typedef struct{
    u8 setval;
}Setting_Stu;

typedef struct{
    Setting_Stu ext_input;   //External input method; 0:LEU Level; 1:Pul Pulse
    Setting_Stu trg_pri;     //TRG Priority; 0:EIN Time priority; 1:SiG Signal priority
    Setting_Stu buzzer_sw;   //Buzzer; 0:OFF; 1:ON
    Setting_Stu opl_not;     //Attendant exclusion; 0:OFF; 1:ON
    Setting_Stu more_led;    //Multi-chip LED; 0:OFF; 1:ON
    Setting_Stu usart_baud;  //RS-232;baud 0:9600; 1:19200; 2:38400
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

typedef union{
    u8 Byte;
    struct
    {
        u8 Trg : 1;
        u8 Emg : 1;
        u8 Trg1 : 1;
        u8 Trg2 : 1;
        u8 Trg3 : 1;
        u8 Trg4 : 1;
        u8 b6 : 1;
        u8 b7 : 1;
    };
}EXTINSTU;

typedef struct{
    u8 hundreds;
    u8 point0;
    u8 decde;
    u8 point1;
    u8 unit;
    u8 point2;
}NUMSTU;

typedef struct{
    u16 timer;
    u8  flag_off;
}CH_UVON;

typedef struct{
    NUMSTU Cumulative_Time;
    CH_UVON Uvledon;
    u8 Uvch;
    u16 SetTime; //max=9990//9990*100ms=999s
    u32 CumulTime;
    u8 Level;
    u8 Uvon;
    u8 Option;
    u8 Ready;
    u16 Current;
    u8 flag_error_curr;
    u8 ErrCode;
    u8 Error_Curr;
    u8 Error_Connect;
    u8 undercurr_cnt;
    u8 overcurr_cnt;
    u8 flag_ext_type;
    u8 flag_ext_uvon;
}CH_STU;

typedef enum{
    NONE_ERR = 0,
    ERR_UNUNIT = 1, //Ununited
    ERR_Eo1 = 12,   //Emergency
    ERR_FE2 = 32,   //EEPROM Error
    ERR_Ld1 = 41,   //UV-LED over current
    ERR_Ld2 = 42,   //UV-LED under current
    ERR_Ld3 = 43,   //UV-LED faulty union
    ERR_Ld4 = 44,   //UV-LED cannot be identified
}ERROR_CODE;


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
#define flag_allch_on      Flag1.b3
#define flag_test_mode     Flag1.b4
#define flag_test_232      Flag1.b5
#define flag_extall_on     Flag1.b6
#define flag_err_emg       Flag1.b7

#define flag_extlevel_on   Flag2.b0

extern BaseFlagStu Flag0;
extern BaseFlagStu Flag1;
extern BaseFlagStu Flag2;
extern KEYSTU KeySta;
extern u16 time_ms;
extern u8 time_10ms;
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
extern BaseFlagStu UVCh_Check;
extern MODE rs_mode;
extern Proj_Stu Set_Mode;
extern Proj_Stu Set_Mode_Cache;
extern Proj_Stu Initial_ModeSet;
extern u16 time_adc_wait;
extern u8 time_keysta_del;
extern CH_STU SEL_CH;
extern u8 run_ch;
extern NUMSTU Time_stu;
extern CH_UVON Time_uvch[CHNUM];
extern u16 time_allch;
extern u8 uvonch_last;
extern BuzStu Buzzer_Set;
extern u16 time_display_cnt;
extern EXTINSTU ExtIn_Now;
extern EXTINSTU ExtIn_Flag;
extern u8 Err_Emg;

void Init_FWDT(void);
void FWDT_Clear(void);
void Init_uvch(void);
void Initial_poweron_state(void);
void Initial_UVLed_CheckIn(void);

#endif
