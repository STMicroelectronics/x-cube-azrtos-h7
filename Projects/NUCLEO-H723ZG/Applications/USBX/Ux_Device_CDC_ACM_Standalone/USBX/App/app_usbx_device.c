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
#define USBX_MEMORY_SIZE          (7 * 1024)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma location = 0x24029000
#elif defined ( __CC_ARM ) /* MDK ARM Compiler */
__attribute__((section(".UsbxPoolSection")))
#elif defined ( __GNUC__ ) /* GNU Compiler */
__attribute__((section(".UsbxPoolSection")))
#endif
static CHAR ux_system_memory_pool[USBX_MEMORY_SIZE];
UX_SLAVE_CLASS_CDC_ACM_PARAMETER    cdc_acm_parameter;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
VOID *  _tx_thread_system_stack_ptr;
/* USER CODE END PFP */

/**
  * @brief  Application FileX Initialization.
  * @param  void
  * @retval int
  */
UINT MX_USBX_Device_Init(VOID)
{
  UINT ret = UX_SUCCESS;

  /* USER CODE BEGIN MX_USBX_Device_MEM_POOL */
  /* USER CODE END MX_USBX_Device_MEM_POOL */

  /* USER CODE BEGIN MX_USBX_Device_Init */
  CHAR *pointer;
  ULONG device_framework_fs_length;
  ULONG string_framework_length;
  ULONG languge_id_framework_length;
  UCHAR *device_framework_full_speed;
  UCHAR *string_framework;
  UCHAR *language_id_framework;

  /* Use static memory block.  */
  pointer = ux_system_memory_pool;

  /* Initialize USBX Memory */
  if (ux_system_initialize(pointer, USBX_MEMORY_SIZE, UX_NULL, 0) != UX_SUCCESS)
  {
    return UX_ERROR;
  }

  /* Get_Device_Framework_Full_Speed and get the length */
  device_framework_full_speed = USBD_Get_Device_Framework_Speed(USBD_FULL_SPEED,
                                &device_framework_fs_length);

  /* Get_String_Framework and get the length */
  string_framework = USBD_Get_String_Framework(&string_framework_length);

  /* Get_Language_Id_Framework and get the length */
  language_id_framework = USBD_Get_Language_Id_Framework(&languge_id_framework_length);

  /* The code below is required for installing the device portion of USBX.
     In this application */
  if (ux_device_stack_initialize(NULL,
                                 0,
                                 device_framework_full_speed,
                                 device_framework_fs_length,
                                 string_framework,
                                 string_framework_length,
                                 language_id_framework,
                                 languge_id_framework_length,
                                 UX_NULL) != UX_SUCCESS)
  {
    Error_Handler();
  }

  /* Initialize the cdc class parameters for the device. */
  cdc_acm_parameter.ux_slave_class_cdc_acm_instance_activate    = CDC_ACM_Init;
  cdc_acm_parameter.ux_slave_class_cdc_acm_instance_deactivate  = CDC_ACM_DeInit;
  cdc_acm_parameter.ux_slave_class_cdc_acm_parameter_change     = CDC_ACM_Control;

  /* Registers a slave class to the slave stack. The class is connected with
     interface 0 */
  if (ux_device_stack_class_register(_ux_system_slave_class_cdc_acm_name,
                                     ux_device_class_cdc_acm_entry, 1, 0,
                                     (VOID *)&cdc_acm_parameter) != UX_SUCCESS)
  {
    Error_Handler();
  }
  
  MX_USB_Device_Init();

  /* USER CODE END MX_USBX_Device_Init */

  return ret;
}

/* USER CODE BEGIN 1 */
/**
  * @brief MX_USBX_Device_Process
  *        Run USBX state machine.
  * @retval None
  */
VOID MX_USBX_Device_Process(VOID *arg)
{
  ux_device_stack_tasks_run();
  CDC_ACM_Read_Task();
  CDC_ACM_Write_Task();
}
/**
  * @brief MX_USB_Device_Init
  *        Initialization of USB device.
  * Init USB device Library, add supported class and start the library
  * @retval None
  */
VOID MX_USB_Device_Init(VOID)
{
  /* USER CODE BEGIN USB_Device_Init_PreTreatment_0 */
  /* USER CODE END USB_Device_Init_PreTreatment_0 */

  /* Enable the USB voltage level detector */
  HAL_PWREx_EnableUSBVoltageDetector();

  /* USB_OTG_HS init function */
  MX_USB_OTG_HS_PCD_Init();

  /* USER CODE BEGIN USB_Device_Init_PreTreatment_1 */
  HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_HS, 0x200);
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 0, 0x10);
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 1, 0x20);
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 2, 0x10);
  /* USER CODE END USB_Device_Init_PreTreatment_1 */

  /* initialize the device controller driver */
  ux_dcd_stm32_initialize((ULONG)USB_OTG_HS, (ULONG)&hpcd_USB_OTG_HS);

  /* Start the USB device */
  HAL_PCD_Start(&hpcd_USB_OTG_HS);

  /* USER CODE BEGIN USB_Device_Init_PostTreatment */
  /* USER CODE END USB_Device_Init_PostTreatment */
}
  

ALIGN_TYPE _ux_utility_interrupt_disable(VOID)
{
  __istate_t interrupt_save;
  interrupt_save = __get_interrupt_state();
  __disable_interrupt();
  return interrupt_save;
}
VOID _ux_utility_interrupt_restore(ALIGN_TYPE flags)
{

  __set_interrupt_state(flags);

}
/* Time Tick Get for host timing  */
ULONG _ux_utility_time_get(VOID)
{
#if UX_PERIODIC_RATE != 1000
#warning UX_PERIODIC_RATE should be 1000 for 1ms tick
#endif
  return(HAL_GetTick());
}
/* USER CODE END 1 */
