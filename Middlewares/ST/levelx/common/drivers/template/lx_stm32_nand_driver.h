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

#ifndef LX_STM32_NAND_DRIVER_H
#define LX_STM32_NAND_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "lx_api.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
/* Define the flag for format and erase NAND flash memory */
/* #define LX_DRIVER_ERASES_FLASH_AFTER_INIT */

/* Define constants for the NAND flash custom. */
#error "[This error was thrown on purpose] : define the flags below related to the NAND Flash memory used"

#define TOTAL_BLOCKS                        0             /* Number of total blocks */
#define BYTES_PER_PHYSICAL_PAGE             0             /* Number of bytes per page */
#define SPARE_BYTES_PER_PAGE                0             /* Number of spare bytes per page */
#define PHYSICAL_PAGES_PER_BLOCK            0             /* Number of physical pages per block minimum 2 */


#define SPARE_DATA1_OFFSET                  0             /* Offset of spare data 1 */
#define SPARE_DATA1_LENGTH                  0             /* length of spare data 1 */
#define SPARE_DATA2_OFFSET                  0             /* Offset of spare data 2 */
#define SPARE_DATA2_LENGTH                  0             /* length of spare data 2 */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

UINT  lx_stm32_nand_custom_driver_initialize(LX_NAND_FLASH *nand_flash);

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
#endif /* LX_STM32_NOR_DRIVER_H */

