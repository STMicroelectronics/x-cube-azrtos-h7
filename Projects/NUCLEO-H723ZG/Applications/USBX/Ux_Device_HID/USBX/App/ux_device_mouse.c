/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_mouse.c
  * @author  MCD Application Team
  * @brief   USBX Device applicative file
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
#include "ux_device_mouse.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "tx_api.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define CURSOR_STEP     5

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void GetPointerData(UX_SLAVE_CLASS_HID_EVENT *hid_event);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
UINT app_usbx_device_thread_hid_callback(UX_SLAVE_CLASS_HID *hid,
                                         UX_SLAVE_CLASS_HID_EVENT *hid_event)
{
  return 0U;
}
/**
  * @brief  Function implementing usbx_hid_thread_entry.
  * @param arg: Not used
  * @retval None
  */
void usbx_hid_thread_entry(ULONG arg)
{
  UX_SLAVE_DEVICE    *device;
  UX_SLAVE_INTERFACE *interface;
  UX_SLAVE_CLASS_HID *hid;
  UX_SLAVE_CLASS_HID_EVENT hid_event;

  device = &_ux_system_slave->ux_system_slave_device;
  ux_utility_memory_set(&hid_event, 0, sizeof(UX_SLAVE_CLASS_HID_EVENT));

  while (1)
  {
    if (device->ux_slave_device_state == UX_DEVICE_CONFIGURED)
    {
      interface = device->ux_slave_device_first_interface;
      hid = interface->ux_slave_interface_class_instance;

      /* sleep for 10ms */
      tx_thread_sleep(0.01 * TX_TIMER_TICKS_PER_SECOND);

      GetPointerData(&hid_event);
      ux_device_class_hid_event_set(hid, &hid_event);
    }
    else
    {
      /* sleep for 10ms */
      tx_thread_sleep(0.01 * TX_TIMER_TICKS_PER_SECOND);
    }
  }
}

/**
  * @brief  Gets Pointer Data.
  * @param  pbuf: Pointer to report
  * @retval None
  */
static void GetPointerData(UX_SLAVE_CLASS_HID_EVENT *hid_event)
{
  static int8_t cnt = 0;
  int8_t  x = 0, y = 0;

  if (cnt++ > 0)
  {
    x = CURSOR_STEP;
  }
  else
  {
    x = -CURSOR_STEP;
  }

  /* Mouse event. Length is fixed to . */
  hid_event->ux_device_class_hid_event_length = 3;

  /* Set X position. */
  hid_event->ux_device_class_hid_event_buffer[0] = x;

  /* Set Y position. */
  hid_event->ux_device_class_hid_event_buffer[1] = y;

  hid_event->ux_device_class_hid_event_buffer[2] = 0;
}

/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
