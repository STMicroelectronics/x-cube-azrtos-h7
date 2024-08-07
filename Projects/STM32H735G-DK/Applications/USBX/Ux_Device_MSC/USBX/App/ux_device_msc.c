/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_msc.c
  * @author  MCD Application Team
  * @brief   USBX Device MSC applicative source file
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
#include "ux_device_msc.h"

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
#define SD_READ_FLAG   0x01
#define SD_WRITE_FLAG  0x02
#define SD_TIMEOUT     100U
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern TX_EVENT_FLAGS_GROUP EventFlag;
extern HAL_SD_CardInfoTypeDef USBD_SD_CardInfo;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static int32_t check_sd_status(VOID);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  USBD_STORAGE_Activate
  *         This function is called when insertion of a storage device.
  * @param  storage_instance: Pointer to the storage class instance.
  * @retval none
  */
VOID USBD_STORAGE_Activate(VOID *storage_instance)
{
  /* USER CODE BEGIN USBD_STORAGE_Activate */
  UX_PARAMETER_NOT_USED(storage_instance);
  /* USER CODE END USBD_STORAGE_Activate */

  return;
}

/**
  * @brief  USBD_STORAGE_Deactivate
  *         This function is called when extraction of a storage device.
  * @param  storage_instance: Pointer to the storage class instance.
  * @retval none
  */
VOID USBD_STORAGE_Deactivate(VOID *storage_instance)
{
  /* USER CODE BEGIN USBD_STORAGE_Desactivate */
  UX_PARAMETER_NOT_USED(storage_instance);
  /* USER CODE END USBD_STORAGE_Desactivate */

  return;
}

/**
  * @brief  USBD_STORAGE_Read
  *         This function is invoked to read from media.
  * @param  storage_instance : Pointer to the storage class instance.
  * @param  lun: Logical unit number is the command is directed to.
  * @param  data_pointer: Address of the buffer to be used for reading or writing.
  * @param  number_blocks: number of sectors to read/write.
  * @param  lba: Logical block address is the sector address to read.
  * @param  media_status: should be filled out exactly like the media status
  *                       callback return value.
  * @retval status
  */
UINT USBD_STORAGE_Read(VOID *storage_instance, ULONG lun, UCHAR *data_pointer,
                       ULONG number_blocks, ULONG lba, ULONG *media_status)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_STORAGE_Read */
  UX_PARAMETER_NOT_USED(storage_instance);
  UX_PARAMETER_NOT_USED(lun);
  UX_PARAMETER_NOT_USED(media_status);

  ULONG ReadFlags = 0U;

  /* Check if the SD card is present */
  if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_5) == GPIO_PIN_SET)
  {
    /* Check id SD card is ready */
    if(check_sd_status() != HAL_OK)
    {
      Error_Handler();
    }

    /* Start the Dma write */
    status =  HAL_SD_ReadBlocks_DMA(&hsd1, data_pointer, lba, number_blocks);
    if(status != HAL_OK)
    {
      Error_Handler();
    }

    /* Wait on readflag until SD card is ready to use for new operation */
    if (tx_event_flags_get(&EventFlag, SD_READ_FLAG, TX_OR_CLEAR,
                           &ReadFlags, TX_WAIT_FOREVER) != TX_SUCCESS)
    {
      Error_Handler();
    }
  }

  /* USER CODE END USBD_STORAGE_Read */

  return status;
}

/**
  * @brief  USBD_STORAGE_Write
  *         This function is invoked to write in media.
  * @param  storage_instance : Pointer to the storage class instance.
  * @param  lun: Logical unit number is the command is directed to.
  * @param  data_pointer: Address of the buffer to be used for reading or writing.
  * @param  number_blocks: number of sectors to read/write.
  * @param  lba: Logical block address is the sector address to read.
  * @param  media_status: should be filled out exactly like the media status
  *                       callback return value.
  * @retval status
  */
UINT USBD_STORAGE_Write(VOID *storage_instance, ULONG lun, UCHAR *data_pointer,
                        ULONG number_blocks, ULONG lba, ULONG *media_status)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_STORAGE_Write */
  UX_PARAMETER_NOT_USED(storage_instance);
  UX_PARAMETER_NOT_USED(lun);
  UX_PARAMETER_NOT_USED(media_status);

  ULONG WriteFlags = 0U;

  /* Check if the SD card is present */
  if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_5) == GPIO_PIN_SET)
  {
    /* Check id SD card is ready */
    if(check_sd_status() != HAL_OK)
    {
      Error_Handler();
    }

    /* Start the Dma write */
    status = HAL_SD_WriteBlocks_DMA(&hsd1, data_pointer, lba, number_blocks);

    if(status != HAL_OK)
    {
      Error_Handler();
    }

    /* Wait on writeflag until SD card is ready to use for new operation */
    if (tx_event_flags_get(&EventFlag, SD_WRITE_FLAG, TX_OR_CLEAR,
                           &WriteFlags, TX_WAIT_FOREVER) != TX_SUCCESS)
    {
      Error_Handler();
    }
  }

  /* USER CODE END USBD_STORAGE_Write */

  return status;
}

/**
  * @brief  USBD_STORAGE_Flush
  *         This function is invoked to flush media.
  * @param  storage_instance : Pointer to the storage class instance.
  * @param  lun: Logical unit number is the command is directed to.
  * @param  number_blocks: number of sectors to read/write.
  * @param  lba: Logical block address is the sector address to read.
  * @param  media_status: should be filled out exactly like the media status
  *                       callback return value.
  * @retval status
  */
UINT USBD_STORAGE_Flush(VOID *storage_instance, ULONG lun, ULONG number_blocks,
                        ULONG lba, ULONG *media_status)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_STORAGE_Flush */
  UX_PARAMETER_NOT_USED(storage_instance);
  UX_PARAMETER_NOT_USED(lun);
  UX_PARAMETER_NOT_USED(number_blocks);
  UX_PARAMETER_NOT_USED(lba);
  UX_PARAMETER_NOT_USED(media_status);
  /* USER CODE END USBD_STORAGE_Flush */

  return status;
}

/**
  * @brief  USBD_STORAGE_Status
  *         This function is invoked to obtain the status of the device.
  * @param  storage_instance : Pointer to the storage class instance.
  * @param  lun: Logical unit number is the command is directed to.
  * @param  media_id: is not currently used.
  * @param  media_status: should be filled out exactly like the media status
  *                       callback return value.
  * @retval status
  */
UINT USBD_STORAGE_Status(VOID *storage_instance, ULONG lun, ULONG media_id,
                         ULONG *media_status)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_STORAGE_Status */
  UX_PARAMETER_NOT_USED(storage_instance);
  UX_PARAMETER_NOT_USED(lun);
  UX_PARAMETER_NOT_USED(media_id);
  UX_PARAMETER_NOT_USED(media_status);
  /* USER CODE END USBD_STORAGE_Status */

  return status;
}

/**
  * @brief  USBD_STORAGE_Notification
  *         This function is invoked to obtain the notification of the device.
  * @param  storage_instance : Pointer to the storage class instance.
  * @param  lun: Logical unit number is the command is directed to.
  * @param  media_id: is not currently used.
  * @param  notification_class: specifies the class of notification.
  * @param  media_notification: response for the notification.
  * @param  media_notification_length: length of the response buffer.
  * @retval status
  */
UINT USBD_STORAGE_Notification(VOID *storage_instance, ULONG lun, ULONG media_id,
                               ULONG notification_class, UCHAR **media_notification,
                               ULONG *media_notification_length)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_STORAGE_Notification */
  UX_PARAMETER_NOT_USED(storage_instance);
  UX_PARAMETER_NOT_USED(lun);
  UX_PARAMETER_NOT_USED(media_id);
  UX_PARAMETER_NOT_USED(notification_class);
  UX_PARAMETER_NOT_USED(media_notification);
  UX_PARAMETER_NOT_USED(media_notification_length);
  /* USER CODE END USBD_STORAGE_Notification */

  return status;
}

/**
  * @brief  USBD_STORAGE_GetMediaLastLba
  *         Get Media last LBA.
  * @param  none
  * @retval last lba
  */
ULONG USBD_STORAGE_GetMediaLastLba(VOID)
{
  ULONG LastLba = 0U;

  /* USER CODE BEGIN USBD_STORAGE_GetMediaLastLba */

  LastLba = (ULONG)(USBD_SD_CardInfo.BlockNbr - 1);

  /* USER CODE END USBD_STORAGE_GetMediaLastLba */

  return LastLba;
}

/**
  * @brief  USBD_STORAGE_GetMediaBlocklength
  *         Get Media block length.
  * @param  none.
  * @retval block length.
  */
ULONG USBD_STORAGE_GetMediaBlocklength(VOID)
{
  ULONG MediaBlockLen = 0U;

  /* USER CODE BEGIN USBD_STORAGE_GetMediaBlocklength */

  MediaBlockLen = (ULONG) USBD_SD_CardInfo.BlockSize;

  /* USER CODE END USBD_STORAGE_GetMediaBlocklength */

  return MediaBlockLen;
}

/* USER CODE BEGIN 1 */

/**
  * @brief  BSP_SD_WriteCpltCallback
  *         BSP Tx Transfer completed callbacks
  * @param  Instance
  * @retval none
  */
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
  if (tx_event_flags_set(&EventFlag, SD_WRITE_FLAG, TX_OR) != TX_SUCCESS)
  {
    Error_Handler();
  }
}

/**
  * @brief  BSP_SD_ReadCpltCallback
  *         BSP Rx Transfer completed callbacks
  * @param  Instance
  * @retval None
  */
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
  if (tx_event_flags_set(&EventFlag, SD_READ_FLAG, TX_OR) != TX_SUCCESS)
  {
    Error_Handler();
  }
}

/**
  * @brief  check_sd_status
  *         check SD card Transfer Status.
  * @param  none
  * @retval BSP status
  */
static int32_t check_sd_status(VOID)
{
  uint32_t start = tx_time_get();

  while (tx_time_get() - start < SD_TIMEOUT)
  {
    if (HAL_SD_GetCardState(&hsd1) == HAL_SD_CARD_TRANSFER)
    {
      return HAL_OK;
    }
  }

  return HAL_ERROR;
}

/* USER CODE END 1 */
