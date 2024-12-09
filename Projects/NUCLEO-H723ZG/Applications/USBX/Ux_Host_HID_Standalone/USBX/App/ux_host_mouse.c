/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_host_mouse.c
  * @author  MCD Application Team
  * @brief   USBX Host HID Mouse applicative source file
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
extern UX_HOST_CLASS_HID_MOUSE *mouse;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/**
  * @brief  USBX_HOST_HID_MOUSE_Task
  * @param  none
  * @retval none
  */
VOID USBX_HOST_HID_MOUSE_Task(VOID)
{
  static LONG old_Pos_x = 0;
  static LONG old_Pos_y = 0;
  LONG actual_Pos_x = 0;
  LONG actual_Pos_y = 0;
  ULONG actual_mouse_buttons = 0;
  static ULONG old_mouse_buttons = 0;
  SLONG actual_mouse_wheel = 0;
  static SLONG old_mouse_wheel = 0;

  /* Check if mouse instance is Null */
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
  }
}

/* USER CODE END 1 */
