
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
#define FX_APP_THREAD_STACK_SIZE         (2 * 1024)
/* Main thread priority */
#define FX_APP_THREAD_PRIO               1

/* USER CODE BEGIN PD */
/* Sample Applications thread's stack size */
#define FILEX_DEFAULT_STACK_SIZE                        (2 * 1024)

/* Fx_Nand_Disk_Thread priority */
#define DEFAULT_THREAD_PRIO                        1

/* Fx_Nand_Disk_Thread preemption priority */
#define DEFAULT_PREEMPTION_THRESHOLD               1

/* Hidden sectors */
#define FX_NAND_DISK_HIDDEN_SECTORS                0

/* Heads */
#define FX_NAND_DISK_HEADS                         1

/* Sectors per track */
#define FX_NAND_DISK_SECTORS_PER_TRACK             1

/* Length of the message to be written in the file */
#define FX_NAND_DISK_LENGTH                        28

/* Total sectors */
#define FX_NAND_DISK_TOTAL_SECTORS                (((TOTAL_BLOCKS-1) * PHYSICAL_PAGES_PER_BLOCK * BYTES_PER_PHYSICAL_PAGE)/ FX_NAND_DISK_SECTOR_SIZE)

/* Number of FATs */
#define FX_NAND_DISK_NUMBER_OF_FATS                1

/* Directory Entries */
#define FX_NAND_DISK_DIRECTORY_ENTRIES             32

/* Sector size */
#define FX_NAND_DISK_SECTOR_SIZE                   BYTES_PER_PHYSICAL_PAGE /* sector size is always the page size of the underlying NAND hardware */

/* Sectors per cluster */
#define FX_NAND_DISK_SECTORS_PER_CLUSTER           1

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Main thread global data structures.  */
TX_THREAD       fx_app_thread;

/* USER CODE BEGIN PV */

/* Buffer for FileX FX_MEDIA sector cache. this should be 32-Bytes
aligned to avoid cache maintenance issues */
ALIGN_32BYTES(UCHAR media_memory[2048]);

/* Define FileX global data structures.  */
FX_MEDIA        nand_flash_disk;
FX_FILE         fx_file;

/* Define ThreadX global data structures.  */
TX_THREAD       fx_thread;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* Main thread entry function.  */
void fx_app_thread_entry(ULONG thread_input);

/* USER CODE BEGIN PFP */

void fx_thread_entry(ULONG thread_input);
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
  /* USER CODE BEGIN fx_app_thread_entry 0 */
  UINT status;
  ULONG bytes_read;
  ULONG available_space_pre;
  ULONG available_space_post;
  CHAR read_buffer[32];
  CHAR data[] = "This is FileX working on STM32";

  printf("FileX/LevelX NAND-FMC Application Start.\n");

  /* Print the absolute size of the NAND chip*/
  printf("Total NAND Flash Chip size is: %u bytes.\n", NAND_FLASH_SIZE);

  /* Format the NAND flash as FAT */
  status =  fx_media_format(&nand_flash_disk,
                            fx_stm32_levelx_nand_driver,         /* Driver entry */
                            (VOID *)CUSTOM_DRIVER_ID,          /* NAND disk memory pointer */
                            media_memory,                        /* Media buffer pointer */
                            sizeof(media_memory),                /* Media buffer size */
                            "NAND_FLASH_DISK",                   /* Volume Name */
                            FX_NAND_DISK_NUMBER_OF_FATS,         /* Number of FATs */
                            FX_NAND_DISK_DIRECTORY_ENTRIES,      /* Directory Entries */
                            FX_NAND_DISK_HIDDEN_SECTORS,         /* Hidden sectors */
                            FX_NAND_DISK_TOTAL_SECTORS,          /* Total sectors */
                            FX_NAND_DISK_SECTOR_SIZE,            /* Sector size */
                            FX_NAND_DISK_SECTORS_PER_CLUSTER,    /* Sectors per cluster */
                            FX_NAND_DISK_HEADS,                  /* Heads */
                            FX_NAND_DISK_SECTORS_PER_TRACK);     /* Sectors per track */

  /* Check if the format status */
  if (status != FX_SUCCESS)
  {
    Error_Handler();
  }

  /* Open the FMC NAND Flash disk driver.  */
  status =  fx_media_open(&nand_flash_disk, "FX_LX_NAND_DISK", fx_stm32_levelx_nand_driver, (VOID *)CUSTOM_DRIVER_ID, media_memory, sizeof(media_memory));

  /* Check the media open status.  */
  if (status != FX_SUCCESS)
  {
    Error_Handler();
  }

  /* Get the available usable space */
  status =  fx_media_space_available(&nand_flash_disk, &available_space_pre);

  printf("User available NAND Flash disk space size before file is written: %lu bytes.\n", available_space_pre);

  /* Check the get available state request status.  */
  if (status != FX_SUCCESS)
  {
    Error_Handler();
  }

  /* Create a file called STM32.TXT in the root directory.  */
  status =  fx_file_create(&nand_flash_disk, "STM32.TXT");

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
  status =  fx_file_open(&nand_flash_disk, &fx_file, "STM32.TXT", FX_OPEN_FOR_WRITE);

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

  /* Check the media flush  status.  */
  if (status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    Error_Handler();
  }

  /* Open the test file.  */
  status =  fx_file_open(&nand_flash_disk, &fx_file, "STM32.TXT", FX_OPEN_FOR_READ);

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

  /* Get the available usable space, after the file has been created */
  status =  fx_media_space_available(&nand_flash_disk, &available_space_post);

  printf("User available NAND Flash disk space size after file is written: %lu bytes.\n", available_space_post);
  printf("The test file occupied a total of %lu cluster(s) (%u per cluster).\n",
         (available_space_pre - available_space_post) / (nand_flash_disk.fx_media_bytes_per_sector * nand_flash_disk.fx_media_sectors_per_cluster),
         nand_flash_disk.fx_media_bytes_per_sector * nand_flash_disk.fx_media_sectors_per_cluster);

  /* Check the get available state request status.  */
  if (status != FX_SUCCESS)
  {
    Error_Handler();
  }

  /* Close the media.  */
  status =  fx_media_close(&nand_flash_disk);

  /* Check the media close status.  */
  if (status != FX_SUCCESS)
  {
    /* Error closing the media, call error handler.  */
    Error_Handler();
  }

  while (1)
  {
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    tx_thread_sleep(40);
  }
  /* USER CODE END fx_app_thread_entry 0 */

  /* USER CODE BEGIN fx_app_thread_entry 1 */

  /* USER CODE END fx_app_thread_entry 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
