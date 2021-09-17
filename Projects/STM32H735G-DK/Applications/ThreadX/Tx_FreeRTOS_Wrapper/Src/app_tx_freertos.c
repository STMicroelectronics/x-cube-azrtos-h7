/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_tx_freertos.c
  * @author  MCD Application Team
  * @brief   ThreadX FreeRTOS Wrapper applicative file
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

/* Includes ------------------------------------------------------------------*/
#include "app_tx_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
TaskHandle_t LEDThread;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void LEDThread_Entry(void *argument);
/* USER CODE END PFP */

/* Global user code ---------------------------------------------------------*/
/* USER CODE BEGIN Global user code */

/* USER CODE END Global user code */
/**
  * @brief  Application ThreadX with FreeRTOS Wrapper Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_TX_FreeRTOS_Init(void)
{
  UINT ret = TX_SUCCESS;
  /* USER CODE BEGIN  App_TX_FreeRTOS_Init */
  tx_kernel_enter();
  /* USER CODE END  App_TX_FreeRTOS_Init */
  return ret;
}

/**
  * @brief  Define the initial system.
  * @param  first_unused_memory : Pointer to the first unused memory
  * @retval None
  */
VOID tx_application_define(VOID *first_unused_memory)
{
  /* USER CODE BEGIN  tx_application_define */
  /* Initialize the adaptation layer with 64KiB of internal heap.*/
  if(tx_freertos_init() != TX_SUCCESS) 
  {
    Error_Handler();
  }
  /* Create LEDThread.  */
  if (xTaskCreate(LEDThread_Entry, "LED Thread", 
                  APP_STACK_SIZE, NULL, LED_THREAD_PRIO,
                  &LEDThread) != pdPASS)
  {
    Error_Handler();
  }
  /* USER CODE END  tx_application_define */
}

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN Private user code */

/* USER CODE END Private user code */

/* USER CODE BEGIN 1 */
/**
  * @brief  Function implementing the LEDThread thread.
  * @param  argument: Not used 
  * @retval None
  */
void LEDThread_Entry(void *argument)
{
  (void) argument;
  /* Infinite loop */
  while(1)
  {
    BSP_LED_Toggle(LED_GREEN);
    /* Delay for 500ms */
    vTaskDelay(500);
  }
}
/* USER CODE END 1 */
