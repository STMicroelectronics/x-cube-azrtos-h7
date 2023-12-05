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
/*    _lx_nand_flash_block_find                           PORTABLE C      */ 
/*                                                           6.2.1       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Xiuwen Cai, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function attempts to find the mapped block for the logical     */ 
/*    sector in the mapping table.                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    logical_sector                        Logical sector number         */ 
/*    superceded_check                      Check for page being          */ 
/*                                            superceded (can happen if   */ 
/*                                            on interruptions of page    */ 
/*                                            write)                      */ 
/*    block                                 Destination for block         */ 
/*    page                                  Destination for page          */ 
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
UINT  _lx_nand_flash_block_find(LX_NAND_FLASH *nand_flash, ULONG logical_sector, ULONG *block, USHORT *block_status)
{

 UINT    block_mapping_index;
 USHORT  block_number;


    /* Get the mapping index from logic sector address.  */
    block_mapping_index = logical_sector / nand_flash -> lx_nand_flash_pages_per_block;

    /* Check the address range.  */
    if (block_mapping_index > nand_flash -> lx_nand_flash_block_mapping_table_size / sizeof(*nand_flash -> lx_nand_flash_block_mapping_table))
    {

        /* Out of range, return an error. */
        return(LX_ERROR);
    }

    /* Get the block number from mapping table.  */
    block_number = nand_flash -> lx_nand_flash_block_mapping_table[block_mapping_index];

    /* Check if it is mapped.  */
    if (block_number != LX_NAND_BLOCK_UNMAPPED)
    {

        /* Return the block status.  */
        *block_status = nand_flash -> lx_nand_flash_block_status_table[block_number];
    }

    /* Return the block number.  */
    *block = (ULONG)block_number;

    /* Return successful completion.  */
    return(LX_SUCCESS);
}

