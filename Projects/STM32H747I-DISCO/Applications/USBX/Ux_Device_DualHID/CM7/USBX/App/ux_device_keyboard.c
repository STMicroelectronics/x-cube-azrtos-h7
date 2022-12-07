/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_keyboard.c
  * @author  MCD Application Team
  * @brief   USBX Device HID Keyboard applicative source file
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

/* Includes ------------------------------------------------------------------*/
#include "ux_device_keyboard.h"

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
UX_SLAVE_CLASS_HID *hid_keyboard;

__IO uint8_t User_Button_State = 0U;
static UCHAR key_button = 0U;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static VOID GetKeyData(UX_SLAVE_CLASS_HID_EVENT *hid_event);
/* USER CODE END 0 */

/**
  * @brief  USBD_HID_Keyboard_Activate
  *         This function is called when insertion of a HID Keyboard device.
  * @param  hid_instance: Pointer to the hid class instance.
  * @retval none
  */
VOID USBD_HID_Keyboard_Activate(VOID *hid_instance)
{
  /* USER CODE BEGIN USBD_HID_Keyboard_Activate */

  /* Save the HID keyboard instance */
  hid_keyboard = (UX_SLAVE_CLASS_HID*) hid_instance;

  /* USER CODE END USBD_HID_Keyboard_Activate */

  return;
}

/**
  * @brief  USBD_HID_Keyboard_Deactivate
  *         This function is called when extraction of a HID Keyboard device.
  * @param  hid_instance: Pointer to the hid class instance.
  * @retval none
  */
VOID USBD_HID_Keyboard_Deactivate(VOID *hid_instance)
{
  /* USER CODE BEGIN USBD_HID_Keyboard_Deactivate */
  UX_PARAMETER_NOT_USED(hid_instance);

  /* Reset the HID mouse instance */
  hid_keyboard = UX_NULL;

  /* USER CODE END USBD_HID_Keyboard_Deactivate */

  return;
}

/**
  * @brief  USBD_HID_Keyboard_SetReport
  *         This function is invoked when the host sends a HID SET_REPORT
  *         to the application over Endpoint 0.
  * @param  hid_instance: Pointer to the hid class instance.
  * @param  hid_event: Pointer to structure of the hid event.
  * @retval status
  */
UINT USBD_HID_Keyboard_SetReport(UX_SLAVE_CLASS_HID *hid_instance,
                                 UX_SLAVE_CLASS_HID_EVENT *hid_event)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_HID_Keyboard_SetReport */
  UX_PARAMETER_NOT_USED(hid_instance);
  UX_PARAMETER_NOT_USED(hid_event);
  /* USER CODE END USBD_HID_Keyboard_SetReport */

  return status;
}

/**
  * @brief  USBD_HID_Keyboard_GetReport
  *         This function is invoked when host is requesting event through
  *         control GET_REPORT request.
  * @param  hid_instance: Pointer to the hid class instance.
  * @param  hid_event: Pointer to structure of the hid event.
  * @retval status
  */
UINT USBD_HID_Keyboard_GetReport(UX_SLAVE_CLASS_HID *hid_instance,
                                 UX_SLAVE_CLASS_HID_EVENT *hid_event)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_HID_Keyboard_GetReport */
  UX_PARAMETER_NOT_USED(hid_instance);
  UX_PARAMETER_NOT_USED(hid_event);
  /* USER CODE END USBD_HID_Keyboard_GetReport */

  return status;
}

/* USER CODE BEGIN 1 */

/**
  * @brief  Function implementing usbx_hid_keyboard_thread_entry.
  * @param  thread_input: not used
  * @retval none
  */
VOID usbx_hid_keyboard_thread_entry(ULONG thread_input)
{
  UX_SLAVE_DEVICE *device;
  UX_SLAVE_CLASS_HID_EVENT hid_event;

  TX_PARAMETER_NOT_USED(thread_input);

  /* Get the pointer to the device */
  device = &_ux_system_slave -> ux_system_slave_device;

  /* Reset the HID event structure */
  ux_utility_memory_set(&hid_event, 0, sizeof(UX_SLAVE_CLASS_HID_EVENT));

  while (1)
  {
    /* Check if the device state already configured */
    if ((device->ux_slave_device_state == UX_DEVICE_CONFIGURED) && (hid_keyboard != UX_NULL))
    {
      /* Sleep Thread for 20ms */
      tx_thread_sleep(MS_TO_TICK(20));

      /* Check if user button is pressed */
      if (User_Button_State)
      {
        /* Get the key button */
        GetKeyData(&hid_event);

        /* Send keyboard event */
        ux_device_class_hid_event_set(hid_keyboard, &hid_event);

        /* Next event has the key button depressed */
        hid_event.ux_device_class_hid_event_buffer[2] = 0;

        /* Set hid envent length to 8 */
        hid_event.ux_device_class_hid_event_length = 8;

        /* Send keyboard event */
        ux_device_class_hid_event_set(hid_keyboard, &hid_event);

        /* Reset User Button state */
        User_Button_State = 0;
      }
    }
    else
    {
      /* Sleep thread for 10ms */
      tx_thread_sleep(MS_TO_TICK(10));
    }
  }
}

/**
  * @brief  GetKeyData
  *         Gets Pointer Data.
  * @param  hid_event: Pointer to hid event buffer.
  * @retval none
  */
static VOID GetKeyData(UX_SLAVE_CLASS_HID_EVENT *hid_event)
{
  if (key_button == 30)
  {
    key_button = 0x04;
  }

  /* Set hid envent length to 8 */
  hid_event->ux_device_class_hid_event_length = 8;

  /* This byte is a modifier byte */
  hid_event->ux_device_class_hid_event_buffer[0] = 0;

  /* This byte is reserved */
  hid_event->ux_device_class_hid_event_buffer[1] = 0;

  /* Update key button byte */
  hid_event->ux_device_class_hid_event_buffer[2] = key_button;

  /* Increment counter */
  key_button++;
}

/* USER CODE END 1 */
