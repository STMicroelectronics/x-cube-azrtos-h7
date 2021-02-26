/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_host_mouse.c
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

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern ux_app_devInfotypeDef        ux_dev_info;
extern UX_HOST_CLASS_HID_MOUSE      *mouse;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
void  hid_mouse_thread_entry(ULONG arg);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

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
    if ((ux_dev_info.Device_Type == Mouse_Device) && (ux_dev_info.Dev_state == Device_connected))
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

/* USER CODE END 1 */
