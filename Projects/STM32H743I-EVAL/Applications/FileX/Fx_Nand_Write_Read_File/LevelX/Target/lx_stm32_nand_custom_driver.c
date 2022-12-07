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

#include "lx_stm32_nand_custom_driver.h"

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

/* USER CODE BEGIN 0 */
UINT  _lx_nand_flash_erase_all_driver(VOID);

static uint8_t flash_is_initialized = 0;
static ULONG  nand_flash_rw_buffer[WORDS_PER_PHYSICAL_PAGE] = {0};
static UCHAR  Buffer_Spare_Area[SPARE_BYTES_PER_PAGE] = {0};

/* USER CODE END 0 */

#ifndef WORDS_PER_PHYSICAL_PAGE
#define WORDS_PER_PHYSICAL_PAGE 512
#endif

ULONG  nand_flash_buffer[WORDS_PER_PHYSICAL_PAGE];

UINT lx_stm32_nand_custom_driver_initialize(LX_NAND_FLASH *nand_flash)
{
  UINT ret = LX_SUCCESS;

  ULONG total_blocks = 0;
  ULONG pages_per_block = 0;
  ULONG bytes_per_page = 0;

  /* USER CODE BEGIN Init_Section_0 */
  NAND_IDTypeDef pNAND_ID;

  if (!flash_is_initialized)
  {
    /* Initialize FMC interface */
    if (BSP_FMC_NAND_Init(NAND_INSTANCE) != BSP_ERROR_NONE)
    {
      return (LX_ERROR);
    }

    /*Read & check the NAND device IDs*/
    /* Initialize the ID structure */
    pNAND_ID.Maker_Id = 0x00;
    pNAND_ID.Device_Id = 0x00;
    pNAND_ID.Third_Id = 0x00;
    pNAND_ID.Fourth_Id = 0x00;

    /* Read the NAND memory ID */
    BSP_FMC_NAND_ReadID(NAND_INSTANCE, &pNAND_ID);

    /* Test the NAND ID correctness */
    if (pNAND_ID.Maker_Id  != BSP_NAND_MANUFACTURER_CODE)
    {
      return (LX_ERROR);
    }
    else if (pNAND_ID.Third_Id != BSP_NAND_THIRD_ID)
    {
      return (LX_ERROR);
    }
    else if (pNAND_ID.Fourth_Id != BSP_NAND_FOURTH_ID)
    {
      return (LX_ERROR);
    }

    /* If ERASE_CHIP enabled erase the NAND device */
    if (LX_DRIVER_ERASES_FLASH_AFTER_INIT)
    {
      if (_lx_nand_flash_erase_all_driver() != LX_SUCCESS)
      {
        return (LX_ERROR);
      }
    }
    flash_is_initialized = 1;
  }

  total_blocks = TOTAL_BLOCKS;
  pages_per_block = PHYSICAL_PAGES_PER_BLOCK;
  bytes_per_page = BYTES_PER_PHYSICAL_PAGE;
  /*USER CODE END Init_Section_0 */

  nand_flash->lx_nand_flash_total_blocks =    total_blocks;
  nand_flash->lx_nand_flash_pages_per_block = pages_per_block;
  nand_flash->lx_nand_flash_bytes_per_page =  bytes_per_page;

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
  BSP_NAND_AddressTypeDef_t Bsp_Address = {0};
  Bsp_Address.Block = (uint16_t) block;
  Bsp_Address.Page = (uint16_t) page;

  if (BSP_FMC_NAND_Read(NAND_INSTANCE, &Bsp_Address, (uint16_t *) destination, 1) != BSP_ERROR_NONE)
  {
    return (LX_ERROR);
  }
 /* USER CODE END driver_read */

  return ret;
}

static UINT  lx_nand_driver_write(ULONG block, ULONG page, ULONG *source, ULONG words)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN driver_write */
  BSP_NAND_AddressTypeDef_t Bsp_Address = {0};

  Bsp_Address.Block = (uint16_t) block;
  Bsp_Address.Page = (uint16_t) page;

  if (BSP_FMC_NAND_Write(NAND_INSTANCE, &Bsp_Address, (uint16_t *) source, 1) != BSP_ERROR_NONE)
  {
    return (LX_ERROR);
  }
 /* USER CODE END driver_write */

  return ret;
}

static UINT  lx_nand_driver_block_erase(ULONG block, ULONG erase_count)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN block_erase */
  LX_PARAMETER_NOT_USED(erase_count);
  BSP_NAND_AddressTypeDef_t BspAddress = {0};

  BspAddress.Block = (uint16_t) block;

  if (BSP_FMC_NAND_Erase_Block(NAND_INSTANCE, &BspAddress) != BSP_ERROR_NONE)
  {
    return (LX_ERROR);
  }
 /* USER CODE END block_erase */

  return ret;
}

static UINT  lx_nand_driver_block_erased_verify(ULONG block)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN block_erase_verify */
  uint32_t index1 = 0;
  BSP_NAND_AddressTypeDef_t Bsp_Address = {0};

  Bsp_Address.Block = (uint16_t) block;


  for (index1 = 0; index1 < PHYSICAL_PAGES_PER_BLOCK ; index1++)
  {
    if (lx_nand_driver_page_erased_verify(Bsp_Address.Block, Bsp_Address.Page) != LX_SUCCESS)
    {
      return (LX_ERROR);
    }

    Bsp_Address.Page++;
  }
 /* USER CODE END block_erase_verify */

  return ret;
}

static UINT  lx_nand_driver_page_erased_verify(ULONG block, ULONG page)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN page_erased_verify */
  ULONG   *word_ptr;
  ULONG   words;
  BSP_NAND_AddressTypeDef_t Bsp_Address = {0};

  memset(nand_flash_rw_buffer, 0, sizeof(nand_flash_rw_buffer));

  Bsp_Address.Block = (uint16_t) block;
  Bsp_Address.Page = (uint16_t) page;

  if (BSP_FMC_NAND_Read(NAND_INSTANCE, &Bsp_Address, (uint16_t *) nand_flash_rw_buffer, 1) != BSP_ERROR_NONE)
  {
    return (LX_ERROR);
  }

  word_ptr = (ULONG *) & (nand_flash_rw_buffer[0]);

  /* Calculate the number of words in a page.  */
  words =  WORDS_PER_PHYSICAL_PAGE;

  /* Loop to read flash.  */
  while (words--)
  {
    /* Is this word erased?  */
    if (*word_ptr++ != 0xFFFFFFFF)
    {
      return (LX_ERROR);
    }
  }
 /* USER CODE END page_erased_verify */

  return ret;
}

static UINT  lx_nand_driver_block_status_get(ULONG block, UCHAR *bad_block_byte)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN block_status_get */
  BSP_NAND_AddressTypeDef_t Bsp_Address = {0};

  Bsp_Address.Block = (uint16_t) block;

  if (BSP_FMC_NAND_Read_SpareArea(NAND_INSTANCE, &Bsp_Address, (uint16_t *) Buffer_Spare_Area, 1) != BSP_ERROR_NONE)
  {
    return (LX_ERROR);
  }

  *bad_block_byte = (UCHAR) Buffer_Spare_Area[BAD_BLOCK_POSITION];
 /* USER CODE END block_status_get*/

  return ret;
}

static UINT  lx_nand_driver_block_status_set(ULONG block, UCHAR bad_block_byte)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN block_status_set */
  BSP_NAND_AddressTypeDef_t Bsp_Address = {0};

  Bsp_Address.Block = (uint16_t) block;

  if (BSP_FMC_NAND_Read_SpareArea(NAND_INSTANCE, &Bsp_Address, (uint16_t *) Buffer_Spare_Area, 1) != BSP_ERROR_NONE)
  {
    return (LX_ERROR);
  }

  Buffer_Spare_Area[BAD_BLOCK_POSITION] = bad_block_byte;

  if (BSP_FMC_NAND_Write_SpareArea(NAND_INSTANCE, &Bsp_Address, (uint16_t *) Buffer_Spare_Area, 1) != BSP_ERROR_NONE)
  {
    return (LX_ERROR);
  }
 /* USER CODE END block_status_set */

  return ret;
}

static UINT  lx_nand_driver_extra_bytes_get(ULONG block, ULONG page, UCHAR *destination, UINT size)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN extra_bytes_get */
  UCHAR   *source;

  BSP_NAND_AddressTypeDef_t Bsp_Address = {0};

  Bsp_Address.Block = (uint16_t) block;
  Bsp_Address.Page = (uint16_t) page;

  if (BSP_FMC_NAND_Read_SpareArea(NAND_INSTANCE, &Bsp_Address, (uint16_t *) Buffer_Spare_Area, 1) != BSP_ERROR_NONE)
  {
    return (LX_ERROR);
  }

  source = (UCHAR *) & (Buffer_Spare_Area[EXTRA_BYTE_POSITION]);

  while (size--)
  {

    /* Retrieve an extra byte from the spare area.  */
    *destination++ =  *source++;
  }
 /* USER CODE END extra_bytes_get */

  return ret;
}

static UINT  lx_nand_driver_extra_bytes_set(ULONG block, ULONG page, UCHAR *source, UINT size)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN extra_bytes_set */
  UCHAR   *destination;

  BSP_NAND_AddressTypeDef_t Bsp_Address = {0};

  Bsp_Address.Block = (uint16_t) block;
  Bsp_Address.Page = (uint16_t) page;

  if (BSP_FMC_NAND_Read_SpareArea(NAND_INSTANCE, &Bsp_Address, (uint16_t *) Buffer_Spare_Area, 1) != BSP_ERROR_NONE)
  {
    return (LX_ERROR);
  }

  destination = (UCHAR *) & (Buffer_Spare_Area[EXTRA_BYTE_POSITION]);

  while (size--)
  {

    /* Retrieve an extra byte from the spare area.  */
    *destination++ =  *source++;
  }

  if (BSP_FMC_NAND_Write_SpareArea(NAND_INSTANCE, &Bsp_Address, (uint16_t *) Buffer_Spare_Area, 1) != BSP_ERROR_NONE)
  {
    return (LX_ERROR);
  }
 /* USER CODE END extra_bytes_set */

  return ret;
}

static UINT  lx_nand_driver_system_error(UINT error_code, ULONG block, ULONG page)
{
  UINT ret = LX_SUCCESS;

 /* USER CODE BEGIN system_error */
  LX_PARAMETER_NOT_USED(error_code);
  LX_PARAMETER_NOT_USED(block);
  LX_PARAMETER_NOT_USED(page);

  /* Custom processing goes here...  all errors except for LX_NAND_ERROR_CORRECTED are fatal.  */
  ret = LX_ERROR;
 /* USER CODE END system_error */

  return ret;
}

/* USER CODE BEGIN 1 */
/* Erase chip function*/
UINT  _lx_nand_flash_erase_all_driver(VOID)
{
  if (BSP_FMC_NAND_EraseChip(NAND_INSTANCE) != BSP_ERROR_NONE)
  {
    return (LX_ERROR);
  }

  return (LX_SUCCESS);
}
/* USER CODE END 1 */
