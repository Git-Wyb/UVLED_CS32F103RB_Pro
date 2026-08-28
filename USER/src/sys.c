#include "sys.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_misc.h"

u16 time_ms = 0;
u16 time_10ms = 0;
u16 time_keyscan = 0;
KEYSTU KeySta = {0};
KEYSTU KeyStaNow = {0};
u8 keysta_last = 0;
NUMSTU UV_Time[4] = {0};

//system_clock = 72MHz.
void RCC_Configuration(void)
{
    //SystemInit();
}
