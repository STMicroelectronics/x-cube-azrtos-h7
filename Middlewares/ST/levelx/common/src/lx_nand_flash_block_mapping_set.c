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
/*    _lx_nand_flash_block_mapping_set                    PORTABLE C      */ 
/*                                                           6.2.1       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Xiuwen Cai, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets block mappings.                                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    logical_sector                        Logical sector number         */ 
/*    block                                 Block number                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _lx_nand_flash_metadata_write         Write metadata                */ 
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
UINT  _lx_nand_flash_block_mapping_set(LX_NAND_FLASH *nand_flash, ULONG logical_sector, ULONG block)
{

UINT    block_mapping_index;
UCHAR   page_number;
UINT    status;


    /* Get the mapping index from logic sector address.  */
    block_mapping_index = logical_sector / nand_flash -> lx_nand_flash_pages_per_block;

    /* Check the address range.  */
    if (block_mapping_index > nand_flash -> lx_nand_flash_block_mapping_table_size / sizeof(*nand_flash -> lx_nand_flash_block_mapping_table))
    {

        /* Out of range, return an error. */
        return(LX_ERROR);
    }

    /* Save the block number to mapping table.  */
    nand_flash -> lx_nand_flash_block_mapping_table[block_mapping_index] = (USHORT)block;

    /* Get the page number to write.  */
    page_number = (UCHAR)(block_mapping_index * sizeof(*nand_flash -> lx_nand_flash_block_mapping_table) / nand_flash -> lx_nand_flash_bytes_per_page);

    /* Save the mapping table.  */
    status = _lx_nand_flash_metadata_write(nand_flash, ((UCHAR*)nand_flash -> lx_nand_flash_block_mapping_table) + 
                                                page_number * nand_flash -> lx_nand_flash_bytes_per_page, 
                                                LX_NAND_PAGE_TYPE_BLOCK_MAPPING_TABLE | page_number);

    /* Return status.  */
    return(status);
}

