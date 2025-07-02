
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
#define DEFAULT_SECTOR_SIZE              512
#define FILEX_DEFAULT_STACK_SIZE         (3 * 1024)

/* Thread_0 priority */
#define DEFAULT_THREAD_PRIO              10

/* Thread_0 preemption priority */
#define DEFAULT_PREEMPTION_THRESHOLD      DEFAULT_THREAD_PRIO

/* fx media buffer of size equals a one sector */
#define DEFAULT_MEDIA_BUF_LENGTH         DEFAULT_SECTOR_SIZE

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Main thread global data structures.  */
TX_THREAD       fx_app_thread;

/* Buffer for FileX FX_MEDIA sector cache. */
uint32_t fx_nor_qspi_media_memory[FX_NOR_QSPI_SECTOR_SIZE / sizeof(uint32_t)];
/* Define FileX global data structures.  */
FX_MEDIA        nor_qspi_flash_disk;

/* USER CODE BEGIN PV */

/* Define FileX global data structures.  */
FX_FILE         fx_file;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* Main thread entry function.  */
void fx_app_thread_entry(ULONG thread_input);

/* USER CODE BEGIN PFP */

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
  UINT nor_qspi_status = FX_SUCCESS;
  /* USER CODE BEGIN fx_app_thread_entry 0 */
  ULONG available_space_pre;
  ULONG available_space_post;
  ULONG bytes_read;
  CHAR read_buffer[32];
  CHAR data[] = "This is FileX working on STM32";

  printf("[Cortex-M4]: FileX/LevelX NOR QUAD-SPI Application Start.\n");

  /* Print the absolute size of the NOR chip*/
  printf("[Cortex-M4]: Total NOR Flash Chip size is: %lu bytes.\n", (unsigned long)LX_STM32_QSPI_FLASH_SIZE);

  /* USER CODE END fx_app_thread_entry 0 */

  /* Format the QUAD-SPI NOR flash as FAT */
  nor_qspi_status =  fx_media_format(&nor_qspi_flash_disk,                                                               // nor_qspi_flash_disk pointer
                                     fx_stm32_levelx_nor_driver,                                                         // Driver entry
                                     (VOID *)LX_NOR_QSPI_DRIVER_ID,                                                      // Device info pointer
                                     (UCHAR *) fx_nor_qspi_media_memory,                                                 // Media buffer pointer
                                     sizeof(fx_nor_qspi_media_memory),                                                   // Media buffer size
                                     FX_NOR_QSPI_VOLUME_NAME,                                                            // Volume Name
                                     FX_NOR_QSPI_NUMBER_OF_FATS,                                                         // Number of FATs
                                     32,                                                                                 // Directory Entries
                                     FX_NOR_QSPI_HIDDEN_SECTORS,                                                         // Hidden sectors
                                     ((LX_STM32_QSPI_FLASH_SIZE - LX_STM32_QSPI_SECTOR_SIZE) / FX_NOR_QSPI_SECTOR_SIZE), // Total sectors minus one
                                     FX_NOR_QSPI_SECTOR_SIZE,                                                            // Sector size
                                     8,                                                                                  // Sectors per cluster
                                     1,                                                                                  // Heads
                                     1);                                                                                 // Sectors per track

  /* Check the format nor_qspi_status */
  if (nor_qspi_status != FX_SUCCESS)
  {
    /* USER CODE BEGIN QUAD-SPI NOR format error */
    while(1);
    /* USER CODE END QUAD-SPI NOR format error */
  }

  /* Open the QUAD-SPI NOR driver */
  nor_qspi_status =  fx_media_open(&nor_qspi_flash_disk, FX_NOR_QSPI_VOLUME_NAME, fx_stm32_levelx_nor_driver, (VOID *)LX_NOR_QSPI_DRIVER_ID, (VOID *) fx_nor_qspi_media_memory, sizeof(fx_nor_qspi_media_memory));

  /* Check the media open nor_qspi_status */
  if (nor_qspi_status != FX_SUCCESS)
  {
    /* USER CODE BEGIN QUAD-SPI NOR open error */
    while(1);
    /* USER CODE END QUAD-SPI NOR open error */
  }

  /* USER CODE BEGIN fx_app_thread_entry 1 */

  /* Get the available usable space */
  nor_qspi_status =  fx_media_space_available(&nor_qspi_flash_disk, &available_space_pre);

  printf("[Cortex-M4]: User available NOR Flash disk space size before file is written: %lu bytes.\n", available_space_pre);

  /* Check the get available state request status.  */
  if (nor_qspi_status != FX_SUCCESS)
  {
    Error_Handler();
  }

  /* Create a file called STM32.TXT in the root directory.  */
  nor_qspi_status =  fx_file_create(&nor_qspi_flash_disk, "STM32.TXT");

  /* Check the create status.  */
  if (nor_qspi_status != FX_SUCCESS)
  {
    /* Check for an already created status. This is expected on the
    second pass of this loop!  */
    if (nor_qspi_status != FX_ALREADY_CREATED)
    {
      /* Create error, call error handler.  */
      Error_Handler();
    }
  }

  /* Open the test file.  */
  nor_qspi_status =  fx_file_open(&nor_qspi_flash_disk, &fx_file, "STM32.TXT", FX_OPEN_FOR_WRITE);

  /* Check the file open status.  */
  if (nor_qspi_status != FX_SUCCESS)
  {
    /* Error opening file, call error handler.  */
    Error_Handler();
  }

  /* Seek to the beginning of the test file.  */
  nor_qspi_status =  fx_file_seek(&fx_file, 0);

  /* Check the file seek status.  */
  if (nor_qspi_status != FX_SUCCESS)
  {
    /* Error performing file seek, call error handler.  */
    Error_Handler();
  }

  nor_qspi_status =  fx_file_write(&fx_file, data, sizeof(data));

  /* Check the file write status.  */
  if (nor_qspi_status != FX_SUCCESS)
  {
    /* Error writing to a file, call error handler.  */
    Error_Handler();
  }

  /* Close the test file.  */
  nor_qspi_status =  fx_file_close(&fx_file);

  /* Check the file close status.  */
  if (nor_qspi_status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    Error_Handler();
  }

  nor_qspi_status = fx_media_flush(&nor_qspi_flash_disk);

  /* Check the media flush  status.  */
  if (nor_qspi_status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    Error_Handler();
  }

  /* Open the test file.  */
  nor_qspi_status =  fx_file_open(&nor_qspi_flash_disk, &fx_file, "STM32.TXT", FX_OPEN_FOR_READ);

  /* Check the file open status.  */
  if (nor_qspi_status != FX_SUCCESS)
  {
    /* Error opening file, call error handler.  */
    Error_Handler();
  }

  /* Seek to the beginning of the test file.  */
  nor_qspi_status =  fx_file_seek(&fx_file, 0);

  /* Check the file seek status.  */
  if (nor_qspi_status != FX_SUCCESS)
  {
    /* Error performing file seek, call error handler.  */
    Error_Handler();
  }

  /* Read the first 31 bytes of the test file.  */
  nor_qspi_status =  fx_file_read(&fx_file, read_buffer, sizeof(data), &bytes_read);

  /* Check the file read status.  */
  if ((nor_qspi_status != FX_SUCCESS) || (bytes_read != sizeof(data)))
  {
    /* Error reading file, call error handler.  */
    Error_Handler();
  }

  /* Close the test file.  */
  nor_qspi_status =  fx_file_close(&fx_file);

  /* Check the file close status.  */
  if (nor_qspi_status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    Error_Handler();
  }

  /* Get the available usable space, after the file has been created */
  nor_qspi_status =  fx_media_space_available(&nor_qspi_flash_disk, &available_space_post);

  printf("[Cortex-M4]: User available NOR Flash disk space size after file is written: %lu bytes.\n", available_space_post);
  printf("[Cortex-M4]: The test file occupied a total of %lu cluster(s) (%u per cluster).\n",
         (available_space_pre - available_space_post) / (nor_qspi_flash_disk.fx_media_bytes_per_sector * nor_qspi_flash_disk.fx_media_sectors_per_cluster),
         nor_qspi_flash_disk.fx_media_bytes_per_sector * nor_qspi_flash_disk.fx_media_sectors_per_cluster);

  /* Check the get available state request status.  */
  if (nor_qspi_status != FX_SUCCESS)
  {
    Error_Handler();
  }

  /* Close the media.  */
  nor_qspi_status =  fx_media_close(&nor_qspi_flash_disk);

  /* Check the media close status.  */
  if (nor_qspi_status != FX_SUCCESS)
  {
    /* Error closing the media, call error handler.  */
    Error_Handler();
  }

  while(1)
  {
    HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
    tx_thread_sleep(40);
  }
  /* USER CODE END fx_app_thread_entry 1 */
}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
