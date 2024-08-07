/* This FileX test concentrates on the file create/delete operations.  */

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
static FX_FILE                  file_1;
static FX_FILE                  file_2;
static FX_FILE                  file_3;
static FX_FILE                  file_4;



/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                    *ram_disk_memory;
static UCHAR                    *cache_buffer;
#else
static UCHAR                     cache_buffer[CACHE_SIZE];
#endif


/* Define thread prototypes.  */

void    filex_file_create_delete_exfat_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_file_create_delete_exfat_application_define(void *first_unused_memory)
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
UCHAR       buffer[30];
ULONG       actual;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   exFAT File create/delete test..........................");

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
    status =  fx_directory_create(&ram_disk, "subdir");
    status += fx_file_create(&ram_disk, "rootname");
    status += fx_file_create(&ram_disk, "rootname1");
    status += fx_file_create(&ram_disk, "/subdir/rootname");
    status += fx_file_create(&ram_disk, "/subdir/rootname1");

    /* Check the file create status.  */
    if (status != FX_SUCCESS)
    {

        /* Error creating file.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(4);
    }

    /* Attempt to create the same file again. This should cause an error!  */
    status =  fx_file_create(&ram_disk, "rootname");
    
    /* Check the file create status.  */
    if (status != FX_ALREADY_CREATED)
    {

        /* Error attempting to create the same file.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(5);
    }
    
    /* Open all the files.  */
    status =   fx_file_open(&ram_disk, &file_1, "rootname", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_2, "rootname1", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_3, "/subdir/rootname", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_4, "/subdir/rootname1", FX_OPEN_FOR_WRITE);
    
    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        /* Error opening files.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(6);
    }

    /* Now write a buffer to each file.   */
    status +=  fx_file_write(&file_1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status +=  fx_file_write(&file_2, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status +=  fx_file_write(&file_3, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status +=  fx_file_write(&file_4, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);

    /* Check the file write status.  */
    if (status != FX_SUCCESS)
    {

        /* Error writing files.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(7);
    }

    /* Seek to the beginning of each file.  */
    status +=  fx_file_seek(&file_1, 0);
    status +=  fx_file_seek(&file_2, 0);
    status +=  fx_file_seek(&file_3, 0);
    status +=  fx_file_seek(&file_4, 0);

    /* Check the seek status.  */
    if (status != FX_SUCCESS)
    {

        /* Error seeking in files.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(8);
    }

    /* Now read the buffer.  */
    status =  fx_file_read(&file_1, buffer, 30, &actual);
    
    /* Check for error.  */
    if ((status) || (actual != 26))
    {
    
        /* Error reading file.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(9);
    }

    /* Now read the buffer.  */
    status =  fx_file_read(&file_2, buffer, 30, &actual);
    
    /* Check for error.  */
    if ((status) || (actual != 26))
    {
    
        /* Error reading file.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(10);
    }

    /* Now read the buffer.  */
    status =  fx_file_read(&file_3, buffer, 30, &actual);
    
    /* Check for error.  */
    if ((status) || (actual != 26))
    {
    
        /* Error reading file.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(11);
    }

    /* Now read the buffer.  */
    status =  fx_file_read(&file_4, buffer, 30, &actual);
    
    /* Check for error.  */
    if ((status) || (actual != 26))
    {
    
        /* Error reading file.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(12);
    }

    /* Close all files.  */
    status +=  fx_file_close(&file_1);
    status +=  fx_file_close(&file_2);
    status +=  fx_file_close(&file_3);
    status +=  fx_file_close(&file_4);

    /* Check for error.  */
    if (status)
    {
    
        /* Error closing files.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(13);
    }

    /* Delete all files.  */
    status += fx_file_delete(&ram_disk, "rootname");
    status += fx_file_delete(&ram_disk, "rootname1");
    status += fx_file_delete(&ram_disk, "/subdir/rootname");
    status += fx_file_delete(&ram_disk, "/subdir/rootname1");
    status += fx_directory_delete(&ram_disk, "/subdir");

    /* Check for error.  */
    if (status)
    {
    
        /* Error deleting files.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(14);
    }

    /* Attempt to delete an already deleted file.  */
    status += fx_file_delete(&ram_disk, "rootname");

    /* Check for error.  */
    if (status != FX_NOT_FOUND)
    {
    
        /* Error deleting files.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(15);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS) 
    {

        printf("ERROR!\n");
        test_control_return(16);
    }
    else
    {

        printf("SUCCESS!\n");
        test_control_return(0);
    }
}

#endif /* FX_ENABLE_EXFAT */
