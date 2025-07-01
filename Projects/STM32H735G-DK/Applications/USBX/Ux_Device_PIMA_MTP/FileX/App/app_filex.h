
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_filex.h
  * @author  MCD Application Team
  * @brief   FileX applicative header file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_FILEX_H__
#define __APP_FILEX_H__

#ifdef __cplusplus
extern "C" {
#endif

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/* Includes ------------------------------------------------------------------*/
#include "fx_api.h"
#include "fx_stm32_sd_driver.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define MTP_SUCCESS        0
#define MTP_ERROR          1
#define MTP_MAX_HANDLES    20
#define FX_SD_VOLUME_NAME       "STM32_SDIO_DISK"
#define FX_SD_NUMBER_OF_FATS    1
#define FX_SD_HIDDEN_SECTORS    0
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
UINT MX_FileX_Init(VOID *memory_ptr);
/* USER CODE BEGIN EFP */
VOID SD_MediaOpen(VOID);
UINT SD_MediaClose(VOID);
UINT SD_MediaFormat(VOID);
VOID SD_InitOperations(VOID);

UINT Object_SetHandleInfo(UINT *obj_handle_index, CHAR *file_name, ULONG file_size);
VOID Object_GetNameFromFullName(CHAR *full_filename, UCHAR *filename);
UINT Object_GetFormatFromName(CHAR *file_name);
ULONG Object_GetHandlesIndex(ULONG Param3, ULONG *obj_handle);
UINT Object_HandleCheck(ULONG object_handle, ULONG *handle_index);
UINT Object_GetHandleInfo(ULONG handle_index, VOID **object_handle_info);
UINT Object_GetHandleFile(ULONG handle_index, FX_FILE **mtp_object_file);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN PD */
/* Structure of the Object property dataset */
typedef struct OBJECT_PROP_VALUE_DATASET
{
  ULONG object_storage_id;
  ULONG object_format;
  ULONG object_protection_status;
  ULONG object_size;
  UINT  object_file_name_length;
  UCHAR object_file_name[FX_MAX_LONG_NAME_LEN];
  UINT  object_file_full_name_length;
  UCHAR object_file_full_name[FX_MAX_LONG_NAME_LEN];
  ULONG object_parent_object;
  ULONG object_identifier[4];
}ObjectPropertyDataSetTypeDef;

typedef struct OBJECT_HANDLE_INFO_STRUCT
{
  CHAR *Path;
  FX_FILE object_file;
  ULONG object_handle_index;
  ObjectPropertyDataSetTypeDef object_property;
}ObjectHandleTypeDef;

/* USER CODE END PD */

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
#ifdef __cplusplus
}
#endif
#endif /* __APP_FILEX_H__ */
