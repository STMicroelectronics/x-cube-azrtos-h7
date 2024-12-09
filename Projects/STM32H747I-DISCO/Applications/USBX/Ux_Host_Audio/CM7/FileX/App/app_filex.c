
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_filex.c
  * @author  MCD Application Team
  * @brief   FileX applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "app_usbx_host.h"
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
FX_FILE         fx_play_file;
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
  UINT sd_status = FX_SUCCESS;
  /* USER CODE BEGIN fx_app_thread_entry 0 */

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

  /* USER CODE END fx_app_thread_entry 1 */
}

/* USER CODE BEGIN 1 */
/**
  * @brief  SD_StorageParse
  *         Copies disk content in the explorer list.
  * @param  file_list
  * @retval Status
  */
VOID SD_StorageParse(FileListTypeDef *file_list)
{
  UINT file_exist;
  CHAR object_name[64];
  ULONG object_size;

  file_list->ptr = 0;

  USBH_UsrLog("audio file(s) [ROOT]:\n");

  do
  {
    /* Reset object name */
    _fx_utility_memory_set((UCHAR*)object_name, 0, 64);

    /* Retrieve the next entry in the default directory */
    file_exist = fx_directory_next_full_entry_find(&sdio_disk, object_name, FX_NULL, &object_size, FX_NULL,
                                                   FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL);

    if (file_exist != FX_SUCCESS)
    {
      USBH_UsrLog("\nEnd of files list.\n");

      break;
    }

    if ((strstr(object_name, "wav")) || (strstr(object_name, "WAV")))
    {
      strncpy((char *)file_list->file[file_list->ptr].name, (char *)object_name, FX_MAX_LONG_NAME_LEN);
      file_list->file[file_list->ptr].type = 1;
      file_list->file[file_list->ptr].size = object_size;

      /* Shows audio file (*.wav) on the root */
      USBH_UsrLog("%s", (char *)file_list->file[file_list->ptr].name);

      /* Increment total file number */
      file_list->ptr++;
    }

  }while(file_exist == FX_SUCCESS);

  if (file_list->ptr == 0U)
  {
    USBH_UsrLog("There is no WAV file on the microSD.\n");
  }
}

/**
  * @brief  AUDIO_GetFileInfo
  *         Get information from wav file header.
  * @param  file_idx: file index
  * @param  file_list: file list struct
  * @param  info: wav file info
  * @retval status
  */
VOID AUDIO_GetFileInfo(uint16_t file_idx, FileListTypeDef *file_list, WAV_InfoTypedef *info)
{
  ULONG bytes_read;

  /* Open file for read */
  if (fx_file_open(&sdio_disk, &fx_play_file, (char *)file_list->file[file_idx].name,
                   FX_OPEN_FOR_READ) != FX_SUCCESS)
  {
    while(1);
  }

  /* Get WAV header */
  if (fx_file_read(&fx_play_file, info, sizeof(WAV_InfoTypedef), &bytes_read) != FX_SUCCESS)
  {
    while(1);
  }
}

/**
  * @brief  AUDIO_ReadFromWAVFile
  *         Read audio frame from wav file.
  * @param  destination: pointer to destination buffer.
  * @param  read_size: size to read.
  * @param  requested_length: requested length.
  * @param  offset: data offset.
  * @param  info: wav file info.
  * @retval none
  */
VOID AUDIO_ReadFromWAVFile(UCHAR *destination, ULONG read_size, ULONG *requested_length,
                           ULONG *offset, WAV_InfoTypedef *info)
{
  ULONG actual = 0;
  ULONG actual_bytes;

  if (*offset + read_size > info->FileSize)
  {
    actual = info->FileSize - *offset;
  }
  else
  {
    actual = read_size;
  }

  /* check if audio wav have 2 channels */
  if (info->NbrChannels == 2U)
  {
    fx_file_seek(&fx_play_file, *offset);

    /* Read audio frame from wav file */
    if (fx_file_read(&fx_play_file, destination, actual, &actual_bytes) == FX_END_OF_FILE)
    {
      AUDIO_EndOfWAVFile();
    }

    /* Update data offset */
    *offset += actual;
  }

  if (requested_length != 0U)
  {
    /* Update requested length */
    *requested_length = actual;
  }
}

/**
  * @brief  AUDIO_EndOfWAVFile
  *         Close audio file.
  * @param  none
  * @retval none
  */
VOID AUDIO_EndOfWAVFile(VOID)
{
  if (fx_file_close(&fx_play_file) != FX_SUCCESS)
  {
    while(1);
  }
}
/* USER CODE END 1 */
