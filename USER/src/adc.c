#include "adc.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_misc.h"
#include "cs32f10x_dma.h"
#include "TM1639.h"
#include "timer.h"

#define ADC1_OUTDAT_REG_ADDRESS    0x40012440

static void adc1_dma_config(void);
static volatile uint16_t adc_conv[6];
u16 adc_ch1 = 0;
u16 Low_Valtage = 0;
u8 uvled_adc_ch[4] = {ADC_CHANNEL_1,ADC_CHANNEL_2,ADC_CHANNEL_13,ADC_CHANNEL_11};
/*
    LOW valtage PA0 -> ADC1_IN0
    CH1 -> PA1_ADC1_IN1
    CH2 -> PA2_ADC1_IN2
    CH3 -> PC3_ADC1_IN13
    CH4 -> PC1_ADC1_IN11
*/
void Init_Adc(void)
{
    adc_cfg_t adc_cfg;
    adc1_dma_config();
    
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOA);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOC);
    gpio_mode_config(GPIOA, GPIO_PIN_0, GPIO_MODE_IN_ANALOG);
    gpio_mode_config(GPIOA, GPIO_PIN_1, GPIO_MODE_IN_ANALOG);
    gpio_mode_config(GPIOA, GPIO_PIN_2, GPIO_MODE_IN_ANALOG);
    gpio_mode_config(GPIOC, GPIO_PIN_1, GPIO_MODE_IN_ANALOG);
    gpio_mode_config(GPIOC, GPIO_PIN_3, GPIO_MODE_IN_ANALOG);
    
    /* Configure ADC1 clock. */
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_ADC1);
    rcu_adcclk_config(RCU_ADCCLK_SEL_PCLK2_DIV6); //72 /6 = 12MHz

    /* Configure the basic information of ADC1. */
    __ADC_DEF_INIT(ADC1);
    adc_cfg.adc_mode    = ADC_MODE_INDEPENDENT; //ADC_MODE
    adc_cfg.scan_mode   = ADC_SCAN_MODE_EN; //ADC_SCAN_MODE
    adc_cfg.ext_trigger = ADC_EXT_TRIGGER_SWSTART; //ADC_EXT_TRIGGER
    adc_cfg.data_align  = ADC_DATA_ALIGN_RIGHT; //ADC_DATA_ALIGN
    adc_cfg.conv_mode   = ADC_CONV_MODE_CONTINUE; //ADC_CONV_MODE
    adc_cfg.ch_num      = 5; //Number of channels
    adc_init(ADC1, &adc_cfg);
    
    /* ADC1 regular channel1 configuration. */ 
    adc_regular_channel_config(ADC1, ADC_CHANNEL_0, ADC_SAMPLE_TIME_55_5_CYCLE, 1); //Regular Group Conversion 1(adc_conv[0])
    adc_regular_channel_config(ADC1, uvled_adc_ch[0], ADC_SAMPLE_TIME_55_5_CYCLE, 2);//Regular Group Conversion 2(adc_conv[1])
    adc_regular_channel_config(ADC1, uvled_adc_ch[1], ADC_SAMPLE_TIME_55_5_CYCLE, 3);//Regular Group Conversion 3(adc_conv[2])
    adc_regular_channel_config(ADC1, uvled_adc_ch[2], ADC_SAMPLE_TIME_55_5_CYCLE, 4);//Regular Group Conversion 4(adc_conv[3])
    adc_regular_channel_config(ADC1, uvled_adc_ch[3], ADC_SAMPLE_TIME_55_5_CYCLE, 5);//Regular Group Conversion 5(adc_conv[4])

    __ADC_DMA_ENABLE(ADC1);
    __DMA_ENABLE(DMA1_CHANNEL1);
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
    
    __ADC_REG_CONV_START(ADC1);
}

#if 1
static void adc1_dma_config(void)
{
    dma_config_t  dma_configStruct;
    nvic_init_t nvic_config_struct;
    //Enable DMA1 clock  
    __RCU_AHB_CLK_ENABLE(RCU_AHB_PERI_DMA1);
    
    // DMA1 Channel 1 Config
    dma_def_init(DMA1_CHANNEL1);
    dma_configStruct.peri_base_addr = (uint32_t)(&(ADC1->OUTDAT));
    dma_configStruct.mem_base_addr = (uint32_t)adc_conv;
    dma_configStruct.transfer_direct = DMA_TRANS_DIR_FROM_PERI;
    dma_configStruct.buf_size = 5;
    dma_configStruct.peri_inc_flag = DMA_PERI_INC_DISABLE;
    dma_configStruct.mem_inc_flag = DMA_MEM_INC_ENABLE;
    dma_configStruct.peri_data_width = DMA_PERI_DATA_WIDTH_HALFWORD;
    dma_configStruct.mem_data_width = DMA_MEM_DATA_WIDTH_HALFWORD;
    dma_configStruct.operate_mode = DMA_OPERATE_MODE_CIRCULAR;
    dma_configStruct.priority_level = DMA_CHANNEL_PRIORITY_HIGH;
    dma_configStruct.m2m_flag = DMA_M2M_MODE_DISABLE;
    dma_init(DMA1_CHANNEL1, &dma_configStruct);
    
     /* Enable and set DMA1_CHANNEL1 Interrupt */
    nvic_config_struct.nvic_irqchannel = IRQn_DMA1_Channel1;
    nvic_config_struct.nvic_irq_pre_priority = 0;
    nvic_config_struct.nvic_irq_sub_priority = 0;
    nvic_config_struct.nvic_irq_enable = ENABLE;
    nvic_init(&nvic_config_struct);
    __DMA_INTR_ENABLE(DMA1_CHANNEL1,DMA_INTR_CMP);
}
#endif

void _UVLED_CurrCheck_Enable(void)
{
    flag_adc_en = 1;
    Init_Adc();
    time_adc_wait = 50;
}
    
void _UVLED_CurrCheck_Disable(void)
{
    flag_adc_en = 0;
    time_adc_wait = 0;
    __ADC_REG_CONV_STOP(ADC1);
    __ADC_DISABLE(ADC1);
    __ADC_DMA_DISABLE(ADC1);
    __DMA_INTR_DISABLE(DMA1_CHANNEL1,DMA_INTR_CMP);
    __DMA_DISABLE(DMA1_CHANNEL1);
}

u8 ich = 0;
void DMA1_Channel1_IRQHandler(void)
{
    if(__DMA1_FLAG_STATUS_GET(DMA1_FLAG_CMP1) == SET)
    {
        __DMA1_FLAG_CLEAR(DMA1_FLAG_CMP1);
        if(time_adc_wait == 0)
        {
            Low_Valtage = (adc_conv[0] * 3300) / 4095;
            PHY_CH[0].Current = (adc_conv[1] * 3300) / 4095;
            PHY_CH[1].Current = (adc_conv[2] * 3300) / 4095;
            PHY_CH[2].Current = (adc_conv[3] * 3300) / 4095;
            PHY_CH[3].Current = (adc_conv[4] * 3300) / 4095;
            flag_adc_ok = 1;
            for(ich = 0; ich < 4; ich++)
            {
                if(PHY_CH[ich].Uvon == 1)
                {
                    if(PHY_CH[ich].Current < 20) 
                    {
                        PHY_CH[ich].Error_Curr = 1;
                        Bueezr_Config(200,0,0);
                        PHY_CH[ich].Uvon = 0;
                        timer1_channel_gpiomode(ich,1,0);
                    }
                    else if(PHY_CH[ich].Current > 2400)
                    {
                        PHY_CH[ich].Error_Curr = 2;
                        Bueezr_Config(200,0,0);
                        PHY_CH[ich].Uvon = 0;
                        timer1_channel_gpiomode(ich,1,0);
                    }
                    else
                    {
                        
                    }
                }
            }
        }
    }
}


u8 vy = 0;
u8 vx = 0;
void adc_dma_value(void)
{                 // Clear DMA CMP1 flag.
    if(__DMA1_FLAG_STATUS_GET(DMA1_FLAG_CMP1) == SET)
    {
        __DMA1_FLAG_CLEAR(DMA1_FLAG_CMP1);
        
        for(vx = 0; vx < 5; vx++)
        {
            Adc_Value_Buff[vx][vy] = adc_conv[vx];
        }
        vy++;
        if(vy >= 7)
        {
            vy = 0;
            __ADC_REG_CONV_STOP(ADC1);
            flag_adc_ok = 1;
        }
    }
}

void uvled_current_error(void)
{
    u8 ch = 0;
    u16 x = 0;
    for(ch = 0; ch < 4; ch++)
    {
        if(PHY_CH[ch].Error_Curr == 2 && flag_error_over == 0)
        {
            flag_error_over = 1;
            if(ch == 0) x = 666;
            if(ch == 1) x = 777;
            if(ch == 2) x = 888;
            if(ch == 3) x = 999;
            TM1639_DisplayNum(x);
            Timer_Uvon[ch].uvontimer = 0;
            Timer_Uvon[ch].uvoff_flag = 0;
            CH_LED_switch(ch,PHY_CH[ch].Error_Curr,1);
        }
        else if(PHY_CH[ch].Error_Curr == 1 && flag_error_under == 0)
        {
            flag_error_under = 1;
            if(ch == 0) x = 111;
            if(ch == 1) x = 222;
            if(ch == 2) x = 333;
            if(ch == 3) x = 444;
            TM1639_DisplayNum(x);
            Timer_Uvon[ch].uvontimer = 0;
            Timer_Uvon[ch].uvoff_flag = 0;
            CH_LED_switch(ch,PHY_CH[ch].Error_Curr,1);
        }
    }
}

void uvled_current_detection(void)
{
//    u8 i = 0;
    if(flag_adc_ok == 1)
    {
        flag_adc_ok = 0;
        //Low_Valtage = ((bubble_sort_average_value(&Adc_Value_Buff[0][0],7) * 3300) / 4095);
        //PHY_CH[0].Current = ((bubble_sort_average_value(&Adc_Value_Buff[1][0],7) * 3300) / 4095);
        //PHY_CH[1].Current = ((bubble_sort_average_value(&Adc_Value_Buff[2][0],7) * 3300) / 4095);
        //PHY_CH[2].Current = ((bubble_sort_average_value(&Adc_Value_Buff[3][0],7) * 3300) / 4095);
        //PHY_CH[3].Current = ((bubble_sort_average_value(&Adc_Value_Buff[4][0],7) * 3300) / 4095);
/*
        for(i = 0; i < 4; i++)
        {
            if(PHY_CH[i].Uvon == 1)
            {
                if(PHY_CH[i].Current > 2400)
                {
                    //wait_ms(50);
                    PHY_CH[i].Uvon = 0;
                    PHY_CH[i].Time = 0;
                    timer1_channel_gpiomode(i,1,0);
                    Timer_Uvon[i].uvontimer = 0;
                    Timer_Uvon[i].uvoff_flag = 0;
                    CH_LED_switch(i, 1, 1);
                }
                else if(PHY_CH[i].Current < 20)
                {
                    //wait_ms(50);
                    PHY_CH[i].Uvon = 0;
                    PHY_CH[i].Time = 0;
                    timer1_channel_gpiomode(i,1,0);
                    Timer_Uvon[i].uvontimer = 0;
                    Timer_Uvon[i].uvoff_flag = 0;
                    CH_LED_switch(i, 1, 1);
                }
            }
        }*/
        if(Low_Valtage <= 1600)
        {
            gpio_mode_config(GPIOC, GPIO_PIN_13, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
            __GPIO_PIN_RESET(GPIOC,GPIO_PIN_13);
        }
        //__ADC_REG_CONV_START(ADC1);
    }
}

u16 bubble_sort_average_value(u16 *buff,u16 len)
{
    u8 i=0,j=0;
    u16 temp = 0;
    if(len < 3) return 0;
    
    for (i = 0; i < len; i++)
	{
		for (j = 0; j < len - 1; j++) //sort from smallest to biggest
		{
			if (buff[j] > buff[j + 1])
			{
				temp = buff[j];
				buff[j] = buff[j + 1];
				buff[j + 1] = temp;
			}
		}
	}
    temp = 0;
    for(i = 1; i <= len-2; i++)
    {
        temp += buff[i];
    }
    
    return (temp / (len-2));
}

