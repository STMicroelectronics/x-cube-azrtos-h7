/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_usbx_device.c
  * @author  MCD Application Team
  * @brief   USBX Device applicative file
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

static ULONG ccid_interface_number;
static ULONG ccid_configuration_number;
static UX_DEVICE_CLASS_CCID_PARAMETER ccid_parameter;
static TX_THREAD ux_device_app_thread;
extern UX_DEVICE_CLASS_CCID_HANDLES USBD_CCID_Handles;
extern ULONG USBD_CCID_Clocks[];
extern ULONG USBD_CCID_DataRates[];
extern PCD_HandleTypeDef hpcd_USB_OTG_HS;

/* USER CODE BEGIN PV */
TX_THREAD ux_ccid_thread;
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

  /* Allocate the stack for ccid thread */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       UX_DEVICE_APP_THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the ccid thread */
  if (tx_thread_create(&ux_ccid_thread, "ccid_usbx_app_thread_entry",
                       usbx_ccid_thread_entry, 1, pointer,
                       UX_DEVICE_APP_THREAD_STACK_SIZE, 20, 20, TX_NO_TIME_SLICE,
                       TX_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
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

  /* Initialize the ccid class parameters for the device */
  ccid_parameter.ux_device_class_ccid_handles             = &USBD_CCID_Handles;
  ccid_parameter.ux_device_class_ccid_instance_activate   = USBD_CCID_Activate;
  ccid_parameter.ux_device_class_ccid_instance_deactivate = USBD_CCID_Deactivate;
  ccid_parameter.ux_device_class_ccid_max_n_slots         = USBD_CCID_MAX_SLOT_INDEX;
  ccid_parameter.ux_device_class_ccid_max_n_busy_slots    = USBD_CCID_MAX_BUSY_SLOTS;
  ccid_parameter.ux_device_class_ccid_max_transfer_length = USBD_CCID_MAX_BLOCK_SIZE_HEADER;
  ccid_parameter.ux_device_class_ccid_n_clocks            = USBD_CCID_N_CLOCKS;
  ccid_parameter.ux_device_class_ccid_clocks              = USBD_CCID_Clocks;
  ccid_parameter.ux_device_class_ccid_n_data_rates        = USBD_CCID_N_DATA_RATES;
  ccid_parameter.ux_device_class_ccid_data_rates          = USBD_CCID_DataRates;

  /* USER CODE BEGIN CCID_PARAMETER */

  /* USER CODE END CCID_PARAMETER */

  /* Get ccid configuration number */
  ccid_configuration_number = USBD_Get_Configuration_Number(CLASS_TYPE_CCID, 0);

  /* Find ccid interface number */
  ccid_interface_number = USBD_Get_Interface_Number(CLASS_TYPE_CCID, 0);

  /* Initialize the device CCID class */
  if (ux_device_stack_class_register(_ux_system_device_class_ccid_name,
                                     ux_device_class_ccid_entry,
                                     ccid_configuration_number,
                                     ccid_interface_number,
                                     &ccid_parameter) != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_DEVICE_CCID_REGISTER_ERROR */
    return UX_ERROR;
    /* USER CODE END USBX_DEVICE_CCID_REGISTER_ERROR */
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

  /* Unregister CCID class. */
  if (ux_device_stack_class_unregister(_ux_system_device_class_ccid_name,
                                     ux_device_class_ccid_entry) != UX_SUCCESS)
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

  /* USB_OTG_HS init function */
  MX_USB_OTG_HS_PCD_Init();

  /* Initialize the Stack USB Device*/
  if (MX_USBX_Device_Stack_Init() != UX_SUCCESS)
  {
    /* USER CODE BEGIN MAIN_INITIALIZE_STACK_ERROR */
    Error_Handler();
    /* USER CODE END MAIN_INITIALIZE_STACK_ERROR */
  }

  /* Start the USB device */
  HAL_PCD_Start(&hpcd_USB_OTG_HS);

  /* USER CODE END app_ux_device_thread_entry */
}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
