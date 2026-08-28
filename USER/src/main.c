#include "sys.h"
#include "gpio.h"
#include "timer.h"
#include "usart.h"
#include "adc.h"
#include "iic.h"
#include "eeprom.h"
#include "Application.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_misc.h"
#include "TM1639.h"


int main(void)
{
    Init_Gpio();
    Init_Timer1(30); //f=100k
    Init_Timer3();
    Init_Adc();
    Bueezr_Switch(ENABLE);
    wait_ms(1000);
    Bueezr_Switch(DISABLE);
    Init_Usart1(115200);
    Init_TM1639();

    UV_Time[0].hundreds = 8;
    UV_Time[0].point0 = 1;
    UV_Time[0].decde = 8;
    UV_Time[0].point1 = 1;
    UV_Time[0].unit = 8;
    UV_Time[0].point2 = 1;
    TM1639_DisplayUVtime(UV_Time[0]);
    while(1)
    {
        //wait_ms(20);
        if(time_keyscan == 0)
        {
            time_keyscan = 20;
            TM1639_Read_Key();
            Key_Handle();
        }
    }
}

