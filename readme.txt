git@github.com:Git-Wyb/UVLED_CS32F103RB_Pro.git

2026.09.02
优化，添加按键扫描，按键切换通道。
串口初始化函数会引起UVLED通道2异常，暂时先屏蔽串口，后续再解决。

2026.09.10  UVLED_ADC_ERRCHECKE02.hex，UVLED过流/欠流检测改为ADC DMA中断。