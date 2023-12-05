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
/*    _lx_nand_flash_free_block_list_add                  PORTABLE C      */ 
/*                                                           6.2.1       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Xiuwen Cai, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds a block to free block list.                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    block                                 Block number                  */ 
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
UINT  _lx_nand_flash_free_block_list_add(LX_NAND_FLASH* nand_flash, ULONG block)
{

ULONG insert_position;
INT search_position;
UCHAR new_block_erase_count;


    /* Get insert position for the free block list.  */
    insert_position = nand_flash -> lx_nand_flash_free_block_list_tail;

    /* Check if the list if full.  */
    if (insert_position > nand_flash -> lx_nand_flash_mapped_block_list_head)
    {

        /* Return an error.  */
        return(LX_ERROR);
    }

    /* Get the erase count.  */
    new_block_erase_count = nand_flash -> lx_nand_flash_erase_count_table[block];

    /* Add one block to the free list.  */
    nand_flash -> lx_nand_flash_free_block_list_tail++;

    /* Initialize the search pointer.  */
    search_position = (INT)insert_position - 1;

    /* Loop to search the insert position by block erase count.  */
    while ((search_position >= 0) && 
           (nand_flash -> lx_nand_flash_erase_count_table[nand_flash -> lx_nand_flash_block_list[search_position]] < new_block_erase_count))
    {

        /* Move the item in the list.  */
        nand_flash -> lx_nand_flash_block_list[insert_position] = nand_flash -> lx_nand_flash_block_list[search_position];
        search_position--;
        insert_position--;
    }

    /* Insert the new block to the list.  */
    nand_flash -> lx_nand_flash_block_list[insert_position] = (USHORT)block;

    /* Return successful completion.  */
    return(LX_SUCCESS);
}

