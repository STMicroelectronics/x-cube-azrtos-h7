/* This FileX test concentrates on the file attributes read/set operation.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include    <stdio.h>
#include   "fx_ram_driver_test.h"

void  test_control_return(UINT status);

#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              128*128


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD               ftest_0;
#endif
static FX_MEDIA                ram_disk;


/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *ram_disk_memory;
static UCHAR                  *cache_buffer;
#else
static UCHAR                  cache_buffer[CACHE_SIZE];
#endif


/* Define thread prototypes.  */

void    filex_directory_attributes_read_set_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_directory_attributes_read_set_application_define(void *first_unused_memory)
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
UINT        attributes;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Directory attributes read/set test.....................");

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
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(2);
    }
    
/* test error checking */
#ifndef FX_DISABLE_ERROR_CHECKING
    
    /* check directory attributes read null pointer error */
    status = fx_directory_attributes_read(FX_NULL, "TEST_DIR", &attributes);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(99);
    }
    
    /* check directory attributes set null pointer error */
    status = fx_directory_attributes_set(FX_NULL, "TEST_DIR", attributes);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(99);
    }
    
    /* check directory attributes set null pointer error */
    status = fx_directory_attributes_set(&ram_disk, "TEST_DIR", 0x1111);
    if (status != FX_INVALID_ATTR)
    {
        printf("ERROR!\n");
        test_control_return(99);
    }
    
#endif
    
    /* check directory attributes read to make sure it throws an error when the media hasnt been opened */
    status = fx_directory_attributes_read(&ram_disk, "TEST_DIR", &attributes);
    if (status != FX_MEDIA_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(99);
    }
    
    /* check directory attributes set to make sure it throws an error when the media hasnt been opened */
    status = fx_directory_attributes_set(&ram_disk, "TEST_DIR", 0x0000);
    if (status != FX_MEDIA_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(99);
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

    /* Create a directory called TEST_DIR in the root directory.  */
    status =  fx_directory_create(&ram_disk, "TEST_DIR");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(4);
    }

    /* Create a file called NOT_A_DIR in the root directory.  */
    status =  fx_file_create(&ram_disk, "NOT_A_DIR");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(4);
    }

    /* Invalidate the media cache.  */
    fx_media_cache_invalidate(&ram_disk);

    /* Pickup the attributes of the directory.  */
    status =  fx_directory_attributes_read(&ram_disk, "TEST_DIR", &attributes);

    /* Check the attributes read status.  */
    if ((status != FX_SUCCESS) || (attributes != FX_DIRECTORY))
    {

        printf("ERROR!\n");
        test_control_return(5);
    }
    
    /* check directory attributes set to make sure it throws an error when the directory is write protected */
    ram_disk.fx_media_driver_write_protect = FX_TRUE;
    status = fx_directory_attributes_set(&ram_disk, "TEST_DIR", attributes);
    if (status == FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(99);
    }
    ram_disk.fx_media_driver_write_protect = FX_FALSE;
    
    /* check directory attributes read to make sure it throws an error when it isnt a directory */
    status = fx_directory_attributes_read(&ram_disk, "NOT_A_DIR", &attributes);
    if (status != FX_NOT_DIRECTORY)
    {
        printf("ERROR!\n");
        test_control_return(99);
    }
    
    /* check directory attributes read to make sure it throws an error when the directory isnt found */
    status = fx_directory_attributes_read(&ram_disk, "DOES_NOT_EXIST", &attributes);
    if (status == FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(99);
    }
    
    /* check directory attributes set to make sure it throws an error when it isnt a directory */
    status = fx_directory_attributes_set(&ram_disk, "NOT_A_DIR", attributes);
    if (status != FX_NOT_DIRECTORY)
    {
        printf("ERROR!\n");
        test_control_return(99);
    }
    
    /* check directory attributes set to make sure it throws an error when the directory isnt found */
    status = fx_directory_attributes_set(&ram_disk, "DOES_NOT_EXIST", attributes);
    if (status == FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(99);
    }

    /* Invalidate the media cache.  */
    fx_media_cache_invalidate(&ram_disk);

    /* Now write the attributes out for the directory.  */
    status =  fx_directory_attributes_set(&ram_disk, "TEST_DIR", attributes | FX_ARCHIVE | FX_SYSTEM | FX_READ_ONLY | FX_HIDDEN);

    /* Check the attributes set status.  */
    if (status != FX_SUCCESS) 
    {

        printf("ERROR!\n");
        test_control_return(6);
    }

    /* Invalidate the media cache.  */
    fx_media_cache_invalidate(&ram_disk);

    /* Pickup the attributes of the directory again.  */
    status =  fx_directory_attributes_read(&ram_disk, "TEST_DIR", &attributes);

    /* Check the attributes read status.  */
    if ((status != FX_SUCCESS) || (attributes != (FX_ARCHIVE | FX_READ_ONLY | FX_DIRECTORY | FX_SYSTEM | FX_HIDDEN)))
    {

        printf("ERROR!\n");
        test_control_return(7);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);   

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(8);
    }
    else
    {

        printf("SUCCESS!\n");
        test_control_return(0);
    }
}

