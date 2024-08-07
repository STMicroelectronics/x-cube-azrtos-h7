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
#define COMPOSITE_DEVICE                0x000000EF
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

static TX_THREAD ux_host_app_thread;

/* USER CODE BEGIN PV */
TX_THREAD                  cdc_acm_send_thread;
TX_THREAD                  cdc_acm_recieve_thread;
TX_THREAD                  keyboard_app_thread;
TX_THREAD                  mouse_app_thread;
TX_EVENT_FLAGS_GROUP       ux_app_EventFlag;
UX_HOST_CLASS_CDC_ACM      *cdc_acm;
UX_HOST_CLASS_HID          *hid_instance;
UX_HOST_CLASS_HID_MOUSE    *mouse;
UX_HOST_CLASS_HID_KEYBOARD *keyboard;

extern uint16_t                        RxSzeIdx;
extern UX_HOST_CLASS_CDC_ACM_RECEPTION cdc_acm_reception;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static VOID app_ux_host_thread_entry(ULONG thread_input);
static UINT ux_host_event_callback(ULONG event, UX_HOST_CLASS *current_class, VOID *current_instance);
static VOID ux_host_error_callback(UINT system_level, UINT system_context, UINT error_code);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/**
  * @brief  Application USBX Host Initialization.
  * @param  memory_ptr: memory pointer
  * @retval status
  */
UINT MX_USBX_Host_Init(VOID *memory_ptr)
{
  UINT ret = UX_SUCCESS;
  UCHAR *pointer;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

  /* USER CODE BEGIN MX_USBX_Host_Init0 */

  /* USER CODE END MX_USBX_Host_Init0 */

  /* Allocate the stack for USBX Memory */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       USBX_HOST_MEMORY_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_ALLOCATE_STACK_ERROR */
    return TX_POOL_ERROR;
    /* USER CODE END USBX_ALLOCATE_STACK_ERROR */
  }

  /* Initialize USBX Memory */
  if (ux_system_initialize(pointer, USBX_HOST_MEMORY_STACK_SIZE, UX_NULL, 0) != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_SYSTEM_INITIALIZE_ERROR */
    return UX_ERROR;
    /* USER CODE END USBX_SYSTEM_INITIALIZE_ERROR */
  }

  /* Install the host portion of USBX */
  if (ux_host_stack_initialize(ux_host_event_callback) != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_HOST_INITIALIZE_ERROR */
    return UX_ERROR;
    /* USER CODE END USBX_HOST_INITIALIZE_ERROR */
  }

  /* Register a callback error function */
  ux_utility_error_callback_register(&ux_host_error_callback);

  /* Initialize the host hid class */
  if (ux_host_stack_class_register(_ux_system_host_class_hid_name,
                                   ux_host_class_hid_entry) != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_HSOT_HID_REGISTER_ERROR */
    return UX_ERROR;
    /* USER CODE END USBX_HSOT_HID_REGISTER_ERROR */
  }

  /* Initialize the host hid mouse client */
  if (ux_host_class_hid_client_register(_ux_system_host_class_hid_client_mouse_name,
                                        ux_host_class_hid_mouse_entry) != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_HOST_HID_MOUSE_REGISTER_ERROR */
    return UX_ERROR;
    /* USER CODE END USBX_HOST_HID_MOUSE_REGISTER_ERROR */
  }

  /* Initialize the host hid keyboard client */
  if (ux_host_class_hid_client_register(_ux_system_host_class_hid_client_keyboard_name,
                                        ux_host_class_hid_keyboard_entry) != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_HOST_HID_KEYBOARD_REGISTER_ERROR */
    return UX_ERROR;
    /* USER CODE END USBX_HOST_HID_KEYBOARD_REGISTER_ERROR */
  }

  /* Initialize the host cdc acm class */
  if ((ux_host_stack_class_register(_ux_system_host_class_cdc_acm_name,
                                    ux_host_class_cdc_acm_entry)) != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_HOST_CDC_ACM_REGISTER_ERROR */
    return UX_ERROR;
    /* USER CODE END USBX_HOST_CDC_ACM_REGISTER_ERROR */
  }

  /* Allocate the stack for host application main thread */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, UX_HOST_APP_THREAD_STACK_SIZE,
                       TX_NO_WAIT) != TX_SUCCESS)
  {
    /* USER CODE BEGIN MAIN_THREAD_ALLOCATE_STACK_ERROR */
    return TX_POOL_ERROR;
    /* USER CODE END MAIN_THREAD_ALLOCATE_STACK_ERROR */
  }

  /* Create the host application main thread */
  if (tx_thread_create(&ux_host_app_thread, UX_HOST_APP_THREAD_NAME, app_ux_host_thread_entry,
                       0, pointer, UX_HOST_APP_THREAD_STACK_SIZE, UX_HOST_APP_THREAD_PRIO,
                       UX_HOST_APP_THREAD_PREEMPTION_THRESHOLD, UX_HOST_APP_THREAD_TIME_SLICE,
                       UX_HOST_APP_THREAD_START_OPTION) != TX_SUCCESS)
  {
    /* USER CODE BEGIN MAIN_THREAD_CREATE_ERROR */
    return TX_THREAD_ERROR;
    /* USER CODE END MAIN_THREAD_CREATE_ERROR */
  }

  /* USER CODE BEGIN MX_USBX_Host_Init1 */

  /* Allocate the stack for cdc_acm receive thread */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       UX_HOST_APP_THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the cdc_acm_recieve thread */
  if (tx_thread_create(&cdc_acm_recieve_thread, "CDC Receive App thread",
                       cdc_acm_recieve_app_thread_entry, 0, pointer,
                       UX_HOST_APP_THREAD_STACK_SIZE, 30, 30, 0,
                       TX_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

  /* Allocate the stack for cdc_acm send thread */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       UX_HOST_APP_THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the cdc_acm_send thread */
  if (tx_thread_create(&cdc_acm_send_thread, "CDC Read App thread",
                       cdc_acm_send_app_thread_entry, 0, pointer,
                       UX_HOST_APP_THREAD_STACK_SIZE, 30, 30, 0,
                       TX_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

/* Allocate the stack for HID mouse App thread */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       UX_HOST_APP_THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the HID mouse App thread */
  if (tx_thread_create(&mouse_app_thread, "HID mouse App thread", hid_mouse_thread_entry,
                       0, pointer, UX_HOST_APP_THREAD_STACK_SIZE, 30, 30, 1, TX_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

  /* Allocate the stack for HID Keyboard App thread */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       UX_HOST_APP_THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the HID Keyboard App thread */
  if (tx_thread_create(&keyboard_app_thread, "HID Keyboard App thread", hid_keyboard_thread_entry,
                       0, pointer, UX_HOST_APP_THREAD_STACK_SIZE, 30, 30, 1, TX_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

  /* Create the event flags group */
  if (tx_event_flags_create(&ux_app_EventFlag, "Event Flag") != TX_SUCCESS)
  {
    return TX_GROUP_ERROR;
  }

  /* USER CODE END MX_USBX_Host_Init1 */

  return ret;
}

/**
  * @brief  Function implementing app_ux_host_thread_entry.
  * @param  thread_input: User thread input parameter.
  * @retval none
  */
static VOID app_ux_host_thread_entry(ULONG thread_input)
{
  /* USER CODE BEGIN app_ux_host_thread_entry */

  /* Initialization of USB host */
  USBX_APP_Host_Init();

  /* USER CODE END app_ux_host_thread_entry */
}

/**
  * @brief  ux_host_event_callback
  *         This callback is invoked to notify application of instance changes.
  * @param  event: event code.
  * @param  current_class: Pointer to class.
  * @param  current_instance: Pointer to class instance.
  * @retval status
  */
UINT ux_host_event_callback(ULONG event, UX_HOST_CLASS *current_class, VOID *current_instance)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN ux_host_event_callback0 */

  /* Get current Hid Client */
  UX_HOST_CLASS_HID_CLIENT *client  = (UX_HOST_CLASS_HID_CLIENT *)current_instance;

  /* USER CODE END ux_host_event_callback0 */

  switch (event)
  {
    case UX_DEVICE_INSERTION:

      /* USER CODE BEGIN UX_DEVICE_INSERTION */

      /* Get current CDC Class */
      if (current_class -> ux_host_class_entry_function == ux_host_class_cdc_acm_entry)
      {
        if (cdc_acm == UX_NULL)
        {
          /* Get current CDC Instance */
          cdc_acm = (UX_HOST_CLASS_CDC_ACM *)current_instance;

          /* Check if this is CDC DATA instance */
          if (cdc_acm -> ux_host_class_cdc_acm_bulk_in_endpoint == UX_NULL)
          {
            cdc_acm = UX_NULL;
          }
          else
          {
            USBH_UsrLog("USB CDC Device Found");
          }
        }
      }

      /* Get current Hid Class */
      if (current_class -> ux_host_class_entry_function == ux_host_class_hid_entry)
      {
        if (hid_instance == UX_NULL)
        {
          /* Get current Hid Instance */
          hid_instance = (UX_HOST_CLASS_HID *)current_instance;
        }
      }
      /* USER CODE END UX_DEVICE_INSERTION */

      break;

    case UX_DEVICE_REMOVAL:

      /* USER CODE BEGIN UX_DEVICE_REMOVAL */

      if ((VOID*)cdc_acm == current_instance)
      {
        /* Clear cdc instance */
        cdc_acm = UX_NULL;

        /* Reinitialize reception block size index */
        RxSzeIdx=0;

        USBH_UsrLog("\nUSB CDC ACM Device Removal");

        /* Set REMOVED_CDC_INSTANCE flag */
        if (tx_event_flags_set(&ux_app_EventFlag, REMOVED_CDC_INSTANCE, TX_OR) != TX_SUCCESS)
        {
          Error_Handler();
        }
      }

      if ((VOID*)hid_instance == current_instance)
      {
        /* Clear hid instance */
        hid_instance = UX_NULL;
      }

      /* USER CODE END UX_DEVICE_REMOVAL */

      break;

    case UX_HID_CLIENT_INSERTION:

      /* USER CODE BEGIN UX_HID_CLIENT_INSERTION */

      USBH_UsrLog("\nHID Client Plugged");

      /* Check the HID_client if this is a HID keyboard device */
      if (client -> ux_host_class_hid_client_handler == ux_host_class_hid_keyboard_entry)
      {
        /* Get current Hid Client */
        if (keyboard == UX_NULL)
        {
          keyboard = client -> ux_host_class_hid_client_local_instance;

          USBH_UsrLog("HID_Keyboard_Device Found");
        }
      }

      /* Check the HID_client if this is a HID mouse device */
      if (client -> ux_host_class_hid_client_handler == ux_host_class_hid_mouse_entry)
      {
        /* Get current Hid Client */
        if (mouse == UX_NULL)
        {
          mouse = client -> ux_host_class_hid_client_local_instance;

          USBH_UsrLog("HID_Mouse_Device Found");
        }
      }

      /* USER CODE END UX_HID_CLIENT_INSERTION */

      break;

    case UX_HID_CLIENT_REMOVAL:

      /* USER CODE BEGIN UX_HID_CLIENT_REMOVAL */

      /* Clear hid client local instance */
      if ((VOID*)keyboard == client -> ux_host_class_hid_client_local_instance)
      {
        /* Clear hid keyboard instance */
        keyboard = UX_NULL;

        USBH_UsrLog("\nHID Client Keyboard Unplugged");
      }

      if ((VOID*)mouse == client -> ux_host_class_hid_client_local_instance)
      {
        /* Clear hid mouse instance */
        mouse = UX_NULL;

        USBH_UsrLog("\nHID Client Mouse Unplugged");
      }

      /* USER CODE END UX_HID_CLIENT_REMOVAL */

      break;

    case UX_DEVICE_CONNECTION:

      /* USER CODE BEGIN UX_DEVICE_CONNECTION */

      if (_ux_system_host -> ux_system_host_device_array->ux_device_descriptor.bDeviceClass == COMPOSITE_DEVICE)
      {
        /* Composite interface type */
        USBH_UsrLog("\nComposite Interface initialized");
      }
      else
      {
         /* Simple interface type */
        USBH_UsrLog("\nSimple Interface initialized");
      }

      USBH_UsrLog("\nPID: %#x ", (UINT)_ux_system_host -> ux_system_host_device_array->ux_device_descriptor.idProduct);
      USBH_UsrLog("VID: %#x ", (UINT)_ux_system_host -> ux_system_host_device_array->ux_device_descriptor.idVendor);

      if (keyboard != UX_NULL)
      {
        USBH_UsrLog("keyboard is ready...");
      }

      if (mouse != UX_NULL)
      {
        USBH_UsrLog("Mouse is ready...");
      }

      if (cdc_acm != UX_NULL)
      {
        USBH_UsrLog("CDC Data Interface initialized");
      }

      /* USER CODE END UX_DEVICE_CONNECTION */

      break;

    case UX_DEVICE_DISCONNECTION:

      /* USER CODE BEGIN UX_DEVICE_DISCONNECTION */

      /* USER CODE END UX_DEVICE_DISCONNECTION */

      break;

    default:

      /* USER CODE BEGIN EVENT_DEFAULT */

      /* USER CODE END EVENT_DEFAULT */

      break;
  }

  /* USER CODE BEGIN ux_host_event_callback1 */

  /* USER CODE END ux_host_event_callback1 */

  return status;
}

/**
  * @brief ux_host_error_callback
  *         This callback is invoked to notify application of error changes.
  * @param  system_level: system level parameter.
  * @param  system_context: system context code.
  * @param  error_code: error event code.
  * @retval Status
  */
VOID ux_host_error_callback(UINT system_level, UINT system_context, UINT error_code)
{
  /* USER CODE BEGIN ux_host_error_callback0 */

  /* USER CODE END ux_host_error_callback0 */

  switch (error_code)
  {
    case UX_DEVICE_ENUMERATION_FAILURE:

      /* USER CODE BEGIN UX_DEVICE_ENUMERATION_FAILURE */

      USBH_UsrLog("USB Device Enumeration Failure");

      /* USER CODE END UX_DEVICE_ENUMERATION_FAILURE */

      break;

    case  UX_NO_DEVICE_CONNECTED:

      /* USER CODE BEGIN UX_NO_DEVICE_CONNECTED */

      USBH_UsrLog("USB Device disconnected");

      /* USER CODE END UX_NO_DEVICE_CONNECTED */

      break;

    default:

      /* USER CODE BEGIN ERROR_DEFAULT */

      /* USER CODE END ERROR_DEFAULT */

      break;
  }

  /* USER CODE BEGIN ux_host_error_callback1 */

  /* USER CODE END ux_host_error_callback1 */
}

/* USER CODE BEGIN 1 */
/**
  * @brief  USBX_APP_Host_Init
  *         Initialization of USB Host.
  * @param  none
  * @retval none
  */
VOID USBX_APP_Host_Init(VOID)
{
  /* USER CODE BEGIN USB_Host_Init_PreTreatment_0 */

  /* USER CODE END USB_Host_Init_PreTreatment_0 */

  /* Initialize the LL driver */
  MX_USB_OTG_HS_HCD_Init();

  /* Initialize the host controller driver */
  ux_host_stack_hcd_register(_ux_system_host_hcd_stm32_name,
                             _ux_hcd_stm32_initialize, USB_OTG_HS_PERIPH_BASE,
                             (ULONG)&hhcd_USB_OTG_HS);

  /* Drive vbus */
  USBH_DriverVBUS(USB_VBUS_TRUE);

  /* Enable USB Global Interrupt */
  HAL_HCD_Start(&hhcd_USB_OTG_HS);

  /* USER CODE BEGIN USB_Host_Init_PreTreatment1 */

  /* Start Application Message */
  USBH_UsrLog(" **** USB OTG HS composite Host **** \n");
  USBH_UsrLog("USB Host library started.\n");

  /* Wait for Device to be attached */
  USBH_UsrLog("Starting Composite Application");
  USBH_UsrLog("Connect your composite HID/CDC_ACM Device");

  /* USER CODE END USB_Host_Init_PreTreatment1 */
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
    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_5, GPIO_PIN_SET);
    /* USER CODE END DRIVE_HIGH_CHARGE_FOR_HS */
  }
  else
  {
    /* Drive low Charge pump */
    /* Add IOE driver control */
    /* USER CODE BEGIN DRIVE_LOW_CHARGE_FOR_HS */
    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_5, GPIO_PIN_RESET);
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
  if (GPIO_Pin == GPIO_PIN_13)
  {
    /* Set NEW_DATA_TO_SEND flag */
    if (tx_event_flags_set(&ux_app_EventFlag, NEW_DATA_TO_SEND, TX_OR) != TX_SUCCESS)
    {
      Error_Handler();
    }
  }
}
/* USER CODE END 1 */
