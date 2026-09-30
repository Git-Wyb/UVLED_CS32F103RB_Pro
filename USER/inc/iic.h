#ifndef __IIC_H__
#define __IIC_H__

#include "sys.h"

void SoftI2C_Init(void);
void SoftI2C_SendByte(uint8_t data);
uint8_t SoftI2C_ReadByte(void);
void SoftI2C_NAck(void);
void SoftI2C_Ack(void);
uint8_t SoftI2C_WaitAck(void);
void SoftI2C_Stop(void);
void SoftI2C_Start(void);

#endif
