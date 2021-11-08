/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_usbx_host.c
  * @author  MCD Application Team
  * @brief   USBX host applicative file
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
#include "app_usbx_host.h"

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
/* USER CODE BEGIN PV */
extern HCD_HandleTypeDef               hhcd_USB_OTG_HS;
UX_HOST_CLASS_CDC_ACM                  *app_cdc_acm;
TX_QUEUE                               ux_app_MsgQueue;
TX_EVENT_FLAGS_GROUP                   ux_app_EventFlag;
TX_THREAD                              ux_app_thread;
TX_THREAD                              cdc_acm_send_thread;
TX_THREAD                              cdc_acm_recieve_thread;
ux_app_stateTypeDef                    ux_app_state;
TX_BYTE_POOL                           *ux_app_byte_pool;

#if defined ( __ICCARM__ ) /* IAR Compiler */
  #pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
__ALIGN_BEGIN ux_app_devInfotypeDef     ux_dev_info  __ALIGN_END;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
extern void MX_USB_OTG_HS_HCD_Init(void);
extern void cdc_acm_send_app_thread_entry(ULONG arg);
extern void cdc_acm_recieve_app_thread_entry(ULONG arg);
extern void Error_Handler(void);
/* USER CODE END PFP */
/**
  * @brief  Application USBX Host Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT MX_USBX_Host_Init(VOID *memory_ptr)
{
  UINT ret = UX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

  /* USER CODE BEGIN MX_USBX_Host_MEM_POOL */
  /* USER CODE END MX_USBX_Host_MEM_POOL */

  /* USER CODE BEGIN MX_USBX_Host_Init */
  CHAR *pointer;

  /* Store byte_pool into ux_app_byte_bool */
  ux_app_byte_pool = byte_pool;

  /* Allocate the stack for USBX.*/
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       USBX_MEMORY_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Initialize USBX memory. */
  if (ux_system_initialize(pointer, USBX_MEMORY_SIZE, UX_NULL, 0) != UX_SUCCESS)
  {
    return UX_ERROR;
  }

  /* register a callback error function */
  _ux_utility_error_callback_register(&ux_host_error_callback);

  /* Allocate the stack for thread 0.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       USBX_APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the main App thread.  */
  if (tx_thread_create(&ux_app_thread, "thread 0", usbx_app_thread_entry, 0,
                       pointer, USBX_APP_STACK_SIZE, 25, 25, 0,
                       TX_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

  /* Allocate the stack for thread 1.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       (USBX_APP_STACK_SIZE * 3), TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the cdc_acm_recieve thread.  */
  if (tx_thread_create(&cdc_acm_recieve_thread, "thread 1", cdc_acm_recieve_app_thread_entry, 0,
                       pointer, (USBX_APP_STACK_SIZE * 3), 30, 30, 0,
                       TX_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

  /* Allocate the stack for thread 2.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       (USBX_APP_STACK_SIZE * 2), TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the cdc_acm_send  thread.  */
  if (tx_thread_create(&cdc_acm_send_thread, "thread 2", cdc_acm_send_app_thread_entry, 0,
                       pointer, (USBX_APP_STACK_SIZE * 2), 30, 30, 0,
                       TX_AUTO_START) != TX_SUCCESS)
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
    ret = TX_QUEUE_ERROR;
  }

  /* Create the event flags group. */
  if (tx_event_flags_create(&ux_app_EventFlag, "Event Flag") != TX_SUCCESS)
  {
    return TX_GROUP_ERROR;
  }

  /* USER CODE END MX_USBX_Host_Init */

  return ret;
}

/* USER CODE BEGIN 1 */

/**
  * @brief  Application_thread_entry .
  * @param  ULONG arg
  * @retval Void
  */
void  usbx_app_thread_entry(ULONG arg)
{
  /* Initialize USBX_Host */
  MX_USB_Host_Init();

  /* Start Application */
  USBH_UsrLog(" **** USB OTG HS in FS CDC Host **** \n");
  USBH_UsrLog("USB Host library started.\n");

  /* Initialize Application process */
  USBH_UsrLog("Starting CDC Application");
  USBH_UsrLog("Connect your CDC Device\n");

  while (1)
  {
    if (tx_queue_receive(&ux_app_MsgQueue, &ux_dev_info, TX_WAIT_FOREVER) != TX_SUCCESS)
    {
      Error_Handler();
    }

    if ((ux_dev_info.Dev_state == Device_connected) && (ux_dev_info.Device_Type == CDC_ACM_Device))
    {
      if (app_cdc_acm -> ux_host_class_cdc_acm_interface -> ux_interface_descriptor.bInterfaceClass == UX_HOST_CLASS_CDC_DATA_CLASS)
      {
        /* update Application process */
        ux_app_state = App_Ready;

        /* Print Device informayions such as PID and VID */
        USBH_UsrLog("USB CDC Device Found");
        USBH_UsrLog("PID: %#x ", (UINT)app_cdc_acm -> ux_host_class_cdc_acm_device -> ux_device_descriptor.idProduct);
        USBH_UsrLog("VID: %#x ", (UINT)app_cdc_acm -> ux_host_class_cdc_acm_device -> ux_device_descriptor.idVendor);
        USBH_UsrLog("Data Interface initialized");
      }
      else
      {
        USBH_UsrLog("Unable to initialize data Interface");
        ux_app_state = App_Idle;
      }
    }
  }
}

/**
  * @brief MX_USB_Host_Init
  *        Initialization of USB Host.
  * Init USB Host Library, add supported class and start the library
  * @retval None
  */
UINT MX_USB_Host_Init(void)
{
  UINT ret = UX_SUCCESS;
  /* USER CODE BEGIN USB_Host_Init_PreTreatment_0 */
  /* USER CODE END USB_Host_Init_PreTreatment_0 */

  /* The code below is required for installing the host portion of USBX.  */
  if (ux_host_stack_initialize(ux_host_event_callback) != UX_SUCCESS)
  {
    return UX_ERROR;
  }

  /* Register cdc_acm class. */
  if ((ux_host_stack_class_register(_ux_system_host_class_cdc_acm_name,
                                    _ux_host_class_cdc_acm_entry)) != UX_SUCCESS)
  {
    return UX_ERROR;
  }

  /* Initialize the LL driver */
  MX_USB_OTG_HS_HCD_Init();

  /* Register all the USB host controllers available in this system. */
  if (ux_host_stack_hcd_register(_ux_system_host_hcd_stm32_name,
                                 _ux_hcd_stm32_initialize,
                                 USB_OTG_HS_PERIPH_BASE,
                                 (ULONG)&hhcd_USB_OTG_HS) != UX_SUCCESS)
  {
    return UX_ERROR;
  }

  /* Drive vbus to be addedhere */
  USBH_DriverVBUS(USB_VBUS_TRUE);

  /* Enable USB Global Interrupt */
  HAL_HCD_Start(&hhcd_USB_OTG_HS);

  /* USER CODE BEGIN USB_Host_Init_PreTreatment_1 */
  /* Initialize user_button to handle sending data */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

  /* USER CODE END USB_Host_Init_PreTreatment_1 */


  /* USER CODE BEGIN USB_Host_Init_PostTreatment */
  /* USER CODE END USB_Host_Init_PostTreatment */
  return ret ;
}

/**
* @brief ux_host_event_callback
* @param ULONG event
           This parameter can be one of the these values:
             1 : UX_DEVICE_INSERTION
             2 : UX_DEVICE_REMOVAL
         UX_HOST_CLASS * Current_class
         VOID * Current_instance
* @retval Status
*/
UINT ux_host_event_callback(ULONG event, UX_HOST_CLASS *Current_class, VOID *Current_instance)
{
  UINT                   status;
  UX_HOST_CLASS          *cdc_acm_class;

  switch (event)
  {
    case UX_DEVICE_INSERTION :

      /* Device connection */
      ux_dev_info.Dev_state = Device_connected;

      status = ux_host_stack_class_get(_ux_system_host_class_cdc_acm_name, &cdc_acm_class);

      if (status == UX_SUCCESS)
      {
        if (cdc_acm_class == Current_class)
        {
          app_cdc_acm             = Current_instance;
          ux_dev_info.Device_Type = CDC_ACM_Device;

          /* Check if this is a data interface */
          if (app_cdc_acm -> ux_host_class_cdc_acm_interface -> ux_interface_descriptor.bInterfaceClass == UX_HOST_CLASS_CDC_DATA_CLASS)
          {
            tx_queue_send(&ux_app_MsgQueue, &ux_dev_info, TX_NO_WAIT);
          }
        }
      }
      else
      {
        ux_dev_info.Device_Type = Unsupported_Device;
        tx_queue_send(&ux_app_MsgQueue, &ux_dev_info, TX_NO_WAIT);
      }
      break;

    case UX_DEVICE_REMOVAL :
      if (app_cdc_acm != NULL)
      {
      USBH_UsrLog("Device Removal");
      app_cdc_acm = NULL;
      ux_dev_info.Dev_state   = Device_disconnected;
      ux_dev_info.Device_Type = Unknown_Device;
      tx_queue_send(&ux_app_MsgQueue, &ux_dev_info, TX_NO_WAIT);
      }
      break;

    default:
      break;
  }

  return (UINT) UX_SUCCESS;
}

/**
* @brief ux_host_error_callback
* @param ULONG event
         UINT system_context
         UINT error_code
* @retval Status
*/
VOID ux_host_error_callback(UINT system_level, UINT system_context, UINT error_code)
{
  switch (error_code)
  {
    case UX_DEVICE_ENUMERATION_FAILURE :
      USBH_UsrLog("Enumeration Failure");
      break;

    case UX_NO_DEVICE_CONNECTED :
      USBH_UsrLog("USB Device disconnected");
      break;

    default:
      break;
  }
}

/**
* @brief  Drive VBUS.
* @param  state : VBUS state
*          This parameter can be one of the these values:
*           1 : VBUS Active
*           0 : VBUS Inactive
* @retval Status
*/
void USBH_DriverVBUS(uint8_t state)
{
  /* USER CODE BEGIN 0 */

  /* USER CODE END 0*/

  if (state == USB_VBUS_TRUE)
  {
    /* Drive high Charge pump */
    /* Add IOE driver control */
    /* USER CODE BEGIN DRIVE_HIGH_CHARGE_FOR_HS */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, GPIO_PIN_RESET);
    /* USER CODE END DRIVE_HIGH_CHARGE_FOR_HS */
  }
  else
  {
    /* Drive low Charge pump */
    /* Add IOE driver control */
    /* USER CODE BEGIN DRIVE_LOW_CHARGE_FOR_HS */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, GPIO_PIN_SET);
    /* USER CODE END DRIVE_LOW_CHARGE_FOR_HS */
  }

  HAL_Delay(200);
}

/**
  * @brief  GPIO EXTI Callback function
  *         Handle Sending data through key button
  * @param  GPIO_Pin
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == BUTTON_KEY_PIN)
  {
    /* Set NEW_DATA_TO_SEND flag */
    if (tx_event_flags_set(&ux_app_EventFlag, NEW_DATA_TO_SEND, TX_OR) != TX_SUCCESS)
    {
      Error_Handler();
    }
  }
}
/* USER CODE END 1 */
