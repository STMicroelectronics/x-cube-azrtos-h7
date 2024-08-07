/* Test the condition of fat entry broken. */
#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_utility.h"
#include   "fx_ram_driver_test.h"
#include    <stdio.h>

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

void    filex_unicode_file_directory_rename_extra_test_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);

/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_unicode_file_directory_rename_extra_test_application_define(void *first_unused_memory)
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

static UCHAR                    new_file_name5[] =      {'a', 0, 'b', 0, 'c', 0, 0, 0}; 
static UCHAR                    new_file_name5_bak[] =  {'a', 0, 'b', 0, 'c', 0, 0, 0}; 
static UCHAR                    new_file_name6[] =      {'1', 0, 'b', 0, 'c', 0, 'd', 0, 0, 0}; 
static UCHAR                    destination_name[100];

/* Define the test threads.  */

static void    ftest_0_entry(ULONG thread_input)
{

UINT status, length, count, new_length, old_length;
UCHAR temp;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Unicode file directory rename extra test...............");

    /* Format the media with FAT32.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                              _fx_ram_driver,         // Driver entry            
                              ram_disk_memory_large,  // RAM disk memory pointer
                              cache_buffer,           // Media buffer pointer
                              CACHE_SIZE,             // Media buffer size 
                              "MY_RAM_DISK",          // Volume Name
                              1,                      // Number of FATs
                              32,                     // Directory Entries
                              0,                      // Hidden sectors
                              70000 * 8,              // Total sectors 
                              256,                    // Sector size   
                              8,                      // Sectors per cluster
                              1,                      // Heads
                              1);                     // Sectors per track 

    return_value_if_fail( status == FX_SUCCESS, 1);
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_value_if_fail( status == FX_SUCCESS, 2);

    length = fx_unicode_length_get(new_file_name5);
    status = fx_unicode_file_create( &ram_disk, new_file_name5, length, (CHAR *)destination_name);
    return_value_if_fail( status == FX_SUCCESS, 3);

    temp = new_file_name5[0];
    
    /* Attempt to create more than 26 files with unicode names with the same length. */
    for ( count = 0; count < 28; count++)
    {
        new_file_name5[0]++;
        length = fx_unicode_length_get(new_file_name5);
        status = fx_unicode_file_create( &ram_disk, new_file_name5, length, (CHAR *)destination_name);
        if ( count <=24)
        {

            /* Succeed while less than 26 files is created. */
            return_value_if_fail( status == FX_SUCCESS, 4 + count);
        }
        else
        {

            /* Exceeded the limitation. */
            return_value_if_fail( status == FX_ALREADY_CREATED, 4 + count);
        }
    }

    length = fx_unicode_length_get(new_file_name6);
    status = fx_unicode_file_create( &ram_disk, new_file_name6, length, (CHAR *)destination_name);
    return_value_if_fail( status == FX_SUCCESS, 32);

    /* Touch the limitation by renaming. */
    new_length = fx_unicode_length_get(new_file_name5);
    old_length = fx_unicode_length_get(new_file_name6);
    status = fx_unicode_file_rename( &ram_disk, new_file_name6, old_length, new_file_name5, new_length, (CHAR *)destination_name);
    return_value_if_fail( status == FX_ALREADY_CREATED, 33);

    /* Renaming a file with the same length is premitted. */
    new_length = fx_unicode_length_get(new_file_name5);
    old_length = fx_unicode_length_get(new_file_name5_bak);
    status = fx_unicode_file_rename( &ram_disk, new_file_name5_bak, old_length, new_file_name5, new_length, (CHAR *)destination_name);
    return_value_if_fail( status == FX_SUCCESS, 34);

    /* Recover new_file_name5 for the test for renaming directory. */
    new_file_name5[0] = temp;

    /* Abort the disk to reuse memory. */
    fx_media_abort( &ram_disk);

    /* Format the media with FAT32.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                              _fx_ram_driver,         // Driver entry            
                              ram_disk_memory_large,  // RAM disk memory pointer
                              cache_buffer,           // Media buffer pointer
                              CACHE_SIZE,             // Media buffer size 
                              "MY_RAM_DISK",          // Volume Name
                              1,                      // Number of FATs
                              32,                     // Directory Entries
                              0,                      // Hidden sectors
                              70000 * 8,              // Total sectors 
                              256,                    // Sector size   
                              8,                      // Sectors per cluster
                              1,                      // Heads
                              1);                     // Sectors per track 

    return_value_if_fail( status == FX_SUCCESS, 1);
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_value_if_fail( status == FX_SUCCESS, 2);

    length = fx_unicode_length_get(new_file_name5);
    status = fx_unicode_directory_create( &ram_disk, new_file_name5, length, (CHAR *)destination_name);
    return_value_if_fail( status == FX_SUCCESS, 3);

    temp = new_file_name5[0];
    
    /* Attempt to create more than 26 files with unicode names with the same length. */
    for ( count = 0; count < 28; count++)
    {
        new_file_name5[0]++;
        length = fx_unicode_length_get(new_file_name5);
        status = fx_unicode_directory_create( &ram_disk, new_file_name5, length, (CHAR *)destination_name);
        if ( count <=24)
        {

            /* Succeed while less than 26 files is created. */
            return_value_if_fail( status == FX_SUCCESS, 4 + count);
        }
        else
        {

            /* Exceeded the limitation. */
            return_value_if_fail( status == FX_ALREADY_CREATED, 4 + count);
        }
    }

    length = fx_unicode_length_get(new_file_name6);
    status = fx_unicode_directory_create( &ram_disk, new_file_name6, length, (CHAR *)destination_name);
    return_value_if_fail( status == FX_SUCCESS, 32);

    /* Touch the limitation by renaming. */
    new_length = fx_unicode_length_get(new_file_name5);
    old_length = fx_unicode_length_get(new_file_name6);
    status = fx_unicode_directory_rename( &ram_disk, new_file_name6, old_length, new_file_name5, new_length, (CHAR *)destination_name);
    return_value_if_fail( status == FX_ALREADY_CREATED, 33);

    /* Renaming a file with the same length is premitted. */
    new_length = fx_unicode_length_get(new_file_name5);
    old_length = fx_unicode_length_get(new_file_name5_bak);
    status = fx_unicode_directory_rename( &ram_disk, new_file_name5_bak, old_length, new_file_name5, new_length, (CHAR *)destination_name);
    return_value_if_fail( status == FX_SUCCESS, 34);
    printf("SUCCESS!\n");
    test_control_return(0);
}
