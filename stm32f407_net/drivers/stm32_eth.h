/**
  ******************************************************************************
  * @file    stm32_eth.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    11/20/2009
  * @brief   This file contains all the functions prototypes for the Ethernet
  *          firmware library.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_ETH_H
#define __STM32_ETH_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4x7_eth.h"
#include "stm32f4xx_gpio.h" 
#include "stm32f4xx_dma.h" 
#include "stm32f4xx_rcc.h" 
	 
/** @addtogroup STM32_ETH_Driver
  * @{
  */

/** @defgroup ETH_Exported_Types
  * @{
  */

/**
  * @brief  ETH MAC Init structure definition
  * @note   The user should not configure all the ETH_InitTypeDef structure's fields.
  *   By calling the ETH_StructInit function the structure¡¯s fields are set to their default values.
  *   Only the parameters that will be set to a non-default value should be configured.
  */
void rt_hw_stm32_eth_init(void);
#ifdef __cplusplus
}
#endif



#endif /* __STM32_ETH_H */
/**
  * @}
  */


/**
  * @}
  */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
