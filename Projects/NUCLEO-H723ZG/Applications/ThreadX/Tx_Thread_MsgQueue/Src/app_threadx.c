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
TX_THREAD              MsgSenderThreadOne;
TX_THREAD              MsgReceiverThread;
TX_THREAD              MsgSenderThreadTwo;
TX_QUEUE               MsgQueueOne;
TX_QUEUE               MsgQueueTwo;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
void    MsgSenderThreadOne_Entry(ULONG thread_input);
void    MsgSenderThreadTwo_Entry(ULONG thread_input);
void    MsgReceiverThread_Entry(ULONG thread_input);
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

  /* USER CODE BEGIN App_ThreadX_Init */
  CHAR *pointer;

  /* Allocate the stack for MsgSenderThreadOne.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
  }
  
  /* Create MsgSenderThreadOne.  */
  if (tx_thread_create(&MsgSenderThreadOne, "Message Queue Sender Thread One", 
                       MsgSenderThreadOne_Entry, 0, pointer, APP_STACK_SIZE, 
                       SENDER_THREAD_PRIO, SENDER_THREAD_PREEMPTION_THRESHOLD, 
                       TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
  {
    ret = TX_THREAD_ERROR;
  }
  
  /* Allocate the stack for MsgSenderThreadTwo.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
  }
  
  /* Create MsgSenderThreadTwo.  */
  if (tx_thread_create(&MsgSenderThreadTwo, "Message Queue Sender Thread Two",
                       MsgSenderThreadTwo_Entry, 0, pointer, APP_STACK_SIZE, 
                       SENDER_THREAD_PRIO, SENDER_THREAD_PREEMPTION_THRESHOLD,
                       TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
  {
    ret = TX_THREAD_ERROR;
  }
  
  /* Allocate the stack for MsgReceiverThread.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
  }
  
  /* Create MsgReceiverThread.  */
  if (tx_thread_create(&MsgReceiverThread, "Message Queue Receiver Thread", 
                       MsgReceiverThread_Entry, 0, pointer, APP_STACK_SIZE, 
                       RECEIVER_THREAD_PRIO, RECEIVER_THREAD_PREEMPTION_THRESHOLD,
                       TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
  {
    ret = TX_THREAD_ERROR;
  }
  
  /* Allocate the MsgQueueOne.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       APP_QUEUE_SIZE*sizeof(ULONG), TX_NO_WAIT) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
  }
  
  /* Create the MsgQueueOne shared by MsgSenderThreadOne and MsgReceiverThread */
  if (tx_queue_create(&MsgQueueOne, "Message Queue One",TX_1_ULONG,
                      pointer, APP_QUEUE_SIZE*sizeof(ULONG)) != TX_SUCCESS)
  {
    ret = TX_QUEUE_ERROR;
  }
  
  /* Allocate the MsgQueueTwo.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       APP_QUEUE_SIZE*sizeof(ULONG), TX_NO_WAIT) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
  }
  
  /* Create the MsgQueueTwo shared by MsgSenderThreadTwo and MsgReceiverThread.  */
  if (tx_queue_create(&MsgQueueTwo, "Message Queue Two", TX_1_ULONG,
                      pointer, APP_QUEUE_SIZE*sizeof(ULONG)) != TX_SUCCESS)
  {
    ret = TX_QUEUE_ERROR;
  }
  /* USER CODE END App_ThreadX_Init */

  return ret;
}

/**
  * @brief  MX_ThreadX_Init
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
  * @brief  Function implementing the MsgSenderThreadOne thread.
  * @param  thread_input: Not used 
  * @retval None
  */
void MsgSenderThreadOne_Entry(ULONG thread_input)
{
  ULONG Msg = TOGGLE_LED;
  (void) thread_input;
  /* Infinite loop */
  while(1)
  {
    /* Send message to MsgQueueOne.  */
    if (tx_queue_send(&MsgQueueOne, &Msg, TX_WAIT_FOREVER) != TX_SUCCESS)
    {
      Error_Handler();
    }
    /* Sleep for 200ms */
    tx_thread_sleep(20);
  }
}

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
    status = tx_queue_receive(&MsgQueueOne, &RMsg, TX_NO_WAIT);
    if (status == TX_SUCCESS)
    {
      /* Check Message value */
      if (RMsg != TOGGLE_LED)
      {
        Error_Handler();
      }
      else
      {
        BSP_LED_Toggle(LED_GREEN);
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
        {
          BSP_LED_Toggle(LED_RED);
        }
      }
    }
  }
}
/* USER CODE END 1 */
