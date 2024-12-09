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
/*    _lx_nand_flash_format                               PORTABLE C      */ 
/*                                                           6.3.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Xiuwen Cai, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function erases and formats a NAND flash.                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    name                                  Name of NAND flash instance   */ 
/*    nand_driver_initialize                Driver initialize             */ 
/*    memory_ptr                            Pointer to memory used by the */
/*                                            LevelX for this NAND.       */
/*    memory_size                           Size of memory - must at least*/
/*                                            7 * total block count +     */
/*                                            2 * page size               */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    (nand_driver_initialize)              Driver initialize             */ 
/*    LX_MEMSET                             Initialize memory             */ 
/*    _lx_nand_flash_memory_initialize      Initialize buffer             */ 
/*    _lx_nand_flash_driver_block_status_get                              */
/*                                          Driver block status get       */ 
/*    _lx_nand_flash_driver_block_status_set                              */ 
/*                                          Driver block status set       */ 
/*    _lx_nand_flash_metadata_build         Build metadata                */ 
/*    _lx_nand_flash_metadata_write         Write metadata                */
/*    _lx_nand_flash_driver_block_erase     Driver block erase            */ 
/*    _lx_nand_flash_system_error           System error handler          */ 
/*    tx_mutex_create                       Create thread-safe mutex      */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  03-08-2023     Xiuwen Cai               Initial Version 6.2.1         */
/*  10-31-2023     Xiuwen Cai               Modified comment(s),          */
/*                                            avoided clearing user       */
/*                                            extension in flash control  */
/*                                            block,                      */
/*                                            resulting in version 6.3.0  */
/*                                                                        */
/**************************************************************************/
UINT  _lx_nand_flash_format(LX_NAND_FLASH* nand_flash, CHAR* name,
                            UINT(*nand_driver_initialize)(LX_NAND_FLASH*),
                            ULONG* memory_ptr, UINT memory_size)
{

ULONG                       block;
UCHAR                       block_status;
UINT                        status;
UCHAR                       *page_buffer_ptr;

    LX_PARAMETER_NOT_USED(name);

    /* Clear the NAND flash control block. User extension is not cleared.  */
    LX_MEMSET(nand_flash, 0, (ULONG)((UCHAR*)&(nand_flash -> lx_nand_flash_open_previous) - (UCHAR*)nand_flash) + sizeof(nand_flash -> lx_nand_flash_open_previous));

    /* Call the flash driver's initialization function.  */
    (nand_driver_initialize)(nand_flash);

    /* Determine if we can support this NAND flash size.  */
    if (nand_flash -> lx_nand_flash_pages_per_block > LX_NAND_MAX_PAGE_PER_BLOCK || nand_flash -> lx_nand_flash_total_blocks > LX_NAND_MAX_BLOCK_COUNT)
    {
        return(LX_ERROR);
    }

    /* Check if it is new LevelX NAND driver.  */
    if (nand_flash -> lx_nand_flash_driver_pages_read == LX_NULL || nand_flash -> lx_nand_flash_driver_pages_write == LX_NULL || nand_flash -> lx_nand_flash_driver_pages_copy == LX_NULL)
    {
        return(LX_ERROR);
    }

    /* Check the spare data length.  */
    if (nand_flash -> lx_nand_flash_spare_data1_length < sizeof(ULONG))
    {
        return(LX_ERROR);
    }

    /* Calculate the number of words per block and per page.  */
    nand_flash -> lx_nand_flash_words_per_page = (nand_flash -> lx_nand_flash_bytes_per_page / sizeof(ULONG));
    nand_flash -> lx_nand_flash_words_per_block = (nand_flash -> lx_nand_flash_words_per_page * nand_flash -> lx_nand_flash_pages_per_block);

    /* Calculate the total pages.  */
    nand_flash -> lx_nand_flash_total_pages = nand_flash -> lx_nand_flash_total_blocks * nand_flash -> lx_nand_flash_pages_per_block;

    /* Initialize memory buffer.  */
    status = _lx_nand_flash_memory_initialize(nand_flash, memory_ptr, memory_size);
    if (status != LX_SUCCESS)
    {
        return(status);
    }

    /* Initialize block numbers.  */
    nand_flash -> lx_nand_flash_metadata_block_number = LX_NAND_BLOCK_UNMAPPED;
    nand_flash -> lx_nand_flash_metadata_block_number_next = LX_NAND_BLOCK_UNMAPPED;
    nand_flash -> lx_nand_flash_backup_metadata_block_number = LX_NAND_BLOCK_UNMAPPED;
    nand_flash -> lx_nand_flash_backup_metadata_block_number_next = LX_NAND_BLOCK_UNMAPPED;

    /* Initialize the block status buffer.  */
    LX_MEMSET(nand_flash -> lx_nand_flash_block_status_table, 0xFF, nand_flash -> lx_nand_flash_block_status_table_size);

    /* Loop through the blocks to check for bad blocks and determine the minimum and maximum erase count for each good block.  */
    for (block = 0; block < nand_flash -> lx_nand_flash_total_blocks; block++)
    {

        /* First, check to make sure this block is good.  */
        status = _lx_nand_flash_driver_block_status_get(nand_flash, block, &block_status);

        /* Check for an error from flash driver.   */
        if (status)
        {

            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, block, 0);

            /* Return an error.  */
            return(LX_ERROR);
        }

        /* Is this block bad?  */
        if (block_status != LX_NAND_GOOD_BLOCK)
        {

            /* Yes, this block is bad.  */

            /* Increment the number of bad blocks.  */
            nand_flash -> lx_nand_flash_bad_blocks++;

            /* Save the block status.  */
            nand_flash -> lx_nand_flash_block_status_table[block] = LX_NAND_BLOCK_STATUS_BAD;

            /* Continue to the next block.  */
            continue;
        }

        /* Erase the block.  */
        status = _lx_nand_flash_driver_block_erase(nand_flash, block, 0);

        /* Check for an error from flash driver.   */
        if (status)
        {

            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, block, 0);

            /* Attempt to mark this block as bad.  */
            status = _lx_nand_flash_driver_block_status_set(nand_flash, block, LX_NAND_BAD_BLOCK);

            /* Check for error in setting the block status.  */
            if (status)
            {

                /* Call system error handler.  */
                _lx_nand_flash_system_error(nand_flash, status, block, 0);
            }

            /* Increment the bad block count.  */
            nand_flash -> lx_nand_flash_bad_blocks++;

            /* Save the block status.  */
            nand_flash -> lx_nand_flash_block_status_table[block] = LX_NAND_BLOCK_STATUS_BAD;
        }
        else
        {

            /* Allocate blocks for metadata.  */
            if (nand_flash -> lx_nand_flash_metadata_block_number == LX_NAND_BLOCK_UNMAPPED)
            {
                nand_flash -> lx_nand_flash_metadata_block_number = block;
                nand_flash -> lx_nand_flash_metadata_block_number_current = block;
            }
            else if (nand_flash -> lx_nand_flash_backup_metadata_block_number == LX_NAND_BLOCK_UNMAPPED)
            {
                nand_flash -> lx_nand_flash_backup_metadata_block_number = block;
                nand_flash -> lx_nand_flash_backup_metadata_block_number_current = block;
            }
            else if (nand_flash -> lx_nand_flash_metadata_block_number_next == LX_NAND_BLOCK_UNMAPPED)
            {
                nand_flash -> lx_nand_flash_metadata_block_number_next = block;
            }
            else if (nand_flash -> lx_nand_flash_backup_metadata_block_number_next == LX_NAND_BLOCK_UNMAPPED)
            {
                nand_flash -> lx_nand_flash_backup_metadata_block_number_next = block;
            }
        }
    }

    /* There should be enough blocks for metadata.  */
    if (nand_flash -> lx_nand_flash_backup_metadata_block_number_next == LX_NAND_BLOCK_UNMAPPED)
    {
        return (LX_NO_BLOCKS);
    }

    /* Save the block status for metadata.  */
    nand_flash -> lx_nand_flash_block_status_table[nand_flash -> lx_nand_flash_backup_metadata_block_number_next] = LX_NAND_BLOCK_STATUS_ALLOCATED;
    nand_flash -> lx_nand_flash_block_status_table[nand_flash -> lx_nand_flash_metadata_block_number_next] = LX_NAND_BLOCK_STATUS_ALLOCATED;
    nand_flash -> lx_nand_flash_block_status_table[nand_flash -> lx_nand_flash_backup_metadata_block_number] = (USHORT)nand_flash -> lx_nand_flash_backup_metadata_block_number_next;
    nand_flash -> lx_nand_flash_block_status_table[nand_flash -> lx_nand_flash_metadata_block_number] = (USHORT)nand_flash -> lx_nand_flash_metadata_block_number_next;
    
    /* Initialize the mapping table.  */
    LX_MEMSET(nand_flash -> lx_nand_flash_block_mapping_table, 0xFF, nand_flash -> lx_nand_flash_block_mapping_table_size);

    /* Build initial metadata.  */
    status = _lx_nand_flash_metadata_build(nand_flash);
    if (status != LX_SUCCESS)
    {
        /* Return error status.  */
        return(status);
    }

    /* Get buffer for page data.  */
    page_buffer_ptr = nand_flash -> lx_nand_flash_page_buffer;

    /* Initialize the page buffer.  */
    LX_MEMSET(page_buffer_ptr, 0xFF, nand_flash -> lx_nand_flash_bytes_per_page);

    /* Set the next block numbers.  */
    LX_UTILITY_LONG_SET(&page_buffer_ptr[LX_NAND_BLOCK_LINK_MAIN_METADATA_OFFSET], nand_flash -> lx_nand_flash_metadata_block_number_next);
    LX_UTILITY_LONG_SET(&page_buffer_ptr[LX_NAND_BLOCK_LINK_BACKUP_METADATA_OFFSET], nand_flash -> lx_nand_flash_backup_metadata_block_number_next);

    /* Save the next block numbers to metadata block.  */
    status = _lx_nand_flash_metadata_write(nand_flash, page_buffer_ptr, LX_NAND_PAGE_TYPE_BLOCK_LINK);

    if (status != LX_SUCCESS)
    {
        /* Return error status.  */
        return(status);
    }

    /* Return a successful completion.  */
    return(LX_SUCCESS);
}

