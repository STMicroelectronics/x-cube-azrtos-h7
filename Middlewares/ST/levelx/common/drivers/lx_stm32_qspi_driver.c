/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

#include "lx_stm32_qspi_driver.h"

static UINT  lx_qspi_driver_read_sector(ULONG *flash_address, ULONG *destination, ULONG words);
static UINT  lx_qspi_driver_write_sector(ULONG *flash_address, ULONG *source, ULONG words);

static UINT  lx_qspi_driver_erase_block(ULONG block, ULONG erase_count);
static UINT  lx_qspi_driver_block_erased_verify(ULONG block);

static UINT  lx_qspi_driver_system_error(UINT error_code);

static UINT  check_status(void);

#ifndef LX_DIRECT_READ
ULONG  nor_sector_memory[DEFAULT_BLOCK_SIZE * 2];
#endif

static UINT is_initialized = LX_FALSE;

static UINT check_status(void)
{
  ULONG start = tx_time_get();
  while( BSP_QSPI_GetStatus(QSPI_INSTANCE) < 0 )
  {
    if(tx_time_get() - start > (120 * TX_TIMER_TICKS_PER_SECOND))
      return LX_ERROR;
  }

  return LX_SUCCESS;
}

UINT lx_stm32_qspi_initialize(LX_NOR_FLASH *nor_flash)
{
  BSP_QSPI_Info_t qspi_info;

  if (is_initialized == LX_FALSE)
  {
    if(BSP_QSPI_GetInfo(QSPI_INSTANCE, &qspi_info) != BSP_ERROR_NONE)
    {
      return LX_ERROR;
    }

#if (LX_DRIVER_CALLS_QSPI_INIT == 1)

    TX_INTERRUPT_SAVE_AREA
      BSP_QSPI_Init_t qspi_config;

    /* QSPI device configuration */
    qspi_config.InterfaceMode = BSP_QSPI_QPI_MODE;
    qspi_config.TransferRate  = BSP_QSPI_DTR_TRANSFER;

    TX_DISABLE

      if(BSP_QSPI_Init(QSPI_INSTANCE, &qspi_config) != BSP_ERROR_NONE)
      {
        return LX_ERROR;
      }

#if (LX_DRIVER_ERASES_QSPI_AFTER_INIT == 1)
    if( BSP_QSPI_EraseChip(QSPI_INSTANCE) != BSP_ERROR_NONE)
    {
      return LX_ERROR;
    }

    if(check_status() != LX_SUCCESS)
    {
      return LX_ERROR;
    }

#endif

    TX_RESTORE

#endif

      /* Setup the base address of the flash memory.  */
      nor_flash->lx_nor_flash_base_address = 0;

    /* Setup geometry of the flash.  */
    nor_flash->lx_nor_flash_total_blocks = qspi_info.FlashSize / qspi_info.EraseSectorSize;
    nor_flash->lx_nor_flash_words_per_block = qspi_info.EraseSectorSize / sizeof(ULONG);

    nor_flash->lx_nor_flash_driver_read = lx_qspi_driver_read_sector;
    nor_flash->lx_nor_flash_driver_write = lx_qspi_driver_write_sector;

    nor_flash->lx_nor_flash_driver_block_erase = lx_qspi_driver_erase_block;
    nor_flash->lx_nor_flash_driver_block_erased_verify = lx_qspi_driver_block_erased_verify;

    nor_flash->lx_nor_flash_driver_system_error = lx_qspi_driver_system_error;

#ifndef LX_DIRECT_READ
    /* Setup local buffer for NOR flash operation. This buffer must be the sector size of the NOR flash memory.  */
    nor_flash->lx_nor_flash_sector_buffer =  &nor_sector_memory[0];
#endif
    is_initialized = LX_TRUE;
  }
  /* Return success.  */
  return(LX_SUCCESS);
}

static UINT  lx_qspi_driver_read_sector(ULONG *flash_address, ULONG *destination, ULONG words)
{
  if(check_status() != LX_SUCCESS)
  {
    return LX_ERROR;
  }

  if(BSP_QSPI_Read(QSPI_INSTANCE, (uint8_t *)destination, (uint32_t)flash_address, (uint32_t) words * sizeof(ULONG)) != BSP_ERROR_NONE)
  {
    return(LX_ERROR);
  }
  else
  {
    return(LX_SUCCESS);
  }
}

static UINT  lx_qspi_driver_write_sector(ULONG *flash_address, ULONG *source, ULONG words)
{
  if(check_status() != LX_SUCCESS)
  {
    return LX_ERROR;
  }

  if(BSP_QSPI_Write(QSPI_INSTANCE, (uint8_t *) source, (uint32_t) flash_address, (uint32_t) words * sizeof(ULONG)) != BSP_ERROR_NONE)
  {
    return(LX_ERROR);
  }
  else
  {
    return(LX_SUCCESS);
  }
}

static UINT  lx_qspi_driver_erase_block(ULONG block, ULONG erase_count)
{
  LX_PARAMETER_NOT_USED(erase_count);

  if(check_status() != LX_SUCCESS)
  {
    return LX_ERROR;
  }

  if(BSP_QSPI_EraseBlock(QSPI_INSTANCE, block * DEFAULT_BLOCK_SIZE, BSP_QSPI_ERASE_8K) != BSP_ERROR_NONE)
  {
    return(LX_ERROR);
  }
  else
  {
    return(LX_SUCCESS);
  }
}

static UINT  lx_qspi_driver_block_erased_verify(ULONG block)
{
  LX_PARAMETER_NOT_USED(block);

  /*
  * Implemnting this API is costy as we need to read the block and verify that it was actually erased.
  * We rely on the HW and just return LX_SUCCESS
  */

  return(LX_SUCCESS);
}

static UINT  lx_qspi_driver_system_error(UINT error_code)
{
  LX_PARAMETER_NOT_USED(error_code);

  return(LX_ERROR);
}

