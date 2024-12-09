
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_filex.c
  * @author  MCD Application Team
  * @brief   FileX applicative file
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
#include "app_filex.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* Main thread stack size */
#define FX_APP_THREAD_STACK_SIZE         1024
/* Main thread priority */
#define FX_APP_THREAD_PRIO               10

/* USER CODE BEGIN PD */

#define DEFAULT_STACK_SIZE               (2 * 1024)
#define DEFAULT_THREAD_PRIO              10
#define DEFAULT_TIME_SLICE               4
#define DEFAULT_PREEMPTION_THRESHOLD     DEFAULT_THREAD_PRIO

#define THREAD_ID_M                      0
#define THREAD_ID_1                      1
#define THREAD_ID_2                      2

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Main thread global data structures.  */
TX_THREAD       fx_app_thread;

/* Buffer for FileX FX_MEDIA sector cache. */
ALIGN_32BYTES (uint32_t fx_sd_media_memory[FX_STM32_SD_DEFAULT_SECTOR_SIZE / sizeof(uint32_t)]);
/* Define FileX global data structures.  */
FX_MEDIA        sdio_disk;

/* USER CODE BEGIN PV */

/* Define FileX global data structures.  */
FX_FILE         fx_file_one;
FX_FILE         fx_file_two;
/* Define ThreadX global data structures.  */
TX_THREAD       fx_thread_one;
TX_THREAD       fx_thread_two;
/* Define child threads completion event flags */
TX_EVENT_FLAGS_GROUP    finish_flag;

const static CHAR data[] = "This is FileX working concurrently on STM32";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* Main thread entry function.  */
void fx_app_thread_entry(ULONG thread_input);

/* USER CODE BEGIN PFP */
VOID fx_thread_one_entry(ULONG thread_input);
VOID fx_thread_two_entry(ULONG thread_input);
VOID App_Error_Handler(INT id);

void Error_Handler(void);
/* USER CODE END PFP */

/**
  * @brief  Application FileX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT MX_FileX_Init(VOID *memory_ptr)
{
  UINT ret = FX_SUCCESS;

  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;
  VOID *pointer;

  /* USER CODE BEGIN MX_FileX_MEM_POOL */
  /* USER CODE END MX_FileX_MEM_POOL */

  /* USER CODE BEGIN 0 */

  /* USER CODE END 0 */

  /*Allocate memory for the main thread's stack*/
  ret = tx_byte_allocate(byte_pool, &pointer, FX_APP_THREAD_STACK_SIZE, TX_NO_WAIT);

  /* Check FX_APP_THREAD_STACK_SIZE allocation*/
  if (ret != FX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the main thread.  */
  ret = tx_thread_create(&fx_app_thread, FX_APP_THREAD_NAME, fx_app_thread_entry, 0, pointer, FX_APP_THREAD_STACK_SIZE,
                         FX_APP_THREAD_PRIO, FX_APP_PREEMPTION_THRESHOLD, FX_APP_THREAD_TIME_SLICE, FX_APP_THREAD_AUTO_START);

  /* Check main thread creation */
  if (ret != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }
  /* USER CODE BEGIN MX_FileX_Init */

  /* Allocate memory for the 1st concurrent thread's stack */
  ret = tx_byte_allocate(byte_pool, &pointer, DEFAULT_STACK_SIZE, TX_NO_WAIT);

  if (ret != FX_SUCCESS)
  {
    /* Failed at allocating memory */
    Error_Handler();
  }

  /* Create the 1st concurrent thread.  */
  ret = tx_thread_create(&fx_thread_one, "fx_thread_one", fx_thread_one_entry, 0, pointer, DEFAULT_STACK_SIZE, DEFAULT_THREAD_PRIO,
                         DEFAULT_PREEMPTION_THRESHOLD, DEFAULT_TIME_SLICE, TX_DONT_START);

  if (ret != FX_SUCCESS)
  {
    /* Failed at creating thread */
    Error_Handler();
  }

  /* Allocate memory for the 2nd concurrent thread's stack */
  ret = tx_byte_allocate(byte_pool, &pointer, DEFAULT_STACK_SIZE, TX_NO_WAIT);

  if (ret != FX_SUCCESS)
  {
    /* Failed at allocating memory */
    Error_Handler();
  }

  /* Create the 2nd concurrent thread */
  ret = tx_thread_create(&fx_thread_two, "fx_thread_two", fx_thread_two_entry, 0, pointer, DEFAULT_STACK_SIZE, DEFAULT_THREAD_PRIO,
                         DEFAULT_PREEMPTION_THRESHOLD, DEFAULT_TIME_SLICE, TX_DONT_START);

  if (ret != FX_SUCCESS)
  {
    /* Failed at creating thread */
    Error_Handler();
  }

  /* An event flag to indicate the status of execution */
  ret = tx_event_flags_create(&finish_flag, "event_flag");

  if (ret != FX_SUCCESS)
  {
    /* Failed at creating event flag */
    Error_Handler();
  }

  /* USER CODE END MX_FileX_Init */

  /* Initialize FileX.  */
  fx_system_initialize();

  /* USER CODE BEGIN MX_FileX_Init 1*/

  /* USER CODE END MX_FileX_Init 1*/

  return ret;
}

 /**
 * @brief  Main thread entry.
 * @param thread_input: ULONG user argument used by the thread entry
 * @retval none
 */
void fx_app_thread_entry(ULONG thread_input)
{
  UINT sd_status = FX_SUCCESS;
  /* USER CODE BEGIN fx_app_thread_entry 0 */
  ULONG event_flags;
  /* USER CODE END fx_app_thread_entry 0 */

  /* Open the SD disk driver */
  sd_status =  fx_media_open(&sdio_disk, FX_SD_VOLUME_NAME, fx_stm32_sd_driver, (VOID *)FX_NULL, (VOID *) fx_sd_media_memory, sizeof(fx_sd_media_memory));

  /* Check the media open sd_status */
  if (sd_status != FX_SUCCESS)
  {
    /* USER CODE BEGIN SD open error */
    while(1);
    /* USER CODE END SD open error */
  }

  /* USER CODE BEGIN fx_app_thread_entry 1 */
  /* Media opened successfully, we start the concurrent threads. */
  sd_status = tx_thread_resume(&fx_thread_one) & tx_thread_resume(&fx_thread_two);

  /* Check the concurrent thread was started correctly.  */
  if (sd_status != TX_SUCCESS)
  {
    App_Error_Handler(THREAD_ID_M);
  }

  /* block here waiting for concurrent threads to finish processing */
  sd_status = tx_event_flags_get(&finish_flag, 0x11, TX_AND_CLEAR,
                                 &event_flags, TX_WAIT_FOREVER);

  /* Check the status.  */
  if (sd_status != TX_SUCCESS)
  {
    /* Error getting the event flags, call error handler.  */
    App_Error_Handler(THREAD_ID_M);
  }

  /* Close the media.  */
  sd_status =  fx_media_close(&sdio_disk);

  /* Check the media close status.  */
  if (sd_status != FX_SUCCESS)
  {
    /* Error closing the media, call error handler.  */
    App_Error_Handler(THREAD_ID_M);
  }

  /* Toggle green LED to indicate processing finish OK */
  while (1)
  {
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    tx_thread_sleep(40);
  }
  /* USER CODE END fx_app_thread_entry 1 */
}

/* USER CODE BEGIN 1 */

VOID fx_thread_one_entry(ULONG thread_input)
{

  UINT status;
  ULONG bytes_read;
  CHAR read_buffer[32];

  /* Create a file called STM32_FILE1.TXT in the root directory.  */
  status =  fx_file_create(&sdio_disk, "STM32_FILE1.TXT");

  /* Check the create status.  */
  if (status != FX_SUCCESS)
  {
    /* Check for an already created status. This is expected on the
    second pass of this loop!  */
    if (status != FX_ALREADY_CREATED)
    {
      /* Create error, call error handler.  */
      App_Error_Handler(THREAD_ID_1);
    }
  }

  /* Open the test file.  */
  status =  fx_file_open(&sdio_disk, &fx_file_one, "STM32_FILE1.TXT", FX_OPEN_FOR_WRITE);

  /* Check the file open status.  */
  if (status != FX_SUCCESS)
  {
    /* Error opening file, call error handler.  */
    App_Error_Handler(THREAD_ID_1);
  }

  /* Seek to the beginning of the test file.  */
  status =  fx_file_seek(&fx_file_one, 0);

  /* Check the file seek status.  */
  if (status != FX_SUCCESS)
  {
    /* Error performing file seek, call error handler.  */
    App_Error_Handler(THREAD_ID_1);
  }

  /* Write a string to the test file.  */
  status =  fx_file_write(&fx_file_one, (void*) data, sizeof(data));

  /* Check the file write status.  */
  if (status != FX_SUCCESS)
  {
    /* Error writing to a file, call error handler.  */
    App_Error_Handler(THREAD_ID_1);
  }

  /* Close the test file.  */
  status =  fx_file_close(&fx_file_one);

  /* Check the file close status.  */
  if (status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    App_Error_Handler(THREAD_ID_1);
  }

  status = fx_media_flush(&sdio_disk);

  /* Check the media flush  status.  */
  if (status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    App_Error_Handler(THREAD_ID_1);
  }

  /* Open the test file.  */
  status =  fx_file_open(&sdio_disk, &fx_file_one, "STM32_FILE1.TXT", FX_OPEN_FOR_READ);

  /* Check the file open status.  */
  if (status != FX_SUCCESS)
  {
    /* Error opening file, call error handler.  */
    App_Error_Handler(THREAD_ID_1);
  }

  /* Seek to the beginning of the test file.  */
  status =  fx_file_seek(&fx_file_one, 0);

  /* Check the file seek status.  */
  if (status != FX_SUCCESS)
  {
    /* Error performing file seek, call error handler.  */
    App_Error_Handler(THREAD_ID_1);
  }

  /* Read the content of the test file.  */
  status =  fx_file_read(&fx_file_one, read_buffer, sizeof(data), &bytes_read);

  /* Check the file read status.  */
  if ((status != FX_SUCCESS) || (bytes_read != sizeof(data)))
  {
    /* Error reading file, call error handler.  */
    App_Error_Handler(THREAD_ID_1);
  }

  /* Close the test file.  */
  status =  fx_file_close(&fx_file_one);

  /* Check the file close status.  */
  if (status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    App_Error_Handler(THREAD_ID_1);
  }

  status = tx_event_flags_set(&finish_flag,  0x01, TX_OR);

  /* Check the event setting status.  */
  if (status != TX_SUCCESS)
  {
    /* Error calling the event setter, call error handler.  */
    App_Error_Handler(THREAD_ID_1);
  }

  while (1)
  {
    /* Do nothing wait for the other thread */
      tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
  }
}

VOID fx_thread_two_entry(ULONG thread_input)
{

  UINT status;
  ULONG bytes_read;
  CHAR read_buffer[32];

  /* Create a file called STM32_FILE2.TXT in the root directory.  */
  status =  fx_file_create(&sdio_disk, "STM32_FILE2.TXT");

  /* Check the create status.  */
  if (status != FX_SUCCESS)
  {
    /* Check for an already created status. This is expected on the
    second pass of this loop!  */
    if (status != FX_ALREADY_CREATED)
    {
      /* Create error, call error handler.  */
      App_Error_Handler(THREAD_ID_2);
    }
  }

  /* Open the test file.  */
  status =  fx_file_open(&sdio_disk, &fx_file_two, "STM32_FILE2.TXT", FX_OPEN_FOR_WRITE);

  /* Check the file open status.  */
  if (status != FX_SUCCESS)
  {
    /* Error opening file, call error handler.  */
    App_Error_Handler(THREAD_ID_2);
  }

  /* Seek to the beginning of the test file.  */
  status =  fx_file_seek(&fx_file_two, 0);

  /* Check the file seek status.  */
  if (status != FX_SUCCESS)
  {
    /* Error performing file seek, call error handler.  */
    App_Error_Handler(THREAD_ID_2);
  }

  /* Write a string to the test file.  */
  status =  fx_file_write(&fx_file_two, (void*) data, sizeof(data));

  /* Check the file write status.  */
  if (status != FX_SUCCESS)
  {
    /* Error writing to a file, call error handler.  */
    App_Error_Handler(THREAD_ID_2);
  }

  /* Close the test file.  */
  status =  fx_file_close(&fx_file_two);

  /* Check the file close status.  */
  if (status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    App_Error_Handler(THREAD_ID_2);
  }

  status = fx_media_flush(&sdio_disk);

  /* Check the media flush  status.  */
  if (status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    App_Error_Handler(THREAD_ID_2);
  }

  /* Open the test file.  */
  status =  fx_file_open(&sdio_disk, &fx_file_two, "STM32_FILE2.TXT", FX_OPEN_FOR_READ);

  /* Check the file open status.  */
  if (status != FX_SUCCESS)
  {
    /* Error opening file, call error handler.  */
    App_Error_Handler(THREAD_ID_2);
  }

  /* Seek to the beginning of the test file.  */
  status =  fx_file_seek(&fx_file_two, 0);

  /* Check the file seek status.  */
  if (status != FX_SUCCESS)
  {
    /* Error performing file seek, call error handler.  */
    App_Error_Handler(THREAD_ID_2);
  }

  /* Read the content of the test file.  */
  status =  fx_file_read(&fx_file_two, read_buffer, sizeof(data), &bytes_read);

  /* Check the file read status.  */
  if ((status != FX_SUCCESS) || (bytes_read != sizeof(data)))
  {
    /* Error reading file, call error handler.  */
    App_Error_Handler(THREAD_ID_2);
  }

  /* Close the test file.  */
  status =  fx_file_close(&fx_file_two);

  /* Check the file close status.  */
  if (status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    App_Error_Handler(THREAD_ID_2);
  }

  status = tx_event_flags_set(&finish_flag,  0x10, TX_OR);

  /* Check the event setting status.  */
  if (status != TX_SUCCESS)
  {
    /* Error calling the event setter, call error handler.  */
    App_Error_Handler(THREAD_ID_1);
  }

  while (1)
  {
    /* Do nothing wait for the other thread */
      tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
  }
}

VOID App_Error_Handler(INT id)
{

  switch (id)
  {

    case THREAD_ID_M :
      /* terminate the other threads to preserve the call stack pointing here */
      tx_thread_terminate(&fx_thread_one);
      tx_thread_terminate(&fx_thread_two);
      break;

    case THREAD_ID_1 :
      /* terminate the other threads to preserve the call stack pointing here */
      tx_thread_terminate(&fx_app_thread);
      tx_thread_terminate(&fx_thread_two);
      break;

    case THREAD_ID_2 :
      /* terminate the other threads to preserve the call stack pointing here */
      tx_thread_terminate(&fx_app_thread);
      tx_thread_terminate(&fx_thread_one);
      break;
  }

  /* Call the main error handler */
  Error_Handler();

}
/* USER CODE END 1 */
