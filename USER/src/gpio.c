#include "gpio.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_misc.h"

void Init_Gpio(void)
{
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOA);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOB);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOC);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOD);
    
    //INPUT CHECK1234
    gpio_mode_config(GPIOA, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_MODE_IN_PU);
    gpio_mode_config(GPIOB, GPIO_PIN_0, GPIO_MODE_IN_PU);
    gpio_mode_config(GPIOC, GPIO_PIN_4|GPIO_PIN_5, GPIO_MODE_IN_PU);
    
    //OUTPUT ch port 
    gpio_mode_config(GPIOC, GPIO_PIN_0, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    gpio_mode_config(GPIOB, GPIO_PIN_4, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    gpio_mode_config(GPIOD, GPIO_PIN_2, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    gpio_mode_config(GPIOC, GPIO_PIN_2, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    CH1_PROT_OUT(0);
    CH2_PROT_OUT(0);
    CH3_PROT_OUT(0);
    CH4_PROT_OUT(0);
}


