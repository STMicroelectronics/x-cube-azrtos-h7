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
/*    _lx_nand_flash_data_page_copy                       PORTABLE C      */ 
/*                                                           6.4.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Xiuwen Cai, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function copies logical sectors into new block.                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    logical_sector                        Logical sector number         */ 
/*    source_block                          Source block number           */ 
/*    src_block_status                      Source block status           */
/*    destination_block                     Destination block number      */ 
/*    dest_block_status_ptr                 Pointer to destination block  */ 
/*                                            status                      */
/*    sectors                               Number of sectors to copy     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    lx_nand_flash_driver_pages_copy       Driver pages copy             */ 
/*    lx_nand_flash_driver_pages_read       Driver pages read             */ 
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
/*  03-08-2023     Xiuwen Cai               Initial Version 6.2.1         */
/*  12-31-2023     Xiuwen Cai               Modified comment(s),          */
/*                                            fixed sequential checking   */
/*                                            logic,                      */
/*                                            resulting in version 6.4.0  */
/*                                                                        */
/**************************************************************************/
UINT  _lx_nand_flash_data_page_copy(LX_NAND_FLASH* nand_flash, ULONG logical_sector, ULONG source_block, USHORT src_block_status,
                                    ULONG destination_block, USHORT* dest_block_status_ptr, ULONG sectors)
{

LONG    source_page;
ULONG   destination_page;
UINT    status = LX_SUCCESS;
UINT    i;
ULONG   available_pages;
ULONG   spare_data1;
UCHAR  *spare_buffer_ptr;
ULONG   dest_block_status;
ULONG   number_of_pages;


    /* Get the destination block status.  */
    dest_block_status = *dest_block_status_ptr;

    /* Get the current page number of the destination block.  */
    destination_page = dest_block_status & LX_NAND_BLOCK_STATUS_PAGE_NUMBER_MASK;

    /* Get the available pages in the source block.  */
    available_pages = (src_block_status & LX_NAND_BLOCK_STATUS_FULL) ? (nand_flash -> lx_nand_flash_pages_per_block) : (src_block_status & LX_NAND_BLOCK_STATUS_PAGE_NUMBER_MASK);

    /* Check if pages in the source block are sequential.  */
    if (src_block_status & LX_NAND_BLOCK_STATUS_NON_SEQUENTIAL)
    {

        /* Get buffer for spare data.  */
        spare_buffer_ptr = nand_flash -> lx_nand_flash_page_buffer + nand_flash -> lx_nand_flash_bytes_per_page;

        /* Loop to copy the data.  */
        for (i = 0; i < sectors; i++)
        {

            /* Loop to search the page.  */
            for (source_page = (LONG)(available_pages - 1); source_page >= 0; source_page--)
            {

                /* Read one page.  */
#ifdef LX_NAND_ENABLE_CONTROL_BLOCK_FOR_DRIVER_INTERFACE
                status = (nand_flash -> lx_nand_flash_driver_pages_read)(nand_flash, source_block, (ULONG)source_page, LX_NULL, spare_buffer_ptr, 1);
#else
                status = (nand_flash -> lx_nand_flash_driver_pages_read)(source_block, (ULONG)source_page, LX_NULL, spare_buffer_ptr, 1);
#endif

                /* Check for an error from flash driver.   */
                if (status)
                {

                    /* Call system error handler.  */
                    _lx_nand_flash_system_error(nand_flash, status, source_block, 0);

                    /* Return an error.  */
                    return(LX_ERROR);
                }

                /* Get the spare data.  */
                spare_data1 = LX_UTILITY_LONG_GET(&spare_buffer_ptr[nand_flash -> lx_nand_flash_spare_data1_offset]);

                /* Check the address match.  */
                if ((spare_data1 & LX_NAND_PAGE_TYPE_USER_DATA_MASK) == (logical_sector + i))
                {

                    /* Check if the page contains valid data.  */
                    if ((spare_data1 & (~LX_NAND_PAGE_TYPE_USER_DATA_MASK)) == (LX_NAND_PAGE_TYPE_USER_DATA))
                    {

                        /* Call the driver to copy the page.  */
#ifdef LX_NAND_ENABLE_CONTROL_BLOCK_FOR_DRIVER_INTERFACE
                        status = (nand_flash -> lx_nand_flash_driver_pages_copy)(nand_flash, source_block, (ULONG)source_page, destination_block, destination_page, 1, nand_flash -> lx_nand_flash_page_buffer);
#else
                        status = (nand_flash -> lx_nand_flash_driver_pages_copy)(source_block, (ULONG)source_page, destination_block, destination_page, 1, nand_flash -> lx_nand_flash_page_buffer);
#endif

                        /* Check for an error from flash driver.   */
                        if (status)
                        {

                            /* Call system error handler.  */
                            _lx_nand_flash_system_error(nand_flash, status, source_block, 0);

                            /* Return an error.  */
                            return(LX_ERROR);
                        }

                        /* Check if the pages in destination block is still sequential.  */
                        if (destination_page != ((logical_sector + i) % nand_flash -> lx_nand_flash_pages_per_block))
                        {
                            /* Mark the block status as non sequential.  */
                            dest_block_status |= LX_NAND_BLOCK_STATUS_NON_SEQUENTIAL;
                        }

                        /* Update the available pages.  */
                        destination_page++;

                        /* Check if available page count reaches pages per block.  */
                        if (destination_page == nand_flash -> lx_nand_flash_pages_per_block)
                        {

                            /* Set block full flag.  */
                            dest_block_status |= LX_NAND_BLOCK_STATUS_FULL;
                        }
                    }
                    break;
                }
            }
        }
    }
    else
    {

        /* Get the source page number.  */
        source_page = (LONG)(logical_sector % nand_flash -> lx_nand_flash_pages_per_block);

        /* Check if the page to copy is greater than the available pages.  */
        number_of_pages = ((ULONG)source_page + sectors) > available_pages ?  
            (available_pages > (ULONG)source_page ? available_pages - (ULONG)source_page : 0) : sectors;

        /* Check if there is any pages to be copied.  */
        if (number_of_pages)
        {

            /* Call the driver to copy pages.  */
#ifdef LX_NAND_ENABLE_CONTROL_BLOCK_FOR_DRIVER_INTERFACE
            status = (nand_flash -> lx_nand_flash_driver_pages_copy)(nand_flash, source_block, (ULONG)source_page, destination_block, destination_page, number_of_pages, nand_flash -> lx_nand_flash_page_buffer);
#else
            status = (nand_flash -> lx_nand_flash_driver_pages_copy)(source_block, (ULONG)source_page, destination_block, destination_page, number_of_pages, nand_flash -> lx_nand_flash_page_buffer);
#endif

            /* Check for an error from flash driver.   */
            if (status)
            {

                /* Call system error handler.  */
                _lx_nand_flash_system_error(nand_flash, status, source_block, 0);

                /* Return an error.  */
                return(LX_ERROR);
            }

            /* Check if the pages in destination block is still sequential.  */
            if ((ULONG)source_page != destination_page)
            {

                /* Mark the block status as non sequential.  */
                dest_block_status |= LX_NAND_BLOCK_STATUS_NON_SEQUENTIAL;
            }

            /* Update the available pages.  */
            destination_page += number_of_pages;

            /* Check if available page count reaches pages per block.  */
            if (destination_page == nand_flash -> lx_nand_flash_pages_per_block)
            {

                /* Set block full flag.  */
                dest_block_status |= LX_NAND_BLOCK_STATUS_FULL;
            }
        }
    }

    /* Return the block status.  */
    *dest_block_status_ptr = (USHORT)(destination_page | (dest_block_status & ~LX_NAND_BLOCK_STATUS_PAGE_NUMBER_MASK));

    /* Return status.  */
    return(status);
}

