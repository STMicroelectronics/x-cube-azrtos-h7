/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_pima_mtp.h
  * @author  MCD Application Team
  * @brief   USBX Device PIMA MTP applicative header file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#ifndef __UX_DEVICE_PIMA_MTP_H__
#define __UX_DEVICE_PIMA_MTP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/* Includes ------------------------------------------------------------------*/
#include "ux_api.h"
#include "ux_device_class_pima.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_filex.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define MAX_OBJECT_HANDLE_LEN 100U
#define MAX_FILE_NAME         255U
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
VOID USBD_PIMA_MTP_Activate(VOID *pima_instance);
VOID USBD_PIMA_MTP_Deactivate(VOID *pima_instance);
UINT USBD_MTP_Cancel(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance);
UINT USBD_MTP_DeviceReset(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance);
UINT USBD_MTP_GetDevicePropDesc(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                                ULONG device_property,
                                UCHAR **device_prop_dataset,
                                ULONG *device_prop_dataset_length);
UINT USBD_MTP_GetDevicePropValue(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                                 ULONG device_property,
                                 UCHAR **device_prop_value,
                                 ULONG *device_prop_value_length);
UINT USBD_MTP_SetDevicePropValue(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                                 ULONG device_property,
                                 UCHAR *device_prop_value,
                                 ULONG device_prop_value_length);
UINT USBD_MTP_FormatStorage(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                            ULONG storage_id);
UINT USBD_MTP_GetStorageInfo(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                             ULONG storage_id);
UINT USBD_MTP_GetObjectNumber(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                              ULONG object_format_code,
                              ULONG object_association,
                              ULONG *object_number);
UINT USBD_MTP_GetObjectHandles(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                               ULONG object_handles_format_code,
                               ULONG object_handles_association,
                               ULONG *object_handles_array,
                               ULONG object_handles_max_number);
UINT USBD_MTP_GetObjectInfo(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                            ULONG object_handle,
                            UX_SLAVE_CLASS_PIMA_OBJECT **object);
UINT USBD_MTP_GetObjectData(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                            ULONG object_handle,
                            UCHAR *object_buffer,
                            ULONG object_offset,
                            ULONG object_length_requested,
                            ULONG *object_actual_length);
UINT USBD_MTP_SendObjectInfo(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                             UX_SLAVE_CLASS_PIMA_OBJECT *object,
                             ULONG storage_id, ULONG parent_object_handle,
                             ULONG *object_handle);
UINT USBD_MTP_SendObjectData(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                             ULONG object_handle, ULONG phase,
                             UCHAR *object_buffer, ULONG object_offset,
                             ULONG object_length);
UINT USBD_MTP_DeleteObject(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                           ULONG object_handle);
UINT USBD_MTP_GetObjectPropDesc(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                                ULONG object_property,
                                ULONG object_format_code,
                                UCHAR **object_prop_dataset,
                                ULONG *object_prop_dataset_length);
UINT USBD_MTP_GetObjectPropValue(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                                 ULONG object_handle,
                                 ULONG object_prop_code,
                                 UCHAR **object_prop_value,
                                 ULONG *object_prop_value_length);
UINT USBD_MTP_SetObjectPropValue(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                                 ULONG object_handle,
                                 ULONG object_prop_code,
                                 UCHAR *object_prop_value,
                                 ULONG object_prop_value_length);
UINT USBD_MTP_GetObjectReferences(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                                  ULONG object_handle,
                                  UCHAR **object_handle_array,
                                  ULONG *object_handle_array_length);
UINT USBD_MTP_GetObjectReferences(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                                  ULONG object_handle,
                                  UCHAR **object_handle_array,
                                  ULONG *object_handle_array_length);
UINT USBD_MTP_SetObjectReferences(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                                  ULONG object_handle,
                                  UCHAR *object_handle_array,
                                  ULONG object_handle_array_length);
ULONG USBD_MTP_StorageGetMaxCapabilityLow(VOID);
ULONG USBD_MTP_StorageGetMaxCapabilityHigh(VOID);
ULONG USBD_MTP_StorageGetFreeSpaceLow(VOID);
ULONG USBD_MTP_StorageGetFreeSpaceHigh(VOID);
ULONG USBD_MTP_StorageGetFreeSpaceImage(VOID);

/* USER CODE BEGIN EFP */
UINT MTP_GetObjectHandle(ULONG object_handle, UX_SLAVE_CLASS_PIMA_OBJECT **object);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */
static const uint16_t DeviceDefaultValue[] = {'S', 'T', 'M', '3', '2', 0};
static const uint16_t DeviceCurrentValue[] = {'S', 'T', 'M', '3', '2', ' ', 'V', '1', '.', '0', 0};
static const uint16_t DefaultFileName[] = {'N','e','w',' ','F','o','l','d','e','r',0};

#define DEFAULT_FILE_NAME_LEN    (uint8_t)(sizeof(DefaultFileName) / 2U)

typedef struct
{
  uint16_t DevicePropertyCode;
  uint16_t DataType;
  uint8_t GetSet;
  uint8_t DefaultValue_length;
  uint16_t DefaultValue[sizeof(DeviceDefaultValue)/ 2U];
  uint8_t CurrentValue_length;
  uint16_t CurrentValue[sizeof(DeviceCurrentValue)/ 2U];
  uint8_t FormFlag;
}__PACKED MTP_DevicePropDescTypeDef;

typedef struct
{
  ULONG ObjectHandle_len;
  ULONG ObjectHandle[MAX_OBJECT_HANDLE_LEN];
}MTP_ObjectHandleTypeDef;


typedef struct
{
  uint16_t ObjectPropertyCode;
  uint16_t DataType;
  uint8_t GetSet;
  uint8_t *DefValue;
  uint32_t GroupCode;
  uint8_t FormFlag;
}MTP_ObjectPropDescTypeDef;

typedef struct
{
  uint8_t FileName_len;
  uint16_t FileName[MAX_FILE_NAME];
} MTP_FileNameTypeDef;

/* USER CODE END PD */

#ifndef USBD_MTP_STORAGE_ID
#define USBD_MTP_STORAGE_ID  0x00010001U
#endif

#ifndef USBD_MTP_STORAGE_TYPE
#define USBD_MTP_STORAGE_TYPE  UX_DEVICE_CLASS_PIMA_STC_REMOVABLE_RAM
#endif

#ifndef USBD_MTP_STORAGE_FILE_SYSTEM_TYPE
#define USBD_MTP_STORAGE_FILE_SYSTEM_TYPE  UX_DEVICE_CLASS_PIMA_FSTC_GENERIC_FLAT
#endif

#ifndef USBD_MTP_STORAGE_FILE_ACCESS_CAPABILITY
#define USBD_MTP_STORAGE_FILE_ACCESS_CAPABILITY  UX_DEVICE_CLASS_PIMA_AC_READ_WRITE
#endif

#ifndef USBD_MTP_INFO_MANUFACTURER
#define USBD_MTP_INFO_MANUFACTURER  "STMicroelectronics"
#endif

#ifndef USBD_MTP_INFO_MODEL
#define USBD_MTP_INFO_MODEL  "STM32"
#endif

#ifndef USBD_MTP_INFO_VERSION
#define USBD_MTP_INFO_VERSION  "V1.00"
#endif

#ifndef USBD_MTP_INFO_SERIAL_NUMBER
#define USBD_MTP_INFO_SERIAL_NUMBER  "001"
#endif

#ifndef USBD_MTP_STORAGE_DESCRIPTION
#define USBD_MTP_STORAGE_DESCRIPTION  "STM32 MTP Storage Volume"
#endif

#ifndef USBD_MTP_STORAGE_DESCRIPTION_IDENTIFIER
#define USBD_MTP_STORAGE_DESCRIPTION_IDENTIFIER  "STM32 MTP Storage Label"
#endif

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

#ifdef __cplusplus
}
#endif
#endif  /* __UX_DEVICE_PIMA_MTP_H__ */
