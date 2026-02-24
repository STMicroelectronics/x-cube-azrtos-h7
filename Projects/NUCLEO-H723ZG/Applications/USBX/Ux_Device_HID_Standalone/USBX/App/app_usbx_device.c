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

static ULONG hid_mouse_interface_number;
static ULONG hid_mouse_configuration_number;
static UX_SLAVE_CLASS_HID_PARAMETER hid_mouse_parameter;
extern PCD_HandleTypeDef hpcd_USB_OTG_HS;

/* USER CODE BEGIN PV */

extern uint8_t User_Button_State;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/**
  * @brief  Application USBX Device Initialization.
  * @param  none
  * @retval status
  */
UINT MX_USBX_Device_Init(VOID)
{

  UINT ret = UX_SUCCESS;

  /* USER CODE BEGIN MX_USBX_Device_Init0 */

  /* USB_OTG_HS init function */
  MX_USB_OTG_HS_PCD_Init();

  /* USER CODE END MX_USBX_Device_Init0 */

  /* Initialize the Stack USB Device*/
  if (MX_USBX_Device_Stack_Init() != UX_SUCCESS)
  {
    /* USER CODE BEGIN MAIN_INITIALIZE_STACK_ERROR */
    return UX_ERROR;
    /* USER CODE END MAIN_INITIALIZE_STACK_ERROR */
  }

  /* USER CODE BEGIN MX_USBX_Device_Init1 */

  /* Start the USB device */
  HAL_PCD_Start(&hpcd_USB_OTG_HS);

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

  /* Initialize the hid mouse class parameters for the device */
  hid_mouse_parameter.ux_slave_class_hid_instance_activate         = USBD_HID_Mouse_Activate;
  hid_mouse_parameter.ux_slave_class_hid_instance_deactivate       = USBD_HID_Mouse_Deactivate;
  hid_mouse_parameter.ux_device_class_hid_parameter_report_address = USBD_HID_ReportDesc(INTERFACE_HID_MOUSE);
  hid_mouse_parameter.ux_device_class_hid_parameter_report_length  = USBD_HID_ReportDesc_length(INTERFACE_HID_MOUSE);
  hid_mouse_parameter.ux_device_class_hid_parameter_report_id      = UX_FALSE;
  hid_mouse_parameter.ux_device_class_hid_parameter_callback       = USBD_HID_Mouse_SetReport;
  hid_mouse_parameter.ux_device_class_hid_parameter_get_callback   = USBD_HID_Mouse_GetReport;

  /* USER CODE BEGIN HID_MOUSE_PARAMETER */

  /* USER CODE END HID_MOUSE_PARAMETER */

  /* Get hid mouse configuration number */
  hid_mouse_configuration_number = USBD_Get_Configuration_Number(CLASS_TYPE_HID, INTERFACE_HID_MOUSE);

  /* Find hid mouse interface number */
  hid_mouse_interface_number = USBD_Get_Interface_Number(CLASS_TYPE_HID, INTERFACE_HID_MOUSE);

  /* Initialize the device hid Mouse class */
  if (ux_device_stack_class_register(_ux_system_slave_class_hid_name,
                                     ux_device_class_hid_entry,
                                     hid_mouse_configuration_number,
                                     hid_mouse_interface_number,
                                     &hid_mouse_parameter) != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_DEVICE_HID_MOUSE_REGISTER_ERROR */
    return UX_ERROR;
    /* USER CODE END USBX_DEVICE_HID_MOUSE_REGISTER_ERROR */
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

/* Unregister hid class. */
  if (ux_device_stack_class_unregister(_ux_system_slave_class_hid_name,
                                       ux_device_class_hid_entry) != UX_SUCCESS)
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

/* USER CODE BEGIN 2 */
/**
  * @brief  MX_USBX_Device_Process
  *         Run USBX state machine.
  * @param  arg: not used
  * @retval none
  */
VOID USBX_Device_Process(VOID *arg)
{
  ux_device_stack_tasks_run();
  USBX_DEVICE_HID_MOUSE_Task();
}

/**
  * @brief  HAL_GPIO_EXTI_Callback
  *         EXTI line detection callback.
  * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

  /* Check if EXTI from User Button */
  if (GPIO_Pin == BUTTON_USER_Pin)
  {
    User_Button_State ^= 1U;
  }
}

/* USER CODE END 2 */
