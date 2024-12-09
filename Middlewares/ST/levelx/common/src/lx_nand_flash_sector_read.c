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
/*    _lx_nand_flash_sector_read                          PORTABLE C      */ 
/*                                                           6.2.1       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Xiuwen Cai, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function reads a logical sector from NAND flash.               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    logical_sector                        Logical sector number         */ 
/*    buffer                                Pointer to buffer to read into*/ 
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
/*    lx_nand_flash_driver_pages_read       Read pages                    */ 
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
UINT  _lx_nand_flash_sector_read(LX_NAND_FLASH *nand_flash, ULONG logical_sector, VOID *buffer)
{

UINT        status;
ULONG       i;
ULONG       *word_ptr;
ULONG       block;
USHORT      block_status;
UCHAR       *spare_buffer_ptr;
ULONG       available_pages;
LONG        page;

#ifdef LX_THREAD_SAFE_ENABLE

    /* Obtain the thread safe mutex.  */
    tx_mutex_get(&nand_flash -> lx_nand_flash_mutex, TX_WAIT_FOREVER);
#endif

    /* Increment the number of read requests.  */
    nand_flash -> lx_nand_flash_diagnostic_sector_read_requests++;

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
                status = (nand_flash -> lx_nand_flash_driver_pages_read)(nand_flash, block, (ULONG)page, (UCHAR*)buffer, spare_buffer_ptr, 1);
#else
                status = (nand_flash -> lx_nand_flash_driver_pages_read)(block, (ULONG)page, (UCHAR*)buffer, spare_buffer_ptr, 1);
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
#ifdef LX_THREAD_SAFE_ENABLE

                    /* Release the thread safe mutex.  */
                    tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif
                    /* Return successful completion.  */
                    return(LX_SUCCESS);
                }
            }
        }
        else
        {

            /* Check if the logical sector is available.  */
            if (logical_sector % nand_flash -> lx_nand_flash_pages_per_block < available_pages)
            {

                /* Read a page.  */
#ifdef LX_NAND_ENABLE_CONTROL_BLOCK_FOR_DRIVER_INTERFACE
                status = (nand_flash -> lx_nand_flash_driver_pages_read)(nand_flash, block, logical_sector % nand_flash -> lx_nand_flash_pages_per_block, (UCHAR*)buffer, spare_buffer_ptr, 1);
#else
                status = (nand_flash -> lx_nand_flash_driver_pages_read)(block, logical_sector % nand_flash -> lx_nand_flash_pages_per_block, (UCHAR*)buffer, spare_buffer_ptr, 1);
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
#ifdef LX_THREAD_SAFE_ENABLE

                /* Release the thread safe mutex.  */
                tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif
                /* Return successful completion.  */
                return(LX_SUCCESS);
            }
        }
    }

    /* Sector hasn't been written. Simply fill the destination buffer with ones and return success.  */

    /* Setup pointer to users buffer.  */
    word_ptr =  (ULONG *) buffer;
    
    /* Put all ones in he buffer.  */
    for (i = 0; i < nand_flash -> lx_nand_flash_words_per_page; i++)
    {
    
        /* Copy a word.  */
        *word_ptr++ =  LX_ALL_ONES;
    }       

    /* Set the status to success.  */
    status =  LX_SUCCESS;

#ifdef LX_THREAD_SAFE_ENABLE

    /* Release the thread safe mutex.  */
    tx_mutex_put(&nand_flash -> lx_nand_flash_mutex);
#endif

    /* Return status.  */
    return(status);
}

