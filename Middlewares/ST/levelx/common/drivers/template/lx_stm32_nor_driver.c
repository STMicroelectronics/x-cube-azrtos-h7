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

/* Include necessary files.  */
#include "lx_stm32_nor_driver.h"

static UINT  lx_nor_read(ULONG *flash_address, ULONG *destination, ULONG words);
static UINT  lx_nor_write(ULONG *flash_address, ULONG *source, ULONG words);

static UINT  lx_nor_block_erase(ULONG block, ULONG erase_count);
static UINT  lx_nor_block_erased_verify(ULONG block);

UINT  lx_stm32_nor_initialize(LX_NOR_FLASH *nor_flash)
{
    /* this function is the main driver entry it should implment
     * - the specific device initialization.
     * - setup the nor flash genometry
     * - define the driver function pointers for the NOR flash services (read, write, eraseblock,)
     
     --> Setup geometry of the flash. <--
     
       nor_flash->lx_nor_flash_total_blocks <-- define the total blocks in the flash
       nor_flash->lx_nor_flash_words_per_block <-- setup the words per block

     -->Setup function pointers for the NOR flash services <--

    nor_flash->lx_nor_flash_driver_read = lx_nor_simulator_read;
    nor_flash->lx_nor_flash_driver_write = lx_nor_simulator_write;

    nor_flash->lx_nor_flash_driver_block_erase = lx_nor_simulator_block_erase;
    nor_flash->lx_nor_flash_driver_block_erased_verify = lx_nor_simulator_block_erased_verify;

    --> etup local buffer for NOR flash operation. This buffer must be the sector size of the NOR flash memory <--
    nor_flash->lx_nor_flash_sector_buffer <-- define a buffer to be used by the levelx for internal operations.

    */
    return(LX_SUCCESS);
}


static UINT  lx_nor_read(ULONG *flash_address, ULONG *destination, ULONG words)
{

    return(LX_SUCCESS);
}


static UINT  lx_nor_write(ULONG *flash_address, ULONG *source, ULONG words)
{

    return(LX_SUCCESS);
}

static UINT  lx_nor_block_erase(ULONG block, ULONG erase_count)
{

    return(LX_SUCCESS);
}

static UINT  lx_nor_block_erased_verify(ULONG block)
{
    return(LX_SUCCESS);
}

