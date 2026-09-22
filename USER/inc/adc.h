#ifndef __ADC_H__
#define __ADC_H__

#include "sys.h"

void Init_Adc(void);
void adc_nvic_config(void);
void get_adc_val(void);
u16 data_average_calcul(u16 *buff,u16 len);
void uvled_current_detection(void);
void adc_dma_value(void);
void uvled_current_error_handle(void);
void _UVLED_CurrCheck_Enable(void);
void _UVLED_CurrCheck_Disable(void);

#endif
