#include "timer.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_misc.h"
#include "cs32f10x_tim.h"
#include "cs32f10x_gpio.h"

u16 pwm_buf[100] = {7,   14, 22, 29, 36, 43, 50, 58, 65, 72,
                    79,  86, 94,101,108,115,122,130,137,144,
                    151,158,166,173,180,187,194,202,209,216,
                    223,230,238,245,252,259,266,274,281,288,
                    295,302,310,317,324,331,338,346,353,360,
                    367,374,382,389,396,403,410,418,425,432,
                    439,446,454,461,468,475,482,490,497,504,
                    511,518,526,533,540,547,554,562,569,576,
                    583,590,598,605,612,619,626,634,641,648,
                    655,662,670,677,684,691,698,706,713,720};
void Init_Timer3(void)
{
    tim_base_t timer_config_struct;
    nvic_init_t nvic_config_struct;

    __RCU_APB1_CLK_ENABLE(RCU_APB1_PERI_TIM3); // TIM3 clock enable
    
    // Time base configuration
    timer_config_struct.period = 1000 - 1; //ARR自动重载值
    timer_config_struct.pre_div = 0;
    timer_config_struct.clk_div = 0;
    timer_config_struct.cnt_mode = TIM_CNT_MODE_UP;
    tim_base_init(TIM3, &timer_config_struct);
    
    //f = fCK_PDIV / (PDIV[15:0] + 1) = 72MHz / (71+1) = 1MHz.
    tim_prescaler_set(TIM3, 71, TIM_PDIV_MODE_IMMEDIATE); // Prescaler configuration,
    
    /* Configure and enable TIM interrupt. */
    nvic_config_struct.nvic_irqchannel = IRQn_TIM3;
    nvic_config_struct.nvic_irq_enable = ENABLE;
    nvic_config_struct.nvic_irq_pre_priority = 1;
    nvic_config_struct.nvic_irq_sub_priority = 0;
    nvic_init(&nvic_config_struct);
    
    __TIM_INTR_ENABLE(TIM3,TIM_INTR_UPDATE);
    __TIM_ENABLE(TIM3);
}

u16 ix = 0;
void TIM3_IRQHandler(void)
{
    if(__TIM_FLAG_STATUS_GET(TIM3,TIM_FLAG_UPDATE) != RESET)
    {
        __TIM_FLAG_CLEAR(TIM3,TIM_FLAG_UPDATE);
        
        if(time_ms) time_ms--;
        if(time_keyscan) time_keyscan--;
        time_10ms++;
        if(time_10ms >= 10)
        {
            time_10ms = 0;
        }
    }
}

void wait_ms(u16 ms)
{
    time_ms = ms;
    while(time_ms)
    {
        ;
    }
}

//BUZZER: TIM2 -> CH3, f = 2.4Khz
void Init_Timer2(void)
{
    tim_base_t timer_config_struct;
    tim_choc_t timer_compare_struct;

    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOB);
    __RCU_APB1_CLK_ENABLE(RCU_APB1_PERI_TIM2); // TIM2 clock enable  (PCLK=72MHZ)
    
    gpio_mode_config(GPIOB, GPIO_PIN_10, GPIO_MODE_OUT_AFPP(GPIO_SPEED_HIGH));
    gpio_pin_remap_config(GPIO_REMAP_PARTIAL2_TIM2,ENABLE); //Function reuse with remapping TIM2_CH3
    
    // Time Base configuration
    timer_config_struct.period = 30000 - 1; //ARR自动重载值,clk=72MHz / f=2.4K = 30000; 
    timer_config_struct.pre_div = 0;
    timer_config_struct.clk_div = 0;
    timer_config_struct.cnt_mode = TIM_CNT_MODE_UP;
    timer_config_struct.repeate_count = 0;
    tim_base_init(TIM2, &timer_config_struct);
    
    // Channel 4 Configuration in PWM mode.
    timer_compare_struct.oc_mode = TIM_OCMODE_PWM1;
    timer_compare_struct.output_state = TIM_OUTPUT_EN; //Enable output compare state
    timer_compare_struct.output_state_n = TIM_OUTPUT_N_DIS; //Disable complementary output Compare
    timer_compare_struct.polarity = TIM_OUTPUT_POLARITY_HIGH; //Caputure compare polarity is high level
    timer_compare_struct.polarity_n = TIM_OUTPUT_POLARITY_N_LOW; //Complementary caputure compare polarity is low level
    timer_compare_struct.idle_state = TIM_OUTPUT_IDLE_SET;  //Set idle output value of compare
    timer_compare_struct.idle_state_n = TIM_OUTPUT_N_IDLE_RESET; //Reset idle output value of compare.
    
    timer_compare_struct.channel = TIM_CHANNEL_3;
    timer_compare_struct.pulse = 15000;  //PWM = 50%;
    tim_choc_init(TIM2, &timer_compare_struct);
    
    __TIM_ENABLE(TIM2); 
    __TIM_FUNC_ENABLE(TIM2, CH_OUTPUT);
    tim_chcc_enable_ctrl(TIM2,TIM_CHANNEL_3,ENABLE);
}

void Bueezr_Switch(enable_state_t sta)
{
    if(sta == ENABLE) 
    {
        Init_Timer2();
    }
    else
    {
        tim_chcc_enable_ctrl(TIM2,TIM_CHANNEL_3,DISABLE);
        __TIM_FUNC_DISABLE(TIM2, CH_OUTPUT);
        __TIM_DISABLE(TIM2);
        gpio_mode_config(GPIOB, GPIO_PIN_10, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
        __GPIO_PIN_SET(GPIOB,GPIO_PIN_10);
    }
}

u16 arr_period = 0;
/* UV LED CH1 - CH4: PA8 - PA11: TIM1 -> CH1 - CH4*/
void Init_Timer1(u8 fkhz)
{
    tim_base_t timer_config_struct;
    tim_choc_t timer_compare_struct;
    tim_def_init(TIM1);

    arr_period = (u16)(72000 / fkhz);
    
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOA);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_TIM1); // TIM1 clock enable  (PCLK=72MHZ)
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_AFIO);
    
    gpio_mode_config(GPIOA, GPIO_PIN_8,  GPIO_MODE_OUT_AFPP(GPIO_SPEED_HIGH));
    gpio_mode_config(GPIOA, GPIO_PIN_9,  GPIO_MODE_OUT_AFPP(GPIO_SPEED_HIGH));
    gpio_mode_config(GPIOA, GPIO_PIN_10, GPIO_MODE_OUT_AFPP(GPIO_SPEED_HIGH));
    gpio_mode_config(GPIOA, GPIO_PIN_11, GPIO_MODE_OUT_AFPP(GPIO_SPEED_HIGH));
    //gpio_pin_remap_config(GPIO_REMAP_PARTIAL_TIM1,ENABLE); //No need for re-mapping
    
    // Time Base configuration
    timer_config_struct.period = arr_period;//7200 - 1; //ARR自动重载值,clk=72MHz / f=10K =7200
    timer_config_struct.pre_div = 0;
    timer_config_struct.clk_div = 0;
    timer_config_struct.cnt_mode = TIM_CNT_MODE_UP;
    timer_config_struct.repeate_count = 0;
    tim_base_init(TIM1, &timer_config_struct);
    
    // Channel 4 Configuration in PWM mode.
    timer_compare_struct.oc_mode = TIM_OCMODE_PWM1;
    timer_compare_struct.output_state = TIM_OUTPUT_EN; //Enable output compare state
    timer_compare_struct.output_state_n = TIM_OUTPUT_N_DIS; //Disable complementary output Compare
    timer_compare_struct.polarity = TIM_OUTPUT_POLARITY_HIGH; //Caputure compare polarity is high level
    timer_compare_struct.polarity_n = TIM_OUTPUT_POLARITY_N_LOW; //Complementary caputure compare polarity is low level
    timer_compare_struct.idle_state = TIM_OUTPUT_IDLE_RESET;  //Set idle output value of compare
    timer_compare_struct.idle_state_n = TIM_OUTPUT_N_IDLE_RESET; //Reset idle output value of compare.
    
    timer_compare_struct.channel = TIM_CHANNEL_1;
    timer_compare_struct.pulse = 0;
    tim_choc_init(TIM1, &timer_compare_struct);
    
    timer_compare_struct.channel = TIM_CHANNEL_2;
    timer_compare_struct.pulse = 0;
    tim_choc_init(TIM1, &timer_compare_struct);
    
    timer_compare_struct.channel = TIM_CHANNEL_3;
    timer_compare_struct.pulse = 0;
    tim_choc_init(TIM1, &timer_compare_struct);
    
    timer_compare_struct.channel = TIM_CHANNEL_4;
    timer_compare_struct.pulse = 0;
    tim_choc_init(TIM1, &timer_compare_struct);
    
    __TIM_ENABLE(TIM1); 
    __TIM_FUNC_ENABLE(TIM1, CH_OUTPUT);
    tim_chcc_enable_ctrl(TIM1,TIM_CHANNEL_1,DISABLE);
    tim_chcc_enable_ctrl(TIM1,TIM_CHANNEL_2,DISABLE);
    tim_chcc_enable_ctrl(TIM1,TIM_CHANNEL_3,DISABLE);
    tim_chcc_enable_ctrl(TIM1,TIM_CHANNEL_4,DISABLE);
}

void tim_pwm_output_enable_ctrl(tim_reg_t* ptr_timer, uint16_t channel, enable_state_t onoff)
{
    uint16_t temp = 0;

    temp =  (uint16_t)(0x0001 << (channel * 4));

    /* clear the CHxCCEN bit */
    ptr_timer->CCCTR &= (uint16_t)~ temp;

    /* config CHxCCEN bit */ 
    ptr_timer->CCCTR |=  (uint16_t)(onoff << (channel * 4));

}

void UV_LED_Switch(CH_ENUM channel, enable_state_t onoff)
{
    if(channel == TIM_CHANNEL_ALL)
    {
        tim_pwm_output_enable_ctrl(TIM1,TIM_CHANNEL_1,onoff);
        tim_pwm_output_enable_ctrl(TIM1,TIM_CHANNEL_2,onoff);
        tim_pwm_output_enable_ctrl(TIM1,TIM_CHANNEL_3,onoff);
        tim_pwm_output_enable_ctrl(TIM1,TIM_CHANNEL_4,onoff);
    }
    else tim_pwm_output_enable_ctrl(TIM1,channel,onoff);
}
 
void UV_LED_PwmSet(CH_ENUM channel, u8 pwm)
{
    if(pwm > 100) pwm = 100;
    //if(pwm == 0) pwm = 1;
    if(channel == TIM_CHANNEL_ALL)
    {
        TIM1->CHXCCVAL[TIM_CHANNEL_1] = (pwm * (arr_period / 100));
        TIM1->CHXCCVAL[TIM_CHANNEL_2] = (pwm * (arr_period / 100));
        TIM1->CHXCCVAL[TIM_CHANNEL_3] = (pwm * (arr_period / 100));
        TIM1->CHXCCVAL[TIM_CHANNEL_4] = (pwm * (arr_period / 100));
    }
    else TIM1->CHXCCVAL[channel] = (pwm * (arr_period / 100));
}

