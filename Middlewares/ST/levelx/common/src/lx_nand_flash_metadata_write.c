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


/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/** LevelX Component                                                      */ 
/**                                                                       */
/**   NAND Flash                                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define LX_SOURCE_CODE


/* Disable ThreadX error checking.  */

#ifndef LX_DISABLE_ERROR_CHECKING
#define LX_DISABLE_ERROR_CHECKING
#endif


/* Include necessary system files.  */

#include "lx_api.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _lx_nand_flash_metadata_write                       PORTABLE C      */ 
/*                                                           6.2.1       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Xiuwen Cai, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function writes metadata pages into current metadata block and */ 
/*    allocates new blocks for metadata.                                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    main_buffer                           Main page buffer              */ 
/*    spare_value                           Value for spare bytes         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    lx_nand_flash_driver_pages_write      Driver pages write            */ 
/*    _lx_nand_flash_metadata_allocate      Allocate blocks for metadata  */ 
/*    _lx_nand_flash_system_error           Internal system error handler */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Internal LevelX                                                     */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  03-08-2023     Xiuwen Cai               Initial Version 6.2.1        */
/*                                                                        */
/**************************************************************************/
UINT  _lx_nand_flash_metadata_write(LX_NAND_FLASH *nand_flash, UCHAR* main_buffer, ULONG spare_value)
{

ULONG   block;
ULONG   page;
UINT    status;
UCHAR   *spare_buffer_ptr;


    /* Setup spare buffer pointer.  */
    spare_buffer_ptr = nand_flash -> lx_nand_flash_page_buffer + nand_flash -> lx_nand_flash_bytes_per_page;

    /* Initialize the spare buffer.  */
    LX_MEMSET(spare_buffer_ptr, 0xFF, nand_flash -> lx_nand_flash_spare_total_length);

    /* Check if there is enough spare data for metadata block number.  */
    if (nand_flash -> lx_nand_flash_spare_data2_length >= 2)
    {

        /* Save metadata block number in spare bytes.  */
        LX_UTILITY_SHORT_SET(&spare_buffer_ptr[nand_flash -> lx_nand_flash_spare_data2_offset], nand_flash -> lx_nand_flash_metadata_block_number);
    }

    /* Save metadata type data in spare bytes.  */
    LX_UTILITY_LONG_SET(&spare_buffer_ptr[nand_flash -> lx_nand_flash_spare_data1_offset], spare_value);

    /* Get current metadata block number. */
    block = nand_flash -> lx_nand_flash_metadata_block_number_current;

    /* Get current metadata page number. */
    page = nand_flash -> lx_nand_flash_metadata_block_current_page;

    /* Write the page.  */
#ifdef LX_NAND_ENABLE_CONTROL_BLOCK_FOR_DRIVER_INTERFACE
    status = (nand_flash -> lx_nand_flash_driver_pages_write)(nand_flash, block, page, main_buffer, spare_buffer_ptr, 1);
#else
    status = (nand_flash -> lx_nand_flash_driver_pages_write)(block, page, main_buffer, spare_buffer_ptr, 1);
#endif

    /* Check for an error from flash driver.   */    
    if (status)
    {

        /* Call system error handler.  */
        _lx_nand_flash_system_error(nand_flash, status, block, 0);

        /* Return an error.  */
        return(status);
    }

    /* Increase current page for metadata block.  */
    nand_flash -> lx_nand_flash_metadata_block_current_page++;

    /* Get current backup metadata block number. */
    block = nand_flash -> lx_nand_flash_backup_metadata_block_number_current;

    /* Get current backup metadata page number. */
    page = nand_flash -> lx_nand_flash_backup_metadata_block_current_page;

    /* Write the page.  */
#ifdef LX_NAND_ENABLE_CONTROL_BLOCK_FOR_DRIVER_INTERFACE
    status = (nand_flash -> lx_nand_flash_driver_pages_write)(nand_flash, block, page, main_buffer, spare_buffer_ptr, 1);
#else
    status = (nand_flash -> lx_nand_flash_driver_pages_write)(block, page, main_buffer, spare_buffer_ptr, 1);
#endif

    /* Check for an error from flash driver.   */
    if (status)
    {

        /* Call system error handler.  */
        _lx_nand_flash_system_error(nand_flash, status, block, 0);

        /* Return an error.  */
        return(status);
    }

    /* Increase current page for backup metadata block.  */
    nand_flash -> lx_nand_flash_backup_metadata_block_current_page++;

    /* Allocate new block for metadata if necessary.  */
    _lx_nand_flash_metadata_allocate(nand_flash);

    /* Return sector not found status.  */
    return(status);
}

