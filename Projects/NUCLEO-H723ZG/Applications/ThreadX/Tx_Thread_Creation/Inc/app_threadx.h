/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.h
  * @author  MCD Application Team
  * @brief   ThreadX applicative header file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_THREADX_H__
#define __APP_THREADX_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "tx_api.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
UINT App_ThreadX_Init(VOID *memory_ptr);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define APP_STACK_SIZE                           512
#define APP_BYTE_POOL_SIZE                       (12 * 1024)

#define THREAD_ONE_PRIO                          10
#define THREAD_ONE_PREEMPTION_THRESHOLD          THREAD_ONE_PRIO
#define THREAD_TWO_PRIO                          10
#define THREAD_TWO_PREEMPTION_THRESHOLD          9
#define MAIN_THREAD_PRIO                         5
#define MAIN_THREAD_PREEMPTION_THRESHOLD         MAIN_THREAD_PRIO

#define NEW_THREAD_TWO_PRIO                      8
#define NEW_THREAD_TWO_PREEMPTION_THRESHOLD      8

#define THREAD_ONE_EVT                           0x01
#define THREAD_TWO_EVT                           0x02
/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* __APP_THREADX_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
