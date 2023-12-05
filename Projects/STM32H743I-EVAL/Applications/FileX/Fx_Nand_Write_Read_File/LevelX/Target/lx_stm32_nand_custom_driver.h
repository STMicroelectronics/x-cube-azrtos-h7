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
#include "stm32h743i_eval_fmc_nand.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
/* Define constants for the NAND flash simulation. */
#define TOTAL_BLOCKS                        2048
#define PHYSICAL_PAGES_PER_BLOCK            64                                  /* Min value of 2                                              */
#define BYTES_PER_PHYSICAL_PAGE             2048                                /* 2048 bytes per page                                         */
#define WORDS_PER_PHYSICAL_PAGE             BYTES_PER_PHYSICAL_PAGE/4           /* Words per page                                              */
#define SPARE_BYTES_PER_PAGE                64                                  /* 64 "spare" bytes per page                                   */
/* For 2048 byte block spare area:                             */
#define BAD_BLOCK_POSITION                  0                                   /*      0 is the bad block byte position                        */
#define EXTRA_BYTE_POSITION                 2                                   /*      2 is the extra bytes starting byte position             */
#define ECC_BYTE_POSITION                   40                                  /*      40 is the ECC starting byte position                   */

#define NAND_INSTANCE                       0
/* Define the flag for erase NAND flash memory */
#define LX_DRIVER_ERASES_FLASH_AFTER_INIT
#define NAND_FLASH_SIZE                     (TOTAL_BLOCKS * PHYSICAL_PAGES_PER_BLOCK * BYTES_PER_PHYSICAL_PAGE)

#define SPARE_DATA1_OFFSET                  0
#define SPARE_DATA1_LENGTH                  0x40
#define SPARE_DATA2_OFFSET                  0
#define SPARE_DATA2_LENGTH                  0

/* Definition of the spare area is relative to the block size of the NAND part and perhaps manufactures of the NAND part.
   Here are some common definitions:

   256 Byte Page

        Bytes           Meaning

        0,1,2           ECC bytes
        3,4,6,7         Extra
        5               Bad block flag

    512 Byte Page

        Bytes           Meaning

        0,1,2,3,6,7     ECC bytes
        8-15            Extra
        5               Bad block flag

    2048 Byte Page

        Bytes           Meaning

        0               Bad block flag
        2-39            Extra
        40-63           ECC bytes
*/

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
#endif /* LX_STM32_NAND_DRIVER_H */

