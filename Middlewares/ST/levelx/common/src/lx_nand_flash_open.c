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
/*    _lx_nand_flash_open                                 PORTABLE C      */ 
/*                                                           6.3.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Xiuwen Cai, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function opens a NAND flash instance and ensures the           */ 
/*    NAND flash is in a coherent state.                                  */ 
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
/*    _lx_nand_flash_memory_initialize      Initialize buffer             */ 
/*    _lx_nand_flash_driver_block_status_get                              */ 
/*                                          Get block status              */ 
/*    lx_nand_flash_driver_pages_read       Read pages                    */ 
/*    _lx_nand_flash_free_block_list_add    Add free block to list        */
/*    _lx_nand_flash_mapped_block_list_add  Add mapped block to list      */
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
UINT  _lx_nand_flash_open(LX_NAND_FLASH  *nand_flash, CHAR *name, UINT (*nand_driver_initialize)(LX_NAND_FLASH *),
                          ULONG* memory_ptr, UINT memory_size)
{

ULONG                       block;
ULONG                       page;
UCHAR                       block_status;
ULONG                       block_count;
UINT                        status;
LX_NAND_FLASH               *tail_ptr;
LX_NAND_DEVICE_INFO         *nand_device_info_page;
UCHAR                       *spare_buffer_ptr;
UCHAR                       *page_buffer_ptr;
ULONG                       page_type;
UCHAR                       page_index;
LX_INTERRUPT_SAVE_AREA

    LX_PARAMETER_NOT_USED(name);

    /* Clear the NAND flash control block. User extension is not cleared.  */
    LX_MEMSET(nand_flash, 0, (ULONG)((UCHAR*)&(nand_flash -> lx_nand_flash_open_previous) - (UCHAR*)nand_flash) + sizeof(nand_flash -> lx_nand_flash_open_previous));

    /* Call the flash driver's initialization function.  */
    (nand_driver_initialize)(nand_flash);

    /* Determine if we can support this NAND flash size.  */
    if (nand_flash -> lx_nand_flash_pages_per_block > LX_NAND_MAX_PAGE_PER_BLOCK || nand_flash -> lx_nand_flash_total_blocks > LX_NAND_MAX_BLOCK_COUNT)
    {

        /* Return an error.  */
        return(LX_ERROR);
    }

    /* Check if it is new LevelX NAND driver.  */
    if (nand_flash -> lx_nand_flash_driver_pages_read == LX_NULL || nand_flash -> lx_nand_flash_driver_pages_write == LX_NULL || nand_flash -> lx_nand_flash_driver_pages_copy == LX_NULL)
    {

        /* Return an error.  */
        return(LX_ERROR);
    }

    /* Check the spare data length.  */
    if (nand_flash -> lx_nand_flash_spare_data1_length < sizeof(ULONG))
    {

        /* Return an error.  */
        return(LX_ERROR);
    }

    /* Calculate the number of words per block and per page.  */
    nand_flash -> lx_nand_flash_words_per_page =   (nand_flash -> lx_nand_flash_bytes_per_page / sizeof(ULONG));
    nand_flash -> lx_nand_flash_words_per_block =  (nand_flash -> lx_nand_flash_words_per_page * nand_flash -> lx_nand_flash_pages_per_block);

    /* Calculate the total pages.  */
    nand_flash -> lx_nand_flash_total_pages =   nand_flash -> lx_nand_flash_total_blocks * nand_flash -> lx_nand_flash_pages_per_block;

    /* Initialize memory buffer.  */
    status = _lx_nand_flash_memory_initialize(nand_flash, memory_ptr, memory_size);
    if (status != LX_SUCCESS)
    {
        return(status);
    }

    /* Initialize block numbers.  */
    nand_flash -> lx_nand_flash_metadata_block_number = LX_NAND_BLOCK_UNMAPPED;
    nand_flash -> lx_nand_flash_backup_metadata_block_number = LX_NAND_BLOCK_UNMAPPED;

    /* Setup page buffer and spare buffer pointers.  */
    page_buffer_ptr = nand_flash -> lx_nand_flash_page_buffer;
    spare_buffer_ptr = page_buffer_ptr + nand_flash -> lx_nand_flash_bytes_per_page;

    /* Loop through the blocks to check for bad blocks and determine the minimum and maximum erase count for each good block.  */
    for (block = 0; block < nand_flash -> lx_nand_flash_total_blocks; block++)
    {
    
        /* First, check to make sure this block is good.  */
        status =  _lx_nand_flash_driver_block_status_get(nand_flash, block, &block_status);

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

        /* Call driver read function to read page 0.  */
#ifdef LX_NAND_ENABLE_CONTROL_BLOCK_FOR_DRIVER_INTERFACE
        status = (nand_flash -> lx_nand_flash_driver_pages_read)(nand_flash, block, 0, page_buffer_ptr, spare_buffer_ptr, 1);
#else
        status = (nand_flash -> lx_nand_flash_driver_pages_read)(block, 0, page_buffer_ptr, spare_buffer_ptr, 1);
#endif

        /* Check for an error from flash driver.   */
        if (status)
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

        /* Get the page type.  */
        page_type = LX_UTILITY_LONG_GET(&spare_buffer_ptr[nand_flash -> lx_nand_flash_spare_data1_offset]);

        /* Check if the type is device info.  */
        if (page_type == LX_NAND_PAGE_TYPE_DEVICE_INFO)
        {

            /* Get the device info page.  */
            nand_device_info_page = (LX_NAND_DEVICE_INFO*)page_buffer_ptr;

            /* Check signature.  */
            if (nand_device_info_page -> lx_nand_device_info_signature1 == LX_NAND_DEVICE_INFO_SIGNATURE1 &&
                nand_device_info_page -> lx_nand_device_info_signature2 == LX_NAND_DEVICE_INFO_SIGNATURE2)
            {

                /* Save the block numbers.  */
                nand_flash -> lx_nand_flash_metadata_block_number = nand_device_info_page -> lx_nand_device_info_metadata_block_number;
                nand_flash -> lx_nand_flash_backup_metadata_block_number = nand_device_info_page -> lx_nand_device_info_backup_metadata_block_number;
                break;
            }

        }

    }    

    /* Check if we have found the metadata block.  */
    if (nand_flash -> lx_nand_flash_metadata_block_number == LX_NAND_BLOCK_UNMAPPED)
    {

        /* Not found, return an error.  */
        return (LX_ERROR);
    }

    /* Initialize metadata block numbers and lists.  */
    nand_flash -> lx_nand_flash_metadata_block_number_current = nand_flash -> lx_nand_flash_metadata_block_number;
    nand_flash -> lx_nand_flash_backup_metadata_block_number_current = nand_flash -> lx_nand_flash_backup_metadata_block_number;
    nand_flash -> lx_nand_flash_metadata_block[0] = (USHORT)nand_flash -> lx_nand_flash_metadata_block_number;
    nand_flash -> lx_nand_flash_backup_metadata_block[0] = (USHORT)nand_flash -> lx_nand_flash_backup_metadata_block_number;

    /* Found one metadata block.  */
    nand_flash -> lx_nand_flash_metadata_block_count = 1;

    /* Clear searched block count.  */
    block_count = 0;

    do
    { 

        /* Initialize next block to unmapped.  */
        nand_flash -> lx_nand_flash_metadata_block_number_next = LX_NAND_BLOCK_UNMAPPED;
        nand_flash -> lx_nand_flash_backup_metadata_block_number_next = LX_NAND_BLOCK_UNMAPPED;

        /* Loop to read pages in the metadata block.  */        
        for (page = 0; page < nand_flash -> lx_nand_flash_pages_per_block ; page++)
        {

            /* Call driver read function to read page.  */
#ifdef LX_NAND_ENABLE_CONTROL_BLOCK_FOR_DRIVER_INTERFACE
            status = (nand_flash -> lx_nand_flash_driver_pages_read)(nand_flash, block, page, page_buffer_ptr, spare_buffer_ptr, 1);
#else
            status = (nand_flash -> lx_nand_flash_driver_pages_read)(block, page, page_buffer_ptr, spare_buffer_ptr, 1);
#endif

            /* Check for an error from flash driver.   */
            if (status)
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

            /* Get page type and page index.  */
            page_type = LX_UTILITY_LONG_GET(&spare_buffer_ptr[nand_flash -> lx_nand_flash_spare_data1_offset]) & (~LX_NAND_PAGE_TYPE_PAGE_NUMBER_MASK);
            page_index = LX_UTILITY_LONG_GET(&spare_buffer_ptr[nand_flash -> lx_nand_flash_spare_data1_offset]) & LX_NAND_PAGE_TYPE_PAGE_NUMBER_MASK;

            /* Process metadata page by type.  */
            switch (page_type)
            {
            case LX_NAND_PAGE_TYPE_DEVICE_INFO:

                /* This page is for device info.  */
                nand_device_info_page = (LX_NAND_DEVICE_INFO*)page_buffer_ptr;

                /* Get the base erase count.  */
                nand_flash -> lx_nand_flash_base_erase_count = nand_device_info_page -> lx_nand_device_info_base_erase_count;
                break;

            case LX_NAND_PAGE_TYPE_ERASE_COUNT_TABLE:

                /* Check if page index is valid.  */
                if (((ULONG)page_index + 1) * nand_flash -> lx_nand_flash_bytes_per_page > nand_flash -> lx_nand_flash_erase_count_table_size)
                {

                    /* Invalid page index. Return an error.  */
                    status = LX_ERROR;
                    break;
                }

                /* Copy page data to erase count table.  */
                LX_MEMCPY(nand_flash -> lx_nand_flash_erase_count_table + page_index * nand_flash -> lx_nand_flash_bytes_per_page, /* Use case of memcpy is verified. */
                            page_buffer_ptr, nand_flash -> lx_nand_flash_bytes_per_page);
                break;

            case LX_NAND_PAGE_TYPE_BLOCK_MAPPING_TABLE:

                /* Check if page index is valid.  */
                if (((ULONG)page_index + 1) * nand_flash -> lx_nand_flash_bytes_per_page > nand_flash -> lx_nand_flash_block_mapping_table_size)
                {

                    /* Invalid page index. Return an error.  */
                    status = LX_ERROR;
                    break;
                }

                /* Copy page data to block mapping table.  */
                LX_MEMCPY(nand_flash -> lx_nand_flash_block_mapping_table + page_index * nand_flash -> lx_nand_flash_bytes_per_page / sizeof(*nand_flash -> lx_nand_flash_block_mapping_table), /* Use case of memcpy is verified. */
                    page_buffer_ptr, nand_flash -> lx_nand_flash_bytes_per_page);
                break;

            case LX_NAND_PAGE_TYPE_BLOCK_STATUS_TABLE:

                /* Check if page index is valid.  */
                if (((ULONG)page_index + 1) * nand_flash -> lx_nand_flash_bytes_per_page > nand_flash -> lx_nand_flash_block_status_table_size)
                {

                    /* Invalid page index. Return an error.  */
                    status = LX_ERROR;
                    break;
                }

                /* Copy page data to block status table.  */
                LX_MEMCPY(nand_flash -> lx_nand_flash_block_status_table + page_index * nand_flash -> lx_nand_flash_bytes_per_page / sizeof(*nand_flash -> lx_nand_flash_block_status_table), /* Use case of memcpy is verified. */
                    page_buffer_ptr, nand_flash -> lx_nand_flash_bytes_per_page);
                break;

            case LX_NAND_PAGE_TYPE_FREE_PAGE:

                /* Found a free page. Update current page.  */
                nand_flash -> lx_nand_flash_metadata_block_current_page = page;
                nand_flash -> lx_nand_flash_backup_metadata_block_current_page = page;

                /* Skip all the remaining pages.  */
                page = nand_flash -> lx_nand_flash_pages_per_block;
                break;

            case LX_NAND_PAGE_TYPE_BLOCK_LINK:

                /* Found next blocks. Update next block numbers.  */
                nand_flash -> lx_nand_flash_metadata_block_number_next = LX_UTILITY_LONG_GET(&page_buffer_ptr[LX_NAND_BLOCK_LINK_MAIN_METADATA_OFFSET]);
                nand_flash -> lx_nand_flash_backup_metadata_block_number_next = LX_UTILITY_LONG_GET(&page_buffer_ptr[LX_NAND_BLOCK_LINK_BACKUP_METADATA_OFFSET]);

                /* Save block numbers in metadata block lists.  */
                nand_flash -> lx_nand_flash_metadata_block[nand_flash -> lx_nand_flash_metadata_block_count] = (USHORT)LX_UTILITY_LONG_GET(&page_buffer_ptr[LX_NAND_BLOCK_LINK_MAIN_METADATA_OFFSET]);
                nand_flash -> lx_nand_flash_backup_metadata_block[nand_flash -> lx_nand_flash_metadata_block_count] = (USHORT)LX_UTILITY_LONG_GET(&page_buffer_ptr[LX_NAND_BLOCK_LINK_BACKUP_METADATA_OFFSET]);

                /* Increase metadata block count.  */
                nand_flash -> lx_nand_flash_metadata_block_count++;

                break;

            default:

                /* Unknown type, return error.  */
                status = LX_ERROR;
            }
            
            /* Check status.  */
            if (status == LX_ERROR)
            {

                /* Error, break the loop.  */
                break;
            }

        }

        /* Check if we have reached the last page.  */
        if (page == nand_flash -> lx_nand_flash_pages_per_block)
        {

            /* Move to next block.  */
            nand_flash -> lx_nand_flash_metadata_block_number_current = nand_flash -> lx_nand_flash_metadata_block_number_next;
            nand_flash -> lx_nand_flash_backup_metadata_block_number_current = nand_flash -> lx_nand_flash_backup_metadata_block_number_next;

            /* Make sure the block is valid.  */
            if (nand_flash -> lx_nand_flash_metadata_block_number_current == LX_NAND_BLOCK_UNMAPPED)
            {

                /* Error, break the loop.  */
                break;
            }

            /* Get the block to process.  */
            block = nand_flash -> lx_nand_flash_metadata_block_number_current;
        }

        /* Increase the processed block number.  */
        block_count++;

        /* If block count is larger than total blocks, there is an error.  */
        if (block_count >= nand_flash -> lx_nand_flash_total_blocks)
        {

            /* Break the loop.  */
            break;
        }
    } while (nand_flash -> lx_nand_flash_metadata_block_current_page == 0 && status != LX_ERROR);

    /* Check if metadata page is processed correctly.  */
    if (nand_flash -> lx_nand_flash_metadata_block_current_page == 0 || status == LX_ERROR)
    {

        /* Return an error.  */
        return(LX_ERROR);
    }

    /* Loop to build free and mapped block lists.  */
    for (block = 0; block < nand_flash -> lx_nand_flash_total_blocks; block++)
    {

        /* Check for free blocks.  */
        if (nand_flash -> lx_nand_flash_block_status_table[block] == LX_NAND_BLOCK_STATUS_FREE)
        {

            /* Add the block to free block list.  */
            _lx_nand_flash_free_block_list_add(nand_flash, block);
        }
        
        /* Check for mapped blocks.  */
        if (nand_flash -> lx_nand_flash_block_mapping_table[block] != LX_NAND_BLOCK_UNMAPPED)
        {

            /* Add the block to free block list.  */
            _lx_nand_flash_mapped_block_list_add(nand_flash, block);
        }
    }


#ifdef LX_THREAD_SAFE_ENABLE

    /* If the thread safe option is enabled, create a ThreadX mutex that will be used in all external APIs 
       in order to provide thread-safe operation.  */
    status =  tx_mutex_create(&nand_flash -> lx_nand_flash_mutex, "NAND Flash Mutex", TX_NO_INHERIT);

    /* Determine if the mutex creation encountered an error.  */
    if (status != LX_SUCCESS)
    {
    
        /* Call system error handler, since this should not happen.  */
        _lx_nand_flash_system_error(nand_flash, LX_SYSTEM_MUTEX_CREATE_FAILED, 0, 0);
    
        /* Return error to caller.  */
        return(LX_ERROR);
    }
#endif

    /* Lockout interrupts.  */
    LX_DISABLE

    /* At this point, the NAND flash has been opened successfully.  Place the 
       NAND flash control block on the linked list of currently opened NAND flashes.  */

    /* Set the NAND flash state to open.  */
    nand_flash -> lx_nand_flash_state =  LX_NAND_FLASH_OPENED;

    /* Place the NAND flash control block on the list of opened NAND flashes.  First,
       check for an empty list.  */
    if (_lx_nand_flash_opened_count)
    {

        /* List is not empty - other NAND flashes are open.  */

        /* Pickup tail pointer.  */
        tail_ptr =  _lx_nand_flash_opened_ptr -> lx_nand_flash_open_previous;

        /* Place the new NAND flash control block in the list.  */
        _lx_nand_flash_opened_ptr -> lx_nand_flash_open_previous =  nand_flash;
        tail_ptr -> lx_nand_flash_open_next =                       nand_flash;

        /* Setup this NAND flash's opened links.  */
        nand_flash -> lx_nand_flash_open_previous =  tail_ptr;
        nand_flash -> lx_nand_flash_open_next =      _lx_nand_flash_opened_ptr;   
    }
    else
    {

        /* The opened NAND flash list is empty.  Add the NAND flash to empty list.  */
        _lx_nand_flash_opened_ptr =                 nand_flash;
        nand_flash -> lx_nand_flash_open_next =      nand_flash;
        nand_flash -> lx_nand_flash_open_previous =  nand_flash;
    }

    /* Increment the opened NAND flash counter.  */
    _lx_nand_flash_opened_count++;

    /* Restore interrupts.  */
    LX_RESTORE

    /* Return a successful completion.  */
    return(LX_SUCCESS);
}

