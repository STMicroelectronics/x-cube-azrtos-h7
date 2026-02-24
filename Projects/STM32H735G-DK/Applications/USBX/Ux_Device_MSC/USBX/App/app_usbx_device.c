/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_usbx_device.c
  * @author  MCD Application Team
  * @brief   USBX Device applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
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
#include "app_usbx.h"

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

static ULONG storage_interface_number;
static ULONG storage_configuration_number;
static UX_SLAVE_CLASS_STORAGE_PARAMETER storage_parameter;
static TX_THREAD ux_device_app_thread;
extern PCD_HandleTypeDef hpcd_USB_OTG_HS;

/* USER CODE BEGIN PV */
TX_EVENT_FLAGS_GROUP EventFlag;
extern HAL_SD_CardInfoTypeDef USBD_SD_CardInfo;
uint8_t USB_Device_State = STOP_USB_DEVICE;
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
  UCHAR *pointer;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

  /* USER CODE BEGIN MX_USBX_Device_Init0 */

  /* USER CODE END MX_USBX_Device_Init0 */

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

  /* Create the event flags group */
  if (tx_event_flags_create(&EventFlag, "Event Flag") != TX_SUCCESS)
  {
    return TX_GROUP_ERROR;
  }

  /* USER CODE END MX_USBX_Device_Init1 */

  return ret;
}

/**
  * @brief  Application USBX Device Initialization.
  * @param  None
  * @retval ret
  */
UINT MX_USBX_Device_Stack_Init(void)
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

  /* USER CODE BEGIN MX_USBX_Device_Stack_Init_PreTreatment */

  /* USER CODE END MX_USBX_Device_Stack_Init_PreTreatment */

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

  /* Initialize the storage class parameters for the device */
  storage_parameter.ux_slave_class_storage_instance_activate   = USBD_STORAGE_Activate;
  storage_parameter.ux_slave_class_storage_instance_deactivate = USBD_STORAGE_Deactivate;

  /* Store the number of LUN in this device storage instance */
  storage_parameter.ux_slave_class_storage_parameter_number_lun = STORAGE_NUMBER_LUN;

  /* Initialize the storage class parameters for reading/writing to the Flash Disk */
  storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_last_lba = USBD_STORAGE_GetMediaLastLba();

  storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_block_length = USBD_STORAGE_GetMediaBlocklength();

  storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_type = 0;

  storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_removable_flag = STORAGE_REMOVABLE_FLAG;

  storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_read_only_flag = STORAGE_READ_ONLY;

  storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_read = USBD_STORAGE_Read;

  storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_write = USBD_STORAGE_Write;

  storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_flush = USBD_STORAGE_Flush;

  storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_status = USBD_STORAGE_Status;

  storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_notification = USBD_STORAGE_Notification;

  /* USER CODE BEGIN STORAGE_PARAMETER */

  /* USER CODE END STORAGE_PARAMETER */

  /* Get storage configuration number */
  storage_configuration_number = USBD_Get_Configuration_Number(CLASS_TYPE_MSC, 0);

  /* Find storage interface number */
  storage_interface_number = USBD_Get_Interface_Number(CLASS_TYPE_MSC, 0);

  /* Initialize the device storage class */
  if (ux_device_stack_class_register(_ux_system_slave_class_storage_name,
                                     ux_device_class_storage_entry,
                                     storage_configuration_number,
                                     storage_interface_number,
                                     &storage_parameter) != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_DEVICE_STORAGE_REGISTER_ERROR */
    return UX_ERROR;
    /* USER CODE END USBX_DEVICE_STORAGE_REGISTER_ERROR */
  }

  /* Initialize and link controller HAL driver */
  ux_dcd_stm32_initialize((ULONG)USB_OTG_HS, (ULONG)&hpcd_USB_OTG_HS);

  /* USER CODE BEGIN MX_USBX_Device_Stack_Init_PostTreatment */

  /* USER CODE END MX_USBX_Device_Stack_Init_PostTreatment */

return ret;

}

/**
  * @brief MX_USBX_Device_Stack_DeInit
  *        Unitialization of USB Device.
  * uninitialize the device stack, unregister of device class stack
  * unregister of the usb device controller
  * @param  None
  * @retval ret
  */
UINT MX_USBX_Device_Stack_DeInit(void)
{
  UINT ret = UX_SUCCESS;

  /* USER CODE BEGIN MX_USBX_Device_Stack_DeInit_PreTreatment */

  /* USER CODE END MX_USBX_Device_Stack_DeInit_PreTreatment */

  /* Uninitialize and unlink controller HAL driver */
  ux_dcd_stm32_uninitialize((ULONG)USB_OTG_HS, (ULONG)&hpcd_USB_OTG_HS);

/* Unregister storage class. */
  if (ux_device_stack_class_unregister(_ux_system_slave_class_storage_name,
                                       ux_device_class_storage_entry) != UX_SUCCESS)
  {
    return UX_ERROR;
  }

  /* The code below is required for uninstalling the device portion of USBX.  */
  if (ux_device_stack_uninitialize() != UX_SUCCESS)
  {
    return UX_ERROR;
  }

  /* USER CODE BEGIN MX_USBX_Device_Stack_DeInit_PostTreatment */

  /* USER CODE END MX_USBX_Device_Stack_DeInit_PostTreatment */

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
  UINT status;

  /* USB_OTG_HS init function */
  MX_USB_OTG_HS_PCD_Init();

  /* Check if SD card is plugged and USB cable is connected */
  if ((HAL_GPIO_ReadPin(SD_DETECT_GPIO_Port, SD_DETECT_Pin) == GPIO_PIN_RESET) &&
      (HAL_GPIO_ReadPin(USB_DETECT_GPIO_Port, USB_DETECT_Pin) == GPIO_PIN_SET))
    {
      MX_SDMMC1_SD_Init();
      /* Get SD card info */
      status = HAL_SD_GetCardInfo(&hsd1, &USBD_SD_CardInfo);

      if (status != HAL_OK)
      {
        Error_Handler();
      }

      if (MX_USBX_Device_Stack_Init() != UX_SUCCESS)
      {
        /* USER CODE BEGIN MAIN_INITIALIZE_STACK_ERROR */
        Error_Handler();
        /* USER CODE END MAIN_INITIALIZE_STACK_ERROR */
      }

      /* Start USB device */
      HAL_PCD_Start(&hpcd_USB_OTG_HS);
      USB_Device_State = START_USB_DEVICE;
    }

  while(1)
  {
    ULONG actual_events;
    /* Wait for an event */
    tx_event_flags_get(&EventFlag, 0x30, TX_OR_CLEAR, &actual_events, TX_WAIT_FOREVER);

    tx_thread_sleep(MS_TO_TICK(1000));
    if ((HAL_GPIO_ReadPin(SD_DETECT_GPIO_Port, SD_DETECT_Pin) == GPIO_PIN_RESET) &&
        (HAL_GPIO_ReadPin(USB_DETECT_GPIO_Port, USB_DETECT_Pin) == GPIO_PIN_SET) &&
        (USB_Device_State == STOP_USB_DEVICE))
    {
      USB_Device_State = START_USB_DEVICE;
      MX_SDMMC1_SD_Init();
      /* Get SD card info */
      status = HAL_SD_GetCardInfo(&hsd1, &USBD_SD_CardInfo);
      if (status != HAL_OK)
      {
        Error_Handler();
      }
      if (MX_USBX_Device_Stack_Init() != UX_SUCCESS)
      {
        /* USER CODE BEGIN MAIN_INITIALIZE_STACK_ERROR */
        Error_Handler();
        /* USER CODE END MAIN_INITIALIZE_STACK_ERROR */
      }
      /* Start USB device */
      HAL_PCD_Start(&hpcd_USB_OTG_HS);
    }

    if (((HAL_GPIO_ReadPin(SD_DETECT_GPIO_Port, SD_DETECT_Pin) == GPIO_PIN_SET) || (HAL_GPIO_ReadPin(USB_DETECT_GPIO_Port, USB_DETECT_Pin) == GPIO_PIN_RESET)) && (USB_Device_State == START_USB_DEVICE))
    {
      USB_Device_State = STOP_USB_DEVICE;
      MX_SDMMC1_SD_DeInit();
      ux_device_stack_disconnect();
      HAL_PCD_Stop(&hpcd_USB_OTG_HS);
      if (MX_USBX_Device_Stack_DeInit() != UX_SUCCESS)
      {
        /* USER CODE BEGIN MAIN_DEINITIALIZE_STACK_ERROR */
        Error_Handler();
        /* USER CODE END MAIN_DEINITIALIZE_STACK_ERROR */
      }
    }
  }
  /* USER CODE END app_ux_device_thread_entry */
}

/* USER CODE BEGIN 2 */

/**
  * @brief  EXTI line detection callback.
  * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if ((GPIO_Pin == SD_DETECT_Pin) || (GPIO_Pin == USB_DETECT_Pin))
  {
    tx_event_flags_set(&EventFlag, 0x30, TX_OR);
  }
}

/* USER CODE END 2 */
