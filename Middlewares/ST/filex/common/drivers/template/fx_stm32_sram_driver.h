/**
  ******************************************************************************
  * @file    fx_stm32_sram_driver_template.h
  * @author  MCD Application Team
  * @brief   stm32 FileX sram driver header filet template.
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
#ifndef FX_STM32_SRAM_DRIVER_H
#define FX_STM32_SRAM_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "fx_api.h"

#include "stm32XXXX.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/

#define FX_SRAM_DISK_BASE_ADDRESS         D1_AXISRAM_BASE

/* define the SRAM DISK size */
#define FX_SRAM_DISK_SIZE                 (32 * 1024)


/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

VOID fx_stm32_ram_driver(FX_MEDIA *media_ptr);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

#ifdef __cplusplus
}
#endif
#endif /* FX_STM32_SRAM_DRIVER_H */
