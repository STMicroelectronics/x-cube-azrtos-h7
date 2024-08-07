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

static ULONG dfu_interface_number;
static ULONG dfu_configuration_number;
static UX_SLAVE_CLASS_DFU_PARAMETER dfu_parameter;
static TX_THREAD ux_device_app_thread;

/* USER CODE BEGIN PV */

extern UINT Leave_DFU_State;

TX_QUEUE ux_app_MsgQueue;
static TX_THREAD usbx_dfu_download_thread;

#if defined ( __ICCARM__ ) /* IAR Compiler */
  #pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
__ALIGN_BEGIN ux_dfu_downloadInfotypeDef  ux_dfu_download  __ALIGN_END;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static VOID app_ux_device_thread_entry(ULONG thread_input);
static UINT USBD_ChangeFunction(ULONG Device_State);
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
                                 USBD_ChangeFunction) != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_DEVICE_INITIALIZE_ERROR */
    return UX_ERROR;
    /* USER CODE END USBX_DEVICE_INITIALIZE_ERROR */
  }

  /* Initialize the dfu class parameters for the device */
  dfu_parameter.ux_slave_class_dfu_parameter_instance_activate   = USBD_DFU_Activate;
  dfu_parameter.ux_slave_class_dfu_parameter_instance_deactivate = USBD_DFU_Deactivate;
  dfu_parameter.ux_slave_class_dfu_parameter_get_status          = USBD_DFU_GetStatus;
  dfu_parameter.ux_slave_class_dfu_parameter_read                = USBD_DFU_Read;
  dfu_parameter.ux_slave_class_dfu_parameter_write               = USBD_DFU_Write;
  dfu_parameter.ux_slave_class_dfu_parameter_notify              = USBD_DFU_Notify;
#ifdef UX_DEVICE_CLASS_DFU_CUSTOM_REQUEST_ENABLE
  dfu_parameter.ux_device_class_dfu_parameter_custom_request     = USBD_DFU_CustomRequest;
#endif /* UX_DEVICE_CLASS_DFU_CUSTOM_REQUEST_ENABLE */
  dfu_parameter.ux_slave_class_dfu_parameter_framework           = device_framework_full_speed;
  dfu_parameter.ux_slave_class_dfu_parameter_framework_length    = device_framework_fs_length;

  /* USER CODE BEGIN DFU_PARAMETER */

  /* USER CODE END DFU_PARAMETER */

  /* Get dfu configuration number */
  dfu_configuration_number = USBD_Get_Configuration_Number(CLASS_TYPE_DFU, 0);

  /* Find dfu interface number */
  dfu_interface_number = USBD_Get_Interface_Number(CLASS_TYPE_DFU, 0);

  /* Initialize the device dfu class */
  if (ux_device_stack_class_register(_ux_system_slave_class_dfu_name,
                                     ux_device_class_dfu_entry,
                                     dfu_configuration_number,
                                     dfu_interface_number,
                                     &dfu_parameter) != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_DEVICE_DFU_REGISTER_ERROR */
    return UX_ERROR;
    /* USER CODE END USBX_DEVICE_DFU_REGISTER_ERROR */
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

  /* Allocate the stack for dfu download thread */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, 1024, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the dfu download thread */
  if (tx_thread_create(&usbx_dfu_download_thread, "usbx_dfu_download_thread_entry",
                       usbx_dfu_download_thread_entry, 0, pointer, 1024,
                       20, 20, TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

  /* Allocate Memory for the Queue */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       APP_QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the MsgQueue */
  if (tx_queue_create(&ux_app_MsgQueue, "Message Queue app", TX_1_ULONG,
                      pointer, APP_QUEUE_SIZE * sizeof(ULONG)) != TX_SUCCESS)
  {
    return TX_QUEUE_ERROR;
  }

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

/**
  * @brief  USBD_ChangeFunction
  *         This function is called when the device state changes.
  * @param  Device_State: USB Device State
  * @retval status
  */
static UINT USBD_ChangeFunction(ULONG Device_State)
{
   UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_ChangeFunction0 */

  /* USER CODE END USBD_ChangeFunction0 */

  switch (Device_State)
  {
    case UX_DEVICE_ATTACHED:

      /* USER CODE BEGIN UX_DEVICE_ATTACHED */

      /* USER CODE END UX_DEVICE_ATTACHED */

      break;

    case UX_DEVICE_REMOVED:

      /* USER CODE BEGIN UX_DEVICE_REMOVED */

      if (_ux_system_slave -> ux_system_slave_device_dfu_mode == UX_DEVICE_CLASS_DFU_MODE_DFU)
      {
        if (Leave_DFU_State != LEAVE_DFU_DISABLED)
        {
          /* Generate system reset to allow jumping to the user code */
          NVIC_SystemReset();
        }
      }

      /* USER CODE END UX_DEVICE_REMOVED */

      break;

    case UX_DCD_STM32_DEVICE_CONNECTED:

      /* USER CODE BEGIN UX_DCD_STM32_DEVICE_CONNECTED */

      /* USER CODE END UX_DCD_STM32_DEVICE_CONNECTED */

      break;

    case UX_DCD_STM32_DEVICE_DISCONNECTED:

      /* USER CODE BEGIN UX_DCD_STM32_DEVICE_DISCONNECTED */

      /* USER CODE END UX_DCD_STM32_DEVICE_DISCONNECTED */

      break;

    case UX_DCD_STM32_DEVICE_SUSPENDED:

      /* USER CODE BEGIN UX_DCD_STM32_DEVICE_SUSPENDED */

      /* USER CODE END UX_DCD_STM32_DEVICE_SUSPENDED */

      break;

    case UX_DCD_STM32_DEVICE_RESUMED:

      /* USER CODE BEGIN UX_DCD_STM32_DEVICE_RESUMED */

      /* USER CODE END UX_DCD_STM32_DEVICE_RESUMED */

      break;

    case UX_DCD_STM32_SOF_RECEIVED:

      /* USER CODE BEGIN UX_DCD_STM32_SOF_RECEIVED */

      /* USER CODE END UX_DCD_STM32_SOF_RECEIVED */

      break;

    default:

      /* USER CODE BEGIN DEFAULT */

      /* USER CODE END DEFAULT */

      break;

  }

  /* USER CODE BEGIN USBD_ChangeFunction1 */

  /* USER CODE END USBD_ChangeFunction1 */

  return status;
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
  /* Set Tx FIFO */
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 0, 0x80);

  /* USER CODE END USB_Device_Init_PreTreatment_1 */

  /* Initialize and link controller HAL driver */
  ux_dcd_stm32_initialize((ULONG)USB_OTG_HS, (ULONG)&hpcd_USB_OTG_HS);

  /* Start the USB device */
  HAL_PCD_Start(&hpcd_USB_OTG_HS);

  /* USER CODE BEGIN USB_Device_Init_PostTreatment */

  /* USER CODE END USB_Device_Init_PostTreatment */
}

/* USER CODE END 1 */
