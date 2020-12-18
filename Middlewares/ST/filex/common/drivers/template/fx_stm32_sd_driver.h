/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/
#ifndef FX_STM32_SD_DRIVER_H
#define FX_STM32_SD_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/*--------------- Includes ---------------*/
#include "fx_api.h"
#include "stm32xxxx_{discovery,eval}_sd.h"
#endif

#define DEFAULT_TIMEOUT                        (10 * TX_TIMER_TICKS_PER_SECOND)
#define SD_INSTANCE                             0

#define DEFAULT_SECTOR_SIZE                     512
#define ENABLE_CACHE_MAINTENANCE                1

#define FX_DRIVER_CALLS_BSP_SD_INIT             0

VOID  fx_stm32_sd_driver(FX_MEDIA *media_ptr);

#ifdef __cplusplus
}
#endif

#endif /* FX_STM32_SD_DRIVER_H */
