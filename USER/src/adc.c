#include "adc.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_misc.h"


/*
    CH1 -> PA1_ADC1_IN1
    CH2 -> PA2_ADC1_IN2
    CH3 -> PC3_ADC1_IN13
    CH4 -> PC1_ADC1_IN11
*/
void Init_Adc(void)
{
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOA);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOC);
    gpio_mode_config(GPIOA, GPIO_PIN_1, GPIO_MODE_IN_ANALOG);
    gpio_mode_config(GPIOA, GPIO_PIN_2, GPIO_MODE_IN_ANALOG);
    gpio_mode_config(GPIOC, GPIO_PIN_1, GPIO_MODE_IN_ANALOG);
    gpio_mode_config(GPIOC, GPIO_PIN_3, GPIO_MODE_IN_ANALOG);
    
     /* Specify high and low analog watchdog thresholds. */
    uint16_t v_high = ((2800 * 0xFFF) / 3300);
    uint16_t v_low = ((800 * 0xFFF) / 3300);
    
    /* Configure ADC1 clock. */
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_ADC1);
    rcu_adcclk_config(RCU_ADCCLK_SEL_PCLK2_DIV8);
    
    adc_cfg_t ptr_cfg;
    
    /* Configure the basic information of ADC1. */
    __ADC_DEF_INIT(ADC1);
    adc_struct_init(&ptr_cfg);
    ptr_cfg.ext_trigger = ADC_EXT_TRIGGER_SWSTART;
    adc_init(ADC1, &ptr_cfg);
    
    /* ADC1 regular channel1 configuration. */ 
    adc_regular_channel_config(ADC1, ADC_CHANNEL_1, ADC_SAMPLE_TIME_55_5_CYCLE, 1);
    adc_regular_channel_config(ADC1, ADC_CHANNEL_2, ADC_SAMPLE_TIME_55_5_CYCLE, 1);
    adc_regular_channel_config(ADC1, ADC_CHANNEL_11, ADC_SAMPLE_TIME_55_5_CYCLE, 1);
    adc_regular_channel_config(ADC1, ADC_CHANNEL_13, ADC_SAMPLE_TIME_55_5_CYCLE, 1);
    
    /* Configure high and low analog watchdog thresholds. */
    adc_analog_wdt_threshold_set(ADC1, v_high, v_low);
    
    /* Configure channel1 as the single analog watchdog guarded channel. */
    adc_analog_channel_config(ADC1, ADC_CHANNEL_1);
    adc_analog_channel_config(ADC1, ADC_CHANNEL_2);
    adc_analog_channel_config(ADC1, ADC_CHANNEL_11);
    adc_analog_channel_config(ADC1, ADC_CHANNEL_13);
    
    /* Enable analog watchdog on one regular channel. */
    adc_analog_wdt_config(ADC1, ADC_ANALOG_WDT_ALL_REG);
    
    /* Enable analog watchdog interrupt. */
    __ADC_INTR_ENABLE(ADC1, ADC_IT_ANALOG_WDT);
    
    /* Enable ADC1. */
    __ADC_ENABLE(ADC1);
    
    /* Enable ADC1 reset calibration register. */
    __ADC_RESET_CALI(ADC1);
    /* Check the end of ADC1 reset calibration register. */
    while(__ADC_RESET_CALI_STATUS_GET(ADC1));
    
    /* Start ADC1 calibration. */
    __ADC_CALI_START(ADC1);
    /* Check the end of ADC1 calibration. */
    while(__ADC_CALI_STATUS_GET(ADC1));
    
    adc_nvic_config();
}

void adc_nvic_config(void)
{
    nvic_init_t ptr_nvic;
    
    /* Configure and enable ADC interrupt. */
    ptr_nvic.nvic_irqchannel = IRQn_ADC1_2;
    ptr_nvic.nvic_irq_enable = ENABLE;
    ptr_nvic.nvic_irq_pre_priority = 0;
    ptr_nvic.nvic_irq_sub_priority = 0;
    nvic_init(&ptr_nvic);
}

void ADC1_2_IRQHandler(void)
{
    __ADC_FLAG_CLEAR(ADC1, ADC_FLAG_ANALOG_WDT);
}
