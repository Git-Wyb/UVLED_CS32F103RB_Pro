#include "gpio.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_misc.h"

void Init_Gpio(void)
{
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOB);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOC);
    //gpio_mode_config(GPIOA, GPIO_PIN_9, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    //__GPIO_PIN_SET(GPIOA,GPIO_PIN_9);
}


