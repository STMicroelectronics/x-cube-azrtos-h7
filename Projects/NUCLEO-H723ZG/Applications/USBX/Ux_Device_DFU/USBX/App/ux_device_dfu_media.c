/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_dfu_media.c
  * @author  MCD Application Team
  * @brief   USBX Device DFU applicative source file
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
#include "ux_device_dfu_media.h"

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
#define DFU_MEDIA_ERASE_TIME    (uint16_t)5U
#define DFU_MEDIA_PROGRAM_TIME  (uint16_t)5U

/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

UX_SLAVE_CLASS_DFU *dfu;

extern TX_QUEUE                         ux_app_MsgQueue;
extern ux_dfu_downloadInfotypeDef       ux_dfu_download;
extern PCD_HandleTypeDef                hpcd_USB_OTG_HS;

ULONG   dfu_status = 0U;
ULONG   Address_ptr;
UCHAR   RX_Data[1024];
UINT    Leave_DFU_State = LEAVE_DFU_DISABLED;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static uint16_t DFU_Erase(uint32_t Address);
static uint32_t GetSector(uint32_t Address);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  USBD_DFU_Activate
  *         This function is called when insertion of a DFU device.
  * @param  dfu_instance: Pointer to the dfu class instance.
  * @retval none
  */
VOID USBD_DFU_Activate(VOID *dfu_instance)
{
  /* USER CODE BEGIN USBD_DFU_Activate */

  /* Save the DFU instance */
  dfu = (UX_SLAVE_CLASS_DFU*) dfu_instance;

  /* Unlock the internal flash */
  HAL_FLASH_Unlock();

  /* USER CODE END USBD_DFU_Activate */

  return;
}

/**
  * @brief  USBD_DFU_Deactivate
  *         This function is called when extraction of a DFU device.
  * @param  dfu_instance: Pointer to the dfu class instance.
  * @retval none
  */
VOID USBD_DFU_Deactivate(VOID *dfu_instance)
{
  /* USER CODE BEGIN USBD_DFU_Deactivate */
  UX_PARAMETER_NOT_USED(dfu_instance);

  /* Reset the DFU instance */
  dfu = UX_NULL;

  /* Lock the internal flash */
  HAL_FLASH_Lock();

  /* USER CODE END USBD_DFU_Deactivate */

  return;
}

/**
  * @brief  USBD_DFU_GetStatus
  *         This function is invoked to get media status.
  * @param  dfu_instance: Pointer to the dfu class instance.
  * @param  media_status: dfu media status.
  * @retval status
  */
UINT USBD_DFU_GetStatus(VOID *dfu_instance, ULONG *media_status)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_DFU_GetStatus */
  UX_PARAMETER_NOT_USED(dfu_instance);

  /* Store DFU status */
  *media_status = dfu_status;

  /* USER CODE END USBD_DFU_GetStatus */

  return status;
}

/**
  * @brief  USBD_DFU_Read
  *         This function is invoked when host is requesting to read from media.
  * @param  dfu_instance: Pointer to the dfu class instance.
  * @param  block_number: block number.
  * @param  data_pointer: Pointer to the Source buffer.
  * @param  length: Number of data to be read (in bytes).
  * @param  actual_length: length of data to be written.
  * @retval status
  */
UINT USBD_DFU_Read(VOID *dfu_instance, ULONG block_number, UCHAR *data_pointer,
                   ULONG length, ULONG *actual_length)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_DFU_Read */
  UX_PARAMETER_NOT_USED(dfu_instance);

  UCHAR *Src_ptr = NULL;
  UINT Block_index = 0U;
  ULONG Address_src = 0U;

  /* Check if block number is NULL */
  if (block_number == 0U)
  {
    /* Store the values of all supported commands */
    *data_pointer = DFU_CMD_GETCOMMANDS;
    *(data_pointer + 1) = DFU_CMD_SETADDRESSPOINTER;
    *(data_pointer + 2) = DFU_CMD_ERASE ;
    *(data_pointer + 3) = 0;
  }
  else if (block_number > 0)
  {
    /* Return the physical address from which the host requests to read data */
    Address_src = ((block_number - 2) * UX_SLAVE_REQUEST_CONTROL_MAX_LENGTH) + Address_ptr;

    /* Get pointer of the source address */
    Src_ptr = (uint8_t*)Address_src;

    /* Perform the Read operation */
    for (Block_index = 0; Block_index < length; Block_index++)
    {
      /* Copy data from Source pointer to data_pointer buffer */
      *(data_pointer + Block_index) = *Src_ptr ++;
    }

    /* Store data length */
    *actual_length = length;
  }
  else
  {
    status = UX_ERROR;
  }

  /* USER CODE END USBD_DFU_Read */

  return status;
}

/**
  * @brief  USBD_DFU_Write
  *         This function is invoked when host is requesting to write in media.
  * @param  dfu_instance: Pointer to the dfu class instance.
  * @param  block_number: block number.
  * @param  data_pointer: Pointer to the Source buffer.
  * @param  length: Number of data to be read (in bytes).
  * @param  media_status: dfu media status.
  * @retval status
  */
UINT USBD_DFU_Write(VOID *dfu_instance, ULONG block_number, UCHAR *data_pointer,
                    ULONG length, ULONG *media_status)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_DFU_Write */
  UX_PARAMETER_NOT_USED(dfu_instance);
  UX_PARAMETER_NOT_USED(media_status);

  ULONG dfu_polltimeout = 0U;

  /* Store ux_dfu_download info */
  ux_dfu_download.wlength = length;
  ux_dfu_download.data_ptr = RX_Data;
  ux_dfu_download.wblock_num = block_number;

  ux_utility_memory_copy(ux_dfu_download.data_ptr, data_pointer, length);

  if((block_number == 0) && (*data_pointer == DFU_CMD_ERASE))
  {
    /* Set the time necessary for an erase operation */
    dfu_polltimeout = DFU_MEDIA_ERASE_TIME;

    /* Set DFU media status Busy, dfu polltimeout in erase phase */
    dfu_status = UX_SLAVE_CLASS_DFU_MEDIA_STATUS_BUSY;
    dfu_status += UX_SLAVE_CLASS_DFU_STATUS_OK << 4;
    dfu_status += (uint8_t)(dfu_polltimeout)<< 8;
  }
  else
  {
    /* Set the time necessary for a program operation */
    dfu_polltimeout = DFU_MEDIA_PROGRAM_TIME;

    /* Set DFU media status Busy, dfu polltimeout in program phase */
    dfu_status = UX_SLAVE_CLASS_DFU_MEDIA_STATUS_BUSY;
    dfu_status += UX_SLAVE_CLASS_DFU_STATUS_OK << 4;
    dfu_status += (uint8_t)(dfu_polltimeout)<< 8;
  }

  /* Put a message queue to usbx_dfu_download_thread_entry */
  if (tx_queue_send(&ux_app_MsgQueue, &ux_dfu_download, TX_NO_WAIT) != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* USER CODE END USBD_DFU_Write */

  return status;
}

/**
  * @brief  USBD_DFU_Notify
  *         This function is invoked to application when a begin and end
  *         of transfer of the firmware occur.
  * @param  dfu_instance: Pointer to the dfu class instance.
  * @param  notification: unused.
  * @retval status
  */
UINT USBD_DFU_Notify(VOID *dfu_instance, ULONG notification)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_DFU_Notify */
  UX_PARAMETER_NOT_USED(dfu_instance);
  UX_PARAMETER_NOT_USED(notification);
  /* USER CODE END USBD_DFU_Notify */

  return status;
}

#ifdef UX_DEVICE_CLASS_DFU_CUSTOM_REQUEST_ENABLE

/**
  * @brief  USBD_DFU_CustomRequest
  *         This function is invoked to Handles DFU sub-protocol request.
  * @param  dfu_instance: Pointer to the dfu class instance.
  * @param  transfer: transfer request.
  * @retval status
  */
UINT USBD_DFU_CustomRequest(VOID *dfu_instance, UX_SLAVE_TRANSFER *transfer)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_DFU_CustomRequest */
  UX_PARAMETER_NOT_USED(dfu_instance);

  UCHAR *setup;
  UCHAR dfu_state;

  status = UX_ERROR;

  /* Get DFU state */
  dfu_state = ux_device_class_dfu_state_get((UX_SLAVE_CLASS_DFU*)dfu);

  setup = transfer->ux_slave_transfer_request_setup;

  if((dfu_state == UX_SYSTEM_DFU_STATE_DFU_IDLE) ||
     (dfu_state == UX_SYSTEM_DFU_STATE_DFU_DNLOAD_IDLE))
  {

    if (setup[UX_SETUP_REQUEST] == UX_SLAVE_CLASS_DFU_COMMAND_DOWNLOAD)
    {

      if ((setup[UX_SETUP_LENGTH] == 0) && (setup[UX_SETUP_LENGTH +1] == 0))
      {
        /* Update Leave DFU state */
        Leave_DFU_State = LEAVE_DFU_ENABLED;

        /* Disconnect the USB device  */
        HAL_PCD_Stop(&hpcd_USB_OTG_HS);

        /* Disconnect USBX stack driver,  */
        ux_device_stack_disconnect();

        status = UX_SUCCESS;
      }
    }
  }

  /* USER CODE END USBD_DFU_CustomRequest */

  return status;
}

#endif /* UX_DEVICE_CLASS_DFU_CUSTOM_REQUEST_ENABLE */

/* USER CODE BEGIN 1 */

/**
  * @brief  Function implementing usbx_dfu_download_thread_entry.
  * @param  thread_input: Not used.
  * @retval none
  */
VOID usbx_dfu_download_thread_entry(ULONG thread_input)
{
  UINT status, Command;
  ULONG Block_index, Data_address, Address_dest, Media_address;
  UX_PARAMETER_NOT_USED(thread_input);

  while (1)
  {

    /* Receive a message queue from DFU_Write callback */
    status = tx_queue_receive(&ux_app_MsgQueue, &ux_dfu_download, TX_WAIT_FOREVER);

    /* Check the completion code and the actual flags returned */
    if (status == TX_SUCCESS)
    {

      if(ux_dfu_download.wblock_num == 0)
      {

        Command = *(ux_dfu_download.data_ptr);

        /* Decode the Special Command */
        switch ( Command )
        {

          case DFU_CMD_SETADDRESSPOINTER:

            /* Get address pointer value used for computing the start address
               for Read and Write memory operations */
            Address_ptr =  *(ux_dfu_download.data_ptr + 1) ;
            Address_ptr += *(ux_dfu_download.data_ptr + 2) << 8 ;
            Address_ptr += *(ux_dfu_download.data_ptr + 3) << 16 ;
            Address_ptr += *(ux_dfu_download.data_ptr + 4) << 24 ;

            /* Set DFU Status OK */
            dfu_status = UX_SLAVE_CLASS_DFU_MEDIA_STATUS_OK;
            dfu_status += UX_SLAVE_CLASS_DFU_STATUS_OK << 4;

            /* Update USB DFU state machine */
            ux_device_class_dfu_state_sync(dfu);

            break;

          case DFU_CMD_ERASE:

            /* Get address pointer value to erase one page of the internal
               media memory */
            Address_ptr =  *(ux_dfu_download.data_ptr + 1);
            Address_ptr += *(ux_dfu_download.data_ptr + 2) << 8;
            Address_ptr += *(ux_dfu_download.data_ptr + 3) << 16;
            Address_ptr += *(ux_dfu_download.data_ptr + 4) << 24;

            /* Erase memory */
            if (DFU_Erase(Address_ptr) != UX_SUCCESS)
            {
              dfu_status =  UX_SLAVE_CLASS_DFU_MEDIA_STATUS_ERROR;
              dfu_status += UX_SLAVE_CLASS_DFU_STATUS_ERROR_ERASE << 4;
            }
            else
            {
              /* Set DFU status OK */
              dfu_status = UX_SLAVE_CLASS_DFU_MEDIA_STATUS_OK;
              dfu_status += UX_SLAVE_CLASS_DFU_STATUS_OK << 4;
            }

            /* Update USB DFU state machine */
            ux_device_class_dfu_state_sync(dfu);

            break;

          default:
            break;

        }
      }
      /* Regular Download Command */
      else
      {
        /* Decode the required address to which the host requests to write data */
        Address_dest = ((ux_dfu_download.wblock_num - 2U) * UX_SLAVE_REQUEST_CONTROL_MAX_LENGTH) +  Address_ptr;

        /* Perform the write operation */
        for (Block_index = 0; Block_index < ux_dfu_download.wlength; Block_index += 32)
        {
          /* Get address of destination buffer  */
          Media_address = (uint32_t) (Address_dest + Block_index);

          /* Get Pointer to the source buffer */
          Data_address  = (uint32_t) (ux_dfu_download.data_ptr + Block_index);

          /* Program flash word at a Address_dest address */
          if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD,
                                Media_address,
                                Data_address) == HAL_OK)
          {
            /* Check the written value */
            if (*(uint64_t *) Data_address != *(uint64_t *) Media_address)
            {
              /* Flash content doesn't match SRAM content, Set DFU status Error */
              dfu_status =  UX_SLAVE_CLASS_DFU_MEDIA_STATUS_ERROR;
              dfu_status += UX_SLAVE_CLASS_DFU_STATUS_ERROR_WRITE << 4;

              /* Update USB DFU state machine */
              ux_device_class_dfu_state_sync(dfu);
            }
          }
          else
          {
            /* Error occurred while writing data in Flash memory, Set DFU status Error */
            dfu_status =  UX_SLAVE_CLASS_DFU_MEDIA_STATUS_ERROR;
            dfu_status += UX_SLAVE_CLASS_DFU_STATUS_ERROR_WRITE << 4;

            /* Update USB DFU state machine */
            ux_device_class_dfu_state_sync(dfu);
          }
        }

        /* Set DFU Status OK */
        dfu_status =  UX_SLAVE_CLASS_DFU_MEDIA_STATUS_OK;
        dfu_status += UX_SLAVE_CLASS_DFU_STATUS_OK << 4;

        /* Update USB DFU state machine */
        ux_device_class_dfu_state_sync(dfu);
      }
    }
    else
    {
      /* Sleep thread for 10ms */
      tx_thread_sleep(MS_TO_TICK(10));
    }
  }
}

/**
  * @brief  DFU_Erase
  *         Erase flash sector.
  * @param  Address: Address of sector to be erased.
  * @retval UX_SUCCESS if operation is successful.
  */
static uint16_t DFU_Erase(uint32_t Address)
{
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint16_t status = UX_SUCCESS;
  uint32_t SECTORError  = 0;
  TX_INTERRUPT_SAVE_AREA

  /* Fill EraseInit structure */
  EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.Banks         = FLASH_BANK_1;
  EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
  EraseInitStruct.Sector        = GetSector(Address);
  EraseInitStruct.NbSectors     = 1;

  TX_DISABLE

  /* Execute erase operation */
  status = HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);

  TX_RESTORE

  if (status != HAL_OK)
  {
    status = UX_ERROR;
  }

  return (status);
}

/**
  * @brief  GetSector
  *         Get the sector of a given address.
  * @param  Address: Address of the FLASH Memory.
  * @retval sector: The sector of a given address.
  */
static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;

  if (Address < (FLASH_BASE + FLASH_BANK_SIZE))
  {
    sector = (Address - FLASH_BASE) / FLASH_SECTOR_SIZE;
  }
  else
  {
    sector = (Address - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_SECTOR_SIZE;
  }

  return (sector);
}
/* USER CODE END 1 */
