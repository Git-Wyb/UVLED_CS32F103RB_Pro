#include "usart.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_misc.h"
#include "UVLED.h"

u8 Rx_Buff[RX_BUFF_MAX] = {0};
u8 Tx_Buff[RX_BUFF_MAX] = {0};
u8 rx_data_buff[RX_BUFF_MAX] = {0};
u8 param_uvch[CHNUM] = {0};
u16 param_level[CHNUM] = {PARAM_INVAR,PARAM_INVAR,PARAM_INVAR,PARAM_INVAR};
u16 param_timer[CHNUM] = {PARAM_INVAR,PARAM_INVAR,PARAM_INVAR,PARAM_INVAR};
u8 rx_size = 0;
u8 tx_size = 0;
u8 rx_index = 0;
u8 RSCmd = 0;
u8 errtype = 0;
u8 test232_rxbuf[10] = {0};
// command strings
char setting_uvexe[16] = "S_uvexe:" ;
char setting_level[16] = "S_level:" ;
char setting_timer[16] = "S_timer:" ;

char request_status[16] = "R_status:" ;
char request_level[16] = "R_level:" ;
char request_timer[16] = "R_timer:" ;
char request_hour[16] = "R_hour:" ;
char request_mode[16] = "R_mode:" ;
char request_ver[16] = "R_ver:" ;

const unsigned char cmd_setting_length[10] =
//unsigned char   cmd_setting_length[9] =
{
    // cmd
    8 , // S_uvexe:*,*,*,*,SUM[2]
    8 , // S_level:***,***,***,***,SUM[2]
    8 , // S_timer:****,****,****,****,SUM[2]
    // req
    9 , // R_status:SUM[2]
    8 , // R_level:SUM[2]
    8 , // R_timer:SUM[2]
    7 , // R_hour:SUM[2]
    7 , // R_mode:SUM[2]
    6 , // R_ver:SUM[2]
    NULL 
} ;

char* cmd_setting_cmd[10] = {
    // setting
    &setting_uvexe[0] ,
    &setting_level[0] ,            // S_level:***,***,***,***SUM[2]
    &setting_timer[0] ,           // S_timer:****,****,****,****SUM[2]
    // request
    &request_status[0] ,          // R_status:SUM[2]
    &request_level[0] ,            // R_level:SUM[2]
    &request_timer[0] ,            // R_timer:SUM[2]
    &request_hour[0] ,               // R_hour:SUM[2]
    &request_mode[0] ,               // R_mode:SUM[2]
    &request_ver[0] ,              // R_ver:SUM[2]
    NULL 
};
    
const unsigned char cmd_setting_cmd_type[10] = {
    CMD_UVEXE ,          // S_uvexe:*,*,*,*SUM[2]
    CMD_LEVEL ,          // S_level:***,***,***,***SUM[2]
    CMD_TIMER ,          // S_timer:****,****,****,****SUM[2]
    // request
    REQ_STATUS ,         // R_status:SUM[2]
    REQ_LEVEL ,          // R_level:SUM[2]
    REQ_TIMER ,          // R_timer:SUM[2]
    REQ_HOUR ,           // R_hour:SUM[2]
    REQ_MODE ,           // R_mode:SUM[2]
    REQ_VER ,            // R_Ver:SUM[2]
    NULL 
    
};

const unsigned char cmd_setting_data_length[10] = {
    // setting
    18 ,              // S_uvexe:*,*,*,*,SUM[2]
    26  ,             // S_level:***,***,***,***,SUM[2]
    30  ,             // S_timer:****,****,****,****,SUM[2]
    // request
    11  ,             // R_status:SUM[2]
    10  ,             // R_level:SUM[2]
    10  ,             // R_timer:SUM[2]
    9  ,              // R_hour:SUM[2]
    9  ,              // R_mode:SUM[2]
    8 ,               // R_Ver:SUM[2]
    NULL
} ;

/*
PB6 -> USART1_TX
PB7 -> USART1_RX
*/
void Init_Usart1(void)
{
    u32 baud = 1;
    usart_config_t ptr_usart;
    nvic_init_t nvic_config_struct;
    
    switch(Set_Mode.usart_baud.setval)
    {
        case 0:
            baud = 9600;
            break;
        case 1:
            baud = 19200;
            break;
        case 2:
            baud = 38400;
            break;
        default:
            baud = 19200;
            break;
    }
    /* Enable clocks. */
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOB);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_USART1);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_AFIO);
    
    gpio_pin_remap_config(AFIO_MP_USART1_RMP,ENABLE);
    /* Configure PB6(TX) and PB7(RX). */
    gpio_mode_config(GPIOB, GPIO_PIN_6, GPIO_MODE_OUT_AFPP(GPIO_SPEED_HIGH));
    gpio_mode_config(GPIOB, GPIO_PIN_7, GPIO_MODE_IN_PU);
    
    /* Configure the basic information of USART1. */
    usart_def_init(USART1);
    ptr_usart.baud_rate = baud;
    ptr_usart.data_width = USART_DATA_WIDTH_8;
    ptr_usart.flow_control = USART_FLOW_CONTROL_NONE;
    ptr_usart.parity_check = USART_PARITY_NONE;
    ptr_usart.stop_bits = USART_STOP_BIT_1;
    ptr_usart.transceiver_mode = USART_MODE_TX_RX;
    usart_init(USART1, &ptr_usart);
    
     /* Enable the USART Interrupt */
    nvic_config_struct.nvic_irqchannel = IRQn_USART1;
    nvic_config_struct.nvic_irq_pre_priority = 1;
    nvic_config_struct.nvic_irq_sub_priority = 1;
    nvic_config_struct.nvic_irq_enable = ENABLE;
    nvic_init(&nvic_config_struct);
    __USART_INTR_ENABLE(USART1, RXNE); // Enable the USART Receive interrupt
    
    /* Enable USART1. */
    __USART_ENABLE(USART1);
}

u8 rxdata = 0;
u8 it = 0;
void USART1_IRQHandler(void)
{
    /* USART Receiver*/
    if (__USART_FLAG_STATUS_GET(USART1, RXNE) == SET)
    {
        if(flag_test_mode == 0)
        {
            Rx_Buff[rx_index++] = (u8)__USART_DATA_RECV(USART1);
            time_usart_rx = 100;
            if(rx_index > 4)
            {
                if(Rx_Buff[rx_index-2] == '\r' && Rx_Buff[rx_index-1] == '\n')
                {
                    usart_recvdata_check(Rx_Buff, rx_index-2); //exclude CR LF.
                    rx_index = 0;
                }
            }
            if(rx_index > RX_BUFF_MAX-1) rx_index = 0;
        }
        else
        {
            if(rxdata == '(' || flag_rx_head == 1)
            {
                flag_rx_head = 1;
                Rx_Buff[rx_index++] = rxdata;
                if(rx_index > 9 || Rx_Buff[rx_index-1] == ')') //"(R)"
                {
                    //__USART_INTR_DISABLE(USART1, RXNE);
                    if(Rx_Buff[1] == 'R' && Rx_Buff[2] == ')')
                    {
                        flag_debug_rx_done = 1;
                    }
                    else if(Rx_Buff[1] == 'T' && Rx_Buff[9] == ')')
                    {
                        flag_test_232 = 1;
                        for(it=0; it<10; it++) test232_rxbuf[it] = Rx_Buff[it];
                    }
                    rx_index = 0;
                    Rx_Buff[2] = 0;
                    Rx_Buff[0] = 0;
                    flag_rx_head = 0;
                }
            }
        }
    }        
}

int fputc(int ch, FILE *f)
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
    __USART_DATA_SEND(USART1, (uint8_t)ch);
    while(__USART_FLAG_STATUS_GET(USART1, TXE) == RESET);
    return ch;
}

void usart1_send_data(u8 *buf,u8 len)
{
    u8 i = 0;
    __USART_INTR_DISABLE(USART1, RXNE);
    for(i=0; i<len; i++)
    {
        __USART_DATA_SEND(USART1, buf[i]);
        while(__USART_FLAG_STATUS_GET(USART1, TXE) == RESET);
    }
    __USART_INTR_ENABLE(USART1, RXNE);
}

void usart_recvdata_check(u8 *buff, u8 length)
{
    u8 sum = 0,rxsum = 0,cnt = 0;
    if(length < 2) return;
    
    for(cnt = 0; cnt < (length - 2); cnt++) //exclude SUM
    {
        sum += buff[cnt];
    }
    rxsum = AsciiToHex_Byte(buff[length-2], buff[length-1]);
    if(sum == rxsum)
    {
        flag_rx_done = 1;
        rx_size = length; //SUM
        memcpy(rx_data_buff, buff, length-2); //exclude SUM.
    }
    else
    {
        errtype = SUM_ERR;
        rscmd_tx_ack(SUM_ERR);
    }
}

void ans_tx_ack(void)
{
    Tx_Buff[0] = 'S';
    Tx_Buff[1] = 'E';
    Tx_Buff[2] = ':';
    data_calcul_sum(Tx_Buff,&Tx_Buff[3],3);
    Tx_Buff[5] = '\r';
    Tx_Buff[6] = '\n';
    tx_size = 7;
}

void Usart_Data_Handle(void)
{
    if(time_usart_rx == 0) rx_index = 0;
    
    if(flag_rx_done)
    {
        flag_rx_done = 0;
        command_handle();
        command_param_handle();
    }
    usart_tx_error();
    if(tx_size)
    {
        usart1_send_data(Tx_Buff,tx_size);
        tx_size = 0;
    }
}

void command_handle(void)
{
    u8 cnt = 0,rcmd = 0;
    u8 ch_cnt = 0;
    
    errtype = CMD_ERR;
    while(cmd_setting_length[cnt] != 0)
    {
        if(strncmp((const char*)rx_data_buff, (const char*)cmd_setting_cmd[cnt], cmd_setting_length[cnt]) == 0)
        {
            if(rx_size == cmd_setting_data_length[cnt])
            {
                rcmd = cmd_setting_cmd_type[cnt];
                switch(rcmd)
                {
                    case CMD_UVEXE:
                        param_uvch[0] = AsciiToBool(rx_data_buff[8]);
                        param_uvch[1] = AsciiToBool(rx_data_buff[10]);
                        param_uvch[2] = AsciiToBool(rx_data_buff[12]);
                        param_uvch[3] = AsciiToBool(rx_data_buff[14]);
                        if((0xFF != param_uvch[0]) && (rx_data_buff[9] == ',')
                          && (0xFF != param_uvch[1]) && (rx_data_buff[11] == ',')
                          && (0xFF != param_uvch[2]) && (rx_data_buff[13] == ',')
                          && (0xFF != param_uvch[3]) && (rx_data_buff[15] == ','))
                        {
                            if(((param_uvch[0] == 1) && (_check_uvled_ch_error(0) != 0))
                                 || ((param_uvch[1] == 1) && (_check_uvled_ch_error(1) != 0))
                                 || ((param_uvch[2] == 1) && (_check_uvled_ch_error(2) != 0))
                                 || ((param_uvch[3] == 1) && (_check_uvled_ch_error(3) != 0)))
                            {
                                errtype = PRM_ERR;
                            }
                            else
                            {
                                RSCmd = CMD_UVEXE;
                                errtype = CMD_NONE;
                            }
                        }
                        rcmd = NON_CMD;
                        break;
                    
                    case CMD_LEVEL:
                        param_level[0] = _cmd_param_check(&rx_data_buff[8],  3, 0, 100);   // ch1
                        param_level[1] = _cmd_param_check(&rx_data_buff[12], 3, 0, 100);  // ch2
                        param_level[2] = _cmd_param_check(&rx_data_buff[16], 3, 0, 100);  // ch3
                        param_level[3] = _cmd_param_check(&rx_data_buff[20], 3, 0, 100);  // ch4
                        if(( param_level[0] != PARAM_ERROR) && (rx_data_buff[11] == ',')
                         && (param_level[1] != PARAM_ERROR) && (rx_data_buff[15] == ',')
                         && (param_level[2] != PARAM_ERROR) && (rx_data_buff[19] == ',')
                         && (param_level[3] != PARAM_ERROR) && (rx_data_buff[23] == ','))
                        {
                            RSCmd = CMD_LEVEL; //OK
                            errtype = CMD_NONE;
                        }
                        else //NG
                        {// parameter error
                            errtype = PRM_ERR;
                        }
                        rcmd = NON_CMD;
                        break;
                    
                    case CMD_TIMER:
                        param_timer[0] = _cmd_param_check(&rx_data_buff[8],  4, 0, 9990);   // ch1
                        param_timer[1] = _cmd_param_check(&rx_data_buff[13], 4, 0, 9990);  // ch2
                        param_timer[2] = _cmd_param_check(&rx_data_buff[18], 4, 0, 9990);  // ch3
                        param_timer[3] = _cmd_param_check(&rx_data_buff[23], 4, 0, 9990);  // ch4
                        if(( param_timer[0] != PARAM_ERROR) && (rx_data_buff[12] == ',')
                         && (param_timer[1] != PARAM_ERROR) && (rx_data_buff[17] == ',')
                         && (param_timer[2] != PARAM_ERROR) && (rx_data_buff[22] == ',')
                         && (param_timer[3] != PARAM_ERROR) && (rx_data_buff[27] == ','))
                        {
                            for(ch_cnt = 0; ch_cnt < 4; ch_cnt++)
                            {
                                if(PARAM_INVAR != param_timer[ch_cnt])
                                {
                                    if(param_timer[ch_cnt] > 999) //99.9 above
                                    {
                                        param_timer[ch_cnt] = ( param_timer[ch_cnt] / 10 ) * 10 ;
                                    }
                                }
                            }
                            RSCmd = CMD_TIMER; //OK
                            errtype = CMD_NONE;
                        }
                        else
                        {
                            errtype = PRM_ERR;
                        }
                        rcmd = NON_CMD;
                        break;

                    case REQ_STATUS:
                        RSCmd = REQ_STATUS;
                        rcmd = NON_CMD;
                        errtype = CMD_NONE;
                        break;
                    
                    case REQ_LEVEL:
                        RSCmd = REQ_LEVEL;
                        rcmd = NON_CMD;
                        errtype = CMD_NONE;
                        break;
                    
                    case REQ_TIMER:
                        RSCmd = REQ_TIMER;
                        rcmd = NON_CMD;
                        errtype = CMD_NONE;
                        break;
                    
                    case REQ_HOUR:
                        RSCmd = REQ_HOUR;
                        rcmd = NON_CMD;
                        errtype = CMD_NONE;
                        break;
                    
                    case REQ_MODE:
                        RSCmd = REQ_MODE;
                        rcmd = NON_CMD;
                        errtype = CMD_NONE;
                        break;
                    
                    case REQ_VER:
                        RSCmd = REQ_VER;
                        rcmd = NON_CMD;
                        errtype = CMD_NONE;
                        break;
                    
                    case NON_CMD:
                    default:
                        break ;   
                }
            }
            else
            {
                errtype = LENGTH_ERR;
            }
            break;
        }
        else
        {
            cnt++;
        }
    }
    rscmd_tx_ack(errtype);
}

void command_param_handle(void)
{
    u8 ch = 0;
    if(RSCmd != CMD_NONE)
    {
        if(rs_mode == RUN_MODE && Err_Emg != ERR_EO1) //no err
        {
            switch(RSCmd)
            {
                case CMD_NONE:
                    break;
                
                case CMD_UVEXE:
                    RSCmd = CMD_NONE;
                    for(ch = 0; ch < CHNUM; ch++)
                    {
                        if(param_uvch[ch] == 1 && PHY_CH[ch].Uvon == 0)
                        {
                            if(_check_uvled_ch_error(ch) == 0)
                            {
                                uvonch_last = 0;
                                PHY_UvLed_switch(ch+1);
                            }
                        }
                        else if(param_uvch[ch] == 0 && PHY_CH[ch].Uvon == 1)
                        {
                            uvonch_last = 0;
                            PHY_UvLed_switch(ch+1);
                        }
                        else{}
                    }
                    rscmd_tx_ack(CMD_NORMAL);
                    break;
                
                case CMD_LEVEL:
                    RSCmd = CMD_NONE;
                    if(Set_Mode.opl_not.setval != 0 || _check_uvled_sta() != 0)  //OPL ON
                    {
                        rscmd_tx_ack(CMD_BSY);
                        break;
                    }
                    else //OPL OFF
                    {
                        for(ch = 0; ch < CHNUM; ch++)
                        {
                            if((param_level[ch] != PARAM_INVAR) && (param_level[ch] != PARAM_ERROR))
                            {
                                UV_LED_PwmSet(ch,param_level[ch]);
                            }
                        }
                        rscmd_tx_ack(CMD_NORMAL);
                    }
                    break;
                
                case CMD_TIMER:
                    RSCmd = CMD_NONE;
                    if(Set_Mode.opl_not.setval != 0 || _check_uvled_sta() != 0)  //OPL ON
                    {
                        rscmd_tx_ack(CMD_BSY);
                        break;
                    }
                    else //OPL OFF
                    {
                        for(ch = 0; ch < CHNUM; ch++)
                        {
                            if((param_timer[ch] != PARAM_INVAR) && (param_timer[ch] != PARAM_ERROR))
                            {
                                _set_uvled_timer(ch, param_timer[ch]);
                            }
                        }
                        rscmd_tx_ack(CMD_NORMAL);
                    }
                    break;
                
                case REQ_STATUS:
                    RSCmd = CMD_NONE;
                    rscmd_tx_ack(REQ_STATUS);
                    break;
                
                case REQ_LEVEL:
                    RSCmd = CMD_NONE;
                    rscmd_tx_ack(REQ_LEVEL);
                    break;
                
                case REQ_TIMER:
                    RSCmd = CMD_NONE;
                    rscmd_tx_ack(REQ_TIMER);
                    break;
                
                case REQ_HOUR:
                    RSCmd = CMD_NONE;
                    rscmd_tx_ack(REQ_HOUR);
                    break;
                
                case REQ_MODE:
                    RSCmd = CMD_NONE;
                    rscmd_tx_ack(REQ_MODE);
                    break;
                
                case REQ_VER:
                    RSCmd = CMD_NONE;
                    rscmd_tx_ack(REQ_VER);
                    break;
                
                default:
                    RSCmd = CMD_NONE;
                    break;
            }
        }
        else
        {
            RSCmd = CMD_NONE;
            rscmd_tx_ack(CMD_BSY);
        }
    }
}

void rscmd_tx_ack(u8 type)
{
    u8 p = 0;
    u8 ch = 0;
    u8 level = 0;
    u16 timer = 0;
    u16 hour = 0;
    switch(type)
    {
        case CMD_NONE:
            break;
        
        case CMD_NORMAL:
            Tx_Buff[0] = 'O' ;
            Tx_Buff[1] = 'K' ;
            Tx_Buff[2] = ':' ;
            data_calcul_sum(&Tx_Buff[0], &Tx_Buff[3], 3) ;
            Tx_Buff[5] = '\r' ;
            Tx_Buff[6] = '\n' ;
            tx_size = 7 ;
            break ;
        
        case CMD_ERR:
            Tx_Buff[0] = 'C' ;
            Tx_Buff[1] = 'E' ;
            Tx_Buff[2] = ':' ;
            data_calcul_sum(&Tx_Buff[0], &Tx_Buff[3], 3) ;
            Tx_Buff[5] = '\r' ;
            Tx_Buff[6] = '\n' ;
            tx_size = 7 ;
            break ;
        
        case PRM_ERR:
            Tx_Buff[0] = 'P' ;
            Tx_Buff[1] = 'E' ;
            Tx_Buff[2] = ':' ;
            data_calcul_sum(&Tx_Buff[0], &Tx_Buff[3], 3) ;
            Tx_Buff[5] = '\r' ;
            Tx_Buff[6] = '\n' ;
            tx_size = 7 ;
            break ;
        
        case SUM_ERR:
            Tx_Buff[0] = 'S' ;
            Tx_Buff[1] = 'E' ;
            Tx_Buff[2] = ':' ;
            data_calcul_sum(&Tx_Buff[0] , &Tx_Buff[3], 3);
            Tx_Buff[5] = '\r' ;
            Tx_Buff[6] = '\n' ;
            tx_size = 7 ;
            break ;
        
        case LENGTH_ERR:
            Tx_Buff[0] = 'L' ;
            Tx_Buff[1] = 'E' ;
            Tx_Buff[2] = ':' ;
            data_calcul_sum(&Tx_Buff[0], &Tx_Buff[3], 3);
            Tx_Buff[5] = '\r' ;
            Tx_Buff[6] = '\n' ;
            tx_size = 7 ;
            break ;
        
        case CMD_BSY:
            Tx_Buff[0] = 'B' ;
            Tx_Buff[1] = 'Z' ;
            Tx_Buff[2] = ':' ;
            data_calcul_sum(&Tx_Buff[0], &Tx_Buff[3], 3);
            Tx_Buff[5] = '\r' ;
            Tx_Buff[6] = '\n' ;
            tx_size = 7 ;
            break ;
        
        case REQ_VER:
            p = 0 ;
            Tx_Buff[p] = 'V' ;
            p++ ;
            Tx_Buff[p] = 'e' ;
            p++ ;
            Tx_Buff[p] = 'r' ;
            p++ ;
            Tx_Buff[p] = ':' ;
            p++ ;       
            Tx_Buff[p] = HexToAscii( ( SOFT_VERSION >> 8 ) & 0x0F ) ;
            p++ ;
            Tx_Buff[p] = '.' ;
            p++ ;
            Tx_Buff[p] = HexToAscii( ( SOFT_VERSION >> 4 ) & 0x0F ) ;
            p++ ;
            Tx_Buff[p] = HexToAscii( ( SOFT_VERSION >> 0 ) & 0x0F ) ;
            p++ ;
            Tx_Buff[p] = ',' ;
            p++ ;
            data_calcul_sum( &Tx_Buff[0] , &Tx_Buff[p] , p ) ;
            p++ ;               // SUM
            p++ ;               // SUM
            Tx_Buff[p] = '\r' ;
            p++ ;
            Tx_Buff[p] = '\n' ;
            p++ ;
            tx_size = p ;
            break ;
            
        case REQ_STATUS:
            p = 0 ;
            Tx_Buff[p] = 's' ;
            p++ ;
            Tx_Buff[p] = 't' ;
            p++ ;
            Tx_Buff[p] = 'a' ;
            p++ ;
            Tx_Buff[p] = 't' ;
            p++ ;
            Tx_Buff[p] = 'u' ;
            p++ ;
            Tx_Buff[p] = 's' ;
            p++ ;
            Tx_Buff[p] = ':' ;
            p++ ;
            if(rs_mode != RUN_MODE)
            {
                for( ch = 0 ; ch < 4 ; ch++ )
                {
                    Tx_Buff[p] = 'B' ;
                    p++ ;
                    Tx_Buff[p] = ',' ;
                    p++ ;
                }
            }
            else if(Err_Emg == ERR_EO1)
            {
                for( ch = 0 ; ch < 4 ; ch++ )
                {
                    Tx_Buff[p] = 'E' ;
                    p++ ;
                    Tx_Buff[p] = '1' ;
                    p++ ;
                    Tx_Buff[p] = '2' ;
                    p++ ;
                    Tx_Buff[p] = ',' ;
                    p++ ;
                }
            }
            else
            {
                for( ch = 0 ; ch < 4 ; ch++ )
                {
                    if(PHY_CH[ch].ErrCode != 0)      // no cable
                    {
                        if( PHY_CH[ch].ErrCode != ERR_Ld3 )
                        {
                            Tx_Buff[p] = 'N' ;                // NO cable
                            p++ ;
                        }
                        else
                        {
                            Tx_Buff[p] = 'E' ;
                            p++ ;
                            Tx_Buff[p] = '4' ;
                            p++ ;
                            Tx_Buff[p] = '3' ;
                            p++ ;
                            Tx_Buff[p] = ',' ;
                            p++ ;
                        }
                    }
                    else
                    {
                        if(PHY_CH[ch].Ready)
                        {
                            Tx_Buff[p] = 'R' ;
                            p++ ;
                        }
                        else if(PHY_CH[ch].Uvon)
                        {
                            Tx_Buff[p] = 'O' ;
                            p++ ;
                        }
                        else
                        {
                            Tx_Buff[p] = 'E' ;
                            p++ ;
                            Tx_Buff[p] = HexToAscii(PHY_CH[ch].ErrCode / 10 ) ;
                            p++ ;
                            Tx_Buff[p] = HexToAscii(PHY_CH[ch].ErrCode % 10 ) ;
                            p++ ;
                        }
                    }
                }
                Tx_Buff[p] = ',' ;
                p++ ;
            }
            data_calcul_sum( &Tx_Buff[0] , &Tx_Buff[p] , p ) ;
            p++ ;
            p++ ;
            Tx_Buff[p] = '\r' ;
            p++ ;
            Tx_Buff[p] = '\n' ;
            p++ ;
            tx_size = p ;
            break ;
                
        case REQ_LEVEL:
            p = 0 ;
            Tx_Buff[p] = 'l' ;
            p++ ;
            Tx_Buff[p] = 'e' ;
            p++ ;
            Tx_Buff[p] = 'v' ;
            p++ ;
            Tx_Buff[p] = 'e' ;
            p++ ;
            Tx_Buff[p] = 'l' ;
            p++ ;
            Tx_Buff[p] = ':' ;
            p++ ;
            for( ch = 0 ; ch < 4 ; ch++ )
            {
                level = PHY_CH[ch].SetLevel;
                Tx_Buff[p] = HexToAscii( level / 100 ) ;     // １００の位
                p++ ;

                level %= 100 ;
                Tx_Buff[p] = HexToAscii( level / 10 ) ;      // １０の位
                p++ ;

                level %= 10 ;
                Tx_Buff[p] = HexToAscii( level ) ;           // １の位
                p++ ;
                Tx_Buff[p] = ',' ;
                p++ ;
            }
            data_calcul_sum( &Tx_Buff[0] , &Tx_Buff[p] , p ) ;
            p++ ;
            p++ ;
            Tx_Buff[p] = '\r' ;
            p++ ;
            Tx_Buff[p] = '\n' ;
            p++ ;
            tx_size = p ;
            break ;
            
        case REQ_TIMER:
            p = 0 ;
            Tx_Buff[p] = 't' ;
            p++ ;
            Tx_Buff[p] = 'i' ;
            p++ ;
            Tx_Buff[p] = 'm' ;
            p++ ;
            Tx_Buff[p] = 'e' ;
            p++ ;
            Tx_Buff[p] = 'r' ;
            p++ ;
            Tx_Buff[p] = ':' ;
            p++ ;
            for( ch = 0 ; ch < 4 ; ch++ )
            {
                timer = PHY_CH[ch].SetTime ;
                Tx_Buff[p] = HexToAscii( timer / 1000 ) ;        // １０００の位
                p++ ;
                timer %= 1000 ;
                Tx_Buff[p] = HexToAscii( timer / 100 ) ;         // １００の位
                p++ ;
                timer %= 100 ;
                Tx_Buff[p] = HexToAscii( timer / 10 ) ;          //　１０の位
                p++ ;
                timer %= 10 ;
                Tx_Buff[p] = HexToAscii( timer ) ;               // １の位
                p++ ;
                Tx_Buff[p] = ',' ;
                p++ ;
            }
            data_calcul_sum( &Tx_Buff[0] , &Tx_Buff[p] , p ) ;
            p++ ;
            p++ ;
            Tx_Buff[p] = '\r' ;
            p++ ;
            Tx_Buff[p] = '\n' ;
            p++ ;
            tx_size = p ;
            break ;
            
        case REQ_HOUR:
            p = 0 ;
            Tx_Buff[p] = 'h' ;
            p++ ;
            Tx_Buff[p] = 'o' ;
            p++ ;
            Tx_Buff[p] = 'u' ;
            p++ ;
            Tx_Buff[p] = 'r' ;
            p++ ;
            Tx_Buff[p] = ':' ;
            p++ ;
            for( ch = 0 ; ch < 4 ; ch++ )
            {
                if(PHY_CH[ch].pCH.CumulTime > TIME_CUMUL_MAX) PHY_CH[ch].pCH.CumulTime = TIME_CUMUL_MAX;
                hour = (PHY_CH[ch].pCH.CumulTime / 36000);
                Tx_Buff[p] = HexToAscii( hour / 10000 ) ;        // １００００の位
                p++ ;
                hour %= 10000 ;
                Tx_Buff[p] = HexToAscii( hour / 1000 ) ;        // １０００の位
                p++ ;
                hour %= 1000 ;
                Tx_Buff[p] = HexToAscii( hour / 100 ) ;         // １００の位
                p++ ;
                hour %= 100 ;
                Tx_Buff[p] = HexToAscii( hour / 10 ) ;          //　１０の位
                p++ ;
                hour %= 10 ;
                Tx_Buff[p] = HexToAscii( hour ) ;               // １の位
                p++ ;
                Tx_Buff[p] = ',' ;
                p++ ;
            }
            data_calcul_sum( &Tx_Buff[0] , &Tx_Buff[p] , p ) ;
            p++ ;
            p++ ;
            Tx_Buff[p] = '\r' ;
            p++ ;
            Tx_Buff[p] = '\n' ;
            p++ ;
            tx_size = p ;
            break ;
            
        case REQ_MODE:                              // 動作モード要求
            p = 0 ;
            Tx_Buff[p] = 'm' ;
            p++ ;
            Tx_Buff[p] = 'o' ;
            p++ ;
            Tx_Buff[p] = 'd' ;
            p++ ;
            Tx_Buff[p] = 'e' ;
            p++ ;
            Tx_Buff[p] = ':' ;
            p++ ;
            
            if(Set_Mode.ext_input.setval == 0)    // 外部入力設定
            {
                Tx_Buff[p] = '0' ;                // レベル
            }
            else
            {
                Tx_Buff[p] = '1' ;                // パルス
            }
            p++ ;
            Tx_Buff[p] = ',' ;
            p++ ;

            if(Set_Mode.trg_pri.setval == 0)    // トリガ入力優先度
            {
                Tx_Buff[p] = '0' ;                // 時間優先
            }
            else
            {
                Tx_Buff[p] = '1' ;                // 信号優先
            }
            p++ ;
            Tx_Buff[p] = ',' ;
            p++ ;
            
            if(Set_Mode.buzzer_sw.setval == 0)    // ブザー
            {
                Tx_Buff[p] = '0' ;                // ＯＦＦ
            }
            else
            {
                Tx_Buff[p] = '1' ;                // ＯＮ
            }
            p++ ;
            Tx_Buff[p] = ',' ;
            p++ ;
            
            if(Set_Mode.opl_not.setval == 0)    // 操作禁止
            {
                Tx_Buff[p] = '0' ;                // 操作許可（Ｕｎｌｏｃｋ）
            }
            else
            {
                Tx_Buff[p] = '1' ;                // 操作禁止（ＬＯＣＫ）
            }
            p++ ;
            Tx_Buff[p] = ',' ;
            p++ ;

            if(Set_Mode.H_4VH.setval == 0)      // Ｈ－４ＶＨ　ＭＯＤＥ
            {
                Tx_Buff[p] = '0' ;                // Ｈ－４ＶＨ　Ｍｏｄｅ（ＯＦＦ）
            }
            else
            {
                Tx_Buff[p] = '1' ;                // Ｈ－４ＶＨ　Ｍｏｄｅ（ＯＮ）
            }
            p++ ;
            Tx_Buff[p] = ',' ;
            p++ ;
            switch(Set_Mode.usart_baud.setval)       // ＲＳ２３２Ｃ（ｂｐｓ）
            {
                case 0:
                    Tx_Buff[p] = '0' ;            // 9600bps
                    break ;
                
                case 1:
                    Tx_Buff[p] = '1' ;            // 19200bps
                    break ;
                
                case 2:
                    Tx_Buff[p] = '2' ;            // 38400bps
                    break ;
                
                default:
                    Tx_Buff[p] = 'X' ;            // fail
                    break ;
            }
            p++ ;
            Tx_Buff[p] = ',' ;
            p++ ;
            data_calcul_sum( &Tx_Buff[0] , &Tx_Buff[p] , p ) ;
            p++ ;                                 
            p++ ;
            Tx_Buff[p] = '\r' ;
            p++ ;
            Tx_Buff[p] = '\n' ;
            p++ ;
            tx_size = p ;
            break ;
            
        default:
            break ;
    }
}

void usart_tx_error(void)
{
    u8 ch = 0,p = 0;
    if(flag_tx_error)
    {
        flag_tx_error = 0;
        for(ch = 0; ch < CHNUM; ch++)
        {
            if(PHY_CH[ch].ErrCode && PHY_CH[ch].flag_ch_err_tx == 0)
            {
                PHY_CH[ch].flag_ch_err_tx = 1;
                p = 0 ;
                Tx_Buff[p] = 'E' ;
                p++ ;
                Tx_Buff[p] = 'r' ;
                p++ ;
                Tx_Buff[p] = 'r' ;
                p++ ;
                Tx_Buff[p] = ':' ;
                p++ ;
                Tx_Buff[p] = HexToAscii( ch + 1 ) ;                           // エラーチャンネル
                p++ ;
                Tx_Buff[p] = ',';                                             // エラーチャンネル
                p++ ;
                Tx_Buff[p] = HexToAscii(PHY_CH[ch].ErrCode / 10) ;            // エラーコード上位桁
                p++ ;
                Tx_Buff[p] = HexToAscii(PHY_CH[ch].ErrCode % 10) ;                     // エラーコード下位桁
                p++ ;
                Tx_Buff[p] = ',' ;                                            // SUM値の前にカンマ
                p++ ;
                data_calcul_sum( &Tx_Buff[0] , &Tx_Buff[p] , p ) ;
                p++ ;                                   //サム値格納あとは２バイト更新
                p++ ;

                Tx_Buff[p] = '\r' ;
                p++ ;
                Tx_Buff[p] = '\n' ;
                p++ ;
                tx_size = p ;
            }
        }
    }
}

u16 _cmd_param_check(u8 *buff, u8 length, u16 val_min, u16 val_max)
{
    u16 value = 0; u8 cnt = 0; u8 l_dec = 0;
    
    if(*buff == '*')
    {
        value = PARAM_INVAR;  //unchanged
        for( cnt = 1 ; cnt < length ; cnt++ )
        {
            if( buff[cnt] != '*' )        
            {
                value = PARAM_ERROR; //NG
            }
        }
    }
    else
    {
        for(cnt = 0; cnt < length; cnt++)
        {
            value *= 10 ;
            l_dec = AsciiToHex(*buff) ;
            if(0xFF != l_dec)
            {
                value += l_dec ;
            }
            else
            {
                value = PARAM_ERROR ;          // NG
                break ;
            }
            buff++ ;
        }
        if(value != PARAM_ERROR)
        {
            if(value < val_min || value > val_max) value = PARAM_ERROR;
        }
    }
    return value;
}

void ans_make_uvon(u8 *uv_req)                                // エラー応答
{
    u8 pos = 0 ;
    u8 i = 0;
    
    Tx_Buff[0] = 'P' ;
    Tx_Buff[1] = 'E' ;
    Tx_Buff[2] = ':' ;
    pos = 3 ;
    for(i = 0; i < 4; i++)
    {
        if((uv_req[i] == 1) && (_check_uvled_ch_error(i) != 0))
        {
            Tx_Buff[pos] = HexToAscii(i + 1) ;             // ＣＨ番号セット
            pos++ ;
            Tx_Buff[pos] = ',' ;             // ＣＨ番号セット
            pos++ ;
            Tx_Buff[pos] = 'E' ;
            pos++ ;
            Tx_Buff[pos] = HexToAscii(PHY_CH[i].ErrCode / 10 ) ;     // 10進上位桁
            pos++ ;
            Tx_Buff[pos] = HexToAscii(PHY_CH[i].ErrCode % 10 ) ;     // 10進下位桁
            pos++ ;
            Tx_Buff[pos] = ',' ;
            pos++ ;
        }
    }
    data_calcul_sum( &Tx_Buff[0], &Tx_Buff[pos], pos) ;
    pos++ ;
    pos++ ;
    Tx_Buff[pos] = '\r' ;
    pos++ ;
    Tx_Buff[pos] = '\n' ;
    pos++ ;
    tx_size = pos  ;
}

unsigned char AsciiToBool(unsigned char data)
{
    unsigned char rc = 0xFF;

    switch(data)
    {
        case '0':
            rc = 0 ;
            break ;
        
        case '1':
            rc = 1 ;
            break ;
        
        case '*':
            rc = 2 ;
            break ;
        
        default:
            rc = 0xFF ;
            break ;
    }    
    return(rc) ;
}

void data_calcul_sum(unsigned char *buff,unsigned char *sumbuff,unsigned int size)
{
    unsigned char sum = 0;
    unsigned int i = 0;
    
    for(i = 0; i < size; i++)
    {
        sum += buff[i];
    }
    *sumbuff = HexToAscii((sum >> 4) & 0x0F);
    *(sumbuff+1) = HexToAscii(sum & 0x0F);                
}

/* '0' ~ 'F' => 0x00 ~ 0x0F*/
unsigned char AsciiToHex(unsigned char num)
{
   unsigned char rc = 0xFF;

   if(0x30 <= num && num <= 0x39)
   {
      rc = num - 0x30; // 0-9
   }
   else if(0x41 <= num && num <= 0x46)
   {
      rc = num - 0x37; //A-F
   }
   return rc;
}

/* 0x00 ~ 0x0F => '0' ~ 'F' */
unsigned char HexToAscii(unsigned char hex)
{
	unsigned char i;
	hex = hex & 0x0F;
	if (hex < 0x0A)
		i = hex | 0x30;
	else
		i = hex + 0x37;
	return i;
}

unsigned char AsciiToHex_Byte(unsigned char asc1, unsigned char asc0)
{
	unsigned char i;
	i = (AsciiToHex(asc1) << 4) + (AsciiToHex(asc0) & 0x0F);
	return i;
}

unsigned char DecimalToAscii(unsigned int dec, unsigned char *str)	//Decimal conversion to string
{
	unsigned char i = 0;
	unsigned char dec_str[10] = {0};
	unsigned int decnum = 0;
	unsigned char len = 0;
	
	if(dec == 0)
	{
		*str = '0';
		return 1;
	}
	decnum = dec / 10;
	while(decnum != 0)
	{
		dec_str[len++] = dec % 10 + '0';
		dec = decnum;
		decnum = dec / 10;
	}
	if(dec != 0) dec_str[len++] = dec + '0';
	for(i=0;i<len;i++)
	{
		*str = dec_str[len-i-1];
		str++;
	}
	return len;
}
