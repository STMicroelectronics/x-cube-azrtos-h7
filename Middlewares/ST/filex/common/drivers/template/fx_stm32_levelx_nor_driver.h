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

#ifndef FX_STM32_LX_NOR_DRIVER_H
#define FX_STM32_LX_NOR_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/*--------------- Includes ---------------*/
#include "fx_api.h"
#include "lx_api.h"

//#define LX_NOR_OSPI_DRIVER
//#define LX_NOR_SIMULATOR_DRIVER
//#define USE_LX_NOR_CUSTOM_DRIVER

#ifdef LX_NOR_SIMULATOR_DRIVER
#include "lx_stm32_nor_simulator_driver.h"

#define LX_NOR_SIMULATOR_DRIVER_ID        0x01
#define LX_NOR_SIMULATOR_DRIVER_NAME      "FX Levelx NOR flash Simulator"
#endif

#ifdef LX_NOR_OSPI_DRIVER
#include "lx_stm32_ospi_driver.h"

#define LX_NOR_OSPI_DRIVER_ID            0x02
#define LX_NOR_OSPI_DRIVER_NAME          "FX Levelx OctoSPI driver"
#endif

/* uncomment the define below to support custom drivers */
/* #define USE_LX_NOR_CUSTOM_DRIVER */

#ifdef USE_LX_NOR_CUSTOM_DRIVER
/*
 * define the Custom levelx nor drivers to be supported by the filex
 * for example:

#define CUSTOM_DRIVER_ID          0x10 <-- MUST BE DIFFRENT TO 0 
#define NOR_CUSTOM_DRIVER_NAME    "NOR CUSTOM DRIVER"
#include "lx_nor_custom_driver.h"
                                "driver name"          "driver id"               "levelx entry function"
#define NOR_CUSTOM_DRIVERS   {NOR_CUSTOM_DRIVER_NAME, CUSTOM_DRIVER_ID, lx_nor_customer_driver_initialize}
 */
#define LX_NOR_CUSTOM_DRIVERS
#endif

#define MAX_LX_NOR_DRIVERS     8
#define UNKNOWN_DRIVER_ID      0xDEADBEEF
/* uncomment the define below to force the usage of a single NOR driver */
#define USE_LX_NOR_DEFAULT_DRIVER

/* when using a single driver define the NOR_DEFAULT_DRIVER_ID with the one of the above defined drivers ID */
// #define NOR_DEFAULT_DRIVER  LX_NOR_SIMULATOR_DRIVER_ID

/* main FileX driver entry function */
VOID  fx_stm32_levelx_nor_driver(FX_MEDIA *media_ptr);

#ifdef __cplusplus
}
#endif

#endif /*FX_STM32_LX_NOR_DRIVER_H*/
