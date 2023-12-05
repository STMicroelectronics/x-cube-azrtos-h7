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


/* Includes ------------------------------------------------------------------*/
#include "lx_stm32_nand_custom_driver.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/




static UINT  lx_nand_driver_read(ULONG block, ULONG page, ULONG *destination, ULONG words);
static UINT  lx_nand_driver_write(ULONG block, ULONG page, ULONG *source, ULONG words);

static UINT  lx_nand_driver_block_erase(ULONG block, ULONG erase_count);
static UINT  lx_nand_driver_block_erased_verify(ULONG block);
static UINT  lx_nand_driver_page_erased_verify(ULONG block, ULONG page);

static UINT  lx_nand_driver_block_status_get(ULONG block, UCHAR *bad_block_byte);
static UINT  lx_nand_driver_block_status_set(ULONG block, UCHAR bad_block_byte);

static UINT  lx_nand_driver_extra_bytes_get(ULONG block, ULONG page, UCHAR *destination, UINT size);
static UINT  lx_nand_driver_extra_bytes_set(ULONG block, ULONG page, UCHAR *source, UINT size);

static UINT  lx_nand_driver_system_error(UINT error_code, ULONG block, ULONG page);

static UINT  lx_nand_flash_driver_pages_read(ULONG block, ULONG page, UCHAR *main_buffer, UCHAR *spare_buffer, ULONG pages);
static UINT  lx_nand_flash_driver_pages_write(ULONG block, ULONG page, UCHAR *main_buffer, UCHAR *spare_buffer, ULONG pages);
static UINT  lx_nand_flash_driver_pages_copy(ULONG source_block, ULONG source_page, ULONG destination_block, ULONG destination_page, ULONG pages, UCHAR *data_buffer);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

#ifndef WORDS_PER_PHYSICAL_PAGE
#define WORDS_PER_PHYSICAL_PAGE 512
#endif

static UCHAR  nand_flash_buffer[WORDS_PER_PHYSICAL_PAGE];

UINT lx_stm32_nand_custom_driver_initialize(LX_NAND_FLASH *nand_flash)
{
  UINT ret = LX_SUCCESS;

  /* USER CODE BEGIN Init_Section_0 */

  /*USER CODE END Init_Section_0 */

  nand_flash->lx_nand_flash_total_blocks =                  TOTAL_BLOCKS;
  nand_flash->lx_nand_flash_pages_per_block =               PHYSICAL_PAGES_PER_BLOCK;
  nand_flash->lx_nand_flash_bytes_per_page =                BYTES_PER_PHYSICAL_PAGE;

  nand_flash -> lx_nand_flash_spare_data1_offset =          SPARE_DATA1_OFFSET;
  nand_flash -> lx_nand_flash_spare_data1_length =          SPARE_DATA1_LENGTH;

  nand_flash -> lx_nand_flash_spare_data2_offset =          SPARE_DATA2_OFFSET;
  nand_flash -> lx_nand_flash_spare_data2_length =          SPARE_DATA2_LENGTH;

  nand_flash -> lx_nand_flash_spare_total_length =          SPARE_BYTES_PER_PAGE;


  /* USER CODE BEGIN Init_Section_1 */

  /*USER CODE END Init_Section_1 */

  nand_flash->lx_nand_flash_driver_read =                   lx_nand_driver_read;
  nand_flash->lx_nand_flash_driver_write =                  lx_nand_driver_write;

  nand_flash->lx_nand_flash_driver_block_erase =            lx_nand_driver_block_erase;
  nand_flash->lx_nand_flash_driver_block_erased_verify =    lx_nand_driver_block_erased_verify;
  nand_flash->lx_nand_flash_driver_page_erased_verify =     lx_nand_driver_page_erased_verify;

  nand_flash->lx_nand_flash_driver_block_status_get =       lx_nand_driver_block_status_get;
  nand_flash->lx_nand_flash_driver_block_status_set =       lx_nand_driver_block_status_set;

  nand_flash->lx_nand_flash_driver_extra_bytes_get =        lx_nand_driver_extra_bytes_get;
  nand_flash->lx_nand_flash_driver_extra_bytes_set =        lx_nand_driver_extra_bytes_set;

  nand_flash->lx_nand_flash_driver_system_error =           lx_nand_driver_system_error;

  nand_flash->lx_nand_flash_driver_pages_read =             lx_nand_flash_driver_pages_read;
  nand_flash->lx_nand_flash_driver_pages_write =            lx_nand_flash_driver_pages_write;
  nand_flash->lx_nand_flash_driver_pages_copy =             lx_nand_flash_driver_pages_copy;

  /* USER CODE BEGIN Init_Section_2 */

  /*USER CODE END Init_Section_2 */

  nand_flash->lx_nand_flash_page_buffer =  &nand_flash_buffer[0];

  /* USER CODE BEGIN Init_Section_3 */

  /*USER CODE END Init_Section_3 */
  return ret;

}

static UINT  lx_nand_driver_read(ULONG block, ULONG page, ULONG *destination, ULONG words)
{

  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN driver_read */

 /* USER CODE END driver_read */

  return ret;
}

static UINT  lx_nand_driver_write(ULONG block, ULONG page, ULONG *source, ULONG words)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN driver_write */

 /* USER CODE END driver_write */

  return ret;
}

static UINT  lx_nand_driver_block_erase(ULONG block, ULONG erase_count)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN block_erase */

 /* USER CODE END block_erase */

  return ret;
}

static UINT  lx_nand_driver_block_erased_verify(ULONG block)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN block_erase_verify */

 /* USER CODE END block_erase_verify */

  return ret;
}

static UINT  lx_nand_driver_page_erased_verify(ULONG block, ULONG page)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN page_erased_verify */

 /* USER CODE END page_erased_verify */

  return ret;
}

static UINT  lx_nand_driver_block_status_get(ULONG block, UCHAR *bad_block_byte)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN block_status_get */

 /* USER CODE END block_status_get*/

  return ret;
}

static UINT  lx_nand_driver_block_status_set(ULONG block, UCHAR bad_block_byte)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN block_status_set */

 /* USER CODE END block_status_set */

  return ret;
}

static UINT  lx_nand_driver_extra_bytes_get(ULONG block, ULONG page, UCHAR *destination, UINT size)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN extra_bytes_get */

 /* USER CODE END extra_bytes_get */

  return ret;
}

static UINT  lx_nand_driver_extra_bytes_set(ULONG block, ULONG page, UCHAR *source, UINT size)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN extra_bytes_set */

 /* USER CODE END extra_bytes_set */

  return ret;
}

static UINT  lx_nand_driver_system_error(UINT error_code, ULONG block, ULONG page)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN system_error */

 /* USER CODE END system_error */

  return ret;
}

static UINT  lx_nand_flash_driver_pages_read(ULONG block, ULONG page, UCHAR *main_buffer, UCHAR *spare_buffer, ULONG pages)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN driver_pages_read */

 /* USER CODE END driver_pages_read */

  return ret;
}

static UINT  lx_nand_flash_driver_pages_write(ULONG block, ULONG page, UCHAR *main_buffer, UCHAR *spare_buffer, ULONG pages)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN driver_pages_write */

 /* USER CODE END driver_pages_write */

  return ret;
}

static UINT  lx_nand_flash_driver_pages_copy(ULONG source_block, ULONG source_page, ULONG destination_block, ULONG destination_page, ULONG pages, UCHAR *data_buffer)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN driver_pages_copy */

 /* USER CODE END driver_pages_copy */

  return ret;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
