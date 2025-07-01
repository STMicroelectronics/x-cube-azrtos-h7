
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

/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define FX_APP_STACK_SIZE       2*1024
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
FX_MEDIA sdio_disk;
FX_FILE fx_file;
TX_THREAD fx_app_thread;

UINT object_handles_counter;
UINT object_handle_index_num = 1;
ObjectHandleTypeDef ObjectHandleInfo[MTP_MAX_HANDLES];
ALIGN_32BYTES (uint32_t fx_sd_media_memory[FX_STM32_SD_DEFAULT_SECTOR_SIZE / sizeof(uint32_t)]);
extern HAL_SD_CardInfoTypeDef  pCardInfo;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */
VOID fx_thread_entry(ULONG thread_input);
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
  if (tx_byte_allocate(byte_pool, &pointer, FX_APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the main thread.  */
  if(tx_thread_create(&fx_app_thread, "FileX App Thread", fx_thread_entry,
                      0, pointer, FX_APP_STACK_SIZE, 20, 20,
                      TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

  /* USER CODE END MX_FileX_Init */

  /* Initialize FileX.  */
  fx_system_initialize();

  /* USER CODE BEGIN MX_FileX_Init 1*/

  /* USER CODE END MX_FileX_Init 1*/

  return ret;
}

/* USER CODE BEGIN 2 */

VOID fx_thread_entry(ULONG thread_input)
{
  /* Open SD media */
  SD_MediaOpen();

  SD_InitOperations();
}

/**
  * @brief  SD_InitOperations
  *         Start parse root directory in SD card
  * @param  none
  * @retval none
  */
VOID SD_InitOperations(VOID)
{
  UINT status = FX_SUCCESS;
  CHAR file_name[FX_MAX_LONG_NAME_LEN];
  UINT attributes, year, month, day, hour,  minute, second;
  ULONG size;

  /* Get the first directory entry in the default directory with full information */
  status = fx_directory_first_full_entry_find(&sdio_disk, file_name, &attributes, &size,
                                              &year, &month, &day, &hour, &minute, &second);

  while (status == FX_SUCCESS)
  {
    if ((attributes == FX_DIRECTORY) || (attributes == FX_ARCHIVE ))
    {
      /* Increment total object number counter */
      object_handles_counter += 1;

      /* Set handle info */
      Object_SetHandleInfo(&object_handle_index_num, file_name, size);

      /* Reset file name array */
      _fx_utility_memory_set((UCHAR*)file_name, 0, FX_MAX_LONG_NAME_LEN);
    }

    /* Get the next directory entry in the default directory with full information */
    status = fx_directory_next_full_entry_find(&sdio_disk, file_name, &attributes, &size,
                                               &year, &month, &day, &hour, &minute, &second);
  }
}

/**
  * @brief  SD_MediaOpen
  *         Open SD media
  * @param  none
  * @retval none
  */
VOID SD_MediaOpen(VOID)
{
  UINT status;

  /* Open the SD disk driver */
  status = fx_media_open(&sdio_disk, FX_SD_VOLUME_NAME, fx_stm32_sd_driver,
                         (VOID *)FX_NULL, (VOID *) fx_sd_media_memory,
                         sizeof(fx_sd_media_memory));

  if (status != FX_SUCCESS)
  {
    Error_Handler();
  }
}

/**
* @brief  SD_MediaClose
*         Close SD media
* @param  none
* @retval status
*/
UINT SD_MediaClose(VOID)
{
  UINT status;

  /* Close the SD disk driver */
  status = fx_media_close(&sdio_disk);

  return status;
}

/**
* @brief  SD_MediaFormat
*         Format SD media
* @param  none
* @retval status
*/
UINT SD_MediaFormat(VOID)
{
  UINT sd_status;

  /* Format the SD memory as FAT */
  sd_status =  fx_media_format(&sdio_disk,                          // SD_Disk pointer
                               fx_stm32_sd_driver,                  // Driver entry
                               (VOID *)FX_NULL,                     // Device info pointer
                               (UCHAR *) fx_sd_media_memory,        // Media buffer pointer
                               sizeof(fx_sd_media_memory),          // Media buffer size
                               FX_SD_VOLUME_NAME,                   // Volume Name
                               FX_SD_NUMBER_OF_FATS,                // Number of FATs
                               32,                                  // Directory Entries
                               FX_SD_HIDDEN_SECTORS,                // Hidden sectors
                               pCardInfo.BlockNbr,                  // Total sectors
                               FX_STM32_SD_DEFAULT_SECTOR_SIZE,     // Sector size
                               8,                                   // Sectors per cluster
                               1,                                   // Heads
                               1);                                  // Sectors per track

  return sd_status;

}

/**
  * @brief  Object_SetHandleInfo
  *         Add new object handle info in object in object info struct.
  * @param  obj_handle_index: Pointer to object handle index.
  * @param  file_name: object name.
  * @param  file_size: object size.
  * @retval status
  */
UINT Object_SetHandleInfo(UINT *obj_handle_index, CHAR *file_name, ULONG file_size)
{
  UINT status = MTP_ERROR;
  UINT handle_index, idx;
  static ULONG object_persistent_unique_identifier = 0;

  for (handle_index = 0U; handle_index < MTP_MAX_HANDLES; handle_index++)
  {
    /* Find an empty slot */
    if (ObjectHandleInfo[handle_index].object_handle_index == 0)
    {
      *obj_handle_index += 1;

      /* Fill Object Handle information */
      ObjectHandleInfo[handle_index].Path = "/";
      ObjectHandleInfo[handle_index].object_handle_index = *obj_handle_index;
      ObjectHandleInfo[handle_index].object_property.object_storage_id = 0x00010001U;
      ObjectHandleInfo[handle_index].object_property.object_file_full_name_length = strlen(file_name);

      for (idx = 0U; idx < ObjectHandleInfo[handle_index].object_property.object_file_full_name_length; idx++)
      {
        ObjectHandleInfo[handle_index].object_property.object_file_full_name[idx] = file_name[idx];
      }

      Object_GetNameFromFullName(file_name, ObjectHandleInfo[handle_index].object_property.object_file_name);

      ObjectHandleInfo[handle_index].object_property.object_file_name_length =
        strlen((CHAR*) ObjectHandleInfo[handle_index].object_property.object_file_name);

      ObjectHandleInfo[handle_index].object_property.object_format = Object_GetFormatFromName(file_name);
      ObjectHandleInfo[handle_index].object_property.object_protection_status = 0;
      ObjectHandleInfo[handle_index].object_property.object_size = file_size;
      ObjectHandleInfo[handle_index].object_property.object_parent_object = 0;
      ObjectHandleInfo[handle_index].object_property.object_identifier[0] = object_persistent_unique_identifier++;

      /* Set object file size */
      ObjectHandleInfo[handle_index].object_file.fx_file_current_file_size = file_size;

      status = MTP_SUCCESS;

      break;
    }
  }

  return status;
}

/**
  * @brief  Object_GetNameFromFullName
  *         Get object name from file name (with out file extension).
  * @param  full_filename: object full name.
  * @param  filename: object name.
  * @retval none
  */
VOID Object_GetNameFromFullName(CHAR *full_filename, UCHAR *filename)
{
  UINT PLocation, i, idx = 0U;

  i = strlen(full_filename) - 1;

  while (i && full_filename[i] != '.')
  {
    i--;
  }

  if (i != 0U)
  {
    PLocation = i;
  }
  else
  {
    PLocation = strlen(full_filename);
    i = strlen(full_filename) - 1;
  }

  while (i && full_filename[i] != '\\')
  {
    if (full_filename[i] != '\\')
      i--;
  }


  if (full_filename[i] == '\\')
  {
    i++;
  }


  while (i < PLocation)
  {
    filename[idx] = full_filename[i];
    idx++;
    i++;
  }

  filename[idx] = '\0';
}

/**
  * @brief  Object_GetFormatFromName
  *         Get object format (file extension) from file name.
  * @param  file_name: object full name.
  * @retval object format
  */
UINT Object_GetFormatFromName(CHAR *file_name)
{
  UINT objformat;
  CHAR FileExt[5];

  memset(FileExt, 0, sizeof(FileExt));

  CHAR* ext = strrchr(file_name, '.');

  /* Get file type */
  if (ext != 0U && (strlen(ext + 1) <= 5))
  {
    strcpy(FileExt, (ext + 1));
  }

  if ((strcmp(FileExt, "TXT") == 0) || (strcmp(FileExt, "txt") == 0))
  {
    objformat = 0x3004U;
  }
  else if ((strcmp(FileExt, "jpg") == 0) || (strcmp(FileExt, "JPG") == 0))
  {
    objformat = 0x3801U;
  }
  else if ((strcmp(FileExt, "MP4") == 0) || (strcmp(FileExt, "mp4") == 0))
  {
    objformat = 0xB982U;
  }
  else if ((strcmp(FileExt, "WAV") == 0) || (strcmp(FileExt, "wav") == 0))
  {
    objformat = 0x3008U;
  }
  else if ((strcmp(FileExt, "PDF") == 0) || (strcmp(FileExt, "pdf") == 0))
  {
    objformat = 0x3000U;
  }
  else
  {
    objformat = 0x3000U;
  }

  /* Return object format */
  return objformat;
}

/**
  * @brief  Object_GetHandlesIndex
  *         Get all object handle.
  * @param  Param3: current object handle
  * @param  obj_handle: all objects handle files in current object
  * @retval number of object handle in current object
  */
ULONG Object_GetHandlesIndex(ULONG Param3, ULONG *obj_handle)
{
  UINT index;
  ULONG object_handle_num = 0;

  if (Param3 == 0xFFFFFFFF)
  {
    for (index = 0; index < MTP_MAX_HANDLES; index++)
    {
      if (ObjectHandleInfo[index].object_handle_index != 0)
      {
        obj_handle[index] = ObjectHandleInfo[index].object_handle_index;
      }
    }

    /* Return number of object in Root folder */
    object_handle_num = object_handles_counter;
  }

  return object_handle_num;
}

/**
  * @brief  Object_HandleCheck
  *         Get object format (file extension) from file name.
  * @param  object_handle: object handle.
  * @param  handle_index: index of object.
  * @retval status
  */
UINT Object_HandleCheck(ULONG object_handle, ULONG *handle_index)
{
  UINT status = MTP_ERROR;
  UINT index;

  for (index = 0U; index < MTP_MAX_HANDLES; index++)
  {
    if ((ObjectHandleInfo[index].object_handle_index == object_handle)
        && (ObjectHandleInfo[index].object_handle_index != 0))
    {
      *handle_index = index;

      status = MTP_SUCCESS;

      break;
    }
  }

  return status;
}

/**
  * @brief  Object_GetHandleInfo
  *         Get object handle info
  * @param  handle_index: object handle index.
  * @param  object_handle_info: pointer to object info.
  * @retval status
  */
UINT Object_GetHandleInfo(ULONG handle_index, VOID **object_handle_info)
{

  *object_handle_info = &ObjectHandleInfo[handle_index].object_property;

  return MTP_SUCCESS;
}

/**
  * @brief  Object_GetHandleFile
  *         Get address of object handle fil.
  * @param  handle_index: index of object.
  * @param  mtp_object_file: pointer to fx_file of object.
  * @retval status
  */
UINT Object_GetHandleFile(ULONG handle_index, FX_FILE **mtp_object_file)
{

  *mtp_object_file = &ObjectHandleInfo[handle_index].object_file;

  return MTP_SUCCESS;
}

/* USER CODE END 2 */
