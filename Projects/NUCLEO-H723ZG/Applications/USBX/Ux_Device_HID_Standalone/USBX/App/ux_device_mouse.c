/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_mouse.c
  * @author  MCD Application Team
  * @brief   USBX Device HID Mouse applicative source file
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
#include "ux_device_mouse.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CURSOR_STEP         10
#define BUTTON_DETECT_WAIT  10
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
UX_SLAVE_CLASS_HID *hid_mouse;
__IO uint8_t User_Button_State = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static VOID GetPointerData(UX_SLAVE_CLASS_HID_EVENT *hid_event);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  USBD_HID_Mouse_Activate
  *         This function is called when insertion of a HID Mouse device.
  * @param  hid_instance: Pointer to the hid class instance.
  * @retval none
  */
VOID USBD_HID_Mouse_Activate(VOID *hid_instance)
{
  /* USER CODE BEGIN USBD_HID_Mouse_Activate */

  /* Save the HID mouse instance */
  hid_mouse = (UX_SLAVE_CLASS_HID *) hid_instance;

  /* USER CODE END USBD_HID_Mouse_Activate */

  return;
}

/**
  * @brief  USBD_HID_Mouse_Deactivate
  *         This function is called when extraction of a HID Mouse device.
  * @param  hid_instance: Pointer to the hid class instance.
  * @retval none
  */
VOID USBD_HID_Mouse_Deactivate(VOID *hid_instance)
{
  /* USER CODE BEGIN USBD_HID_Mouse_Deactivate */
  UX_PARAMETER_NOT_USED(hid_instance);

  /* Reset the HID mouse instance */
  hid_mouse = UX_NULL;

  /* USER CODE END USBD_HID_Mouse_Deactivate */

  return;
}

/**
  * @brief  USBD_HID_Mouse_SetReport
  *         This function is invoked when the host sends a HID SET_REPORT
  *         to the application over Endpoint 0.
  * @param  hid_instance: Pointer to the hid class instance.
  * @param  hid_event: Pointer to structure of the hid event.
  * @retval status
  */
UINT USBD_HID_Mouse_SetReport(UX_SLAVE_CLASS_HID *hid_instance,
                              UX_SLAVE_CLASS_HID_EVENT *hid_event)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_HID_Mouse_SetReport */
  UX_PARAMETER_NOT_USED(hid_instance);
  UX_PARAMETER_NOT_USED(hid_event);
  /* USER CODE END USBD_HID_Mouse_SetReport */

  return status;
}

/**
  * @brief  USBD_HID_Mouse_GetReport
  *         This function is invoked when host is requesting event through
  *         control GET_REPORT request.
  * @param  hid_instance: Pointer to the hid class instance.
  * @param  hid_event: Pointer to structure of the hid event.
  * @retval status
  */
UINT USBD_HID_Mouse_GetReport(UX_SLAVE_CLASS_HID *hid_instance,
                              UX_SLAVE_CLASS_HID_EVENT *hid_event)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_HID_Mouse_GetReport */
  UX_PARAMETER_NOT_USED(hid_instance);
  UX_PARAMETER_NOT_USED(hid_event);
  /* USER CODE END USBD_HID_Mouse_GetReport */

  return status;
}

/* USER CODE BEGIN 1 */

/**
  * @brief  USBX_DEVICE_HID_MOUSE_Task
  *         Run HID mouse task
  * @param  none
  * @retval none
  */
VOID USBX_DEVICE_HID_MOUSE_Task(VOID)
{
  ULONG tick;
  UX_SLAVE_DEVICE *device;
  UX_SLAVE_CLASS_HID_EVENT hid_event;

  device = &_ux_system_slave->ux_system_slave_device;
  ux_utility_memory_set(&hid_event, 0, sizeof(UX_SLAVE_CLASS_HID_EVENT));

  tick = HAL_GetTick();

  /* Check if the device state already configured */
  if ((device->ux_slave_device_state == UX_DEVICE_CONFIGURED) && (hid_mouse != UX_NULL))
  {

    /* Sleep for 10ms */
    if (ux_utility_time_elapsed(tick, HAL_GetTick()) > BUTTON_DETECT_WAIT)
    {
      return;
    }

    tick = HAL_GetTick();

    /* Check if user button is pressed */
    if (User_Button_State)
    {
      /* Get the new position */
      GetPointerData(&hid_event);

      /* Send an event to the hid */
      ux_device_class_hid_event_set(hid_mouse, &hid_event);

      /* Reset User Button state */
      User_Button_State = 0U;
    }
  }
}
/**
  * @brief  GetPointerData
  *         Gets Pointer Data.
  * @param  hid_event: Pointer to structure of the hid event.
  * @retval None
  */
static VOID GetPointerData(UX_SLAVE_CLASS_HID_EVENT *hid_event)
{
  static int8_t counter = 0;
  static int8_t x = 0, y = 0;

  /* Reset counter */
  if (counter == 16)
  {
    counter = 0;
  }
  /* Update pointer position x and y */
  if ((counter >= 0) && (counter < 4))
  {
    y = 0;
    x = CURSOR_STEP;
  }
  else if ((counter >= 4) && (counter < 8))
  {
    x = 0;
    y = CURSOR_STEP;
  }
  else if ((counter >= 8) && (counter < 12))
  {
    y = 0;
    x = -CURSOR_STEP;
  }
  else
  {
    x = 0;
    y = -CURSOR_STEP;
  }

  /* Increment counter */
  counter++;

  /* Mouse event. Length is fixed to 3 */
  hid_event->ux_device_class_hid_event_length = 3;

  /* Set X position. */
  hid_event->ux_device_class_hid_event_buffer[0] = x;

  /* Set Y position. */
  hid_event->ux_device_class_hid_event_buffer[1] = y;

  hid_event->ux_device_class_hid_event_buffer[2] = 0;
}

/* USER CODE END 1 */
