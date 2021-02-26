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

static UINT  _lx_nand_flash_read_driver(ULONG block, ULONG page, ULONG *destination, ULONG words);
static UINT  _lx_nand_flash_write_driver(ULONG block, ULONG page, ULONG *source, ULONG words);
static UINT  _lx_nand_flash_block_erase_driver(ULONG block, ULONG erase_count);
static UINT  _lx_nand_flash_block_erased_verify_driver(ULONG block);
static UINT  _lx_nand_flash_page_erased_verify_driver(ULONG block, ULONG page);
static UINT  _lx_nand_flash_block_status_get_driver(ULONG block, UCHAR *bad_block_byte);
static UINT  _lx_nand_flash_block_status_set_driver(ULONG block, UCHAR bad_block_byte);
static UINT  _lx_nand_flash_extra_bytes_get_driver(ULONG block, ULONG page, UCHAR *destination, UINT size);
static UINT  _lx_nand_flash_extra_bytes_set_driver(ULONG block, ULONG page, UCHAR *source, UINT size);
static UINT  _lx_nand_flash_system_error_driver(UINT error_code, ULONG block, ULONG page);

/* Define NAND flash buffers for LevelX.  */
ULONG  nand_flash_driver_buffer[WORDS_PER_PHYSICAL_PAGE] = {0};
ULONG  nand_flash_rw_buffer[WORDS_PER_PHYSICAL_PAGE] = {0};
UCHAR  Buffer_Spare_Area[SPARE_BYTES_PER_PAGE] = {0};

uint8_t flash_is_initialized = 0;

/* Initialize nand flash function*/
UINT  _lx_nand_flash_initialize_driver(LX_NAND_FLASH *nand_flash)
{
  NAND_IDTypeDef pNAND_ID;

  if (!flash_is_initialized)
  {
    /* Initialize FMC interface */
    if (BSP_FMC_NAND_Init(NAND_INSTANCE) != BSP_ERROR_NONE)
    {
      return(LX_ERROR);
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
    if(pNAND_ID.Maker_Id  != BSP_NAND_MANUFACTURER_CODE)
      return(LX_ERROR);
    else if (pNAND_ID.Device_Id != BSP_NAND_DEVICE_CODE)
      return(LX_ERROR);
    else if (pNAND_ID.Third_Id != BSP_NAND_THIRD_ID)
      return(LX_ERROR);
    else if (pNAND_ID.Fourth_Id != BSP_NAND_FOURTH_ID)
      return(LX_ERROR);

    /* If ERASE_CHIP enabled erase the NAND device */
    if (LX_DRIVER_ERASES_FLASH_AFTER_INIT)
    {
      if (_lx_nand_flash_erase_all_driver() != LX_SUCCESS)
      {
        return(LX_ERROR);
      }
    }

    flash_is_initialized = 1;
  }
  /* Setup geometry of the NAND flash.  */
  nand_flash -> lx_nand_flash_total_blocks =                  TOTAL_BLOCKS;
  nand_flash -> lx_nand_flash_pages_per_block =               PHYSICAL_PAGES_PER_BLOCK;
  nand_flash -> lx_nand_flash_bytes_per_page =                BYTES_PER_PHYSICAL_PAGE;

  /* Setup function pointers for the NAND flash services.  */
  nand_flash -> lx_nand_flash_driver_read =                   _lx_nand_flash_read_driver;
  nand_flash -> lx_nand_flash_driver_write =                  _lx_nand_flash_write_driver;
  nand_flash -> lx_nand_flash_driver_block_erase =            _lx_nand_flash_block_erase_driver;
  nand_flash -> lx_nand_flash_driver_block_erased_verify =    _lx_nand_flash_block_erased_verify_driver;
  nand_flash -> lx_nand_flash_driver_page_erased_verify =     _lx_nand_flash_page_erased_verify_driver;
  nand_flash -> lx_nand_flash_driver_block_status_get =       _lx_nand_flash_block_status_get_driver;
  nand_flash -> lx_nand_flash_driver_block_status_set =       _lx_nand_flash_block_status_set_driver;
  nand_flash -> lx_nand_flash_driver_extra_bytes_get =        _lx_nand_flash_extra_bytes_get_driver;
  nand_flash -> lx_nand_flash_driver_extra_bytes_set =        _lx_nand_flash_extra_bytes_set_driver;
  nand_flash -> lx_nand_flash_driver_system_error =           _lx_nand_flash_system_error_driver;

  /* Setup local buffer for NAND flash operation. This buffer must be the page size of the NAND flash memory.  */
  nand_flash -> lx_nand_flash_page_buffer =  &nand_flash_driver_buffer[0];

  /* Return success.  */
  return(LX_SUCCESS);
}

/* Read from nand flash memory function*/
static UINT  _lx_nand_flash_read_driver(ULONG block, ULONG page, ULONG *destination, ULONG words)
{
  BSP_NAND_AddressTypeDef_t Bsp_Address = {0};

  Bsp_Address.Block = (uint16_t) block;
  Bsp_Address.Page = (uint16_t) page;

  if (BSP_FMC_NAND_Read(NAND_INSTANCE, &Bsp_Address, (uint16_t *) destination, 1)!= BSP_ERROR_NONE)
  {
    return(LX_ERROR);
  }

  return(LX_SUCCESS);
}

/* Write to nand flash memory function*/
static UINT  _lx_nand_flash_write_driver(ULONG block, ULONG page, ULONG *source, ULONG words)
{
  BSP_NAND_AddressTypeDef_t Bsp_Address = {0};

  Bsp_Address.Block = (uint16_t) block;
  Bsp_Address.Page = (uint16_t) page;

  if (BSP_FMC_NAND_Write(NAND_INSTANCE, &Bsp_Address, (uint16_t *) source, 1) != BSP_ERROR_NONE)
  {
    return(LX_ERROR);
  }

  return(LX_SUCCESS);
}

/* Erase block function*/
static UINT  _lx_nand_flash_block_erase_driver(ULONG block, ULONG erase_count)
{
  LX_PARAMETER_NOT_USED(erase_count);

  BSP_NAND_AddressTypeDef_t BspAddress = {0};

  BspAddress.Block = (uint16_t) block;

  if (BSP_FMC_NAND_Erase_Block(NAND_INSTANCE, &BspAddress) != BSP_ERROR_NONE)
  {
    return(LX_ERROR);
  }

  return(LX_SUCCESS);
}

/* Erase chip function*/
UINT  _lx_nand_flash_erase_all_driver(VOID)
{
  if (BSP_FMC_NAND_EraseChip(NAND_INSTANCE) != BSP_ERROR_NONE)
  {
    return(LX_ERROR);
  }

  return(LX_SUCCESS);
}

/* Verify block erased function*/
static UINT  _lx_nand_flash_block_erased_verify_driver(ULONG block)
{
  uint32_t index1 = 0;
  BSP_NAND_AddressTypeDef_t Bsp_Address = {0};

  Bsp_Address.Block = (uint16_t) block;


  for (index1 = 0; index1 < PHYSICAL_PAGES_PER_BLOCK ; index1++)
  {
    if (_lx_nand_flash_page_erased_verify_driver(Bsp_Address.Block, Bsp_Address.Page) != LX_SUCCESS)
      return(LX_ERROR);

    Bsp_Address.Page++;
  }

  /* Return success.  */
  return(LX_SUCCESS);
}

/* Verify page erased function*/
static UINT  _lx_nand_flash_page_erased_verify_driver(ULONG block, ULONG page)
{
  ULONG   *word_ptr;
  ULONG   words;
  BSP_NAND_AddressTypeDef_t Bsp_Address = {0};

  memset(nand_flash_rw_buffer, 0, sizeof(nand_flash_rw_buffer));

  Bsp_Address.Block = (uint16_t) block;
  Bsp_Address.Page = (uint16_t) page;

  if (BSP_FMC_NAND_Read(NAND_INSTANCE, &Bsp_Address, (uint16_t *) nand_flash_rw_buffer, 1)!= BSP_ERROR_NONE)
  {
    return(LX_ERROR);
  }

  word_ptr = (ULONG *) &(nand_flash_rw_buffer[0]);

  /* Calculate the number of words in a page.  */
  words =  WORDS_PER_PHYSICAL_PAGE;

  /* Loop to read flash.  */
  while (words--)
  {
    /* Is this word erased?  */
    if (*word_ptr++ != 0xFFFFFFFF)
      return(LX_ERROR);
  }

  /* Return success.  */
  return(LX_SUCCESS);
}

/* Get block status function*/
static UINT  _lx_nand_flash_block_status_get_driver(ULONG block, UCHAR *bad_block_byte)
{
  BSP_NAND_AddressTypeDef_t Bsp_Address = {0};

  Bsp_Address.Block = (uint16_t) block;

  if (BSP_FMC_NAND_Read_SpareArea(NAND_INSTANCE, &Bsp_Address, (uint16_t *) Buffer_Spare_Area, 1) != BSP_ERROR_NONE)
  {
    return(LX_ERROR);
  }

  *bad_block_byte = (UCHAR) Buffer_Spare_Area[BAD_BLOCK_POSITION];

  /* Return success.  */
  return(LX_SUCCESS);
}

/* Set block status function*/
static UINT  _lx_nand_flash_block_status_set_driver(ULONG block, UCHAR bad_block_byte)
{
  BSP_NAND_AddressTypeDef_t Bsp_Address = {0};

  Bsp_Address.Block = (uint16_t) block;

  if (BSP_FMC_NAND_Read_SpareArea(NAND_INSTANCE, &Bsp_Address, (uint16_t *) Buffer_Spare_Area, 1) != BSP_ERROR_NONE)
  {
    return(LX_ERROR);
  }

  Buffer_Spare_Area[BAD_BLOCK_POSITION] = bad_block_byte;

  if (BSP_FMC_NAND_Write_SpareArea(NAND_INSTANCE, &Bsp_Address, (uint16_t *) Buffer_Spare_Area, 1) != BSP_ERROR_NONE)
  {
    return(LX_ERROR);
  }

  /* Return success.  */
  return(LX_SUCCESS);
}

/* Get extra byte function*/
static UINT  _lx_nand_flash_extra_bytes_get_driver(ULONG block, ULONG page, UCHAR *destination, UINT size)
{
  UCHAR   *source;

  BSP_NAND_AddressTypeDef_t Bsp_Address = {0};

  Bsp_Address.Block = (uint16_t) block;
  Bsp_Address.Page = (uint16_t) page;

  if (BSP_FMC_NAND_Read_SpareArea(NAND_INSTANCE, &Bsp_Address, (uint16_t *) Buffer_Spare_Area, 1) != BSP_ERROR_NONE)
  {
    return(LX_ERROR);
  }

  source = (UCHAR *) &(Buffer_Spare_Area[EXTRA_BYTE_POSITION]);

  while (size--)
  {

    /* Retrieve an extra byte from the spare area.  */
    *destination++ =  *source++;
  }

  /* Return success.  */
  return(LX_SUCCESS);
}

/* Set extra byte function*/
static UINT  _lx_nand_flash_extra_bytes_set_driver(ULONG block, ULONG page, UCHAR *source, UINT size)
{
  UCHAR   *destination;

  BSP_NAND_AddressTypeDef_t Bsp_Address = {0};

  Bsp_Address.Block = (uint16_t) block;
  Bsp_Address.Page = (uint16_t) page;

  if (BSP_FMC_NAND_Read_SpareArea(NAND_INSTANCE, &Bsp_Address, (uint16_t *) Buffer_Spare_Area, 1) != BSP_ERROR_NONE)
  {
    return(LX_ERROR);
  }

  destination = (UCHAR *) &(Buffer_Spare_Area[EXTRA_BYTE_POSITION]);

  while (size--)
  {

    /* Retrieve an extra byte from the spare area.  */
    *destination++ =  *source++;
  }

  if (BSP_FMC_NAND_Write_SpareArea(NAND_INSTANCE, &Bsp_Address, (uint16_t *) Buffer_Spare_Area, 1) != BSP_ERROR_NONE)
  {
    return(LX_ERROR);
  }

  /* Return success.  */
  return(LX_SUCCESS);
}

/* System error function */
static UINT  _lx_nand_flash_system_error_driver(UINT error_code, ULONG block, ULONG page)
{
  LX_PARAMETER_NOT_USED(error_code);
  LX_PARAMETER_NOT_USED(block);
  LX_PARAMETER_NOT_USED(page);

  /* Custom processing goes here...  all errors except for LX_NAND_ERROR_CORRECTED are fatal.  */
  return(LX_ERROR);
}



