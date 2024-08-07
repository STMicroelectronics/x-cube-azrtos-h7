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

static ULONG cdc_ecm_interface_number;
static ULONG cdc_ecm_configuration_number;
static UCHAR cdc_ecm_local_nodeid[UX_DEVICE_CLASS_CDC_ECM_NODE_ID_LENGTH];
static UCHAR cdc_ecm_remote_nodeid[UX_DEVICE_CLASS_CDC_ECM_NODE_ID_LENGTH];
static UX_SLAVE_CLASS_CDC_ECM_PARAMETER cdc_ecm_parameter;
static TX_THREAD ux_device_app_thread;

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

  /* Initialize the cdc ecm class parameters for the device */
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_instance_activate   = USBD_CDC_ECM_Activate;
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_instance_deactivate = USBD_CDC_ECM_Deactivate;

  /* Get CDC ECM local MAC address */
  USBD_CDC_ECM_GetMacAdd((uint8_t *)CDC_ECM_LOCAL_MAC_STR_DESC, cdc_ecm_local_nodeid);

  /* Define CDC ECM local node id */
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_local_node_id[0] = cdc_ecm_local_nodeid[0];
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_local_node_id[1] = cdc_ecm_local_nodeid[1];
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_local_node_id[2] = cdc_ecm_local_nodeid[2];
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_local_node_id[3] = cdc_ecm_local_nodeid[3];
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_local_node_id[4] = cdc_ecm_local_nodeid[4];
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_local_node_id[5] = cdc_ecm_local_nodeid[5];

  /* Get CDC ECM remote MAC address */
  USBD_CDC_ECM_GetMacAdd((uint8_t *)CDC_ECM_REMOTE_MAC_STR_DESC, cdc_ecm_remote_nodeid);

  /* Define CDC ECM remote node id */
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_remote_node_id[0] = cdc_ecm_remote_nodeid[0];
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_remote_node_id[1] = cdc_ecm_remote_nodeid[1];
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_remote_node_id[2] = cdc_ecm_remote_nodeid[2];
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_remote_node_id[3] = cdc_ecm_remote_nodeid[3];
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_remote_node_id[4] = cdc_ecm_remote_nodeid[4];
  cdc_ecm_parameter.ux_slave_class_cdc_ecm_parameter_remote_node_id[5] = cdc_ecm_remote_nodeid[5];

  /* USER CODE BEGIN CDC_ECM_PARAMETER */

  /* USER CODE END CDC_ECM_PARAMETER */

  /* Get cdc ecm configuration number */
  cdc_ecm_configuration_number = USBD_Get_Configuration_Number(CLASS_TYPE_CDC_ECM, 0);

  /* Find cdc ecm interface number */
  cdc_ecm_interface_number = USBD_Get_Interface_Number(CLASS_TYPE_CDC_ECM, 0);

  /* Initialize the device cdc ecm class */
  if (ux_device_stack_class_register(_ux_system_slave_class_cdc_ecm_name,
                                     ux_device_class_cdc_ecm_entry,
                                     cdc_ecm_configuration_number,
                                     cdc_ecm_interface_number,
                                     &cdc_ecm_parameter) != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_DEVICE_CDC_ECM_REGISTER_ERROR */
    return UX_ERROR;
    /* USER CODE END USBX_DEVICE_CDC_ECM_REGISTER_ERROR */
  }

  /* Perform the initialization of the network driver. This will initialize the
     USBX network layer */

  ux_network_driver_init();

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
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 0, 0x10);
  /* Set Tx FIFO 1 */
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 1, 0x80);
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
