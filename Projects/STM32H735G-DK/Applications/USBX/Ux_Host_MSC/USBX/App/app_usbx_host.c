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

static TX_THREAD ux_host_app_thread;

/* USER CODE BEGIN PV */
TX_THREAD                   msc_app_thread;
UX_HOST_CLASS_STORAGE       *storage;
UX_HOST_CLASS_STORAGE_MEDIA *storage_media;
FX_MEDIA                    *media;
TX_EVENT_FLAGS_GROUP        ux_app_EventFlag;
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

  /* Initialize the host storage class */
  if (ux_host_stack_class_register(_ux_system_host_class_storage_name,
                                   ux_host_class_storage_entry) != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_HOST_STORAGE_REGISTER_ERROR */
    return UX_ERROR;
    /* USER CODE END USBX_HOST_STORAGE_REGISTER_ERROR */
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

  /* Allocate the stack for storrage app thread  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
                       UX_HOST_APP_THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  /* Create the storage applicative process thread */
  if (tx_thread_create(&msc_app_thread, "MSC App thread", msc_process_thread_entry,
                       0, pointer, UX_HOST_APP_THREAD_STACK_SIZE, 30, 30, 0,
                       TX_AUTO_START) != TX_SUCCESS)
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

  /* USER CODE END ux_host_event_callback0 */

  switch (event)
  {
    case UX_DEVICE_INSERTION:

      /* USER CODE BEGIN UX_DEVICE_INSERTION */

      /* Get current Storage Class */
      if (current_class -> ux_host_class_entry_function == ux_host_class_storage_entry)
      {
        if (storage == UX_NULL)
        {
          /* Get current Storage Instance */
          storage = (UX_HOST_CLASS_STORAGE *)current_instance;

          USBH_UsrLog("\nUSB Mass Storage Device Plugged");
          USBH_UsrLog("PID: %#x ", (UINT)storage -> ux_host_class_storage_device -> ux_device_descriptor.idProduct);
          USBH_UsrLog("VID: %#x ", (UINT)storage -> ux_host_class_storage_device -> ux_device_descriptor.idVendor);

          /* Get the storage media */
          storage_media = (UX_HOST_CLASS_STORAGE_MEDIA *)current_class -> ux_host_class_media;

          if (storage_media -> ux_host_class_storage_media_lun != 0)
          {
            storage_media = UX_NULL;
          }
          else
          {
            /* Get the media file */
            media = &storage_media->ux_host_class_storage_media;
          }

          /* Check the storage class state */
          if (storage -> ux_host_class_storage_state ==  UX_HOST_CLASS_INSTANCE_LIVE)
          {
            /* Set STORAGE_MEDIA flag */
            if (tx_event_flags_set(&ux_app_EventFlag, STORAGE_MEDIA, TX_OR) != TX_SUCCESS)
            {
              Error_Handler();
            }
          }
        }
      }

      /* USER CODE END UX_DEVICE_INSERTION */

      break;

    case UX_DEVICE_REMOVAL:

      /* USER CODE BEGIN UX_DEVICE_REMOVAL */

      if ((VOID*)storage == current_instance)
      {
        /* Clear storage media instance & media file */
        storage = UX_NULL;
        storage_media = UX_NULL;
        media = UX_NULL;

        USBH_UsrLog("\nUSB Mass Storage Device Unplugged");
      }

      /* USER CODE END UX_DEVICE_REMOVAL */

      break;

#if defined (UX_HOST_CLASS_STORAGE_NO_FILEX)
    case UX_STORAGE_MEDIA_INSERTION:

      /* USER CODE BEGIN UX_STORAGE_MEDIA_INSERTION */

      /* USER CODE END UX_STORAGE_MEDIA_INSERTION */

      break;

    case UX_STORAGE_MEDIA_REMOVAL:

      /* USER CODE BEGIN UX_STORAGE_MEDIA_REMOVAL */

      /* USER CODE END UX_STORAGE_MEDIA_REMOVAL */

      break;
#endif

    case UX_DEVICE_CONNECTION:

      /* USER CODE BEGIN UX_DEVICE_CONNECTION */

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
  * @param  None
  * @retval None
  */
VOID USBX_APP_Host_Init(VOID)
{
  /* USER CODE BEGIN USB_Host_Init_PreTreatment_0 */

  /* USER CODE END USB_Host_Init_PreTreatment_0 */

  /* Initialize the LL driver */
  MX_USB_OTG_HS_HCD_Init();

  /* Register all the USB host controllers available in this system. */
  ux_host_stack_hcd_register(_ux_system_host_hcd_stm32_name,
                             _ux_hcd_stm32_initialize, USB_OTG_HS_PERIPH_BASE,
                             (ULONG)&hhcd_USB_OTG_HS);

  /* Drive vbus */
  USBH_DriverVBUS(USB_VBUS_TRUE);

  /* Enable USB Global Interrupt */
  HAL_HCD_Start(&hhcd_USB_OTG_HS);

  /* USER CODE BEGIN USB_Host_Init_PreTreatment1 */

  /* Start Application Message */
  USBH_UsrLog(" **** USB OTG HS in FS MSC Host **** \n");
  USBH_UsrLog("USB Host library started.\n");

  /* Wait for Device to be attached */
  USBH_UsrLog("Starting MSC Application");
  USBH_UsrLog("Connect your MSC Device");

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

/* USER CODE END 1 */
