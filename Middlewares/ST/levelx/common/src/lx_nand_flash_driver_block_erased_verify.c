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
/*    _lx_nand_flash_driver_block_erased_verify           PORTABLE C      */ 
/*                                                           6.2.1       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function calls the driver to verify the block was erased.      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
/*    block                                 Block number                  */ 
/*    page                                  Page number                   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    (lx_nand_flash_driver_block_erased_verify)                          */ 
/*                                          Driver verify page erased     */ 
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
/*  03-08-2023     Xiuwen Cai               Modified comment(s),          */
/*                                            added new driver interface, */
/*                                            resulting in version 6.2.1 */
/*                                                                        */
/**************************************************************************/
UINT  _lx_nand_flash_driver_block_erased_verify(LX_NAND_FLASH *nand_flash, ULONG block)
{

UINT    status;

    /* Increment the block erased verify count.  */
    nand_flash -> lx_nand_flash_diagnostic_block_erased_verifies++;

    /* Call driver block erased verify function.  */
#ifdef LX_NAND_ENABLE_CONTROL_BLOCK_FOR_DRIVER_INTERFACE
    status =  (nand_flash -> lx_nand_flash_driver_block_erased_verify)(nand_flash, block);
#else
    status =  (nand_flash -> lx_nand_flash_driver_block_erased_verify)(block);
#endif
    /* Return status.  */
    return(status);
}


