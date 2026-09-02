#ifndef __UVLED_H__
#define __UVLED_H__

#include "sys.h"
#include "TM1639.h"
#include "timer.h"

void PHY_UvLed_Refresh(void);
void PHY_UVLed_Select(u8 ch);

#endif
