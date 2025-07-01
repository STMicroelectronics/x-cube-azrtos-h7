/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ThreadX/Tx_CMSIS_Wrapper/CM7/Src/app_tx_cmsisrtos.c
  * @author  MCD Application Team
  * @brief   ThreadX CMSIS RTOS Wrapper applicative file
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

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/* Includes ------------------------------------------------------------------*/
#include "app_tx_cmsisrtos.h"

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
extern UART_HandleTypeDef huart1;
__IO uint32_t OsStatus = 0;
osThreadId_t ThreadOne;
osThreadId_t ThreadTwo;
APP_SYNC_TYPE SyncObject;
static osThreadAttr_t attr = {
                        .priority = osPriorityNormal,
                        .stack_size = APP_THREAD_STACK_SIZE,
                      };
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void ThreadOne_Entry(void *argument);
static void ThreadTwo_Entry(void *argument);
static void Led_Toggle(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t iter);
/* USER CODE END PFP */

/* Global user code ---------------------------------------------------------*/
/* USER CODE BEGIN Global user code */

/* USER CODE END Global user code */
/**
  * @brief  Application ThreadX with CMSIS RTOS Wrapper Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
uint32_t App_TX_CmsisRTOS_Init(void)
{
  osStatus_t ret = osOK;
  /* USER CODE BEGIN  App_TX_CmsisRTOS_Init */
  ret = osKernelInitialize();

  if (ret == osOK)
  {
    /* Create the thread(s) */
    /* definition and creation of ThreadOne */
    attr.name = "ThreadOne";
    ThreadOne = osThreadNew(ThreadOne_Entry, NULL, (const osThreadAttr_t *)&attr);
    if(ThreadOne == NULL)
    {
      return ((uint32_t)osError);
    }
    /* definition and creation of ThreadTwo */
    attr.name = "ThreadTwo";
    ThreadTwo = osThreadNew(ThreadTwo_Entry, NULL, (const osThreadAttr_t *)&attr);
    if(ThreadTwo == NULL)
    {
      return ((uint32_t)osError);
    }

    SyncObject = APP_SYNC_CREATE();
    /* Start scheduler */
    ret = osKernelStart();
  }
  /* USER CODE END  App_TX_CmsisRTOS_Init */
  return ((uint32_t)ret);
}

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN Private user code */

/* USER CODE END Private user code */

/* USER CODE BEGIN 2 */
/**
  * @brief  Function implementing the ThreadOne thread.
  * @param  argument: Not used
  * @retval None
  */
static void ThreadOne_Entry(void *argument)
{
  (void) argument;
  uint32_t iteration = 0;

  /* Infinite loop */
  while(1)
  {
    /* try to acquire the sync object without waiting */
    if (APP_SYNC_GET(SyncObject, DEFAULT_WAIT_TIME) == osOK)
    {
      printf("** ThreadOne : SyncObject acquired ** \n");

      /*sync object acquired, toggle the LED_GREEN each 500ms for 5s */
      Led_Toggle(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 10);


      /*release the sync object */
      APP_SYNC_PUT(SyncObject);

      printf("** ThreadOne : SyncObject released ** \n");

      osDelay(1);
    }
    else
    {

      if ((iteration % 20) == 0)
      {
        printf("** ThreadOne : waiting for SyncObject !! **\n");
      }
    }
    iteration++;
  }
}

/**
* @brief Function implementing the ThreadTwo thread.
* @param argument: Not used
* @retval None
*/
static void ThreadTwo_Entry(void *argument)
{
  (void) argument;
  uint32_t iteration = 0;

  /* Infinite loop */
  while(1)
  {
    /* try to acquire the sync object without waiting */
    if (APP_SYNC_GET(SyncObject, DEFAULT_WAIT_TIME) == osOK)
    {
      printf("** ThreadTwo : SyncObject acquired ** \n");

      /*Sync object acquired toggle the LED_RED each 500ms for 5s*/
      Led_Toggle(LED_RED_GPIO_Port, LED_RED_Pin, 10);

      /*release the sync object*/
      APP_SYNC_PUT(SyncObject);

      printf("** ThreadTwo : SyncObject released ** \n");

      osDelay(1);

    }
    else
    {
      if ((iteration % 20) == 0)
      {
        printf("** ThreadTwo : waiting for SyncObject !! **\n");
      }
    }
    iteration++;
  }
}

/**
  * @brief Critical section function that needs acquiring SyncObject.
  * @param  led: LED to toggle
  * @param  iter: Number of iterations
  * @retval None
  */
static void Led_Toggle(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t iter)
{
  uint32_t i;

  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);

  for (i =0; i < iter; i++)
  {
    HAL_GPIO_TogglePin(GPIOx, GPIO_Pin);
    osDelay(50);
  }

  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
}

/* USER CODE END  2*/
