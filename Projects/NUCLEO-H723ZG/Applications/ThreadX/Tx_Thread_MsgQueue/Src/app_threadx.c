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
TX_QUEUE tx_app_msg_queue;
/* USER CODE BEGIN PV */
TX_THREAD MsgReceiverThread;
TX_THREAD MsgSenderThreadTwo;
TX_QUEUE  MsgQueueTwo;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
void MsgSenderThreadTwo_Entry(ULONG thread_input);
void MsgReceiverThread_Entry(ULONG thread_input);
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

  /* Allocate the stack for Message Queue Sender Thread One  */
  if (tx_byte_allocate(byte_pool, (VOID**) &pointer,
                       TX_APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }
  /* Create Message Queue Sender Thread One.  */
  if (tx_thread_create(&tx_app_thread, "Message Queue Sender Thread One", MsgSenderThreadOne_Entry, 0, pointer,
                       TX_APP_STACK_SIZE, TX_APP_THREAD_PRIO, TX_APP_THREAD_PREEMPTION_THRESHOLD,
                       TX_APP_THREAD_TIME_SLICE, TX_APP_THREAD_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }
  /* Allocate the stack for Message Queue One.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       TX_APP_MSG_QUEUE_FULL_SIZE * sizeof(ULONG), TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }
  /* Create Message Queue One.  */
  if (tx_queue_create(&tx_app_msg_queue, "Message Queue One", TX_APP_SINGLE_MSG_SIZE,
                      pointer, TX_APP_MSG_QUEUE_FULL_SIZE * sizeof(ULONG)) != TX_SUCCESS)
  {
    return TX_QUEUE_ERROR;
  }

  /* USER CODE BEGIN App_ThreadX_Init */

  /* Allocate the stack for Message Queue Sender Thread Two.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       TX_APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create Message Queue Sender Thread Two.  */
  if (tx_thread_create(&MsgSenderThreadTwo, "Message Queue Sender Thread Two", MsgSenderThreadTwo_Entry, 0, pointer,
                       TX_APP_STACK_SIZE, TX_APP_THREAD_PRIO, TX_APP_THREAD_PREEMPTION_THRESHOLD,
                       TX_APP_THREAD_TIME_SLICE, TX_APP_THREAD_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

  /* Allocate the stack for Message Queue Receiver Thread.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       TX_APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create Message Queue Receiver Thread.  */
  if (tx_thread_create(&MsgReceiverThread, "Message Queue Receiver Thread", MsgReceiverThread_Entry, 0, pointer,
                       TX_APP_STACK_SIZE, RECEIVER_THREAD_PRIO, RECEIVER_THREAD_PREEMPTION_THRESHOLD,
                       TX_APP_THREAD_TIME_SLICE, TX_APP_THREAD_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }
  /* Allocate the Message Queue Two.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       TX_APP_MSG_QUEUE_FULL_SIZE * sizeof(ULONG), TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the Message Queue Two shared by Message Queue Sender Thread Two and Message Queue Receiver Thread.  */
  if (tx_queue_create(&MsgQueueTwo, "Message Queue Two", TX_APP_SINGLE_MSG_SIZE,
                      pointer, TX_APP_MSG_QUEUE_FULL_SIZE * sizeof(ULONG)) != TX_SUCCESS)
  {
    return TX_QUEUE_ERROR;
  }
  /* USER CODE END App_ThreadX_Init */

  return ret;
}
/**
  * @brief  Function implementing the MsgSenderThreadOne_Entry thread.
  * @param  thread_input: Hardcoded to 0.
  * @retval None
  */
void MsgSenderThreadOne_Entry(ULONG thread_input)
{
  /* USER CODE BEGIN MsgSenderThreadOne_Entry */
  ULONG Msg = TOGGLE_LED;
  (void) thread_input;
  /* Infinite loop */
  while(1)
  {
    /* Send message to Message Queue One.  */
    if (tx_queue_send(&tx_app_msg_queue, &Msg, TX_WAIT_FOREVER) != TX_SUCCESS)
    {
      Error_Handler();
    }
    /* Sleep for 200ms */
    tx_thread_sleep(20);
  }
  /* USER CODE END MsgSenderThreadOne_Entry */
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
  * @brief  Function implementing the MsgSenderThreadTwo thread.
  * @param  thread_input: Not used
  * @retval None
  */
void MsgSenderThreadTwo_Entry(ULONG thread_input)
{
  ULONG Msg = TOGGLE_LED;
  (void) thread_input;
  /* Infinite loop */
  while(1)
  {
    /* Send message to MsgQueueTwo.  */
    if (tx_queue_send(&MsgQueueTwo, &Msg, TX_WAIT_FOREVER) != TX_SUCCESS)
    {
      Error_Handler();
    }
    /* Sleep for 500s */
    tx_thread_sleep(50);
  }
}

/**
  * @brief  Function implementing the MsgReceiverThread thread.
  * @param  thread_input: Not used
  * @retval None
  */
void MsgReceiverThread_Entry(ULONG thread_input)
{
  ULONG RMsg = 0;
  UINT status = 0 ;
  (void) thread_input;
  /* Infinite loop */
  while (1)
  {
    /* Determine whether a message MsgQueueOne or MsgQueueTwo is available */
    status = tx_queue_receive(&tx_app_msg_queue, &RMsg, TX_NO_WAIT);
    if (status == TX_SUCCESS)
    {
      /* Check Message value */
      if (RMsg != TOGGLE_LED)
      {
        Error_Handler();
      }
      else
      {
        /* Turn ON GREEN LED */
        printf("** Message is available on MsgQueueOne: SET_GREEN_LED**\r\n");
        HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
      }
    }
    else
    {
      status = tx_queue_receive(&MsgQueueTwo, &RMsg, TX_NO_WAIT);
      if ( status == TX_SUCCESS)
      {
        /* Check Message value */
        if (RMsg != TOGGLE_LED)
        {
          Error_Handler();
        }
        else
        {          /* Turn ON RED LED */
          printf("** Message is available on MsgQueueTwo: SET_RED_LED**\r\n");
          HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
        }
      }
    }
  }
}
/* USER CODE END 1 */
