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
/*    _lx_nand_flash_sector_release                       PORTABLE C      */ 
/*                                                           6.2.1       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Xiuwen Cai, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function releases a logical sector from being managed in the   */ 
/*    NAND flash.                                                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    logical_sector                        Logical sector number         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _lx_nand_flash_block_find             Find the mapped block         */ 
/*    lx_nand_flash_driver_pages_read       Read pages                    */ 
/*    _lx_nand_flash_block_allocate         Allocate block                */ 
/*    _lx_nand_flash_mapped_block_list_remove                             */
/*                                          Remove mapped block           */ 
/*    _lx_nand_flash_data_page_copy         Copy data pages               */ 
/*    _lx_nand_flash_free_block_list_add    Add free block to list        */
/*    _lx_nand_flash_block_mapping_set      Set block mapping             */ 
/*    _lx_nand_flash_driver_block_erase     Erase block                   */ 
/*    _lx_nand_flash_erase_count_set        Set erase count               */
/*    _lx_nand_flash_block_data_move        Move block data               */
/*    _lx_nand_flash_block_status_set       Set block status              */ 
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
UINT  _lx_nand_flash_sector_release(LX_NAND_FLASH *nand_flash, ULONG logical_sector)
{

UINT        status;
ULONG       block;
USHORT      block_status;
UCHAR       *spare_buffer_ptr;
ULONG       available_pages;
LONG        page;
UINT        release_sector = LX_FALSE;
ULONG       new_block;
USHORT      new_block_status;

#ifdef LX_THREAD_SAFE_ENABLE

    /* Obtain the thread safe mutex.  */
    tx_mutex_get(&nand_flash -> lx_nand_flash_mutex, TX_WAIT_FOREVER);
#endif

    /* Increment the number of release requests.  */
    nand_flash -> lx_nand_flash_diagnostic_sector_release_requests++;


    /* See if we can find the sector in the current mapping.  */
    status = _lx_nand_flash_block_find(nand_flash, logical_sector, &block, &block_status);

    /* Check return status.   */
    if (status != LX_SUCCESS)
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

    /* Determine if the block is mapped.  */
    if (block != LX_NAND_BLOCK_UNMAPPED)
    {

        /* Setup spare buffer pointer.  */
        spare_buffer_ptr = (UCHAR*)nand_flash -> lx_nand_flash_page_buffer;

        /* Get available pages in this block.  */
        available_pages = block_status & LX_NAND_BLOCK_STATUS_FULL ? nand_flash -> lx_nand_flash_pages_per_block : block_status & LX_NAND_BLOCK_STATUS_PAGE_NUMBER_MASK;

        /* Determine if the pages are recorded sequentially.  */
        if (block_status & LX_NAND_BLOCK_STATUS_NON_SEQUENTIAL)
        {

            /* Loop to search the logical page.  */
            for (page = (LONG)available_pages - 1; page >= 0; page--)
            {

                /* Read a page.  */
#ifdef LX_NAND_ENABLE_CONTROL_BLOCK_FOR_DRIVER_INTERFACE
                status = (nand_flash -> lx_nand_flash_driver_pages_read)(nand_flash, block, (ULONG)page, LX_NULL, spare_buffer_ptr, 1);
#else
                status = (nand_flash -> lx_nand_flash_driver_pages_read)(block, (ULONG)page, LX_NULL, spare_buffer_ptr, 1);
#endif

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

                /* Get the logical sector number from spare bytes, and check if it matches the addressed sector number.  */
                if ((LX_UTILITY_LONG_GET(&spare_buffer_ptr[nand_flash -> lx_nand_flash_spare_data1_offset]) & LX_NAND_PAGE_TYPE_USER_DATA_MASK) == logical_sector)
                {

                    /* Make sure the sector is not released.  */
                    if ((LX_UTILITY_LONG_GET(&spare_buffer_ptr[nand_flash -> lx_nand_flash_spare_data1_offset]) & (~LX_NAND_PAGE_TYPE_USER_DATA_MASK)) == (LX_NAND_PAGE_TYPE_USER_DATA))
                    {

                        /* Set release sector flag.  */
                        release_sector = LX_TRUE;
                    }
                }
            }
        }
        else
        {

            /* Check if the logical sector is available.  */
            if (logical_sector % nand_flash -> lx_nand_flash_pages_per_block < available_pages)
            {

                /* Set release sector flag.  */
                release_sector = LX_TRUE;
            }
        }

        /* Determine if the sector needs to be released.  */
        if (release_sector)
        {

            /* Check if the block is full.  */
            if (block_status & LX_NAND_BLOCK_STATUS_FULL)
            {

                /* Allocate a new block.  */
                status = _lx_nand_flash_block_allocate(nand_flash, &new_block);

                /* Check return status.   */
                if (status != LX_SUCCESS)
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

                /* Set new block status to allocated.  */
                new_block_status = LX_NAND_BLOCK_STATUS_ALLOCATED;

                /* Remove the old block from mapped block list.  */
                _lx_nand_flash_mapped_block_list_remove(nand_flash, logical_sector / nand_flash -> lx_nand_flash_pages_per_block);

                /* Copy valid sector to new block.  */
                status = _lx_nand_flash_data_page_copy(nand_flash, logical_sector - (logical_sector % nand_flash -> lx_nand_flash_pages_per_block), block, block_status, new_block, &new_block_status, (logical_sector % nand_flash -> lx_nand_flash_pages_per_block));

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

                /* Determine if there are sectors after the addressed sector need to be copied.  */
                if (logical_sector % nand_flash -> lx_nand_flash_pages_per_block < nand_flash -> lx_nand_flash_pages_per_block - 1)
                {

                    /* Copy valid sector to new block.  */
                    status = _lx_nand_flash_data_page_copy(nand_flash, logical_sector + 1, block, block_status, new_block, &new_block_status, (nand_flash -> lx_nand_flash_pages_per_block - 1) - (logical_sector % nand_flash -> lx_nand_flash_pages_per_block));

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
                }

                /* Check new block status to see if there is valid pages in the block.  */
                if ((new_block_status & LX_NAND_BLOCK_STATUS_PAGE_NUMBER_MASK) == 0)
                {

                    /* Add the block to free block list.  */
                    _lx_nand_flash_free_block_list_add(nand_flash, new_block);

                    /* Set new block to unmapped.  */
                    new_block = LX_NAND_BLOCK_UNMAPPED;
                }
                else
                {

                    /* Set new block status.  */
                    status = _lx_nand_flash_block_status_set(nand_flash, new_block, new_block_status);

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

                /* Update block mapping.  */
                _lx_nand_flash_block_mapping_set(nand_flash, logical_sector, new_block);

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

                /* Check if there is valid pages in the new block.  */
                if ((new_block_status & LX_NAND_BLOCK_STATUS_PAGE_NUMBER_MASK) != 0)
                {
                    
                    /* Add the new block to mapped block list.  */
                    _lx_nand_flash_mapped_block_list_add(nand_flash, logical_sector / nand_flash -> lx_nand_flash_pages_per_block);
                }
            }
            else
            {

                /* Set page buffer to all 0xFF bytes.  */
                LX_MEMSET(nand_flash -> lx_nand_flash_page_buffer, 0xFF, nand_flash -> lx_nand_flash_bytes_per_page + nand_flash -> lx_nand_flash_spare_total_length);
                
                /* Setup spare buffer pointer.  */
                spare_buffer_ptr = nand_flash -> lx_nand_flash_page_buffer + nand_flash -> lx_nand_flash_bytes_per_page;

                /* Check if there is enough spare data for metadata block number.  */
                if (nand_flash -> lx_nand_flash_spare_data2_length >= sizeof(USHORT))
                {

                    /* Save metadata block number in spare bytes.  */
                    LX_UTILITY_SHORT_SET(&spare_buffer_ptr[nand_flash -> lx_nand_flash_spare_data2_offset], nand_flash -> lx_nand_flash_metadata_block_number);
                }

                /* Set page type and sector address.  */
                LX_UTILITY_LONG_SET(&spare_buffer_ptr[nand_flash -> lx_nand_flash_spare_data1_offset], LX_NAND_PAGE_TYPE_USER_DATA_RELEASED | logical_sector);
                
                /* Write the page.  */
#ifdef LX_NAND_ENABLE_CONTROL_BLOCK_FOR_DRIVER_INTERFACE
                status = (nand_flash -> lx_nand_flash_driver_pages_write)(nand_flash, block, available_pages, (UCHAR*)nand_flash -> lx_nand_flash_page_buffer, spare_buffer_ptr, 1);
#else
                status = (nand_flash -> lx_nand_flash_driver_pages_write)(block, available_pages, (UCHAR*)nand_flash -> lx_nand_flash_page_buffer, spare_buffer_ptr, 1);
#endif

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

                /* Increase available pages count.  */
                available_pages++;

                /* Check if available pages count reaches total pages per block.  */
                if (available_pages == nand_flash -> lx_nand_flash_pages_per_block)
                {

                    /* Set block full status flag.  */
                    block_status |= LX_NAND_BLOCK_STATUS_FULL;
                }

                /* Build block status word.  */
                block_status = (USHORT)(available_pages | (block_status & ~LX_NAND_BLOCK_STATUS_PAGE_NUMBER_MASK));

                /* Set non sequential status flag.  */
                block_status |= LX_NAND_BLOCK_STATUS_NON_SEQUENTIAL;

                /* Set block status.  */
                status = _lx_nand_flash_block_status_set(nand_flash, block, block_status);

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
        }
    }
#ifdef LX_THREAD_SAFE_ENABLE

    /* Release the thread safe mutex.  */
    tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

    /* Return status.  */
    return(status);
}

