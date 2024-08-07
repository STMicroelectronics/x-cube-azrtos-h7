/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_usbx_device.c
  * @author  MCD Application Team
  * @brief   USBX Device applicative file
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
#include "app_usbx_device.h"

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

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

static ULONG pima_mtp_interface_number;
static ULONG pima_mtp_configuration_number;
static UX_SLAVE_CLASS_PIMA_PARAMETER pima_mtp_parameter;
static TX_THREAD ux_device_app_thread;
extern USHORT USBD_MTP_DevicePropSupported[];
extern USHORT USBD_MTP_DeviceSupportedCaptureFormats[];
extern USHORT USBD_MTP_DeviceSupportedImageFormats[];
extern USHORT USBD_MTP_ObjectPropSupported[];

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static VOID app_ux_device_thread_entry(ULONG thread_input);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/**
  * @brief  Application USBX Device Initialization.
  * @param  memory_ptr: memory pointer
  * @retval status
  */
UINT MX_USBX_Device_Init(VOID *memory_ptr)
{
  UINT ret = UX_SUCCESS;
  UCHAR *device_framework_high_speed;
  UCHAR *device_framework_full_speed;
  ULONG device_framework_hs_length;
  ULONG device_framework_fs_length;
  ULONG string_framework_length;
  ULONG language_id_framework_length;
  UCHAR *string_framework;
  UCHAR *language_id_framework;
  UCHAR *pointer;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

  /* USER CODE BEGIN MX_USBX_Device_Init0 */

  /* USER CODE END MX_USBX_Device_Init0 */

  /* Allocate the stack for USBX Memory */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       USBX_DEVICE_MEMORY_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_ALLOCATE_STACK_ERROR */
    return TX_POOL_ERROR;
    /* USER CODE END USBX_ALLOCATE_STACK_ERROR */
  }

  /* Initialize USBX Memory */
  if (ux_system_initialize(pointer, USBX_DEVICE_MEMORY_STACK_SIZE, UX_NULL, 0) != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_SYSTEM_INITIALIZE_ERROR */
    return UX_ERROR;
    /* USER CODE END USBX_SYSTEM_INITIALIZE_ERROR */
  }

  /* Get Device Framework High Speed and get the length */
  device_framework_high_speed = USBD_Get_Device_Framework_Speed(USBD_HIGH_SPEED,
                                                                &device_framework_hs_length);

  /* Get Device Framework Full Speed and get the length */
  device_framework_full_speed = USBD_Get_Device_Framework_Speed(USBD_FULL_SPEED,
                                                                &device_framework_fs_length);

  /* Get String Framework and get the length */
  string_framework = USBD_Get_String_Framework(&string_framework_length);

  /* Get Language Id Framework and get the length */
  language_id_framework = USBD_Get_Language_Id_Framework(&language_id_framework_length);

  /* Install the device portion of USBX */
  if (ux_device_stack_initialize(device_framework_high_speed,
                                 device_framework_hs_length,
                                 device_framework_full_speed,
                                 device_framework_fs_length,
                                 string_framework,
                                 string_framework_length,
                                 language_id_framework,
                                 language_id_framework_length,
                                 UX_NULL) != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_DEVICE_INITIALIZE_ERROR */
    return UX_ERROR;
    /* USER CODE END USBX_DEVICE_INITIALIZE_ERROR */
  }

  /* Initialize the pima class parameters for the device */
  pima_mtp_parameter.ux_device_class_pima_instance_activate = USBD_PIMA_MTP_Activate;
  pima_mtp_parameter.ux_device_class_pima_instance_deactivate = USBD_PIMA_MTP_Deactivate;

  pima_mtp_parameter.ux_device_class_pima_parameter_manufacturer                   = (UCHAR *)USBD_MTP_INFO_MANUFACTURER;
  pima_mtp_parameter.ux_device_class_pima_parameter_model                          = (UCHAR *)USBD_MTP_INFO_MODEL;
  pima_mtp_parameter.ux_device_class_pima_parameter_device_version                 = (UCHAR *)USBD_MTP_INFO_VERSION;
  pima_mtp_parameter.ux_device_class_pima_parameter_serial_number                  = (UCHAR *)USBD_MTP_INFO_SERIAL_NUMBER;
  pima_mtp_parameter.ux_device_class_pima_parameter_storage_id                     = USBD_MTP_STORAGE_ID;
  pima_mtp_parameter.ux_device_class_pima_parameter_storage_type                   = USBD_MTP_STORAGE_TYPE;
  pima_mtp_parameter.ux_device_class_pima_parameter_storage_file_system_type       = USBD_MTP_STORAGE_FILE_SYSTEM_TYPE;
  pima_mtp_parameter.ux_device_class_pima_parameter_storage_access_capability      = USBD_MTP_STORAGE_FILE_ACCESS_CAPABILITY;
  pima_mtp_parameter.ux_device_class_pima_parameter_storage_max_capacity_low       = USBD_MTP_StorageGetMaxCapabilityLow();
  pima_mtp_parameter.ux_device_class_pima_parameter_storage_max_capacity_high      = USBD_MTP_StorageGetMaxCapabilityHigh();
  pima_mtp_parameter.ux_device_class_pima_parameter_storage_free_space_low         = USBD_MTP_StorageGetFreeSpaceLow();
  pima_mtp_parameter.ux_device_class_pima_parameter_storage_free_space_high        = USBD_MTP_StorageGetFreeSpaceHigh();
  pima_mtp_parameter.ux_device_class_pima_parameter_storage_free_space_image       = USBD_MTP_StorageGetFreeSpaceImage();
  pima_mtp_parameter.ux_device_class_pima_parameter_storage_description            = (UCHAR*)USBD_MTP_STORAGE_DESCRIPTION;
  pima_mtp_parameter.ux_device_class_pima_parameter_storage_volume_label           = (UCHAR*)USBD_MTP_STORAGE_DESCRIPTION_IDENTIFIER;
  pima_mtp_parameter.ux_device_class_pima_parameter_device_properties_list         = USBD_MTP_DevicePropSupported;
  pima_mtp_parameter.ux_device_class_pima_parameter_supported_capture_formats_list = USBD_MTP_DeviceSupportedCaptureFormats;
  pima_mtp_parameter.ux_device_class_pima_parameter_supported_image_formats_list   = USBD_MTP_DeviceSupportedImageFormats;
  pima_mtp_parameter.ux_device_class_pima_parameter_object_properties_list         = USBD_MTP_ObjectPropSupported;

  /* Define the pima callbacks */
  pima_mtp_parameter.ux_device_class_pima_parameter_cancel                = USBD_MTP_Cancel;
  pima_mtp_parameter.ux_device_class_pima_parameter_device_reset          = USBD_MTP_DeviceReset;
  pima_mtp_parameter.ux_device_class_pima_parameter_device_prop_desc_get  = USBD_MTP_GetDevicePropDesc;
  pima_mtp_parameter.ux_device_class_pima_parameter_device_prop_value_get = USBD_MTP_GetDevicePropValue;
  pima_mtp_parameter.ux_device_class_pima_parameter_device_prop_value_set = USBD_MTP_SetDevicePropValue;
  pima_mtp_parameter.ux_device_class_pima_parameter_storage_format        = USBD_MTP_FormatStorage;
  pima_mtp_parameter.ux_device_class_pima_parameter_storage_info_get      = USBD_MTP_GetStorageInfo;
  pima_mtp_parameter.ux_device_class_pima_parameter_object_number_get     = USBD_MTP_GetObjectNumber;
  pima_mtp_parameter.ux_device_class_pima_parameter_object_handles_get    = USBD_MTP_GetObjectHandles;
  pima_mtp_parameter.ux_device_class_pima_parameter_object_info_get       = USBD_MTP_GetObjectInfo;
  pima_mtp_parameter.ux_device_class_pima_parameter_object_data_get       = USBD_MTP_GetObjectData;
  pima_mtp_parameter.ux_device_class_pima_parameter_object_info_send      = USBD_MTP_SendObjectInfo;
  pima_mtp_parameter.ux_device_class_pima_parameter_object_data_send      = USBD_MTP_SendObjectData;
  pima_mtp_parameter.ux_device_class_pima_parameter_object_delete         = USBD_MTP_DeleteObject;
  pima_mtp_parameter.ux_device_class_pima_parameter_object_prop_desc_get  = USBD_MTP_GetObjectPropDesc;
  pima_mtp_parameter.ux_device_class_pima_parameter_object_prop_value_get = USBD_MTP_GetObjectPropValue;
  pima_mtp_parameter.ux_device_class_pima_parameter_object_prop_value_set = USBD_MTP_SetObjectPropValue;
  pima_mtp_parameter.ux_device_class_pima_parameter_object_references_get = USBD_MTP_GetObjectReferences;
  pima_mtp_parameter.ux_device_class_pima_parameter_object_references_set = USBD_MTP_SetObjectReferences;

  /* Store the instance owner */
  pima_mtp_parameter.ux_device_class_pima_parameter_application = NULL;

  /* USER CODE BEGIN PIMA_MTP_PARAMETER */

  /* USER CODE END PIMA_MTP_PARAMETER */

  /* Get pima mtp configuration number */
  pima_mtp_configuration_number = USBD_Get_Configuration_Number(CLASS_TYPE_PIMA_MTP, 0);

  /* Find pima mtp interface number */
  pima_mtp_interface_number = USBD_Get_Interface_Number(CLASS_TYPE_PIMA_MTP, 0);

  /* Initialize the device pima mtp class */
  if (ux_device_stack_class_register(_ux_system_slave_class_pima_name,
                                     ux_device_class_pima_entry,
                                     pima_mtp_configuration_number,
                                     pima_mtp_interface_number,
                                     &pima_mtp_parameter) != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_DEVICE_MTP_REGISTER_ERROR */
    return UX_ERROR;
    /* USER CODE END USBX_DEVICE_MTP_REGISTER_ERROR */
  }

  /* Allocate the stack for device application main thread */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, UX_DEVICE_APP_THREAD_STACK_SIZE,
                       TX_NO_WAIT) != TX_SUCCESS)
  {
    /* USER CODE BEGIN MAIN_THREAD_ALLOCATE_STACK_ERROR */
    return TX_POOL_ERROR;
    /* USER CODE END MAIN_THREAD_ALLOCATE_STACK_ERROR */
  }

  /* Create the device application main thread */
  if (tx_thread_create(&ux_device_app_thread, UX_DEVICE_APP_THREAD_NAME, app_ux_device_thread_entry,
                       0, pointer, UX_DEVICE_APP_THREAD_STACK_SIZE, UX_DEVICE_APP_THREAD_PRIO,
                       UX_DEVICE_APP_THREAD_PREEMPTION_THRESHOLD, UX_DEVICE_APP_THREAD_TIME_SLICE,
                       UX_DEVICE_APP_THREAD_START_OPTION) != TX_SUCCESS)
  {
    /* USER CODE BEGIN MAIN_THREAD_CREATE_ERROR */
    return TX_THREAD_ERROR;
    /* USER CODE END MAIN_THREAD_CREATE_ERROR */
  }

  /* USER CODE BEGIN MX_USBX_Device_Init1 */

  /* USER CODE END MX_USBX_Device_Init1 */

  return ret;
}

/**
  * @brief  Function implementing app_ux_device_thread_entry.
  * @param  thread_input: User thread input parameter.
  * @retval none
  */
static VOID app_ux_device_thread_entry(ULONG thread_input)
{
  /* USER CODE BEGIN app_ux_device_thread_entry */

  /* Initialization of USB device */
  USBX_APP_Device_Init();

  /* USER CODE END app_ux_device_thread_entry */
}

/* USER CODE BEGIN 1 */

/**
  * @brief  USBX_APP_Device_Init
  *         Initialization of USB device.
  * @param  none
  * @retval none
  */
VOID USBX_APP_Device_Init(VOID)
{
  /* USER CODE BEGIN USB_Device_Init_PreTreatment_0 */
  /* USER CODE END USB_Device_Init_PreTreatment_0 */

  /* USB_OTG_HS init function */
  MX_USB_OTG_HS_PCD_Init();

  /* USER CODE BEGIN USB_Device_Init_PreTreatment_1 */

  /* Set Rx FIFO */
  HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_HS, 0x200);
  /* Set Tx FIFO 0 */
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 0, 0x40);
  /* Set Tx FIFO 1 */
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 1, 0x140);
    /* Set Tx FIFO 2 */
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 2, 0x20);

  /* USER CODE END USB_Device_Init_PreTreatment_1 */

  /* Initialize and link controller HAL driver */
  ux_dcd_stm32_initialize((ULONG)USB_OTG_HS, (ULONG)&hpcd_USB_OTG_HS);

  /* Start USB device */
  HAL_PCD_Start(&hpcd_USB_OTG_HS);

  /* USER CODE BEGIN USB_Device_Init_PostTreatment */

  /* USER CODE END USB_Device_Init_PostTreatment */
}

/* USER CODE END 1 */
