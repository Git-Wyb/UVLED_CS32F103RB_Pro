/***************************************************************************//**
 * @file        cs32f10x_it.c
 * @version     V2.0.1
 * @author      Software Development
 * @brief       This file provides all the interrupt handle functions.
 *              
 * @copyright   Copyright (C) Software Development. All rights reserved.
 ****************************************************************************/
 
#include "cs32f10x_it.h"
#include <stdio.h>
#include "cs32f10x_adc.h"

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/


/**@brief       This function handles NMI exception.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void NMI_Handler(void)
{
    /* User code */
}

/**@brief       This function handles Hard Fault exception.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void HardFault_Handler(void)
{
  while (1)
  {
      /* User code */
  }
}

/**@brief       This function handles Memory Manage exception.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void MemManage_Handler(void)
{
  while (1)
  {
      /* User code */
  }
}

/**@brief       This function handles Bus Fault exception.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void BusFault_Handler(void)
{
  while (1)
  {
      /* User code */
  }
}

/**@brief       This function handles Usage Fault exception.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void UsageFault_Handler(void)
{
  while (1)
  {
      /* User code */
  }
}

/**@brief       This function handles SVCall exception.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void SVC_Handler(void)
{
    /* User code */
}

/**@brief       This function handles Debug Monitor exception.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void DebugMon_Handler(void)
{
    /* User code */
}

/**@brief       This function handles PendSVC exception.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void PendSV_Handler(void)
{
    /* User code */
}

/**@brief       This function handles SysTick exception.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void SysTick_Handler(void)
{
    /* User code */
}

/******************************************************************************/
/*                 CS32F10x Peripherals Interrupt Handlers                   */
/******************************************************************************/

/**
  * @brief  This function handles ADC interrupt request.
  * @param  None
  * @retval None
  */
void ADC1_2_IRQHandler(void)
{
    printf("Voltage value is out of range!\r\n");
    __ADC_FLAG_CLEAR(ADC1, ADC_FLAG_ANALOG_WDT);
}

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2021 ChipSea *****END OF FILE****/
