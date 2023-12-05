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
/*    _lx_nand_flash_block_data_move                      PORTABLE C      */ 
/*                                                           6.2.1       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Xiuwen Cai, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function finds a block with less erase count and copies pages  */ 
/*    into new block.                                                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    new_block                             New block number              */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _lx_nand_flash_mapped_block_list_get  Get mapped block index        */ 
/*    _lx_nand_flash_block_find             Find the mapped block         */ 
/*    _lx_nand_flash_data_page_copy         Copy data pages               */ 
/*    _lx_nand_flash_free_block_list_add    Add free block to list        */
/*    _lx_nand_flash_block_status_set       Set block status              */ 
/*    _lx_nand_flash_block_mapping_set      Set block mapping             */ 
/*    _lx_nand_flash_mapped_block_list_add  Add mapped block to list      */
/*    _lx_nand_flash_driver_block_erase     Erase block                   */ 
/*    _lx_nand_flash_erase_count_set        Set erase count               */
/*    _lx_nand_flash_free_block_list_add    Add free block to list        */
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
UINT  _lx_nand_flash_block_data_move(LX_NAND_FLASH *nand_flash, ULONG new_block)
{

UINT        status;
ULONG       block;
USHORT      block_status;
USHORT      new_block_status;
ULONG       block_mapping_index;


    /* Get the mapped address to move the data.  */
    status = _lx_nand_flash_mapped_block_list_get(nand_flash, &block_mapping_index);

    if (status)
    {

        /* Return an error.  */
        return(LX_ERROR);
    }

    /* Find the block number to be moved.  */
    status = _lx_nand_flash_block_find(nand_flash, block_mapping_index * nand_flash -> lx_nand_flash_pages_per_block, &block, &block_status);

    /* Check the status.  */
    if (status)
    {

        /* Return an error.  */
        return(LX_ERROR);
    }

    /* Set new block status to allocated for now.  */
    new_block_status = LX_NAND_BLOCK_STATUS_ALLOCATED;

    /* Copy data from old block to new block.  */
    status = _lx_nand_flash_data_page_copy(nand_flash, block_mapping_index * nand_flash -> lx_nand_flash_pages_per_block, block, block_status, new_block, &new_block_status, nand_flash -> lx_nand_flash_pages_per_block);

    /* Check the status.  */
    if (status)
    {

        /* Call system error handler.  */
        _lx_nand_flash_system_error(nand_flash, status, new_block, 0);

        /* Return an error.  */
        return(LX_ERROR);
    }

    /* Check if no page was written.  */
    if ((new_block_status & LX_NAND_BLOCK_STATUS_PAGE_NUMBER_MASK) == 0)
    {

        /* Mark the new block as free.  */
        new_block_status = LX_NAND_BLOCK_STATUS_FREE;

        /* Add the new block to free list.  */
        _lx_nand_flash_free_block_list_add(nand_flash, new_block);

    }

    /* Set the block status for the new block.  */
    status = _lx_nand_flash_block_status_set(nand_flash, new_block, new_block_status);

    /* Check for an error from flash driver.   */
    if (status)
    {

        /* Call system error handler.  */
        _lx_nand_flash_system_error(nand_flash, status, new_block, 0);

        /* Return an error.  */
        return(LX_ERROR);
    }

    /* Check if the block is actually mapped.  */
    if (new_block_status == LX_NAND_BLOCK_STATUS_FREE)
    {

        /* Unmap the block.  */
        new_block = LX_NAND_BLOCK_UNMAPPED;
    }

    /* Update block mapping.  */
    _lx_nand_flash_block_mapping_set(nand_flash, block_mapping_index * nand_flash -> lx_nand_flash_pages_per_block, new_block);

    /* Check if the block is actually mapped.  */
    if (new_block != LX_NAND_BLOCK_UNMAPPED)
    {

        /* Add the block to mapped list.  */
        _lx_nand_flash_mapped_block_list_add(nand_flash, block_mapping_index);
    }

    /* Erase the old block.  */
    status = _lx_nand_flash_driver_block_erase(nand_flash, block, nand_flash -> lx_nand_flash_base_erase_count + nand_flash -> lx_nand_flash_erase_count_table[block] + 1);

    /* Check for an error from flash driver.   */
    if (status)
    {

        /* Call system error handler.  */
        _lx_nand_flash_system_error(nand_flash, status, block, 0);

        /* Return an error.  */
        return(LX_ERROR);
    }

    /* Update the erase count for the erased block.  */
    status = _lx_nand_flash_erase_count_set(nand_flash, block, (UCHAR)(nand_flash -> lx_nand_flash_erase_count_table[block] + 1));

    /* Check for an error from flash driver.   */
    if (status)
    {

        /* Call system error handler.  */
        _lx_nand_flash_system_error(nand_flash, status, block, 0);

        /* Return an error.  */
        return(LX_ERROR);
    }

    /* Set the block status to free.  */
    status = _lx_nand_flash_block_status_set(nand_flash, block, LX_NAND_BLOCK_STATUS_FREE);

    /* Check for an error from flash driver.   */
    if (status)
    {

        /* Call system error handler.  */
        _lx_nand_flash_system_error(nand_flash, status, block, 0);


        /* Return an error.  */
        return(LX_ERROR);
    }

    /* Add the block to free block list.  */
    status = _lx_nand_flash_free_block_list_add(nand_flash, block);

    /* Return status.  */
    return(status);
}

