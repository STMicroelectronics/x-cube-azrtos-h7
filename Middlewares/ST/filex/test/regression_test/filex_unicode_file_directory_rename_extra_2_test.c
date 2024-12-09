/* Test the condition of fat entry broken. */
#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_utility.h"
#include   "fx_ram_driver_test.h"
#include    <stdio.h>

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

void    filex_unicode_file_directory_rename_extra_2_test_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);

/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_unicode_file_directory_rename_extra_2_test_application_define(void *first_unused_memory)
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

static UCHAR                    new_file_name[] =  {'a', 0, 'b', 0, 'c', 0, 0, 0};
static UCHAR                    long_unicode_name1[] =  {1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 0, 0}; 
static UCHAR                    long_unicode_name2[] =  {2, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 0, 0}; 
static UCHAR                    destination_name[100];

/* Define the test threads.  */

static void    ftest_0_entry(ULONG thread_input)
{

UINT status, length, new_length, old_length;
UCHAR temp;
FX_LOCAL_PATH local_path;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Unicode file directory rename extra 2 test.............");

    /* Format the media with FAT32.  This needs to be done before opening it!  */
    status = fx_media_format(&ram_disk, 
                              _fx_ram_driver,         // Driver entry            
                              ram_disk_memory_large,  // RAM disk memory pointer
                              cache_buffer,           // Media buffer pointer
                              CACHE_SIZE,             // Media buffer size 
                              "MY_RAM_DISK",          // Volume Name
                              1,                      // Number of FATs
                              32,                     // Directory Entries
                              0,                      // Hidden sectors
                              70000,                  // Total sectors 
                              128,                    // Sector size   
                              1,                      // Sectors per cluster
                              1,                      // Heads
                              1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    length = fx_unicode_length_get(new_file_name);
    status = fx_unicode_file_create( &ram_disk, new_file_name, length, (CHAR *)destination_name);
    return_if_fail( status == FX_SUCCESS);

#if !defined(FX_ENABLE_FAULT_TOLERANT) && !defined(FX_DISABLE_CACHE)
    /* Make an IO error when fx_unicode_file_create call  _fx_directory_search. */
    length = fx_unicode_length_get(long_unicode_name1);
    _fx_ram_driver_io_error_request = 10;
    status = fx_unicode_file_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name);
    return_if_fail( status == FX_IO_ERROR);
#endif

    /* Close the media. */
    status = fx_media_close(&ram_disk); 
    return_if_fail( status == FX_SUCCESS);

    /* Format the media with FAT32.  This needs to be done before opening it!  */
    status = fx_media_format(&ram_disk, 
                              _fx_ram_driver,         // Driver entry            
                              ram_disk_memory_large,  // RAM disk memory pointer
                              cache_buffer,           // Media buffer pointer
                              CACHE_SIZE,             // Media buffer size 
                              "MY_RAM_DISK",          // Volume Name
                              1,                      // Number of FATs
                              32,                     // Directory Entries
                              0,                      // Hidden sectors
                              70000,                  // Total sectors 
                              128,                    // Sector size   
                              1,                      // Sectors per cluster
                              1,                      // Heads
                              1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    length = fx_unicode_length_get(new_file_name);
    status = fx_unicode_file_create( &ram_disk, new_file_name, length, (CHAR *)destination_name);
    return_if_fail( status == FX_SUCCESS);

#if !defined(FX_ENABLE_FAULT_TOLERANT) && !defined(FX_DISABLE_CACHE)
    /* Make an IO error when fx_unicode_file_create call  _fx_unicode_directory_entry_change. */
    length = fx_unicode_length_get(long_unicode_name1);
    _fx_ram_driver_io_error_request = 11;
    status = fx_unicode_file_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name);
    return_if_fail( status == FX_IO_ERROR);
#endif

    /* Close the media. */
    status = fx_media_close(&ram_disk); 
    return_if_fail( status == FX_SUCCESS);
    
    fx_media_abort( &ram_disk);

    /* Try fx_unicode_direcotry_create this time. */
    /* Format the media with FAT32.  This needs to be done before opening it!  */
    status += fx_media_format(&ram_disk, 
                              _fx_ram_driver,         // Driver entry            
                              ram_disk_memory_large,  // RAM disk memory pointer
                              cache_buffer,           // Media buffer pointer
                              CACHE_SIZE,             // Media buffer size 
                              "MY_RAM_DISK",          // Volume Name
                              1,                      // Number of FATs
                              32,                     // Directory Entries
                              0,                      // Hidden sectors
                              70000,                  // Total sectors 
                              128,                    // Sector size   
                              1,                      // Sectors per cluster
                              1,                      // Heads
                              1);                     // Sectors per track 
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    length = fx_unicode_length_get(new_file_name);
    status = fx_unicode_directory_create( &ram_disk, new_file_name, length, (CHAR *)destination_name);
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO error when fx_unicode_file_create call  _fx_unicode_directory_entry_change. */
    length = fx_unicode_length_get(long_unicode_name1);
    _fx_ram_driver_io_error_request = 14;
    status = fx_unicode_directory_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name);
    return_if_fail( status == FX_IO_ERROR);

    /* Close the media. */
    status = fx_media_close(&ram_disk); 
    return_if_fail( status == FX_SUCCESS);

    fx_media_abort( &ram_disk);

    /* Try fx_unicode_file_rename this time. */
    /* Format the media with FAT32.  This needs to be done before opening it!  */
    status = fx_media_format(&ram_disk, 
                              _fx_ram_driver,         // Driver entry            
                              ram_disk_memory_large,  // RAM disk memory pointer
                              cache_buffer,           // Media buffer pointer
                              CACHE_SIZE,             // Media buffer size 
                              "MY_RAM_DISK",          // Volume Name
                              1,                      // Number of FATs
                              32,                     // Directory Entries
                              0,                      // Hidden sectors
                              70000,                  // Total sectors 
                              128,                    // Sector size   
                              1,                      // Sectors per cluster
                              1,                      // Heads
                              1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    length = fx_unicode_length_get(new_file_name);
    status = fx_unicode_file_create( &ram_disk, new_file_name, length, (CHAR *)destination_name);
    return_if_fail( status == FX_SUCCESS);

    length = fx_unicode_length_get(long_unicode_name1);
    status = fx_unicode_file_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name);
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO ERROR when fx_unicode_file_rename call _fx_unicode_directory_entry_change. */
    old_length = fx_unicode_length_get( long_unicode_name1);
    new_length = fx_unicode_length_get( long_unicode_name2);
    _fx_utility_logical_sector_read_error_request = 27;
    status = fx_unicode_file_rename( &ram_disk, long_unicode_name1, old_length, long_unicode_name2, new_length, (CHAR *)destination_name);
    return_if_fail( status == FX_IO_ERROR);

    /* Close the media. */
    status = fx_media_close(&ram_disk); 
    return_if_fail( status == FX_SUCCESS);

    fx_media_abort( &ram_disk);

    /* Try fx_unicode_file_rename this time. */
    /* Format the media with FAT32.  This needs to be done before opening it!  */
    status = fx_media_format(&ram_disk, 
                              _fx_ram_driver,         // Driver entry            
                              ram_disk_memory_large,  // RAM disk memory pointer
                              cache_buffer,           // Media buffer pointer
                              CACHE_SIZE,             // Media buffer size 
                              "MY_RAM_DISK",          // Volume Name
                              1,                      // Number of FATs
                              32,                     // Directory Entries
                              0,                      // Hidden sectors
                              70000,                  // Total sectors 
                              128,                    // Sector size   
                              1,                      // Sectors per cluster
                              1,                      // Heads
                              1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    length = fx_unicode_length_get(new_file_name);
    status = fx_unicode_file_create( &ram_disk, new_file_name, length, (CHAR *)destination_name);
    return_if_fail( status == FX_SUCCESS);

    length = fx_unicode_length_get(long_unicode_name1);
    status = fx_unicode_file_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name);
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO ERROR when fx_unicode_file_rename call _fx_directory_search. */
    old_length = fx_unicode_length_get( long_unicode_name1);
    new_length = fx_unicode_length_get( long_unicode_name2);
    _fx_utility_logical_sector_read_error_request = 32;
    status = fx_unicode_file_rename( &ram_disk, long_unicode_name1, old_length, long_unicode_name2, new_length, (CHAR *)destination_name);
    return_if_fail( status == FX_IO_ERROR);

    /* Close the media. */
    status = fx_media_close(&ram_disk); 
    return_if_fail( status == FX_SUCCESS);

    fx_media_abort( &ram_disk);

    /* Try fx_unicode_directory_rename this time. */
    /* Format the media with FAT32.  This needs to be done before opening it!  */
    status = fx_media_format(&ram_disk, 
                              _fx_ram_driver,         // Driver entry            
                              ram_disk_memory_large,  // RAM disk memory pointer
                              cache_buffer,           // Media buffer pointer
                              CACHE_SIZE,             // Media buffer size 
                              "MY_RAM_DISK",          // Volume Name
                              1,                      // Number of FATs
                              32,                     // Directory Entries
                              0,                      // Hidden sectors
                              70000,                  // Total sectors 
                              128,                    // Sector size   
                              1,                      // Sectors per cluster
                              1,                      // Heads
                              1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    length = fx_unicode_length_get(new_file_name);
    status = fx_unicode_directory_create( &ram_disk, new_file_name, length, (CHAR *)destination_name);
    return_if_fail( status == FX_SUCCESS);

    length = fx_unicode_length_get(long_unicode_name1);
    status = fx_unicode_directory_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name);
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO ERROR when fx_unicode_file_rename call _fx_directory_search. */
    old_length = fx_unicode_length_get( long_unicode_name1);
    new_length = fx_unicode_length_get( long_unicode_name2);
    _fx_utility_logical_sector_read_error_request = 27;
    status = fx_unicode_directory_rename( &ram_disk, long_unicode_name1, old_length, long_unicode_name2, new_length, (CHAR *)destination_name);
    return_if_fail( status == FX_IO_ERROR);

    /* Close the media. */
    status = fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    fx_media_abort( &ram_disk);

    /* Try fx_unicode_directory_rename this time. */
    /* Format the media with FAT32.  This needs to be done before opening it!  */
    status = fx_media_format(&ram_disk, 
                              _fx_ram_driver,         // Driver entry            
                              ram_disk_memory_large,  // RAM disk memory pointer
                              cache_buffer,           // Media buffer pointer
                              CACHE_SIZE,             // Media buffer size 
                              "MY_RAM_DISK",          // Volume Name
                              1,                      // Number of FATs
                              32,                     // Directory Entries
                              0,                      // Hidden sectors
                              70000,                  // Total sectors 
                              128,                    // Sector size   
                              1,                      // Sectors per cluster
                              1,                      // Heads
                              1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    length = fx_unicode_length_get(new_file_name);
    status = fx_unicode_directory_create( &ram_disk, new_file_name, length, (CHAR *)destination_name);
    return_if_fail( status == FX_SUCCESS);

    length = fx_unicode_length_get(long_unicode_name1);
    status = fx_unicode_directory_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name);
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO ERROR when fx_unicode_file_rename call _fx_unicode_directory_entry_change. */
    old_length = fx_unicode_length_get( long_unicode_name1);
    new_length = fx_unicode_length_get( long_unicode_name2);
    _fx_utility_logical_sector_read_error_request = 32;
    status = fx_unicode_directory_rename( &ram_disk, long_unicode_name1, old_length, long_unicode_name2, new_length, (CHAR *)destination_name);
    return_if_fail( status == FX_IO_ERROR);

    /* Close the media. */
    status = fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    fx_media_abort( &ram_disk);

    /* Format the media with FAT32.  This needs to be done before opening it!  */
    status = fx_media_format(&ram_disk, 
                              _fx_ram_driver,         // Driver entry            
                              ram_disk_memory_large,  // RAM disk memory pointer
                              cache_buffer,           // Media buffer pointer
                              CACHE_SIZE,             // Media buffer size 
                              "MY_RAM_DISK",          // Volume Name
                              1,                      // Number of FATs
                              32,                     // Directory Entries
                              0,                      // Hidden sectors
                              70000,                  // Total sectors 
                              128,                    // Sector size   
                              1,                      // Sectors per cluster
                              1,                      // Heads
                              1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO error when fx_unicode_directory_rename call  _fx_directory_search. */
    _fx_ram_driver_io_error_request = 1;
    length = fx_unicode_length_get(new_file_name);
    status = fx_unicode_directory_create( &ram_disk, new_file_name, length, (CHAR *)destination_name);

    /* Close the media. */
    status = fx_media_close(&ram_disk); 
    return_if_fail( status == FX_SUCCESS);
    
    fx_media_abort( &ram_disk);

    /* Format the media with FAT32.  This needs to be done before opening it!  */
    status = fx_media_format(&ram_disk, 
                              _fx_ram_driver,    // Driver entry            
                              ram_disk_memory_large,  // RAM disk memory pointer
                              cache_buffer,           // Media buffer pointer
                              CACHE_SIZE,             // Media buffer size 
                              "MY_RAM_DISK",          // Volume Name
                              1,                      // Number of FATs
                              32,                     // Directory Entries
                              0,                      // Hidden sectors
                              70000,                  // Total sectors 
                              128,                    // Sector size   
                              1,                      // Sectors per cluster
                              1,                      // Heads
                              1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    length = fx_unicode_length_get(new_file_name);
    status = fx_unicode_directory_create( &ram_disk, new_file_name, length, (CHAR *)destination_name);
    return_if_fail( status == FX_SUCCESS);

    length = fx_unicode_length_get(long_unicode_name1);
    status = fx_unicode_directory_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name);
    return_if_fail( status == FX_SUCCESS);

    /* Flush everything out. */
    fx_media_flush( &ram_disk);
    _fx_utility_FAT_flush( &ram_disk);
    _fx_utility_logical_sector_flush( &ram_disk, 1, 60000, FX_TRUE);
    for (UINT i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster = 0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value = 0;
    }

#ifndef FX_ENABLE_FAULT_TOLERANT

    /* Make an IO ERROR in fx_unicode_directory_entry_read while calling _fx_utility_FAT_entry_read. */
    old_length = fx_unicode_length_get( long_unicode_name1);
    new_length = fx_unicode_length_get( long_unicode_name2);
    _fx_ram_driver_io_error_request = 2;
    status = fx_unicode_directory_rename( &ram_disk, long_unicode_name1, old_length, long_unicode_name2, new_length, (CHAR *)destination_name);
    return_if_fail( status == FX_IO_ERROR);

#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Close the media. */
    status = fx_media_close(&ram_disk); 
    return_if_fail( status == FX_SUCCESS);
    
    fx_media_abort( &ram_disk);

    /* Format the media with FAT32.  This needs to be done before opening it!  */
    status = fx_media_format(&ram_disk, 
                              _fx_ram_driver,         // Driver entry            
                              ram_disk_memory_large,  // RAM disk memory pointer
                              cache_buffer,           // Media buffer pointer
                              0,                      // Media buffer size 
                              "MY_RAM_DISK",          // Volume Name
                              1,                      // Number of FATs
                              32,                     // Directory Entries
                              0,                      // Hidden sectors
                              70000,                  // Total sectors 
                              128,                    // Sector size   
                              1,                      // Sectors per cluster
                              1,                      // Heads
                              1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    length = fx_unicode_length_get(new_file_name);
    status = fx_unicode_directory_create( &ram_disk, new_file_name, length, (CHAR *)destination_name);
    return_if_fail( status == FX_SUCCESS);

#ifndef FX_ENABLE_FAULT_TOLERANT

    /* Make an IO error in _fx_unicode_directory_entry_change while calling _fx_utility_logical_sector_read at Line 358. */
    _fx_utility_logical_sector_read_error_request = 24;
    length = fx_unicode_length_get(long_unicode_name1);
    status = fx_unicode_directory_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name);
    return_if_fail( status == FX_IO_ERROR);

#endif

    /* Close the media. */
    status = fx_media_close(&ram_disk); 
    return_if_fail( status == FX_SUCCESS);
    
    fx_media_abort( &ram_disk);

    /* Format the media with FAT32.  This needs to be done before opening it!  */
    status = fx_media_format(&ram_disk, 
                              _fx_ram_driver,         // Driver entry
                              ram_disk_memory_large,  // RAM disk memory pointer
                              cache_buffer,           // Media buffer pointer
                              0,                      // Media buffer size 
                              "MY_RAM_DISK",          // Volume Name
                              1,                      // Number of FATs
                              32,                     // Directory Entries
                              0,                      // Hidden sectors
                              70000,                  // Total sectors s
                              128,                    // Sector size   
                              1,                      // Sectors per cluster
                              1,                      // Heads
                              1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    length = fx_unicode_length_get(new_file_name);
    status = fx_unicode_directory_create( &ram_disk, new_file_name, length, (CHAR *)destination_name);
#ifndef FX_STANDALONE_ENABLE
    status =   fx_directory_local_path_set(&ram_disk, &local_path, (CHAR *)destination_name);
#else
    status =   fx_directory_default_set(&ram_disk, (CHAR *)destination_name);
#endif
    return_if_fail( status == FX_SUCCESS);

    /* Create enough files to enforce cache flush. */
    temp = long_unicode_name1[0];
    for (UINT i = 0; i < 76; i++)
    {
        long_unicode_name1[0]++;
        length = fx_unicode_length_get(long_unicode_name1);
        status = fx_unicode_directory_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name);
    }
    return_if_fail( status == FX_SUCCESS);

    /* fx_unicode_directory_create will determine whether the file to be created is already existed by calling fx_unicode_directory_search. */
    /* The search will fail because of IO_ERROR rather than FILE_ALREADY_CREATED, but the creation of file will still succeeded, since we recover the driver asap. :) */
    long_unicode_name1[0]++;
    length = fx_unicode_length_get(long_unicode_name1);
    _fx_ram_driver_io_error_request = 22;
    status = fx_unicode_directory_create( &ram_disk, long_unicode_name1, length, (CHAR *)destination_name);
    long_unicode_name1[0] = temp;
    return_if_fail( status == FX_SUCCESS);
    
    status = fx_media_close(&ram_disk); 
    return_if_fail( status == FX_SUCCESS);
    
    printf("SUCCESS!\n");
    test_control_return(0);
}
