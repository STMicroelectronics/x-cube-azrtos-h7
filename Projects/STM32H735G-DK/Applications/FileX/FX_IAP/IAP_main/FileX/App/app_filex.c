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
#include "stm32h7xx_hal_flash.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
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
/* USER CODE BEGIN PV */
/* Buffer for FileX FX_MEDIA sector cache. this should be 32-Bytes
aligned to avoid cache maintenance issues */
ALIGN_32BYTES (uint32_t media_memory[FX_STM32_SD_DEFAULT_SECTOR_SIZE / sizeof(uint32_t)]);

/* Flash buffer*/
UINT *read_buffer;

/* Define FileX global data structures.  */
FX_MEDIA        sdio_disk;
FX_FILE         fx_file;
/* Define ThreadX global data structures.  */
TX_THREAD       fx_thread;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
VOID fx_thread_entry(ULONG thread_input);
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

  /* USER CODE BEGIN MX_FileX_MEM_POOL */
  /* USER CODE END MX_FileX_MEM_POOL */

  /* USER CODE BEGIN MX_FileX_Init */

  VOID *pointer;

  /* Allocate memory for the main thread's stack */
  ret = tx_byte_allocate(byte_pool, &pointer, DEFAULT_STACK_SIZE, TX_NO_WAIT);

  if (ret != FX_SUCCESS)
  {
    /* Failed at allocating memory */
    Error_Handler();
  }

  /* Create the main thread.  */
  tx_thread_create(&fx_thread, "fx_thread", fx_thread_entry, 0, pointer, DEFAULT_STACK_SIZE, DEFAULT_THREAD_PRIO,
                   DEFAULT_PREEMPTION_THRESHOLD, TX_NO_TIME_SLICE, TX_AUTO_START);

  /* Allocate memory for the flash buffer */
  ret = tx_byte_allocate(byte_pool, (VOID **) &read_buffer, FLASH_WORD_SIZE, TX_NO_WAIT);

  if (ret != FX_SUCCESS)
  {
    /* Failed at allocating memory */
    Error_Handler();
  }

  /* Initialize FileX.  */
  fx_system_initialize();

  /* USER CODE END MX_FileX_Init */
  return ret;
}

/* USER CODE BEGIN 1 */

static VOID os_delay(ULONG delay)
{
  ULONG start = tx_time_get();
  while ((tx_time_get() - start) < delay)
  {
  }
}

VOID fx_thread_entry(ULONG thread_input)
{
  UINT status;
  ULONG bytes_read = 0;
  ULONG FlashAddress = APP_ADDRESS;
  ULONG flash_ret;
  uint32_t SectorError;
  FLASH_EraseInitTypeDef EraseInitStruct;
  TX_INTERRUPT_SAVE_AREA

  /* Open the SD disk driver.  */
  status =  fx_media_open(&sdio_disk, "STM32_SDIO_DISK", fx_stm32_sd_driver, 0, (VOID *) media_memory, sizeof(media_memory));

  /* Check the media open status.  */
  if (status != FX_SUCCESS)
  {
    Error_Handler();
  }

  /* Open the test file.  */
  status =  fx_file_open(&sdio_disk, &fx_file, FW_NAME_STRING, FX_OPEN_FOR_READ);

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

  if(flash_ret != HAL_OK) {
    /* Error while erasing, call error handler */
    Error_Handler();
  }

  /* Start flash programming */
  do {

    /* Read FLASH_WORD_SIZE chunk from the exec into buffer.  */
    status =  fx_file_read(&fx_file, (VOID *) read_buffer, FLASH_WORD_SIZE, &bytes_read);

    /* Check the file read status.  */
    if (status != FX_SUCCESS)
    {
      /* We check if we reached EOF */
      if (status == FX_END_OF_FILE)
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

  } while (bytes_read);

  /* Flash programmed, can be locked again */
  flash_ret = HAL_FLASH_Lock();

  /* Check the flash lock status */
  if (flash_ret != HAL_OK)
  {
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

  /* Toggle green LED to indicate programming finish OK */
  while(1)
  {
    BSP_LED_Toggle(LED_GREEN);
    os_delay(40);
  }
}

/* USER CODE END 1 */
