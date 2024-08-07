/*This test is determined to cover lines 315 327 352 390 in fx_unicode_directory_entry_change.c. */

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
static UCHAR                     cache_buffer[256];
#endif

/* Define thread prototypes.  */

void    filex_unicode_directory_entry_change_test_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);

extern UINT  _fx_unicode_directory_entry_change(FX_MEDIA *media_ptr, FX_DIR_ENTRY *entry_ptr, UCHAR *unicode_name, ULONG unicode_name_length);

static  UINT driver_called_counter = 0;
/* Create a terrible driver. */
static void _fx_terrible_driver(FX_MEDIA *media_ptr)
{
    driver_called_counter++;
    // printf("\n_fx_terrible_driver has been called %d times.", driver_called_counter);
    if (driver_called_counter == 3)
    {
        media_ptr->fx_media_driver_status=FX_IO_ERROR;
        return;
    }
    (*_fx_ram_driver)(media_ptr);
}

/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_unicode_directory_entry_change_test_application_define(void *first_unused_memory)
#endif
{
#ifndef FX_STANDALONE_ENABLE
    UCHAR    *pointer;

    /* Setup the working pointer.  */
    pointer = (UCHAR *)first_unused_memory;

    /* Create the main thread.  */
    tx_thread_create(&ftest_0, "thread 0", ftest_0_entry, 0,
        pointer, DEMO_STACK_SIZE,
        4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer = pointer + DEMO_STACK_SIZE;

    /* Setup memory for the RAM disk and the sector cache.  */
    cache_buffer = pointer;
    pointer = pointer + CACHE_SIZE;
    ram_disk_memory = pointer;

#endif

    /* Initialize the FileX system.  */
    fx_system_initialize();

#ifdef FX_STANDALONE_ENABLE
    ftest_0_entry(0);
#endif
}

static UCHAR                    long_unicode_name1[] = { 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 0, 0, 0, 0, 0,};

/* Define the test threads.  */

static void    ftest_0_entry(ULONG thread_input)
{
FX_DIR_ENTRY entry;  /* set a point to the entry  */
FX_DIR_ENTRY  *entry_ptr=&entry;
UINT   status, length;

    FX_PARAMETER_NOT_USED(thread_input);
    
    /* Print out some test information banners.  */
    printf("FileX Test:   Unicode directory entry change test....................");

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

    return_value_if_fail(status == FX_SUCCESS, 1);

    status = fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_value_if_fail(status == FX_SUCCESS, 2);

    /* initiallize the entry  */
    entry_ptr->fx_dir_entry_name = "Z12345";
    entry_ptr->fx_dir_entry_attributes = 16;
    entry_ptr->fx_dir_entry_byte_offset = 0;
    entry_ptr->fx_dir_entry_cluster = 3;
    entry_ptr->fx_dir_entry_created_date = 18977;
    entry_ptr->fx_dir_entry_created_time = 0;
    entry_ptr->fx_dir_entry_created_time_ms = 0;
    entry_ptr->fx_dir_entry_date = 18977;
    entry_ptr->fx_dir_entry_file_size = 0;
    entry_ptr->fx_dir_entry_last_accessed_date = 18977;
    entry_ptr->fx_dir_entry_last_search_byte_offset = 0;
    entry_ptr->fx_dir_entry_last_search_cluster = 0;
    entry_ptr->fx_dir_entry_last_search_log_sector = 0;
    entry_ptr->fx_dir_entry_last_search_relative_cluster = 0;
    entry_ptr->fx_dir_entry_log_sector = 2189;
    entry_ptr->fx_dir_entry_long_name_present = 1;
    entry_ptr->fx_dir_entry_long_name_shorted = 0;
    entry_ptr->fx_dir_entry_next_log_sector = 0;
    entry_ptr->fx_dir_entry_reserved = 0;
    entry_ptr->fx_dir_entry_time = 0;

    /*This case is determined to cover lines 390 in fx_unicode_directory_entry_change.c. */
    length = fx_unicode_length_get(long_unicode_name1);
    status=_fx_unicode_directory_entry_change(&ram_disk, entry_ptr, long_unicode_name1, length);
    return_value_if_fail(status == FX_FILE_CORRUPT, 3);

    /*This case is determined to cover lines 327 in fx_unicode_directory_entry_change.c. */
    *(ram_disk.fx_media_driver_buffer) = 90;
    status = _fx_unicode_directory_entry_change(&ram_disk, entry_ptr, long_unicode_name1, length);
    return_value_if_fail(status == FX_FILE_CORRUPT, 4);

    /*This case is determined to cover lines 352 in fx_unicode_directory_entry_change.c. */
    entry_ptr->fx_dir_entry_byte_offset = CACHE_SIZE + 1;
    *(ram_disk.fx_media_driver_buffer+ CACHE_SIZE+1) = 90;

    /*set the the logical sector that is in root directory */
    entry_ptr->fx_dir_entry_log_sector =ram_disk.fx_media_data_sector_start-1;
    status = _fx_unicode_directory_entry_change(&ram_disk, entry_ptr, long_unicode_name1, length);
    return_value_if_fail(status == FX_FILE_CORRUPT, 5);
    
    /*This case is determined to cover lines 315 in fx_unicode_directory_entry_change.c. */
    entry_ptr->fx_dir_entry_byte_offset = CACHE_SIZE + 1;
    *(ram_disk.fx_media_driver_buffer + CACHE_SIZE + 1) = 90;
    entry_ptr->fx_dir_entry_log_sector = ram_disk.fx_media_data_sector_start ;
    ram_disk.fx_media_fat_cache[8].fx_fat_cache_entry_cluster = 20;

    /* Register our terrible dirver to make IO ERROR at a particular time. */
    ram_disk.fx_media_driver_entry = _fx_terrible_driver;
    status = _fx_unicode_directory_entry_change(&ram_disk, entry_ptr, long_unicode_name1, length);
    /* Unregister our terrible driver. */
    ram_disk.fx_media_driver_entry = _fx_ram_driver;
    return_value_if_fail(status == FX_IO_ERROR, 6);

    /* initiallize the entry  */
    entry_ptr->fx_dir_entry_name = "Z12345";
    entry_ptr->fx_dir_entry_attributes = 16;
    entry_ptr->fx_dir_entry_byte_offset = 0;
    entry_ptr->fx_dir_entry_cluster = 3;
    entry_ptr->fx_dir_entry_created_date = 18977;
    entry_ptr->fx_dir_entry_created_time = 0;
    entry_ptr->fx_dir_entry_created_time_ms = 0;
    entry_ptr->fx_dir_entry_date = 18977;
    entry_ptr->fx_dir_entry_file_size = 0;
    entry_ptr->fx_dir_entry_last_accessed_date = 18977;
    entry_ptr->fx_dir_entry_last_search_byte_offset = 0;
    entry_ptr->fx_dir_entry_last_search_cluster = 0;
    entry_ptr->fx_dir_entry_last_search_log_sector = 0;
    entry_ptr->fx_dir_entry_last_search_relative_cluster = 0;
    entry_ptr->fx_dir_entry_log_sector = 2189;
    entry_ptr->fx_dir_entry_long_name_present = 1;
    entry_ptr->fx_dir_entry_long_name_shorted = 0;
    entry_ptr->fx_dir_entry_next_log_sector = 0;
    entry_ptr->fx_dir_entry_reserved = 0;
    entry_ptr->fx_dir_entry_time = 0;

    /* Covered the last branch at Line 323 in fx_unicode_directory_entry_change.c. */
    *(ram_disk.fx_media_driver_buffer) = 90;
    /* Register a wrong information in fat cache. */
    ram_disk.fx_media_fat_cache[8].fx_fat_cache_entry_cluster = 2;
    ram_disk.fx_media_fat_cache[8].fx_fat_cache_entry_value = 0;
    status = _fx_unicode_directory_entry_change(&ram_disk, entry_ptr, long_unicode_name1, length);

    printf("SUCCESS!\n");
    test_control_return(0);
}
