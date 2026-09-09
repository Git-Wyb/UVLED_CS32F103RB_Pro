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

#define CHECK0      0x00
#define CHECK1      0x01
#define CHECK2      0x02
#define CHECK_NONE  0x03

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
    u32 Time;
    u8 Level;
    u8 Uvon;
    u8 Error;
    u8 Option;
    u8 Ready;
    u16 Current;
}CH_STU;

typedef struct{
    u16 uvontimer;
    u8  uvoff_flag;
}CH_UVON;

#define flag_buzzer_sw  Flag0.b0
#define flag_rx_done    Flag0.b1
#define flag_adc_ok     Flag0.b2
#define flag_rx_head    Flag0.b3

extern BaseFlagStu Flag0;
extern KEYSTU KeySta;
extern u16 time_ms;
extern u16 time_10ms;
extern u16 time_keyscan;
extern u8 keysta_last;
extern KEYSTU KeyStaNow;
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

void Init_uvch(void);

#endif
