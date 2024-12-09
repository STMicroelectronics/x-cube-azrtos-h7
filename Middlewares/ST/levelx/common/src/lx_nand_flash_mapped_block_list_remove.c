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
/*    _lx_nand_flash_mapped_block_list_remove             PORTABLE C      */ 
/*                                                           6.2.1       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Xiuwen Cai, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function removes mapped block from list.                       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    block_mapping_index                   Block mapping index           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
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
UINT  _lx_nand_flash_mapped_block_list_remove(LX_NAND_FLASH* nand_flash, ULONG block_mapping_index)
{

ULONG search_position;


    /* Initialize the search pointer.  */
    search_position = nand_flash -> lx_nand_flash_mapped_block_list_head + 1;

    /* Loop to search the block in the list.  */
    while (search_position < nand_flash -> lx_nand_flash_block_list_size)
    {

        /* Check if there is a match in the list.  */
        if (nand_flash -> lx_nand_flash_block_list[search_position] == block_mapping_index)
        {

            /* Get out of the loop.  */
            break;
        }

        /* Move to next position.  */
        search_position++;
    }

    /* Check if the block is found.  */
    if (search_position < nand_flash -> lx_nand_flash_block_list_size)
    {

        /* Remove one item from the list.  */
        nand_flash -> lx_nand_flash_mapped_block_list_head++;

        /* Loop to move items in the list.  */
        while (search_position > nand_flash -> lx_nand_flash_mapped_block_list_head)
        {

            /* Move the item in the list.  */
            nand_flash -> lx_nand_flash_block_list[search_position] = nand_flash -> lx_nand_flash_block_list[search_position - 1];
            search_position--;
        }
    }
    else
    {

        /* Return error.  */
        return(LX_NO_BLOCKS);
    }

    /* Return successful completion.  */
    return(LX_SUCCESS);
}

