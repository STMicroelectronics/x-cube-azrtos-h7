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

#include "lx_stm32_ospi_driver.h"

static UINT  lx_ospi_driver_read_sector(ULONG *flash_address, ULONG *destination, ULONG words);
static UINT  lx_ospi_driver_write_sector(ULONG *flash_address, ULONG *source, ULONG words);

static UINT  lx_ospi_driver_erase_block(ULONG block, ULONG erase_count);
static UINT  lx_ospi_driver_block_erased_verify(ULONG block);

static UINT  lx_ospi_driver_system_error(UINT error_code);

#ifndef LX_DIRECT_READ
ULONG  nor_sector_memory[512];
#endif

static UINT is_initialized = LX_FALSE;
static UINT erase_sector_size = 0;

static UINT check_status(void)
{
  ULONG start = tx_time_get();
  while( tx_time_get() - start < 10 * HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
  {
    if(BSP_OSPI_NOR_GetStatus(OSPI_INSTANCE) == BSP_ERROR_NONE)
      return LX_SUCCESS;
  }

  return LX_ERROR;
}

UINT lx_stm32_ospi_initialize(LX_NOR_FLASH *nor_flash)
{
  BSP_OSPI_NOR_Info_t ospi_info;

  if (is_initialized == LX_FALSE)
  {

#if (LX_DRIVER_CALLS_OSPI_INIT == 1)

    BSP_OSPI_NOR_Init_t ospi_config;

    /* OSPI device configuration */
    ospi_config.InterfaceMode = BSP_OSPI_NOR_OPI_MODE;
    ospi_config.TransferRate  = BSP_OSPI_NOR_DTR_TRANSFER;

    if(BSP_OSPI_NOR_Init(OSPI_INSTANCE, &ospi_config) != BSP_ERROR_NONE)
    {
      return LX_ERROR;
    }

#if (LX_DRIVER_ERASES_OSPI_AFTER_INIT == 1)

    if(BSP_OSPI_NOR_Erase_Chip(OSPI_INSTANCE) != BSP_ERROR_NONE)
    {
      return LX_ERROR;
    }
#endif

#endif

    if(check_status() != LX_SUCCESS)
    {
      return LX_ERROR;
    }

    if(BSP_OSPI_NOR_GetInfo(OSPI_INSTANCE, &ospi_info) != BSP_ERROR_NONE)
    {
      return LX_ERROR;
    }
    
    /* Setup the base address of the flash memory.  */
      nor_flash->lx_nor_flash_base_address = 0;

    /* Setup geometry of the flash.  */
    nor_flash->lx_nor_flash_total_blocks = ospi_info.EraseSectorsNumber;
    nor_flash->lx_nor_flash_words_per_block = ospi_info.EraseSectorSize / sizeof(ULONG);

    nor_flash->lx_nor_flash_driver_read = lx_ospi_driver_read_sector;
    nor_flash->lx_nor_flash_driver_write = lx_ospi_driver_write_sector;

    nor_flash->lx_nor_flash_driver_block_erase = lx_ospi_driver_erase_block;
    nor_flash->lx_nor_flash_driver_block_erased_verify = lx_ospi_driver_block_erased_verify;

    nor_flash->lx_nor_flash_driver_system_error = lx_ospi_driver_system_error;

#ifndef LX_DIRECT_READ
    /* Setup local buffer for NOR flash operation. This buffer must be the sector size of the NOR flash memory.  */
    nor_flash->lx_nor_flash_sector_buffer =  &nor_sector_memory[0];
#endif
    is_initialized = LX_TRUE;

    erase_sector_size = ospi_info.EraseSectorSize;
  }
  /* Return success.  */
  return(LX_SUCCESS);
}

static UINT  lx_ospi_driver_read_sector(ULONG *flash_address, ULONG *destination, ULONG words)
{
  if(BSP_OSPI_NOR_Read(OSPI_INSTANCE, (uint8_t *) destination, (uint32_t) flash_address, (uint32_t) words * sizeof(ULONG)) != BSP_ERROR_NONE)
  {
    return(LX_ERROR);
  }
  else
  {
    return(LX_SUCCESS);
  }
}

static UINT  lx_ospi_driver_write_sector(ULONG *flash_address, ULONG *source, ULONG words)
{
  if(BSP_OSPI_NOR_Write(OSPI_INSTANCE, (uint8_t *) source, (uint32_t) flash_address, (uint32_t) words * sizeof(ULONG)) != BSP_ERROR_NONE)
  {
    return(LX_ERROR);
  }
  else
  {
    return(LX_SUCCESS);
  }
}

static UINT  lx_ospi_driver_erase_block(ULONG block, ULONG erase_count)
{
  BSP_OSPI_NOR_Erase_t erase_block_flag;
  
  LX_PARAMETER_NOT_USED(erase_count);

  erase_block_flag = (erase_sector_size == BSP_OSPI_NOR_BLOCK_64K)? BSP_OSPI_NOR_ERASE_64K : BSP_OSPI_NOR_ERASE_4K;

  if(BSP_OSPI_NOR_Erase_Block(OSPI_INSTANCE, block * erase_sector_size, erase_block_flag) != BSP_ERROR_NONE)
  {
    return(LX_ERROR);
  }
  else
  {
    return check_status();
  }
}

static UINT  lx_ospi_driver_block_erased_verify(ULONG block)
{
   LX_PARAMETER_NOT_USED(block);

   /*
    * Implemnting this API is costy as we need to read the block and verify that it was actually erased.
    * We rely on the HW and just return LX_SUCCESS
    */

    return(LX_SUCCESS);
}

static UINT  lx_ospi_driver_system_error(UINT error_code)
{
  LX_PARAMETER_NOT_USED(error_code);

  return(LX_ERROR);
}

