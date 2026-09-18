#ifndef __UVLED_H__
#define __UVLED_H__

#include "sys.h"
#include "TM1639.h"
#include "timer.h"

void PHY_UvLed_Refresh(void);
void PHY_UVLed_Select(u8 ch);
void PHY_Set_UVLed_Time(u32 time);
void PHY_UVon_ChLed(void);
void PHY_UVLed_CheckIn(void);
u8 _check_uvled_err(void);
u8 _check_uvled_option(void);
u8 _check_uvled_sta(void);
void _clear_uvled_option(void);
void _uvled_ch_last(u8 ch);
u8 _get_uvled_level(void);
u32 _get_uvled_time(void);
u8 _check_uvled_connect_err(void);
void PHY_UVLed_Connect_Check(void);

#endif
