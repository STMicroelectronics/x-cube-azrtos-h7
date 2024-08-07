/* This FileX test concentrates on the basic media format, open, close operation.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"

#ifdef FX_ENABLE_EXFAT

#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              FX_EXFAT_SECTOR_SIZE


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD               ftest_0;
#endif
static FX_MEDIA                ram_disk;
static FX_FILE                 my_file;


/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *ram_disk_memory;
static UCHAR                  *cache_buffer;
#else
static UCHAR                   cache_buffer[CACHE_SIZE];
#endif


/* Define thread prototypes.  */

void    filex_media_close_with_file_opened_exfat_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_media_close_with_file_opened_exfat_application_define(void *first_unused_memory)
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
ULONG       actual;
UCHAR       local_buffer[32];

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   exFAT Media close with file opened test................");
    
/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send null pointer to generate an error */
    status = fx_media_exFAT_format(FX_NULL, _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE, "MY_RAM_DISK", 1, 0, 256, 512, 1, 12345, 0);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(11);
    }
#endif /* FX_DISABLE_ERROR_CHECKING */

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_exFAT_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            0,                      // Hidden sectors
                            256,                    // Total sectors 
                            FX_EXFAT_SECTOR_SIZE,   // Sector size   
                            1,                      // exFAT Sectors per cluster
                            12345,                  // Volume ID
                            0);                     // Boundary unit

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(2);
    }
    
/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send null pointer to generate an error */
    status = fx_media_open(FX_NULL, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(11);
    }
#endif /* FX_DISABLE_ERROR_CHECKING */

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(3);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

        /* Check for an already created status.  This is not fatal, just 
           let the user know.  */
        if (status != FX_ALREADY_CREATED)
        {

            printf("ERROR!\n");
            test_control_return(3);
        }
    }

    /* Open the test file.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(4);
    }

    /* Seek to the beginning of the test file.  */
    status =  fx_file_seek(&my_file, 0);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(5);
    }

    /* Write a string to the test file.  */
    status =  fx_file_write(&my_file, " ABCDEFGHIJKLMNOPQRSTUVWXYZ\n", 28);

    /* Check the file write status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(6);
    }

    /* Seek to the beginning of the test file.  */
    status =  fx_file_seek(&my_file, 0);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(7);
    }

    /* Close the media without closing the file.  */
    status =  fx_media_close(&ram_disk);

    /* Re-Open the ram_disk.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(3);
    }

    /* Open the test file.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(4);
    }

    /* Seek to the beginning of the test file.  */
    status =  fx_file_seek(&my_file, 0);

    /* Read the first 28 bytes of the test file.  */
    status =  fx_file_read(&my_file, local_buffer, 28, &actual);

    /* Check the file read status.  */
    if ((status != FX_SUCCESS) || (actual != 28))
    {

        printf("ERROR!\n");
        test_control_return(8);
    }

    /* Close the test file.  */
    status =  fx_file_close(&my_file);

    /* Check the file close status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(9);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(10);
    }
    else
    {

        printf("SUCCESS!\n");
        test_control_return(0);
    }
}

#endif /* FX_ENABLE_EXFAT */
