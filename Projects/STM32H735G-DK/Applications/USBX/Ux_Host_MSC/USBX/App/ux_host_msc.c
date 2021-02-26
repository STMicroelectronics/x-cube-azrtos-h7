/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_host_msc.c
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
extern TX_QUEUE                     ux_app_MsgQueue_msc;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
void  msc_process_thread_entry(ULONG arg);
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
  FX_MEDIA    *fx_media;

  while(1)
  {
    status = tx_queue_receive(&ux_app_MsgQueue_msc, &fx_media, TX_WAIT_FOREVER);
    if ((storage != NULL) && (fx_media != NULL))
    {
      /* Create a file */
      status = App_File_Create(fx_media);

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
      status = App_File_Write(fx_media);

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
      status = App_File_Read(fx_media);

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
      tx_thread_sleep(10);
    }
  }
}

/* USER CODE END 1 */
