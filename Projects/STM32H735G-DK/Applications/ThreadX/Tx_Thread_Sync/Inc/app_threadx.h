/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.h
  * @author  MCD Application Team
  * @brief   ThreadX applicative header file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020-2021 STMicroelectronics.
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

/* Private defines -----------------------------------------------------------*/
#define TX_APP_STACK_SIZE                       512
#define TX_APP_THREAD_PRIO                      10
/* USER CODE BEGIN PD */
#define USE_TX_MUTEX
/*#define USE_TX_SEMAPHORE*/

#if defined(USE_TX_MUTEX) && defined(USE_TX_SEMAPHORE)
#error "Only one synchronization API must be enabled"
#endif

#ifdef USE_TX_MUTEX
/* Use TX_MUTEX as sync object */
#define APP_SYNC_TYPE                        TX_MUTEX
#define TX_SYNC_ERROR                        TX_MUTEX_ERROR

#define APP_SYNC_GET                         tx_mutex_get
#define APP_SYNC_PUT                         tx_mutex_put
#define APP_SYNC_CREATE(a)                   tx_mutex_create((a),"tx app mutex", TX_NO_INHERIT)

#else
/* define TX_SEMAPHORE  as sync object*/
#define APP_SYNC_TYPE                        TX_SEMAPHORE
#define TX_SYNC_ERROR                        TX_SEMAPHORE_ERROR

#define APP_SYNC_GET                         tx_semaphore_get
#define APP_SYNC_PUT(a)                      tx_semaphore_ceiling_put((a), 1)
#define APP_SYNC_CREATE(a)                   tx_semaphore_create((a),"tx app binary semaphore", 1)
#endif

#define TX_APP_THREAD_TIME_SLICE             5
/* USER CODE END PD */

/* Main thread defines -------------------------------------------------------*/
#ifndef TX_APP_THREAD_PREEMPTION_THRESHOLD
#define TX_APP_THREAD_PREEMPTION_THRESHOLD      TX_APP_THREAD_PRIO
#endif

#ifndef TX_APP_THREAD_TIME_SLICE
#define TX_APP_THREAD_TIME_SLICE                TX_NO_TIME_SLICE
#endif
#ifndef TX_APP_THREAD_AUTO_START
#define TX_APP_THREAD_AUTO_START                TX_AUTO_START
#endif
/* USER CODE BEGIN MTD */

/* USER CODE END MTD */

/* Exported macro ------------------------------------------------------------*/

/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
UINT App_ThreadX_Init(VOID *memory_ptr);
void MX_ThreadX_Init(void);
void ThreadOne_Entry(ULONG thread_input);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* __APP_THREADX_H__ */
