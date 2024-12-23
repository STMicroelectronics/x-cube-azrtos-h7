/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_rndis.c
  * @author  MCD Application Team
  * @brief   USBX Device RNDIS applicative source file
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
#include "ux_device_rndis.h"
#include <stdlib.h>
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  USBD_RNDIS_Activate
  *         This function is called when insertion of a RNDIS device.
  * @param  rndis_instance: Pointer to the rndis class instance.
  * @retval none
  */
VOID USBD_RNDIS_Activate(VOID *rndis_instance)
{
  /* USER CODE BEGIN USBD_RNDIS_Activate */
  UX_PARAMETER_NOT_USED(rndis_instance);
  /* USER CODE END USBD_RNDIS_Activate */

  return;
}

/**
  * @brief  USBD_RNDIS_Deactivate
  *         This function is called when extraction of a RNDIS device.
  * @param  rndis_instance: Pointer to the rndis class instance.
  * @retval none
  */
VOID USBD_RNDIS_Deactivate(VOID *rndis_instance)
{
  /* USER CODE BEGIN USBD_RNDIS_Deactivate */
  UX_PARAMETER_NOT_USED(rndis_instance);
  /* USER CODE END USBD_RNDIS_Deactivate */

  return;
}

/**
  * @brief  USBD_RNDIS_GetMacAdd
  *         This function is called to get rndis node id from MAC Address.
  * @param  mac_add: Pointer string to MAC address.
  * @param  mac_node: Pointer to mac node buffer.
  * @retval none
  */
VOID USBD_RNDIS_GetMacAdd(UCHAR *mac_add, UCHAR *mac_node)
{
  /* USER CODE BEGIN USBD_RNDIS_GetMacAdd */
  CHAR dummy[2] = {0};
  UCHAR i = 0, j = 0, k = 0;
  UCHAR num = 0;

  for(k = 0; k < 6; k++)
  {
    for(i = 0; i < 2; i++)
    {
      dummy[i] = mac_add[i+j];
    }

    num = strtol(dummy, NULL, 16);

    j=j+2;

    mac_node[k] = num;
  }
  /* USER CODE END USBD_RNDIS_GetMacAdd */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
