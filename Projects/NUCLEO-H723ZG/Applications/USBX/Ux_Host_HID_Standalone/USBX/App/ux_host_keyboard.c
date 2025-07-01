/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_host_keyboard.c
  * @author  MCD Application Team
  * @brief   USBX Host HID Keyboard applicative source file
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

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/* Includes ------------------------------------------------------------------*/
#include "ux_host_keyboard.h"

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
extern UX_HOST_CLASS_HID_KEYBOARD *keyboard;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* USER CODE BEGIN 2 */

/**
  * @brief  USBX_HOST_HID_KEYBORAD_Task
  * @param  none
  * @retval none
  */
VOID USBX_HOST_HID_KEYBORAD_Task(VOID)
{
  ULONG keyboard_key;
  ULONG keyboard_state;

  /* Start if the hid client is a keyboard and connected */
  if ((keyboard != NULL) &&
      (keyboard->ux_host_class_hid_keyboard_state == (ULONG) UX_HOST_CLASS_INSTANCE_LIVE))
  {
    /* Get a key and state from the keyboard */
    if (ux_host_class_hid_keyboard_key_get(keyboard, &keyboard_key, &keyboard_state) == UX_SUCCESS)
    {
      /* Print the key pressed */
      USBH_UsrLog("%c", (CHAR)keyboard_key);
    }
  }
}

/* USER CODE END 2 */

