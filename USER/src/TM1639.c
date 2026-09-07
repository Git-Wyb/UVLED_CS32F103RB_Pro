#include "TM1639.h"

u16 Led_Sta = 0;
/* ====== 共阴 7 段码表（与 TM1639 段映射直接对应） ====== */
/* 索引: 0~9, A~F, -, 空格.  bit0=a, bit1=b, bit2=c, bit3=d, bit4=e, bit5=f, bit6=g, bit7=dp */
static const uint8_t TM1639_Font[] = {
    0x3F, /* 0 */
    0x06, /* 1 */
    0x5B, /* 2 */
    0x4F, /* 3 */
    0x66, /* 4 */
    0x6D, /* 5 */
    0x7D, /* 6 */
    0x07, /* 7 */
    0x7F, /* 8 */
    0x6F, /* 9 */
    0x77, /* A */
    0x7C, /* b */
    0x39, /* C */
    0x5E, /* d */
    0x79, /* E */
    0x71, /* F */
    0x40, /* - */
    0x00, /* 空格(blank) */
};
/*
 ——a——
|     |
f     b
|     |
 ——g——
|     |
e     c
|     |
 ——d—— dp(point)
bit0 - bit3 => a b c d; bit4 - bit7 => e f g dp*/
static const uint8_t UVLED_Font[][3] = {
    0xFF,0xFF,0xFF, //8.8.8 //0
    0x79,0x04,0x54, //Ein
    0x73,0x50,0x04, //Pri
    0x7C,0x1C,0x50, //bur
    0x5C,0x73,0x38, //oPL
    0x66,0x3E,0x74, //4Uh  //5
    0x5B,0x4F,0x5B, //232
    0x38,0x79,0x3E, //LEU
    0x73,0x1C,0x38, //PuL
    0x78,0x04,0x54, //ein
    0x6D,0x04,0x3D, //SiG  //10
    0x00,0x5C,0x54, // on
    0x5C,0x71,0x71, //oFF
    0x00,0x6F,0x7D, // 96
    0x06,0x6F,0x5B, //192
    0x4F,0x7F,0x66, //384  //15
    0x39,0x76,0X06, //CH1  //16
    0x39,0x76,0x5B, //CH2
    0x39,0x76,0x4F, //CH3
    0x39,0x76,0x66, //CH4  //19
    0x77,0x38,0x38  //ALL  //20
};

/* 简单微秒延时（72MHz 系统时钟下粗延时） */
static void TM1639_DelayUs(uint16_t us)
{
    volatile uint16_t i;
    while (us--)
        for (i = 0; i < 80; i++);
}

/* ====== 底层：发送一个字节（LSB first） ====== */
static void TM1639_SendByte(uint8_t dat)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        TM1639_CLK_L();
        TM1639_DelayUs(1);
        if (dat & 0x01)
        {
            TM1639_DIO_H();
        }
        else
        {
            TM1639_DIO_L();
        }
        dat >>= 1;
        TM1639_DelayUs(1);
        TM1639_CLK_H();          /* 上升沿锁存数据 */
        TM1639_DelayUs(2);
    }
}

/* ====== 发送命令 ====== */
static void TM1639_SendCmd(uint8_t cmd)
{
    TM1639_STB_L();
    TM1639_DelayUs(2);
    TM1639_SendByte(cmd);
    TM1639_STB_H();
    TM1639_DelayUs(2);
}

/* ====== 初始化 ====== */
void Init_TM1639(void)
{
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOC);

    gpio_mode_config(TM1639_STB_PORT, TM1639_STB_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    gpio_mode_config(TM1639_CLK_PORT, TM1639_CLK_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    gpio_mode_config(TM1639_DIO_PORT, TM1639_DIO_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    
    /* 初始状态 */
    TM1639_STB_H();
    TM1639_CLK_H();
    TM1639_DIO_H();
    TM1639_DelayUs(10);

    /* 上电清零显存（必须，否则可能乱码） */
    TM1639_Clear();

    /* 开显示，中等亮度 */
    TM1639_SetBrightness(5);
}

/* ====== 清屏 ====== */
void TM1639_Clear(void)
{
    uint8_t i;
    /* 数据命令：写显示寄存器 + 地址自动递增 (0x40) */
    TM1639_SendCmd(0x40);
    /* 起始地址 0x00 */
    TM1639_STB_L();
    TM1639_SendByte(0xC0);
    /* 连续写 16 字节清零（00H~0FH） */
    for (i = 0; i < 16; i++)
        TM1639_SendByte(0x00);
    TM1639_STB_H();
    TM1639_DelayUs(1);
}

/* ====== 设置亮度 ====== */
/* level: 0~7, 对应占空比 1/16 ~ 14/16; 显示开关在 bit3 */
void TM1639_SetBrightness(uint8_t level)
{
    if (level > 7) level = 7;
    TM1639_SendCmd(0x88 | level);   /* 0x88 = 显示开 + 亮度0 */
}

/* ====== 向指定地址写一个字节 ====== */
void TM1639_WriteByte(uint8_t addr, uint8_t data)
{
    /* 固定地址模式 (0x44) */
    TM1639_SendCmd(0x44);
    /* 地址命令 + 数据 */
    TM1639_STB_L();
    TM1639_SendByte(0xC0 | (addr & 0x0F));
    TM1639_SendByte(data);
    TM1639_STB_H();
    TM1639_DelayUs(1);
}

void TM1639_WriteByte40(uint8_t addr, uint8_t dat)
{
    TM1639_SendCmd(0x40);                       /* 写+自动地址递增（和 Clear 同一个模式） */
    TM1639_STB_L();
    TM1639_DelayUs(10);
    TM1639_SendByte(0xC0 | (addr & 0x0F));      /* 起始地址 */
    TM1639_SendByte(dat);                       /* 只写 1 个字节 */
    TM1639_DelayUs(10);
    TM1639_STB_H();
    TM1639_DelayUs(10);
}

void TM1639_DisplayChar(uint8_t pos, char ch, uint8_t point)
{
    uint8_t seg, idx;
    if(point > 1) point = 1;
    
    if (pos > 2) return;

    if (ch >= '0' && ch <= '9')
        idx = ch - '0';
    else if (ch >= 'A' && ch <= 'F')
        idx = ch - 'A' + 10;
    else if (ch >= 'a' && ch <= 'f')
        idx = ch - 'a' + 10;
    else if (ch == '-')
        idx = 16;
    else
        idx = 17;  /* 空格 */

    seg = TM1639_Font[idx] | (point << 7);
    
    uint8_t addr = pos * 2;      /* 每位两个地址：addr=低字节, addr+1=高字节 */
    uint8_t lo = seg & 0x0F;       /* 低 4 位 → SEG1~4 = a~d */
    uint8_t hi = (seg >> 4) & 0x0F;/* 高 4 位 → SEG9~12 = e,f,g,dp */

    TM1639_SendCmd(0x40);               /* 自动递增，一次写两个字节 */
    TM1639_STB_L();
    TM1639_DelayUs(10);
    TM1639_SendByte(0xC0 | addr);       /* 起始地址 */
    TM1639_SendByte(lo);                /* 低字节：a~d */
    TM1639_SendByte(hi);                /* 高字节：e,f,g,dp */
    TM1639_DelayUs(10);
    TM1639_STB_H();
    TM1639_DelayUs(10);
}

void TM1639_DisplayChar_Num(uint8_t pos, char ch)
{
    uint8_t addr = pos * 2;      /* 每位两个地址：addr=低字节, addr+1=高字节 */
    uint8_t lo = ch & 0x0F;       /* 低 4 位 → SEG1~4 = a~d */
    uint8_t hi = (ch >> 4) & 0x0F;/* 高 4 位 → SEG9~12 = e,f,g,dp */

    TM1639_SendCmd(0x40);               /* 自动递增，一次写两个字节 */
    TM1639_STB_L();
    TM1639_DelayUs(10);
    TM1639_SendByte(0xC0 | addr);       /* 起始地址 */
    TM1639_SendByte(lo);                /* 低字节：a~d */
    TM1639_SendByte(hi);                /* 高字节：e,f,g,dp */
    TM1639_DelayUs(10);
    TM1639_STB_H();
    TM1639_DelayUs(10);
}
/**
 * @brief  显示 0~999 整数（3位，前导零消隐）
 */
void TM1639_DisplayNum(uint16_t num)
{
    uint8_t h, t, o;
    if (num > 999) num = 999;

    h = num / 100;
    t = (num / 10) % 10;
    o = num % 10;

    /* 前导零消隐 */
    if (h == 0) {
        TM1639_DisplayChar_Num(0, 0x00);  /* 空格 */
        if (t == 0)
            TM1639_DisplayChar_Num(1, 0x00);
        else
            TM1639_DisplayChar_Num(1, TM1639_Font[t]);
    } else {
        TM1639_DisplayChar_Num(0, TM1639_Font[h]);
        TM1639_DisplayChar_Num(1, TM1639_Font[t]);
    }
    TM1639_DisplayChar_Num(2, TM1639_Font[o]);
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

void TM1639_DisplayUVtime(NUMSTU uvtime)
{
    if(uvtime.hundreds == 0) TM1639_DisplayChar(DISPLAY_ADDR0,' ',0);
    else
    {
        uvtime.hundreds = uvtime.hundreds + '0';
        TM1639_DisplayChar(DISPLAY_ADDR0,uvtime.hundreds,uvtime.point0);
    }
    
    if(uvtime.decde == 0) TM1639_DisplayChar(DISPLAY_ADDR1,' ',0);
    else
    {
        uvtime.decde = uvtime.decde + '0';
        TM1639_DisplayChar(DISPLAY_ADDR1,uvtime.decde,uvtime.point1);
    }
    
    if(uvtime.unit == 0) TM1639_DisplayChar(DISPLAY_ADDR2,' ',0);
    else
    {
        uvtime.unit = uvtime.unit + '0';
        TM1639_DisplayChar(DISPLAY_ADDR2,uvtime.unit,uvtime.point2);
    }
}

static uint8_t ReadByte(void)
{
    uint8_t i, dat = 0;
    for (i = 0; i < 8; i++)
    {
        dat >>= 1;
        TM1639_CLK_L();
        TM1639_DelayUs(2);          /* 等待数据稳定 */
        if (TM1639_DIO_R())
            dat |= 0x80;     /* 高位先入，最后右移得到 LSB 优先 */
        TM1639_CLK_H();
        TM1639_DelayUs(2);
    }
    return dat;
}

/* ========== 新增：读取按键状态 ========== */
/**
 * @brief  读取 8 个按键状态
 * @return 按键值，每一位对应一个按键：1=按下，0=松开
 *         bit0~bit3 = SEG1~SEG4 & K1
 *         bit4~bit7 = SEG1~SEG4 & K2
 */

uint8_t TM1639_Read_Key(void)
{
    uint8_t byte_k1, byte_k2;
    
    /* 1. 发送读键命令 */
    TM1639_STB_L();
    TM1639_DelayUs(2);
    TM1639_SendByte(0x42); /* 读按键数据命令 */
    TM1639_DelayUs(2);

    /* 2. DIO 切换为上拉输入 */
    TM1639_DIO_IN();
    TM1639_DelayUs(5);

    /* 3. 读取 2 字节按键数据 */
    byte_k1 = ReadByte();  /* 第1字节：K1 组（SEG1~SEG4） */
    byte_k2 = ReadByte();  /* 第2字节：K2 组（SEG1~SEG4） */

    /* 4. STB 拉高，DIO 切回推挽输出 */
    TM1639_STB_H();
    TM1639_DelayUs(10);
    
    TM1639_DIO_OUT();

    /* 5. 整理按键值：低4位=K1组，高4位=K2组 */
    //key_val = (byte_k1 & 0x0F) | ((byte_k2 & 0x0F) << 4);
    if(byte_k1 & 0x04)  KeySta.Up = 1;
    else KeySta.Up = 0;
    
    if(byte_k2 & 0x40)  KeySta.Down = 1;
    else KeySta.Down = 0;
    
    if(byte_k1 & 0x40)  KeySta.Bk = 1;
    else KeySta.Bk = 0;
    
    if(byte_k2 & 0x04)  KeySta.Fw = 1;
    else KeySta.Fw = 0;
    
    if(byte_k2 & 0x08)  KeySta.Set = 1;
    else KeySta.Set = 0;
    
    if(byte_k1 & 0x08)  KeySta.Mode = 1;
    else KeySta.Mode = 0;
    
    if(byte_k1 & 0x80)  KeySta.Ch = 1;
    else KeySta.Ch = 0;
    
    if(byte_k2 & 0x80)  KeySta.Uvon = 1;
    else KeySta.Uvon = 0;
    
    return KeySta.Byte;
}


void TM1639_LED_switch(LED_ENUM lednum,u8 onoff)
{
    u8 addr = 0x00,val = 0x00;

    if(onoff) Led_Sta = Led_Sta | (0x01 << lednum);
    else      Led_Sta = Led_Sta & (~(0x01 << lednum));
    
    /* 计算对应地址和位：0~3 → GRID4(0x06)，4~7 → GRID5(0x08),GRID6(0x0A) */
    if(lednum < 4) 
    {
        addr = 0x06;
        val = Led_Sta & 0x0F;
    }
    else if(lednum < 8)
    {
        addr = 0x08;
        val = (Led_Sta >> 4) & 0x0F;
    }
    else
    {
        addr = 0x0A;
        val = (Led_Sta >> 8) & 0x0F;
    }
    
    TM1639_WriteByte(addr,val);
}

void CH_LED_switch(u8 ledch,u8 err,u8 onoff)
{
    switch(ledch)
    {
        case 0:
            if(err == 0)
            {
                TM1639_LED_switch(LED_CH1_GREEN,onoff);
                TM1639_LED_switch(LED_CH1_RED,0);
            }
            else 
            {
                TM1639_LED_switch(LED_CH1_RED,onoff);
                TM1639_LED_switch(LED_CH1_GREEN,0);
            }
            break;
        
        case 1:
            if(err == 0) 
            {
                TM1639_LED_switch(LED_CH2_GREEN,onoff);
                TM1639_LED_switch(LED_CH2_RED,0);
            }
            else
            {
                TM1639_LED_switch(LED_CH2_RED,onoff);
                TM1639_LED_switch(LED_CH2_GREEN,0);
            }
            break;
        
        case 2:
            if(err == 0)
            {
                TM1639_LED_switch(LED_CH3_GREEN,onoff);
                TM1639_LED_switch(LED_CH3_RED,0);
            }
            else 
            {
                TM1639_LED_switch(LED_CH3_RED,onoff);
                TM1639_LED_switch(LED_CH3_GREEN,0);
            }
            break;
        
        case 3:
            if(err == 0) 
            {
                TM1639_LED_switch(LED_CH4_GREEN,onoff);
                TM1639_LED_switch(LED_CH4_RED,0);
            }
            else
            {
                TM1639_LED_switch(LED_CH4_RED,onoff);
                TM1639_LED_switch(LED_CH4_GREEN,0);
            }
            break;
        
        default:
            TM1639_LED_switch(LED_CH1_GREEN,0);
            TM1639_LED_switch(LED_CH1_RED,0);
            TM1639_LED_switch(LED_CH2_GREEN,0);
            TM1639_LED_switch(LED_CH2_RED,0);
            TM1639_LED_switch(LED_CH3_GREEN,0);
            TM1639_LED_switch(LED_CH3_RED,0);
            TM1639_LED_switch(LED_CH4_GREEN,0);
            TM1639_LED_switch(LED_CH4_RED,0);
            break;
    }
}
    
void TM1639_Display_UVLED_Char(DISPLAY_ENUM pos)
{
    uint8_t seg = 0,i = 0;
    if(pos > DISPLAY_ALL) pos = DISPLAY_ALL;
    
    for(i = 0; i < 3; i++)
    {
        seg = UVLED_Font[pos][i];
        uint8_t addr = i * 2;      /* 每位两个地址：addr=低字节, addr+1=高字节 */
        uint8_t lo = seg & 0x0F;       /* 低 4 位 → SEG1~4 = a~d */
        uint8_t hi = (seg >> 4) & 0x0F;/* 高 4 位 → SEG9~12 = e,f,g,dp */

        TM1639_SendCmd(0x40);               /* 自动递增，一次写两个字节 */
        TM1639_STB_L();
        TM1639_DelayUs(10);
        TM1639_SendByte(0xC0 | addr);       /* 起始地址 */
        TM1639_SendByte(lo);                /* 低字节：a~d */
        TM1639_SendByte(hi);                /* 高字节：e,f,g,dp */
        TM1639_DelayUs(10);
        TM1639_STB_H();
        TM1639_DelayUs(10);
    }
}

void Display_UVLED_Mode(DISPLAY_ENUM dis)
{
    switch(dis)
    {
        case DISPLAY_Ein:
            TM1639_Display_UVLED_Char(DISPLAY_Ein);
            break;
    }
}
#if 0
void TM1639_SetDP(uint8_t pos, u8 state)
{
    uint8_t addr = pos * 2 + 1;  // 定位到该位的高地址
    if (addr >= 16) return;       // 地址越界保护

    if (state == 1)
        disp_buf[addr] |=  (1 << 3);  // dp对应bit3，置1点亮
    else
        disp_buf[addr] &= ~(1 << 3);  // 清0熄灭

    TM1639_Refresh();  // 刷新到芯片
}

void TM1639_Refresh(void)
{
    uint8_t i;
    TM1639_SendCmd(0x40);               /* 自动地址递增 */
    TM1639_STB_L();
    TM1639_DelayUs(10);
    TM1639_SendByte(0xC0);              /* 起始地址 0x00 */
    for (i = 0; i < 16; i++)    /* 连续写入16字节缓存 */
        TM1639_SendByte(disp_buf[i]);
    TM1639_DelayUs(10);
    TM1639_STB_H();
    TM1639_DelayUs(10);
}

void TM1639_DisplayFloat(float val)
{
    int32_t int_num;
    uint8_t dig0, dig1, dig2;
    int8_t dp_pos = -1;  // -1=不点亮小数点

    /* 输入范围钳位 */
    if (val < 0.0f) val = 0.0f;
    if (val > 999.9f) val = 999.9f;

    /* 放大10倍转整数 + 四舍五入，彻底避免浮点误差 */
    int_num = (int32_t)(val * 10.0f + 0.5f);

    /* 拆分数字 + 确定小数点位置 */
    if (int_num >= 1000)          /* ≥100：三位整数，无小数 */
    {
        dig0 = (uint8_t)(int_num / 1000);
        dig1 = (uint8_t)((int_num / 100) % 10);
        dig2 = (uint8_t)((int_num / 10) % 10);
        dp_pos = -1;
    }
    else if (int_num >= 100)      /* 10~99.9：两位整数 + 一位小数 */
    {
        dig0 = (uint8_t)((int_num / 100) % 10);
        dig1 = (uint8_t)((int_num / 10) % 10);
        dig2 = (uint8_t)(int_num % 10);
        dp_pos = 1;  // 小数点在第1位（中间位）右下角
    }
    else                          /* 0.0~9.9：一位整数 + 一位小数 */
    {
        dig0 = (uint8_t)(int_num / 10);
        dig1 = (uint8_t)(int_num % 10);
        dig2 = 0xFF; // 标记第三位熄灭
        dp_pos = 0;  // 小数点在第0位（最左位）右下角
    }

    /* 写入显示缓存 */
    TM1639_DisplayChar(0, seg_table[dig0]);
    TM1639_DisplayChar(1, seg_table[dig1]);
    
    if (dig2 == 0xFF)
        TM1639_DisplayChar(2, 0x00);  // 第三位熄灭
    else
        TM1639_DisplayChar(2, seg_table[dig2]);

    /* 设置小数点状态 */
    TM1639_SetDP(0, (dp_pos == 0) ? ENABLE : DISABLE);
    TM1639_SetDP(1, (dp_pos == 1) ? ENABLE : DISABLE);
    TM1639_SetDP(2, DISABLE);

    /* 一次性刷新生效 */
    TM1639_Refresh();
}
#endif
