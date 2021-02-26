/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_host_keyboard.c
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
extern ux_app_devInfotypeDef          ux_dev_info;
extern UX_HOST_CLASS_HID_KEYBOARD     *keyboard;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
void  hid_keyboard_thread_entry(ULONG arg);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

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
    if ((ux_dev_info.HID_Type == Keyboard_Device) && (ux_dev_info.Dev_state == Device_connected))
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

/* USER CODE END 1 */
