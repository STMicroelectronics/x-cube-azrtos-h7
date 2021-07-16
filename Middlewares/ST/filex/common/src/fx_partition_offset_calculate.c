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
/** FileX Component                                                       */ 
/**                                                                       */
/**   Application Utility                                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_utility.h"


/* Define internal data structures.  */

typedef struct FX_MEDIA_PARTITION_STRUCT
{
    ULONG fx_media_part_start;    
    ULONG fx_media_part_size;
} FX_MEDIA_PARTITION;


/* Define function prototypes for the partition table parsing application
   utility.  */

UINT    _fx_partition_offset_calculate(void  *partition_sector, UINT partition,
                                     ULONG *partition_start, ULONG *partition_size);
UINT    _fx_utility_partition_get(FX_MEDIA_PARTITION *partition_table, 
                                UINT *count, ULONG sector, UCHAR *sector_buffer);


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _fx_partition_offset_calculate                      PORTABLE C      */ 
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function calculates the sector offset to the specified         */ 
/*    partition.  The buffer containing the partition table is also       */ 
/*    supplied to this function.  If the buffer supplied is a boot        */ 
/*    record (which could be the case in non-partition systems), this     */ 
/*    function returns an offset of zero, the total sectors, and a        */ 
/*    successful status indicating that the buffer supplied is the boot   */ 
/*    record.  Otherwise, if a partition is found, this function returns  */ 
/*    the sector offset to its boot record along with a successful        */ 
/*    status. If the specified partition is not found or the buffer is    */ 
/*    not a partition table or boot record, this function returns an      */ 
/*    error.                                                              */ 
/*                                                                        */ 
/*    Note: Empty partitions have a FX_SUCCESS return code, however their */ 
/*          starting sector is FX_NULL and the size returned is 0.        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    partition_sector                      Pointer to buffer containing  */ 
/*                                            either the partition table  */ 
/*                                            or the boot sector          */ 
/*    partition                             Desired partition             */ 
/*    partition_start                       Return partition start        */ 
/*    partition_size                        Return partition size         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    return status                                                       */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*     _fx_utility_partition_get            Actual partition parsing      */ 
/*                                            routine                     */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Driver                                                  */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-02-2021     William E. Lamie         Modified comment(s),          */
/*                                            ignored signature check for */
/*                                            no partition situation,     */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/
UINT  _fx_partition_offset_calculate(void  *partition_sector, UINT partition,
                                     ULONG *partition_start, ULONG *partition_size)
{
    
FX_MEDIA_PARTITION  partition_table[4];
UINT                count;
ULONG64             total_sectors;
UCHAR               *partition_sector_ptr;


    /* Setup working pointer and initialize count.  */
    partition_sector_ptr =  partition_sector;
    count =  0;

    /* Check for a real boot sector instead of a partition table.  */
    if ((partition_sector_ptr[0] == 0xe9) || ((partition_sector_ptr[0] == 0xeb) && (partition_sector_ptr[2] == 0x90)))    
    {
    
        /* Yes, a real boot sector could be present.  */  

        /* See if there are good values for sectors per FAT.  */
        if (partition_sector_ptr[0x16] || partition_sector_ptr[0x17] || partition_sector_ptr[0x24] || partition_sector_ptr[0x25] || partition_sector_ptr[0x26] || partition_sector_ptr[0x27])
        {

            /* There are values for sectors per FAT.  */

            /* Determine if there is a total sector count.  */
            total_sectors =  0;

            if (partition_sector_ptr[0x13] || partition_sector_ptr[0x14])
            {

                /* Calculate the total sectors, FAT12/16.  */
                total_sectors =  (((ULONG) partition_sector_ptr[0x14]) << 8) | ((ULONG) partition_sector_ptr[0x13]);
            }
            else if (partition_sector_ptr[0x20] || partition_sector_ptr[0x21] || partition_sector_ptr[0x22] || partition_sector_ptr[0x23])
            {

                /* Calculate the total sectors, FAT32.  */
                total_sectors =  (((ULONG) partition_sector_ptr[0x23]) << 24) | 
                                 (((ULONG) partition_sector_ptr[0x22]) << 16) |
                                 (((ULONG) partition_sector_ptr[0x21]) << 8)  |
                                 ((ULONG) partition_sector_ptr[0x20]);
            }       

            /* Determine if there is a total sector count.  */
            if (total_sectors)
            {

                if (partition_start != FX_NULL)
                {
                    /* Return an offset of 0, size of boot record, and a successful status.  */
                    *partition_start =  0;
                }

                /* Determine if the total sectors is required.  */
                if (partition_size != FX_NULL)
                {

                    /* Return the total sectors.  */
                    *partition_size =  (ULONG)(total_sectors & 0xFFFFFFFF);
                }

                /* Return success!  */
                return(FX_SUCCESS);
            }
        }
#ifdef FX_ENABLE_EXFAT
        /* See if there are good values for sectors per exFAT.  */
        else if (partition_sector_ptr[0x0b] == 0 && partition_sector_ptr[0x0c] == 0)
        {
            /* There are values for sectors per exFAT.  */

            /* Calculate the total sectors.  */
            total_sectors = _fx_utility_64_unsigned_read(&partition_sector_ptr[FX_EF_VOLUME_LENGTH]);

            /* Determine if there is a total sector count.  */
            if (total_sectors)
            {

                if (partition_start != FX_NULL)
                {
                    /* Return an offset of 0, size of boot record, and a successful status.  */
                    *partition_start =  0;
                }

                /* Determine if the total sectors is required.  */
                if (partition_size != FX_NULL)
                {

                    if (total_sectors > 0xFFFFFFFF)
                    {

                        /* Overflow. Just return not found. */
                        return(FX_NOT_FOUND);
                    }

                    /* Return the total sectors.  */
                    *partition_size =  (ULONG)(total_sectors & 0xFFFFFFFF);
                }

                /* Return success!  */
                return(FX_SUCCESS);
            }
        }
#endif /* FX_ENABLE_EXFAT */
    }

    /* Check signature to make sure the buffer is valid.  */
    if ((partition_sector_ptr[510] != 0x55) || (partition_sector_ptr[511] != 0xAA))
    {

        /* Invalid, return an error.  */
        return(FX_NOT_FOUND);
    }
    
    /* Not bootable, look for specific partition.  */
    _fx_utility_partition_get(partition_table, &count, 0, partition_sector_ptr);

    /* Determine if return value is valid.  */
    if (partition >= count)
    {

        /* No, return an error.  */
        return(FX_NOT_FOUND);
    }

    /* Return the partition starting sector, if non-NULL.  */
    if (partition_start != FX_NULL)
    {
        *partition_start =  partition_table[partition].fx_media_part_start;
    }

    /* Return the partition size, if non-NULL.  */
    if (partition_size != FX_NULL)
    {
        *partition_size =  partition_table[partition].fx_media_part_size;
    }

    /* Return successful completion.  */
    return(FX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _fx_utility_partition_get                           PORTABLE C      */ 
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function parses the partition sector and completes the         */ 
/*    supplied partition entry structure.                                 */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    partition_table                       Pointer to partition table    */ 
/*    count                                 Number of partitions found    */ 
/*    sector                                Base sector                   */ 
/*    sector_buffer                         Buffer containing partition   */ 
/*                                            table                       */ 
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
/*    _fx_partition_offset_calculate        Calculate partition offset    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-02-2021     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/
UINT  _fx_utility_partition_get(FX_MEDIA_PARTITION *partition_table, 
                                UINT *count, ULONG sector, UCHAR *sector_buffer)
{

UINT    i;
ULONG   base_sector, value;

    /* This parameter has not been supported yet. */
    FX_PARAMETER_NOT_USED(sector); 

    /* Initialize base sector.  */
    base_sector =  0;

    for(i = 446; i <= 494; i+=16)
    {
        if (sector_buffer[i + 4] == 0) /* no partition entry here */
        {

            partition_table[*count].fx_media_part_start = 0;
            partition_table[*count].fx_media_part_size  = 0;
        }
        else
        {

            value =  (ULONG) sector_buffer[i + 8]; /* little endian start value */
            value =  (((ULONG) sector_buffer[i + 9]) << 8) | value;
            value =  (((ULONG) sector_buffer[i + 10]) << 16) | value;
            value =  (((ULONG) sector_buffer[i + 11]) << 24) | value;
            partition_table[*count].fx_media_part_start = value + base_sector;

            value =  (ULONG) sector_buffer[i + 12]; /* little endian size value */
            value =  (((ULONG) sector_buffer[i + 13]) << 8) | value;
            value =  (((ULONG) sector_buffer[i + 14]) << 16) | value;
            value =  (((ULONG) sector_buffer[i + 15]) << 24) | value;
            partition_table[*count].fx_media_part_size = value;
        }

        (*count)++;
    }

    /* Return success.  */
    return(FX_SUCCESS);
}

