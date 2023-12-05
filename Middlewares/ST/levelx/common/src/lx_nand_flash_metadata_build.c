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
/*    _lx_nand_flash_metadata_build                       PORTABLE C      */ 
/*                                                           6.2.1       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Xiuwen Cai, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function rewrites all metadata pages.                          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nand_flash                            NAND flash instance           */ 
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
UINT  _lx_nand_flash_metadata_build(LX_NAND_FLASH *nand_flash)
{

UINT                status;
LX_NAND_DEVICE_INFO *nand_device_info_page;
UINT                page_count;
UINT                i;

    /* Build device info page.  */
    nand_device_info_page = (LX_NAND_DEVICE_INFO*)nand_flash -> lx_nand_flash_page_buffer;
    nand_device_info_page -> lx_nand_device_info_signature1 = LX_NAND_DEVICE_INFO_SIGNATURE1;
    nand_device_info_page -> lx_nand_device_info_signature2 = LX_NAND_DEVICE_INFO_SIGNATURE2;
    nand_device_info_page -> lx_nand_device_info_major_version = LEVELX_MAJOR_VERSION;
    nand_device_info_page -> lx_nand_device_info_minor_version = LEVELX_MINOR_VERSION;
    nand_device_info_page -> lx_nand_device_info_patch_version = LEVELX_PATCH_VERSION;
    nand_device_info_page -> lx_nand_device_info_metadata_block_number = nand_flash -> lx_nand_flash_metadata_block_number;
    nand_device_info_page -> lx_nand_device_info_backup_metadata_block_number = nand_flash -> lx_nand_flash_backup_metadata_block_number;
    nand_device_info_page -> lx_nand_device_info_base_erase_count = nand_flash -> lx_nand_flash_base_erase_count;

    /* Write metadata.  */
    status = _lx_nand_flash_metadata_write(nand_flash, (UCHAR*)nand_device_info_page, LX_NAND_PAGE_TYPE_DEVICE_INFO);

    /* Check return status.  */
    if (status != LX_SUCCESS)
    {

        /* Return error status.  */
        return(status);
    }

    /* Calculate page count for erase count table.  */
    page_count = (nand_flash -> lx_nand_flash_erase_count_table_size + (nand_flash -> lx_nand_flash_bytes_per_page - 1)) / nand_flash -> lx_nand_flash_bytes_per_page;

    /* Loop to write all the pages.  */
    for (i = 0; i < page_count; i++)
    {

        /* Write erase count table.  */
        status = _lx_nand_flash_metadata_write(nand_flash, (UCHAR*)(nand_flash -> lx_nand_flash_erase_count_table + 
                                                i * nand_flash -> lx_nand_flash_bytes_per_page),
                                                LX_NAND_PAGE_TYPE_ERASE_COUNT_TABLE | i);

        /* Check return status.  */
        if (status != LX_SUCCESS)
        {

            /* Return error status.  */
            return(status);
        }
    }

    /* Calculate page count for block mapping table.  */
    page_count = (nand_flash -> lx_nand_flash_block_mapping_table_size + (nand_flash -> lx_nand_flash_bytes_per_page - 1)) / nand_flash -> lx_nand_flash_bytes_per_page;

    /* Loop to write all the pages.  */
    for (i = 0; i < page_count; i++)
    {

        /* Write block mapping table.  */
        status = _lx_nand_flash_metadata_write(nand_flash, (UCHAR*)(nand_flash -> lx_nand_flash_block_mapping_table + 
                                                i * nand_flash -> lx_nand_flash_bytes_per_page / sizeof(*nand_flash -> lx_nand_flash_block_mapping_table)), 
                                                LX_NAND_PAGE_TYPE_BLOCK_MAPPING_TABLE | i);
        /* Check return status.  */
        if (status != LX_SUCCESS)
        {

            /* Return error status.  */
            return(status);
        }
    }

    /* Calculate page count for block status table.  */
    page_count = (nand_flash -> lx_nand_flash_block_status_table_size + (nand_flash -> lx_nand_flash_bytes_per_page - 1)) / nand_flash -> lx_nand_flash_bytes_per_page;

    /* Loop to write all the pages.  */
    for (i = 0; i < page_count; i++)
    {

        /* Write block status table.  */
        status = _lx_nand_flash_metadata_write(nand_flash, (UCHAR*)(nand_flash -> lx_nand_flash_block_status_table + 
                                            i * nand_flash -> lx_nand_flash_bytes_per_page / sizeof(*nand_flash -> lx_nand_flash_block_status_table)), 
                                            LX_NAND_PAGE_TYPE_BLOCK_STATUS_TABLE | i);

        /* Check return status.  */
        if (status != LX_SUCCESS)
        {
            
            /* Return error status.  */
            return(status);
        }
    }

    /* Return status.  */
    return(status);
}

