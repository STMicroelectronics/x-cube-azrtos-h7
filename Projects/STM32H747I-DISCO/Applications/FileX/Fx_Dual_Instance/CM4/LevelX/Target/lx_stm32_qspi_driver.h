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

#ifndef LX_STM32_QSPI_DRIVER_H
#define LX_STM32_QSPI_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "lx_api.h"

#include "stm32h747i_discovery_qspi.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* the QuadSPI instance ,default value set to 0 */
#define QSPI_INSTANCE                       0

#define DEFAULT_BLOCK_SIZE                  (4 * 1024)

/* when set to 1, the QSPI init is done by the driver, otherwise it is up to the application to initialize it */
#define LX_DRIVER_CALLS_QSPI_INIT           1

#if (LX_DRIVER_CALLS_QSPI_INIT == 1)

/* allow the driver to fully erase the QuadSPI chip. This should be used carefully.
 * the call is blocking and takes a while. by default it is set to 0.
 * When creating a fresh file system on the NOR Flash this flag should be enabled.
 */
#define LX_DRIVER_ERASES_QSPI_AFTER_INIT    1
#endif

/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
UINT  lx_stm32_qspi_initialize(LX_NOR_FLASH *nor_flash);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* LX_STM32_QSPI_DRIVER_H */
