#include "iic.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_misc.h"
#if 1
#define IIC_SCL_PORT GPIOB
#define IIC_SCL_PIN  GPIO_PIN_8

#define IIC_SDA_PORT GPIOB
#define IIC_SDA_PIN  GPIO_PIN_9

#define WP_PORT GPIOB
#define WP_PIN  GPIO_PIN_5

#define SDA_READ() __GPIO_INPUT_PIN_GET(IIC_SDA_PORT, IIC_SDA_PIN)

#define SCL_H()   __GPIO_PIN_SET(IIC_SCL_PORT, IIC_SCL_PIN)
#define SCL_L()   __GPIO_PIN_RESET(IIC_SCL_PORT, IIC_SCL_PIN)
#define SDA_H()   __GPIO_PIN_SET(IIC_SDA_PORT, IIC_SDA_PIN)
#define SDA_L()   __GPIO_PIN_RESET(IIC_SDA_PORT, IIC_SDA_PIN)

/*
 * 简单延时
 * 如果系统主频是 72MHz，这个延时可以用于低速软件 I2C。
 */
static void I2C_Delay(void)
{
    volatile uint8_t i;

    for(i = 0; i < 10; i++)
    {
        __NOP();
    }
}

/*
 * SDA 设置为开漏输出
 */
static void SDA_Output(void)
{
    gpio_mode_config(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_MODE_OUT_OD(GPIO_SPEED_HIGH));
}


/*
 * SDA 设置为输入
 */
static void SDA_Input(void)
{
    gpio_mode_config(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_MODE_IN_PU);
}


/*
 * 软件 I2C 初始化
 */
void SoftI2C_Init(void)
{
    /* SCL */
    gpio_mode_config(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_MODE_OUT_OD(GPIO_SPEED_HIGH));
    gpio_mode_config(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    __GPIO_PIN_RESET(GPIOB, GPIO_PIN_5);
    /* SDA */
    SDA_Output();

    /* 总线空闲 */
    SDA_H();
    SCL_H();

    I2C_Delay();
}


/*
 * I2C START
 *
 * SDA:  ￣￣\____
 * SCL:  ￣￣￣￣\_
 */
void SoftI2C_Start(void)
{
    SDA_Output();

    SDA_H();
    SCL_H();

    I2C_Delay();

    SDA_L();

    I2C_Delay();

    SCL_L();

    I2C_Delay();
}


/*
 * I2C STOP
 *
 * SDA: ____/￣￣
 * SCL: ____/￣￣
 */
void SoftI2C_Stop(void)
{
    SDA_Output();

    SCL_L();
    SDA_L();

    I2C_Delay();

    SCL_H();

    I2C_Delay();

    SDA_H();

    I2C_Delay();
}


/*
 * 等待从机 ACK
 *
 * 返回：
 * 0 = 收到 ACK
 * 1 = 没有 ACK
 */
uint8_t SoftI2C_WaitAck(void)
{
    uint16_t timeout = 0;

    SDA_Input();

    SDA_H();

    I2C_Delay();

    SCL_H();

    I2C_Delay();

    while(SDA_READ())
    {
        timeout++;

        if(timeout > 1000)
        {
            SCL_L();
            SDA_Output();

            return 1;
        }
    }

    SCL_L();

    SDA_Output();

    return 0;
}


/*
 * 主机发送 ACK
 */
void SoftI2C_Ack(void)
{
    SDA_Output();

    SDA_L();

    I2C_Delay();

    SCL_H();

    I2C_Delay();

    SCL_L();

    I2C_Delay();

    SDA_H();
}


/*
 * 主机发送 NACK
 */
void SoftI2C_NAck(void)
{
    SDA_Output();

    SDA_H();

    I2C_Delay();

    SCL_H();

    I2C_Delay();

    SCL_L();

    I2C_Delay();
}


/*
 * 发送一个字节
 * MSB first
 */
void SoftI2C_SendByte(uint8_t data)
{
    uint8_t i;

    SDA_Output();

    for(i = 0; i < 8; i++)
    {
        SCL_L();

        if(data & 0x80)
        {
            SDA_H();
        }
        else
        {
            SDA_L();
        }

        I2C_Delay();

        SCL_H();

        I2C_Delay();

        data <<= 1;
    }

    SCL_L();

    SDA_H();
}


/*
 * 接收一个字节
 */
uint8_t SoftI2C_ReadByte(void)
{
    uint8_t i;
    uint8_t data = 0;

    SDA_Input();

    for(i = 0; i < 8; i++)
    {
        SCL_L();

        I2C_Delay();

        SCL_H();

        I2C_Delay();

        data <<= 1;

        if(SDA_READ())
        {
            data |= 0x01;
        }
    }

    SCL_L();

    SDA_Output();

    return data;
}
#endif
