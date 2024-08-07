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

void    filex_unicode_fat_entry_3_test_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);

/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_unicode_fat_entry_3_test_application_define(void *first_unused_memory)
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
UCHAR       destination_name1[20] = {0};
UCHAR       destination_name2[20] = {0};
UCHAR       destination_name3[20] = {0};
UCHAR       destination_name4[20] = {0};
UCHAR       destination_name5[20] = {0};
UCHAR       destination_name6[20] = {0};
UCHAR       destination_name7[20] = {0};
UCHAR       destination_name8[20] = {0};
UCHAR       long_unicode_name[] =   {2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 15, 0, 0, 0};
ULONG       length = 14;
UCHAR       fat_buffer[128] = {0};
FX_LOCAL_PATH   local_path;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Unicode fat entry 3 test...............................");

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
                            7000,                   // Total sectors - FAT16
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Create a directory and many subdirectories to build fat chain. */
    status = fx_unicode_directory_create(&ram_disk, long_unicode_name, length, (CHAR *)destination_name);
#ifndef FX_STANDALONE_ENABLE
    status =   fx_directory_local_path_set(&ram_disk, &local_path, (CHAR *)destination_name);
#else
    status =   fx_directory_default_set(&ram_disk, (CHAR *)destination_name);
#endif

    long_unicode_name[0]++;
    status += fx_unicode_directory_create(&ram_disk, long_unicode_name, length, (CHAR *)destination_name1);
    long_unicode_name[0]++;
    status += fx_unicode_directory_create(&ram_disk, long_unicode_name, length, (CHAR *)destination_name2);
    long_unicode_name[0]++;
    status += fx_unicode_directory_create(&ram_disk, long_unicode_name, length, (CHAR *)destination_name3);
    long_unicode_name[0]++;
    status += fx_unicode_directory_create(&ram_disk, long_unicode_name, length, (CHAR *)destination_name4);
    long_unicode_name[0]++;
    status += fx_unicode_directory_create(&ram_disk, long_unicode_name, length, (CHAR *)destination_name5);
    long_unicode_name[0]++;
    status += fx_unicode_directory_create(&ram_disk, long_unicode_name, length, (CHAR *)destination_name6);
    return_if_fail( status == FX_SUCCESS);

    /* Flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (int i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) fat_buffer);  

    /* Add a FAT entry randomly in the FAT table.  */
    fat_buffer[6] =  0x32;
    fat_buffer[7] =  0x00;

    /* Write the FAT corruption out.  */
    status += fx_media_write(&ram_disk, 1, (VOID *) fat_buffer);
    return_if_fail( status == FX_SUCCESS);

    long_unicode_name[0]++;
    status = fx_unicode_directory_create(&ram_disk, long_unicode_name, length, (CHAR *)destination_name7);
    return_if_fail( status == FX_FAT_READ_ERROR);

    /* Flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (int i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) fat_buffer);  

    /* Add a FAT entry randomly in the FAT table.  */
    fat_buffer[6] =  0x03;
    fat_buffer[7] =  0x00;

    /* Write the FAT corruption out.  */
    status += fx_media_write(&ram_disk, 1, (VOID *) fat_buffer);
    status += fx_unicode_directory_create(&ram_disk, long_unicode_name, length, (CHAR *)destination_name7);
    return_if_fail( status == FX_FAT_READ_ERROR);

    /* Flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (int i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) fat_buffer);  

    /* Recovery FAT chain.  */
    fat_buffer[6] =  0x06;
    fat_buffer[7] =  0x00;

    /* Write the FAT corruption out.  */
    status += fx_media_write(&ram_disk, 1, (VOID *) fat_buffer);
    status += fx_unicode_directory_create(&ram_disk, long_unicode_name, length, (CHAR *)destination_name7);
    return_if_fail( status == FX_SUCCESS);

    /* Flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (int i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) fat_buffer);  

    /* Make FAT chain a circle.  */
    fat_buffer[12] =  0x03;
    fat_buffer[13] =  0x00;

    /* Write the FAT corruption out.  */
    status += fx_media_write(&ram_disk, 1, (VOID *) fat_buffer);
    status += fx_unicode_directory_create(&ram_disk, long_unicode_name, length, (CHAR *)destination_name8);
    return_if_fail( status == FX_FAT_READ_ERROR);

    printf("SUCCESS!\n");
    test_control_return(0);
}
