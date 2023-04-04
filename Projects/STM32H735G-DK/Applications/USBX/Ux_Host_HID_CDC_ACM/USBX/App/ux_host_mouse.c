/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_host_mouse.c
  * @author  MCD Application Team
  * @brief   USBX Host HID Mouse applicative source file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
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
#include "ux_host_mouse.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_usbx_host.h"
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
extern UX_HOST_CLASS_HID_MOUSE      *mouse;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/**
  * @brief  Function implementing hid_mouse_thread_entry.
  * @param  thread_input: Not used
  * @retval none
  */
VOID hid_mouse_thread_entry(ULONG thread_input)
{
  LONG  old_Pos_x = 0;
  LONG  old_Pos_y = 0;
  LONG  actual_Pos_x = 0;
  LONG  actual_Pos_y = 0;
  ULONG actual_mouse_buttons = 0;
  ULONG old_mouse_buttons = 0;
  SLONG actual_mouse_wheel = 0;
  SLONG old_mouse_wheel = 0;

  while (1)
  {
    /* Start if the hid client is a mouse and connected */
    if ((mouse != NULL) &&
        (mouse -> ux_host_class_hid_mouse_state == (ULONG) UX_HOST_CLASS_INSTANCE_LIVE))
    {

      /* Get Mouse position */
      if (ux_host_class_hid_mouse_position_get(mouse, &actual_Pos_x, &actual_Pos_y) == UX_SUCCESS)
      {
        if (((actual_Pos_x != old_Pos_x) || (actual_Pos_y != old_Pos_y)) &&
            (actual_Pos_x != 0) && (actual_Pos_y != 0))
        {
          USBH_UsrLog("Pos_x = %ld Pos_y= %ld", actual_Pos_x, actual_Pos_y);

          /* Update (x,y) old position */
          old_Pos_x = actual_Pos_x;
          old_Pos_y = actual_Pos_y;
        }
      }
      else
      {
        tx_thread_sleep(MS_TO_TICK(10));
      }

      /* Get Mouse buttons value */
      if (ux_host_class_hid_mouse_buttons_get(mouse, &actual_mouse_buttons) == UX_SUCCESS)
      {
        if (actual_mouse_buttons != old_mouse_buttons)
        {
          /* Check which button is pressed */
          if (actual_mouse_buttons & UX_HOST_CLASS_HID_MOUSE_BUTTON_1_PRESSED)
          {
            USBH_UsrLog("Left Button Pressed");
          }

          if (actual_mouse_buttons & UX_HOST_CLASS_HID_MOUSE_BUTTON_2_PRESSED)
          {
            USBH_UsrLog("Right Button Pressed");
          }

          if (actual_mouse_buttons & UX_HOST_CLASS_HID_MOUSE_BUTTON_3_PRESSED)
          {
            USBH_UsrLog("Middle Button Pressed");
          }

          /* Update button old value */
          old_mouse_buttons = actual_mouse_buttons;
        }
      }
      else
      {
        tx_thread_sleep(MS_TO_TICK(10));
      }

      /* Get hid wheel mouse position */
      if (ux_host_class_hid_mouse_wheel_get(mouse, &actual_mouse_wheel) == UX_SUCCESS)
      {
        if ((actual_mouse_wheel != old_mouse_wheel) && (actual_mouse_wheel != 0))
        {
          USBH_UsrLog("Pos_weel = %ld", actual_mouse_wheel);

          /* Update wheel mouse movement value */
          old_mouse_wheel = actual_mouse_wheel;
        }
      }
      else
      {
        tx_thread_sleep(MS_TO_TICK(10));
      }
    }
    else
    {
      tx_thread_sleep(MS_TO_TICK(10));
    }
  }
}

/* USER CODE END 1 */
