
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

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/* Includes ------------------------------------------------------------------*/
#include "app_filex.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

#define FILEX_DEFAULT_STACK_SIZE               (2 * 1024)
#define APP_QUEUE_SIZE                          16

/* fx_sd_thread priority */
#define DEFAULT_THREAD_PRIO              10

/* Msg content*/
typedef enum {
  CARD_STATUS_CHANGED             = 99,
  CARD_STATUS_DISCONNECTED        = 88,
  CARD_STATUS_CONNECTED           = 77
} SD_ConnectionStateTypeDef;

/* fx_sd_thread preemption priority */
#define DEFAULT_PREEMPTION_THRESHOLD      DEFAULT_THREAD_PRIO

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
FX_FILE         fx_file;

/* Define ThreadX global data structures.  */
TX_QUEUE        tx_msg_queue;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* Main thread entry function.  */
void fx_app_thread_entry(ULONG thread_input);

/* USER CODE BEGIN PFP */
static UINT SD_IsDetected(uint32_t Instance);
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

  /* Allocate Memory for the Queue */
  ret = tx_byte_allocate(byte_pool, (VOID **) &pointer, APP_QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT);

  /*Check memory allocation for the queue*/
  if (ret != FX_SUCCESS)
  {
    Error_Handler();
  }

  /* Create message queue*/
  ret = tx_queue_create(&tx_msg_queue, "sd_event_queue", 1, pointer, APP_QUEUE_SIZE * sizeof(ULONG));

  /* Check main thread creation */
  if (ret != FX_SUCCESS)
  {
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
  UINT status;
  ULONG r_msg;
  ULONG s_msg = CARD_STATUS_CHANGED;
  ULONG last_status = CARD_STATUS_DISCONNECTED;
  ULONG bytes_read;
  CHAR read_buffer[32];
  CHAR data[] = "This is FileX working on STM32";

  if(SD_IsDetected(FX_STM32_SD_INSTANCE) == HAL_OK)
  {
    /* SD card is already inserted, place the info into the queue */
    tx_queue_send(&tx_msg_queue, &s_msg, TX_NO_WAIT);
  }
  else
  {
    /* Indicate that SD card is not inserted from start */
    HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
  }

  /* Infinite Loop */
  for( ;; )
  {

    /* We wait here for a valid SD card insertion event, if it is not inserted already */
    while(1)
    {

      while(tx_queue_receive(&tx_msg_queue, &r_msg, TX_TIMER_TICKS_PER_SECOND / 2) != TX_SUCCESS)
      {
        /* Toggle Blue LED to indicate idle state after a successful operation */
        if(last_status == CARD_STATUS_CONNECTED)
        {
          HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
        }
      }

      /* check if we received the correct event msg */
      if(r_msg == CARD_STATUS_CHANGED)
      {
        /* reset the status */
        r_msg = 0;

        /* for debouncing purpose we wait a bit till it settles down */
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2);

        if(SD_IsDetected(FX_STM32_SD_INSTANCE) == HAL_OK)
        {
          /* We have a valid SD insertion event, start processing.. */
          /* Update last known status */
          last_status = CARD_STATUS_CONNECTED;
          HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);


          break;
        }
        else
        {
          /* Update last known status */
          last_status = CARD_STATUS_DISCONNECTED;
          /* Switch on the Red LED and switch off the Blue LED */
          HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);
        }
      }

    }
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
/* Create a file called STM32.TXT in the root directory.  */
    status =  fx_file_create(&sdio_disk, "STM32.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {
      /* Check for an already created status. This is expected on the
      second pass of this loop!  */
      if (status != FX_ALREADY_CREATED)
      {
        /* Create error, call error handler.  */
        Error_Handler();
      }
    }

    /* Open the test file.  */
    status =  fx_file_open(&sdio_disk, &fx_file, "STM32.TXT", FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {
      /* Error opening file, call error handler.  */
      Error_Handler();
    }

    /* Seek to the beginning of the test file.  */
    status =  fx_file_seek(&fx_file, 0);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)
    {
      /* Error performing file seek, call error handler.  */
      Error_Handler();
    }

    /* Write a string to the test file.  */
    status =  fx_file_write(&fx_file, data, sizeof(data));

    /* Check the file write status.  */
    if (status != FX_SUCCESS)
    {
      /* Error writing to a file, call error handler.  */
      Error_Handler();
    }

    /* Close the test file.  */
    status =  fx_file_close(&fx_file);

    /* Check the file close status.  */
    if (status != FX_SUCCESS)
    {
      /* Error closing the file, call error handler.  */
      Error_Handler();
    }

    status = fx_media_flush(&sdio_disk);

    /* Check the media flush  status.  */
    if (status != FX_SUCCESS)
    {
      /* Error closing the file, call error handler.  */
      Error_Handler();
    }

    /* Open the test file.  */
    status =  fx_file_open(&sdio_disk, &fx_file, "STM32.TXT", FX_OPEN_FOR_READ);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {
      /* Error opening file, call error handler.  */
      Error_Handler();
    }

    /* Seek to the beginning of the test file.  */
    status =  fx_file_seek(&fx_file, 0);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)
    {
      /* Error performing file seek, call error handler.  */
      Error_Handler();
    }

    /* Read the first 28 bytes of the test file.  */
    status =  fx_file_read(&fx_file, read_buffer, sizeof(data), &bytes_read);

    /* Check the file read status.  */
    if ((status != FX_SUCCESS) || (bytes_read != sizeof(data)))
    {
      /* Error reading file, call error handler.  */
      Error_Handler();
    }

    /* Close the test file.  */
    status =  fx_file_close(&fx_file);

    /* Check the file close status.  */
    if (status != FX_SUCCESS)
    {
      /* Error closing the file, call error handler.  */
      Error_Handler();
    }

    /* Close the media.  */
    status =  fx_media_close(&sdio_disk);

    /* Check the media close status.  */
    if (status != FX_SUCCESS)
    {
      /* Error closing the media, call error handler.  */
      Error_Handler();
    }

  }
  /* USER CODE END fx_app_thread_entry 1 */
}

/* USER CODE BEGIN 2 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  ULONG s_msg = CARD_STATUS_CHANGED;
  if (GPIO_Pin == SD_DETECT_PIN)
  {
    tx_queue_send(&tx_msg_queue, &s_msg, TX_NO_WAIT);
  }
}

static UINT SD_IsDetected(uint32_t Instance)
{
  UINT ret;

  ret = (uint32_t)HAL_GPIO_ReadPin(SD_DETECT_GPIO_PORT, SD_DETECT_PIN);

  if(ret != GPIO_PIN_RESET)
  {
    ret = (UINT)HAL_ERROR;
  }
  else
  {
    ret = (UINT)HAL_OK;
  }

  return ret;
}

/* USER CODE END 2 */
