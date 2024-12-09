/**
  ******************************************************************************
  * @file    flash_if.h
  * @author  MCD Application Team
  * @brief   Header for flash_if.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_IF_H
#define __FLASH_IF_H

/* Includes ------------------------------------------------------------------*/
#include <inttypes.h>

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
extern const uint32_t USER_FLASH_START_ADDRESS;

/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void FLASH_If_Init(void);
int32_t FLASH_If_Erase(uint32_t address);
uint32_t FLASH_If_Write(uint32_t flash_address, const uint8_t *data, uint32_t data_length);

#endif /* __FLASH_IF_H */
