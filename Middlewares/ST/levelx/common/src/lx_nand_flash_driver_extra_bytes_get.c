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
/*    _lx_nand_flash_driver_extra_bytes_get               PORTABLE C      */ 
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function calls the driver to get the extra bytes of a NAND     */ 
/*    page.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    block                                 Block number                  */ 
/*    page                                  Page number                   */ 
/*    destination                           Pointer to destination buffer */ 
/*    words                                 Number of words to read       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    (lx_nand_flash_driver_extra_bytes_get)Get extra bytes from spare    */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Internal LevelX                                                     */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
UINT  _lx_nand_flash_driver_extra_bytes_get(LX_NAND_FLASH *nand_flash, ULONG block, ULONG page, UCHAR *destination, UINT size)
{

ULONG                   *source_ptr;
ULONG                   *destination_ptr;
ULONG                   cache_index;
UINT                    status;


    /* Determine if the page extra bytes cache is disabled.  */
    if (nand_flash -> lx_nand_flash_page_extra_bytes_cache == LX_NULL)
    {

        /* Increment the page extra bytes get count.  */
        nand_flash -> lx_nand_flash_diagnostic_page_extra_bytes_gets++;

        /* Call driver extra bytes get function.  */
        status =  (nand_flash -> lx_nand_flash_driver_extra_bytes_get)(block, page, destination, size);
    }
    else
    {
    
        /* Calculate the cache index.  */
        cache_index =  (block * nand_flash -> lx_nand_flash_pages_per_block) + page;

        /* Setup the destination pointer.  */
        destination_ptr =  (ULONG *) destination;
        
        /* Determine if this cache entry is valid.  */
        if (nand_flash -> lx_nand_flash_page_extra_bytes_cache[cache_index].lx_nand_page_extra_info_logical_sector != 0)
        {
    
            /* Simply return this value.  */
            *destination_ptr =  nand_flash -> lx_nand_flash_page_extra_bytes_cache[cache_index].lx_nand_page_extra_info_logical_sector;
        
            /* Increment the number of page extra bytes cache hits.  */
            nand_flash -> lx_nand_flash_diagnostic_page_extra_bytes_cache_hits++;
        
            /* Return successful status.  */
            status =  LX_SUCCESS;
        }
        else
        {
        
            /* Increment the page extra bytes get count.  */
            nand_flash -> lx_nand_flash_diagnostic_page_extra_bytes_gets++;

            /* Call driver extra bytes get function.  */
            status =  (nand_flash -> lx_nand_flash_driver_extra_bytes_get)(block, page, destination, size);

            /* Increment the number of page extra bytes cache misses.  */
            nand_flash -> lx_nand_flash_diagnostic_page_extra_bytes_cache_misses++;

            /* Setup destination pointer.  */
            destination_ptr =  &nand_flash -> lx_nand_flash_page_extra_bytes_cache[cache_index].lx_nand_page_extra_info_logical_sector;
    
            /* Setup source pointer.  */
            source_ptr =  (ULONG *) destination;
        
            /* Save the value in the page extra bytes cache.  */
            *destination_ptr =  *source_ptr;
        }
    }
    
    /* Return status.  */
    return(status);
}


