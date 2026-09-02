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
#include "UVLED.h"


int main(void)
{
    Init_uvch();
    Init_Gpio();
    Init_Timer1(50); //f=50k
    Init_Timer3();
    Init_Adc();
    Bueezr_Switch(ENABLE);
    wait_ms(1000);
    Bueezr_Switch(DISABLE);
    //Init_Usart1(115200);
    Init_TM1639();
    TM1639_Display_UVLED_Char(DISPLAY_888);
    
    while(1)
    {
        //wait_ms(20);
        if(time_keyscan == 0)
        {
            time_keyscan = 20;
            TM1639_Read_Key();
            Key_Handle();
        }
        //PHY_UvLed_Refresh();
    }
}

