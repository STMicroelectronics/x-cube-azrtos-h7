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

/* HAL DMA mode based implementation for QuadSPI component MT25TL01G
 * The present implementation assumes the following settings are set for single bank:

  QuadSPI Mode: Bank1 with QuadSPI Lines
  ClockPrescaler = 3;
  FifoThreshold = 1;
  SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;
  FlashSize = 26;
  ChipSelectHighTime = QSPI_CS_HIGH_TIME_4_CYCLE;
  ClockMode = QSPI_CLOCK_MODE_0;
  FlashID = QSPI_FLASH_ID_1;
  DualFlash = QSPI_DUALFLASH_ENABLE;

 * Different configuration can be used but need to be reflected in
 * the implementation guarded with QSPI_HAL_CFG_xxx user tags.
 */

/* The following semaphores are being to notify about RX/TX completion. It needs to be released in the transfer callbacks */
TX_SEMAPHORE qspi_tx_semaphore;
TX_SEMAPHORE qspi_rx_semaphore;

/* USER CODE BEGIN DUAL_BANK_FLAG */
/* USER CODE BEGIN DUAL_BANK_FLAG */

#define QSPI_WRITE_ENABLE_SPI_MODE            0
#define QSPI_WRITE_ENABLE_QPI_MODE            1

static uint8_t qspi_memory_reset              (QSPI_HandleTypeDef *quadspi_handle);

static uint8_t qspi_set_write_enable          (QSPI_HandleTypeDef *quadspi_handle, uint8_t mode);
static uint8_t qspi_auto_polling_ready        (QSPI_HandleTypeDef *quadspi_handle, uint32_t timeout);

static uint8_t qspi_dummy_cycles_configure    (QSPI_HandleTypeDef *quadspi_handle);

static uint8_t qspi_enter_qpi_mode            (QSPI_HandleTypeDef *quadspi_handle);
static uint8_t qspi_enter_4bytes_address_mode (QSPI_HandleTypeDef *quadspi_handle);

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
  /* USER CODE BEGIN PRE_QSPI_Init */
  UNUSED(instance);
  /* USER CODE END PRE_QSPI_Init */

#if (LX_STM32_QSPI_INIT == 1)
  qspi_handle.Instance = QUADSPI;
  if (HAL_QSPI_DeInit(&qspi_handle) != HAL_OK)
  {
    return 1;
  }

  /* Init the QSPI */
  qspi_driver_init();
#endif

  /* QSPI memory reset */
  if (qspi_memory_reset(&qspi_handle) != 0)
  {
    return 1;
  }

  /* USER CODE BEGIN POST_QSPI_Init */
  /* Configure automatic polling mode to wait the memory is ready */
  if (qspi_auto_polling_ready(&hqspi, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != 0)
  {
    return 1;
  }

  /* Force Flash enter 4 Byte address mode */
  if (qspi_enter_4bytes_address_mode(&hqspi) != 0)
  {
    return 1;
  }

  /* Configuration of the dummy cycles on QSPI memory side */
  if (qspi_dummy_cycles_configure(&hqspi) != 0)
  {
    return 1;
  }

  if (qspi_enter_qpi_mode(&hqspi) != 0)
  {
    return 1;
  }
  /* USER CODE END POST_QSPI_Init */

  return 0;
}

/**
  * @brief  De-Initializes the QSPI interface.
  * @param  Instance QSPI Instance
  * @retval  0 on Success 1 on Failure
  */
INT lx_stm32_qspi_lowlevel_deinit(UINT instance)
{
  /* USER CODE BEGIN PRE_QSPI_DeInit */
  UNUSED(instance);
  /* USER CODE END PRE_QSPI_DeInit */

#if (LX_STM32_QSPI_INIT == 1)
  /* Call the DeInit function to reset the driver */
  if (HAL_QSPI_DeInit(&qspi_handle) != HAL_OK)
  {
    return 1;
  }
#endif
  /* USER CODE BEGIN POST_QSPI_DeInit */

  /* USER CODE END POST_QSPI_DeInit */

  return 0;
}

/**
* @brief Get the status of the QSPI instance
* @param UINT instance QSPI instance
* @retval 0 if the QSPI is ready 1 otherwise
*/
INT lx_stm32_qspi_get_status(UINT instance)
{
  /* USER CODE BEGIN PRE_QSPI_Get_Status */
  UNUSED(instance);
  /* USER CODE END PRE_QSPI_Get_Status */

  QSPI_CommandTypeDef s_command;
  uint8_t reg;
  /* Initialize the read flag status register command */
  /* USER CODE BEGIN QSPI_HAL_CFG_GetStatus */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = LX_STM32_QSPI_READ_STATUS_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  /* USER CODE END QSPI_HAL_CFG_GetStatus */

  /* Configure the command */
  if (HAL_QSPI_Command(&qspi_handle, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Receive the data */
  if (HAL_QSPI_Receive(&qspi_handle, &reg, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }
  /* Check the value of the register */
  if ((reg & LX_STM32_QSPI_SR_WIP) != 0U)
  {
    return 1;
  }

  /* USER CODE BEGIN POST_QSPI_Get_Status */

  /* USER CODE END POST_QSPI_Get_Status */

  return 0;
}

/**
* @brief Get size info of the flash memory
* @param UINT instance QSPI instance
* @param ULONG * block_size pointer to be filled with Flash block size
* @param ULONG * total_blocks pointer to be filled with Flash total number of blocks
* @retval 0 on Success and block_size and total_blocks are correctly filled
          1 on Failure, block_size = 0, total_blocks = 0
*/
INT lx_stm32_qspi_get_info(UINT instance, ULONG *block_size, ULONG *total_blocks)
{
  /* USER CODE BEGIN PRE_QSPI_Get_Info */
  UNUSED(instance);
  /* USER CODE END PRE_QSPI_Get_Info */

  *block_size = LX_STM32_QSPI_SECTOR_SIZE;
  *total_blocks = (LX_STM32_QSPI_FLASH_SIZE / LX_STM32_QSPI_SECTOR_SIZE);

  /* USER CODE BEGIN POST_QSPI_Get_Info */

  /* USER CODE END POST_QSPI_Get_Info */

  return 0;
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
  /* USER CODE BEGIN PRE_QSPI_Read */
  UNUSED(instance);
  /* USER CODE END PRE_QSPI_Read */

  QSPI_CommandTypeDef s_command;

  /* Initialize the read command */
  /* USER CODE BEGIN QSPI_HAL_CFG_READ_1 */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = LX_STM32_QSPI_QUAD_INOUT_FAST_READ_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
  s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
  s_command.Address           = (uint32_t)address;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = LX_STM32_QSPI_DUMMY_CYCLES_READ_QUAD;
  s_command.NbData            = (uint32_t) words * sizeof(ULONG);
  s_command.DdrMode           = QSPI_DDR_MODE_ENABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_HALF_CLK_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  /* USER CODE END QSPI_HAL_CFG_READ_1 */

  /* Configure the command */
  if (HAL_QSPI_Command(&qspi_handle, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive_DMA(&qspi_handle, (uint8_t *)buffer) != HAL_OK)
  {
    return 1;
  }

  /* USER CODE BEGIN POST_QSPI_Read */

  /* USER CODE END POST_QSPI_Read */

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
  /* USER CODE BEGIN PRE_QSPI_Write */
  UNUSED(instance);
  /* USER CODE END PRE_QSPI_Write */

  QSPI_CommandTypeDef s_command;
  uint32_t end_addr, current_size, current_addr;
  uint8_t* data_buffer = (uint8_t *) buffer;

  /* Calculation of the size between the write address and the end of the page */
  current_size = LX_STM32_QSPI_PAGE_SIZE - ((uint32_t) address % LX_STM32_QSPI_PAGE_SIZE);

  /* Check if the size of the data is less than the remaining place in the page */
  if (current_size > ((uint32_t) words * sizeof(ULONG)))
  {
    current_size = (uint32_t) words * sizeof(ULONG);
  }

  /* Initialize the address variables */
  current_addr = (uint32_t) address;
  end_addr = (uint32_t) address + (uint32_t) words * sizeof(ULONG);

  /* Initialize the program command */
  /* USER CODE BEGIN QSPI_HAL_CFG_WRITE */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = LX_STM32_QSPI_QUAD_IN_FAST_PROG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
  s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  /* USER CODE END QSPI_HAL_CFG_WRITE */

  /* Perform the write page by page */
  do
  {
    s_command.Address = current_addr;
    s_command.NbData  = current_size;

    /* Enable write operations */
    if (qspi_set_write_enable(&qspi_handle, QSPI_WRITE_ENABLE_QPI_MODE) != 0)
    {
      return 1;
    }

    /* Configure the command */
    if (HAL_QSPI_Command(&qspi_handle, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return 1;
    }

    /* Transmission of the data */
    if (HAL_QSPI_Transmit_DMA(&qspi_handle, (uint8_t*)data_buffer) != HAL_OK)
    {
      return 1;
    }

    /* Check the status of the transmission of the data */
    if(tx_semaphore_get(&qspi_tx_semaphore, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != TX_SUCCESS)
    {
      return 1;
    }

    /* Configure automatic polling mode to wait for end of program */
    if (qspi_auto_polling_ready(&qspi_handle, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != 0)
    {
      return 1;
    }

    /* Update the address and size variables for next page programming */
    current_addr += current_size;
    data_buffer += current_size;
    current_size = ((current_addr + LX_STM32_QSPI_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : LX_STM32_QSPI_PAGE_SIZE;
  } while (current_addr < end_addr);

  /* Release qspi_tx_semaphore in case of writing success */
  if(tx_semaphore_put(&qspi_tx_semaphore) != TX_SUCCESS)
  {
    return 1;
  }

  /* USER CODE BEGIN POST_QSPI_Write */

  /* USER CODE END POST_QSPI_Write */

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
  /* USER CODE BEGIN PRE_QSPI_Erase */
  UNUSED(instance);
  /* USER CODE END PRE_QSPI_Erase */

  QSPI_CommandTypeDef s_command;
  uint32_t erase_timeout;

  /* Initialize the erase command */
  /* USER CODE BEGIN QSPI_HAL_CFG_ERASE */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if(full_chip_erase)
  {
    s_command.Instruction       = LX_STM32_QSPI_DIE_ERASE_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    erase_timeout               = LX_STM32_QSPI_DIE_ERASE_MAX_TIME;
  }
  else
  {
    s_command.Instruction       = LX_STM32_QSPI_SECTOR_ERASE_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
    s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
    s_command.Address           = block * LX_STM32_QSPI_SECTOR_SIZE;
    erase_timeout               = LX_STM32_QSPI_SECTOR_ERASE_MAX_TIME;
  }
  /* USER CODE END QSPI_HAL_CFG_ERASE */

  /* Enable write operations */
  if (qspi_set_write_enable(&qspi_handle, QSPI_WRITE_ENABLE_QPI_MODE) != 0)
  {
    return 1;
  }

  /* Send the command */
  if (HAL_QSPI_Command(&qspi_handle, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Configure automatic polling mode to wait for end of erase */
  if (qspi_auto_polling_ready(&qspi_handle, erase_timeout) != 0)
  {
    return 1;
  }

  /* USER CODE BEGIN POST_QSPI_Erase */

  /* USER CODE END POST_QSPI_Erase */

  return 0;
}

/**
* @brief Check that a block was actually erased
* @param UINT instance QSPI instance
* @param ULONG  block the block to be checked
* @retval 0 on Success 1 on Failure
*/
INT lx_stm32_qspi_is_block_erased(UINT instance, ULONG block)
{

  /* USER CODE BEGIN QSPI_Block_Erased */

  /* USER CODE END QSPI_Block_Erased */

  return 0;
}

/**
* @brief Return QSPI driver system error code
* @param UINT error_code the QSPI driver system error code
* @retval error_code on Failure and 0 on Success
*/
UINT  lx_qspi_driver_system_error(UINT error_code)
{
  UINT status = LX_ERROR;

  /* USER CODE BEGIN QSPI_System_Error */

  /* USER CODE END QSPI_System_Error */

  return status;
}

/**
  * @brief  Reset the QSPI memory.
  * @param  quadspi_handle: QSPI Instance
  * @retval  0 on Success 1 on Failure
  */
static uint8_t qspi_memory_reset(QSPI_HandleTypeDef *quadspi_handle)
{
  QSPI_CommandTypeDef      s_command;

  /* Initialize the reset enable command for qpi mode */
  /* USER CODE BEGIN QSPI_HAL_CFG_MEMORY_RESET_ENABLE_QPI */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = LX_STM32_QSPI_RESET_ENABLE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  /* USER CODE END QSPI_HAL_CFG_MEMORY_RESET_ENABLE_QPI */

  /* Send the command */
  if (HAL_QSPI_Command(quadspi_handle, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Initialize the reset command for the qpi mode*/
  /* USER CODE BEGIN QSPI_HAL_CFG_MEMORY_RESET_QPI */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = LX_STM32_QSPI_RESET_MEMORY_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  /* USER CODE END QSPI_HAL_CFG_MEMORY_RESET_QPI */

  /* Send the command */
  if (HAL_QSPI_Command(quadspi_handle, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Configure automatic polling mode to wait for end of reset memory */
  if (qspi_auto_polling_ready(quadspi_handle, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != 0)
  {
    return 1;
  }

  /* Initialize the reset command for the qpi mode*/
  /* USER CODE BEGIN QSPI_HAL_CFG_MEMORY_RESET_ENABLE_SPI */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = LX_STM32_QSPI_RESET_ENABLE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  /* USER CODE END QSPI_HAL_CFG_MEMORY_RESET_ENABLE_SPI */

  /* Send the command */
  if (HAL_QSPI_Command(quadspi_handle, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }
  /* Initialize the reset command for spi mode*/
  /* USER CODE BEGIN QSPI_HAL_CFG_MEMORY_RESET_SPI */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = LX_STM32_QSPI_RESET_MEMORY_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  /* USER CODE END QSPI_HAL_CFG_MEMORY_RESET_SPI */
  /* Send the command */
  if (HAL_QSPI_Command(quadspi_handle, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  return 0;
}

/**
  * @brief  Send a Write Enable and wait it is effective.
  * @param  quadspi_handle: QSPI handle
  * @retval 0 on Success 1 on Failure
  */
static uint8_t qspi_set_write_enable(QSPI_HandleTypeDef *quadspi_handle, uint8_t mode)
{
  QSPI_CommandTypeDef     s_command;
  QSPI_AutoPollingTypeDef s_config;

  /* Enable write operations */
  /* USER CODE BEGIN QSPI_HAL_CFG_WRITE_ENABLE_1 */
  s_command.InstructionMode   = (mode == QSPI_WRITE_ENABLE_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = LX_STM32_QSPI_WRITE_ENABLE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  /* USER CODE END QSPI_HAL_CFG_WRITE_ENABLE_1 */

  if (HAL_QSPI_Command(quadspi_handle, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Configure automatic polling mode to wait for write enabling */
  /* USER CODE BEGIN QSPI_HAL_CFG_WRITE_ENABLE_2 */
  s_config.Match           = LX_STM32_QSPI_SR_WREN | (LX_STM32_QSPI_SR_WREN << 8);
  s_config.Mask            = LX_STM32_QSPI_SR_WREN | (LX_STM32_QSPI_SR_WREN << 8);
  s_config.MatchMode       = QSPI_MATCH_MODE_AND;
  s_config.StatusBytesSize = 2;
  s_config.Interval        = 0x10;
  s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  s_command.Instruction    = LX_STM32_QSPI_READ_STATUS_REG_CMD;
  s_command.DataMode       = (mode == QSPI_WRITE_ENABLE_QPI_MODE) ? QSPI_DATA_4_LINES : QSPI_DATA_1_LINE;
  /* USER CODE END QSPI_HAL_CFG_WRITE_ENABLE_2 */

  if (HAL_QSPI_AutoPolling(quadspi_handle, &s_command, &s_config, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  return 0;
}

/**
  * @brief  Read the SR of the memory and wait the EOP.
  * @param  quadspi_handle: QSPI handle pointer
  * @param  timeout: timeout value before returning an error
  * @retval 0 on Success 1 on Failure
  */
static uint8_t qspi_auto_polling_ready(QSPI_HandleTypeDef *quadspi_handle, uint32_t timeout)
{
  QSPI_CommandTypeDef     s_command;
  QSPI_AutoPollingTypeDef s_config;

  /* Configure automatic polling mode to wait for memory ready */
  /* USER CODE BEGIN QSPI_HAL_CFG_MEM_READY */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = LX_STM32_QSPI_READ_STATUS_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = 2;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  s_config.Match           = 0;
  s_config.Mask            = LX_STM32_QSPI_SR_WIP | (LX_STM32_QSPI_SR_WIP << 8);
  s_config.MatchMode       = QSPI_MATCH_MODE_AND;
  s_config.StatusBytesSize = 2;
  s_config.Interval        = 0x10;
  s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;
  /* USER CODE END QSPI_HAL_CFG_MEM_READY */

  if (HAL_QSPI_AutoPolling(quadspi_handle, &s_command, &s_config, timeout) != HAL_OK)
  {
    return 1;
  }

  return 0;
}

/**
  * @brief  This function set the QSPI memory in 4-byte address mode
  *          SPI/QPI; 1-0-1/4-0-4
  * @retval 0 on Success 1 on Failure
  */
static uint8_t qspi_enter_4bytes_address_mode(QSPI_HandleTypeDef *quadspi_handle)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = LX_STM32_QSPI_ENTER_4_BYTE_ADDR_MODE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /*write enable */
  if(qspi_set_write_enable(quadspi_handle, QSPI_WRITE_ENABLE_SPI_MODE) != 0)
  {
    return 1;
  }

  /* Send the command */
  if (HAL_QSPI_Command(quadspi_handle, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Configure automatic polling mode to wait the memory is ready */
  if (qspi_auto_polling_ready(quadspi_handle, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != 0)
  {
    return 1;
  }

  return 0;
}

/**
  * @brief  This function configure the dummy cycles on memory side.
  *         Dummy cycle bit locate in Configuration Register[7:6]
  * @param  quadspi_handle: QSPI Instance
  * @retval BSP status
  */
static uint8_t qspi_dummy_cycles_configure(QSPI_HandleTypeDef *quadspi_handle)
{
  QSPI_CommandTypeDef s_command;
  uint8_t reg[2];

  /* Initialize the read volatile configuration register command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = LX_STM32_QSPI_READ_VOL_CFG_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.NbData            = 2;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(quadspi_handle, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(quadspi_handle, &(reg[1]), HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /*write enable */
  if(qspi_set_write_enable(quadspi_handle, QSPI_WRITE_ENABLE_SPI_MODE) != 0)
  {
    return 1;
  }

  /* Update volatile configuration register (with new dummy cycles) */
  s_command.Instruction = LX_STM32_QSPI_WRITE_VOL_CFG_REG_CMD;
  MODIFY_REG(reg[1], 0xF0F0, ((LX_STM32_QSPI_DUMMY_CYCLES_READ_QUAD_STR << 4) |
                               (LX_STM32_QSPI_DUMMY_CYCLES_READ_QUAD_STR << 12)));
  /* Configure the write volatile configuration register command */
  if (HAL_QSPI_Command(quadspi_handle, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Transmission of the data */
  if (HAL_QSPI_Transmit(quadspi_handle, &(reg[0]), HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Return status */
  return 0;
}

/**
  * @brief  This function put QSPI memory in QPI mode (Quad I/O) from SPI mode.
  *         SPI -> QPI; 1-x-x -> 4-4-4
  *         SPI; 1-0-0
  * @param  quadspi_handle: QSPI Instance
  * @retval 0 on Success 1 on Failure
  */
static uint8_t qspi_enter_qpi_mode(QSPI_HandleTypeDef *quadspi_handle)
{
  QSPI_CommandTypeDef s_command;

  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = LX_STM32_QSPI_ENTER_QUAD_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(quadspi_handle, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  return 0;
}

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  handle_qspi: QSPI handle
  * @retval None
  */
void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *handle_qspi)
{
  /* USER CODE BEGIN PRE_RX_CMPLT */

  /* USER CODE END PRE_RX_CMPLT */

  tx_semaphore_put(&qspi_rx_semaphore);

  /* USER CODE BEGIN POST_RX_CMPLT */

  /* USER CODE END POST_RX_CMPLT */
}

/**
  * @brief  Tx Transfer completed callbacks.
  * @param  handle_qspi: QSPI handle
  * @retval None
  */
void HAL_QSPI_TxCpltCallback(QSPI_HandleTypeDef *handle_qspi)
{
  /* USER CODE BEGIN PRE_TX_CMPLT */

  /* USER CODE END PRE_TX_CMPLT */

  tx_semaphore_put(&qspi_tx_semaphore);

  /* USER CODE BEGIN POST_TX_CMPLT */

  /* USER CODE END POST_TX_CMPLT */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
