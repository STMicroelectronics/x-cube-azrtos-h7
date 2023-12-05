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
/*    _lx_nand_flash_block_status_set                     PORTABLE C      */ 
/*                                                           6.2.1       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Xiuwen Cai, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets block status.                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    block                                 Block number                  */ 
/*    block_status                          Block status                  */ 
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
UINT  _lx_nand_flash_block_status_set(LX_NAND_FLASH *nand_flash, ULONG block, ULONG block_status)
{

UCHAR   page_number;
UINT    status;


    /* Save the block status to status table.  */
    nand_flash -> lx_nand_flash_block_status_table[block] = (USHORT)block_status;

    /* Get the page number to write.  */
    page_number = (UCHAR)(block * sizeof(*nand_flash -> lx_nand_flash_block_status_table) / nand_flash -> lx_nand_flash_bytes_per_page);

    /* Save the status table.  */
    status = _lx_nand_flash_metadata_write(nand_flash, ((UCHAR*)nand_flash -> lx_nand_flash_block_status_table) + 
                                            page_number * nand_flash -> lx_nand_flash_bytes_per_page, LX_NAND_PAGE_TYPE_BLOCK_STATUS_TABLE | page_number);

    /* Return status.  */
    return(status);
}

