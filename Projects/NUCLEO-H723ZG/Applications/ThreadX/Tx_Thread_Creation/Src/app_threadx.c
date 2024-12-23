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
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TX_THREAD tx_app_thread;
/* USER CODE BEGIN PV */
TX_THREAD ThreadOne;
TX_THREAD ThreadTwo;
TX_EVENT_FLAGS_GROUP EventFlag;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
void ThreadOne_Entry(ULONG thread_input);
void ThreadTwo_Entry(ULONG thread_input);
void App_Delay(uint32_t Delay);
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

  /* Allocate the stack for Main Thread  */
  if (tx_byte_allocate(byte_pool, (VOID**) &pointer,
                       TX_APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }
  /* Create Main Thread.  */
  if (tx_thread_create(&tx_app_thread, "Main Thread", MainThread_Entry, 0, pointer,
                       TX_APP_STACK_SIZE, TX_APP_THREAD_PRIO, TX_APP_THREAD_PREEMPTION_THRESHOLD,
                       TX_APP_THREAD_TIME_SLICE, TX_APP_THREAD_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

  /* USER CODE BEGIN App_ThreadX_Init */
  /* Allocate the stack for ThreadOne.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       TX_APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create ThreadOne.  */
  if (tx_thread_create(&ThreadOne, "Thread One", ThreadOne_Entry, 0, pointer,
                       TX_APP_STACK_SIZE, THREAD_ONE_PRIO, THREAD_ONE_PREEMPTION_THRESHOLD,
                       TX_APP_THREAD_TIME_SLICE, TX_APP_THREAD_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

  /* Allocate the stack for ThreadTwo.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       TX_APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create ThreadTwo.  */
  if (tx_thread_create(&ThreadTwo, "Thread Two", ThreadTwo_Entry, 0, pointer,
                       TX_APP_STACK_SIZE, THREAD_TWO_PRIO, THREAD_TWO_PREEMPTION_THRESHOLD,
                       TX_APP_THREAD_TIME_SLICE, TX_APP_THREAD_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

  /* Create the event flags group.  */
  if (tx_event_flags_create(&EventFlag, "Event Flag") != TX_SUCCESS)
  {
    return TX_GROUP_ERROR;
  }
  /* USER CODE END App_ThreadX_Init */

  return ret;
}
/**
  * @brief  Function implementing the MainThread_Entry thread.
  * @param  thread_input: Hardcoded to 0.
  * @retval None
  */
void MainThread_Entry(ULONG thread_input)
{
  /* USER CODE BEGIN MainThread_Entry */
  UINT old_prio = 0;
  UINT old_pre_threshold = 0;
  ULONG   actual_flags = 0;
  uint8_t count = 0;
  (void) thread_input;

  while (count < 3)
  {
    count++;
    if (tx_event_flags_get(&EventFlag, THREAD_ONE_EVT, TX_OR_CLEAR,
                           &actual_flags, TX_WAIT_FOREVER) != TX_SUCCESS)
    {
      Error_Handler();
    }
    else
    {
      /* Update the priority and preemption threshold of ThreadTwo
      to allow the preemption of ThreadOne */
      tx_thread_priority_change(&ThreadTwo, NEW_THREAD_TWO_PRIO, &old_prio);
      tx_thread_preemption_change(&ThreadTwo, NEW_THREAD_TWO_PREEMPTION_THRESHOLD, &old_pre_threshold);

      if (tx_event_flags_get(&EventFlag, THREAD_TWO_EVT, TX_OR_CLEAR,
                             &actual_flags, TX_WAIT_FOREVER) != TX_SUCCESS)
      {
        Error_Handler();
      }
      else
      {
        /* Reset the priority and preemption threshold of ThreadTwo */
        tx_thread_priority_change(&ThreadTwo, THREAD_TWO_PRIO, &old_prio);
        tx_thread_preemption_change(&ThreadTwo, THREAD_TWO_PREEMPTION_THRESHOLD, &old_pre_threshold);
      }
    }
  }

  /* Destroy ThreadOne and ThreadTwo */
  tx_thread_terminate(&ThreadOne);
  tx_thread_terminate(&ThreadTwo);

  /* Infinite loop */
  while(1)
  {
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);

    /* Thread sleep for 1s */
    tx_thread_sleep(100);
  }
  /* USER CODE END MainThread_Entry */
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
  * @brief  Function implementing the ThreadOne thread.
  * @param  thread_input: Not used
  * @retval None
  */
void ThreadOne_Entry(ULONG thread_input)
{
  (void) thread_input;
  uint8_t count = 0;
  /* Infinite loop */
  while(1)
  {
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);

    /* Delay for 500ms (App_Delay is used to avoid context change). */
    App_Delay(50);
    count ++;
    if (count == 10)
    {
      count = 0;
      if (tx_event_flags_set(&EventFlag, THREAD_ONE_EVT, TX_OR) != TX_SUCCESS)
      {
        Error_Handler();
      }
    }
  }
}

/**
  * @brief  Function implementing the ThreadTwo thread.
  * @param  thread_input: Not used
  * @retval None
  */
void ThreadTwo_Entry(ULONG thread_input)
{
  (void) thread_input;
  uint8_t count = 0;
  /* Infinite loop */
  while (1)
  {
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    /* Delay for 200ms (App_Delay is used to avoid context change). */
    App_Delay(20);
    count ++;
    if (count == 25)
    {
      count = 0;
      if (tx_event_flags_set(&EventFlag, THREAD_TWO_EVT, TX_OR) != TX_SUCCESS)
      {
        Error_Handler();
      }
    }
  }
}

/**
  * @brief  Application Delay function.
  * @param  Delay : number of ticks to wait
  * @retval None
  */
void App_Delay(uint32_t Delay)
{
  UINT initial_time = tx_time_get();
  while ((tx_time_get() - initial_time) < Delay);
}
/* USER CODE END 1 */
