/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_usbx_host.c
  * @author  MCD Application Team
  * @brief   USBX host applicative file
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
#include "app_usbx_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define USBX_APP_STACK_SIZE                          1024
#define USBX_MEMORY_SIZE                             (64 * 1024)
#define USBX_APP_BYTE_POOL_SIZE                      (5120 + (USBX_MEMORY_SIZE))

#define APP_QUEUE_SIZE                                5

/* Set usbx_pool start address to 0x24027000 */
#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma location = 0x24027000
#elif defined ( __GNUC__ ) /* GNU Compiler */
__attribute__((section(".UsbxPoolSection")))
#endif
static uint8_t usbx_pool[USBX_APP_BYTE_POOL_SIZE];
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern HCD_HandleTypeDef            hhcd_USB_OTG_HS;
TX_THREAD                           ux_app_thread;
TX_THREAD                           keyboard_app_thread;
TX_THREAD                           mouse_app_thread;
TX_BYTE_POOL                        ux_byte_pool;
TX_QUEUE                            MsgQueue;
UX_HOST_CLASS_HID                   *hid;
UX_HOST_CLASS_HID_CLIENT            *hid_client;
UX_HOST_CLASS_HID_MOUSE             *mouse;
UX_HOST_CLASS_HID_KEYBOARD          *keyboard;
UINT                                status;
Device_info                         dev_info;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
extern void MX_USB_OTG_HS_HCD_Init(void);
/* USER CODE END PFP */
/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  Application USBX Host Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_USBX_Host_Init(VOID *memory_ptr)
{
  UINT ret = UX_SUCCESS;

  /* USER CODE BEGIN  App_USBX_Host_Init */
  CHAR *pointer;

  /* Create a byte memory pool from which to allocate the thread stacks.  */
  if (tx_byte_pool_create(&ux_byte_pool, "ux byte pool 0", usbx_pool,
                          USBX_APP_BYTE_POOL_SIZE) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
  }

  /* Allocate the stack for thread 0. */
  if (tx_byte_allocate(&ux_byte_pool, (VOID **) &pointer,
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
  if (tx_byte_allocate(&ux_byte_pool, (VOID **) &pointer,
                       USBX_APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
  }

  /* Create the main App thread. */
  if (tx_thread_create(&ux_app_thread, "thread 0", usbx_app_thread_entry, 0,
                       pointer, USBX_APP_STACK_SIZE, 20, 20, 1,
                       TX_AUTO_START) != TX_SUCCESS)
  {
    ret = TX_THREAD_ERROR;
  }

  /* Allocate the stack for thread 1. */
  if (tx_byte_allocate(&ux_byte_pool, (VOID **) &pointer,
                       USBX_APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
  }

  /* Create the HID mouse App thread. */
  if (tx_thread_create(&mouse_app_thread, "thread 1", hid_mouse_thread_entry, 0,
                       pointer, USBX_APP_STACK_SIZE, 30, 30, 1,
                       TX_AUTO_START) != TX_SUCCESS)
  {
    ret = TX_THREAD_ERROR;
  }

  /* Allocate the stack for thread 2. */
  if (tx_byte_allocate(&ux_byte_pool, (VOID **) &pointer,
                       USBX_APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
  }

  /* Create the HID Keyboard App thread. */
  if (tx_thread_create(&keyboard_app_thread, "thread 1", hid_keyboard_thread_entry, 0,
                       pointer, USBX_APP_STACK_SIZE, 30, 30, 1,
                       TX_AUTO_START) != TX_SUCCESS)
  {
    ret = TX_THREAD_ERROR;
  }

  /* Allocate Memory for the Queue */
  if (tx_byte_allocate(&ux_byte_pool, (VOID **) &pointer,
                       APP_QUEUE_SIZE * sizeof(Device_info), TX_NO_WAIT) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
  }

  /* Create the MsgQueue */
  if (tx_queue_create(&MsgQueue, "Message Queue app", sizeof(Device_info),
                      pointer, APP_QUEUE_SIZE * sizeof(Device_info)) != TX_SUCCESS)
  {
    ret = TX_QUEUE_ERROR;
  }

  /* USER CODE END  App_USBX_Host_Init */

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
  USBH_UsrLog(" **** USB OTG HS in FS HID Host **** \n");
  USBH_UsrLog("USB Host library started.\n");

  /* Wait for Device to be attached */
  USBH_UsrLog("Starting HID Application");
  USBH_UsrLog("Connect your HID Device\n");

  while (1)
  {
    /* Wait for a hid device to be connected */
    status = tx_queue_receive(&MsgQueue, &dev_info, TX_WAIT_FOREVER);

    if (dev_info.Dev_state == Device_connected)
    {
      switch (dev_info.Device_Type)
      {
        case Mouse_Device :
          mouse = hid_client-> ux_host_class_hid_client_local_instance;
          USBH_UsrLog("HID_Mouse_Device");
          USBH_UsrLog("PID: %#x ", (UINT)mouse ->ux_host_class_hid_mouse_hid->ux_host_class_hid_device->ux_device_descriptor.idProduct);
          USBH_UsrLog("VID: %#x ", (UINT)mouse ->ux_host_class_hid_mouse_hid->ux_host_class_hid_device->ux_device_descriptor.idVendor);
          USBH_UsrLog("USB HID Host Mouse App...");
          USBH_UsrLog("Mouse is ready...\n");
          break;

        case Keyboard_Device :
          keyboard = hid_client-> ux_host_class_hid_client_local_instance;
          USBH_UsrLog("HID_Keyboard_Device");
          USBH_UsrLog("PID: %#x ", (UINT)keyboard ->ux_host_class_hid_keyboard_hid->ux_host_class_hid_device->ux_device_descriptor.idProduct);
          USBH_UsrLog("VID: %#x ", (UINT)keyboard ->ux_host_class_hid_keyboard_hid->ux_host_class_hid_device->ux_device_descriptor.idVendor);
          USBH_UsrLog("USB HID Host Keyboard App...");
          USBH_UsrLog("keyboard is ready...\n");
          break;

        case Unknown_Device :
          USBH_ErrLog("Unsupported USB device");
          break;

        default :
          break;
      }
    }
    else
    {
      /* clear hid_client local instance */
      mouse = NULL;
      keyboard = NULL;
    }
  }
}

/**
  * @brief  hid_mouse_thread_entry .
  * @param  ULONG arg
  * @retval Void
  */
void  hid_mouse_thread_entry(ULONG arg)
{
  UINT status;
  ULONG value;
  LONG  old_Pos_x = 0;
  LONG  old_Pos_y = 0;
  LONG  Pos_x = 0;
  LONG  Pos_y = 0;
  SLONG mouse_wheel_movement = 0;
  SLONG old_mouse_wheel_movement = 0;

  while (1)
  {
    /* start if the hid client is a mouse and connected */
    if ((dev_info.Device_Type == Mouse_Device) && (dev_info.Dev_state == Device_connected))
    {
      Pos_x = mouse ->ux_host_class_hid_mouse_x_position;
      Pos_y = mouse ->ux_host_class_hid_mouse_y_position;

      /* get Mouse position */
      status = ux_host_class_hid_mouse_position_get(mouse, &Pos_x, &Pos_y);

      if (status == UX_SUCCESS)
      {
        if ((Pos_x != old_Pos_x) || (Pos_y != old_Pos_y))
        {
          USBH_UsrLog("Pos_x = %ld Pos_y= %ld", Pos_x, Pos_y);

          /* update (x,y)old position */
          old_Pos_x = Pos_x;
          old_Pos_y = Pos_y;
        }

        /* get Mouse buttons value */
        value = mouse->ux_host_class_hid_mouse_buttons;
        status = _ux_host_class_hid_mouse_buttons_get(mouse, &value);

        if (status == UX_SUCCESS)
        {
          /* check which button is pressed */
          if (value & UX_HOST_CLASS_HID_MOUSE_BUTTON_1_PRESSED)
          {
            USBH_UsrLog("Left Button Pressed");
          }

          if (value & UX_HOST_CLASS_HID_MOUSE_BUTTON_2_PRESSED)
          {
            USBH_UsrLog("Right Button Pressed");
          }

          if (value & UX_HOST_CLASS_HID_MOUSE_BUTTON_3_PRESSED)
          {
            USBH_UsrLog("Middle Button Pressed");
          }

          /* get hid wheel mouse position */
          mouse_wheel_movement = mouse-> ux_host_class_hid_mouse_wheel;
          status = _ux_host_class_hid_mouse_wheel_get(mouse, &mouse_wheel_movement);

          if (status == UX_SUCCESS)
          {
            if (mouse_wheel_movement != old_mouse_wheel_movement)
            {
              USBH_UsrLog("Pos_weel = %ld", mouse_wheel_movement);

              /* update wheel mouse movement value */
              old_mouse_wheel_movement = mouse_wheel_movement;
            }
          }

          /* Re-initialize mouse buttons Value */
          value = 0x0U;
          tx_thread_sleep(10);
        }
      }
    }
    else
    {
      tx_thread_sleep(10);
    }
  }
}


/**
  * @brief  hid_keyboard_thread_entry .
  * @param  ULONG arg
  * @retval Void
  */
void  hid_keyboard_thread_entry(ULONG arg)
{
  UINT status;
  ULONG keyboard_key;
  ULONG keyboard_state;
  while (1)
  {
    /* start if the hid client is a keyboard and connected */
    if ((dev_info.Device_Type == Keyboard_Device) && (dev_info.Dev_state == Device_connected))
    {
      /* get the keyboard key pressed */
      status = _ux_host_class_hid_keyboard_key_get(keyboard, &keyboard_key, &keyboard_state);

      if (status == UX_SUCCESS)
      {
        /* print the key pressed */
        USBH_UsrLog("%c", (CHAR)keyboard_key);
      }
    }
    else
    {
      tx_thread_sleep(10);
    }
  }
}

/**
* @brief ux_host_event_callback
* @param ULONG event
           This parameter can be one of the these values:
             1 : UX_DEVICE_INSERTION
             2 : UX_DEVICE_REMOVAL
             3 : UX_HID_CLIENT_INSERTION
             4 : UX_HID_CLIENT_REMOVAL
         UX_HOST_CLASS * Current_class
         VOID * Current_instance
* @retval Status
*/
UINT ux_host_event_callback(ULONG event, UX_HOST_CLASS *Current_class, VOID *Current_instance)
{
  UINT status;
  UX_HOST_CLASS *hid_class;

  switch (event)
  {
    case UX_DEVICE_INSERTION :
      /* Get current Hid Class */
      status = ux_host_stack_class_get(_ux_system_host_class_hid_name, &hid_class);

      if (status == UX_SUCCESS)
      {
        if ((hid_class == Current_class) && (hid == NULL))
        {
          /* Get current Hid Instance */
          hid = Current_instance;
          /* Get the HID Client */
          hid_client = hid ->ux_host_class_hid_client;

          if (hid->ux_host_class_hid_client->ux_host_class_hid_client_status != (ULONG) UX_HOST_CLASS_INSTANCE_LIVE)
          {
            dev_info.Device_Type = Unknown_Device;
          }
          /* Check the HID_client if this is a HID mouse device. */
          if (ux_utility_memory_compare(hid_client -> ux_host_class_hid_client_name,
                                        _ux_system_host_class_hid_client_mouse_name,
                                        ux_utility_string_length_get(_ux_system_host_class_hid_client_mouse_name)) == UX_SUCCESS)
          {
            /* update HID device Type */
            dev_info.Device_Type = Mouse_Device;

            /* put a message queue to usbx_app_thread_entry */
            tx_queue_send(&MsgQueue, &dev_info, TX_NO_WAIT);
          }

          /* Check the HID_client if this is a HID keyboard device. */
          else if (ux_utility_memory_compare(hid_client -> ux_host_class_hid_client_name,
                                             _ux_system_host_class_hid_client_keyboard_name,
                                             ux_utility_string_length_get(_ux_system_host_class_hid_client_keyboard_name)) == UX_SUCCESS)
          {
            /* update HID device Type */
            dev_info.Device_Type = Keyboard_Device;

            /* put a message queue to usbx_app_thread_entry */
            tx_queue_send(&MsgQueue, &dev_info, TX_NO_WAIT);
          }
          else
          {
            dev_info.Device_Type = Unknown_Device;
            dev_info.Dev_state = Device_connected;
            tx_queue_send(&MsgQueue, &dev_info, TX_NO_WAIT);
          }
        }
      }
      else
      {
        /* No HID class found */
        USBH_ErrLog("NO HID Class found");
      }
      break;

    case UX_DEVICE_REMOVAL :

      if (Current_instance == hid)
      {
        /* Free Instance */
        hid = NULL;
        USBH_UsrLog("USB Device Unplugged");
        dev_info.Dev_state   = No_Device;
        dev_info.Device_Type = Unknown_Device;
      }
      break;

    case UX_HID_CLIENT_INSERTION :
      USBH_UsrLog("HID Client Plugged");
      dev_info.Dev_state = Device_connected;
      break;

    case UX_HID_CLIENT_REMOVAL:
      USBH_UsrLog("HID Client Unplugged");
      dev_info.Dev_state   =  Device_disconnected;
      dev_info.Device_Type =  Unknown_Device;
      tx_queue_send(&MsgQueue, &dev_info, TX_NO_WAIT);

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

      dev_info.Device_Type = Unknown_Device;
      dev_info.Dev_state   = Device_connected;
      tx_queue_send(&MsgQueue, &dev_info, TX_NO_WAIT);
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
UINT MX_USB_Host_Init(void)
{
  UINT ret = UX_SUCCESS;
  /* USER CODE BEGIN USB_Host_Init_PreTreatment_0 */
  /* USER CODE END USB_Host_Init_PreTreatment_0 */

  /* The code below is required for installing the host portion of USBX. */
  if (ux_host_stack_initialize(ux_host_event_callback) != UX_SUCCESS)
  {
    status = UX_ERROR;
  }

  /* Register hid class. */
  if (ux_host_stack_class_register(_ux_system_host_class_hid_name,
                                   _ux_host_class_hid_entry) != UX_SUCCESS)
  {
    status = UX_ERROR;
  }

  /* Register HID Mouse client */
  if (ux_host_class_hid_client_register(_ux_system_host_class_hid_client_mouse_name,
                                        ux_host_class_hid_mouse_entry) != UX_SUCCESS)
  {
    status = UX_ERROR;
  }

  /* Register HID Mouse client */
  if (ux_host_class_hid_client_register(_ux_system_host_class_hid_client_keyboard_name,
                                        ux_host_class_hid_keyboard_entry) != UX_SUCCESS)
  {
    status = UX_ERROR;
  }

  /* Initialize the LL driver */
  MX_USB_OTG_HS_HCD_Init();

  /* Register all the USB host controllers available in this system.  */
  if (ux_host_stack_hcd_register(_ux_system_host_hcd_stm32_name,
                                 _ux_hcd_stm32_initialize, USB_OTG_HS_PERIPH_BASE,
                                 (ULONG)&hhcd_USB_OTG_HS) != UX_SUCCESS)
  {
    status = UX_ERROR;
  }

  /* Drive vbus */
  USBH_DriverVBUS(USB_VBUS_TRUE);

  /* Enable USB Global Interrupt*/
  HAL_HCD_Start(&hhcd_USB_OTG_HS);

  /* USER CODE BEGIN USB_Host_Init_PreTreatment_1 */
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
/* USER CODE END 1 */
