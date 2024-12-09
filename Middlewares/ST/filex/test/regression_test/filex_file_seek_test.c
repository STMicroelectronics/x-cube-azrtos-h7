/* This FileX test concentrates on the file seek operation.  */

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


/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *ram_disk_memory;
static UCHAR                  *cache_buffer;
#else
static UCHAR                  cache_buffer[CACHE_SIZE];
#endif
static UCHAR                  buffer[128];

/* Define thread prototypes.  */

void    filex_file_seek_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_file_seek_application_define(void *first_unused_memory)
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
ULONG       read_value;
ULONG       write_value;
ULONG       available_bytes;
ULONG       i;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   File seek test.........................................");

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
    
    /* Attempt to seek before the media is opened to generate an error */
    status = fx_file_extended_seek(&my_file, 0);
    if (status != FX_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(2);
    }
    
    /* Attempt to seek before the media is opened to generate an error */
    status = fx_file_extended_relative_seek(&my_file, 0, 0);
    if (status != FX_NOT_OPEN)
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

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(5);
    }

    /* Open the test file.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(6);
    }
    
    /* Attempt to seek before the media is opened to generate an error */
    status = fx_file_extended_relative_seek(&my_file, 0xFFFFFFFFFFFFFFFF, FX_SEEK_BACK);
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(7);
    }
    
/* test error checking */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send null pointer to generate an error */
    status = fx_media_space_available(FX_NULL, FX_NULL);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(8);
    }
    
    /* send an invalid option to generate an error */
    status = fx_file_relative_seek(&my_file, 0xFFFFFFFF, 4);
    if (status != FX_INVALID_OPTION)
    {
        printf("ERROR!\n");
        test_control_return(9);
    }
    
    /* send null pointer to generate an error */
    status = fx_file_extended_seek(FX_NULL, 0xFF);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(10);
    }
    
    /* send null pointer to generate an error */
    status = fx_file_extended_relative_seek(FX_NULL, 0xFF, FX_SEEK_BEGIN);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(11);
    }
    
    /* send null pointer to generate an error */
    status = fx_file_extended_relative_seek(&my_file, 0xFF, 4);
    if (status != FX_INVALID_OPTION)
    {
        printf("ERROR!\n");
        test_control_return(12);
    }
    
#endif /* FX_DISABLE_ERROR_CHECKING */

    /* Pickup the available bytes in the media.  */
    status =  fx_media_space_available(&ram_disk, &available_bytes);
    
    /* Check for available bytes error.  */
    if ((status != FX_SUCCESS) || (available_bytes < sizeof(ULONG)))
    {

        printf("ERROR!\n");
        test_control_return(13);
    }

    /* Loop to write successive bytes out to the file.... to fill the media!  */
    i =  0;
    write_value =  0;
    while (i < available_bytes)
    {
    
        /* Write 4 bytes to the file.  */
        status =  fx_file_write(&my_file, (void *) &write_value, sizeof(ULONG));

        /* Check the file write status.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(14);
        }
        
        /* Increment byte count.  */
        i =  i + sizeof(ULONG);
        
        /* Increment write value.  */
        write_value++;
    }
    
    /* Pickup the available bytes in the media again.  */
    status =  fx_media_space_available(&ram_disk, &i);
    
    /* Check for available bytes error.  */
    if ((status != FX_SUCCESS) || (i != 0))
    {

        printf("ERROR!\n");
        test_control_return(15);
    }
    
#ifndef FX_DISABLE_CACHE
    /* At this point, we should invalidate the (which also flushes the cache) media to ensure that all 
       dirty sectors are written.  */
    status =  fx_media_cache_invalidate(&ram_disk);
    
    /* Check for invalidate errors.  */
    if ((status != FX_SUCCESS) || (ram_disk.fx_media_sector_cache_dirty_count))
    {

        printf("ERROR!\n");
        test_control_return(16);
    }
    
    /* See if any sectors are still valid in the cache.  */
    for (i = 0; i < FX_MAX_SECTOR_CACHE; i++)
    {
    
        /* Determine if this cache entry is still valid.  */
        if (ram_disk.fx_media_sector_cache[i].fx_cached_sector_valid)
        {
        
            printf("ERROR!\n");
            test_control_return(17);
        }
    }
#endif
    
    /* Seek to the beginning of the test file.  */
    status =  fx_file_seek(&my_file, 0);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(18);
    }

    /* Read the 4 bytes at the front of the file... should be 0!  */
    status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != 0) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(19);
    }

    /* Read the next 4 bytes at the front of the file... should be 1!  */
    status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != 1) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(20);
    }

    /* Seek to near the last 4 bytes of the file.  */
    status =  fx_file_seek(&my_file, available_bytes - 4);
    
    /* Read the last 4 bytes of the file...  should be available_bytes/sizeof(ULONG)!  */
    status +=  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != (available_bytes/sizeof(ULONG) - 1)) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(21);
    }
    
    /* Read the past the end of the file...  should get an error in this case.   */
    status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if (status != FX_END_OF_FILE)
    {
    
        printf("ERROR!\n");
        test_control_return(22);
    }

    /* Seek to the middle of the file.  */
    status =  fx_file_seek(&my_file, available_bytes/2);
    
    /* Read the middle 4 bytes of the file...  should be (available_bytes/2)/sizeof(ULONG)!  */
    status +=  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != (available_bytes/(2*sizeof(ULONG)))) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(23);
    }

    /* Seek to the end of the file.  */
    status =  fx_file_seek(&my_file, 0xFFFFFFFF);

    /* Determine if it is correct.  */
    if (status != FX_SUCCESS)
    {
    
        printf("ERROR!\n");
        test_control_return(24);
    }

    /* Close the file.  */
    status +=  fx_file_close(&my_file);

    /* Open the test file for fast reading.  */
    status +=  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_READ_FAST);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(25);
    }

    /* Read the 4 bytes at the front of the file... should be 0!  */
    status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != 0) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(26);
    }

    /* Read the next 4 bytes at the front of the file... should be 1!  */
    status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != 1) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(27);
    }

    /* Seek to near the last 4 bytes of the file.  */
    status =  fx_file_seek(&my_file, available_bytes - 4);
    
    /* Read the last 4 bytes of the file...  should be available_bytes/sizeof(ULONG)!  */
    status +=  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != (available_bytes/sizeof(ULONG) - 1)) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(28);
    }
    
    /* Read the past the end of the file...  should get an error in this case.   */
    status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if (status != FX_END_OF_FILE)
    {
    
        printf("ERROR!\n");
        test_control_return(29);
    }

    /* Seek to the middle of the file.  */
    status =  fx_file_seek(&my_file, available_bytes/2);
    
    /* Read the middle 4 bytes of the file...  should be (available_bytes/2)/sizeof(ULONG)!  */
    status +=  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != (available_bytes/(2*sizeof(ULONG)))) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(30);
    }

    /* Seek to the end of the file.  */
    status =  fx_file_seek(&my_file, 0xFFFFFFFF);

    /* Determine if it is correct.  */
    if (status != FX_SUCCESS)
    {
    
        printf("ERROR!\n");
        test_control_return(31);
    }

    /* Close the file.  */
    status +=  fx_file_close(&my_file);

    /* Open the test file for reading.  */
    status +=  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_READ);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(32);
    }
    
/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send null pointer to generate an error */
    status = fx_file_relative_seek(FX_NULL, 0, FX_SEEK_BEGIN);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(33);
    }
#endif /* FX_DISABLE_ERROR_CHECKING */

    /* Read the 4 bytes at the front of the file... should be 0!  */
    status =  fx_file_relative_seek(&my_file, 0, FX_SEEK_BEGIN);
    status +=  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != 0) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(34);
    }

    /* Read the next 4 bytes at the front of the file... should be 1!  */
    status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != 1) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(35);
    }

    /* Seek to near the last 4 bytes of the file.  */
    status =  fx_file_relative_seek(&my_file, 0xFFFFFFFF, FX_SEEK_BEGIN);
    status += fx_file_relative_seek(&my_file, 4, FX_SEEK_BACK);
    
    /* Read the last 4 bytes of the file...  should be available_bytes/sizeof(ULONG)!  */
    status +=  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != (available_bytes/sizeof(ULONG) - 1)) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(36);
    }
    
    /* Read the past the end of the file...  should get an error in this case.   */
    status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if (status != FX_END_OF_FILE)
    {
    
        printf("ERROR!\n");
        test_control_return(37);
    }

    /* Seek to the middle of the file.  */
    status =  fx_file_relative_seek(&my_file, 0xFFFFFFFF, FX_SEEK_END);
    status += fx_file_relative_seek(&my_file, available_bytes/2, FX_SEEK_FORWARD);
    
    /* Read the middle 4 bytes of the file...  should be (available_bytes/2)/sizeof(ULONG)!  */
    status +=  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != (available_bytes/(2*sizeof(ULONG)))) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(38);
    }

    /* Seek to the end of the file.  */
    status =  fx_file_relative_seek(&my_file, 0, FX_SEEK_END);
 
    /* Determine if it is correct.  */
    if (status != FX_SUCCESS)
    {
    
        printf("ERROR!\n");
        test_control_return(39);
    }

    /* Close the file.  */
    status +=  fx_file_close(&my_file);

    /* Close the media.  */
    status +=  fx_media_close(&ram_disk);   

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(40);
    }
    
    /* Test corner cases of extended seek.  */    
    
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
        test_control_return(41);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(42);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(43);
    }

    /* Open the test file.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(44);
    }

    /* Write data to the file.  */
    status =  fx_file_write(&my_file, buffer, 128);
    status += fx_file_write(&my_file, buffer, 128);
    status += fx_file_write(&my_file, buffer, 128);
    status += fx_file_write(&my_file, buffer, 128);

    status += fx_file_write(&my_file, buffer, 128);
    status += fx_file_write(&my_file, buffer, 128);
    status += fx_file_write(&my_file, buffer, 128);
    status += fx_file_write(&my_file, buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(45);
    }

    /* Seek to the beginning of the file.  */
    status = fx_file_extended_seek(&my_file, 0);
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(46);
    }

    /* Seek to the second to beginning of the last cluster of the file.  */
    status = fx_file_extended_seek(&my_file, 1024-128);
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(47);
    }

    /* Seek to the end of the file with an I/O error.  */
    _fx_utility_fat_entry_read_error_request =  1;
    status = fx_file_extended_seek(&my_file, 1024);
    _fx_utility_fat_entry_read_error_request =  0;
    if (status != FX_IO_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(48);
    }

    /* Seek to the end of the file with a FAT entry of 1 error.  */
    status =  fx_file_extended_seek(&my_file, 0);
    _fx_utility_fat_entry_read_error_request =  10001;
    status += fx_file_extended_seek(&my_file, 1024);
    _fx_utility_fat_entry_read_error_request =  0;
    if (status != FX_FILE_CORRUPT)
    {
        printf("ERROR!\n");
        test_control_return(49);
    }

    /* Seek to the end of the file with a FAT entry of max fat value error.  */
    _fx_utility_fat_entry_read_error_request =  20001;
    status = fx_file_extended_seek(&my_file, 1024);
    _fx_utility_fat_entry_read_error_request =  0;
    if (status != FX_FILE_CORRUPT)
    {
        printf("ERROR!\n");
        test_control_return(50);
    }

    /* Seek to the end of the file with a FAT entry of minimal value error.  */
    _fx_utility_fat_entry_read_error_request =  10008;
    status = fx_file_extended_seek(&my_file, 1024);
    _fx_utility_fat_entry_read_error_request =  0;
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(51);
    }
    
    /* Close the file and the media.  */
    status =  fx_file_close(&my_file);
    status += fx_media_close(&ram_disk);
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(52);
    }

    /* Test zero divisor checking in fx_file_extended_seek.  */
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

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(53);
    }

    /* Open the ram_disk.  */
    status = fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(54);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status = fx_file_create(&ram_disk, "TEST.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(55);
    }

    /* Open the test file.  */
    status = fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(56);
    }


    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(57);
    }

    /* Corrupt the media.  */
    ram_disk.fx_media_bytes_per_sector = 0;

    status = fx_file_extended_seek(&my_file, 1);

    if (status != FX_MEDIA_INVALID)
    {
        printf("ERROR!\n");
        test_control_return(58);
    }
    ram_disk.fx_media_bytes_per_sector = 128;

    /* Write data to the file.  */
    status = fx_file_write(&my_file, buffer, 128);

    /* Corrupt the media.  */
    ram_disk.fx_media_sectors_per_FAT = 0;

    status = fx_file_write(&my_file, buffer, 128);

    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(59);
    }

    /* Close the file and the media.  */
    status = fx_file_close(&my_file);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(60);
    }

    /* Corrupt the media.  */
    ram_disk.fx_media_bytes_per_sector = 0;

    /* Open the test file.  */
    status = fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_MEDIA_INVALID)
    {

        printf("ERROR!\n");
        test_control_return(56);
    }
    ram_disk.fx_media_bytes_per_sector = 128;

    status = fx_media_close(&ram_disk);

    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(60);
    }
    else
    {

        printf("SUCCESS!\n");
        test_control_return(0);
    }
}

