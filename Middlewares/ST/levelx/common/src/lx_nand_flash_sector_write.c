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
/*    _lx_nand_flash_sector_write                         PORTABLE C      */ 
/*                                                           6.2.1       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Xiuwen Cai, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function writes a logical sector to the NAND flash page.       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    logical_sector                        Logical sector number         */ 
/*    buffer                                Pointer to buffer to write    */ 
/*                                            (the size is number of      */ 
/*                                             bytes in a page)           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _lx_nand_flash_block_find             Find the mapped block         */ 
/*    _lx_nand_flash_block_allocate         Allocate block                */ 
/*    _lx_nand_flash_mapped_block_list_remove                             */
/*                                          Remove mapped block           */ 
/*    _lx_nand_flash_data_page_copy         Copy data pages               */ 
/*    _lx_nand_flash_free_block_list_add    Add free block to list        */
/*    _lx_nand_flash_block_mapping_set      Set block mapping             */ 
/*    lx_nand_flash_driver_pages_write      Write pages                   */ 
/*    _lx_nand_flash_block_status_set       Set block status              */ 
/*    _lx_nand_flash_driver_block_erase     Erase block                   */ 
/*    _lx_nand_flash_erase_count_set        Set erase count               */
/*    _lx_nand_flash_block_data_move        Move block data               */
/*    _lx_nand_flash_mapped_block_list_add  Add mapped block to list      */
/*    _lx_nand_flash_system_error           Internal system error handler */ 
/*    tx_mutex_get                          Get thread protection         */ 
/*    tx_mutex_put                          Release thread protection     */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  03-08-2023     Xiuwen Cai               Initial Version 6.2.1        */
/*                                                                        */
/**************************************************************************/
UINT  _lx_nand_flash_sector_write(LX_NAND_FLASH *nand_flash, ULONG logical_sector, VOID *buffer)
{

UINT                                status;
ULONG                               block;
ULONG                               new_block;
ULONG                               page;
USHORT                              block_status = 0;
USHORT                              new_block_status = LX_NAND_BLOCK_STATUS_ALLOCATED;
UCHAR                               *spare_buffer_ptr;
UINT                                update_mapping = LX_FALSE;
UINT                                copy_block = LX_FALSE;

#ifdef LX_THREAD_SAFE_ENABLE

    /* Obtain the thread safe mutex.  */
    tx_mutex_get(&nand_flash -> lx_nand_flash_mutex, TX_WAIT_FOREVER);
#endif

    /* Increment the number of write requests.  */
    nand_flash -> lx_nand_flash_diagnostic_sector_write_requests++;

    /* See if we can find the logical sector in the current mapping.  */
    status = _lx_nand_flash_block_find(nand_flash, logical_sector, &block, &block_status);
    
    /* Check return status.   */
    if(status != LX_SUCCESS)
    {

        /* Call system error handler.  */
        _lx_nand_flash_system_error(nand_flash, status, block, 0);

        /* Determine if the error is fatal.  */
        if (status != LX_NAND_ERROR_CORRECTED)
        {
#ifdef LX_THREAD_SAFE_ENABLE

            /* Release the thread safe mutex.  */
            tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

            /* Return an error.  */
            return(LX_ERROR);
        }
    }

    /* Check if block is unmapped or block is full.  */
    if (block == LX_NAND_BLOCK_UNMAPPED || block_status & LX_NAND_BLOCK_STATUS_FULL)
    {

        /* Allocate a new block.  */
        status = _lx_nand_flash_block_allocate(nand_flash, &new_block);

        /* Check if there is no blocks.  */
        if (status == LX_NO_BLOCKS)
        {
#ifdef LX_THREAD_SAFE_ENABLE

            /* Release the thread safe mutex.  */
            tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

            /* Return error.  */
            return(status);
        }

        /* Check return status.  */
        else if (status != LX_SUCCESS)
        {

            /* Call system error handler.  */
            _lx_nand_flash_system_error(nand_flash, status, new_block, 0);

            /* Determine if the error is fatal.  */
            if (status != LX_NAND_ERROR_CORRECTED)
            {
#ifdef LX_THREAD_SAFE_ENABLE

                /* Release the thread safe mutex.  */
                tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

                /* Return an error.  */
                return(LX_ERROR);
            }
        }

        /* Check if the block is full.  */
        if (block_status & LX_NAND_BLOCK_STATUS_FULL)
        {

            /* Set copy block flag.  */
            copy_block = LX_TRUE;
        }

        /* Set update mapping flag.  */
        update_mapping = LX_TRUE;
    }
    else
    {

        /* Set new block to the same as old block.  */
        new_block = block;
        new_block_status = block_status;
    }
    
    /* Check if copy block flag is set.  */
    if (copy_block)
    {

        /* Remove the old block from mapped block list.  */
        _lx_nand_flash_mapped_block_list_remove(nand_flash, logical_sector / nand_flash -> lx_nand_flash_pages_per_block);

        /* Copy valid sector to new block.  */
        status =  _lx_nand_flash_data_page_copy(nand_flash, logical_sector - (logical_sector % nand_flash -> lx_nand_flash_pages_per_block), block, block_status, new_block, &new_block_status, (logical_sector % nand_flash -> lx_nand_flash_pages_per_block));

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
    }
    
    /* Check if update mapping flag is set.  */
    if (update_mapping)
    {

        /* Update block mapping.  */
        _lx_nand_flash_block_mapping_set(nand_flash, logical_sector, new_block);
    }

    /* Setup spare buffer pointer.  */
    spare_buffer_ptr = nand_flash -> lx_nand_flash_page_buffer;

    /* Set spare buffer to all 0xFF bytes.  */
    LX_MEMSET(spare_buffer_ptr, 0xFF, nand_flash -> lx_nand_flash_spare_total_length);
                
    /* Check if there is enough spare data for metadata block number.  */
    if (nand_flash -> lx_nand_flash_spare_data2_length >= sizeof(USHORT))
    {

        /* Save metadata block number in spare bytes.  */
        LX_UTILITY_SHORT_SET(&spare_buffer_ptr[nand_flash -> lx_nand_flash_spare_data2_offset], nand_flash -> lx_nand_flash_metadata_block_number);
    }

    /* Set page type and sector address.  */
    LX_UTILITY_LONG_SET(&spare_buffer_ptr[nand_flash -> lx_nand_flash_spare_data1_offset], LX_NAND_PAGE_TYPE_USER_DATA | logical_sector);
    
    /* Get page to write.  */
    page = new_block_status & LX_NAND_BLOCK_STATUS_PAGE_NUMBER_MASK;

    /* Write the page.  */
#ifdef LX_NAND_ENABLE_CONTROL_BLOCK_FOR_DRIVER_INTERFACE
    status = (nand_flash -> lx_nand_flash_driver_pages_write)(nand_flash, new_block, page, (UCHAR*)buffer, spare_buffer_ptr, 1);
#else
    status = (nand_flash -> lx_nand_flash_driver_pages_write)(new_block, page, (UCHAR*)buffer, spare_buffer_ptr, 1);
#endif

    /* Check for an error from flash driver.   */
    if (status)
    {

        /* Call system error handler.  */
        _lx_nand_flash_system_error(nand_flash, status, new_block, 0);
#ifdef LX_THREAD_SAFE_ENABLE

        /* Release the thread safe mutex.  */
        tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

        /* Return an error.  */
        return(LX_ERROR);
    }

    /* Determine if the sector number is sequential.  */
    if ((new_block_status & LX_NAND_BLOCK_STATUS_PAGE_NUMBER_MASK) != (logical_sector % nand_flash -> lx_nand_flash_pages_per_block))
    {

        /* Set non sequential status flag.  */
        new_block_status |= LX_NAND_BLOCK_STATUS_NON_SEQUENTIAL;
    }

    /* Increase page number.  */
    page++;

    /* Check if page number reaches total pages per block.  */
    if (page == nand_flash -> lx_nand_flash_pages_per_block)
    {

        /* Set block full status flag.  */
        new_block_status |= LX_NAND_BLOCK_STATUS_FULL;
    }

    /* Build block status word.  */
    new_block_status = (USHORT)(page | (new_block_status & ~LX_NAND_BLOCK_STATUS_PAGE_NUMBER_MASK));

    /* Determine if there are sectors after the addressed sector need to be copied.  */
    if (copy_block && ((logical_sector % nand_flash -> lx_nand_flash_pages_per_block) < (nand_flash -> lx_nand_flash_pages_per_block - 1)))
    {

        /* Copy valid sector to new block.  */
        status = _lx_nand_flash_data_page_copy(nand_flash, logical_sector + 1, block, block_status, new_block, &new_block_status, (nand_flash -> lx_nand_flash_pages_per_block - 1) - (logical_sector % nand_flash -> lx_nand_flash_pages_per_block));

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
    }

    /* Set new block status.  */
    status = _lx_nand_flash_block_status_set(nand_flash, new_block, new_block_status);

    /* Check for an error from flash driver.   */
    if (status)
    {

        /* Call system error handler.  */
        _lx_nand_flash_system_error(nand_flash, status, new_block, 0);
#ifdef LX_THREAD_SAFE_ENABLE

        /* Release the thread safe mutex.  */
        tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

        /* Return an error.  */
        return(LX_ERROR);
    }

    /* Check if copy block flag is set.  */
    if (copy_block)
    {

        /* Erase old block.  */
        status = _lx_nand_flash_driver_block_erase(nand_flash, block, nand_flash -> lx_nand_flash_base_erase_count + nand_flash -> lx_nand_flash_erase_count_table[block] + 1);

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

        /* Update erase count for the old block.  */
        status = _lx_nand_flash_erase_count_set(nand_flash, block, (UCHAR)(nand_flash -> lx_nand_flash_erase_count_table[block] + 1));

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

            /* Set the block status to free.  */
            status = _lx_nand_flash_block_status_set(nand_flash, block, LX_NAND_BLOCK_STATUS_FREE);

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

            /* Add the block to free block list.  */
            _lx_nand_flash_free_block_list_add(nand_flash, block);
        }
    }

    /* Check if update mapping flag is set.  */
    if (update_mapping)
    {

        /* Add the new block to mapped block list.  */
        _lx_nand_flash_mapped_block_list_add(nand_flash, logical_sector / nand_flash -> lx_nand_flash_pages_per_block);
    }

#ifdef LX_THREAD_SAFE_ENABLE

    /* Release the thread safe mutex.  */
    tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

    /* Return the completion status.  */
    return(status);
}


