#include "usart.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_misc.h"


void Init_Usart1(u32 baud)
{
    usart_config_t ptr_usart;
    nvic_init_t nvic_config_struct;
    
    /* Enable clocks. */
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOB);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_USART1);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_AFIO);
    
    /* Configure PB6(TX) and PB7(RX). */
    gpio_mode_config(GPIOB, GPIO_PIN_6, GPIO_MODE_OUT_AFPP(GPIO_SPEED_HIGH));
    gpio_mode_config(GPIOB, GPIO_PIN_7, GPIO_MODE_IN_PU);
    
    /* Configure the basic information of USART1. */
    usart_def_init(USART1);
    ptr_usart.baud_rate = baud;
    ptr_usart.data_width = USART_DATA_WIDTH_8;
    ptr_usart.flow_control = USART_FLOW_CONTROL_NONE;
    ptr_usart.parity_check = USART_PARITY_NONE;
    ptr_usart.stop_bits = USART_STOP_BIT_1;
    ptr_usart.transceiver_mode = USART_MODE_TX_RX;
    usart_init(USART1, &ptr_usart);
    
     /* Enable the USART Interrupt */
    nvic_config_struct.nvic_irqchannel = IRQn_USART1;
    nvic_config_struct.nvic_irq_pre_priority = 4;
    nvic_config_struct.nvic_irq_sub_priority = 0;
    nvic_config_struct.nvic_irq_enable = ENABLE;
    nvic_init(&nvic_config_struct);
    __USART_INTR_ENABLE(USART1, RXNE); // Enable the USART Receive interrupt
    
    /* Enable USART1. */
    __USART_ENABLE(USART1);
}

u8 rxdata = 0;
void USART1_IRQHandler(void)
{
    /* USART Receiver*/
    if (__USART_FLAG_STATUS_GET(USART1, RXNE) == SET)
    {
        rxdata = (u8)__USART_DATA_RECV(USART1);
    }        
}

