#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "sys.h"
#include "TM1639.h"
#include "timer.h"
#include "UVLED.h"

#define PRO_MODE 1

void Key_Handle(void);
void Key_Up_Scan(void);
void Key_Down_Scan(void);
void Key_Set_Scan(void);
void Key_Bk_Scan(void);
void Key_Fw_Scan(void);
void Key_Uvon_Scan(void);
void Key_Ch_Scan(void);
void Key_Mode_Scan(void);
void _clear_KeyStaFlag(void);
void PHY_UvLed_Mode_Handle(void);
void menuset_handle(u8 type,u8 set,u8 opt);
void PHY_Mode_Switch(MODE mode);
void PHY_Light_ModeLED(LED_ENUM mode);
void _set_ch_temporary_param(u8 ch);
void display_uvled_time(u16 time);

#endif
