/**
  ******************************************************************************
  * @file    fx_stm32_sram_driver_template.h
  * @author  MCD Application Team
  * @brief   stm32 FileX sram driver header filet template.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
#ifndef FX_STM32_SRAM_DRIVER_H
#define FX_STM32_SRAM_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/*------- Includes -------*/

#include "fx_api.h"

/* include the stm32h7xx.h to be able to access the memory region defines */
#include "stm32h7xx.h"

/* define the FileX SRAM DISK base address */

#define FX_SRAM_DISK_BASE_ADDRESS         D1_AXISRAM1_BASE

/* define the SRAM DISK size */
#define FX_SRAM_DISK_SIZE                 (128 * 1024)

VOID fx_stm32_ram_driver(FX_MEDIA *media_ptr);

#ifdef __cplusplus
}
#endif
#endif /* FX_STM32_SRAM_DRIVER_H */
