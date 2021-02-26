/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_usbx_device.c
  * @author  MCD Application Team
  * @brief   USBX Device applicative file
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
#include "app_usbx_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usb_otg.h"
#include "ux_api.h"
#include "ux_system.h"
#include "ux_utility.h"
#include "ux_device_stack.h"
#include "ux_dcd_stm32.h"
#include "ux_device_descriptors.h"
#include "ux_device_class_storage.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* Thread priority */
#define DEFAULT_THREAD_PRIO                   10

/* Thread preemption priority */
#define DEFAULT_PREEMPTION_THRESHOLD          DEFAULT_THREAD_PRIO

/* USB App Stack Size */
#define USBX_APP_STACK_SIZE                   (2 * 1024)

/* Usb Memory Size */
#define USBX_MEMORY_SIZE                      (32 * 1024)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

TX_THREAD                           ux_app_thread;
TX_EVENT_FLAGS_GROUP                EventFlag;
UX_SLAVE_CLASS_STORAGE_PARAMETER    storage_parameter;
CHAR                                *pointer;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

VOID  usbx_app_thread_entry(ULONG arg);

UINT  app_usb_device_state_change(ULONG event);

UINT  app_usb_device_thread_media_read(VOID *storage, ULONG lun,
                                       UCHAR *data_pointer, ULONG number_blocks,
                                       ULONG lba, ULONG *media_status);

UINT  app_usb_device_thread_media_write(VOID *storage, ULONG lun,
                                        UCHAR *data_pointer, ULONG number_blocks,
                                        ULONG lba, ULONG *media_status);

UINT  app_usb_device_thread_media_status(VOID *storage, ULONG lun,
                                         ULONG media_id, ULONG *media_status);

/* USER CODE END PFP */
/**
  * @brief  Application USBX Device Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_USBX_Device_Init(VOID *memory_ptr)
{
  UINT ret = UX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;
  /* USER CODE BEGIN App_USBX_Device_Init */
  ULONG device_framework_hs_length;
  ULONG device_framework_fs_length;
  ULONG string_framework_length;
  ULONG languge_id_framework_length;
  UCHAR *device_framework_high_speed;
  UCHAR *device_framework_full_speed;
  UCHAR *string_framework;
  UCHAR *language_id_framework;

  /* Allocate the USBX_MEMORY_SIZE. */
  tx_byte_allocate(byte_pool, (VOID **) &pointer,
                   USBX_MEMORY_SIZE, TX_NO_WAIT);

  /* Initialize USBX Memory */
  ux_system_initialize(pointer, USBX_MEMORY_SIZE, UX_NULL, 0);

  /* Get_Device_Framework_High_Speed and get the length */
  device_framework_high_speed = USBD_Get_Device_Framework_Speed(USBD_HIGH_SPEED,
                                &device_framework_hs_length);

  /* Get_Device_Framework_Full_Speed and get the length */
  device_framework_full_speed = USBD_Get_Device_Framework_Speed(USBD_FULL_SPEED,
                                &device_framework_fs_length);

  /* Get_String_Framework and get the length */
  string_framework = USBD_Get_String_Framework(&string_framework_length);

  /* Get_Language_Id_Framework and get the length */
  language_id_framework = USBD_Get_Language_Id_Framework(&languge_id_framework_length);

  /* The code below is required for installing the device portion of USBX.
     In this application */
  ret =  _ux_device_stack_initialize(device_framework_high_speed,
                                     device_framework_hs_length,
                                     device_framework_full_speed,
                                     device_framework_fs_length,
                                     string_framework,
                                     string_framework_length,
                                     language_id_framework,
                                     languge_id_framework_length, UX_NULL);

  if (ret != UX_SUCCESS)
  {
    return ret;
  }

  /* Store the number of LUN in this device storage instance. */
  storage_parameter.ux_slave_class_storage_parameter_number_lun = 1;

  /* Initialize the storage class parameters for reading/writing to the Flash Disk. */
  storage_parameter.ux_slave_class_storage_parameter_lun[0].
  ux_slave_class_storage_media_last_lba = (ULONG)(USBD_SD_CardInfo.BlockNbr - 1);

  storage_parameter.ux_slave_class_storage_parameter_lun[0].
  ux_slave_class_storage_media_block_length = USBD_SD_CardInfo.BlockSize;

  storage_parameter.ux_slave_class_storage_parameter_lun[0].
  ux_slave_class_storage_media_type = 0;

  storage_parameter.ux_slave_class_storage_parameter_lun[0].
  ux_slave_class_storage_media_removable_flag = 0x80;

  storage_parameter.ux_slave_class_storage_parameter_lun[0].
  ux_slave_class_storage_media_read = app_usb_device_thread_media_read;

  storage_parameter.ux_slave_class_storage_parameter_lun[0].
  ux_slave_class_storage_media_write = app_usb_device_thread_media_write;

  storage_parameter.ux_slave_class_storage_parameter_lun[0].
  ux_slave_class_storage_media_status = app_usb_device_thread_media_status;

  /* Initialize the device storage class. The class is connected with interface 0 on configuration 1. */
  ret =  _ux_device_stack_class_register(_ux_system_slave_class_storage_name,
                                         _ux_device_class_storage_entry,
                                         1, 0, (VOID *)&storage_parameter);

  if (ret != UX_SUCCESS)
  {
    return ret;
  }

  /* Allocate the stack for main_usbx_app_thread_entry. */
  tx_byte_allocate(byte_pool, (VOID **) &pointer,
                   USBX_APP_STACK_SIZE, TX_NO_WAIT);

  /* Create the usbx_app_thread_entry.  */
  ret = tx_thread_create(&ux_app_thread, "main_usbx_app_thread_entry",
                         usbx_app_thread_entry, 0,
                         pointer, USBX_APP_STACK_SIZE,
                         DEFAULT_THREAD_PRIO, DEFAULT_PREEMPTION_THRESHOLD,
                         TX_NO_TIME_SLICE, TX_AUTO_START);

  /* Create the event flags group.  */
  if (tx_event_flags_create(&EventFlag, "Event Flag") != TX_SUCCESS)
  {
    ret = TX_GROUP_ERROR;
  }

  /* USER CODE END App_USBX_Device_Init */

  return ret;
}

/* USER CODE BEGIN 1 */
/**
  * @brief  Function implementing usbx_app_thread_entry.
  * @param arg: Not used
  * @retval None
  */
void usbx_app_thread_entry(ULONG arg)
{
  /* Sleep for 100 ms */
  tx_thread_sleep(0.1 * TX_TIMER_TICKS_PER_SECOND);

  /* Initialization of USB device */
  MX_USB_Device_Init();
}

/**
  * @brief MX_USB_Device_Init
  * Initialization of USB device.
  * Init USB device Library, add supported class and start the library
  * @retval None
  */
void MX_USB_Device_Init(void)
{
  /* USER CODE BEGIN USB_Device_Init_PreTreatment_0 */
  /* USER CODE END USB_Device_Init_PreTreatment_0 */

  HAL_PWREx_EnableUSBVoltageDetector();

  /* initialize the device controller HAL driver */
  MX_USB_OTG_HS_PCD_Init();

  /* USER CODE BEGIN USB_Device_Init_PreTreatment_1 */
  HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_HS, 0x200);
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 0, 0x40);
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 1, 0x100);
  /* USER CODE END USB_Device_Init_PreTreatment_1 */

  /* initialize and link controller HAL driver to USBx */
  _ux_dcd_stm32_initialize((ULONG)USB_OTG_HS, (ULONG)&hpcd_USB_OTG_HS);

  /* Start USB device by connecting the DP pullup */
  HAL_PCD_Start(&hpcd_USB_OTG_HS);

  /* USER CODE BEGIN USB_Device_Init_PostTreatment */
  /* USER CODE END USB_Device_Init_PostTreatment */
}

/* USER CODE END 1 */
