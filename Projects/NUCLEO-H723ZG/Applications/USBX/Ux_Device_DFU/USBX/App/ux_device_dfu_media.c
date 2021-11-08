/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_dfu_media.c
  * @author  MCD Application Team
  * @brief   USBX Device applicative file
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
#include "main.h"
#include "tx_api.h"
#include "app_usbx_device.h"
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
#define LEAVE_DFU_ENABLED             1
#define LEAVE_DFU_DISABLED            0
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
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

/**
  * @brief  Initializes Memory routine, Unlock the internal flash.
  * @param  dfu Instance.
  * @retval none.
  */
void DFU_Init(void *dfu)
{
  /* Unlock the internal flash */
  HAL_FLASH_Unlock();
}

/**
  * @brief  DeInitializes Memory routine, Lock the internal flash.
  * @param  dfu: dfu Instance.
  * @retval none.
  */
void DFU_DeInit(void *dfu)
{
  /* Lock the internal flash */
  HAL_FLASH_Lock();
}

/**
  * @brief  Get status routine.
  * @param  dfu: dfu Instance.
  * @param  media_status : dfu media status.
  * @retval UX_SUCCESS.
  */
UINT DFU_GetStatus(void *dfu, ULONG *media_status)
{
  *media_status = dfu_status;

  return (UX_SUCCESS);
}

/**
  * @brief  Inform application when a begin and end of transfer of the firmware
            occur.
  * @param  dfu: dfu Instance.
  * @param  notification: unused.
  * @retval UX_SUCCESS.
  */
UINT DFU_Notify(void *dfu, ULONG notification)
{
  UNUSED(notification);

  return (UX_SUCCESS);
}

/**
  * @brief  Memory read routine.
  * @param  dfu: dfu Instance
  * @param  block_number: block number.
  * @param  data_pointer: Pointer to the Source buffer.
  * @param  length: Number of data to be read (in bytes).
  * @retval Status.
  */
UINT DFU_Read(VOID *dfu, ULONG block_number, UCHAR * data_pointer,
              ULONG length, ULONG *media_status)
{
  UINT   Status      = UX_SUCCESS;
  UCHAR* Src_ptr     = NULL;
  UINT   Bloc_index  = 0;
  ULONG  Address_src = 0;

  if (block_number == 0)
  {
    /* Store the values of all supported commands */
    *data_pointer       = DFU_CMD_GETCOMMANDS;
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
    for (Bloc_index = 0; Bloc_index < length; Bloc_index++)
    {
      /* Copy data from Source pointer  to data_pointer buffer*/
      *(data_pointer + Bloc_index) = *Src_ptr ++;
    }

    *media_status = length;
  }
  else
  {
    Status = UX_ERROR;
  }

  return (Status);
}

/**
  * @brief  Memory write routine.
  * @param  dfu: dfu Instance.
  * @param  block_number: block number
  * @param  data_pointer: Pointer to the Source buffer.
  * @param  length: Number of data to be read (in bytes).
  * @param  media_status: Not used.
  * @retval status.
  */
UINT DFU_Write(VOID *dfu, ULONG block_number, UCHAR * data_pointer,
               ULONG length, ULONG *media_status)
{
  UINT  status  = 0U;
  ULONG dfu_polltimeout = 0U;

  /* store ux_dfu_download info*/
  ux_dfu_download.wlength = length;
  ux_dfu_download.data_ptr = RX_Data;
  ux_dfu_download.wblock_num = block_number;

  ux_utility_memory_copy(ux_dfu_download.data_ptr, data_pointer, length);

  if((block_number == 0) && (*data_pointer == DFU_CMD_ERASE))
  {
    /* set the time necessary for an erase operation*/
    dfu_polltimeout = DFU_MEDIA_ERASE_TIME;

    /* Set DFU media status Busy, dfu polltimeout in erase phase */
    dfu_status =  UX_SLAVE_CLASS_DFU_MEDIA_STATUS_BUSY;
    dfu_status += UX_SLAVE_CLASS_DFU_STATUS_OK << 4;
    dfu_status += (uint8_t)(dfu_polltimeout)<< 8;
  }
  else
  {
    /* set the time necessary for a program operation*/
    dfu_polltimeout = DFU_MEDIA_PROGRAM_TIME;

    /* Set DFU media status Busy, dfu polltimeout in program phase */
    dfu_status =  UX_SLAVE_CLASS_DFU_MEDIA_STATUS_BUSY;
    dfu_status += UX_SLAVE_CLASS_DFU_STATUS_OK << 4;
    dfu_status += (uint8_t)(dfu_polltimeout)<< 8;
  }

  /* put a message queue to usbx_dfu_download_thread_entry */
  if (tx_queue_send(&ux_app_MsgQueue, &ux_dfu_download, TX_NO_WAIT) != TX_SUCCESS)
  {
    Error_Handler();
  }

  return (status);
}

/**
  * @brief  Handles the sub-protocol DFU leave DFU mode request (leaves DFU mode
  *         and resets device to jump to user loaded code).
  * @param  dfu: dfu Instance.
  * @param  transfer: transfer request.
  * @retval None.
  */
UINT DFU_Leave(VOID *dfu, UX_SLAVE_TRANSFER *transfer)
{
  UCHAR *setup;
  UCHAR dfu_state;
  UINT  status = UX_ERROR;

  /* Get DFU state */
  dfu_state = _ux_device_class_dfu_state_get((UX_SLAVE_CLASS_DFU*)dfu);

  setup  = transfer->ux_slave_transfer_request_setup;

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

  return (status);
}

/**
  * @brief  Function implementing usbx_dfu_download_thread_entry.
  * @param  arg: Not used.
  * @retval None.
  */
void usbx_dfu_download_thread_entry(ULONG arg)
{
  UINT                  status;
  UINT                  Command;
  ULONG                 Bloc_index;
  ULONG                 Data_address;
  ULONG                 Address_dest;
  ULONG                 Media_address;
  UX_SLAVE_CLASS_DFU*   dfu = NULL;

  while (1)
  {

    /* receive a message queue from DFU_Write callback*/
    status = tx_queue_receive(&ux_app_MsgQueue, &ux_dfu_download, TX_WAIT_FOREVER);

    /* Check the completion code and the actual flags returned. */
    if (status == TX_SUCCESS)
    {

      if(ux_dfu_download.wblock_num == 0)
      {

        Command = *(ux_dfu_download.data_ptr);

        /* Decode the Special Command*/
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
            dfu_status =  UX_SLAVE_CLASS_DFU_MEDIA_STATUS_OK;
            dfu_status += UX_SLAVE_CLASS_DFU_STATUS_OK << 4;

            /* Update USB DFU state machine */
            ux_device_class_dfu_state_sync(dfu);

            break;

          case DFU_CMD_ERASE:

            /* Get address pointer value to erase one page of the internal
               media memory. */
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
              dfu_status =  UX_SLAVE_CLASS_DFU_MEDIA_STATUS_OK;
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
        for (Bloc_index = 0; Bloc_index < ux_dfu_download.wlength; Bloc_index += 32)
        {
          /* Get address of destination buffer  */
          Media_address = (uint32_t) (Address_dest + Bloc_index);

          /* Get Pointer to the source buffer */
          Data_address  = (uint32_t) (ux_dfu_download.data_ptr + Bloc_index);

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
      tx_thread_sleep(MS_TO_TICK(10));
    }
  }
}



/**
  * @brief  Erase sector.
  * @param  Address: Address of sector to be erased.
  * @retval UX_SUCCESS if operation is successful.
  */
static uint16_t DFU_Erase(uint32_t Address)
{
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t flash_ret    = UX_SUCCESS;
  uint32_t SECTORError  = 0;
  TX_INTERRUPT_SAVE_AREA

  /* Fill EraseInit structure*/
  EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.Banks         = FLASH_BANK_1;
  EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
  EraseInitStruct.Sector        = GetSector(Address);
  EraseInitStruct.NbSectors     = 1;

  TX_DISABLE

  /* Execute erase operation */
  flash_ret = HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);

  TX_RESTORE

  if (flash_ret != HAL_OK)
  {
    flash_ret = UX_ERROR;
  }

  return (flash_ret);
}

/**
  * @brief  Get the sector of a given address
  * @param  Address Address of the FLASH Memory
  * @retval The sector of a given address
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

/**
  * @brief DFU device connection callback.
  * @param  Device_State: dfu Instance.
  * @retval status.
  */
UINT DFU_Device_ConnectionCallback(ULONG Device_State)
{
  if(Device_State == UX_DEVICE_REMOVED)
  {
    if (_ux_system_slave -> ux_system_slave_device_dfu_mode ==  UX_DEVICE_CLASS_DFU_MODE_DFU)
    {
      if (Leave_DFU_State != LEAVE_DFU_DISABLED)
      {
        /* Generate system reset to allow jumping to the user code */
        NVIC_SystemReset();
      }
    }
  }
  return UX_SUCCESS;
}
/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
