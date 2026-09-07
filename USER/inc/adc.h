#ifndef __ADC_H__
#define __ADC_H__

#include "sys.h"

void Init_Adc(void);
void adc_nvic_config(void);
void get_adc_val(void);
u16 bubble_sort_average_value(u16 *buff,u16 len);
void uvled_current_detection(void);
void adc_dma_value(void);

#endif
