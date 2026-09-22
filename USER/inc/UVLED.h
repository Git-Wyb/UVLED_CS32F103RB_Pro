#ifndef __UVLED_H__
#define __UVLED_H__

#include "sys.h"
#include "TM1639.h"
#include "timer.h"

void PHY_UvLed_Refresh(void);
void PHY_UVLed_Select(u8 ch);
void PHY_Set_UVLed_Time(u32 time);
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
void PHY_UvLed_switch(u8 ch);
void PHY_UvLed_Off(void);
void PHY_UvLed_WaitOff(void);
void uvled_ready_check(void);
void PHY_Uvled_PwmSwitch(u8 ch, u8 onoff);
u8 _check_uvled_error(void);
u8 _check_uvled_current_err(void);
u8 _check_uvled_ch_error(u8 ch);
u8 _check_uvled_errLd123(void);
u8 _check_uvled_connect_Ld3(void);

#endif
