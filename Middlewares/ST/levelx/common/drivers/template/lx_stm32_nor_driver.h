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

#ifndef LX_STM32_NOR_DRIVER_H
#define LX_STM32_NOR_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lx_api.h"

/* Define the function prototypes of the LevelX driver entry function.  */
UINT  lx_stm32_nor_initialize(LX_NOR_FLASH *nor_flash);

#ifdef __cplusplus
}
#endif
#endif /* LX_STM32_NOR_DRIVER_H */

