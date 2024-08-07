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
/**   NOR Flash                                                           */
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
/*    _lx_nor_flash_extended_cache_enable                 PORTABLE C      */ 
/*                                                           6.3.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function enables or disables the extended cache.               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    nor_flash                             NOR flash instance            */ 
/*    memory                                Address of RAM for cache      */ 
/*    size                                  Size of the RAM for cache     */ 
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
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  12-31-2020     William E. Lamie         Modified comment(s),          */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1.3  */
/*  06-02-2021     Bhupendra Naphade        Modified comment(s), and      */
/*                                            updated product constants   */
/*                                            resulting in version 6.1.7  */
/*  10-15-2021     Bhupendra Naphade        Modified comment(s), and      */
/*                                            added check for out of      */
/*                                            bound memory access,        */
/*                                            resulting in version 6.1.9  */
/*  10-31-2023     Xiuwen Cai               Modified comment(s),          */
/*                                            added mapping bitmap cache, */
/*                                            added obsolete count cache, */
/*                                            resulting in version 6.3.0  */
/*                                                                        */
/**************************************************************************/
UINT  _lx_nor_flash_extended_cache_enable(LX_NOR_FLASH *nor_flash, VOID *memory, ULONG size)
{
#ifndef LX_NOR_DISABLE_EXTENDED_CACHE

UINT    i;
ULONG   cache_size;
ULONG   *cache_memory;
#ifdef LX_NOR_ENABLE_MAPPING_BITMAP
ULONG   mapping_bitmap_words;
ULONG   mapping_bitmap_word;
ULONG   logical_sector;
ULONG   *mapping_bitmap_ptr;
#endif
#ifdef LX_NOR_ENABLE_OBSOLETE_COUNT_CACHE
ULONG   obsolete_count_words;
ULONG   obsolete_sectors;
#endif
#if defined(LX_NOR_ENABLE_MAPPING_BITMAP) || defined(LX_NOR_ENABLE_OBSOLETE_COUNT_CACHE)
ULONG   *block_word_ptr;
UINT    j;
UINT    status;
ULONG   block_word;
#endif


    /* Determine if memory was specified but with an invalid size (less than one NOR sector).  */
    if ((memory) && (size < LX_NOR_SECTOR_SIZE))
    {
    
        /* Error in memory size supplied.  */
        return(LX_ERROR);
    }

#ifdef LX_THREAD_SAFE_ENABLE

    /* Obtain the thread safe mutex.  */
    tx_mutex_get(&nor_flash -> lx_nor_flash_mutex, TX_WAIT_FOREVER);
#endif

    /* Initialize the internal NOR cache.  */
    nor_flash -> lx_nor_flash_extended_cache_entries =  0;

    /* Calculate cache size in words.  */
    cache_size = size/sizeof(ULONG);

    /* Setup cache memory pointer.  */
    cache_memory =  (ULONG *) memory;

#if defined(LX_NOR_ENABLE_MAPPING_BITMAP) || defined(LX_NOR_ENABLE_OBSOLETE_COUNT_CACHE)

    /* Check if the NOR flash is opened.  */
    if (nor_flash -> lx_nor_flash_state == LX_NOR_FLASH_OPENED)
    {
#if defined(LX_NOR_ENABLE_MAPPING_BITMAP)

        /* Get the mapping bitmap cache size.  */
        mapping_bitmap_words = (nor_flash -> lx_nor_flash_total_physical_sectors + 31) / 32;
        
        /* Check if the mapping bitmap cache fits in the suppiled cache memory.  */
        if (cache_size < mapping_bitmap_words)
        {

            /* Update the cache size.  */
            mapping_bitmap_words = cache_size;
        }
        
        /* Setup the mapping bitmap cache.  */
        nor_flash -> lx_nor_flash_extended_cache_mapping_bitmap =  cache_memory;

        /* Setup the mapping bitmap cache size.  */
        nor_flash -> lx_nor_flash_extended_cache_mapping_bitmap_max_logical_sector =  mapping_bitmap_words * 32;
        
        /* Clear the mapping bitmap cache.  */
        for (i = 0; i < mapping_bitmap_words; i++)
        {
            cache_memory[i] =  0;
        }

        /* Update the cache memory pointer.  */
        mapping_bitmap_ptr =  cache_memory;

        /* Update the cache size.  */
        cache_size =  cache_size - mapping_bitmap_words;

        /* Update the cache memory pointer.  */
        cache_memory =  cache_memory + mapping_bitmap_words;
#endif
        
#if defined(LX_NOR_ENABLE_OBSOLETE_COUNT_CACHE)

        /* Get the obsolete count cache size.  */
        obsolete_count_words = nor_flash -> lx_nor_flash_total_blocks * sizeof(LX_NOR_OBSOLETE_COUNT_CACHE_TYPE) / 4;
        
        /* Check if the obsolete count cache fits in the suppiled cache memory.  */
        if (cache_size < obsolete_count_words)
        {

            /* Update the cache size.  */
            obsolete_count_words = cache_size;
        }
        
        /* Setup the obsolete count cache.  */
        nor_flash -> lx_nor_flash_extended_cache_obsolete_count =  (LX_NOR_OBSOLETE_COUNT_CACHE_TYPE*)cache_memory;

        /* Setup the obsolete count cache size.  */
        nor_flash -> lx_nor_flash_extended_cache_obsolete_count_max_block =  obsolete_count_words * 4 / sizeof(LX_NOR_OBSOLETE_COUNT_CACHE_TYPE);

        /* Update the cache size.  */
        cache_size =  cache_size - obsolete_count_words;

        /* Update the cache memory pointer.  */
        cache_memory =  cache_memory + obsolete_count_words;
#endif

        /* Loop through the blocks.  */
        for (i = 0; i < nor_flash -> lx_nor_flash_total_blocks; i++)
        {
            /* Setup the block word pointer to the first word of the block.  */
            block_word_ptr =  (nor_flash -> lx_nor_flash_base_address + (i * nor_flash -> lx_nor_flash_words_per_block));

#if defined(LX_NOR_ENABLE_OBSOLETE_COUNT_CACHE)

            /* Initialize the obsolete count cache.  */
            obsolete_sectors = 0;
#endif

            /* Now walk the list of logical-physical sector mapping.  */
            for (j = 0; j < nor_flash ->lx_nor_flash_physical_sectors_per_block; j++)
            {
                
                /* Read this word of the sector mapping list.  */
#ifdef LX_DIRECT_READ
    
                /* Read the word directly.  */
                block_word =  *(block_word_ptr + nor_flash -> lx_nor_flash_block_physical_sector_mapping_offset + j);
#else
                status =  _lx_nor_flash_driver_read(nor_flash, (block_word_ptr + nor_flash -> lx_nor_flash_block_physical_sector_mapping_offset + j), &block_word, 1);

                /* Check for an error from flash driver. Drivers should never return an error..  */
                if (status)
                {
    
                    /* Call system error handler.  */
                    _lx_nor_flash_system_error(nor_flash, status);

                    /* Return an error.  */
                    return(LX_ERROR);
                }
#endif
                /* Determine if the entry hasn't been used.  */
                if (block_word == LX_NOR_PHYSICAL_SECTOR_FREE)
                {
                    break;
                }
                
                /* Is this entry valid?  */
                if ((block_word & (LX_NOR_PHYSICAL_SECTOR_VALID | LX_NOR_PHYSICAL_SECTOR_MAPPING_NOT_VALID)) == LX_NOR_PHYSICAL_SECTOR_VALID)
                {
#if defined(LX_NOR_ENABLE_MAPPING_BITMAP)

                    /* Yes, get the logical sector.  */
                    logical_sector = block_word & LX_NOR_LOGICAL_SECTOR_MASK;
                    
                    /* Get the mapping bitmap word.  */
                    mapping_bitmap_word = logical_sector >> 5;
                    
                    /* Check if the mapping bitmap word is within the cache.  */
                    if (mapping_bitmap_word < mapping_bitmap_words)
                    {

                        /* Set the bit in the mapping bitmap.  */
                        mapping_bitmap_ptr[mapping_bitmap_word] |=  (ULONG)(1 << (logical_sector & 31));
                    }
#endif
                    
                }
                else
                {
#if defined(LX_NOR_ENABLE_OBSOLETE_COUNT_CACHE)

                    /* Increment the obsolete sector count.  */
                    obsolete_sectors++;
#endif
                }
            }
#if defined(LX_NOR_ENABLE_OBSOLETE_COUNT_CACHE)

            /* Check if the block is cached by obsolete count cache.  */
            if (i < nor_flash -> lx_nor_flash_extended_cache_obsolete_count_max_block)
            {

                /* Yes, cache the obsolete sector count.  */
                nor_flash -> lx_nor_flash_extended_cache_obsolete_count[i] = (LX_NOR_OBSOLETE_COUNT_CACHE_TYPE)obsolete_sectors;
            }
#endif
        }
    }
#endif
    
    /* Loop through the memory supplied and assign to cache entries.  */
    i =  0;
    while (cache_size >= LX_NOR_SECTOR_SIZE)
    {
    
        /* Setup this cache entry.  */
        nor_flash -> lx_nor_flash_extended_cache[i].lx_nor_flash_extended_cache_entry_sector_address =  LX_NULL;
        nor_flash -> lx_nor_flash_extended_cache[i].lx_nor_flash_extended_cache_entry_sector_memory =   cache_memory;
        nor_flash -> lx_nor_flash_extended_cache[i].lx_nor_flash_extended_cache_entry_access_count =    0;
        
        /* Move the cache memory forward.   */
        cache_memory =  cache_memory + LX_NOR_SECTOR_SIZE;
        
        /* Decrement the size.  */
        cache_size =  cache_size - LX_NOR_SECTOR_SIZE;
    
        /* Move to next cache entry.  */
        i++;
    }
    
    /* Save the number of cache entries.  */
    if(i > LX_NOR_EXTENDED_CACHE_SIZE)
    {

        nor_flash -> lx_nor_flash_extended_cache_entries =  LX_NOR_EXTENDED_CACHE_SIZE;
    }
    else
    {

        nor_flash -> lx_nor_flash_extended_cache_entries =  i;
    }  

#ifdef LX_THREAD_SAFE_ENABLE

    /* Release the thread safe mutex.  */
    tx_mutex_put(&nor_flash -> lx_nor_flash_mutex);
#endif

    /* Return successful completion.  */
    return(LX_SUCCESS);
#else

    LX_PARAMETER_NOT_USED(nor_flash);
    LX_PARAMETER_NOT_USED(memory);
    LX_PARAMETER_NOT_USED(size);

    /* Return disabled error message.  */
    return(LX_DISABLED);
#endif
}


