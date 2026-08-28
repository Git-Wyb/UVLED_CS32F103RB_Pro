/***************************************************************************//**
 * @file        system_cs32f10x.c
 * @version     V2.0.1
 * @author      Software Development
 * @brief       This file contains system clock init functions.
 *              
 * @copyright   Copyright (C) Software Development. All rights reserved.
 ****************************************************************************/

#ifndef __SYSTEM_CS32F10X_H
#define __SYSTEM_CS32F10X_H

#ifdef __cplusplus
 extern "C" {
#endif 

/** @addtogroup SYSTEM
  *
  */




extern uint32_t SystemCoreClock;          /*!< System Clock Frequency (Core Clock) */
extern void SystemInit(void);
extern void SystemCoreClockUpdate(void);


#ifdef __cplusplus
}
#endif

#endif /*__SYSTEM_CS32F10X_H */

/******************* (C) COPYRIGHT 2021 ChipSea *****END OF FILE****/
