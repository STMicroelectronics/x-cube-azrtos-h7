/* This FileX test concentrates on the basic media format, open, close operation.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"

#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              16*128


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

void    filex_media_format_open_close_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_media_format_open_close_application_define(void *first_unused_memory)
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
    actual =  FX_MAX_LONG_NAME_LEN;
    if (actual < 256)
        printf("**** ERROR *****  FileX and tests must be built with FX_MAX_LONG_NAME_LEN=256\n");
    actual =  FX_MAX_LAST_NAME_LEN;
    if (actual < 256)
        printf("**** ERROR *****  FileX and tests must be built with FX_MAX_LAST_NAME_LEN=256\n");

    /* Print out some test information banners.  */     
    printf("FileX Test:   Media format, open, and close test.....................");

    /* Format the media with invalid parameters.  */
    status = fx_media_format(&ram_disk,
        _fx_ram_driver,         // Driver entry
        ram_disk_memory,        // RAM disk memory pointer
        cache_buffer,           // Media buffer pointer
        CACHE_SIZE,             // Media buffer size 
        "MY_RAM_DISK",          // Volume Name
        1,                      // Number of FATs
        32,                     // Directory Entries
        0,                      // Hidden sectors
        256,                    // Total sectors 
        4097,                   // Sector size   
        1,                      // Sectors per cluster
        1,                      // Heads
        1);                     // Sectors per track 

    /* Check status.  */
    if (status != FX_SECTOR_INVALID)
    {

        printf("ERROR!\n");
        test_control_return(2);
    }

    /* Format the media with invalid parameters.  */
    status = fx_media_format(&ram_disk,
        _fx_ram_driver,         // Driver entry
        ram_disk_memory,        // RAM disk memory pointer
        cache_buffer,           // Media buffer pointer
        CACHE_SIZE,             // Media buffer size 
        "MY_RAM_DISK",          // Volume Name
        1,                      // Number of FATs
        32,                     // Directory Entries
        0,                      // Hidden sectors
        256,                    // Total sectors 
        0,                      // Sector size   
        1,                      // Sectors per cluster
        1,                      // Heads
        1);                     // Sectors per track 

    /* Check status.  */
    if (status != FX_SECTOR_INVALID)
    {

        printf("ERROR!\n");
        test_control_return(2);
    }

    /* Format the media with invalid parameters.  */
    status = fx_media_format(&ram_disk,
        _fx_ram_driver,         // Driver entry
        ram_disk_memory,        // RAM disk memory pointer
        cache_buffer,           // Media buffer pointer
        CACHE_SIZE,             // Media buffer size 
        "MY_RAM_DISK",          // Volume Name
        1,                      // Number of FATs
        32,                     // Directory Entries
        0,                      // Hidden sectors
        256,                    // Total sectors 
        128,                    // Sector size   
        129,                    // Sectors per cluster
        1,                      // Heads
        1);                     // Sectors per track 

    /* Check status.  */
    if (status != FX_SECTOR_INVALID)
    {

        printf("ERROR!\n");
        test_control_return(2);
    }

    /* Format the media with invalid parameters.  */
    status = fx_media_format(&ram_disk,
        _fx_ram_driver,         // Driver entry
        ram_disk_memory,        // RAM disk memory pointer
        cache_buffer,           // Media buffer pointer
        CACHE_SIZE,             // Media buffer size 
        "MY_RAM_DISK",          // Volume Name
        1,                      // Number of FATs
        32,                     // Directory Entries
        0,                      // Hidden sectors
        256,                    // Total sectors 
        128,                    // Sector size   
        0,                      // Sectors per cluster
        1,                      // Heads
        1);                     // Sectors per track 

    /* Check status.  */
    if (status != FX_SECTOR_INVALID)
    {

        printf("ERROR!\n");
        test_control_return(2);
    }

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
                            256,                    // Total sectors 
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
    
    /* Try to close the media before it has been opened */
    status = fx_media_close(&ram_disk);
    if (status != FX_MEDIA_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(11);
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
    
/* test error checking */
#ifndef FX_DISABLE_ERROR_CHECKING
    
    /* send a null pointer to generate an error */
    status =  fx_media_close(FX_NULL);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(8);
    }
    
    /* send null pointer to generate an error */
    status = fx_media_open(FX_NULL, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(11);
    }
    
    /* try to open an already open media to generate an error */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(11);
    }
    
#endif /* FX_DISABLE_ERROR_CHECKING */

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

    /* Re-Open the ram_disk.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(3);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");

    /* Check for an already created status.  This is not fatal, just 
       let the user know.  */
    if (status != FX_ALREADY_CREATED)
    {

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
                            256,                    // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(11);
    }

    /* Corrupt the bytes per sector field.  */  
    _fx_utility_16_unsigned_write(&ram_disk_memory[FX_BYTES_SECTOR], 0);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_MEDIA_INVALID)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(12);
    }

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
                            256,                    // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(13);
    }

    /* Corrupt the total sectors field.  */  
    _fx_utility_16_unsigned_write(&ram_disk_memory[FX_SECTORS], 0);
    _fx_utility_32_unsigned_write(&ram_disk_memory[FX_HUGE_SECTORS], 0);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_MEDIA_INVALID)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(14);
    }

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
                            256,                    // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(15);
    }

    /* Corrupt the reserved sectors field.  */  
    _fx_utility_16_unsigned_write(&ram_disk_memory[FX_RESERVED_SECTORS], 0);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_MEDIA_INVALID)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(16);
    }

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
                            256,                    // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(17);
    }

    /* Corrupt the sectors per cluster field.  */  
    ram_disk_memory[FX_SECTORS_CLUSTER] = 0;

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_MEDIA_INVALID)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(18);
    }

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
                            256,                    // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(19);
    }

    /* Corrupt the sectors per FAT field.  */  
    _fx_utility_16_unsigned_write(&ram_disk_memory[FX_SECTORS_PER_FAT], 0);
    _fx_utility_32_unsigned_write(&ram_disk_memory[FX_SECTORS_PER_FAT_32], 0);
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_MEDIA_INVALID)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(20);
    }

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
                            256,                    // Total sectors 
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

    /* Corrupt the number of FATs field.  */  
    ram_disk_memory[FX_NUMBER_OF_FATS] = 0;

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_MEDIA_INVALID)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(22);
    }

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
        256,                    // Total sectors 
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

    /* Format the media with an even number of FAT12 sectors.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            256+9,                  // Total sectors - need 256 clusters for even FAT table logic to be tested
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(23);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_media_close(&ram_disk);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(24);
    }

    /* Format the media with an even number of FAT16 sectors.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            6144+9,                 // Total sectors - need 6144 clusters for even FAT table logic to be tested
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(25);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_media_close(&ram_disk);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(26);
    }


    /* Format the media with an even number of FAT12 sectors.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            256+9,                  // Total sectors - need 256 clusters for even FAT table logic to be tested
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(23);
    }

    /* Clear 480 bytes from offset 30. Verify the compatibility.
     * They are reserved for FDC Descriptor. Some of them are defined in Extended FDC Descriptor. */
    memset(&ram_disk_memory[30], 0, 480);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_media_close(&ram_disk);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(24);
    }

    /* Format the media with an even number of FAT16 sectors.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            6144+9,                 // Total sectors - need 6144 clusters for even FAT table logic to be tested
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(25);
    }

    /* Clear 480 bytes from offset 30. Verify the compatibility.
     * They are reserved for FDC Descriptor. Some of them are defined in Extended FDC Descriptor. */
    memset(&ram_disk_memory[30], 0, 480);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_media_close(&ram_disk);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(26);
    }

    /* Format the media with an even number of FAT32 sectors.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            70656+9,                // Total sectors  - FAT32
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(27);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_media_close(&ram_disk);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(28);
    }


    /* Format the media with an even number of FAT32 sectors.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            70656+9,                // Total sectors  - FAT32
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(27);
    }

    /* Corrupt the root cluster field.  */
    ram_disk_memory[FX_ROOT_CLUSTER_32] = 0;
    ram_disk_memory[FX_ROOT_CLUSTER_32 + 1] = 0;
    ram_disk_memory[FX_ROOT_CLUSTER_32 + 2] = 0;
    ram_disk_memory[FX_ROOT_CLUSTER_32 + 3] = 0;

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_MEDIA_INVALID)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(28);
    }

    /* Format the media with volume name containing space!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "NO NAME",              // Volume Name
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
        test_control_return(29);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    status +=  fx_media_volume_get(&ram_disk, (CHAR *)local_buffer, FX_BOOT_SECTOR);

    /* Check for status.  */
    if ((status != FX_SUCCESS) || (memcmp(local_buffer, "NO NAME", 7)))
    {
        printf("ERROR!\n");
        test_control_return(30);
    }

    status = fx_media_close(&ram_disk);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(31);
    }

    memset(ram_disk_memory, 0, sizeof(ram_disk_memory));

    /* Format the media with an even number of FAT32 sectors.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            70656+9,                // Total sectors  - FAT32
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(32);
    }

    unsigned index =0;
    /* Check the FAT entry values, first byte starts with 0xF(_fx_media_format_media_type) */
    for(index=0; index< ((70656+9)*128);index++)
    {
        if((ram_disk_memory[index]==0xF8) && (ram_disk_memory[index+1]==0xFF) && (ram_disk_memory[index+2]==0xFF) && 
        (ram_disk_memory[index+3]==0x0F) && (ram_disk_memory[index+4]==0xFF) && (ram_disk_memory[index+5]==0xFF) && 
        (ram_disk_memory[index+6]==0xFF) && (ram_disk_memory[index+7]==0x0F))
        {
            break;
        }
    }

    if(index ==(70656+9)*128)
    {

        printf("ERROR!\n");
        test_control_return(33);
    }
    else
    {
        printf("SUCCESS!\n");
        test_control_return(0);

    }
    
}

