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

/* USER CODE BEGIN SECTOR_BUFFER */
ULONG qspi_sector_buffer[LX_STM32_QSPI_SECTOR_SIZE / sizeof(ULONG)];
/* USER CODE END SECTOR_BUFFER */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
* @brief Initializes the QSPI IP instance
* @param UINT instance QSPI instance to initialize
* @retval 0 on success error value otherwise
*/
INT lx_stm32_qspi_lowlevel_init(UINT instance)
{
  INT status = 0;

  /* USER CODE BEGIN QSPI_Init */
  BSP_QSPI_Init_t qspi_config;

  qspi_config.InterfaceMode = BSP_QSPI_QPI_MODE;
  qspi_config.TransferRate  = BSP_QSPI_DTR_TRANSFER;

  if(BSP_QSPI_Init(LX_STM32_QSPI_INSTANCE, &qspi_config) != BSP_ERROR_NONE)
  {
    status = 1;
  }
  /* USER CODE END QSPI_Init */

  return status;
}

/**
* @brief Get the status of the QSPI instance
* @param UINT instance QSPI instance
* @retval 0 if the QSPI is ready 1 otherwise
*/
INT lx_stm32_qspi_get_status(UINT instance)
{
  INT status = 0;

  /* USER CODE BEGIN QSPI_Get_Status */
  if (BSP_QSPI_GetStatus(LX_STM32_QSPI_INSTANCE) != BSP_ERROR_NONE)
  {
    status = 1;
  }
  /* USER CODE END QSPI_Get_Status */

  return status;
}

/**
* @brief Get size info of the flash meomory
* @param UINT instance QSPI instance
* @param ULONG * block_size pointer to be filled with Flash block size
* @param ULONG * total_blocks pointer to be filled with Flash total number of blocks
* @retval 0 on Success and block_size and total_blocks are correctly filled
          1 on Failure, block_size = 0, total_blocks = 0
*/
INT lx_stm32_qspi_get_info(UINT instance, ULONG *block_size, ULONG *total_blocks)
{
  INT status = 0;

  /* USER CODE BEGIN QSPI_Get_Info */
  BSP_QSPI_Info_t qspi_info;
  if(BSP_QSPI_GetInfo(LX_STM32_QSPI_INSTANCE, &qspi_info) != BSP_ERROR_NONE)
  {
    *block_size = 0;
    *total_blocks = 0;

    status = 1;
  }
  else
  {
    *block_size = qspi_info.EraseSectorSize;
    *total_blocks = qspi_info.FlashSize / qspi_info.EraseSectorSize;
  }

  /* USER CODE END QSPI_Get_Info */

  return status;
}

/**
* @brief Read data from the QSPI memory into a buffer
* @param UINT instance QSPI instance
* @param ULONG * address the start address to read from
* @param ULONG * buffer the destination buffer
* @param ULONG words the total number of words to be read
* @retval 0 on Success 1 on Failure
*/
INT lx_stm32_qspi_read(UINT instance, ULONG *address, ULONG *buffer, ULONG words)
{
  INT status = 0;

  /* USER CODE BEGIN QSPI_Read */
  if(BSP_QSPI_Read(LX_STM32_QSPI_INSTANCE, (uint8_t *)buffer, (uint32_t)address, (uint32_t) words * sizeof(ULONG)) != BSP_ERROR_NONE)
  {
    status = 1;
  }
  /* USER CODE END QSPI_Read */

  return status;
}

/**
* @brief write a data buffer into the QSPI memory
* @param UINT instance QSPI instance
* @param ULONG * address the start address to write into
* @param ULONG * buffer the data source buffer
* @param ULONG words the total number of words to be written
* @retval 0 on Success 1 on Failure
*/
INT lx_stm32_qspi_write(UINT instance, ULONG *address, ULONG *buffer, ULONG words)
{
  INT status = 0;

  /* USER CODE BEGIN QSPI_Write */
  if(BSP_QSPI_Write(LX_STM32_QSPI_INSTANCE, (uint8_t *) buffer, (uint32_t) address, (uint32_t) words * sizeof(ULONG)) != BSP_ERROR_NONE)
  {
    status = 1;
  }
  /* USER CODE END QSPI_Write */

  return status;
}

/**
* @brief Erase the whole flash or a single block
* @param UINT instance QSPI instance
* @param ULONG  block the block to be erased
* @param ULONG  erase_count the number of times the block was erased
* @param UINT full_chip_erase if set to 0 a single block is erased otherwise the whole flash is erased
* @retval 0 on Success 1 on Failure
*/
INT lx_stm32_qspi_erase(UINT instance, ULONG block, ULONG erase_count, UINT full_chip_erase)
{
  INT status = 0;

  /* USER CODE BEGIN QSPI_Erase */
   ULONG start_time = LX_STM32_QSPI_CURRENT_TIME();

  if (full_chip_erase == 0)
  {
    if(BSP_QSPI_EraseBlock(LX_STM32_QSPI_INSTANCE, block * LX_STM32_QSPI_SECTOR_SIZE, BSP_QSPI_ERASE_8K) != BSP_ERROR_NONE)
    {
      return 1;
    }

    status = 1;
    while (LX_STM32_QSPI_CURRENT_TIME() - start_time < MT25TL01G_SECTOR_ERASE_MAX_TIME)
    {
      if (BSP_QSPI_GetStatus(LX_STM32_QSPI_INSTANCE) == BSP_ERROR_NONE)
      {
        status = 0;
        break;
      }
    }
  }
  else
  {

    if(BSP_QSPI_EraseChip(LX_STM32_QSPI_INSTANCE) != BSP_ERROR_NONE)
    {
      return 1;
    }
    status = 1;
    while (LX_STM32_QSPI_CURRENT_TIME() - start_time < MT25TL01G_DIE_ERASE_MAX_TIME)
    {
      if (BSP_QSPI_GetStatus(LX_STM32_QSPI_INSTANCE) == BSP_ERROR_NONE)
      {
        status = 0;
        break;
      }
    }

  }
  /* USER CODE END QSPI_Erase */

  return status;
}

/**
* @brief Check that a block was actually erased
* @param UINT instance QSPI instance
* @param ULONG  block the block to be checked
* @retval 0 on Success 1 on Failure
*/
INT lx_stm32_qspi_is_block_erased(UINT instance, ULONG block)
{
  INT status = 0;

  /* USER CODE BEGIN QSPI_Block_Erased */

  /* USER CODE END QSPI_Block_Erased */

  return status;
}

UINT  lx_qspi_driver_system_error(UINT error_code)
{
  UINT status = LX_ERROR;

  /* USER CODE BEGIN QSPI_System_Error */

  /* USER CODE END QSPI_System_Error */

  return status;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
