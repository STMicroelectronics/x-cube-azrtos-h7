/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_usbx.h
  * @author  MCD Application Team
  * @brief   USBX applicative header file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_USBX_H__
#define __APP_USBX_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ux_api.h"

#include "app_usbx_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
#define USBX_MEMORY_STACK_SIZE       1024 * 46

/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported functions prototypes ---------------------------------------------*/
UINT MX_USBX_Init(VOID *memory_ptr);

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* __APP_USBX_H__ */
