#ifndef __SYS_H__
#define __SYS_H__

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
}CH_STU;

extern KEYSTU KeySta;
extern u16 time_ms;
extern u16 time_10ms;
extern u16 time_keyscan;
extern u8 keysta_last;
extern KEYSTU KeyStaNow;
extern NUMSTU UV_Time[4];
extern CH_STU PHY_CH[CHNUM];
void Init_uvch(void);

#endif
