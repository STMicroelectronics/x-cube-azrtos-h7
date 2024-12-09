/* This FileX test concentrates on the file attributes read/set operation.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   <stdio.h> 
#include   "fx_ram_driver_test.h"

#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              128*128


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD               ftest_0;
#endif
static FX_MEDIA                ram_disk;
static FX_FILE                 my_file;
static FX_FILE                 my_file1;
static FX_FILE                 my_file2;
static FX_FILE                 my_file3;
static FX_FILE                 my_file4;
static FX_FILE                 my_file5;
static FX_FILE                 my_file6;
static FX_FILE                 my_file7;
static FX_FILE                 my_file8;
static FX_FILE                 my_file9;


/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *ram_disk_memory;
static UCHAR                  *cache_buffer;
#else
static UCHAR                   cache_buffer[CACHE_SIZE];
#endif


/* Define thread prototypes.  */

void    filex_file_attributes_read_set_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_file_attributes_read_set_application_define(void *first_unused_memory)
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
FX_FILE     open_file;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   File attributes read/set test..........................");

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
        test_control_return(1);
    }
    
    /* Attempt to read file attributes before the media has been opened */
    status = fx_file_attributes_read(&ram_disk, "TEST.TXT", &attributes);
    if (status != FX_MEDIA_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(2);
    }
    
    /* Attempt to set file attributes before the media has been opened */
    status = fx_file_attributes_set(&ram_disk, "TEST.TXT", 0);
    if (status != FX_MEDIA_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(3);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(4);
    }
    
    /* Attempt to set file attributes while the media is write protected */
    ram_disk.fx_media_driver_write_protect = FX_TRUE;
    status = fx_file_attributes_set(&ram_disk, "TEST.TXT", FX_READ_ONLY);
    if (status != FX_WRITE_PROTECT)
    {
        printf("ERROR!\n");
        test_control_return(5);
    }
    ram_disk.fx_media_driver_write_protect = FX_FALSE;
    
    /* attempt to read file attributes from a file that does not exist */
    status = fx_file_attributes_read(&ram_disk, "TEST.TXT", &attributes);
    if (status == FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(6);
    }
    
    /* Attempt to set file attributes before the media has been opened */
    status = fx_file_attributes_set(&ram_disk, "TEST.TXT", attributes);
    if (status == FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(7);
    }
    
    /* attempt to read file attributes from something that is not a file */
    status  = fx_directory_create(&ram_disk, "NOT_A_FILE");
    status += fx_file_attributes_read(&ram_disk, "NOT_A_FILE", &attributes);
    if (status != FX_NOT_A_FILE)
    {
        printf("ERROR!\n");
        test_control_return(8);
    }
    
    /* Attempt to set file attributes before the media has been opened */
    status = fx_file_attributes_set(&ram_disk, "NOT_A_FILE", FX_READ_ONLY);
    if (status != FX_NOT_A_FILE)
    {
        printf("ERROR!\n");
        test_control_return(9);
    }

    /* Attempt to set file attributes with invlaid file name.  */
    status = fx_file_attributes_set(&ram_disk, "NOT_EXIST", FX_READ_ONLY);
    if (status != FX_NOT_FOUND)
    {
        printf("ERROR!\n");
        test_control_return(9);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(10);
    }
    
#ifndef FX_DISABLE_ERROR_CHECKING
    
    /* send null pointer to generate an error */
    status = fx_file_attributes_set(FX_NULL, "filename", FX_READ_ONLY);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(11);
    }
    
    /* send null pointer to generate an error */
    status = fx_file_attributes_set(&ram_disk, "filename", 0x40);
    if (status != FX_INVALID_ATTR)
    {
        printf("ERROR!\n");
        test_control_return(12);
    }
    
    /* send null pointer to generate an error */
    status = fx_file_attributes_read(FX_NULL, "filename", FX_NULL);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(13);
    }
    
#endif

    /* Invalidate the media cache.  */
    fx_media_cache_invalidate(&ram_disk);

    /* Pickup the attributes of the file.  */
    status =  fx_file_attributes_read(&ram_disk, "TEST.TXT", &attributes);

    /* Check the attributes read status.  */
    if ((status != FX_SUCCESS) || (attributes != FX_ARCHIVE))
    {

        printf("ERROR!\n");
        test_control_return(14);
    }

    /* Invalidate the media cache.  */
    fx_media_cache_invalidate(&ram_disk);

    /* Now write the attributes out for the file.  */
    status =  fx_file_attributes_set(&ram_disk, "TEST.TXT", attributes | FX_READ_ONLY);

    /* Check the attributes set status.  */
    if (status != FX_SUCCESS) 
    {

        printf("ERROR!\n");
        test_control_return(15);
    }
    
    /* Write the attributes out to the file while another file is opened to get better code coverage */
    status  = fx_file_create(&ram_disk, "open_file.txt");
    status += fx_file_open(&ram_disk, &open_file, "open_file.txt", FX_OPEN_FOR_WRITE);
    status += fx_file_attributes_set(&ram_disk, "TEST.TXT", attributes | FX_READ_ONLY);
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(16);
    }

    /* Invalidate the media cache.  */
    fx_media_cache_invalidate(&ram_disk);

    /* Pickup the attributes of the file again.  */
    status =  fx_file_attributes_read(&ram_disk, "TEST.TXT", &attributes);

    /* Check the attributes read status.  */
    if ((status != FX_SUCCESS) || (attributes != (FX_ARCHIVE | FX_READ_ONLY)))
    {

        printf("ERROR!\n");
        test_control_return(17);
    }

    /* Open the test file... this should fail since we set the attributes to read only!  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_ACCESS_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(18);
    }
    
    /* Attempt to set file attributes for a file that is open */
    status  = fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_READ);
    status += fx_file_attributes_set(&ram_disk, "TEST.TXT", attributes);
    if (status != FX_ACCESS_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(19);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);   

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(20);
    }

    /* Now test the attributes set with multiple files open that have directory entries that reside on 
       different logical sectors.  */

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
        test_control_return(21);
    }
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(22);
    }

    /* Create a set of test files.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_create(&ram_disk, "TEST1.TXT");
    status += fx_file_create(&ram_disk, "TEST2.TXT");
    status += fx_file_create(&ram_disk, "TEST3.TXT");
    status += fx_file_create(&ram_disk, "TEST4.TXT");
    status += fx_file_create(&ram_disk, "TEST5.TXT");
    status += fx_file_create(&ram_disk, "TEST6.TXT");
    status += fx_file_create(&ram_disk, "TEST7.TXT");
    status += fx_file_create(&ram_disk, "TEST8.TXT");
    status += fx_file_create(&ram_disk, "TEST9.TXT");
    status += fx_file_create(&ram_disk, "TEST10.TXT");

    /* Now open all the files except the TEST.TXT.  */
    status += fx_file_open(&ram_disk, &my_file, "TEST1.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &my_file1, "TEST2.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &my_file2, "TEST3.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &my_file3, "TEST4.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &my_file4, "TEST5.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &my_file5, "TEST6.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &my_file6, "TEST7.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &my_file7, "TEST8.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &my_file8, "TEST9.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &my_file9, "TEST10.TXT", FX_OPEN_FOR_WRITE);
        
    /* Now set the attributes for TEST.TXT.  */
    status += fx_file_attributes_set(&ram_disk, "TEST.TXT", FX_READ_ONLY);
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(23);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);   

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(24);
    }
    else
    {

        printf("SUCCESS!\n");
        test_control_return(0);
    }
}

