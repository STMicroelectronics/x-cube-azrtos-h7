#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_utility.h"
#include   "fx_ram_driver_test.h"
#include    <stdio.h>
#include    <string.h>

#define     DEMO_STACK_SIZE         8192
/* Set the cache size as the size of one sector causing frequently IO operation. */
#define     CACHE_SIZE              128

/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD                ftest_0;
#endif
static FX_MEDIA                 ram_disk;

/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                    *ram_disk_memory;
static UCHAR                    *cache_buffer;
#else
static UCHAR                     cache_buffer[CACHE_SIZE];
#endif

/* Define thread prototypes.  */

void    filex_unicode_3_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);

/* Create a terrible driver. */
static UINT driver_called_counter = 0;
static void _fx_terrible_driver(FX_MEDIA *media_ptr)
{
    driver_called_counter++;
//    printf("\n_fx_terrible_driver has been called %d times.", driver_called_counter);
    if ( 
        // Make IO ERROR to reach the Line 202 in fx_unicode_directory_search.c
        (driver_called_counter == 1)
        )
    {
        media_ptr -> fx_media_driver_status = FX_IO_ERROR;
        return;
    }
    (* _fx_ram_driver)(media_ptr);
}

/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_unicode_3_application_define(void *first_unused_memory)
#endif
{

#ifndef FX_STANDALONE_ENABLE
UCHAR    *pointer;

    
    /* Setup the working pointer.  */
    pointer =  (UCHAR *) first_unused_memory;

    /* Create the main thread.  */
    tx_thread_create(&ftest_0, "thread 0", ftest_0_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer =  pointer + DEMO_STACK_SIZE;

    /* Setup memory for the RAM disk and the sector cache.  */
    cache_buffer =  pointer;
    pointer =  pointer + CACHE_SIZE;
    ram_disk_memory =  pointer;

#endif

    /* Initialize the FileX system.  */
    fx_system_initialize();
#ifdef FX_STANDALONE_ENABLE
    ftest_0_entry(0);
#endif
}

static UCHAR long_unicode_name1[] =      {1, 0, 0, 0}; 
static UCHAR long_unicode_name2[] =      {1, 0, 2, 0, 0, 0}; 

/* Define the test threads.  */

static void    ftest_0_entry(ULONG thread_input)
{

UINT status, length;
UCHAR destination_name[128];
FX_LOCAL_PATH local_path;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Unicode 3 test.........................................");
    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                              _fx_ram_driver,         // Driver entry            
                              ram_disk_memory_large,  // RAM disk memory pointer
                              cache_buffer,           // Media buffer pointer
                              128,                    // Media buffer size 
                              "MY_RAM_DISK",          // Volume Name
                              1,                      // Number of FATs
                              32,                     // Directory Entries
                              0,                      // Hidden sectors
                              70000,                  // Total sectors 
                              128,                    // Sector size   
                              1,                      // Sectors per cluster
                              1,                      // Heads
                              1);                     // Sectors per track 

    return_value_if_fail( status == FX_SUCCESS, 1);
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_value_if_fail( status == FX_SUCCESS, 2);

    // Locate at a subdir.
    length = fx_unicode_length_get( long_unicode_name1);
    status = fx_unicode_directory_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name);
#ifndef FX_STANDALONE_ENABLE
        status =   fx_directory_local_path_set(&ram_disk, &local_path, (CHAR *)destination_name);
#else
        status =   fx_directory_default_set(&ram_disk, (CHAR *)destination_name);
#endif

    /* Flush everything out. */
    status += fx_media_flush( &ram_disk);
    status += _fx_utility_FAT_flush( &ram_disk);
    status += _fx_utility_logical_sector_flush( &ram_disk, 1, 60000, FX_TRUE);
    return_value_if_fail( status == FX_SUCCESS, 3);

    for (UINT i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster = 0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value = 0;
    }

    // Register our terrible driver.
    ram_disk.fx_media_driver_entry = _fx_terrible_driver;

    // fx_unicode_file_create will call fx_unicode_directory_search where we will make an IO ERROR.
    length = fx_unicode_length_get( long_unicode_name2);
    status = fx_unicode_file_create( &ram_disk, long_unicode_name2, length, (CHAR *)destination_name);
    return_value_if_fail( status == FX_SUCCESS, 4);

    // Register our terrible driver.
    ram_disk.fx_media_driver_entry = _fx_ram_driver;

    status = fx_media_close( &ram_disk);
    return_value_if_fail( status == FX_SUCCESS, 5);

    printf("SUCCESS!\n");
    test_control_return(0);
}
