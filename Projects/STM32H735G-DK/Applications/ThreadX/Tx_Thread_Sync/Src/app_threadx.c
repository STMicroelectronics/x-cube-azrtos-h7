/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.c
  * @author  MCD Application Team
  * @brief   ThreadX applicative file
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
#include "app_threadx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "main.h"
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
TX_THREAD tx_app_thread;
/* USER CODE BEGIN PV */
TX_THREAD ThreadTwo;
APP_SYNC_TYPE SyncObject;

extern UART_HandleTypeDef huart3;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
VOID ThreadTwo_Entry(ULONG thread_input);
static VOID Led_Toggle(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UINT iter);
static VOID App_Delay(ULONG Delay);
/* USER CODE END PFP */

/**
  * @brief  Application ThreadX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_ThreadX_Init(VOID *memory_ptr)
{
  UINT ret = TX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;
  /* USER CODE BEGIN App_ThreadX_MEM_POOL */
  /* USER CODE END App_ThreadX_MEM_POOL */
  CHAR *pointer;

  /* Allocate the stack for Thread One  */
  if (tx_byte_allocate(byte_pool, (VOID**) &pointer,
                       TX_APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }
  /* Create Thread One.  */
  if (tx_thread_create(&tx_app_thread, "Thread One", ThreadOne_Entry, 0, pointer,
                       TX_APP_STACK_SIZE, TX_APP_THREAD_PRIO, TX_APP_THREAD_PREEMPTION_THRESHOLD,
                       TX_APP_THREAD_TIME_SLICE, TX_APP_THREAD_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

  /* USER CODE BEGIN App_ThreadX_Init */

  /* Allocate the stack for ThreadTwo.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       TX_APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create Thread Two.  */
  if (tx_thread_create(&ThreadTwo, "Thread Two", ThreadTwo_Entry, 0, pointer,
                       TX_APP_STACK_SIZE, TX_APP_THREAD_PRIO, TX_APP_THREAD_PREEMPTION_THRESHOLD,
                       TX_APP_THREAD_TIME_SLICE, TX_APP_THREAD_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

  /* Create the Synchronization API used by Thread One and Thread Two.*/
  if (APP_SYNC_CREATE(&SyncObject) != TX_SUCCESS)
  {
    return TX_SYNC_ERROR;
  }
  /* USER CODE END App_ThreadX_Init */

  return ret;
}
/**
  * @brief  Function implementing the ThreadOne_Entry thread.
  * @param  thread_input: Hardcoded to 0.
  * @retval None
  */
void ThreadOne_Entry(ULONG thread_input)
{
  /* USER CODE BEGIN ThreadOne_Entry */
  UNUSED(thread_input);
  ULONG iteration = 0;

  /* Infinite loop */
  while(1)
  {
    /* try to acquire the sync object without waiting */
    if (APP_SYNC_GET(&SyncObject, TX_NO_WAIT) == TX_SUCCESS)
    {
      printf("** ThreadOne : SyncObject acquired ** \n");

      /*sync object acquired, toggle the LED_GREEN each 500ms for 5s */
      Led_Toggle(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 10);


      /*release the sync object */
      APP_SYNC_PUT(&SyncObject);

      printf("** ThreadOne : SyncObject released ** \n");

      tx_thread_sleep(1);
    }
    else
    {

      if ((iteration % 2000000) == 0)
      {
        printf("** ThreadOne : waiting for SyncObject !! **\n");
      }
    }
    iteration++;
  }
  /* USER CODE END ThreadOne_Entry */
}

  /**
  * @brief  Function that implements the kernel's initialization.
  * @param  None
  * @retval None
  */
void MX_ThreadX_Init(void)
{
  /* USER CODE BEGIN  Before_Kernel_Start */
  /* USER CODE END  Before_Kernel_Start */

  tx_kernel_enter();

  /* USER CODE BEGIN  Kernel_Start_Error */
  /* USER CODE END  Kernel_Start_Error */
}

/* USER CODE BEGIN 1 */
/**
  * @brief  Function implementing the ThreadTwo thread.
  * @param  thread_input: Not used
  * @retval None
  */
void ThreadTwo_Entry(ULONG thread_input)
{
  UNUSED(thread_input);
  ULONG iteration = 0;

  /* Infinite loop */
  while(1)
  {
    /* try to acquire the sync object without waiting */
    if (APP_SYNC_GET(&SyncObject, TX_NO_WAIT) == TX_SUCCESS)
    {
      printf("** ThreadTwo : SyncObject acquired ** \n");

      /*Sync object acquired toggle the LED_RED each 500ms for 5s*/
      Led_Toggle(LED_RED_GPIO_Port, LED_RED_Pin, 10);

      /*release the sync object*/
      APP_SYNC_PUT(&SyncObject);

      printf("** ThreadTwo : SyncObject released ** \n");

      tx_thread_sleep(1);

    }
    else
    {
      if ((iteration % 2000000) == 0)
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
static VOID Led_Toggle(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UINT iter)
{
  UINT i;

  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);

  for (i =0; i < iter; i++)
  {
    HAL_GPIO_TogglePin(GPIOx, GPIO_Pin);
    /* Toggle the Led each 500ms */
    App_Delay(50);
  }

  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
}

/**
  * @brief  Application Delay function.
  * @param  Delay : number of ticks to wait
  * @retval None
  */
void App_Delay(ULONG Delay)
{
  ULONG initial_time = tx_time_get();
  while ((tx_time_get() - initial_time) < Delay);
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
  HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}
/* USER CODE END 1 */
