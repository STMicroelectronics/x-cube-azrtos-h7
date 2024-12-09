/* This test is determined to cover lines in fx_unicode_directory_entry_read.c.                 */
/* We need a dir_entry whose first byte, ordinal number byte, is big enoutgh to exceed the the  */
/* limit of FX_MAX_LONG_NAME_LEN, so we created a disk with a corrupt dir_entry.                */
#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#include   "tx_thread.h"
#endif
#include   "fx_api.h"
#include   "fx_ram_driver_test.h"
#include   "fx_utility.h"
#include   "fx_unicode.h"
#include   <stdio.h>

#define     DEMO_STACK_SIZE         8192
#define     CACHE_SIZE              16*128

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

void    filex_unicode_directory_entry_test_application_define(void *first_unused_memory);
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
        (driver_called_counter <= 3)
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
void    filex_unicode_directory_entry_test_application_define(void *first_unused_memory)
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

/* Define the test threads.  */

static void    ftest_0_entry(ULONG thread_input)
{

UINT        status;
UCHAR       destination_name[100] = {0};
UCHAR       buffer[512];
UCHAR       long_unicode_name[] =   {2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 15, 0, 0, 0};
ULONG       length = 14;
ULONG       dir_num1, dir_num2;
FX_LOCAL_PATH local_path;
FX_DIR_ENTRY source_dir, destination_dir;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Unicode directory entry test...........................");


    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            512,                    // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    return_value_if_fail( status == FX_SUCCESS, 2);
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_value_if_fail( status == FX_SUCCESS, 3);

    /* For coverage test in fx_directory_free_search.c.
     387                 :       4380 :                     status = _fx_directory_entry_write(media_ptr, entry_ptr);
     388         [ -  + ]:       4380 :                     if(status != FX_SUCCESS)
     389                 :            :                     {
     390                 :          0 :                         return(status);
     391                 :            :                     }
     */
    _fx_directory_entry_write_error_request = 1;

    /* call fx_unicode_directory_create -call-> _fx_unicode_directory_search -call-> _fx_unicode_directory_entry_read */
    status = fx_unicode_directory_create(&ram_disk,  long_unicode_name, length, (CHAR *) destination_name);
    return_value_if_fail( status == FX_IO_ERROR, 3);

    /* Close the media to flush buffer. Now we have a disk with a corrupt dir_entry. */
    status = fx_media_close(&ram_disk);
    return_value_if_fail( status == FX_SUCCESS, 4);


    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            512,                    // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    return_value_if_fail( status == FX_SUCCESS, 2);
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_value_if_fail( status == FX_SUCCESS, 3);

    /* Disable write protect */
    ram_disk.fx_media_driver_write_protect = FX_FALSE;

    /* call fx_unicode_directory_create -call-> _fx_unicode_directory_search -call-> _fx_unicode_directory_entry_read */
    status = fx_unicode_directory_create(&ram_disk,  long_unicode_name, length, (CHAR *) destination_name);
    return_value_if_fail( status == FX_SUCCESS, 3);

    /* After creating first directory on a new disk, the first dir_entry must be located at the head of the disk memory buffer. */
    /* Modify the first byte, ordinal number byte, to make a mistake. */
    *ram_disk.fx_media_memory_buffer |= 0x1f;

    /* Close the media to flush buffer. Now we have a disk with a corrupt dir_entry. */
    status = fx_media_close(&ram_disk);
    return_value_if_fail( status == FX_SUCCESS, 4);

    /* Open the media and try to create the directory again to see what happened. */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_value_if_fail( status == FX_SUCCESS, 5);

#if !defined(FX_ENABLE_FAULT_TOLERANT) && !defined(FX_DISABLE_CACHE)
    /* Try to recreate the same directory to see what happened. */
    status = fx_unicode_directory_create(&ram_disk,  long_unicode_name, length, (CHAR *) destination_name);
    return_value_if_fail( status == FX_SUCCESS, 6);
#endif

    /* At this point the three dir_entry belongs to original directory is invalid(new dir_entry's byteoffset in memory buffer is 96 bytes). */

    /* Close the media to flush buffer. Now we have a disk with a corrupt dir_entry. */
    status = fx_media_close(&ram_disk);
    return_value_if_fail( status == FX_SUCCESS, 7);

    /* This time, we'll modify the second dir_entry's first byte, ordinal byte, which will make the filesystem keep reading entries until FX_FILE_CORRUPT. */
    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            512,                    // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Open the ram_disk.  */
    status +=  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_value_if_fail( status == FX_SUCCESS, 8);

    /* Disable write protect */
    ram_disk.fx_media_driver_write_protect = FX_FALSE;

    /* call fx_unicode_directory_create -call-> _fx_unicode_directory_search -call-> _fx_unicode_directory_entry_read */
    status = fx_unicode_directory_create(&ram_disk,  long_unicode_name, length, (CHAR *) destination_name);
    return_value_if_fail( status == FX_SUCCESS, 9);

    /* Modify second dir_entry this time to make a mistake. */
    ram_disk.fx_media_memory_buffer[32] |= 0x1f;

    /* Close the media to flush buffer. Now we have a disk with a corrupt dir_entry. */
    status = fx_media_close(&ram_disk);
    return_value_if_fail( status == FX_SUCCESS, 10);

    /* Open the media and try to create the directory again to see what happened. */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_value_if_fail( status == FX_SUCCESS, 11);

#if !defined(FX_ENABLE_FAULT_TOLERANT) && !defined(FX_DISABLE_CACHE)
    /* Try to recreate the same directory to see what happened. */
    status = fx_unicode_directory_create(&ram_disk,  long_unicode_name, length, (CHAR *) destination_name);

    /* Since there are no appropriate long name dir_entry behind, the filesystem will read data continously untill logic sector overflow. */
    return_value_if_fail( status == FX_FILE_CORRUPT, 12);
#endif

    /* Add terrible driver to make IO mistake. */
    status = fx_media_close(&ram_disk);
    return_value_if_fail( status == FX_SUCCESS, 13);

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            512,                    // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_value_if_fail( status == FX_SUCCESS, 14);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_value_if_fail( status == FX_SUCCESS, 15);

    /* Register our terrible driver. */
    ram_disk.fx_media_driver_entry = _fx_terrible_driver;
    status = fx_unicode_directory_create(&ram_disk,  long_unicode_name, length, (CHAR *) destination_name);
    return_value_if_fail( status == FX_IO_ERROR, 16);

    /* Unregister our terrible driver. */
    ram_disk.fx_media_driver_entry = _fx_ram_driver;

    status = fx_media_abort( &ram_disk);
    return_value_if_fail( status == FX_SUCCESS, 17);

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            128,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            512,                    // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Open the ram_disk.  */
    status +=  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_value_if_fail( status == FX_SUCCESS, 18);

    /* Set local path at a sub_directory. */
    length = fx_unicode_length_get( long_unicode_name);
    status = fx_unicode_directory_create( &ram_disk,  long_unicode_name, length, (CHAR *)destination_name);
#ifndef FX_STANDALONE_ENABLE
    status +=   fx_directory_local_path_set(&ram_disk, &local_path, (CHAR *)destination_name);
#else
    status +=   fx_directory_default_set(&ram_disk, (CHAR *)destination_name);
#endif
    return_value_if_fail( status == FX_SUCCESS, 19);

    /* Call _fx_unicode_directory_entry_read with uncorrect initial information. */
    destination_dir.fx_dir_entry_name = (CHAR *)buffer;
    source_dir.fx_dir_entry_last_search_cluster = 1;
    source_dir.fx_dir_entry_last_search_relative_cluster = 1;
    dir_num1 = 0;
    status = _fx_unicode_directory_entry_read( &ram_disk, &source_dir, &dir_num1, &destination_dir, destination_name, &dir_num2);

    /* Call _fx_unicode_directory_entry_read with uncorrect initial information. */
    source_dir.fx_dir_entry_last_search_cluster = 1;
    source_dir.fx_dir_entry_last_search_relative_cluster = 0;
    source_dir.fx_dir_entry_last_search_log_sector = 1;
    source_dir.fx_dir_entry_log_sector = 0;
    status = _fx_unicode_directory_entry_read( &ram_disk, &source_dir, &dir_num1, &destination_dir, destination_name, &dir_num2);

    /* Call _fx_unicode_directory_entry_read with uncorrect initial information. */
    source_dir.fx_dir_entry_last_search_cluster = 1;
    source_dir.fx_dir_entry_last_search_relative_cluster = 0;
    source_dir.fx_dir_entry_last_search_byte_offset = 0x20;
    source_dir.fx_dir_entry_byte_offset = 0;
    source_dir.fx_dir_entry_last_search_log_sector = 0;
    source_dir.fx_dir_entry_log_sector = 0;
    status = _fx_unicode_directory_entry_read( &ram_disk, &source_dir, &dir_num1, &destination_dir, destination_name, &dir_num2);

    /* Attempt to cover the branch at Line 140 in fx_utility_FAT_map_flush.c. */
    for (UINT i = 0; i < 128; i++)
        ram_disk.fx_media_fat_secondary_update_map[i] = 0xff;
    _fx_utility_FAT_map_flush( &ram_disk);

    printf("SUCCESS!\n");
    test_control_return(0);
}
