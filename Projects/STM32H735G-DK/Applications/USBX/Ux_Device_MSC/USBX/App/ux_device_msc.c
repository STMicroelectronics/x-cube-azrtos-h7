/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_msc.c
  * @author  MCD Application Team
  * @brief   USBX Device applicative file
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
#include "ux_device_msc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "tx_api.h"
#include "app_usbx_device.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SD_READ_FLAG                         0x01
#define SD_WRITE_FLAG                        0x02
#define SD_TIMEOUT                           100
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static int32_t check_sd_status(uint32_t instance);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief  Function implementing app_usb_device_thread_media_status.
  * @param arg: Not used
  * @retval status
  */
UINT STORAGE_Status(VOID *storage, ULONG lun, ULONG media_id, ULONG *media_status)
{

  /* The ATA drive never fails. This is just for app_usb_device only !!!! */
  return (UX_SUCCESS);
}

/**
  * @brief  Function implementing app_usb_device_thread_media_read.
  * @param  storage : Not used
  * @param  lun: Logical unit number
  * @param  lba: Logical block address
  * @param  number_blocks: Blocks number
  * @param  data_pointer: Data
  * @param  media_status: Not used
  * @retval Status (0 : OK / -1 : Error)
  */
UINT STORAGE_Read(VOID *storage, ULONG lun, UCHAR *data_pointer,
                  ULONG number_blocks, ULONG lba, ULONG *media_status)
{
  UINT status = 0U;
  ULONG ReadFlags = 0U;

  /* Check if the SD card is present */
  if (BSP_SD_IsDetected(0) != SD_NOT_PRESENT)
  {
    /* Check id SD card is ready */
    check_sd_status(0);

    /* Start the Dma write */
    status =  BSP_SD_ReadBlocks_DMA(SD_INSTANCE, (uint32_t *) data_pointer,
                                    lba, number_blocks);

    /* Wait on readflag until SD card is ready to use for new operation */
    if (tx_event_flags_get(&EventFlag, SD_READ_FLAG, TX_OR_CLEAR,
                           &ReadFlags, TX_WAIT_FOREVER) != TX_SUCCESS)
    {
      Error_Handler();
    }
  }

  return (status);
}

/**
  * @brief  Function implementing app_usb_device_thread_media_write.
  * @param  storage : Not used
  * @param  lun: Logical unit number
  * @param  lba: Logical block address
  * @param  number_blocks: Blocks number
  * @param  data_pointer: Data
  * @param  media_status: Not used
  * @retval Status (0 : OK / -1 : Error)
  */
UINT STORAGE_Write(VOID *storage, ULONG lun, UCHAR *data_pointer,
                   ULONG number_blocks, ULONG lba, ULONG *media_status)
{

  UINT status = 0U;
  ULONG WriteFlags = 0U;

  /* Check if the SD card is present */
  if (BSP_SD_IsDetected(0) != SD_NOT_PRESENT)
  {
    /* Check id SD card is ready */
    check_sd_status(0);

    /* Start the Dma write */
    status =  BSP_SD_WriteBlocks_DMA(SD_INSTANCE, (uint32_t *) data_pointer,
                                     lba, number_blocks);

    /* Wait on writeflag until SD card is ready to use for new operation */
    if (tx_event_flags_get(&EventFlag, SD_WRITE_FLAG, TX_OR_CLEAR,
                           &WriteFlags, TX_WAIT_FOREVER) != TX_SUCCESS)
    {
      Error_Handler();
    }
  }

  return (status);
}

/**
  * @brief BSP Tx Transfer completed callbacks
  * @param Instance
  * @retval None
  */
void BSP_SD_WriteCpltCallback(uint32_t Instance)
{
  if (tx_event_flags_set(&EventFlag, SD_WRITE_FLAG, TX_OR) != TX_SUCCESS)
  {
    Error_Handler();
  }
}


/**
  * @brief BSP Rx Transfer completed callbacks
  * @param instance
  * @retval None
  */
void BSP_SD_ReadCpltCallback(uint32_t Instance)
{
  if (tx_event_flags_set(&EventFlag, SD_READ_FLAG, TX_OR) != TX_SUCCESS)
  {
    Error_Handler();
  }
}


/**
  * @brief check SD card Transfer Status
  * @param Instance
  * @retval BSP_ERROR_NONE, BSP_ERROR_BUSY
  */
static int32_t check_sd_status(uint32_t Instance)
{
  uint32_t start = tx_time_get();

  while (tx_time_get() - start < SD_TIMEOUT)
  {
    if (BSP_SD_GetCardState(Instance) == SD_TRANSFER_OK)
    {
      return BSP_ERROR_NONE;
    }
  }

  return BSP_ERROR_BUSY;
}
/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
