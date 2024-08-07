/* This FileX test concentrates on the unicode operations.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_ram_driver_test.h"
#include   <stdio.h>

#define     DEMO_STACK_SIZE         8192
#define     CACHE_SIZE              16*128


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD                ftest_0;
extern TX_THREAD *_tx_thread_current_ptr;
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

void    filex_unicode_name_string_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);

/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_unicode_name_string_application_define(void *first_unused_memory)
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
UCHAR       buffer[512];
#ifndef FX_DISABLE_ERROR_CHECKING
UCHAR       destination_name[300] = {0};
UCHAR       long_unicode_name[] =   {2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 15, 0, 0, 0};
#endif /* FX_DISABLE_ERROR_CHECKING */

#ifndef FX_STANDALONE_ENABLE
UCHAR       *path_ptr;
#else
UCHAR       path_ptr[1024];
#endif
FX_LOCAL_PATH local_path;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Unicode name string test...............................");

    /* Format the media.  This needs to be done before opening it!  */
    status = fx_media_format(&ram_disk, 
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
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

#ifndef FX_DISABLE_ERROR_CHECKING

    /* Disable write protect */
    ram_disk.fx_media_driver_write_protect = FX_FALSE;

    /* lengthen the unicode name to include the last 0 */ 
    ULONG       length = 15;

    /* Test creating directory. */
    status = fx_unicode_directory_create(&ram_disk,  long_unicode_name, length, (CHAR *) destination_name);
    return_if_fail( status == FX_INVALID_NAME);

    /* Test creating file. */
    status = fx_unicode_file_create(&ram_disk,  long_unicode_name, length, (CHAR *) destination_name);
    return_if_fail( status == FX_INVALID_NAME);

#endif

    /* Create a directory named a...(254 times). */
    buffer[0] = '/';

    for (UINT i = 1; i < 300; i++)
        buffer[i] = 'a';

    buffer[255] = 0;

    /* Chenged to the specified directory and ensure the string related to current position is end as zero. */
    status = fx_directory_create( &ram_disk, (CHAR *)buffer);
#ifndef FX_STANDALONE_ENABLE
        status =   fx_directory_local_path_set(&ram_disk, &local_path, (CHAR *)buffer);
#else
        status =   fx_directory_default_set(&ram_disk, (CHAR *)buffer);
#endif
#ifndef FX_STANDALONE_ENABLE
    path_ptr =  (UCHAR *)((FX_PATH *)_tx_thread_current_ptr -> tx_thread_filex_ptr) -> fx_path_string;
#endif
    path_ptr[255] = 0;
    return_if_fail( status == FX_SUCCESS);

    /* Specify last found name as the directory we just created. */
    buffer[255] = '/';
    buffer[256] = 0;
    
    for (UINT i = 0; i <= 256; i++)
        ram_disk.fx_media_last_found_name[i] = (CHAR)buffer[i];

    /* Access the directory and see what will happen. */
    status = fx_directory_create( &ram_disk, "a");
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    printf("SUCCESS!\n");
    test_control_return(0);
}
