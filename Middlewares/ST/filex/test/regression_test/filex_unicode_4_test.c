#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#include   "tx_thread.h"
#endif
#include   "fx_api.h"
#include   "fx_utility.h"
#include   "fx_ram_driver_test.h"
#include    <stdio.h>
#include    <string.h>

#define     DEMO_STACK_SIZE         8192
/* Set the cache size as the size of one sector causing frequently IO operation. */
#define     CACHE_SIZE              128

/* Define the global variable we may use in the future. */
extern ULONG _fx_ram_driver_copy_default_format;
extern UCHAR large_file_name_format[];

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

void    filex_unicode_4_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);

/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_unicode_4_application_define(void *first_unused_memory)
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

static UCHAR long_unicode_name1[] = { 1, 0, 0, 0 };
static UCHAR ascii_file_name[] = { 'a', 'b', 'c', 0 };
static UCHAR specified_ascii_file_name[] = { 'Z', '1', '2', '3', '4', '5', '6', '7', '.', 't', 'x', 't', 0 };
static UCHAR specified_unicode_file_name[] = { 'Z', 0, '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '.', 0, 't', 0, 'x', 0, 't', 0, 0,  0 };
static UCHAR specified_unicode_file_name_1[] = { 'a', 0, '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '.', 0, 't', 0, 'x', 0, 't', 0, 0,  0 };

#ifndef FX_DISABLE_ERROR_CHECKING
static UCHAR specified_unicode_file_name_2[] = { 'a', 0, '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '.', 0, 't', 0, 'x', 0, 0, 't', 0,  0 };
#endif /* FX_DISABLE_ERROR_CHECKING */

/* Define the test threads.  */

static void    ftest_0_entry(ULONG thread_input)
{

UINT status, length, count;
#ifndef FX_DISABLE_ERROR_CHECKING
UINT  new_length, old_length;
#endif /* FX_DISABLE_ERROR_CHECKING */
ULONG ulength;
UCHAR destination_name[128];
UCHAR destination_name_1[128];
UCHAR temp, temp_array[32];
FX_LOCAL_PATH local_path;
FX_FILE my_file;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Unicode 4 test.........................................");

    /* Format the media.  This needs to be done before opening it!  */
    status = fx_media_format(&ram_disk,
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

    return_value_if_fail(status == FX_SUCCESS, 2);
    status = fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_value_if_fail(status == FX_SUCCESS, 3);

    // Create a file whose name's first byte is 0xe5.
    ascii_file_name[0] = 0xe5;
    status = fx_file_create(&ram_disk, (CHAR *)ascii_file_name);
    return_value_if_fail(status == FX_SUCCESS, 4);

    // Attempt to create a unicode file which make fx_unicode_file_create call fx_unicode_directory_search to scan the directory before creation.
    // The file we just created will be read during the scannning so that the branch at Line 290 in fx_unicode_directory_search will be covered.
    length = fx_unicode_length_get(long_unicode_name1);
    status = fx_unicode_file_create(&ram_disk, long_unicode_name1, length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_SUCCESS, 5);

    // Attempt to create two 8.3 format file by both fx_file_create and fx_unicode_file_create to cover the branch at Line 324 in fx_unicode_directory_search.c 
    status = fx_file_create(&ram_disk, (CHAR *)specified_ascii_file_name);
    return_value_if_fail(status == FX_SUCCESS, 6);

    length = fx_unicode_length_get(specified_unicode_file_name);
    status = fx_unicode_file_create(&ram_disk, specified_unicode_file_name, length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_ALREADY_CREATED, 7);

    // Add test code to cover the branch at Line 146 in fx_unicode_directory_search.c
    ascii_file_name[0] = 'a';
    status = fx_directory_create(&ram_disk, (CHAR *)ascii_file_name);
#ifndef FX_STANDALONE_ENABLE
        status =   fx_directory_local_path_set(&ram_disk, &local_path, (CHAR *)ascii_file_name);
#else
        status =   fx_directory_default_set(&ram_disk, (CHAR *)ascii_file_name);
#endif
    return_value_if_fail(status == FX_SUCCESS, 8);

    // Clear the name of current directory.
#ifndef FX_STANDALONE_ENABLE
    ((FX_PATH*)_tx_thread_current_ptr->tx_thread_filex_ptr)->fx_path_directory.fx_dir_entry_name[0] = 0;
#else
        status =   fx_directory_default_set(&ram_disk, "\\");
#endif

    // The file is attempted to be created in the root directory, so it will fail.
    status = fx_unicode_file_create(&ram_disk, specified_unicode_file_name, length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_ALREADY_CREATED, 9);

    status = fx_media_close(&ram_disk);
    return_value_if_fail(status == FX_SUCCESS, 10);

    /* Open the ram_disk in fx_ram_dirver_test.c which contain a long file name dir_entry associated with a free short name entry. */
    _fx_ram_driver_copy_default_format = 1;
    status = fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_value_if_fail(status == FX_SUCCESS, 11);
    _fx_ram_driver_copy_default_format = 0;

    /* Cover the branch around Line 608 in fx_unicode_directory_entry_read.c. */
    length = fx_unicode_length_get(specified_unicode_file_name);
    status = fx_unicode_file_create(&ram_disk, specified_unicode_file_name, length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_SUCCESS, 12);

    /* Create a 8.3 format volume dirctory entry. */
    status = fx_media_volume_set(&ram_disk, "        .   ");
    return_value_if_fail(status == FX_SUCCESS, 13);

    /* Access our directory to cover the branch at Line 587 in fx_unicode_direcotry_entry_read. */
    length = fx_unicode_length_get(specified_unicode_file_name);
    status = fx_unicode_file_create(&ram_disk, specified_unicode_file_name, length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_ALREADY_CREATED, 14);

    /* Create format volume dirctory entry with 11 blanks. */
    status = fx_media_volume_set(&ram_disk, "           ");
    return_value_if_fail(status == FX_SUCCESS, 15);

    /* Access our directory to cover the branch at Line 619 in fx_unicode_direcotry_entry_read. */
    length = fx_unicode_length_get(specified_unicode_file_name);
    status = fx_unicode_file_create(&ram_disk, specified_unicode_file_name, length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_ALREADY_CREATED, 16);

    /* Changed first byte as lowercase. */
    specified_unicode_file_name[0] = 'z';
    status = fx_unicode_file_create(&ram_disk, specified_unicode_file_name, length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_ALREADY_CREATED, 17);

    /* Create a file whose name is begin with 'a'. */
    length = fx_unicode_length_get(specified_unicode_file_name_1);
    status = fx_unicode_file_create(&ram_disk, specified_unicode_file_name_1, length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_SUCCESS, 18);

    /* Changed first byte as uppercase. */
    specified_unicode_file_name_1[0] = 'A';
    length = fx_unicode_length_get(specified_unicode_file_name_1);
    status = fx_unicode_file_create(&ram_disk, specified_unicode_file_name_1, length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_ALREADY_CREATED, 19);

    /* Reuse our disk in fx_ram_driver_test.c to create a set of corrupt dir_entries include a normal dir_entry associated with a specified one whose first byte is 0. */
    status = fx_media_close(&ram_disk);
    _fx_ram_driver_copy_default_format = 1;

    /* Modify the first byte at Line 1738 in fx_ram_driver_test.c. */
    temp = large_file_name_format[(1738 - 852) * 16];
    large_file_name_format[(1738 - 852) * 16] = 0x00;

    /* Modify the first dir_entry, which used to be the first short name dir_entry in the directory, to be the long name of "NT-FILE~.TXT". */
    for (count = 0; count < 32; count++)
    {
        temp_array[count] = large_file_name_format[(1732 - 852) * 16 + count];
        large_file_name_format[(1732 - 852) * 16 + count] = large_file_name_format[(1736 - 852) * 16 + count];
    }
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_value_if_fail(status == FX_SUCCESS, 20);

    /* Revert all we done. */
    _fx_ram_driver_copy_default_format = 0;
    large_file_name_format[(1738 - 852) * 16] = temp;
    for (count = 0; count < 32; count++)
    {
        large_file_name_format[(1732 - 852) * 16 + count] = temp_array[count];
    }

    /* Access the dir_entry we just modified by fx_unicode_name_get. */
    status = fx_unicode_name_get(&ram_disk, "NT-FILE~.TXTa", destination_name_1, &ulength);
    return_value_if_fail(status == FX_NOT_FOUND, 21);
    status = fx_unicode_name_get(&ram_disk, "NT-FILE~.TXT", destination_name_1, &ulength);
    return_value_if_fail(status == FX_SUCCESS, 31);
    status = fx_unicode_name_get(&ram_disk, "NT-FILE~.TX", destination_name_1, &ulength);
    return_value_if_fail(status == FX_NOT_FOUND, 32);

    /* Show special performance in our implement. Althought we assign the first byte of some short name dir_entry as 0, fx_unicode_directory_search consider the long file name return by fx_unicode_directory_entry_read as short name so that we will succeed. */
    status = fx_unicode_name_get(&ram_disk, "aBcD~", destination_name_1, &ulength);
    return_value_if_fail(status == FX_SUCCESS, 33);

#ifndef FX_DISABLE_ERROR_CHECKING
    /* Test unicode name checking. */
    length = fx_unicode_length_get(specified_unicode_file_name_2);
    status = fx_unicode_file_create(&ram_disk, specified_unicode_file_name_2, length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_SUCCESS, 21);
    length++;
    status = fx_unicode_file_create(&ram_disk, specified_unicode_file_name_2, length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_INVALID_NAME, 22);

    length = fx_unicode_length_get(specified_unicode_file_name_2);
    status = fx_unicode_directory_create(&ram_disk, specified_unicode_file_name_2, length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_ALREADY_CREATED, 23);
    length++;
    status = fx_unicode_file_create(&ram_disk, specified_unicode_file_name_2, length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_INVALID_NAME, 24);

    length = fx_unicode_length_get(specified_unicode_file_name_2);
    status = fx_unicode_short_name_get(&ram_disk, specified_unicode_file_name_2, length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_SUCCESS, 25);
    length++;
    status = fx_unicode_short_name_get(&ram_disk, specified_unicode_file_name_2, length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_INVALID_NAME, 26);

    status = fx_unicode_short_name_get_extended(&ram_disk, specified_unicode_file_name_2, length, (CHAR *)destination_name, sizeof(destination_name));
    return_value_if_fail(status == FX_INVALID_NAME, 26);

    /* Test invalid old file name */
    old_length = fx_unicode_length_get(specified_unicode_file_name_2);
    old_length++;
    new_length = fx_unicode_length_get(specified_unicode_file_name_1);
    status = fx_unicode_file_rename(&ram_disk, specified_unicode_file_name_2, old_length, specified_unicode_file_name_1, new_length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_INVALID_NAME, 27);
    status = fx_unicode_directory_rename(&ram_disk, specified_unicode_file_name_2, old_length, specified_unicode_file_name_1, new_length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_INVALID_NAME, 28);

    /* Test invalid new file name */
    old_length = fx_unicode_length_get(specified_unicode_file_name_2);
    new_length = fx_unicode_length_get(specified_unicode_file_name_1);
    new_length++;
    status = fx_unicode_file_rename(&ram_disk, specified_unicode_file_name_2, old_length, specified_unicode_file_name_1, new_length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_INVALID_NAME, 29);
    status = fx_unicode_directory_rename(&ram_disk, specified_unicode_file_name_2, old_length, specified_unicode_file_name_1, new_length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_INVALID_NAME, 30);

    old_length = fx_unicode_length_get(long_unicode_name1);
    new_length = fx_unicode_length_get(specified_unicode_file_name_2);
    status = fx_unicode_file_rename(&ram_disk, long_unicode_name1, old_length, specified_unicode_file_name_2, new_length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_NOT_FOUND, 34);

    old_length = fx_unicode_length_get(long_unicode_name1);
    new_length = fx_unicode_length_get(specified_unicode_file_name_2);
    status = fx_unicode_directory_rename(&ram_disk, long_unicode_name1, old_length, specified_unicode_file_name_2, new_length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_NOT_FOUND, 35);

#endif

    status = fx_media_close(&ram_disk);
    return_value_if_fail(status == FX_SUCCESS, 36);

    /* Open the ram_disk in fx_ram_dirver_test.c. */
    temp_array[0] = large_file_name_format[(1736 - 852) * 16 + 3];
    temp_array[1] = large_file_name_format[(1736 - 852) * 16 + 4];

    /* Insert 00FF in a long file dir_entry. */
    large_file_name_format[(1736 - 852) * 16 + 3] = 0;
    large_file_name_format[(1736 - 852) * 16 + 4] = 0xFF;
    _fx_ram_driver_copy_default_format = 1;
    status = fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_value_if_fail(status == FX_SUCCESS, 37);
    _fx_ram_driver_copy_default_format = 0;

    /* Revert all we done. */
    large_file_name_format[(1736 - 852) * 16 + 3] = temp_array[0];
    large_file_name_format[(1736 - 852) * 16 + 4] = temp_array[1];

    /* Attempt to access our directory. */
    length = fx_unicode_length_get(specified_ascii_file_name);
    status = fx_unicode_file_create(&ram_disk, specified_ascii_file_name, length, (CHAR *)destination_name);
    return_value_if_fail(status == FX_SUCCESS, 38);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format the media.  This needs to be done before opening it!  */
    status = fx_media_format(&ram_disk,
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
    return_if_fail( status == FX_SUCCESS);

    /* Backup the data which will be modified later. */
    temp_array[0] = large_file_name_format[(1734 - 852) * 16];
    temp_array[1] = large_file_name_format[(1734 - 852) * 16 + 11];

    /* Modfy the first dir_entry's ordinary number as 0x40 to make number of lfns be zero. */
    large_file_name_format[(1734 - 852) * 16] = 0x40;
    large_file_name_format[(1734 - 852) * 16 + 11] = (UCHAR)FX_LONG_NAME;
    _fx_ram_driver_copy_default_format = 1;
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    _fx_ram_driver_copy_default_format = 0;

    /* Recover the disk. */
    large_file_name_format[(1734 - 852) * 16] = temp_array[0];
    large_file_name_format[(1734 - 852) * 16 + 11] = temp_array[1];

    /* Access root directory. */
    status = fx_unicode_file_create(&ram_disk, specified_ascii_file_name, length, (CHAR *)destination_name);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format the media.  This needs to be done before opening it!  */
    status = fx_media_format(&ram_disk,
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
    return_if_fail( status == FX_SUCCESS);

    /* Backup the data which will be modified later. */
    temp_array[0] = large_file_name_format[(1734 - 852) * 16];
    temp_array[1] = large_file_name_format[(1734 - 852) * 16 + 11];
    temp_array[2] = large_file_name_format[(1736 - 852) * 16];
    temp_array[3] = large_file_name_format[(1736 - 852) * 16 + 11];

    /* Modfy the second dir_entry's ordinary number as 0x40 to make card number be zero. */
    large_file_name_format[(1734 - 852) * 16] = 0x42;
    large_file_name_format[(1734 - 852) * 16 + 11] = (UCHAR)FX_LONG_NAME;
    large_file_name_format[(1736 - 852) * 16] = 0x40;
    large_file_name_format[(1736 - 852) * 16 + 11] = (UCHAR)FX_LONG_NAME;

    /* Open the disk in fx_ram_driver_test.c */
    _fx_ram_driver_copy_default_format = 1;
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);
    _fx_ram_driver_copy_default_format = 0;

    /* Recover the disk. */
    large_file_name_format[(1734 - 852) * 16] = temp_array[0];
    large_file_name_format[(1734 - 852) * 16 + 11] = temp_array[1];
    large_file_name_format[(1736 - 852) * 16] = temp_array[2];
    large_file_name_format[(1736 - 852) * 16 + 11] = temp_array[3];

    /* Access root directory. */
    status = fx_unicode_file_create(&ram_disk, specified_ascii_file_name, length, (CHAR *)destination_name);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format the media.  This needs to be done before opening it!  */
    status = fx_media_format(&ram_disk,
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
    return_if_fail( status == FX_SUCCESS);

    /* Backup the data which will be modified later. */
    temp_array[0] = large_file_name_format[(1736 - 852) * 16];
    temp_array[1] = large_file_name_format[(1736 - 852) * 16 + 11];
    temp_array[2] = large_file_name_format[(1738 - 852) * 16];
    temp_array[3] = large_file_name_format[(1738 - 852) * 16 + 11];

    /* Modfy the last two dir_entries in root directory, so we have to access the second sector of root directory. */
    large_file_name_format[(1736 - 852) * 16] = 0x43;
    large_file_name_format[(1736 - 852) * 16 + 11] = (UCHAR)FX_LONG_NAME;
    large_file_name_format[(1738 - 852) * 16] = 0x42;
    large_file_name_format[(1738 - 852) * 16 + 11] = (UCHAR)FX_LONG_NAME;

    /* Open the disk in fx_ram_driver_test.c */
    _fx_ram_driver_copy_default_format = 1;
    status += fx_media_open( &ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);
    _fx_ram_driver_copy_default_format = 0;

    /* Recover the disk. */
    large_file_name_format[(1734 - 852) * 16] = temp_array[0];
    large_file_name_format[(1734 - 852) * 16 + 11] = temp_array[1];
    large_file_name_format[(1736 - 852) * 16] = temp_array[2];
    large_file_name_format[(1736 - 852) * 16 + 11] = temp_array[3];

    /* Access root directory, but set the size of root directory as zero to make a mistake. */
    ram_disk.fx_media_root_sectors = 0;
    status = fx_unicode_file_create(&ram_disk, specified_ascii_file_name, length, (CHAR *)destination_name);
    return_if_fail( status == FX_FILE_CORRUPT);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_format(&ram_disk,
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
    status += fx_media_open( &ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    status = fx_file_create( &ram_disk, "src");
    status += fx_file_open( &ram_disk, &my_file, "src", FX_OPEN_FOR_WRITE);
    status += fx_file_write( &my_file, "hello", 5);
    return_if_fail( status == FX_SUCCESS);

    /* IO error in fx_utility_FAT_map_flush.c while calling fx_utility_logical_sector_read. */
    _fx_utility_logical_sector_read_error_request = 2;
    status = fx_media_close( &ram_disk);

    printf("SUCCESS!\n");
    test_control_return(0);
}
