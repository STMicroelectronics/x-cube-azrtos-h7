/* Test the condition of fat entry broken. */
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

void    filex_unicode_2_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);

/* Create a terrible driver. */
static UINT driver_called_counter = 0;
static void _fx_terrible_driver(FX_MEDIA *media_ptr)
{
    driver_called_counter++;
    //printf("\n_fx_terrible_driver has been called %d times.", driver_called_counter);
    
    // Ensure the variable we want to operate normal at the rest of time.
    media_ptr -> fx_media_root_cluster_32 = 2;
    // To cover the branch at Line 203 in fx_unicode_directory_entry_read.c, modify fx_media_root_cluster_32 at a particular time.
    if ( driver_called_counter == 2)
    {
        media_ptr -> fx_media_root_cluster_32 = 0xffffffff;
    }
    if ( driver_called_counter == 9)
    {
        media_ptr -> fx_media_root_cluster_32 = 1;
    }
    (* _fx_ram_driver)(media_ptr);
}

/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_unicode_2_application_define(void *first_unused_memory)
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

static UCHAR long_unicode_name1[] =      {1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 0, 0}; 
/* Define the test threads.  */

static void    ftest_0_entry(ULONG thread_input)
{
UINT status, length;
UCHAR destination_name[128];
UCHAR destination_name_1[128];

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Unicode 2 test.........................................");

    /* Format the media as FAT32.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                              _fx_ram_driver,         // Driver entry            
                              ram_disk_memory_large,  // RAM disk memory pointer
                              cache_buffer,           // Media buffer pointer
                              128,                    // Media buffer size 
                              "MY_RAM_DISK",          // Volume Name
                              1,                      // Number of FATs
                              32,                     // Directory Entries
                              0,                      // Hidden sectors
                              70000,                   // Total sectors 
                              128,                    // Sector size   
                              1,                      // Sectors per cluster
                              1,                      // Heads
                              1);                     // Sectors per track 

    return_value_if_fail( status == FX_SUCCESS, 1);
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_value_if_fail( status == FX_SUCCESS, 2);
    
    length = fx_unicode_length_get( long_unicode_name1);
    status = fx_unicode_directory_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name);
    long_unicode_name1[0]++;
    length = fx_unicode_length_get( long_unicode_name1);
    status = fx_unicode_file_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name_1);
    long_unicode_name1[0]++;
    length = fx_unicode_length_get( long_unicode_name1);
    status = fx_unicode_file_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name_1);

    status = fx_media_flush(&ram_disk);
    return_value_if_fail( status == FX_SUCCESS, 3);

    // Register our terrible driver.
    ram_disk.fx_media_driver_entry = _fx_terrible_driver;

    long_unicode_name1[0]++;
    length = fx_unicode_length_get( long_unicode_name1);
    status = fx_unicode_file_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name_1);
    return_value_if_fail( status == FX_SUCCESS, 4);

    // Register our terrible driver.
    ram_disk.fx_media_driver_entry = _fx_ram_driver;

    /* Corrupt the media.  */
    ram_disk.fx_media_bytes_per_sector = 0;
    status = fx_unicode_file_create(&ram_disk, long_unicode_name1, length, (CHAR*)destination_name_1);
    return_value_if_fail(status == FX_MEDIA_INVALID, 4);
    ram_disk.fx_media_bytes_per_sector = 128;

    status = fx_media_abort( &ram_disk);
    return_value_if_fail( status == FX_SUCCESS, 5);

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
                              70000,                   // Total sectors 
                              128,                    // Sector size   
                              1,                      // Sectors per cluster
                              1,                      // Heads
                              1);                     // Sectors per track 

    return_value_if_fail( status == FX_SUCCESS, 6);
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_value_if_fail( status == FX_SUCCESS, 7);
    
    length = fx_unicode_length_get( long_unicode_name1);
    status = fx_unicode_directory_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name);
    long_unicode_name1[0]++;
    length = fx_unicode_length_get( long_unicode_name1);
    status = fx_unicode_file_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name_1);
    long_unicode_name1[0]++;
    length = fx_unicode_length_get( long_unicode_name1);
    status = fx_unicode_file_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name_1);

    status = fx_media_flush(&ram_disk);
    return_value_if_fail( status == FX_SUCCESS, 8);

    // Register our terrible driver.
    ram_disk.fx_media_driver_entry = _fx_terrible_driver;

    long_unicode_name1[0]++;
    length = fx_unicode_length_get( long_unicode_name1);
    status = fx_unicode_file_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name_1);
    // Our modification will not influence the result.
    return_value_if_fail( status == FX_SUCCESS, 9);

    // Register our terrible driver.
    ram_disk.fx_media_driver_entry = _fx_ram_driver;

    status = fx_media_abort( &ram_disk);
    return_value_if_fail( status == FX_SUCCESS, 10);

    printf("SUCCESS!\n");
    test_control_return(0);
}
