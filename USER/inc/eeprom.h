#ifndef __EEPROM_H__
#define __EEPROM_H__

/**
 ******************************************************************************
 * @file    eeprom.h
 * @brief   TP24C08 (24C08) I2C EEPROM driver for CS32F103RB
 *          SOFTWARE (GPIO bit-banged) I2C version
 *          Default pins: SCL=PB8, SDA=PB9
 ******************************************************************************
 */
#include <stdio.h>
#include "cs32f10x.h"
#include "cs32f10x_conf.h"
#include "sys.h"

/* ===== 引脚配置: 改这里即可换到任意 GPIO ===== */
#define EE_SCL_GPIO      GPIOB
#define EE_SCL_PIN       GPIO_PIN_8
#define EE_SDA_GPIO      GPIOB
#define EE_SDA_PIN       GPIO_PIN_9
#define EE_GPIO_CLK_EN()  do {__RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOB); } while(0)

/* EEPROM 参数 */
#define EE_SIZE            256//1024U
#define EE_PAGE_SIZE       8//16U
#define EE_BLOCK_SIZE      256U

/* A2 引脚电平 (接 GND=0, 接 VCC=1) */
#define EE_A2_PIN          0U

/* Public API (与硬件 I2C 版本完全一致) */
void    EE_Init(void);
uint8_t EE_WriteByte(uint16_t addr, uint8_t data);
uint8_t EE_ReadByte(uint16_t addr, uint8_t *data);
uint8_t EE_WritePage(uint16_t addr, const uint8_t *data, uint16_t len);
uint8_t EE_WriteBuffer(uint16_t addr, const uint8_t *data, uint16_t len);
uint8_t EE_ReadBuffer(uint16_t addr, uint8_t *data, uint16_t len);
uint8_t EE_EraseChip(void);
void EE_Init(void);

#define TP24C02_ADDR_W    0xA0
#define TP24C02_ADDR_R    0xA1

uint8_t TP24C02_WriteByte(uint8_t addr, uint8_t data);
uint8_t TP24C02_ReadByte(uint8_t addr);

uint8_t TP24C02_WriteBuffer(uint8_t addr,
                            uint8_t *buf,
                            uint16_t len);

void TP24C02_ReadBuffer(uint8_t addr,
                        uint8_t *buf,
                        uint16_t len);

void Delay_ms(u8 ms);
uint8_t ZD24C08A_PageWrite(uint16_t addr,
                           uint8_t *buf,
                           uint8_t len);
void ZD24C08A_Read(uint16_t addr,
                   uint8_t *buf,
                   uint16_t len);

uint8_t ZD24C08A_WriteByte(uint16_t addr,
                           uint8_t data);

uint8_t ZD24C08A_ReadByte(uint16_t addr);

#endif

