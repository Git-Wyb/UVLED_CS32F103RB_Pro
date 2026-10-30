#ifndef __USART_H__
#define __USART_H__

#include "sys.h"

#define RX_BUFF_MAX 64

#define PARAM_INVAR 0xFFFF  //The parameters remain unchanged.
#define PARAM_ERROR 0xF0F0  //Parameter error

#define NON_CMD     (0)
#define CMD_UVEXE   (0x01)
#define CMD_LEVEL   (0x02)
#define CMD_TIMER   (0x03)

#define REQ_STATUS  (0x11)
#define REQ_LEVEL   (0x12)
#define REQ_TIMER   (0x13)
#define REQ_HOUR    (0x14)
#define REQ_MODE    (0x15)
#define REQ_VER     (0x16)

#define CMD_NONE    (0)
#define CMD_NORMAL  (0x01)          // コマンド正常受信
#define CMD_ERR     (0x02)          // コマンドエラー（存在しない）
#define PRM_ERR     (0x03)          // パラメータ異常（フォーマット?範囲）
#define SUM_ERR     (0x04)          // チェックサムエラー
#define LENGTH_ERR  (0x05)          // コマンド長異常
#define CMD_BSY     (0x10)          // 処理中です
#define CMD_REQ     (0x20)          // コマンド要求（詳細は別途

extern u8 rx_buf[RX_BUFF_MAX];
extern u8 Tx_Buff[RX_BUFF_MAX];
extern u8 rx_data_buff[RX_BUFF_MAX];
extern u8 rx_index;
extern u8 rx_size;
extern u8 tx_size;
extern u8 test232_rxbuf[10];
unsigned char AsciiToHex(unsigned char num);
unsigned char HexToAscii(unsigned char num);
unsigned char AsciiToHex_Byte(unsigned char asc1, unsigned char asc0);
void usart_recvdata_check(u8 *buff, u8 length);
void Init_Usart1(void);
void usart1_send_data(u8 *buf,u8 len);
void ans_tx_ack(void);
void data_calcul_sum(unsigned char *buff,unsigned char *sumbuff,unsigned int size);
void Usart_Data_Handle(void);
unsigned char AsciiToBool(unsigned char data);
void command_handle(void);
void ans_make_uvon(u8 *uv_req);
u16 _cmd_param_check(u8 *buff, u8 length, u16 val_min, u16 val_max);
void rscmd_tx_ack(u8 errtype);
void usart_tx_error(void);
void command_param_handle(void);

#endif
