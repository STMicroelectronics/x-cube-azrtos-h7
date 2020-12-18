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

#ifndef LX_STM32_OSPI_DRIVER_H
#define LX_STM32_OSPI_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------> Includes <-----------*/
#include "lx_api.h"
#include "stm32h7xxx_{eval,discovery}_ospi.h"

/*-----------> Defines <-----------*/

/* the OctoSPI instance ,defaut value set to 0 */
#define OSPI_INSTANCE   0

/* when set to 1, the BSP_OSPI_Init() is called by the driver, otherwise it is up to the application to intialize it */
#define LX_DRIVER_CALLS_BSP_OSPI_INIT 1

#if (LX_DRIVER_CALLS_BSP_OSPI_INIT == 1)

/* allow the driver to fully erase the OctoSPI chip. This should be used carefully.
 * the call is blocking and takes a while. by default it is set to 0.
 */
#define LX_DRIVER_ERASES_OPSI_AFTER_INIT  0
#endif

UINT  lx_stm32_ospi_initialize(LX_NOR_FLASH *nor_flash);

#ifdef __cplusplus
}
#endif
#endif /* LX_STM32_OSPI_DRIVER_H */

