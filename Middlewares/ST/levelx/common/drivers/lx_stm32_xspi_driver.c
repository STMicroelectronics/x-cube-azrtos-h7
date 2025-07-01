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

#include "lx_stm32_xspi_driver.h"

static UINT  lx_xspi_driver_read_sector(ULONG *flash_address, ULONG *destination, ULONG words);
static UINT  lx_xspi_driver_write_sector(ULONG *flash_address, ULONG *source, ULONG words);

static UINT  lx_xspi_driver_erase_block(ULONG block, ULONG erase_count);
static UINT  lx_xspi_driver_block_erased_verify(ULONG block);

#ifndef LX_DIRECT_READ
extern ULONG  xspi_sector_buffer[LX_NOR_SECTOR_SIZE];
#endif

static UINT is_initialized = LX_FALSE;

/**
* @brief check the status of the octospi memory.
* @param none
* @retval LX_SUCCESS if the octospi is ready, LX_ERROR otherwise
*/

static UINT check_status(void)
{
  ULONG start = LX_STM32_XSPI_CURRENT_TIME();
  while (LX_STM32_XSPI_CURRENT_TIME() - start < LX_STM32_XSPI_DEFAULT_TIMEOUT)
  {
    if (lx_stm32_xspi_get_status(LX_STM32_XSPI_INSTANCE) == 0)
    {
      return LX_SUCCESS;
    }
  }

  return LX_ERROR;
}

/**
* @brief initialize the OctoSPI memory
* @param LX_NOR_FLASH * the levelx NOR flash main instance.
* @retval LX_SUCCESS if the octospi is ready, LX_ERROR otherwise
*/

UINT lx_stm32_xspi_initialize(LX_NOR_FLASH *nor_flash)
{
  INT ret;
  ULONG block_size;
  ULONG total_blocks;

  if (is_initialized == LX_FALSE)
  {

    ret = lx_stm32_xspi_lowlevel_init(LX_STM32_XSPI_INSTANCE);

    if (ret != 0)
    {
      return LX_ERROR;
    }

#if (LX_STM32_XSPI_ERASE == 1)

    ret = lx_stm32_xspi_erase(LX_STM32_XSPI_INSTANCE, (ULONG)0, (ULONG)0, 1);

    if (ret != 0)
    {
      return LX_ERROR;
    }
#endif

    if (check_status() != LX_SUCCESS)
    {
      return LX_ERROR;
    }

    is_initialized = LX_TRUE;
  }

    ret = lx_stm32_xspi_get_info(LX_STM32_XSPI_INSTANCE, &block_size, &total_blocks);

    if (ret != 0)
    {
      return LX_ERROR;
    }

    /* Setup the base address of the flash memory.  */
    nor_flash->lx_nor_flash_base_address = (ULONG*) LX_STM32_XSPI_BASE_ADDRESS;

    /* Setup geometry of the flash.  */
    nor_flash->lx_nor_flash_total_blocks = total_blocks;
    nor_flash->lx_nor_flash_words_per_block = block_size / sizeof(ULONG);

    nor_flash->lx_nor_flash_driver_read = lx_xspi_driver_read_sector;
    nor_flash->lx_nor_flash_driver_write = lx_xspi_driver_write_sector;

    nor_flash->lx_nor_flash_driver_block_erase = lx_xspi_driver_erase_block;
    nor_flash->lx_nor_flash_driver_block_erased_verify = lx_xspi_driver_block_erased_verify;

    nor_flash->lx_nor_flash_driver_system_error = lx_xspi_driver_system_error;

#ifndef LX_DIRECT_READ
    /* Setup local buffer for NOR flash operation. This buffer must be the sector size of the NOR flash memory.  */
    nor_flash->lx_nor_flash_sector_buffer =  &xspi_sector_buffer[0];
#endif

  /* call post init routine*/
  LX_STM32_XSPI_POST_INIT();

  /* Return success.  */
  return LX_SUCCESS;
}

/**
* @brief read data from flash memory address into a destination buffer
* @param ULONG* flash_address the flash address to read from
* @param ULONG* destination the buffer to hold the read data
* @param ULONG words the number of words to read
* @retval LX_SUCCESS if data is read correctly, LX_ERROR on errors
*/


static UINT lx_xspi_driver_read_sector(ULONG *flash_address, ULONG *destination, ULONG words)
{
  UINT status = LX_SUCCESS;

    /* USER CODE BEGIN NOR_READ */
    if (check_status() != LX_SUCCESS)
    {
    return LX_ERROR;
    }

    LX_STM32_XSPI_PRE_READ_TRANSFER(status);

    if (status != LX_SUCCESS)
    {
        return status;
    }

    if (lx_stm32_xspi_read(LX_STM32_XSPI_INSTANCE, flash_address, destination, words) != 0)
    {
        status = LX_ERROR;
        LX_STM32_XSPI_READ_TRANSFER_ERROR(status);
    }
    else
    {
        LX_STM32_XSPI_READ_CPLT_NOTIFY(status);
    }

    LX_STM32_XSPI_POST_READ_TRANSFER(status);
    /* USER CODE END  NOR_READ */

  return status;
}

/**
* @brief write source buffer into flash memory address
* @param ULONG* flash_address the flash address to write into
* @param ULONG* source the data buffer to be written
* @param ULONG words the number of words to write
* @retval LX_SUCCESS if data is written correctly, LX_ERROR on errors
*/

static UINT  lx_xspi_driver_write_sector(ULONG *flash_address, ULONG *source, ULONG words)
{
  UINT status = LX_SUCCESS;

  if (check_status() != LX_SUCCESS)
  {
    return LX_ERROR;
  }

  LX_STM32_XSPI_PRE_WRITE_TRANSFER(status);

  if (status != LX_SUCCESS)
  {
    return status;
  }

  if (lx_stm32_xspi_write(LX_STM32_XSPI_INSTANCE, flash_address, source, words) != 0)
  {
    status = LX_ERROR;
    LX_STM32_XSPI_WRITE_TRANSFER_ERROR(status);
  }
  else
  {
    LX_STM32_XSPI_WRITE_CPLT_NOTIFY(status);
  }

  LX_STM32_XSPI_POST_WRITE_TRANSFER(status);

  return status;
}

static UINT  lx_xspi_driver_erase_block(ULONG block, ULONG erase_count)
{
  UINT status;

  if (check_status() != LX_SUCCESS)
  {
    return LX_ERROR;
  }

  if (lx_stm32_xspi_erase(LX_STM32_XSPI_INSTANCE, block, erase_count, 0) != 0)
  {
    status = LX_ERROR;
  }
  else
  {
    status = check_status();
  }

  return status;
}

static UINT lx_xspi_driver_block_erased_verify(ULONG block)
{
  UINT status;

  if (check_status() != LX_SUCCESS)
  {
    return LX_ERROR;
  }

  if (lx_stm32_xspi_is_block_erased(LX_STM32_XSPI_INSTANCE, block) == 0)
  {
    status = LX_SUCCESS;
  }
  else
  {
    status = LX_ERROR;
  }

  return status;
}

__WEAK UINT lx_xspi_driver_system_error(UINT error_code)
{
  LX_PARAMETER_NOT_USED(error_code);

  return LX_ERROR;
}
