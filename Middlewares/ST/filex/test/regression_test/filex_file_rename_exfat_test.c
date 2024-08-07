/* This FileX test concentrates on the file rename operations.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"

#ifdef FX_ENABLE_EXFAT

#define     DEMO_STACK_SIZE         8192
#define     CACHE_SIZE              FX_EXFAT_SECTOR_SIZE


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

void    filex_file_rename_exfat_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_file_rename_exfat_application_define(void *first_unused_memory)
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

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   exFAT File rename test.................................");

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_exFAT_format(&ram_disk,
                                            _fx_ram_driver,             // Driver entry
                                            ram_disk_memory,           // RAM disk memory pointer
                                            cache_buffer,               // Media buffer pointer
                                            CACHE_SIZE,                 // Media buffer size
                                            "MY_RAM_DISK",              // Volume Name
                                            1,                          // Number of FATs
                                            0,                          // Hidden sectors
                                            256,                        // Total sectors
                                            FX_EXFAT_SECTOR_SIZE,       // Sector size.
                                            4,                          // exFAT Sectors per cluster
                                            12345,                      // Volume ID
                                            0);                         // Boundary unit

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(2);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(3);
    }

    /* Simple 8.3 rename in the root directory.  */
    status =  fx_file_create(&ram_disk, "MYTEST");
    status += fx_file_rename(&ram_disk, "MYTEST", "OURTEST");
    
    /* Check the file rename status.  */
    if (status != FX_SUCCESS)
    {

        /* Error renaming file.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(4);
    }

    /* Now create a 2 sub-directories.  */
    status += fx_directory_create(&ram_disk, "/subdir1");
    status += fx_directory_create(&ram_disk, "/subdir2");
    status += fx_file_rename(&ram_disk, "OURTEST", "/subdir1/NEWOURTEST");
    status += fx_file_rename(&ram_disk, "/subdir1/NEWOURTEST", "/subdir2/OURTEST");
    status += fx_file_rename(&ram_disk, "/subdir2/OURTEST", "OURTEST");
    status += fx_file_delete(&ram_disk, "OURTEST");    
    
    /* Check the file rename status.  */
    if (status != FX_SUCCESS)
    {

        /* Error renaming/moving file.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(5);
    }
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS) 
    {

        printf("ERROR!\n");
        test_control_return(6);
    }
    else
    {

        printf("SUCCESS!\n");
        test_control_return(0);
    }
}

#endif /* FX_ENABLE_EXFAT */
