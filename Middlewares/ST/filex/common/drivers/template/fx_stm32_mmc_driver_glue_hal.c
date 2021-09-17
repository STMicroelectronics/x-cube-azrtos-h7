#include "fx_stm32_mmc_driver.h"

/* USER CODE BEGIN  0 */

/* the implementation below is assuming that the project is
 * generated with STM32CubeMX, otherwise this file should be modified.
 * this glue implementation supports all the MMC/HAL API variants, but the
 * 'fx_stm32_mmcdriver.h' should be adjusted depending on the configuration
 * in this file.
 */

/* Select the HAL API to use  be enabling the correct define */

#define USE_MMC_HAL_DMA_API

/* Define the sync object only when the Polling API is not used */
#ifndef USE_MMC_HAL_POLLING_API

#ifndef FX_STANDALONE_ENABLE
TX_SEMAPHORE transfer_semaphore;
#else
__IO UINT read_transfer_completed;
__IO UINT write_transfer_completed;
#endif

#endif /* USE_MMC_HAL_POLLING_API */

extern MMC_HandleTypeDef hmmc1;
extern void  MX_MMCMMC1_MMC_Init(void);
/* USER CODE END  0 */

/**
* @brief Initializes the MMC IP instance
* @param uINT Instance MMC instance to initialize
* @retval 0 on success error value otherwise
*/
INT fx_stm32_mmc_init(UINT instance)
{
  INT ret = 0;
/* USER CODE BEGIN  FX_MMC_INIT */
#if (FX_STM32_MMC_INIT == 1)
      MX_MMCMMC1_MMC_Init();
#endif
/* USER CODE END  FX_MMC_INIT */

  return ret;
}

/**
* @brief Deinitializes the MMC IP instance
* @param uINT Instance MMC instance to deinitialize
* @retval 0 on success error value otherwise
*/
INT fx_stm32_mmc_deinit(UINT instance)
{
  INT ret = 0;
/* USER CODE BEGIN  FX_MMC_DEINIT */

/* USER CODE END  FX_MMC_DEINIT */

  return ret;
}

/**
* @brief Check the MMC IP status.
* @param uINT Instance MMC instance to check
* @retval 0 when ready 1 when busy
*/
INT fx_stm32_mmc_get_status(UINT instance)
{
  INT ret = 0;
/* USER CODE BEGIN  GET_STATUS */
  if (HAL_MMC_GetCardState(&hmmc1) != HAL_MMC_CARD_TRANSFER)
  {
    ret = HAL_ERROR;
  }
  else
  {
    ret = HAL_OK;
  }
/* USER CODE END  GET_STATUS */
  return ret;
}

/**
* @brief Read Data from the MMC device into a buffer.
* @param uINT *Buffer buffer into which the data is to be read.
* @param uINT StartBlock the first block to start reading from.
* @param uINT NbrOfBlocks total number of blocks to read.
* @retval 0 on success error code otherwise
*/
INT fx_stm32_mmc_read_blocks(UINT instance, UINT *buffer, UINT start_block, UINT total_blocks)
{
  INT ret = 0;
/* USER CODE BEGIN  READ_BLOCKS */
/* #define USE_MMC_HAL_POLLING_API */
 /* #define USE_MMC_HAL_IT_API */

#ifdef USE_MMC_HAL_DMA_API
  if (HAL_MMC_ReadBlocks_DMA(&hmmc1, (uint8_t *)buffer, start_block, total_blocks) != HAL_OK)
#endif

#ifdef USE_MMC_HAL_IT_API
  if (HAL_MMC_ReadBlocks_IT(&hmmc1, (uint8_t *)buffer, start_block, total_blocks) != HAL_OK)
#endif

#ifdef USE_MMC_HAL_POLLING_API
  if (HAL_MMC_ReadBlocks(&hmmc1, (uint8_t *)buffer, start_block, total_blocks, FX_STM32_MMC_DEFAULT_TIMEOUT) != HAL_OK)
#endif
  {
    ret = HAL_ERROR;
  }
  else
  {
    ret = HAL_OK;
  }
/* USER CODE END  READ_BLOCKS */
  return ret;
}
/**
* @brief Write data buffer into the MMC device.
* @param uINT *Buffer buffer .to write into the MMC device.
* @param uINT StartBlock the first block to start writing from.
* @param uINT NbrOfBlocks total number of blocks to write.
* @retval 0 on success error code otherwise
*/

INT fx_stm32_mmc_write_blocks(UINT instance, UINT *buffer, UINT start_block, UINT total_blocks)
{
  INT ret = 0;
/* USER CODE BEGIN  WRITE_BLOCKS */
#ifdef USE_MMC_HAL_DMA_API
  if (HAL_MMC_WriteBlocks_DMA(&hmmc1, (uint8_t *)buffer, start_block, total_blocks) != HAL_OK)
#endif

#ifdef USE_MMC_HAL_IT_API
  if (HAL_MMC_WriteBlocks_IT(&hmmc1, (uint8_t *)buffer, start_block, total_blocks) != HAL_OK)
#endif

#ifdef USE_MMC_HAL_POLLING_API
  if (HAL_MMC_WriteBlocks(&hmmc1, (uint8_t *)buffer, start_block, total_blocks, FX_STM32_MMC_DEFAULT_TIMEOUT) != HAL_OK)
#endif
  {
    ret = HAL_ERROR;
  }
  else
  {
    ret = HAL_OK;
  }
/* USER CODE END  WRITE_BLOCKS */
  return ret;

}


/* USER CODE BEGIN  1 */

#ifndef USE_MMC_HAL_POLLING_API
/**
* @brief MMC DMA Tx Transfer completed callbacks
* @param Instance the sd instance
* @retval None
*/
void HAL_MMC_TxCpltCallback(MMC_HandleTypeDef *hmmc)
{
#ifndef FX_STANDALONE_ENABLE
  tx_semaphore_put(&transfer_semaphore);
#else
  write_transfer_completed = 1;
#endif
}

/**
* @brief MMC DMA Rx Transfer completed callbacks
* @param Instance the sd instance
* @retval None
*/

void HAL_MMC_RxCpltCallback(MMC_HandleTypeDef *hmmc)
{
#ifndef FX_STANDALONE_ENABLE
  tx_semaphore_put(&transfer_semaphore);
#else
  read_transfer_completed = 1;
#endif
}

#endif
/* USER CODE END  1 */
