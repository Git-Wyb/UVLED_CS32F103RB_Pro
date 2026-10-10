#ifndef __GPIO_H__
#define __GPIO_H__

#define GPIOx_IN(GPIOx,PINx)    (u8)__GPIO_INPUT_PIN_GET(GPIOx, PINx)
#define GPIOx_OUT(GPIOx,PINx,y) {if(y){__GPIO_PIN_SET(GPIOx, PINx);}else{__GPIO_PIN_RESET(GPIOx, PINx);}}
//input
#define CH4_CHECK1_IN GPIOx_IN(GPIOA,GPIO_PIN_6) 
#define CH4_CHECK2_IN GPIOx_IN(GPIOA,GPIO_PIN_7)

#define CH3_CHECK1_IN GPIOx_IN(GPIOC,GPIO_PIN_4)
#define CH3_CHECK2_IN GPIOx_IN(GPIOC,GPIO_PIN_5)

#define CH2_CHECK1_IN GPIOx_IN(GPIOB,GPIO_PIN_0)
#define CH2_CHECK2_IN GPIOx_IN(GPIOA,GPIO_PIN_5)

#define CH1_CHECK1_IN GPIOx_IN(GPIOA,GPIO_PIN_4)
#define CH1_CHECK2_IN GPIOx_IN(GPIOA,GPIO_PIN_3)

#define EXT_TRG_IN    GPIOx_IN(GPIOA,GPIO_PIN_12) /* All channels irradiation input. */
#define EXT_TRG1_IN   GPIOx_IN(GPIOC,GPIO_PIN_6)  /* CH1 Individual irradiation input. */
#define EXT_TRG2_IN   GPIOx_IN(GPIOB,GPIO_PIN_15) /* CH2 Individual irradiation input. */
#define EXT_TRG3_IN   GPIOx_IN(GPIOB,GPIO_PIN_14) /* CH3 Individual irradiation input. */
#define EXT_TRG4_IN   GPIOx_IN(GPIOB,GPIO_PIN_13) /* CH4 Individual irradiation input. */
#define EXT_EMG_IN    GPIOx_IN(GPIOC,GPIO_PIN_9)  /* emergency stop. */

//output
#define CH1_PROT_OUT(x)  GPIOx_OUT(GPIOC,GPIO_PIN_0,x)
#define CH2_PROT_OUT(x)  GPIOx_OUT(GPIOB,GPIO_PIN_4,x)
#define CH3_PROT_OUT(x)  GPIOx_OUT(GPIOD,GPIO_PIN_2,x)
#define CH4_PROT_OUT(x)  GPIOx_OUT(GPIOC,GPIO_PIN_2,x)
#define EXT_RYD1_OUT(x)  GPIOx_OUT(GPIOB,GPIO_PIN_12,x) /* CH1 can be used for output irradiation */
#define EXT_RYD2_OUT(x)  GPIOx_OUT(GPIOB,GPIO_PIN_11,x) /* CH2 can be used for output irradiation */
#define EXT_RYD3_OUT(x)  GPIOx_OUT(GPIOB,GPIO_PIN_2,x)  /* CH3 can be used for output irradiation */
#define EXT_RYD4_OUT(x)  GPIOx_OUT(GPIOB,GPIO_PIN_1,x)  /* CH4 can be used for output irradiation */
#define EXT_UVON_OUT(x)  GPIOx_OUT(GPIOC,GPIO_PIN_8,x)  /* As long as there is a channel for outputting, it will output.*/
#define EXT_ERROR_OUT(x) GPIOx_OUT(GPIOC,GPIO_PIN_7,x)  /* If an error occurs in this device, the output */

void Init_Gpio(void);



#endif
