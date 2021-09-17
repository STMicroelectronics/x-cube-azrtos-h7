/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_mouse.c
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

#define CURSOR_STEP     10

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile uint8_t User_Button_State = 0;
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
    /* Check if the device state already configured */
    if (device->ux_slave_device_state == UX_DEVICE_CONFIGURED)
    {
      /* Get the interface */
      interface = device->ux_slave_device_first_interface;
      /* Get the instance */
      hid = interface->ux_slave_interface_class_instance;

      /* sleep for 10ms */
      tx_thread_sleep(0.01 * TX_TIMER_TICKS_PER_SECOND);

      /* Check if user button is pressed */
      if (User_Button_State)
      {
        /* Get the new position */
        GetPointerData(&hid_event);

        /* Send an event to the hid */
        ux_device_class_hid_event_set(hid, &hid_event);

        /* Reset User Button state */
        User_Button_State =0;
      }
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
  static int8_t x = 0, y = 0;

  /* Reset counter */
  if (cnt == 16)
  {
    cnt = 0;
  }
  /* Update pointer position x and y */
  if ((cnt >= 0) && (cnt < 4))
  {
    y=0;
    x = CURSOR_STEP;
  }
  else if ((cnt >= 4) && (cnt < 8))
  {
    x =0;
    y = CURSOR_STEP;
  }
  else if ((cnt >= 8) && (cnt < 12))
  {
    y=0;
    x = -CURSOR_STEP;
  }
  else
  {
    x=0;
    y = -CURSOR_STEP;
  }
  /* Increment counter */
  cnt++;

  /* Mouse event. Length is fixed to . */
  hid_event->ux_device_class_hid_event_length = 3;

  /* Set X position. */
  hid_event->ux_device_class_hid_event_buffer[0] = x;

  /* Set Y position. */
  hid_event->ux_device_class_hid_event_buffer[1] = y;

  hid_event->ux_device_class_hid_event_buffer[2] = 0;
}

/**
  * @brief  EXTI line detection callback.
  * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /* Read User Button state */
  User_Button_State = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_GPIO_EXTI_Callback could be implemented in the user file
   */
}
/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
