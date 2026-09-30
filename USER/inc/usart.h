#ifndef __USART_H__
#define __USART_H__

#include "sys.h"


void Init_Usart1(u32 baud);
void usart1_send_data(u8 *buf,u8 len);

extern u8 rx_buf[10];
extern u8 rx_index;
extern u8 test232_rxbuf[10];

#endif
