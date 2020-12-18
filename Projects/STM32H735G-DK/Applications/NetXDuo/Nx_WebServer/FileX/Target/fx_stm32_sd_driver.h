/**
  ******************************************************************************
  * @file    fx_stm32_sd_driver.h
  * @author  MCD Application Team
  * @brief   stm32 FileX sd driver header file.
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
#ifndef FX_STM32_SD_DRIVER_H
#define FX_STM32_SD_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/*--------------- Includes ---------------*/
#include "fx_api.h"
#include "stm32h735g_discovery_sd.h"
#if (USE_SD_TRANSCEIVER == 1U)
#error "USE_SD_TRANSCEIVER should be set to 0U in the stm32h7xx_hal_conf.h"
#endif

#define DEFAULT_TIMEOUT                        (10 * TX_TIMER_TICKS_PER_SECOND)
#define SD_INSTANCE                             0

#define DEFAULT_SECTOR_SIZE                     512
#define ENABLE_CACHE_MAINTENANCE                1

#define FX_DRIVER_CALLS_BSP_SD_INIT             1

VOID  fx_stm32_sd_driver(FX_MEDIA *media_ptr);

#ifdef __cplusplus
}
#endif

#endif /* FX_STM32_SD_DRIVER_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
