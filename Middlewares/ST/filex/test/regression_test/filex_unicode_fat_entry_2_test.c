/* This test is determined to cover lines in fx_unicode_directory_entry_read.c.                 */
/* We need a dir_entry whose first byte, ordinal number byte, is big enoutgh to exceed the the  */
/* limit of FX_MAX_LONG_NAME_LEN, so we created a disk with a corrupt dir_entry.                */
#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   <stdio.h>
#include   <string.h>
#include   "fx_ram_driver_test.h"

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

void    filex_unicode_fat_entry_2_test_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);

/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_unicode_fat_entry_2_test_application_define(void *first_unused_memory)
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
UCHAR       destination_name2[100] = {0};
UCHAR       long_unicode_name[] =   {2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 15, 0, 0, 0};
ULONG       length = 14;
FX_LOCAL_PATH   local_path;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Unicode fat entry 2 test...............................");

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
    status += fx_unicode_directory_create(&ram_disk,  long_unicode_name, length, (CHAR *)destination_name);
    /* Chdir to the created directory and create a subdirectory. */
#ifndef FX_STANDALONE_ENABLE
    status =   fx_directory_local_path_set(&ram_disk, &local_path, (CHAR *)destination_name);
#else
    status =   fx_directory_default_set(&ram_disk, (CHAR *)destination_name);
#endif

    status += fx_unicode_directory_create(&ram_disk,  long_unicode_name, length, (CHAR *)destination_name2);

    /* Fat entry destory. */
    ram_disk.fx_media_fat_cache[8].fx_fat_cache_entry_value = 0x1ff;
    ram_disk.fx_media_fat_cache[8].fx_fat_cache_entry_dirty = 1;

    /* Close the media to flush buffer. Now we have a disk with a corrupt dir_entry. */
    status += fx_media_close(&ram_disk);
        
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    if (status != FX_SUCCESS) 
    {
        printf("ERROR!\n");
        test_control_return(19);
    }

#ifndef FX_STANDALONE_ENABLE
    status =   fx_directory_local_path_set(&ram_disk, &local_path, (CHAR *)destination_name);
#else
    status =   fx_directory_default_set(&ram_disk, (CHAR *)destination_name);
#endif
    status += fx_unicode_directory_create(&ram_disk,  long_unicode_name, length, (CHAR *)destination_name);

    if (status == FX_SUCCESS) 
    {
        printf("ERROR!\n");
        test_control_return(19);
    }
    else
    {
        printf("SUCCESS!\n");
        test_control_return(0);
    }
}
