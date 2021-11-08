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
/* USER CODE BEGIN PV */
extern HCD_HandleTypeDef                 hhcd_USB_OTG_HS;
TX_THREAD                                ux_app_thread;
TX_THREAD                                keyboard_app_thread;
TX_THREAD                                mouse_app_thread;
TX_QUEUE                                 ux_app_MsgQueue;
UX_HOST_CLASS_HID                        *hid;
UX_HOST_CLASS_HID_CLIENT                 *hid_client;
UX_HOST_CLASS_HID_MOUSE                  *mouse;
UX_HOST_CLASS_HID_KEYBOARD               *keyboard;
UX_HOST_CLASS_CDC_ACM                    *app_cdc_acm;
TX_EVENT_FLAGS_GROUP                     ux_app_EventFlag;
TX_THREAD                                cdc_acm_send_thread;
TX_THREAD                                cdc_acm_recieve_thread;
ux_app_stateTypeDef                      ux_app_state;
TX_BYTE_POOL                             *ux_app_byte_pool;
UINT                                     flag_interface_type = 0;

#if defined ( __ICCARM__ ) /* IAR Compiler */
  #pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
__ALIGN_BEGIN ux_app_devInfotypeDef       ux_dev_info  __ALIGN_END;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static UINT  MX_USB_Host_Init(void);
static void  USBH_DriverVBUS(uint8_t state);
void  usbx_app_thread_entry(ULONG arg);
VOID  ux_host_error_callback(UINT system_level, UINT system_context, UINT error_code);
UINT  ux_host_event_callback(ULONG event, UX_HOST_CLASS *p_host_class, VOID *p_instance);
extern void Error_Handler(void);
extern void MX_USB_OTG_HS_HCD_Init(void);
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

  /* Allocate the stack for thread 0. */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       USBX_MEMORY_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
  }

  /* Initialize USBX memory. */
  if (ux_system_initialize(pointer, USBX_MEMORY_SIZE, UX_NULL, 0) != UX_SUCCESS)

  {
    ret = UX_ERROR;
  }

  /* register a callback error function */
  _ux_utility_error_callback_register(&ux_host_error_callback);

  /* Allocate the stack for thread 0. */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       USBX_APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
  }

  /* Create the main App thread. */
  if (tx_thread_create(&ux_app_thread, "usbx_app_thread_entry",
                       usbx_app_thread_entry, 0, pointer, USBX_APP_STACK_SIZE,
                       25, 25, 1, TX_AUTO_START) != TX_SUCCESS)
  {
    ret = TX_THREAD_ERROR;
  }

  /* Allocate the stack for mouse thread . */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       USBX_APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
  }

  /* Create the HID mouse App thread. */
  if (tx_thread_create(&mouse_app_thread, "mouse thread", hid_mouse_thread_entry,
                       0, pointer, USBX_APP_STACK_SIZE, 26, 26, 1,
                       TX_AUTO_START) != TX_SUCCESS)
  {
    ret = TX_THREAD_ERROR;
  }

  /* Allocate the stack for thread 2. */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       USBX_APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
  }

  /* Create the HID Keyboard App thread. */
  if (tx_thread_create(&keyboard_app_thread, "Keyborad thread ",
                       hid_keyboard_thread_entry, 0, pointer,
                       USBX_APP_STACK_SIZE, 30, 30, 1,
                       TX_AUTO_START) != TX_SUCCESS)
  {
    ret = TX_THREAD_ERROR;
  }

  /* Allocate the stack for thread.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       (USBX_APP_STACK_SIZE * 3), TX_NO_WAIT) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
  }

  /* Create the cdc_acm_recieve thread.  */
  if (tx_thread_create(&cdc_acm_recieve_thread, "cdc_acm_recieve thread",
                       cdc_acm_recieve_app_thread_entry, 0,
                       pointer, (USBX_APP_STACK_SIZE * 3), 30, 30, 0,
                       TX_AUTO_START) != TX_SUCCESS)
  {
    ret = TX_THREAD_ERROR;
  }

  /* Allocate the stack for thread .  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       (USBX_APP_STACK_SIZE * 2), TX_NO_WAIT) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
  }

  /* Create the cdc_acm_send thread.  */
  if (tx_thread_create(&cdc_acm_send_thread, "cdc_acm_send thread",
                       cdc_acm_send_app_thread_entry, 0, pointer,
                       (USBX_APP_STACK_SIZE * 2), 30, 30, 0,
                       TX_AUTO_START) != TX_SUCCESS)
  {
    ret = TX_THREAD_ERROR;
  }

  /* Allocate Memory for the Queue */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       APP_QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
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
    ret = TX_GROUP_ERROR;
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

  /* Start Application Message */
  USBH_UsrLog(" **** USB OTG FS composite Host **** \n");

  USBH_UsrLog("USB Host library started.\n");

  /* Wait for Device to be attached */
  USBH_UsrLog("Starting Composite Application");

  USBH_UsrLog("Connect your composite HID/CDC_ACM Device\n");

  while (1)
  {
    /* Wait for a device to be connected */
    if (tx_queue_receive(&ux_app_MsgQueue, &ux_dev_info,
                         TX_WAIT_FOREVER)!= TX_SUCCESS)
    {
     /*Error*/
     Error_Handler();
    }

    /* Check if the device is connected */
    if (ux_dev_info.Dev_state == Device_connected)
    {
      /* Check if the device is CDC_ACM */
      if (ux_dev_info.CDC_Device_Type == CDC_ACM_Device)
      {
        /* Update Application process status */
        ux_app_state = App_Ready;

        /* Print interface type */
        USBH_UsrLog("CDC_ACM interface Found");
      }
      /* Check if the device HID is Mouse or Keyboard */
      else if (ux_dev_info.HID_Device_Type == Mouse_Device)
      {
        /* Print interface type */
        USBH_UsrLog("HID Mouse interface Found");
      }
      else if (ux_dev_info.HID_Device_Type == Keyboard_Device)
      {
        /* Print interface type */
        USBH_UsrLog("Keyboard Device Found");
      }
    }
    else
    {
      /* Print if Device is Unplugged */
      USBH_UsrLog("USB Device Unplugged");

      /* Set the application state to Idle */
      ux_app_state = App_Idle;

      /* Send the event flag to reset the app state to Idle */
      tx_event_flags_set(&ux_app_EventFlag, NEW_RECEIVED_DATA, TX_NO_WAIT);
    }
  }
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
UINT ux_host_event_callback(ULONG event, UX_HOST_CLASS *Current_class,
                            VOID *Current_instance)
{
  UINT status_hid, status_cdcacm;
  UX_HOST_CLASS *hid_class;
  UX_HOST_CLASS *cdc_acm_class;
  UX_DEVICE *current_device;

  switch (event)
  {
    case UX_DEVICE_INSERTION :

      /* Get HID Class */
      status_hid = ux_host_stack_class_get(_ux_system_host_class_hid_name,
                                           &hid_class);

      /* Get CDC_ACM Class */
      status_cdcacm = ux_host_stack_class_get(_ux_system_host_class_cdc_acm_name,
                                              &cdc_acm_class);

      /* Check the status for cdc acm and hid class */
      if ((status_hid == UX_SUCCESS) && (status_cdcacm == UX_SUCCESS))
      {
        if ((memcmp(hid_class->ux_host_class_name, Current_class->ux_host_class_name, UX_MAX_CLASS_NAME_LENGTH) == 0) && (hid == NULL))
        {
          /* Update the dev_state */
          ux_dev_info.Dev_state = Device_connected;

          /* Get current Hid Instance */
          hid = Current_instance;

          /* Get the HID Client */
          hid_client = hid ->ux_host_class_hid_client;

          /* Check if the Class different from the actual instance */
          if (hid->ux_host_class_hid_client->ux_host_class_hid_client_status !=
             (ULONG) UX_HOST_CLASS_INSTANCE_LIVE)
          {
            ux_dev_info.HID_Device_Type = Unknown_Device;
          }
          /* Check the HID_client if this is a HID mouse device. */
          if (ux_utility_memory_compare(hid_client -> ux_host_class_hid_client_name,
                                        _ux_system_host_class_hid_client_mouse_name,
                                        ux_utility_string_length_get(_ux_system_host_class_hid_client_mouse_name)) == UX_SUCCESS)
          {

            /* Get the current HID device struct */
            current_device = hid->ux_host_class_hid_device;

            mouse = hid_client-> ux_host_class_hid_client_local_instance;

            /* Check if it's a composite class or simple class*/
            if ((current_device->ux_device_descriptor.bDeviceClass == COMPOSITE_DEVICE)
                && (flag_interface_type == 0))
            {
              flag_interface_type = 1;

              /* Set the device_Type to Composite_HID_CDC_ACM*/
              ux_dev_info.COMPO_Device_Type = Composite_HID_CDC_ACM;

              /* Print interface type */
              USBH_UsrLog("Composite Interface initialized");

              /* Print Device information such as PID and VID */
              USBH_UsrLog("PID: %#x ", (UINT)mouse ->ux_host_class_hid_mouse_hid->ux_host_class_hid_device->ux_device_descriptor.idProduct);
              USBH_UsrLog("VID: %#x ", (UINT)mouse ->ux_host_class_hid_mouse_hid->ux_host_class_hid_device->ux_device_descriptor.idVendor);
            }
            else if (current_device->ux_device_descriptor.bDeviceClass == 0)
            {
              /* Print interface type */
              USBH_UsrLog("Simple Interface initialized");

              /* Print Device information such as PID and VID */
              USBH_UsrLog("PID: %#x ", (UINT)mouse ->ux_host_class_hid_mouse_hid->ux_host_class_hid_device->ux_device_descriptor.idProduct);
              USBH_UsrLog("VID: %#x ", (UINT)mouse ->ux_host_class_hid_mouse_hid->ux_host_class_hid_device->ux_device_descriptor.idVendor);
            }
            /* update HID device Type */
            ux_dev_info.HID_Device_Type = Mouse_Device;

            /* Send the updated struct ux_dev_info in the message queue */
            tx_queue_send(&ux_app_MsgQueue, &ux_dev_info, TX_NO_WAIT);
          }

          /* Check the HID_client if this is a HID keyboard device. */
          else if (ux_utility_memory_compare(hid_client ->ux_host_class_hid_client_name,
                                             _ux_system_host_class_hid_client_keyboard_name,
                                             ux_utility_string_length_get(_ux_system_host_class_hid_client_keyboard_name)) == UX_SUCCESS)
          {
            /* Get the current HID device struct */
            current_device = hid->ux_host_class_hid_device;

            keyboard = hid_client-> ux_host_class_hid_client_local_instance;

            /* Check if it's a composite class or simple class*/
            if ((current_device->ux_device_descriptor.bDeviceClass == COMPOSITE_DEVICE) && (flag_interface_type == 0))
            {
              flag_interface_type = 1;

              /* Set the device_Type to Composite_HID_CDC_ACM*/
              ux_dev_info.COMPO_Device_Type = Composite_HID_CDC_ACM;

              /* Print interface type */
              USBH_UsrLog("Composite Interface initialized");

              /* Print Device information such as PID and VID */
              USBH_UsrLog("PID: %#x ", (UINT)keyboard ->ux_host_class_hid_keyboard_hid->ux_host_class_hid_device->ux_device_descriptor.idProduct);
              USBH_UsrLog("VID: %#x ", (UINT)keyboard ->ux_host_class_hid_keyboard_hid->ux_host_class_hid_device->ux_device_descriptor.idVendor);
            }
            else if (current_device->ux_device_descriptor.bDeviceClass == 0)
            {
              /* Print interface type */
              USBH_UsrLog("Simple Interface initialized");

              /* Print Device information such as PID and VID */
              USBH_UsrLog("PID: %#x ", (UINT)keyboard ->ux_host_class_hid_keyboard_hid->ux_host_class_hid_device->ux_device_descriptor.idProduct);
              USBH_UsrLog("VID: %#x ", (UINT)keyboard ->ux_host_class_hid_keyboard_hid->ux_host_class_hid_device->ux_device_descriptor.idVendor);

            }
            /* Update HID device Type */
            ux_dev_info.HID_Device_Type = Keyboard_Device;

            /* Send the updated struct ux_dev_info in the message queue */
            tx_queue_send(&ux_app_MsgQueue, &ux_dev_info, TX_NO_WAIT);
          }
          else
          {
            /* No class found */
            USBH_ErrLog("NO Class found");
          }
        }
        else if (memcmp(cdc_acm_class->ux_host_class_name, Current_class->ux_host_class_name, UX_MAX_CLASS_NAME_LENGTH) == 0)
        {
          /* Device connection */
          ux_dev_info.Dev_state = Device_connected;
          app_cdc_acm = Current_instance;

          /* Check if this is a data interface */
          if (app_cdc_acm ->ux_host_class_cdc_acm_interface ->ux_interface_descriptor.bInterfaceClass == UX_HOST_CLASS_CDC_DATA_CLASS)
          {
            /* Get the current CDC device struct */
            current_device = app_cdc_acm->ux_host_class_cdc_acm_device;

            /* Check if it's a composite class or simple class*/
            if ((current_device->ux_device_descriptor.bDeviceClass == COMPOSITE_DEVICE)
                && (flag_interface_type == 0))
            {
              flag_interface_type = 1;

              /* Set the device_Type to Composite_HID_CDC_ACM*/
              ux_dev_info.COMPO_Device_Type = Composite_HID_CDC_ACM;

              /* Print interface type */
              USBH_UsrLog("Composite Interface initialized");

              /* Print Device information such as PID and VID */
              USBH_UsrLog("PID: %#x ", (UINT)app_cdc_acm ->ux_host_class_cdc_acm_device ->ux_device_descriptor.idProduct);
              USBH_UsrLog("VID: %#x ", (UINT)app_cdc_acm ->ux_host_class_cdc_acm_device ->ux_device_descriptor.idVendor);
            }
            else if (current_device->ux_device_descriptor.bDeviceClass == 2)
            {
              /* Print interface type */
              USBH_UsrLog("Simple Interface initialized");

              /* Print Device information such as PID and VID */
              USBH_UsrLog("PID: %#x ", (UINT)app_cdc_acm ->ux_host_class_cdc_acm_device ->ux_device_descriptor.idProduct);
              USBH_UsrLog("VID: %#x ", (UINT)app_cdc_acm ->ux_host_class_cdc_acm_device ->ux_device_descriptor.idVendor);
            }

            /* Update the CDC_ACM_Device_Type */
            ux_dev_info.CDC_Device_Type = CDC_ACM_Device;

            /* Send the updated struct ux_dev_info in the message queue */
            tx_queue_send(&ux_app_MsgQueue, &ux_dev_info, TX_NO_WAIT);
          }
        }
        else if ((memcmp(cdc_acm_class->ux_host_class_name, Current_class->ux_host_class_name, UX_MAX_CLASS_NAME_LENGTH) != 0 )
        		&& (memcmp(hid_class->ux_host_class_name, Current_class->ux_host_class_name, UX_MAX_CLASS_NAME_LENGTH) != 0))
        {
          /* Set the device type to Unsupported_Device */
          ux_dev_info.HID_Device_Type = Unsupported_Device;
          ux_dev_info.CDC_Device_Type = Unsupported_Device;
          ux_dev_info.COMPO_Device_Type = Unsupported_Device;

          /* Set the dev_state to connected */
          ux_dev_info.Dev_state = Device_connected;

          tx_queue_send(&ux_app_MsgQueue, &ux_dev_info, TX_NO_WAIT);

          /* No class found */
          USBH_ErrLog("NO Class found");
        }
      }

      break;

    case UX_DEVICE_REMOVAL :
      /* Check if HID Interface is unplugged */
      if (ux_dev_info.HID_Device_Type == Mouse_Device)
      {
        /* Set the interface type to Unknown_Device  */
        ux_dev_info.HID_Device_Type = Unknown_Device;
        /* Print interface Status */
        USBH_UsrLog("Unplugged HID Interface");
      }
      /* Check if HID Interface is unplugged */
      else if (ux_dev_info.HID_Device_Type == Keyboard_Device)
      {
        /* Set the interface type to Unknown_Device  */
        ux_dev_info.HID_Device_Type = Unknown_Device;
        /* Print interface Status */
        USBH_UsrLog("Unplugged HID Interface");
      }
      /* Check if CDC_ACM_Device Interface is unplugged */
      else if (ux_dev_info.CDC_Device_Type == CDC_ACM_Device)
      {
        /* Set the interface type to Unknown_Device  */
        ux_dev_info.CDC_Device_Type = Unknown_Device;
        /* Print interface Status */
        USBH_UsrLog("Unplugged CDC Interface");
      }
      /* Check if Composite_HID_CDC_ACM Interface is unplugged */
      else if (ux_dev_info.COMPO_Device_Type == Composite_HID_CDC_ACM)
      {
        /* Set the interface type to Unknown_Device  */
        ux_dev_info.COMPO_Device_Type = Unknown_Device;
        /* Print interface Status */
        USBH_UsrLog("Unplugged Control Interface");
      }

      /* Free Instance */
      app_cdc_acm = NULL;
      hid = NULL;
      /* clear hid_client instance */
      mouse = NULL;
      keyboard = NULL;

      /* Reset interface flag */
      flag_interface_type = 0;

      /* Set the device state to disconnected */
      if ((ux_dev_info.Dev_state == Device_connected) &&
          (ux_dev_info.COMPO_Device_Type != Composite_HID_CDC_ACM) &&
          (ux_dev_info.CDC_Device_Type != CDC_ACM_Device) &&
          ((ux_dev_info.HID_Device_Type != Mouse_Device) ||
          (ux_dev_info.HID_Device_Type != Keyboard_Device)))
      {
        /* Update the Dev_state params*/
        ux_dev_info.Dev_state   = Device_disconnected;

        /* Send the updated struct ux_dev_info in the message queue */
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

      ux_dev_info.HID_Device_Type = Unknown_Device;
      ux_dev_info.CDC_Device_Type = Unknown_Device;
      ux_dev_info.COMPO_Device_Type = Unknown_Device;
      ux_dev_info.Dev_state   = Device_connected;
      tx_queue_send(&ux_app_MsgQueue, &ux_dev_info, TX_NO_WAIT);
      break;

    case  UX_NO_DEVICE_CONNECTED :
      USBH_UsrLog("USB Device disconnected");
      break;

    default:
      break;
  }
}

/**
  * @brief MX_USB_Host_Init
  *        Initialization of USB device.
  * Init USB Host Library, add supported class and start the library
  * @retval None
  */
static UINT MX_USB_Host_Init(void)
{
  UINT ret = UX_SUCCESS;
  /* USER CODE BEGIN USB_Host_Init_PreTreatment_0 */
  /* USER CODE END USB_Host_Init_PreTreatment_0 */

  /* The code below is required for installing the host portion of USBX. */
  if (ux_host_stack_initialize(ux_host_event_callback) != UX_SUCCESS)
  {
    ret = UX_ERROR;
  }

  /* Register hid class. */
  if (ux_host_stack_class_register(_ux_system_host_class_hid_name,
                                   _ux_host_class_hid_entry) != UX_SUCCESS)
  {
    ret = UX_ERROR;
  }

  /* Register HID Mouse client */
  if (ux_host_class_hid_client_register(_ux_system_host_class_hid_client_mouse_name,
                                        ux_host_class_hid_mouse_entry) != UX_SUCCESS)
  {
    ret = UX_ERROR;
  }

  /* Register HID Mouse client */
  if (ux_host_class_hid_client_register(_ux_system_host_class_hid_client_keyboard_name,
                                        ux_host_class_hid_keyboard_entry) != UX_SUCCESS)
  {
    ret = UX_ERROR;
  }

  /* Register cdc_acm class. */
  if ((ux_host_stack_class_register(_ux_system_host_class_cdc_acm_name,
                                    _ux_host_class_cdc_acm_entry)) != UX_SUCCESS)
  {
    ret = UX_ERROR;
  }

  /* Initialize the LL driver */
  MX_USB_OTG_HS_HCD_Init();

  /* Register all the USB host controllers available in this system.  */
  if (ux_host_stack_hcd_register(_ux_system_host_hcd_stm32_name,
                                 _ux_hcd_stm32_initialize, USB_OTG_HS_PERIPH_BASE,
                                 (ULONG)&hhcd_USB_OTG_HS) != UX_SUCCESS)
  {
    ret = UX_ERROR;
  }

  /* Drive vbus */
  USBH_DriverVBUS(USB_VBUS_TRUE);

  /* Enable USB Global Interrupt*/
  HAL_HCD_Start(&hhcd_USB_OTG_HS);

  /* USER CODE BEGIN USB_Host_Init_PreTreatment_1 */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
  /* USER CODE END USB_Host_Init_PreTreatment_1 */

  /* USER CODE BEGIN USB_Host_Init_PostTreatment */
  /* USER CODE END USB_Host_Init_PostTreatment */
  return ret;
}

/**
* @brief  Drive VBUS.
* @param  state : VBUS state
*          This parameter can be one of the these values:
*           1 : VBUS Active
*           0 : VBUS Inactive
* @retval Status
*/
static void USBH_DriverVBUS(uint8_t state)
{
  /* USER CODE BEGIN 0 */

  /* USER CODE END 0*/

  if (state == USB_VBUS_TRUE)
  {
    /* Drive high Charge pump */
    /* Add IOE driver control */
    /* USER CODE BEGIN DRIVE_HIGH_CHARGE_FOR_HS */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
    /* USER CODE END DRIVE_HIGH_CHARGE_FOR_HS */
  }
  else
  {
    /* Drive low Charge pump */
    /* Add IOE driver control */
    /* USER CODE BEGIN DRIVE_LOW_CHARGE_FOR_HS */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
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
  if (GPIO_Pin == BUTTON_USER_PIN)
  {
    /* Set NEW_DATA_TO_SEND flag */
    if (tx_event_flags_set(&ux_app_EventFlag, NEW_DATA_TO_SEND, TX_OR) != TX_SUCCESS)
    {
      Error_Handler();
    }
  }
}
/* USER CODE END 1 */
