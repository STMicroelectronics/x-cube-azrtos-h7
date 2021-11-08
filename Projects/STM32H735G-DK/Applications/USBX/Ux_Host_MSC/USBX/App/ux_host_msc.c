/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_host_msc.c
  * @author  MCD Application Team
  * @brief   USBX host applicative file
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
#include "app_usbx_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_filex.h"
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
extern UX_HOST_CLASS_STORAGE        *storage;
extern FX_MEDIA                     *media;
extern TX_QUEUE                     ux_app_MsgQueue_msc;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
void  msc_process_thread_entry(ULONG arg);
void  Error_Handler(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/**
  * @brief  msc_process_thread_entry .
  * @param  ULONG arg
  * @retval Void
  */
void  msc_process_thread_entry(ULONG arg)
{

  UINT        status ;

  while(1)
  {
    if (tx_queue_receive(&ux_app_MsgQueue_msc, &media, TX_WAIT_FOREVER) != TX_SUCCESS)
    {
      Error_Handler();
    }


    if ((storage != NULL) && (media != NULL))
    {
      /* Create a file */
      status = App_File_Create(media);

      /* check status */
      if (status == UX_SUCCESS)
      {
        USBH_UsrLog("File TEST.TXT Created \n");
      }
      else
      {
        USBH_ErrLog(" !! Could Not Create TEST.TXT File !! ");
        break;
      }

      /* Start write File Operation */
      USBH_UsrLog("Write Process ...... \n");
      status = App_File_Write(media);

      /* check status */
      if (status == UX_SUCCESS)
      {
        USBH_UsrLog("Write Process Success \n");
      }
      else
      {
        USBH_ErrLog("!! Write Process Fail !! ");
        break;
      }

      /* Start Read File Operation and comparison operation */
      USBH_UsrLog("Read Process  ...... \n");
      status = App_File_Read(media);

      /* check status */
      if (status == UX_SUCCESS)
      {
        USBH_UsrLog("Read Process Success  \n");
        USBH_UsrLog("File Closed \n");
        USBH_UsrLog("*** End Files operations ***\n")
      }
      else
      {
        USBH_ErrLog("!! Read Process Fail !! \n");
        break;
      }

    }
    else
    {
      tx_thread_sleep(MS_TO_TICK(10));
    }
  }
}

/* USER CODE END 1 */
