/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_pima_mtp.c
  * @author  MCD Application Team
  * @brief   USBX Device PIMA MTP applicative source file
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

/* Includes ------------------------------------------------------------------*/
#include "ux_device_pima_mtp.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define USBD_MTP_SUPPORTED_PROP   \
  UX_DEVICE_CLASS_PIMA_OBJECT_PROP_STORAGEID, \
  UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FORMAT, \
  UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PROTECTION_STATUS, \
  UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_SIZE, \
  UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FILE_NAME, \
  UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PARENT_OBJECT, \
  UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PERSISTENT_UNIQUE_OBJECT_IDENTIFIER, \
  UX_DEVICE_CLASS_PIMA_OBJECT_PROP_NAME

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* Define PIMA supported device properties */
USHORT USBD_MTP_DevicePropSupported[] = {
  /* USER CODE BEGIN USBD_MTP_DevicePropSupported */
  UX_DEVICE_CLASS_PIMA_DEV_PROP_DEVICE_FRIENDLY_NAME,
  /* USER CODE END USBD_MTP_DevicePropSupported */
  0
};

/* Define PIMA supported capture formats */
USHORT USBD_MTP_DeviceSupportedCaptureFormats[] = {
  /* USER CODE BEGIN USBD_MTP_DeviceSupportedCaptureFormats */
  UX_DEVICE_CLASS_PIMA_OFC_UNDEFINED,
  UX_DEVICE_CLASS_PIMA_OFC_ASSOCIATION,
  UX_DEVICE_CLASS_PIMA_OFC_TEXT,
  /* USER CODE END USBD_MTP_DeviceSupportedCaptureFormats */
  0
};

/* Define PIMA supported image formats */
USHORT USBD_MTP_DeviceSupportedImageFormats[] = {
  /* USER CODE BEGIN USBD_MTP_DeviceSupportedImageFormats */
  UX_DEVICE_CLASS_PIMA_OFC_UNDEFINED,
  UX_DEVICE_CLASS_PIMA_OFC_MP3,
  UX_DEVICE_CLASS_PIMA_OFC_WAV,
  UX_DEVICE_CLASS_PIMA_OFC_TEXT,
  UX_DEVICE_CLASS_PIMA_OFC_ASF,
  UX_DEVICE_CLASS_PIMA_OFC_EXIF_JPEG,
  UX_DEVICE_CLASS_PIMA_OFC_WMA,
  UX_DEVICE_CLASS_PIMA_OFC_WMV,
  UX_DEVICE_CLASS_PIMA_OFC_MP4_CONTAINER,
  /* USER CODE END USBD_MTP_DeviceSupportedImageFormats */
  0
};

/* Object property supported
   WORD 0    : Object Format Code
   WORD 1    : Number of Prop codes for this Object format
   WORD n    : Prop Codes
   WORD n+2  : Next Object Format code ....
*/
USHORT USBD_MTP_ObjectPropSupported[] = {
  /* USER CODE BEGIN USBD_MTP_ObjectPropSupported */

  /* Object format code : Undefined */
  UX_DEVICE_CLASS_PIMA_OFC_UNDEFINED,
  /* NUmber of objects supported for this format */
  8,
  /* Mandatory objects for all formats */
  USBD_MTP_SUPPORTED_PROP,

  /* Object format code : Text */
  UX_DEVICE_CLASS_PIMA_OFC_TEXT,
  /* NUmber of objects supported for this format */
  8,
  /* Mandatory objects for all formats */
  USBD_MTP_SUPPORTED_PROP,

  /* Object format code : JPEG.  */
  UX_DEVICE_CLASS_PIMA_OFC_EXIF_JPEG,
  /* NUmber of objects supported for this format */
  8,
  /* Mandatory objects for all formats */
  USBD_MTP_SUPPORTED_PROP,

  /* Object format code : Advanced System Format */
  UX_DEVICE_CLASS_PIMA_OFC_ASF,
  /* NUmber of objects supported for this format */
  8,
  /* Mandatory objects for all formats */
  USBD_MTP_SUPPORTED_PROP,

  /* Object format code : Audio Clip */
  UX_DEVICE_CLASS_PIMA_OFC_MP3,
  /* NUmber of objects supported for this format */
  8,
  /* Mandatory objects for all formats */
  USBD_MTP_SUPPORTED_PROP,

  /* Object format code : Audio Wav */
  UX_DEVICE_CLASS_PIMA_OFC_WAV,
  /* NUmber of objects supported for this format */
  8,
  /* Mandatory objects for all formats */
  USBD_MTP_SUPPORTED_PROP,

  /* Object format code : Windows Media Audio Clip */
  UX_DEVICE_CLASS_PIMA_OFC_WMA,
  /* NUmber of objects supported for this format */
  8,
  /* Mandatory objects for all formats */
  USBD_MTP_SUPPORTED_PROP,

  /* Object format code : Windows Media Video */
  UX_DEVICE_CLASS_PIMA_OFC_WMV,
  /* NUmber of objects supported for this format */
  8,
  /* Mandatory objects for all formats */
  USBD_MTP_SUPPORTED_PROP,

  /* Object format code : Windows Media Video */
  UX_DEVICE_CLASS_PIMA_OFC_MP4_CONTAINER,
  /* NUmber of objects supported for this format */
  8,
  /* Mandatory objects for all formats */
  USBD_MTP_SUPPORTED_PROP,

  /* USER CODE END USBD_MTP_ObjectPropSupported */
  0
};

/* USER CODE BEGIN PV */
extern FX_MEDIA sdio_disk;

static MTP_ObjectHandleTypeDef    MTP_ObjectHandle;
static MTP_DevicePropDescTypeDef  MTP_DeviceFriendlyName;
static MTP_ObjectPropDescTypeDef  MTP_ObjectPropDesc;
static UX_SLAVE_CLASS_PIMA_OBJECT MTP_Object;
static MTP_FileNameTypeDef        MTP_FileName;

extern UINT object_handles_counter;
extern UINT object_handle_index_num;
extern ObjectHandleTypeDef ObjectHandleInfo[MTP_MAX_HANDLES];
UCHAR object_property_dataset_data_buffer[1024];
UCHAR device_friendly_name_data_buffer[1024];

extern uint32_t media_memory[FX_STM32_SD_DEFAULT_SECTOR_SIZE / sizeof(uint32_t)];
extern HAL_SD_CardInfoTypeDef sdcard_info;

ULONG references_array;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  USBD_PIMA_MTP_Activate
  *         This function is called when insertion of a PIMA MTP device.
  * @param  pima_instance: Pointer to the pima class instance.
  * @retval none
  */
VOID USBD_PIMA_MTP_Activate(VOID *pima_instance)
{
  /* USER CODE BEGIN USBD_PIMA_MTP_Activate */
  UX_PARAMETER_NOT_USED(pima_instance);
  /* USER CODE END USBD_PIMA_MTP_Activate */

  return;
}

/**
  * @brief  USBD_PIMA_MTP_Deactivate
  *         This function is called when extraction of a PIMA MTP device.
  * @param  pima_instance: Pointer to the pima class instance.
  * @retval none
  */
VOID USBD_PIMA_MTP_Deactivate(VOID *pima_instance)
{
  /* USER CODE BEGIN USBD_PIMA_MTP_Deactivate */
  UX_PARAMETER_NOT_USED(pima_instance);
  /* USER CODE END USBD_PIMA_MTP_Deactivate */

  return;
}

/**
  * @brief  USBD_MTP_Cancel
  *         This function is invoked when host requested to cancel operation.
  * @param  pima_instance: Pointer to the pima class instance.
  * @retval status
  */
UINT USBD_MTP_Cancel(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_MTP_Cancel */
  UX_PARAMETER_NOT_USED(pima_instance);
  /* USER CODE END USBD_MTP_Cancel */

  return status;
}

/**
  * @brief  USBD_MTP_DeviceReset
  *         This function is invoked when host requested to reset device.
  * @param  pima_instance: Pointer to the pima class instance.
  * @retval status
  */
UINT USBD_MTP_DeviceReset(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_MTP_DeviceReset */
  UX_PARAMETER_NOT_USED(pima_instance);
  /* USER CODE END USBD_MTP_DeviceReset */

  return status;
}

/**
  * @brief  USBD_MTP_GetDevicePropDesc
  *         This function is invoked when host requested to get device prop.
  * @param  pima_instance: Pointer to the pima class instance.
  * @param  device_property: device property code.
  * @param  device_prop_dataset: device property value.
  * @param  device_prop_dataset_length: device property value length.
  * @retval status
  */
UINT USBD_MTP_GetDevicePropDesc(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                                ULONG device_property,
                                UCHAR **device_prop_dataset,
                                ULONG *device_prop_dataset_length)

{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_MTP_GetDevicePropDesc */
  UX_PARAMETER_NOT_USED(pima_instance);
  UCHAR *device_friendly_name;
  UINT i;

  /* Set device friendly name pointer to the dataset_buffer */
  device_friendly_name = device_friendly_name_data_buffer;

  if (device_property == UX_DEVICE_CLASS_PIMA_DEV_PROP_DEVICE_FRIENDLY_NAME)
  {
    /* Fill the MTP device friendly name struct */
    MTP_DeviceFriendlyName.DevicePropertyCode = UX_DEVICE_CLASS_PIMA_DEV_PROP_DEVICE_FRIENDLY_NAME;
    MTP_DeviceFriendlyName.DataType = UX_DEVICE_CLASS_PIMA_TYPES_STR;
    MTP_DeviceFriendlyName.GetSet   = UX_DEVICE_CLASS_PIMA_OBJECT_PROPERTY_DATASET_VALUE_GETSET;
    MTP_DeviceFriendlyName.DefaultValue_length = sizeof(DeviceDefaultValue)/ 2U;

    for (i = 0U; i < (sizeof(DeviceDefaultValue) / 2U); i++)
    {
      MTP_DeviceFriendlyName.DefaultValue[i] = DeviceDefaultValue[i];
    }

    MTP_DeviceFriendlyName.CurrentValue_length = sizeof(DeviceCurrentValue)/ 2U;

    for (i = 0U; i < (sizeof(DeviceCurrentValue) / 2U); i++)
    {
      MTP_DeviceFriendlyName.CurrentValue[i] = DeviceCurrentValue[i];
    }

    MTP_DeviceFriendlyName.FormFlag = 0U;

    ux_utility_memory_copy(device_friendly_name, (uint8_t *)&MTP_DeviceFriendlyName,
                           sizeof(MTP_DeviceFriendlyName));


    *device_prop_dataset = device_friendly_name;
    *device_prop_dataset_length = sizeof(MTP_DeviceFriendlyName);

    status = UX_SUCCESS;
  }
  else
  {
    status = UX_DEVICE_CLASS_PIMA_RC_DEVICE_PROP_NOT_SUPPORTED;
  }

  /* USER CODE END USBD_MTP_GetDevicePropDesc */

  return status;
}

/**
  * @brief  USBD_MTP_GetDevicePropValue
  *         This function is invoked when host requested to get device
  *         prop value.
  * @param  pima_instance: Pointer to the pima class instance.
  * @param  device_property: device property code.
  * @param  device_prop_value: device property value.
  * @param  device_prop_value_length: device property value length.
  * @retval status
  */
UINT USBD_MTP_GetDevicePropValue(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                                 ULONG device_property,
                                 UCHAR **device_prop_value,
                                 ULONG *device_prop_value_length)

{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_MTP_GetDevicePropValue */
  UX_PARAMETER_NOT_USED(pima_instance);
  UCHAR *device_friendly_name;

  /* Set device friendly name pointer to the dataset_buffer */
  device_friendly_name = device_friendly_name_data_buffer;

  if (device_property == UX_DEVICE_CLASS_PIMA_DEV_PROP_DEVICE_FRIENDLY_NAME)
  {
    *device_prop_value = device_friendly_name + 6;
    *device_prop_value_length = 29;

    status = UX_SUCCESS;
  }
  else
  {
    status = UX_DEVICE_CLASS_PIMA_RC_DEVICE_PROP_NOT_SUPPORTED;
  }

  /* USER CODE END USBD_MTP_GetDevicePropValue */

  return status;
}

/**
  * @brief  USBD_MTP_SetDevicePropValue
  *         This function is invoked when host requested to set device
  *         prop value.
  * @param  pima_instance: Pointer to the pima class instance.
  * @param  device_property: device property code.
  * @param  device_prop_value: device property value.
  * @param  device_prop_value_length: device property value length.
  * @retval status
  */
UINT USBD_MTP_SetDevicePropValue(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                                 ULONG device_property,
                                 UCHAR *device_prop_value,
                                 ULONG device_prop_value_length)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_MTP_SetDevicePropValue */
  UX_PARAMETER_NOT_USED(pima_instance);
  UX_PARAMETER_NOT_USED(device_property);
  UX_PARAMETER_NOT_USED(device_prop_value);
  UX_PARAMETER_NOT_USED(device_prop_value_length);

  /* This request is not supported */
  status = UX_DEVICE_CLASS_PIMA_RC_DEVICE_PROP_NOT_SUPPORTED;

  /* USER CODE END USBD_MTP_SetDevicePropValue */

  return status;
}

/**
  * @brief  USBD_MTP_FormatStorage
  *         This function is invoked when host requested to format storage
  *         media.
  * @param  pima_instance: Pointer to the pima class instance.
  * @param  storage_id: Storage id.
  * @retval status
  */
UINT USBD_MTP_FormatStorage(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                            ULONG storage_id)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_MTP_FormatStorage */
  UX_PARAMETER_NOT_USED(pima_instance);

  /* Check the Media storage ID */
  if (storage_id == USBD_MTP_STORAGE_ID)
  {
    /* Close SD media */
    status = SD_MediaClose();

    if (status == FX_SUCCESS)
    {
      /* Format SD Media operation */
      if (SD_MediaFormat() == FX_SUCCESS)
      {
        /* Set Object Handle info struct to zero */
        ux_utility_memory_set(&ObjectHandleInfo, 0, MTP_MAX_HANDLES * sizeof(ObjectHandleTypeDef));

        /* Re-open the SD disk driver */
        SD_MediaOpen();

        status = UX_SUCCESS ;
      }
      else
      {
        /* Issue when formatting SD media */
        status = UX_DEVICE_CLASS_PIMA_RC_ACCESS_DENIED;
      }
    }
    else
    {
      status = UX_DEVICE_CLASS_PIMA_RC_GENERAL_ERROR;
    }
  }
  else
  {
    /* Invalid storage ID */
    status = UX_DEVICE_CLASS_PIMA_RC_INVALID_STORAGE_ID;
  }

  /* USER CODE END USBD_MTP_FormatStorage */

  return status;
}

/**
  * @brief  USBD_MTP_GetStorageInfo
  *         This function is invoked when host requested to get storage
  *         media information.
  * @param  pima_instance: Pointer to the pima class instance.
  * @param  storage_id: Storage id.
  * @retval status
  */
UINT USBD_MTP_GetStorageInfo(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                             ULONG storage_id)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_MTP_GetStorageInfo */

  /* Check the media storage ID */
  if (storage_id == USBD_MTP_STORAGE_ID)
  {
    /* Fill the mtp pima storage information */
    pima_instance->ux_device_class_pima_storage_type = USBD_MTP_STORAGE_TYPE;
    pima_instance->ux_device_class_pima_storage_file_system_type = USBD_MTP_STORAGE_FILE_SYSTEM_TYPE;
    pima_instance->ux_device_class_pima_storage_access_capability = USBD_MTP_STORAGE_FILE_ACCESS_CAPABILITY;
    pima_instance->ux_device_class_pima_storage_max_capacity_low = USBD_MTP_StorageGetMaxCapabilityLow();
    pima_instance->ux_device_class_pima_storage_max_capacity_high = USBD_MTP_StorageGetMaxCapabilityHigh();
    pima_instance->ux_device_class_pima_storage_free_space_low = USBD_MTP_StorageGetFreeSpaceLow();
    pima_instance->ux_device_class_pima_storage_free_space_high = USBD_MTP_StorageGetFreeSpaceHigh();
    pima_instance->ux_device_class_pima_storage_free_space_image = USBD_MTP_StorageGetFreeSpaceImage();
    pima_instance->ux_device_class_pima_storage_description = (UCHAR*)USBD_MTP_STORAGE_DESCRIPTION;
    pima_instance->ux_device_class_pima_storage_volume_label = (UCHAR*)USBD_MTP_STORAGE_DESCRIPTION_IDENTIFIER;

    status = UX_SUCCESS;
  }
  else
  {
    /* Invalid storage ID */
    status = UX_DEVICE_CLASS_PIMA_RC_INVALID_STORAGE_ID;
  }

  /* USER CODE END USBD_MTP_GetStorageInfo */

  return status;
}

/**
  * @brief  MTP_Get_ObjectNumber
  *         This function is invoked when host requested to get object
  *         number.
  * @param  pima_instance: pima instance
  * @param  object_format_code: Object format
  * @param  object_association: Object association
  * @param  object_number: Object number
  * @retval status
  */
UINT USBD_MTP_GetObjectNumber(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                              ULONG object_format_code,
                              ULONG object_association,
                              ULONG *object_number)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN MTP_Get_ObjectNumber */
  UX_PARAMETER_NOT_USED(pima_instance);
  UX_PARAMETER_NOT_USED(object_format_code);
  UX_PARAMETER_NOT_USED(object_association);

  /* Return the total object number */
  *object_number = object_handles_counter;

  /* USER CODE END MTP_Get_ObjectNumber */

  return status;
}

/**
  * @brief  USBD_MTP_GetObjectHandles
  *         This function is invoked when host requested to get object
  *         handles.
  * @param  pima_instance: Pointer to the pima class instance.
  * @param  object_handles_format_code: Format code for the handles.
  * @param  object_handles_association: Object association code.
  * @param  object_handles_array: Address where to store the handles.
  * @param  object_handles_max_number: Maximum number of handles in the array.
  * @retval status
  */
UINT USBD_MTP_GetObjectHandles(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                               ULONG object_handles_format_code,
                               ULONG object_handles_association,
                               ULONG *object_handles_array,
                               ULONG object_handles_max_number)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_MTP_GetObjectHandles */
  UX_PARAMETER_NOT_USED(pima_instance);
  UX_PARAMETER_NOT_USED(object_handles_format_code);
  ULONG ResponseLength;

  /* Get Pima MTP object handles and length */
  MTP_ObjectHandle.ObjectHandle_len = (ULONG)(Object_GetHandlesIndex(object_handles_association,
                                                                     MTP_ObjectHandle.ObjectHandle));

  if (MTP_ObjectHandle.ObjectHandle_len != 0)
  {
    ResponseLength = (MTP_ObjectHandle.ObjectHandle_len * sizeof(ULONG)) + sizeof(ULONG);

    /* Copy MTP object handle in object array */
    ux_utility_memory_copy(object_handles_array, (uint8_t *)&MTP_ObjectHandle, ResponseLength);
  }
  else
  {
    /* No files in MTP media */
    ux_utility_long_put((UCHAR *) object_handles_array, MTP_ObjectHandle.ObjectHandle_len);
  }

  /* USER CODE END USBD_MTP_GetObjectHandles */

  return status;
}

/**
  * @brief  USBD_MTP_GetObjectInfo
  *         This function is invoked when host requested to get object
  *         information.
  * @param  pima_instance: Pointer to the pima class instance.
  * @param  object_handle: Handle of the object (object index).
  * @param  object: Object pointer address
  * @retval status
  */
UINT USBD_MTP_GetObjectInfo(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                            ULONG object_handle,
                            UX_SLAVE_CLASS_PIMA_OBJECT **object)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_MTP_GetObjectInfo */
  UX_PARAMETER_NOT_USED(pima_instance);
  ULONG handle_index;

  /* Check the object handle exist */
  status = Object_HandleCheck(object_handle, &handle_index);

  if (status == UX_SUCCESS)
  {
    /* Get Pima MTP object struct */
    MTP_GetObjectHandle(object_handle, object);
  }
  else
  {
    /* Invalid object handle */
    status = UX_DEVICE_CLASS_PIMA_RC_INVALID_OBJECT_HANDLE;
  }

  /* USER CODE END USBD_MTP_GetObjectInfo */

  return status;
}

/**
  * @brief  USBD_MTP_GetObjectData
  *         This function is invoked when host requested to get object data.
  * @param  pima_instance: Pointer to the pima class instance.
  * @param  object_handle: Handle of the object (object index).
  * @param  object_buffer: Object buffer address
  * @param  object_length_requested: Object data length requested by
  *                                  the client to the application.
  * @param  object_actual_length: Object data length returned by the application.
  * @retval status
  */
UINT USBD_MTP_GetObjectData(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                            ULONG object_handle,
                            UCHAR *object_buffer,
                            ULONG object_offset,
                            ULONG object_length_requested,
                            ULONG *object_actual_length)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_MTP_GetObjectData */
  UX_PARAMETER_NOT_USED(pima_instance);
  ULONG handle_index;
  FX_FILE *object_file;
  ObjectPropertyDataSetTypeDef *object_info;

  /* Check the object handle */
  status = Object_HandleCheck(object_handle, &handle_index);

  if (status == UX_SUCCESS)
  {
    /* Get object Property info */
    Object_GetHandleInfo(handle_index, (VOID**) &object_info);

    /* Get object file */
    Object_GetHandleFile(handle_index, &object_file);

    /* Check if entire file is read */
    if (object_offset >= object_file->fx_file_current_file_size)
    {
      /* Set actual length to zero */
      *object_actual_length = 0;

      status = UX_SUCCESS;
    }
    else
    {
      /* Check of the filex array handle exist */
      if (object_file->fx_file_id == 0)
      {
        /* Open the file */
        status = fx_file_open(&sdio_disk, object_file,
                              (CHAR*) object_info->object_file_full_name,
                              FX_OPEN_FOR_READ);

        /* Check open file operation status */
        if (status != FX_SUCCESS)
        {
          /* Close file */
          fx_file_close(object_file);

          status = UX_DEVICE_CLASS_PIMA_RC_OBJECT_NOT_OPENED;

          return status;
        }
      }

      /* Seek to the offset of the object file */
      if (fx_file_seek(object_file, object_offset) == FX_SUCCESS)
      {
        /* Read from the file length requested data */
        status = fx_file_read(object_file, object_buffer,
                              object_length_requested,
                              object_actual_length);

        /* Check the read file operation status */
        if (status != FX_SUCCESS)
        {
          /* Close file */
          fx_file_close(object_file);

          switch (status)
          {
            case FX_MEDIA_NOT_OPEN:
              status = UX_DEVICE_CLASS_PIMA_RC_STORE_NOT_AVAILABLE;
              break;

            case FX_NO_MORE_SPACE:
              status = UX_DEVICE_CLASS_PIMA_RC_STORE_FULL;
              break;

            default :
              status = UX_DEVICE_CLASS_PIMA_RC_ACCESS_DENIED;
              break;
          }

          return status;
        }

        /* Check if we have read the entire file */
        if (object_file->fx_file_current_file_size == object_file->fx_file_current_file_offset)
        {
          /* Close file */
          status = fx_file_close(object_file);

          /* FX file id is not cleared by fx_file_close */
          object_file->fx_file_id = 0;

          /* Check the close file operation status */
          if (status != FX_SUCCESS)
          {
            /* Switch close file operation status */
            switch (status)
            {
              case FX_MEDIA_NOT_OPEN :
                status = UX_DEVICE_CLASS_PIMA_RC_STORE_NOT_AVAILABLE;
                break;

              default :
                status = UX_DEVICE_CLASS_PIMA_RC_ACCESS_DENIED;
                break;
            }
          }
        }
      }
      else
      {
        /* Close file */
        fx_file_close(object_file);

        status = UX_DEVICE_CLASS_PIMA_RC_INVALID_OBJECT_REFERENCE;
      }
    }
  }
  else
  {
    /* Invalid object handle */
    status = UX_DEVICE_CLASS_PIMA_RC_INVALID_OBJECT_HANDLE;
  }

  /* USER CODE END USBD_MTP_GetObjectData */

  return status;
}

/**
  * @brief  USBD_MTP_SendObjectInfo
  *         This function is called when the PIMA class needs to receive
  *         the object information in the local system for future storage.
  * @param  pima_instance: Pointer to the pima class instance.
  * @param  object: Pointer to the object.
  * @param  storage_id: Storage id.
  * @param  parent_object_handle: parent object handle.
  * @param  object_handle: Handle of the object.
  * @retval status
  */
UINT USBD_MTP_SendObjectInfo(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                             UX_SLAVE_CLASS_PIMA_OBJECT *object,
                             ULONG storage_id, ULONG parent_object_handle,
                             ULONG *object_handle)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_MTP_SendObjectInfo */
  UX_PARAMETER_NOT_USED(pima_instance);
  UX_PARAMETER_NOT_USED(storage_id);
  UX_PARAMETER_NOT_USED(parent_object_handle);
  CHAR object_filename[FX_MAX_LONG_NAME_LEN];

  /* Make sure we can accommodate a new object here */
  if ((object_handles_counter + 1) < MTP_MAX_HANDLES)
  {
    /* Convert object name from unicode to string */
    ux_utility_unicode_to_string(object->ux_device_class_pima_object_filename,
                                 (UCHAR*) object_filename);

    /* Create the destination file */
    status = fx_file_create(&sdio_disk, object_filename);

    if (status == FX_SUCCESS)
    {
      /* Flush sectors and modified file entries to the physical media */
      fx_media_flush(&sdio_disk);

      /* Increment total object counter */
      object_handles_counter += 1;

      Object_SetHandleInfo(&object_handle_index_num, object_filename,
                           object->ux_device_class_pima_object_compressed_size);

      *object_handle = object_handle_index_num;

      status = UX_SUCCESS;
    }
    else
    {
      /* Identify the problem type of file creation */
      switch (status)
      {
        case FX_MEDIA_NOT_OPEN :
          status = UX_DEVICE_CLASS_PIMA_RC_STORE_NOT_AVAILABLE;
          break;

        case FX_NO_MORE_SPACE :
          status = UX_DEVICE_CLASS_PIMA_RC_STORE_FULL;
          break;

        case FX_WRITE_PROTECT :
          status = UX_DEVICE_CLASS_PIMA_RC_STORE_READ_ONLY;
          break;

        default :
          status = UX_DEVICE_CLASS_PIMA_RC_ACCESS_DENIED;
          break;
      }

      object_handles_counter -= 1;
    }
  }
  else
  {
    /* No more space for handle */
    status = UX_DEVICE_CLASS_PIMA_RC_STORE_FULL;
  }
  /* USER CODE END USBD_MTP_SendObjectInfo */

  return status;
}

/**
  * @brief  USBD_MTP_SendObjectData
  *         This function is called when the PIMA class needs to receive
  *         the object data in the local system for storage.
  * @param  pima_instance: Pointer to the pima class instance.
  * @param  object_handle: Handle of the object.
  * @param  phase: phase of the transfer (active or complete).
  * @param  object_buffer: Object buffer address.
  * @param  object_offset: Address of data.
  * @param  object_length: Object data length sent by application.
  * @retval status
  */
UINT USBD_MTP_SendObjectData(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                             ULONG object_handle, ULONG phase,
                             UCHAR *object_buffer, ULONG object_offset,
                             ULONG object_length)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_MTP_SendObjectData */
  UX_PARAMETER_NOT_USED(pima_instance);
  ULONG handle_index;
  FX_FILE *object_file;
  ObjectPropertyDataSetTypeDef *object_info;

  /* Check the object handle exist */
  status = Object_HandleCheck(object_handle, &handle_index);

  if (status == UX_SUCCESS)
  {
    /* Get object Property info */
    Object_GetHandleInfo(handle_index, (VOID**) &object_info);

    /* Get object file */
    Object_GetHandleFile(handle_index, &object_file);

    /* Switch the transfer phase */
    switch (phase)
    {
      case UX_DEVICE_CLASS_PIMA_OBJECT_TRANSFER_PHASE_ACTIVE:

        /* Open file and seek to begin if it is the first received data */
        if (object_offset == 0)
        {
          /* Open the file */
          if (fx_file_open(&sdio_disk, object_file,
                           (CHAR*) object_info->object_file_full_name,
                           FX_OPEN_FOR_WRITE) != FX_SUCCESS)
          {
            status = UX_DEVICE_CLASS_PIMA_RC_OBJECT_NOT_OPENED;

            break;
          }
          else
          {
            /* Seek to the beginning of the object file */
            if (fx_file_seek(object_file, 0) != UX_SUCCESS)
            {
              /* Close file */
              fx_file_close(object_file);

              status = UX_DEVICE_CLASS_PIMA_RC_OBJECT_NOT_OPENED;

              break;
            }
          }
        }

        /* Write the object data to the media */
        status = fx_file_write(object_file, object_buffer, object_length);

        if (status != FX_SUCCESS)
        {
          /* Close file */
          fx_file_close(object_file);

          /* Identify the problem type of file writing operation  */
          switch (status)
          {
            case FX_MEDIA_NOT_OPEN:
              status =  UX_DEVICE_CLASS_PIMA_RC_STORE_NOT_AVAILABLE;
              break;

            case FX_NO_MORE_SPACE:
              status =  UX_DEVICE_CLASS_PIMA_RC_STORE_FULL;
              break;

            case FX_WRITE_PROTECT:
              status = UX_DEVICE_CLASS_PIMA_RC_STORE_READ_ONLY;
              break;

            default:
              status = UX_DEVICE_CLASS_PIMA_RC_ACCESS_DENIED;
              break;
          }
        }

        status = UX_SUCCESS;

        break;

      case UX_DEVICE_CLASS_PIMA_OBJECT_TRANSFER_PHASE_COMPLETED:

        /* Save final object size */
        object_info->object_size = object_file->fx_file_current_file_size;

        /* Close file */
        status = fx_file_close(object_file);

        /* Check close file operation */
        if (status == FX_SUCCESS)
        {
          object_file->fx_file_id = 0;

          /* Flush sectors and modified file entries to the physical media */
          fx_media_flush(&sdio_disk);

          status = UX_SUCCESS;
        }
        else
        {
          /* Identify the problem type of file closing operation */
          switch (status)
          {
            case FX_MEDIA_NOT_OPEN:
              status = UX_DEVICE_CLASS_PIMA_RC_STORE_NOT_AVAILABLE;
              break;

            default:
              status = UX_DEVICE_CLASS_PIMA_RC_ACCESS_DENIED;
              break;
          }
        }

        break;

      case UX_DEVICE_CLASS_PIMA_OBJECT_TRANSFER_PHASE_COMPLETED_ERROR:

        /* Close and delete the object */
        USBD_MTP_DeleteObject(pima_instance, object_handle);

        status = UX_SUCCESS;
        break;
    }
  }

  /* USER CODE END USBD_MTP_SendObjectData */

  return status;
}

/**
  * @brief  USBD_MTP_DeleteObject
  *         This function is called when the PIMA class needs to delete
  *         an object on the local storage.
  * @param  pima_instance: Pointer to the pima class instance.
  * @param  object_handle: Handle of the object.
  * @retval status
  */
UINT USBD_MTP_DeleteObject(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                           ULONG object_handle)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_MTP_DeleteObject */
  UX_PARAMETER_NOT_USED(pima_instance);
  ULONG handle_index;
  FX_FILE *object_file;
  ObjectPropertyDataSetTypeDef *object_info;

  /* Check the object handle exist */
  status = Object_HandleCheck(object_handle, &handle_index);

  if (status == UX_SUCCESS)
  {
    /* Get object Property info */
    Object_GetHandleInfo(handle_index, (VOID**) &object_info);

    /* Get object file */
    Object_GetHandleFile(handle_index, &object_file);

    /* Close object */
    fx_file_close(object_file);

    /* Delete file form media */
    status = fx_file_delete(&sdio_disk, (CHAR*) object_info->object_file_full_name);

    /* Delete file */
    if (status != FX_SUCCESS)
    {
      /* Identify the problem type of deleting operation */
      switch (status)
      {
        case FX_MEDIA_NOT_OPEN :
          status = UX_DEVICE_CLASS_PIMA_RC_STORE_NOT_AVAILABLE;
          break;

        case FX_WRITE_PROTECT :
          status = UX_DEVICE_CLASS_PIMA_RC_STORE_READ_ONLY;
          break;

        default:
          status = UX_DEVICE_CLASS_PIMA_RC_ACCESS_DENIED;
          break;
      }

      return status;
    }

    /* Flush sectors and modified file entries to the physical media */
    if (fx_media_flush(&sdio_disk) != FX_SUCCESS)
    {
      return UX_DEVICE_CLASS_PIMA_RC_GENERAL_ERROR;
    }

    /* Delete handle from gloal object info struct */
    ux_utility_memory_set(&ObjectHandleInfo[handle_index], 0,
                          sizeof(ObjectHandleTypeDef));

    /* Decrement total object handle counter */
    object_handles_counter -= 1;

    status = UX_SUCCESS;
  }
  else
  {
    /* Invalid object handle */
    status = UX_DEVICE_CLASS_PIMA_RC_INVALID_OBJECT_HANDLE;
  }

  /* USER CODE END USBD_MTP_DeleteObject */

  return status;
}

/**
  * @brief  USBD_MTP_GetObjectPropDesc
  *         This function is invoked when host requested to get object
  *         prop desc.
  * @param  pima_instance: Pointer to the pima class instance.
  * @param  object_property: object property.
  * @param  object_format_code: object format code.
  * @param  object_prop_dataset: object property dataset.
  * @param  object_prop_dataset_length: object property dataset length.
  * @retval status
  */
UINT USBD_MTP_GetObjectPropDesc(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                                ULONG object_property,
                                ULONG object_format_code,
                                UCHAR **object_prop_dataset,
                                ULONG *object_prop_dataset_length)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_MTP_GetObjectPropDesc */
  UX_PARAMETER_NOT_USED(pima_instance);
  UINT undef_format = UX_DEVICE_CLASS_PIMA_OFC_UNDEFINED;
  ULONG storageid = USBD_MTP_STORAGE_ID;
  ULONG dataLength;
  UCHAR *object_property_dataset_data;

  /* Check the object format */
  switch (object_format_code)
  {
    case UX_DEVICE_CLASS_PIMA_OFC_UNDEFINED:
    case UX_DEVICE_CLASS_PIMA_OFC_MP3:
    case UX_DEVICE_CLASS_PIMA_OFC_WAV:
    case UX_DEVICE_CLASS_PIMA_OFC_ASF:
    case UX_DEVICE_CLASS_PIMA_OFC_WMA:
    case UX_DEVICE_CLASS_PIMA_OFC_WMV:
    case UX_DEVICE_CLASS_PIMA_OFC_TEXT:
    case UX_DEVICE_CLASS_PIMA_OFC_MP4_CONTAINER:
    case UX_DEVICE_CLASS_PIMA_OFC_EXIF_JPEG:

      /* Set the pointer to the dataset_buffer */
      object_property_dataset_data = object_property_dataset_data_buffer;

      /* Isolate the property */
      switch (object_property)
      {
        case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_STORAGEID:

          /* Fill MTP object property struct */
          MTP_ObjectPropDesc.ObjectPropertyCode = UX_DEVICE_CLASS_PIMA_OBJECT_PROP_STORAGEID;
          MTP_ObjectPropDesc.DataType = UX_DEVICE_CLASS_PIMA_TYPES_UINT16;
          MTP_ObjectPropDesc.GetSet = UX_DEVICE_CLASS_PIMA_OBJECT_PROPERTY_DATASET_GETSET;
          MTP_ObjectPropDesc.DefValue = (uint8_t *)&storageid;
          MTP_ObjectPropDesc.GroupCode = 0U;
          MTP_ObjectPropDesc.FormFlag = 0U;

          status = UX_SUCCESS;
          break;

        case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FORMAT:

          /* Fill MTP object property struct */
          MTP_ObjectPropDesc.ObjectPropertyCode = UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FORMAT;
          MTP_ObjectPropDesc.DataType = UX_DEVICE_CLASS_PIMA_TYPES_UINT16;
          MTP_ObjectPropDesc.GetSet = UX_DEVICE_CLASS_PIMA_OBJECT_PROPERTY_DATASET_GETSET;
          MTP_ObjectPropDesc.DefValue = (uint8_t *)&undef_format;
          MTP_ObjectPropDesc.GroupCode = 0U;
          MTP_ObjectPropDesc.FormFlag = 0U;

          status = UX_SUCCESS;
          break;

        case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PROTECTION_STATUS:

          /* Fill MTP object property struct */
          MTP_ObjectPropDesc.ObjectPropertyCode = UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PROTECTION_STATUS;
          MTP_ObjectPropDesc.DataType = UX_DEVICE_CLASS_PIMA_TYPES_UINT16;
          MTP_ObjectPropDesc.GetSet = UX_DEVICE_CLASS_PIMA_OBJECT_PROPERTY_DATASET_GETSET;
          MTP_ObjectPropDesc.DefValue = 0U;
          MTP_ObjectPropDesc.GroupCode = 0U;
          MTP_ObjectPropDesc.FormFlag = 0U;

          status = UX_SUCCESS;
          break;

        case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_SIZE:

          /* Fill MTP object property struct */
          MTP_ObjectPropDesc.ObjectPropertyCode = UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_SIZE;
          MTP_ObjectPropDesc.DataType = UX_DEVICE_CLASS_PIMA_TYPES_UINT64;
          MTP_ObjectPropDesc.GetSet = UX_DEVICE_CLASS_PIMA_OBJECT_PROPERTY_DATASET_GETSET;
          MTP_ObjectPropDesc.DefValue = 0U;
          MTP_ObjectPropDesc.GroupCode = 0U;
          MTP_ObjectPropDesc.FormFlag = 0U;

          status = UX_SUCCESS;
          break;

        case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FILE_NAME:

          /* Fill MTP object property struct */
          MTP_ObjectPropDesc.ObjectPropertyCode = UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FILE_NAME;
          MTP_ObjectPropDesc.DataType = UX_DEVICE_CLASS_PIMA_TYPES_STR;
          MTP_ObjectPropDesc.GetSet = UX_DEVICE_CLASS_PIMA_OBJECT_PROPERTY_DATASET_GETSET;
          MTP_FileName.FileName_len = DEFAULT_FILE_NAME_LEN;
          ux_utility_memory_copy((VOID *) &(MTP_FileName.FileName),
                                 (VOID *)DefaultFileName, sizeof(DefaultFileName));

          MTP_ObjectPropDesc.DefValue = (uint8_t *)&MTP_FileName;
          MTP_ObjectPropDesc.GroupCode = 0U;
          MTP_ObjectPropDesc.FormFlag = 0U;

          status = UX_SUCCESS;
          break;

        case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_NAME:

          /* Fill MTP object property struct */
          MTP_ObjectPropDesc.ObjectPropertyCode = UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FILE_NAME;
          MTP_ObjectPropDesc.DataType = UX_DEVICE_CLASS_PIMA_TYPES_STR;
          MTP_ObjectPropDesc.GetSet = UX_DEVICE_CLASS_PIMA_OBJECT_PROPERTY_DATASET_GETSET;
          MTP_FileName.FileName_len = DEFAULT_FILE_NAME_LEN;
          ux_utility_memory_copy((VOID *) &(MTP_FileName.FileName),
                                 (VOID *)DefaultFileName, sizeof(DefaultFileName));

          MTP_ObjectPropDesc.DefValue = (uint8_t *)&MTP_FileName;
          MTP_ObjectPropDesc.GroupCode = 0U;
          MTP_ObjectPropDesc.FormFlag = 0U;

          status = UX_SUCCESS;
          break;

        case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PARENT_OBJECT:

          /* Fill MTP object property struct */
          MTP_ObjectPropDesc.ObjectPropertyCode = UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PARENT_OBJECT;
          MTP_ObjectPropDesc.DataType = UX_DEVICE_CLASS_PIMA_TYPES_UINT32;
          MTP_ObjectPropDesc.GetSet = UX_DEVICE_CLASS_PIMA_OBJECT_PROPERTY_DATASET_GETSET;
          MTP_ObjectPropDesc.DefValue = 0U;
          MTP_ObjectPropDesc.GroupCode = 0U;
          MTP_ObjectPropDesc.FormFlag = 0U;

          status = UX_SUCCESS;
          break;


        case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PERSISTENT_UNIQUE_OBJECT_IDENTIFIER:

          /* Fill MTP object property struct */
          MTP_ObjectPropDesc.ObjectPropertyCode = UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PERSISTENT_UNIQUE_OBJECT_IDENTIFIER;
          MTP_ObjectPropDesc.DataType = UX_DEVICE_CLASS_PIMA_TYPES_UINT128;
          MTP_ObjectPropDesc.GetSet = UX_DEVICE_CLASS_PIMA_OBJECT_PROPERTY_DATASET_GETSET;
          MTP_ObjectPropDesc.DefValue = 0U;
          MTP_ObjectPropDesc.GroupCode = 0U;
          MTP_ObjectPropDesc.FormFlag = 0U;

          status = UX_SUCCESS;
          break;

        default :

          /* Property code is not valid */
          status = UX_DEVICE_CLASS_PIMA_RC_INVALID_OBJECT_PROP_CODE;
          break;

      }

      /* Check the status of the operation */
      if (status == UX_SUCCESS)
      {
        dataLength = 0U;

        /* Copy object property code to the payload data */
        ux_utility_memory_copy(object_property_dataset_data + dataLength,
                               &MTP_ObjectPropDesc.ObjectPropertyCode,
                               sizeof(MTP_ObjectPropDesc.ObjectPropertyCode));

        dataLength += sizeof(MTP_ObjectPropDesc.ObjectPropertyCode);

        /* Copy object Property Data type to the payload data */
        ux_utility_memory_copy(object_property_dataset_data + dataLength,
                               &MTP_ObjectPropDesc.DataType,
                               sizeof(MTP_ObjectPropDesc.DataType));

        dataLength += sizeof(MTP_ObjectPropDesc.DataType);

        /* Copy Property GetSet value to the payload data */
        ux_utility_memory_copy(object_property_dataset_data + dataLength,
                               &MTP_ObjectPropDesc.GetSet,
                               sizeof(MTP_ObjectPropDesc.GetSet));

        dataLength += sizeof(MTP_ObjectPropDesc.GetSet);

        switch(MTP_ObjectPropDesc.DataType)
        {
          case UX_DEVICE_CLASS_PIMA_TYPES_UINT16:

            /* Copy Property default value to the payload data */
            ux_utility_memory_copy(object_property_dataset_data + dataLength,
                                   MTP_ObjectPropDesc.DefValue, sizeof(uint32_t));

            dataLength += sizeof(uint16_t);
            break;

          case UX_DEVICE_CLASS_PIMA_TYPES_UINT32:

            /* Copy Property default value to the payload data */
            ux_utility_memory_copy(object_property_dataset_data + dataLength,
                                   MTP_ObjectPropDesc.DefValue, sizeof(uint32_t));

            dataLength += sizeof(uint32_t);
            break;

          case UX_DEVICE_CLASS_PIMA_TYPES_UINT64:

            /* Copy Property default value to the payload data */
            ux_utility_memory_copy(object_property_dataset_data + dataLength,
                                   MTP_ObjectPropDesc.DefValue, sizeof(uint64_t));


            dataLength += sizeof(uint64_t);
            break;

          case UX_DEVICE_CLASS_PIMA_TYPES_STR:

            /* Copy Property default value to the payload data */
            ux_utility_memory_copy(object_property_dataset_data + dataLength,
                                   MTP_ObjectPropDesc.DefValue, (MTP_FileName.FileName_len * 2U) + 1U);

            dataLength += (MTP_FileName.FileName_len * 2U) + 1U;
            break;

          case UX_DEVICE_CLASS_PIMA_TYPES_UINT128:

            /* Copy Property default value to the payload data */
            ux_utility_memory_copy(object_property_dataset_data + dataLength,
                                   MTP_ObjectPropDesc.DefValue, sizeof(uint64_t) * 2U);

            dataLength += sizeof(uint64_t) * 2U;
            break;

          default:
            break;
        }

        /* Copy Property Group Code value to the payload data */
        ux_utility_memory_copy(object_property_dataset_data + dataLength,
                               &MTP_ObjectPropDesc.GroupCode,
                               sizeof(MTP_ObjectPropDesc.GroupCode));

        dataLength += sizeof(MTP_ObjectPropDesc.GroupCode);

        /* Copy Property Form Flag value to the payload data */
        ux_utility_memory_copy(object_property_dataset_data + dataLength,
                               &MTP_ObjectPropDesc.FormFlag,
                               sizeof(MTP_ObjectPropDesc.FormFlag));

        dataLength += sizeof(MTP_ObjectPropDesc.FormFlag);

        /* Set property dataset value */
        *object_prop_dataset = object_property_dataset_data;

        /* Set the length of property dataset value */
        *object_prop_dataset_length = dataLength;

        status = UX_SUCCESS;
      }

      break;

    default :

      /* Unsupported object format */
      status = UX_DEVICE_CLASS_PIMA_RC_INVALID_OBJECT_FORMAT_CODE;
      break;

  }

  /* USER CODE END USBD_MTP_GetObjectPropDesc */

  return status;
}

/**
  * @brief  USBD_MTP_GetObjectPropValue
  *         This function is invoked when host requested to get object
  *         prop value.
  * @param  pima_instance : Pointer to the pima class instance.
  * @param  object_handle: Handle of the object.
  * @param  object_prop_code: Object property code.
  * @param  object_prop_value: Object property value.
  * @param  object_prop_value_length: Object property value length.
  * @retval status
  */
UINT USBD_MTP_GetObjectPropValue(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                                 ULONG object_handle,
                                 ULONG object_prop_code,
                                 UCHAR **object_prop_value,
                                 ULONG *object_prop_value_length)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_MTP_GetObjectPropValue */
  UX_PARAMETER_NOT_USED(pima_instance);
  ULONG handle_index;
  UCHAR *object_property_dataset_data;
  ObjectPropertyDataSetTypeDef *object_info;

  /* Check the object handle exist */
  status = Object_HandleCheck(object_handle, &handle_index);

  if (status == UX_SUCCESS)
  {
    /* Get object Property info */
    Object_GetHandleInfo(handle_index, (VOID**) &object_info);

    /* Set the pointer to the dataset_buffer */
    object_property_dataset_data = object_property_dataset_data_buffer;

    /* Switch object Property code */
    switch (object_prop_code)
    {
      case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_STORAGEID:

        /* Copy stagre id object property dataset value */
        ux_utility_long_put(object_property_dataset_data, object_info->object_storage_id);

        /* Set property dataset value */
        *object_prop_value = object_property_dataset_data;

        /* Set the length of property dataset value */
        *object_prop_value_length = 4;

        status = UX_SUCCESS;

        break;

      case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FORMAT:

        /* Copy format object property dataset value */
        ux_utility_short_put(object_property_dataset_data, object_info->object_format);

        /* Set property dataset value */
        *object_prop_value = object_property_dataset_data;

        /* Set the length of property dataset value */
        *object_prop_value_length = 2;

        status = UX_SUCCESS;

        break;

      case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PROTECTION_STATUS:

        /* Copy protection status object property dataset value */
        ux_utility_short_put(object_property_dataset_data, object_info->object_protection_status);

        /* Set property dataset value */
        *object_prop_value = object_property_dataset_data;

        /* Set the length of property dataset value */
        *object_prop_value_length = 2;

        status = UX_SUCCESS;

        break;

      case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_SIZE:

        /* Copy object size property dataset value */
        ux_utility_long_put(object_property_dataset_data, object_info->object_size);
        ux_utility_long_put(object_property_dataset_data + 4, 0);

        /* Set property dataset value */
        *object_prop_value = object_property_dataset_data;

        /* Set the length of property dataset value */
        *object_prop_value_length  = 8;

        status = UX_SUCCESS;

        break;

      case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FILE_NAME:

        /* Store the file name in unicode format */
        ux_utility_string_to_unicode(object_info->object_file_full_name, object_property_dataset_data);

        /* Set property dataset value */
        *object_prop_value = object_property_dataset_data;

        /* Set the length of property dataset value */
        *object_prop_value_length = (ULONG) *(object_property_dataset_data) * 2 + 1;

        status = UX_SUCCESS;

        break;

      case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_NAME:

        /* Store the file name in unicode format */
        ux_utility_string_to_unicode(object_info->object_file_name, object_property_dataset_data);

        /* Set property dataset value */
        *object_prop_value = object_property_dataset_data;

        /* Set the length of property dataset value */
        *object_prop_value_length  = (ULONG) *(object_property_dataset_data) * 2  + 1;

        status = UX_SUCCESS;

        break;

      case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PARENT_OBJECT:

        /* Copy object size property dataset value */
        ux_utility_long_put(object_property_dataset_data, object_info->object_parent_object);

        /* Set property dataset value */
        *object_prop_value = object_property_dataset_data;

        /* Set the length of property dataset value */
        *object_prop_value_length = 4;

        status = UX_SUCCESS;

        break;

      case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PERSISTENT_UNIQUE_OBJECT_IDENTIFIER:

        /* Copy object persistent unique indentfier property dataset value */
        ux_utility_memory_copy(object_property_dataset_data, object_info->object_identifier, 16);

        *object_prop_value = object_property_dataset_data;

        /* Set the length of property dataset value */
        *object_prop_value_length  = 16;

        status = UX_SUCCESS;

        break;

      default:

        /* Invalid property code */
        status = UX_DEVICE_CLASS_PIMA_RC_INVALID_OBJECT_PROP_CODE;
        break;
    }
  }
  else
  {
    /* Invalid object handle */
    status = UX_DEVICE_CLASS_PIMA_RC_INVALID_OBJECT_HANDLE;
  }

  /* USER CODE END USBD_MTP_GetObjectPropValue */

  return status;
}

/**
  * @brief  USBD_MTP_SetObjectPropValue
  *         This function is invoked when host requested to set object
  *         prop value.
  * @param  pima_instance : Pointer to the pima class instance.
  * @param  object_handle : Handle of the object.
  * @param  object_prop_code : Object property code.
  * @param  object_prop_value: Object property value.
  * @param  object_prop_value_length: Object property value length.
  * @retval status
  */
UINT USBD_MTP_SetObjectPropValue(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                                 ULONG object_handle,
                                 ULONG object_prop_code,
                                 UCHAR *object_prop_value,
                                 ULONG object_prop_value_length)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_MTP_SetObjectPropValue */
  UX_PARAMETER_NOT_USED(pima_instance);
  UX_PARAMETER_NOT_USED(object_prop_value_length);
  ULONG handle_index;
  ObjectPropertyDataSetTypeDef *object_info;

  /* Check the object handle exist */
  status = Object_HandleCheck(object_handle, &handle_index);

  if (status == UX_SUCCESS)
  {
    /* Get object Property info */
    Object_GetHandleInfo(handle_index, (VOID**) &object_info);

    /* switch object property code */
    switch (object_prop_code)
    {
      case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FILE_NAME :

        /* Copy the file name after translate from Unicode to ASCIIZ */
        ux_utility_unicode_to_string(object_prop_value, object_info->object_file_full_name);

        status = UX_SUCCESS;

        break;

      case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_NAME :

        /* Copy the name after translate from Unicode to ASCIIZ */
        ux_utility_unicode_to_string(object_prop_value, object_info->object_file_name);

        status = UX_SUCCESS;

        break;

      case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_STORAGEID :
      case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FORMAT :
      case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_SIZE :
      case UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PARENT_OBJECT :

        /* Object is write protected */
        status = UX_DEVICE_CLASS_PIMA_RC_OBJECT_WRITE_PROTECTED;
        break;

      default :
        status = UX_DEVICE_CLASS_PIMA_RC_INVALID_OBJECT_PROP_CODE;
        break;
    }
  }
  else
  {
    /* Invalid object handle */
    status = UX_DEVICE_CLASS_PIMA_RC_INVALID_OBJECT_HANDLE;
  }

  /* USER CODE END USBD_MTP_SetObjectPropValue */

  return status;
}

/**
  * @brief  USBD_MTP_GetObjectReferences
  *         This function is invoked when host requested to get object
  *         references.
  * @param  pima_instance : Pointer to the pima class instance.
  * @param  object_handle : Handle of the object.
  * @param  object_handle_array: object handle array.
  * @param  object_handle_array_length: length of object handle array.
  * @retval status
  */
UINT USBD_MTP_GetObjectReferences(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                                  ULONG object_handle,
                                  UCHAR **object_handle_array,
                                  ULONG *object_handle_array_length)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_MTP_GetObjectReferences */
  UX_PARAMETER_NOT_USED(pima_instance);
  ULONG handle_index;

  /* Check the object handle exist */
  if (Object_HandleCheck(object_handle, &handle_index) == UX_SUCCESS)
  {
    references_array = 0;

    /* Set Object references array */
    *object_handle_array = (UCHAR *) &references_array;

    /* Set Object references array length */
    *object_handle_array_length = sizeof(ULONG);

    status = UX_SUCCESS;
  }
  else
  {
    /* Invalid object handle */
    status = UX_DEVICE_CLASS_PIMA_RC_INVALID_OBJECT_HANDLE;
  }

  /* USER CODE END USBD_MTP_GetObjectReferences */

  return status;
}

/**
  * @brief  USBD_MTP_SetObjectReferences
  *         This function is invoked when host requested to set object
  *         references.
  * @param  pima_instance : Pointer to the pima class instance.
  * @param  object_handle : Handle of the object.
  * @param  object_handle_array: object handle array.
  * @param  object_handle_array_length: length of object handle array.
  * @retval status
  */
UINT USBD_MTP_SetObjectReferences(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima_instance,
                                  ULONG object_handle,
                                  UCHAR *object_handle_array,
                                  ULONG object_handle_array_length)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_MTP_SetObjectReferences */
  ULONG handle_index;

  /* Check the object handle existe*/
  status = Object_HandleCheck(object_handle, &handle_index);

  if (status != UX_SUCCESS)
  {
    status = UX_DEVICE_CLASS_PIMA_RC_INVALID_OBJECT_HANDLE;
  }

  /* USER CODE END USBD_MTP_SetObjectReferences */

  return status;
}

/**
  * @brief  USBD_MTP_StorageGetMaxCapabilityLow
  *         This function is called to get storage max capability low.
  * @param  none
  * @retval max capability
  */
ULONG USBD_MTP_StorageGetMaxCapabilityLow(VOID)
{
  ULONG max_capability_low = 0U;

  /* USER CODE BEGIN USBD_MTP_StorageGetMaxCapabilityLow */
  ULONG64 total_sd_size;

  total_sd_size = sdio_disk.fx_media_total_sectors * \
                  sdio_disk.fx_media_bytes_per_sector;

  /* Extract low bytes from total size */
  max_capability_low = total_sd_size & 0xFFFFFFFF;

  /* USER CODE END USBD_MTP_StorageGetMaxCapabilityLow */

  return max_capability_low;
}

/**
  * @brief  USBD_MTP_StorageGetMaxCapabilityHigh
  *         This function is called to get storage max capability high.
  * @param  none
  * @retval max capability high
  */
ULONG USBD_MTP_StorageGetMaxCapabilityHigh(VOID)
{
  ULONG max_capability_high = 0U;

  /* USER CODE BEGIN USBD_MTP_StorageGetMaxCapabilityHigh */
  ULONG64 total_sd_size;

  total_sd_size = sdio_disk.fx_media_total_sectors * \
                  sdio_disk.fx_media_bytes_per_sector;

  /* Extract high bytes from total size */
  max_capability_high = (total_sd_size >> 32) & 0xFFFFFFFF;

  /* USER CODE END USBD_MTP_StorageGetMaxCapabilityHigh */

  return max_capability_high;
}

/**
  * @brief  USBD_MTP_StorageGetFreeSpaceLow
  *         This function is called to get storage free space low.
  * @param  none
  * @retval Free space low
  */
ULONG USBD_MTP_StorageGetFreeSpaceLow(VOID)
{
  ULONG free_space_low = 0U;

  /* USER CODE BEGIN USBD_MTP_StorageGetFreeSpaceLow */
  ULONG64 availabe_sd_size;

  /* Retrieve the available bytes in the media */
  fx_media_extended_space_available(&sdio_disk, &availabe_sd_size);

  /* Extract low bytes from available size */
  free_space_low = availabe_sd_size & 0xFFFFFFFF;

  /* USER CODE END USBD_MTP_StorageGetFreeSpaceLow */

  return free_space_low;
}

/**
  * @brief  USBD_MTP_StorageGetFreeSpaceHigh
  *         This function is called to get storage free space high.
  * @param  none
  * @retval Free space high
  */
ULONG USBD_MTP_StorageGetFreeSpaceHigh(VOID)
{
  ULONG free_space_high = 0U;

  /* USER CODE BEGIN USBD_MTP_StorageGetFreeSpaceHigh */
  ULONG64 availabe_sd_size;

  /* Retrieve the available bytes in the media */
  fx_media_extended_space_available(&sdio_disk, &availabe_sd_size);

  /* Extract high bytes from available size */
  free_space_high = (availabe_sd_size >> 32) & 0xFFFFFFFF;

  /* USER CODE END USBD_MTP_StorageGetFreeSpaceHigh */

  return free_space_high;
}

/**
  * @brief  USBD_MTP_StorageGetFreeSpaceImage
  *         This function is called to get storage free space image.
  * @param  none
  * @retval Free space image
  */
ULONG USBD_MTP_StorageGetFreeSpaceImage(VOID)
{
  ULONG free_space_image = 0U;

  /* USER CODE BEGIN USBD_MTP_StorageGetFreeSpaceImage */
  free_space_image = 0xFFFFFFFF;
  /* USER CODE END USBD_MTP_StorageGetFreeSpaceImage */

  return free_space_image;
}

/* USER CODE BEGIN 1 */


/**
  * @brief  MTP_GetObjectHandle
  *         This function is called to get pima mtp object struct.
  * @param  object_handle: object handle.
  * @param  object: pima mtp object struct.
  * @retval status
  */
UINT MTP_GetObjectHandle(ULONG object_handle, UX_SLAVE_CLASS_PIMA_OBJECT **object)
{
  UINT status, index;

  for (index = 0; index < object_handles_counter; index++)
  {
    if (ObjectHandleInfo[index].object_handle_index == object_handle)
    {
      ux_utility_memory_set(&MTP_Object, 0, sizeof(MTP_Object));

      /* Fill MTP object struct */
      MTP_Object.ux_device_class_pima_object_storage_id = ObjectHandleInfo[index].object_property.object_storage_id;
      MTP_Object.ux_device_class_pima_object_format = ObjectHandleInfo[index].object_property.object_format;
      MTP_Object.ux_device_class_pima_object_compressed_size = ObjectHandleInfo[index].object_property.object_size;
      MTP_Object.ux_device_class_pima_object_protection_status =
        ObjectHandleInfo[index].object_property.object_protection_status;
      MTP_Object.ux_device_class_pima_object_thumb_format = UX_DEVICE_CLASS_PIMA_OFC_UNDEFINED;
      MTP_Object.ux_device_class_pima_object_thumb_compressed_size = 0U;
      MTP_Object.ux_device_class_pima_object_thumb_pix_height = 0U;
      MTP_Object.ux_device_class_pima_object_thumb_pix_width = 0U;
      MTP_Object.ux_device_class_pima_object_image_pix_height = 0U;
      MTP_Object.ux_device_class_pima_object_image_pix_width = 0U;
      MTP_Object.ux_device_class_pima_object_image_bit_depth = 0U;
      MTP_Object.ux_device_class_pima_object_parent_object =
        ObjectHandleInfo[index].object_property.object_parent_object;
      MTP_Object.ux_device_class_pima_object_association_type = 0U;
      MTP_Object.ux_device_class_pima_object_association_desc = 0U;
      MTP_Object.ux_device_class_pima_object_sequence_number = 0U;

      ux_utility_string_to_unicode(ObjectHandleInfo[index].object_property.object_file_full_name,
                                   MTP_Object.ux_device_class_pima_object_filename);

      *object = &MTP_Object;

      status = UX_SUCCESS;
    }
  }

  return status;
}

/* USER CODE END 1 */
