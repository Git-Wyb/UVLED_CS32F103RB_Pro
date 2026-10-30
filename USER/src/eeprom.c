#include "eeprom.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_misc.h"
#include "iic.h"
#include "timer.h"

#define ZD24C08A_BASE_ADDR    0xA0
#define ZD24C08A_SIZE         1024
#define ZD24C08A_PAGE_SIZE    16
/*
 * ZD24C08A:
 *
 * 1024 Bytes
 * 地址范围：0x000 ~ 0x3FF
 *
 * A9 A8 放在 Device Address 中
 *
 * 0x000 ~ 0x0FF -> 0xA0
 * 0x100 ~ 0x1FF -> 0xA2
 * 0x200 ~ 0x2FF -> 0xA4
 * 0x300 ~ 0x3FF -> 0xA6
 */
 
/**
 ******************************************************************************
 * @file    eeprom.c
 * @brief   TP24C08 (24C08) driver — GPIO bit-banged I2C (软件模拟 I2C)
 *          SCL/SDA 配置为开漏输出, 靠外部 4.7kΩ 上拉拉高
 *          总线速率约 100~200kHz (72MHz 主频)
 ******************************************************************************
 */

/* ---------- GPIO 底层操作 ---------- */
#define SCL_H()   __GPIO_PIN_SET(EE_SCL_GPIO, EE_SCL_PIN)
#define SCL_L()   __GPIO_PIN_RESET(EE_SCL_GPIO, EE_SCL_PIN)
#define SDA_H()   __GPIO_PIN_SET(EE_SDA_GPIO, EE_SDA_PIN)
#define SDA_L()   __GPIO_PIN_RESET(EE_SDA_GPIO, EE_SDA_PIN)
#define SDA_READ() __GPIO_INPUT_PIN_GET(EE_SDA_GPIO, EE_SDA_PIN)

u16 addrbuff_setLevtime[4] = {ADDR_CH1_SetLevTime,ADDR_CH2_SetLevTime,ADDR_CH3_SetLevTime,ADDR_CH4_SetLevTime};
u16 addrbuff_cumutime[4] = {ADDR_CH1_CumulTime,ADDR_CH2_CumulTime,ADDR_CH3_CumulTime,ADDR_CH4_CumulTime};

/* 半周期延时 ~5us (约 100kHz), 72MHz 下粗略循环 */
static void i2c_delay(void)
{
    for (volatile int i = 0; i < 20; i++) { __NOP(); }
}

/* ---------- I2C 基础时序 ---------- */
static void i2c_start(void)
{
    SDA_H(); 
    SCL_H(); 
    i2c_delay();
    SDA_L();        
    i2c_delay();
    SCL_L();         
    i2c_delay();
}

static void i2c_stop(void)
{
    SDA_L(); 
    SCL_H(); 
    i2c_delay();
    SDA_H();       
    i2c_delay();
}

/* 主机向从机写 1 字节, 返回 0=ACK, 1=NACK */
static uint8_t i2c_write_byte(uint8_t b)
{
    uint8_t i, ack;
    for (i = 0; i < 8; i++) {
        if (b & 0x80) SDA_H();
        else SDA_L();
        b <<= 1;
        i2c_delay();
        SCL_H();
        i2c_delay();
        SCL_L(); 
        i2c_delay();
    }
    /* 第 9 个时钟: 读 ACK */
    SDA_H(); 
    i2c_delay();          /* 释放 SDA */
    SCL_H(); 
    i2c_delay();
    ack = SDA_READ();              /* 0=ACK, 1=NACK */
    SCL_L(); 
    i2c_delay();
    return ack;
}

/* 主机从从机读 1 字节, ack=0 表示回 ACK(继续读), ack=1 表示 NACK(读完) */
static uint8_t i2c_read_byte(uint8_t nack)
{
    uint8_t i, b = 0;
    SDA_H();                       /* 释放 SDA 输入 */
    for (i = 0; i < 8; i++) {
        b <<= 1;
        i2c_delay();
        SCL_H(); 
        i2c_delay();
        if (SDA_READ()) b |= 1;
        SCL_L();
        i2c_delay();
    }
    /* 发 ACK / NACK */
    if (nack) SDA_H(); 
    else SDA_L();
    i2c_delay();
    SCL_H();
    i2c_delay();
    SCL_L(); 
    i2c_delay();
    SDA_H();
    return b;
}

/* ---------- EEPROM 层 ---------- */
static uint8_t EE_DevAddr(uint16_t addr)
{
    uint8_t block = (addr >> 8) & 0x03;
    return (0x50U) | ((EE_A2_PIN & 0x01U) << 3) | (block << 1);  /* 7bit */
}

/* 等写周期完成: 反复发设备地址, ACK 即写完成 */
static void EE_WaitWriteDone(uint8_t dev7)
{
    uint32_t timeout = 200;
    while (timeout--) {
        i2c_start();
        if (i2c_write_byte(dev7 << 1) == 0) 
        { 
            i2c_stop(); 
            return; 
        }
        i2c_stop();
        i2c_delay();
    }
}

void EE_Init(void)
{
    EE_GPIO_CLK_EN();
    gpio_mode_config(GPIOC, EE_SCL_PIN|EE_SDA_PIN, GPIO_MODE_OUT_OD(GPIO_SPEED_HIGH));
    gpio_mode_config(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    __GPIO_PIN_RESET(GPIOB, GPIO_PIN_5);
    SCL_H(); 
    SDA_H();                   /* 总线空闲 */
}

/* 写 1 字节 */
uint8_t EE_WriteByte(uint16_t addr, uint8_t data)
{
    uint8_t dev7 = EE_DevAddr(addr);
    uint8_t mem  = (uint8_t)(addr & 0xFF);
    if (addr >= EE_SIZE) return 1;

    i2c_start();
    if (i2c_write_byte(dev7 << 1)) 
    { 
        i2c_stop(); 
        return 2; 
    }  /* 写地址(W) */
    if (i2c_write_byte(mem))       
    { 
        i2c_stop(); 
        return 2; 
    }  /* 字节地址 */
    if (i2c_write_byte(data))    
    { 
        i2c_stop(); 
        return 2; 
    }  /* 数据 */
    i2c_stop();
    EE_WaitWriteDone(dev7);
    return 0;
}

/* 读 1 字节 (随机读: dummy write + restart) */
uint8_t EE_ReadByte(uint16_t addr, uint8_t *data)
{
    uint8_t dev7 = EE_DevAddr(addr);
    uint8_t mem  = (uint8_t)(addr & 0xFF);
    if (addr >= EE_SIZE || data == NULL) return 1;

    i2c_start();
    if (i2c_write_byte(dev7 << 1)) { i2c_stop(); return 2; }
    if (i2c_write_byte(mem))       { i2c_stop(); return 2; }

    i2c_start();                                   /* repeated start */
    if (i2c_write_byte((dev7 << 1) | 1)) { i2c_stop(); return 2; }
    *data = i2c_read_byte(1);                     /* NACK = 只读 1 字节 */
    i2c_stop();
    return 0;
}

/* 写一页 (<=16 字节, 不可跨页) */
uint8_t EE_WritePage(uint16_t addr, const uint8_t *data, uint16_t len)
{
    uint8_t dev7 = EE_DevAddr(addr);
    uint8_t mem  = (uint8_t)(addr & 0xFF);
    uint16_t i;

    if (data == NULL || len == 0 || len > EE_PAGE_SIZE) return 1;
    if ((addr % EE_PAGE_SIZE) + len > EE_PAGE_SIZE)     return 3;

    i2c_start();
    if (i2c_write_byte(dev7 << 1)) { i2c_stop(); return 2; }
    if (i2c_write_byte(mem))       { i2c_stop(); return 2; }
    for (i = 0; i < len; i++) {
        if (i2c_write_byte(data[i])) { i2c_stop(); return 2; }
    }
    i2c_stop();
    EE_WaitWriteDone(dev7);
    return 0;
}

/* 连续写, 自动按页/块拆分 */
uint8_t EE_WriteBuffer(uint16_t addr, const uint8_t *data, uint16_t len)
{
    uint16_t i = 0;
    if (data == NULL || (addr + len) > EE_SIZE) return 1;
    while (i < len) {
        uint16_t chunk = EE_PAGE_SIZE - (addr % EE_PAGE_SIZE);
        if (chunk > (len - i)) chunk = len - i;
        if (EE_WritePage(addr, &data[i], chunk) != 0) return 2;
        addr += chunk;
        i    += chunk;
    }
    return 0;
}

/* 连续读, 自动跨 256B 块 */
uint8_t EE_ReadBuffer(uint16_t addr, uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0 || (addr + len) > EE_SIZE) return 1;

    while (len > 0) {
        uint8_t dev7 = EE_DevAddr(addr);
        uint8_t mem  = (uint8_t)(addr & 0xFF);
        uint16_t chunk = EE_BLOCK_SIZE - (addr & 0xFF);
        if (chunk > len) chunk = len;

        i2c_start();
        if (i2c_write_byte(dev7 << 1)) { i2c_stop(); return 2; }
        if (i2c_write_byte(mem))       { i2c_stop(); return 2; }

        i2c_start();
        if (i2c_write_byte((dev7 << 1) | 1)) { i2c_stop(); return 2; }

        for (uint16_t i = 0; i < chunk; i++) {
            data[i] = i2c_read_byte((i == chunk - 1) ? 1 : 0);
        }
        i2c_stop();

        addr += chunk;
        data += chunk;
        len  -= chunk;
    }
    return 0;
}

/* 整片擦除为 0xFF */
uint8_t EE_EraseChip(void)
{
    for (uint16_t a = 0; a < EE_SIZE; a++) {
        if (EE_WriteByte(a, 0xFF) != 0) return 1;
    }
    return 0;
}

/*-------------------------------------------------------------------------------------------------*/

/*
 * 写一个字节
 */
uint8_t TP24C02_WriteByte(uint8_t addr, uint8_t data)
{
    SoftI2C_Start();

    /* 器件地址 + 写 */
    SoftI2C_SendByte(TP24C02_ADDR_W);

    if(SoftI2C_WaitAck())
    {
        SoftI2C_Stop();
        return 1;
    }

    /* EEPROM 内部地址 */
    SoftI2C_SendByte(addr);

    if(SoftI2C_WaitAck())
    {
        SoftI2C_Stop();
        return 1;
    }

    /* 数据 */
    SoftI2C_SendByte(data);

    if(SoftI2C_WaitAck())
    {
        SoftI2C_Stop();
        return 1;
    }

    SoftI2C_Stop();

    /*
     * TP24C02S5 内部写周期最大约 5ms
     */
    Delay_ms(5);

    return 0;
}


/*
 * 读取一个字节
 */
uint8_t TP24C02_ReadByte(uint8_t addr)
{
    uint8_t data;

    /*
     * 第一次：
     * START + 器件地址W + 地址
     */
    SoftI2C_Start();

    SoftI2C_SendByte(TP24C02_ADDR_W);

    if(SoftI2C_WaitAck())
    {
        SoftI2C_Stop();
        return 0xFF;
    }

    SoftI2C_SendByte(addr);

    if(SoftI2C_WaitAck())
    {
        SoftI2C_Stop();
        return 0xFF;
    }

    /*
     * 重复 START
     */
    SoftI2C_Start();

    /*
     * 器件地址 + 读
     */
    SoftI2C_SendByte(TP24C02_ADDR_R);

    if(SoftI2C_WaitAck())
    {
        SoftI2C_Stop();
        return 0xFF;
    }

    /*
     * 读取数据
     */
    data = SoftI2C_ReadByte();

    /*
     * 最后一个字节发送 NACK
     */
    SoftI2C_NAck();

    SoftI2C_Stop();

    return data;
}


/*
 * 连续写
 *
 * 自动处理8字节Page边界
 */
uint8_t TP24C02_WriteBuffer(uint8_t addr,
                            uint8_t *buf,
                            uint16_t len)
{
    uint8_t page_remain;
    uint16_t count;

    while(len)
    {
        /*
         * 当前页剩余空间
         *
         * 例如：
         * addr = 0x03
         *
         * 当前页还剩：
         * 8 - 3 = 5 Byte
         */
        page_remain = 8 - (addr & 0x07);

        count = (len < page_remain) ? len : page_remain;

        SoftI2C_Start();

        SoftI2C_SendByte(TP24C02_ADDR_W);

        if(SoftI2C_WaitAck())
        {
            SoftI2C_Stop();
            return 1;
        }

        SoftI2C_SendByte(addr);

        if(SoftI2C_WaitAck())
        {
            SoftI2C_Stop();
            return 1;
        }

        while(count--)
        {
            SoftI2C_SendByte(*buf);

            if(SoftI2C_WaitAck())
            {
                SoftI2C_Stop();
                return 1;
            }

            addr++;
            buf++;
            len--;
        }

        SoftI2C_Stop();

        /*
         * EEPROM 内部写周期
         */
        Delay_ms(5);
    }

    return 0;
}


/*
 * 连续读取
 */
void TP24C02_ReadBuffer(uint8_t addr,
                        uint8_t *buf,
                        uint16_t len)
{
    if(len == 0)
        return;

    /*
     * 设置 EEPROM 地址
     */
    SoftI2C_Start();

    SoftI2C_SendByte(TP24C02_ADDR_W);
    SoftI2C_WaitAck();

    SoftI2C_SendByte(addr);
    SoftI2C_WaitAck();

    /*
     * 重复 START
     */
    SoftI2C_Start();

    SoftI2C_SendByte(TP24C02_ADDR_R);
    SoftI2C_WaitAck();

    /*
     * 连续读取
     */
    while(len)
    {
        *buf = SoftI2C_ReadByte();

        buf++;
        len--;

        if(len)
        {
            /*
             * 还有数据，发送 ACK
             */
            SoftI2C_Ack();
        }
        else
        {
            /*
             * 最后一个字节，发送 NACK
             */
            SoftI2C_NAck();
        }
    }

    SoftI2C_Stop();
}

void Delay_ms(u8 ms)
{
    wait_ms(ms);
}

/* =============================================================================================================== */

static uint8_t ZD24C08A_GetDeviceAddr(uint16_t addr)
{
    return ZD24C08A_BASE_ADDR |
           ((addr >> 7) & 0x06);
}
static uint8_t ZD24C08A_WaitReady(void)
{
    uint16_t i;
    uint8_t dev_addr;

    /*
     * ACK Polling 不需要真正的数据地址，
     * 可以使用当前地址所在的 Device Address。
     */
    dev_addr = ZD24C08A_BASE_ADDR;

    for(i = 0; i < 1000; i++)
    {
        SoftI2C_Start();

        SoftI2C_SendByte(dev_addr);

        if(SoftI2C_WaitAck() == 0)
        {
            SoftI2C_Stop();
            return 0;
        }

        SoftI2C_Stop();
    }

    return 1;
}


/*==========================================================
 * 写一个字节
 *==========================================================*/

uint8_t ZD24C08A_WriteByte(uint16_t addr,
                           uint8_t data)
{
    uint8_t dev_addr;

    if(addr >= ZD24C08A_SIZE)
        return 1;

    dev_addr = ZD24C08A_GetDeviceAddr(addr);

    SoftI2C_Start();

    /*
     * Device Address + WRITE
     */
    SoftI2C_SendByte(dev_addr);

    if(SoftI2C_WaitAck())
    {
        SoftI2C_Stop();
        return 2;
    }

    /*
     * Word Address
     */
    SoftI2C_SendByte((uint8_t)addr);

    if(SoftI2C_WaitAck())
    {
        SoftI2C_Stop();
        return 3;
    }

    /*
     * Data
     */
    SoftI2C_SendByte(data);

    if(SoftI2C_WaitAck())
    {
        SoftI2C_Stop();
        return 4;
    }

    SoftI2C_Stop();

    /*
     * 等待 EEPROM 内部写完成
     */
    if(ZD24C08A_WaitReady())
        return 5;

    return 0;
}


/*==========================================================
 * 读取一个字节
 *==========================================================*/

uint8_t ZD24C08A_ReadByte(uint16_t addr)
{
    uint8_t dev_addr;
    uint8_t data;

    if(addr >= ZD24C08A_SIZE)
        return 0xFF;

    dev_addr = ZD24C08A_GetDeviceAddr(addr);

    /*
     * 第一步：
     * Dummy Write
     *
     * 写入要读取的地址
     */
    SoftI2C_Start();

    SoftI2C_SendByte(dev_addr);

    if(SoftI2C_WaitAck())
    {
        SoftI2C_Stop();
        return 0xFF;
    }

    SoftI2C_SendByte((uint8_t)addr);

    if(SoftI2C_WaitAck())
    {
        SoftI2C_Stop();
        return 0xFF;
    }

    /*
     * 第二步：
     * Repeated START
     */
    SoftI2C_Start();

    /*
     * Device Address + READ
     */
    SoftI2C_SendByte(dev_addr | 0x01);

    if(SoftI2C_WaitAck())
    {
        SoftI2C_Stop();
        return 0xFF;
    }

    /*
     * 接收数据
     */
    data = SoftI2C_ReadByte();

    /*
     * 最后一个字节必须 NACK
     */
    SoftI2C_NAck();

    SoftI2C_Stop();

    return data;
}

uint8_t ZD24C08A_PageWrite(uint16_t addr,
                           uint8_t *buf,
                           uint8_t len)
{
    uint8_t i;
    uint8_t dev_addr;
    uint8_t page_remain;

    if(addr >= ZD24C08A_SIZE)
        return 1;

    if(len == 0)
        return 0;

    /*
     * 计算当前页剩余空间
     *
     * 例如：
     *
     * addr = 0x10
     * 剩余 16 Byte
     *
     * addr = 0x15
     * 剩余 11 Byte
     */
    page_remain =
        ZD24C08A_PAGE_SIZE -
        (addr & 0x0F);

    /*
     * 不能一次跨页写
     */
    if(len > page_remain)
        return 2;

    /*
     * 不能超过 EEPROM 容量
     */
    if((addr + len) > ZD24C08A_SIZE)
        return 3;

    dev_addr = ZD24C08A_GetDeviceAddr(addr);

    SoftI2C_Start();

    /*
     * Device Address + Write
     */
    SoftI2C_SendByte(dev_addr);

    if(SoftI2C_WaitAck())
    {
        SoftI2C_Stop();
        return 4;
    }

    /*
     * Word Address
     */
    SoftI2C_SendByte((uint8_t)addr);

    if(SoftI2C_WaitAck())
    {
        SoftI2C_Stop();
        return 5;
    }

    /*
     * 连续写数据
     */
    for(i = 0; i < len; i++)
    {
        SoftI2C_SendByte(buf[i]);

        if(SoftI2C_WaitAck())
        {
            SoftI2C_Stop();
            return 6;
        }
    }

    SoftI2C_Stop();

    /*
     * 等待内部写完成
     */
    if(ZD24C08A_WaitReady())
        return 7;

    return 0;
}

void ZD24C08A_Read(uint16_t addr,
                   uint8_t *buf,
                   uint16_t len)
{
    uint16_t i;
    uint8_t dev_addr;

    if(addr >= ZD24C08A_SIZE)
        return;

    if(len == 0)
        return;

    if((addr + len) > ZD24C08A_SIZE)
        len = ZD24C08A_SIZE - addr;

    dev_addr = ZD24C08A_GetDeviceAddr(addr);

    /*
     * Dummy Write
     */
    SoftI2C_Start();

    SoftI2C_SendByte(dev_addr);

    if(SoftI2C_WaitAck())
    {
        SoftI2C_Stop();
        return;
    }

    SoftI2C_SendByte((uint8_t)addr);

    if(SoftI2C_WaitAck())
    {
        SoftI2C_Stop();
        return;
    }

    /*
     * Repeated START
     */
    SoftI2C_Start();

    /*
     * Device Address + READ
     */
    SoftI2C_SendByte(dev_addr | 0x01);

    if(SoftI2C_WaitAck())
    {
        SoftI2C_Stop();
        return;
    }

    /*
     * 连续读取
     */
    for(i = 0; i < len; i++)
    {
        buf[i] = SoftI2C_ReadByte();

        if(i < (len - 1))
        {
            /*
             * 还要继续读取
             * 主机发送 ACK
             */
            SoftI2C_Ack();
        }
        else
        {
            /*
             * 最后一个字节
             * 主机发送 NACK
             */
            SoftI2C_NAck();
        }
    }

    SoftI2C_Stop();
}

u8 sbuff[10] = {0};
void eeprom_save_parameter(EESAVE_TYPE ptype)
{
    u8 si = 0;
    switch(ptype)
    {
        case SAVE_CUMUTIME:
            for(si = 0; si < 4; si++)
            {
                if(PHY_CH[si].flag_update_cumutime)
                {
                    if(PHY_CH[si].pCH.CumulTime > TIME_CUMUL_MAX) PHY_CH[si].pCH.CumulTime = TIME_CUMUL_MAX;
                    ZD24C08A_PageWrite(addrbuff_cumutime[si],&PHY_CH[si].pCH.ctimbyte0,CumulTime_Length);
                    PHY_CH[si].flag_update_cumutime = 0;
                }
            }
            break;
        
        case SAVE_LEVTIME:
            for(si = 0; si < 4; si++)
            {
                if(PHY_CH[si].SetLevel != PHY_CH[si].ReadLevel || PHY_CH[si].SetTime != PHY_CH[si].ReadTime)
                {
                    sbuff[0] = PHY_CH[si].SetLevel;
                    sbuff[1] = 0;
                    sbuff[2] = (PHY_CH[si].SetTime) & 0xFF;
                    sbuff[3] = (PHY_CH[si].SetTime >> 8) & 0xFF;
                    ZD24C08A_PageWrite(addrbuff_setLevtime[si],sbuff,SetLevTime_Length);
                }
            }
            break;
            
        case SAVE_PARAMETER:
            ZD24C08A_PageWrite(ADDR_SET_Parameter,&Set_Mode.ext_input.setval,6);
            break;
    }
}

void eeprom_read_parameter(void)
{
    u8 si = 0;
    
    ZD24C08A_Read(ADDR_CH1_CumulTime,&PHY_CH[0].pCH.ctimbyte0,CumulTime_Length);
    ZD24C08A_Read(ADDR_CH2_CumulTime,&PHY_CH[1].pCH.ctimbyte0,CumulTime_Length);
    ZD24C08A_Read(ADDR_CH3_CumulTime,&PHY_CH[2].pCH.ctimbyte0,CumulTime_Length);
    ZD24C08A_Read(ADDR_CH4_CumulTime,&PHY_CH[3].pCH.ctimbyte0,CumulTime_Length);
    
    for(si = 0; si < 4; si++)
    {
        ZD24C08A_Read(addrbuff_setLevtime[si],sbuff,SetLevTime_Length);
        PHY_CH[si].SetLevel = sbuff[0];
        PHY_CH[si].SetTime = sbuff[2] | (sbuff[3] << 8);
        PHY_CH[si].ReadLevel = PHY_CH[si].SetLevel;
        PHY_CH[si].ReadTime = PHY_CH[si].SetTime;
    }
    
    ZD24C08A_Read(ADDR_SET_Parameter,&Set_Mode.ext_input.setval,6);
}

