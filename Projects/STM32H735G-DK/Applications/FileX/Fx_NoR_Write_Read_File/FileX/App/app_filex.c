/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_filex.c
  * @author  MCD Application Team
  * @brief   FileX applicative file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "app_filex.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define DEFAULT_STACK_SIZE               (2 * 1024)
/* Thread_0 priority */
#define DEFAULT_THREAD_PRIO              10
/* Thread_0 preemption priority */
#define DEFAULT_PREEMPTION_THRESHOLD      DEFAULT_THREAD_PRIO

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* Buffer for FileX FX_MEDIA sector cache. this should be 32-Bytes
aligned to avoid cache maintenance issues */
ALIGN_32BYTES (UCHAR media_memory[1024]);
/* Define FileX global data structures.  */
FX_MEDIA        nor_flash_disk;
FX_FILE         fx_file;
/* Define ThreadX global data structures.  */
TX_THREAD       fx_thread;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

void fx_thread_entry(ULONG thread_input);
void Error_Handler(void);

/* USER CODE END PFP */

/* Global user code ---------------------------------------------------------*/
/* USER CODE BEGIN Global_User_Code */
static VOID os_delay(ULONG delay)
{
  ULONG start = tx_time_get();
  while ((tx_time_get() - start) < delay) {}
}
/* USER CODE END Global_User_Code */

/**
  * @brief  Application FileX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_FileX_Init(VOID *memory_ptr)
{
    UINT ret = FX_SUCCESS;
    /* USER CODE BEGIN App_FileX_Init */
    
    /* Create the main thread.  */
    tx_thread_create(&fx_thread, "thread 0", fx_thread_entry, 0,
                     memory_ptr, DEFAULT_STACK_SIZE,
                     DEFAULT_THREAD_PRIO,
                     DEFAULT_PREEMPTION_THRESHOLD,
                     TX_NO_TIME_SLICE, TX_AUTO_START);
    
    
    /* Initialize FileX.  */
    fx_system_initialize();
  
    /* USER CODE END App_FileX_Init */

    return ret;
}

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN Private_User_Code */

/* USER CODE END Private_User_Code */

/* USER CODE BEGIN 1 */

void fx_thread_entry(ULONG thread_input)
{

  UINT status;
  ULONG bytes_read;
  ULONG available_space_pre;
  ULONG available_space_post;
  CHAR read_buffer[32];
  CHAR data[] = "This is FileX working on STM32";
  BSP_OSPI_NOR_Info_t ospi_info;
  
  printf("FileX/LevelX NOR OCTO-SPI Application Start.\n");
  
  /* Get NOR chip info */
  if(BSP_OSPI_NOR_GetInfo(OSPI_INSTANCE, &ospi_info) != BSP_ERROR_NONE)
  {
    while(1);
  }
  
  /* Print the absolute size of the NOR chip*/
  printf("Total NOR Flash Chip size is: %lu bytes.\n", ospi_info.FlashSize);
  
  /* Format the NOR flash as FAT */
  status =  fx_media_format(&nor_flash_disk,
                            fx_stm32_levelx_nor_driver,   // Driver entry
                            (VOID*)NOR_DEFAULT_DRIVER,    // Device info pointer
                            media_memory,                 // Media buffer pointer
                            sizeof(media_memory),         // Media buffer size
                            "NOR_FLASH_DISK",             // Volume Name
                            1,                            // Number of FATs
                            32,                           // Directory Entries
                            0,                            // Hidden sectors
                            ospi_info.FlashSize/512,      // Total sectors
                            512,                          // Sector size
                            8,                            // Sectors per cluster
                            1,                            // Heads
                            1);                           // Sectors per track
  
  /* Check if the format status */
  if (status != FX_SUCCESS)
  {
    Error_Handler();
  }
  
  /* Open the OCTO-SPI NOR Flash disk driver.  */
  status =  fx_media_open(&nor_flash_disk, "FX_LX_NOR_DISK", fx_stm32_levelx_nor_driver,(VOID*)LX_NOR_OSPI_DRIVER_ID , media_memory, sizeof(media_memory));

  /* Check the media open status.  */
  if (status != FX_SUCCESS)
  {
    Error_Handler();
  }
  
  /* Get the available usable space */
  status =  fx_media_space_available(&nor_flash_disk, &available_space_pre);
  
  printf("User available NOR Flash disk space size before file is written: %lu bytes.\n", available_space_pre);
  
  /* Check the get available state request status.  */
  if (status != FX_SUCCESS)
  {
    Error_Handler();
  }

  /* Create a file called STM32.TXT in the root directory.  */
  status =  fx_file_create(&nor_flash_disk, "STM32.TXT");

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
  status =  fx_file_open(&nor_flash_disk, &fx_file, "STM32.TXT", FX_OPEN_FOR_WRITE);

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

  status = fx_media_flush(&nor_flash_disk);

  /* Check the media flush  status.  */
  if (status != FX_SUCCESS)
  {
    /* Error closing the file, call error handler.  */
    Error_Handler();
  }

   /* Open the test file.  */
  status =  fx_file_open(&nor_flash_disk, &fx_file, "STM32.TXT", FX_OPEN_FOR_READ);

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
  status =  fx_media_space_available(&nor_flash_disk, &available_space_post);
  
  printf("User available NOR Flash disk space size after file is written: %lu bytes.\n", available_space_post);
  printf("The test file occupied a total of %lu cluster(s) (%u per cluster).",
         (available_space_pre - available_space_post) / (nor_flash_disk.fx_media_bytes_per_sector * nor_flash_disk.fx_media_sectors_per_cluster),
         nor_flash_disk.fx_media_bytes_per_sector * nor_flash_disk.fx_media_sectors_per_cluster);
  
  /* Check the get available state request status.  */
  if (status != FX_SUCCESS)
  {
    Error_Handler();
  }

  /* Close the media.  */
  status =  fx_media_close(&nor_flash_disk);

  /* Check the media close status.  */
  if (status != FX_SUCCESS)
  {
    /* Error closing the media, call error handler.  */
    Error_Handler();
  }

  while(1)
  {
      BSP_LED_Toggle(LED1);
      os_delay(40);
  }

}

/* USER CODE END 1 */
