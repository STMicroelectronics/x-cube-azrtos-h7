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

/* Includes ------------------------------------------------------------------*/
#include "app_tx_cmsisrtos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#if defined ( __GNUC__) && !defined(__clang__)
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
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
static void Led_Toggle(Led_TypeDef led, uint32_t iter);
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
    /* definition and creation of ThreadOne */
    attr.name = "ThreadTwo";
    ThreadOne = osThreadNew(ThreadTwo_Entry, NULL, (const osThreadAttr_t *)&attr);
    
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

/* USER CODE BEGIN 1 */
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
      Led_Toggle(LED_GREEN, 10);
      

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
      Led_Toggle(LED_RED, 10);
      
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
static void Led_Toggle(Led_TypeDef led, uint32_t iter)
{
  uint32_t i;

  BSP_LED_Off(led);

  for (i =0; i < iter; i++)
  {
    BSP_LED_Toggle(led);
    osDelay(50);
  }

  BSP_LED_Off(led);
}

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART1 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}
/* USER CODE END 1 */
