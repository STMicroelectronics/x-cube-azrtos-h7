/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ThreadX/Tx_CMSIS_Wrapper/CM7/Inc/app_tx_cmsisrtos.h
  * @author  MCD Application Team
  * @brief   ThreadX CMSIS RTOS Wrapper applicative header file
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
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "cmsis_os2.h"
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
uint32_t App_TX_CmsisRTOS_Init(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
#define USE_TX_MUTEX
/*#define USE_TX_SEMAPHORE*/

#if defined(USE_TX_MUTEX) && defined(USE_TX_SEMAPHORE)
#error "Only one synchronization API must be enabled"
#endif

#ifdef USE_TX_MUTEX
/* Use osMutexId_t as sync object */
#define APP_SYNC_TYPE                        osMutexId_t

#define APP_SYNC_GET                         osMutexAcquire
#define APP_SYNC_PUT                         osMutexRelease
#define APP_SYNC_CREATE()                    osMutexNew(NULL)

#else
/* define osSemaphoreId_t  as sync object*/
#define APP_SYNC_TYPE                        osSemaphoreId_t

#define APP_SYNC_GET                         osSemaphoreAcquire
#define APP_SYNC_PUT                         osSemaphoreRelease
#define APP_SYNC_CREATE()                    osSemaphoreNew(0, 1, NULL)
#endif

#define DEFAULT_WAIT_TIME                   5


#define APP_THREAD_STACK_SIZE    2*256
/* USER CODE END Private defines */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
 }
#endif
#endif /* __APP_THREADX_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
