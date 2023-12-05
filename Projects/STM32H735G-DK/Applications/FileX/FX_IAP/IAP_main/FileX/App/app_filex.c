
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
#include "stm32h7xx_hal_flash.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* Main thread stack size */
#define FX_APP_THREAD_STACK_SIZE         1024 * 2
/* Main thread priority */
#define FX_APP_THREAD_PRIO               10

/* USER CODE BEGIN PD */
#define DEFAULT_STACK_SIZE               (2 * 1024)
#define DEFAULT_THREAD_PRIO              10
#define DEFAULT_TIME_SLICE               4
#define DEFAULT_PREEMPTION_THRESHOLD     DEFAULT_THREAD_PRIO

#define FLASH_WORD_SIZE                  32
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
/* Flash buffer*/
UINT *read_buffer;

/* Define FileX global data structures.  */
FX_FILE         fx_file;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* Main thread entry function.  */
void fx_app_thread_entry(ULONG thread_input);

/* USER CODE BEGIN PFP */

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
  if (ret != FX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }
  /* USER CODE BEGIN MX_FileX_Init */

  /* Allocate memory for the flash buffer */
  ret = tx_byte_allocate(byte_pool, (VOID **) &read_buffer, FLASH_WORD_SIZE, TX_NO_WAIT);

  if (ret != FX_SUCCESS)
  {
    /* Failed at allocating memory */
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
  ULONG bytes_read = 0;
  ULONG FlashAddress = APP_ADDRESS;
  ULONG flash_ret;
  uint32_t SectorError;
  FLASH_EraseInitTypeDef EraseInitStruct;
  TX_INTERRUPT_SAVE_AREA
  /* USER CODE END fx_app_thread_entry 0 */

  /* Open the SD disk driver */
  sd_status =  fx_media_open(&sdio_disk, FX_SD_VOLUME_NAME, fx_stm32_sd_driver, (VOID *)FX_NULL, (VOID *) fx_sd_media_memory, sizeof(fx_sd_media_memory));

  /* Check the media open sd_status */
  if (sd_status != FX_SUCCESS)
  {
    /* USER CODE BEGIN SD open error */
    Error_Handler();
    /* USER CODE END SD open error */
  }

  /* USER CODE BEGIN fx_app_thread_entry 1 */
  /* Open the test file.  */
  sd_status =  fx_file_open(&sdio_disk, &fx_file, FW_NAME_STRING, FX_OPEN_FOR_READ);

  /* Check the file open status.  */
  if (sd_status != FX_SUCCESS)
  {
    /* Error opening file, call error handler.  */
    Error_Handler();
  }

  /* Seek to the beginning of the test file.  */
  sd_status =  fx_file_seek(&fx_file, 0);

  /* Check the file seek status.  */
  if (sd_status != FX_SUCCESS)
  {
    /* Error performing file seek, call error handler.  */
    Error_Handler();
  }

  /* Unlock flash access to start programming operations */
  flash_ret = HAL_FLASH_Unlock();

  /* Check the flash unlock status */
  if (flash_ret != HAL_OK)
  {
    Error_Handler();
  }

  /* Configure flash erase of sector 2 */
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  EraseInitStruct.Sector = FLASH_SECTOR_1;
  EraseInitStruct.NbSectors = 1;

  TX_DISABLE

  /* Execute erase operation */
  flash_ret = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);

  TX_RESTORE

  if (flash_ret != HAL_OK)
  {
    /* Error while erasing, call error handler */
    Error_Handler();
  }

  /* Start flash programming */
  do
  {

    /* Read FLASH_WORD_SIZE chunk from the exec into buffer.  */
    sd_status =  fx_file_read(&fx_file, (VOID *) read_buffer, FLASH_WORD_SIZE, &bytes_read);

    /* Check the file read status.  */
    if (sd_status != FX_SUCCESS)
    {
      /* We check if we reached EOF */
      if (sd_status == FX_END_OF_FILE)
      {
        break;
      }

      /* Error while reading file, call error handler.  */
      Error_Handler();
    }

    /* Program 32 bytes into flash */
    flash_ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, (uint32_t) FlashAddress, (uint32_t) read_buffer);

    /* Check the programming status */
    if (flash_ret != HAL_OK)
    {
      Error_Handler();
    }

    /* Increment the flash address by 32 bytes */
    FlashAddress = FlashAddress + FLASH_WORD_SIZE;

  }
  while (bytes_read);

  /* Flash programmed, can be locked again */
  flash_ret = HAL_FLASH_Lock();

  /* Check the flash lock status */
  if (flash_ret != HAL_OK)
  {
    Error_Handler();
  }

  /* Close the test file.  */
  sd_status =  fx_file_close(&fx_file);

  /* Check the file close status.  */
  if (sd_status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    Error_Handler();
  }

  /* Close the media.  */
  sd_status =  fx_media_close(&sdio_disk);

  /* Check the media close status.  */
  if (sd_status != FX_SUCCESS)
  {
    /* Error closing the media, call error handler.  */
    Error_Handler();
  }

  /* Toggle green LED to indicate programming finish OK */
  while (1)
  {
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    tx_thread_sleep(40);
  }
  /* USER CODE END fx_app_thread_entry 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
