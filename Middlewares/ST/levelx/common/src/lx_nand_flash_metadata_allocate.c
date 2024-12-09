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
/*    _lx_nand_flash_metadata_allocate                    PORTABLE C      */ 
/*                                                           6.2.1       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Xiuwen Cai, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function allocates new blocks for metadata if current metadata */
/*    block is full. This function also frees metadata blocks if the chain*/
/*    is too long.                                                        */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _lx_nand_flash_metadata_build         Build metadata                */ 
/*    _lx_nand_flash_driver_block_erase     Erase block                   */ 
/*    _lx_nand_flash_block_data_move        Move block data               */ 
/*    _lx_nand_flash_free_block_list_add    Add free block list           */ 
/*    _lx_nand_flash_block_allocate         Allocate block                */ 
/*    _lx_nand_flash_block_status_set       Set block status              */
/*    _lx_nand_flash_metadata_write         Write metadata                */
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
UINT  _lx_nand_flash_metadata_allocate(LX_NAND_FLASH *nand_flash)
{

ULONG   block;
ULONG   page;
UINT    status;
UCHAR  *page_buffer_ptr;
UINT    j;
UCHAR   min_erase_count;


    /* Get current page for metadata block.  */
    page = nand_flash -> lx_nand_flash_metadata_block_current_page;

    /* Check if new metadata block is required.  */
    if (page < nand_flash -> lx_nand_flash_pages_per_block)
    {

        /* No new block is required. Just return success.  */
        return(LX_SUCCESS);
    }

    /* Advance to next metadata block.  */
    nand_flash -> lx_nand_flash_metadata_block_number_current = nand_flash -> lx_nand_flash_metadata_block_number_next;

    /* Reset current page number.  */
    nand_flash -> lx_nand_flash_metadata_block_current_page = 0;

    /* Advance to next backup metadata block.  */
    nand_flash -> lx_nand_flash_backup_metadata_block_number_current = nand_flash -> lx_nand_flash_backup_metadata_block_number_next;

    /* Reset current page number for backup metadata block.  */
    nand_flash -> lx_nand_flash_backup_metadata_block_current_page = 0;

    /* Check if number of allocated blocks reaches maximum.  */
    if (nand_flash -> lx_nand_flash_metadata_block_count == LX_NAND_FLASH_MAX_METADATA_BLOCKS)
    {

        /* Loop to mark metadata blocks as free.  */
        for (j = 0; j < LX_NAND_FLASH_MAX_METADATA_BLOCKS - 1; j++)
        {

            /* Free metadata block.  */
            nand_flash -> lx_nand_flash_block_status_table[nand_flash -> lx_nand_flash_metadata_block[j]] = LX_NAND_BLOCK_STATUS_FREE;

            /* Free backup metadata block.  */
            nand_flash -> lx_nand_flash_block_status_table[nand_flash -> lx_nand_flash_backup_metadata_block[j]] = LX_NAND_BLOCK_STATUS_FREE;

            /* Increase erase count.  */
            nand_flash -> lx_nand_flash_erase_count_table[nand_flash -> lx_nand_flash_metadata_block[j]]++;

            /* Increase erase count for backup metadata block.  */
            nand_flash -> lx_nand_flash_erase_count_table[nand_flash -> lx_nand_flash_backup_metadata_block[j]]++;
        }

        /* Set new metadata block head.  */
        nand_flash -> lx_nand_flash_metadata_block_number = nand_flash -> lx_nand_flash_metadata_block_number_current;

        /* Set new backup metadata block head.  */
        nand_flash -> lx_nand_flash_backup_metadata_block_number = nand_flash -> lx_nand_flash_backup_metadata_block_number_current;

        /* Set erase count to maximum.  */
        min_erase_count = 255;

        /* Loop to find the minimum erase count.  */
        for (j = 0; j < nand_flash -> lx_nand_flash_total_blocks; j++)
        {

            /* Exclude erase count of bad block.  */
            if(nand_flash -> lx_nand_flash_block_status_table[j] != LX_NAND_BLOCK_STATUS_BAD)
            { 

                /* Check if it has less erase count.  */
                if (nand_flash -> lx_nand_flash_erase_count_table[j] < min_erase_count)
                {

                    /* Update the minimum erase count.  */
                    min_erase_count = nand_flash -> lx_nand_flash_erase_count_table[j];
                }
            }
        }

        /* Check if the minimum erase count is larger than zero.  */
        if (min_erase_count > 0)
        {

            /* Loop to update erase count.  */
            for (j = 0; j < nand_flash -> lx_nand_flash_total_blocks; j++)
            {

                /* Skip bad blocks.  */
                if (nand_flash -> lx_nand_flash_block_status_table[j] != LX_NAND_BLOCK_STATUS_BAD)
                {

                    /* Update erase count.  */
                     nand_flash -> lx_nand_flash_erase_count_table[j] = (UCHAR)(nand_flash -> lx_nand_flash_erase_count_table[j] - min_erase_count);
                }
            }

            /* Update base erase count.  */
            nand_flash -> lx_nand_flash_base_erase_count += (ULONG)min_erase_count;
        }

        /* Rebuild metadata pages.  */
        status = _lx_nand_flash_metadata_build(nand_flash);

        /* Check return status.  */
        if (status != LX_SUCCESS)
        {

            /* Return error status.  */
            return(status);
        }

        /* Loop to erase freed blocks.  */
        for (j = 0; j < LX_NAND_FLASH_MAX_METADATA_BLOCKS - 1; j++)
        {

            /* Get the block number to erase.  */
            block = nand_flash -> lx_nand_flash_metadata_block[j];

            /* Erase the block.  */
            status = _lx_nand_flash_driver_block_erase(nand_flash, block, nand_flash -> lx_nand_flash_base_erase_count + nand_flash -> lx_nand_flash_erase_count_table[block]);

            /* Check for an error from flash driver.   */
            if (status)
            {

                /* Call system error handler.  */
                _lx_nand_flash_system_error(nand_flash, status, block, 0);
#ifdef LX_THREAD_SAFE_ENABLE

                /* Release the thread safe mutex.  */
                tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

                /* Return an error.  */
                return(LX_ERROR);
            }

            /* Check if the block has too many erases.  */
            if (nand_flash -> lx_nand_flash_erase_count_table[block] > LX_NAND_FLASH_MAX_ERASE_COUNT_DELTA)
            {

                /* Move data from less worn block.  */
                _lx_nand_flash_block_data_move(nand_flash, block);
            }
            else
            {

                /* Add the block to free block list.  */
                _lx_nand_flash_free_block_list_add(nand_flash, block);
            }

            /* Get the block number from backup metadata blocks.  */
            block = nand_flash -> lx_nand_flash_backup_metadata_block[j];

            /* Erase the block.  */
            status = _lx_nand_flash_driver_block_erase(nand_flash, block, nand_flash -> lx_nand_flash_base_erase_count + nand_flash -> lx_nand_flash_erase_count_table[block]);
        
            /* Check for an error from flash driver.   */
            if (status)
            {

                /* Call system error handler.  */
                _lx_nand_flash_system_error(nand_flash, status, block, 0);
#ifdef LX_THREAD_SAFE_ENABLE

                /* Release the thread safe mutex.  */
                tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

                /* Return an error.  */
                return(LX_ERROR);
            }

            /* Check if the block has too many erases.  */
            if (nand_flash -> lx_nand_flash_erase_count_table[block] > LX_NAND_FLASH_MAX_ERASE_COUNT_DELTA)
            {

                /* Move data from less worn block.  */
                _lx_nand_flash_block_data_move(nand_flash, block);
            }
            else
            {

                /* Add the block to free block list.  */
                _lx_nand_flash_free_block_list_add(nand_flash, block);
            }
        }

        /* Update metadata block list.  */
        nand_flash -> lx_nand_flash_metadata_block[0] = (USHORT)nand_flash -> lx_nand_flash_metadata_block_number_current;

        /* Update metadata block list.  */
        nand_flash -> lx_nand_flash_backup_metadata_block[0] = (USHORT)nand_flash -> lx_nand_flash_backup_metadata_block_number_current;

        /* Set metadata block count to one.  */
        nand_flash -> lx_nand_flash_metadata_block_count = 1;
    }

    /* Allocate new block for metadata.  */
    status = _lx_nand_flash_block_allocate(nand_flash, &block);

    /* Check return status.  */
    if (status != LX_SUCCESS)
    {

        /* Call system error handler.  */
        _lx_nand_flash_system_error(nand_flash, status, block, 0);

        /* Determine if the error is fatal.  */
        if (status != LX_NAND_ERROR_CORRECTED)
        {

            /* Return an error.  */
            return(LX_ERROR);
        }
    }

    /* Update next metadata block number.  */
    nand_flash -> lx_nand_flash_metadata_block_number_next = block;

    /* Save the new block in metadata block list.  */
    nand_flash -> lx_nand_flash_metadata_block[nand_flash -> lx_nand_flash_metadata_block_count] = (USHORT)block;

    /* Set the block status to allocated.  */
    status = _lx_nand_flash_block_status_set(nand_flash, block, LX_NAND_BLOCK_STATUS_ALLOCATED);

    /* Check return status.  */
    if (status != LX_SUCCESS)
    {

        /* Return status.  */
        return (status);
    }

    /* Allocate new block for backup metadata.  */
    status = _lx_nand_flash_block_allocate(nand_flash, &block);

    /* Check return status.  */
    if (status != LX_SUCCESS)
    {

        /* Call system error handler.  */
        _lx_nand_flash_system_error(nand_flash, status, block, 0);

        /* Determine if the error is fatal.  */
        if (status != LX_NAND_ERROR_CORRECTED)
        {

            /* Return an error.  */
            return(LX_ERROR);
        }
    }

    /* Update next backup metadata block number.  */
    nand_flash -> lx_nand_flash_backup_metadata_block_number_next = block;

    /* Save the new block in metadata block list.  */
    nand_flash -> lx_nand_flash_backup_metadata_block[nand_flash -> lx_nand_flash_metadata_block_count] = (USHORT)block;

    /* Set the block status to allocated.  */
    status = _lx_nand_flash_block_status_set(nand_flash, block, LX_NAND_BLOCK_STATUS_ALLOCATED);
    if (status != LX_SUCCESS)
    {
        return (status);
    }

    /* Setup page buffer.  */
    page_buffer_ptr = nand_flash -> lx_nand_flash_page_buffer;

    /* Initialize page data.  */
    LX_MEMSET(page_buffer_ptr, 0xFF, nand_flash -> lx_nand_flash_bytes_per_page);

    /* Save next block number.  */
    LX_UTILITY_LONG_SET(&page_buffer_ptr[LX_NAND_BLOCK_LINK_MAIN_METADATA_OFFSET], nand_flash -> lx_nand_flash_metadata_block_number_next);

    /* Save next block number for backup metadata.  */
    LX_UTILITY_LONG_SET(&page_buffer_ptr[LX_NAND_BLOCK_LINK_BACKUP_METADATA_OFFSET], nand_flash -> lx_nand_flash_backup_metadata_block_number_next);

    /* Write metadata.  */
    status = _lx_nand_flash_metadata_write(nand_flash, page_buffer_ptr, LX_NAND_PAGE_TYPE_BLOCK_LINK);

    /* Increase metadata block count.  */
    nand_flash -> lx_nand_flash_metadata_block_count++;

    /* Return sector not found status.  */
    return(status);
}

