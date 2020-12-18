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

#ifndef FX_STM32_DRIVER_H
#define FX_STM32_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fx_api.h"

/*
 * Add driver specific defines
 */

VOID fx_stm32_driver(FX_MEDIA *media_ptr);

#ifdef __cplusplus
}
#endif
#endif /* FX_STM32_DRIVER_H */
