#ifndef __TM1639_H__
#define __TM1639_H__

#include "cs32f10x.h"
#include "cs32f10x_conf.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_misc.h"
#include "cs32f10x_rcu.h"
#include "sys.h"
/*
 ——a——
|      |
f      b
|      |
 ——g——
|      |
e      c
|      |
 ——d——  dp(point)
*/
//---------------------------------------------------------------------------------------------
#define DISPLAY_ADDR0 0x00
#define DISPLAY_ADDR1 0x01
#define DISPLAY_ADDR2 0x02

typedef enum
{
    LED_LEVEL = 0,
    LED_TIME,
    LED_MENU,
    LED_HOUR,
    LED_CH1_RED,
    LED_CH2_RED,
    LED_CH3_RED,
    LED_CH4_RED,
    LED_CH1_GREEN,
    LED_CH2_GREEN,
    LED_CH3_GREEN,
    LED_CH4_GREEN,
}LED_ENUM;

typedef enum
{
    DISPLAY_888 = 0,
    DISPLAY_Ein,
    DISPLAY_Pri,
    DISPLAY_bur,
    DISPLAY_oPL,
    DISPLAY_4Uh,
    DISPLAY_232,
    DISPLAY_LEU,
    DISPLAY_PuL,
    DISPLAY_ein,
    DISPLAY_SiG,
    DISPLAY_on,
    DISPLAY_oFF,
    DISPLAY_96,
    DISPLAY_192,
    DISPLAY_384,
    DISPLAY_CH1,
    DISPLAY_CH2,
    DISPLAY_CH3,
    DISPLAY_CH4,
    DISPLAY_ALL
}DISPLAY_ENUM;

#define TM1639_STB_PORT    GPIOC
#define TM1639_STB_PIN     GPIO_PIN_12
#define TM1639_CLK_PORT    GPIOC
#define TM1639_CLK_PIN     GPIO_PIN_11
#define TM1639_DIO_PORT    GPIOC
#define TM1639_DIO_PIN     GPIO_PIN_10

/* 位操作宏 */
#define TM1639_STB_H()   (__GPIO_PIN_SET(TM1639_STB_PORT,TM1639_STB_PIN))
#define TM1639_STB_L()   (__GPIO_PIN_RESET(TM1639_STB_PORT,TM1639_STB_PIN))
#define TM1639_CLK_H()   (__GPIO_PIN_SET(TM1639_CLK_PORT,TM1639_CLK_PIN))
#define TM1639_CLK_L()   (__GPIO_PIN_RESET(TM1639_CLK_PORT,TM1639_CLK_PIN))
#define TM1639_DIO_H()   (__GPIO_PIN_SET(TM1639_DIO_PORT,TM1639_DIO_PIN))
#define TM1639_DIO_L()   (__GPIO_PIN_RESET(TM1639_DIO_PORT,TM1639_DIO_PIN))
#define TM1639_DIO_R()   (u8)(__GPIO_INPUT_PIN_GET(TM1639_DIO_PORT, TM1639_DIO_PIN))

/* DIO 方向切换（读键时需要输入） */
#define TM1639_DIO_OUT()  do { gpio_mode_config(TM1639_DIO_PORT, TM1639_DIO_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH)); } while(0)  /* PB14 推挽输出 */
#define TM1639_DIO_IN()   do { gpio_mode_config(TM1639_DIO_PORT, TM1639_DIO_PIN, GPIO_MODE_IN_PU); } while(0)  /* PB14 上拉输入 */

/* 函数声明 */
void Init_TM1639(void);
void TM1639_Clear(void);
void TM1639_SetBrightness(uint8_t level);   // 0~7, 0最暗
void TM1639_WriteByte(uint8_t addr, uint8_t data);
void TM1639_WriteBuf(uint8_t addr, uint8_t *buf, uint8_t len);
void TM1639_DisplayNum(uint16_t num);       // 显示 0~999 的整数
void TM1639_DisplayChar(uint8_t pos, char ch, uint8_t point); // pos: 0~2 (第1~3位), ch: 字符
uint16_t TM1639_ReadKey(void);              // 读按键值（如未接按键可不用）
uint8_t TM1639_Read_Key(void);
void TM1639_LED_switch(LED_ENUM lednum,u8 onoff);
void TM1639_DisplayUVtime(NUMSTU uvtime);
void TM1639_Display_UVLED_Char(DISPLAY_ENUM pos);
void CH_LED_switch(u8 ledch,u8 err,u8 onoff);

#endif
/*
       K1     K2
UP     0x04  0x00
Down   0x00  0x40
left   0x40  0x00
right  0x00  0x04
set    0x00  0x08

mode   0x08  0x00
ch     0x80  0x00
uvon   0x00  0x80
*/

