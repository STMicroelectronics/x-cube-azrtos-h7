/**
  ******************************************************************************
  * @file    stm32_fx_levelx_driver_template.h
  * @author  MCD Application Team
  * @brief   Header for stm32_fx_levelx_driver.c module. To be renamed and copied
             under the application source tree.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020 STMicroelectronics. All rights reserved.
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                       opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
**/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef FX_STM32_LX_NOR_DRIVER_H
#define FX_STM32_LX_NOR_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/*--------------- Includes ---------------*/
#include "fx_api.h"
#include "lx_api.h"

#define LX_NOR_OSPI_DRIVER

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

#ifdef LX_NOR_QSPI_DRIVER
#include "lx_stm32_qspi_driver.h"

#define LX_NOR_QSPI_DRIVER_ID           0x03
#define LX_NOR_QSPI_DRIVER_NAME         "FX Levelx QuadSPI driver"
#endif

#ifdef LX_NOR_USE_CUSTOM_DRIVER
/*
 * define the Custom levelx nor drivers to be supported by the filex
 *  for example

#define CUSTOM_DRIVER_ID          0xFFFFFF
#define NOR_CUSTOM_DRIVER_NAME    "NOR CUSTOM DRIVER"
#include "lx_nor_custom_driver.h"
#define NOR_CUSTOM_DRIVERS   {"NOR CUSTOM DRIVER 1", NOR_CUSTOM_DRIVER_1, lx_nor_customer_driver1_initialize}
 */
#define LX_NOR_CUSTOM_DRIVERS
#endif

#define MAX_LX_NOR_DRIVERS     8
#define UNKNOWN_DRIVER_ID      0xFFFFFFFF

#define USE_LX_NOR_DEFAULT_DRIVER

#ifdef USE_LX_NOR_DEFAULT_DRIVER
#define NOR_DEFAULT_DRIVER     LX_NOR_OSPI_DRIVER_ID
#endif

VOID  fx_stm32_levelx_nor_driver(FX_MEDIA *media_ptr);

#ifdef __cplusplus
}
#endif

#endif /*FX_STM32_LX_NOR_DRIVER_H*/
