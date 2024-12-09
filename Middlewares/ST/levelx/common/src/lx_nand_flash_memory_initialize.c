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
/*    _lx_nand_flash_memory_initialize                    PORTABLE C      */ 
/*                                                           6.2.1       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Xiuwen Cai, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function imitialize memory buffer for NAND flash instance.     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    memory_ptr                            Pointer to memory used by the */
/*                                            LevelX for this NAND.       */
/*    memory_size                           Size of memory                */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    LX_MEMSET                             Initialize memory             */ 
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
UINT  _lx_nand_flash_memory_initialize(LX_NAND_FLASH  *nand_flash, ULONG* memory_ptr, UINT memory_size)
{

UINT    memory_offset;
UINT    buffer_size;


    /* Clear the memory buffer.  */
    LX_MEMSET(memory_ptr, 0, memory_size);

    /* Reset the memory offset.  */
    memory_offset = 0;

    /* Set memory size for block mapping table.  */
    buffer_size = nand_flash -> lx_nand_flash_total_blocks * sizeof(*nand_flash -> lx_nand_flash_block_mapping_table);

    /* Make sure the size is at least one page size.  */
    if (buffer_size < nand_flash -> lx_nand_flash_bytes_per_page)
    {
        buffer_size = nand_flash -> lx_nand_flash_bytes_per_page;
    }

    /* Assign memory for block mapping table.  */
    nand_flash -> lx_nand_flash_block_mapping_table = (USHORT*)(((UCHAR*)memory_ptr) + memory_offset);

    /* Update block mapping table size.  */
    nand_flash -> lx_nand_flash_block_mapping_table_size = buffer_size;

    /* Update memory offset.  */
    memory_offset += buffer_size;

    /* Check if there is enough memory.  */
    if (memory_offset > memory_size)
    {

        /* No enough memory, return error.  */
        return(LX_NO_MEMORY);
    }

    /* Set memory size for erase count table.  */
    buffer_size = nand_flash -> lx_nand_flash_total_blocks * sizeof(*nand_flash -> lx_nand_flash_erase_count_table);

    /* Make sure the size is at least one page size.  */
    if (buffer_size < nand_flash -> lx_nand_flash_bytes_per_page)
    {
        buffer_size = nand_flash -> lx_nand_flash_bytes_per_page;
    }

    /* Assign memory for erase count table.  */
    nand_flash -> lx_nand_flash_erase_count_table = (UCHAR*)(((UCHAR*)memory_ptr) + memory_offset);
    
    /* Update memory offset.  */
    memory_offset += buffer_size;

    /* Update erase count table size.  */
    nand_flash -> lx_nand_flash_erase_count_table_size = buffer_size;

    /* Check if there is enough memory.  */
    if (memory_offset > memory_size)
    {

        /* No enough memory, return error.  */
        return(LX_NO_MEMORY);
    }

    /* Assign memory for block list.  */
    nand_flash -> lx_nand_flash_block_list = (USHORT*)(((UCHAR*)memory_ptr) + memory_offset);

    /* Update memory offset.  */
    memory_offset += nand_flash -> lx_nand_flash_total_blocks * sizeof(*nand_flash -> lx_nand_flash_block_list);

    /* Check if there is enough memory.  */
    if (memory_offset > memory_size)
    {

        /* No enough memory, return error.  */
        return(LX_NO_MEMORY);
    }

    /* Update block list size.  */
    nand_flash -> lx_nand_flash_block_list_size = nand_flash -> lx_nand_flash_total_blocks;

    /* Initialize block list. */
    nand_flash -> lx_nand_flash_free_block_list_tail = 0;
    nand_flash -> lx_nand_flash_mapped_block_list_head = nand_flash -> lx_nand_flash_block_list_size - 1;

    /* Set memory size for block status table.  */
    buffer_size = nand_flash -> lx_nand_flash_total_blocks * sizeof(*nand_flash -> lx_nand_flash_block_status_table);

    /* Make sure the size is at least one page size.  */
    if (buffer_size < nand_flash -> lx_nand_flash_bytes_per_page)
    {
        buffer_size = nand_flash -> lx_nand_flash_bytes_per_page;
    }

    /* Assign memory for block status table.  */
    nand_flash -> lx_nand_flash_block_status_table = (USHORT*)(((UCHAR*)memory_ptr) + memory_offset);

    /* Update memory offset.  */
    memory_offset += buffer_size;

    /* Update block status table size.  */
    nand_flash -> lx_nand_flash_block_status_table_size = buffer_size;

    /* Check if there is enough memory.  */
    if (memory_offset > memory_size)
    {

        /* No enough memory, return error.  */
        return(LX_NO_MEMORY);
    }

    /* Assign memory for page buffer.  */
    nand_flash -> lx_nand_flash_page_buffer = ((UCHAR*)memory_ptr) + memory_offset;

    /* Update page buffer size.  */
    nand_flash -> lx_nand_flash_page_buffer_size = memory_size - memory_offset;

    /* Check if there is enough memory.  */
    if (nand_flash -> lx_nand_flash_page_buffer_size < (nand_flash -> lx_nand_flash_bytes_per_page + nand_flash -> lx_nand_flash_spare_total_length) * 2)
    {

        /* No enough memory, return error.  */
        return(LX_NO_MEMORY);
    }

    /* Return a successful completion.  */
    return(LX_SUCCESS);
}

