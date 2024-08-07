/* This FileX test concentrates on the file allocate/truncate operation.  */

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
static FX_FILE                 read_only;

/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *ram_disk_memory;
static UCHAR                  *cache_buffer;
#else
static UCHAR                   cache_buffer[CACHE_SIZE];
#endif


/* Define thread prototypes.  */

void    filex_file_allocate_truncate_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_file_allocate_truncate_application_define(void *first_unused_memory)
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
ULONG       temp;
ULONG       write_value;
ULONG       available_bytes;
CHAR        buffer[1];
ULONG       i;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   File allocate/truncate test............................");

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
    
    /* Attempt to truncate a file before the file is opened */
    status = fx_file_extended_truncate(&my_file, 0);
    if (status != FX_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(2);
    }
    
    /* Attempt to release a file before the file is opened */
    status = fx_file_extended_truncate_release(&my_file, 0);
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

    /* Create a file called READ_ONLY.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "READ_ONLY.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(6);
    }

    /* Pickup the available bytes in the media.  */
    status =  fx_media_space_available(&ram_disk, &available_bytes);
    
    /* Check for available bytes error.  */
    if ((status != FX_SUCCESS) || (available_bytes < sizeof(ULONG)))
    {

        printf("ERROR!\n");
        test_control_return(7);
    }

    /* Open the test files.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status =  fx_file_open(&ram_disk, &read_only, "READ_ONLY.TXT", FX_OPEN_FOR_READ);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(8);
    }
    
    /* Try to release more than was allocated */
    status = fx_file_extended_truncate_release(&my_file, 0xFFFFFFFFFFFFFFFF);
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(9);
    }
    
    /* Attempt to truncate a file that is not open for writing */
    status = fx_file_extended_truncate(&read_only, 0);
    if (status != FX_ACCESS_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(10);
    }
    
    /* Attempt to release a file that is not open for writing */
    status = fx_file_extended_truncate_release(&read_only, 0);
    if (status != FX_ACCESS_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(11);
    }
    
    /* Attempt to truncate a file while the media is write protected */
    ram_disk.fx_media_driver_write_protect = FX_TRUE;
    status = fx_file_extended_truncate(&my_file, 0);
    if (status != FX_WRITE_PROTECT)
    {
        printf("ERROR!\n");
        test_control_return(12);
    }
    
    /* Attempt to release a file while the media is write protected */
    status = fx_file_extended_truncate_release(&my_file, 0);
    if (status != FX_WRITE_PROTECT)
    {
        printf("ERROR!\n");
        test_control_return(13);
    }
    ram_disk.fx_media_driver_write_protect = FX_FALSE;
    
    ram_disk.fx_media_bytes_per_sector = 0;

    /* Attempt to release a file while the media is corrupted.  */
    status = fx_file_extended_truncate_release(&my_file, 0);
    if (status != FX_MEDIA_INVALID)
    {
        printf("ERROR!\n");
        test_control_return(13);
    }
    ram_disk.fx_media_bytes_per_sector = 128;

    /* Attempt to truncate a file to larger than it is */
    status = fx_file_extended_truncate(&my_file, 0xFFFFFFFFFFFFFFFF);
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(14);
    }
    
/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send null pointer to generate an error */
    status = fx_file_allocate(FX_NULL, 1500);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(15);
    }
    
    /* send null pointer to generate an error */
    status = fx_file_extended_truncate(FX_NULL, 0xFF);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(16);
    }
    
    /* send null pointer to generate an error */
    status = fx_file_extended_truncate_release(FX_NULL, 0xFF);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(17);
    }
    
    /* send null pointer to generate an error */
    status = fx_file_extended_allocate(FX_NULL, 0xFF);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(18);
    }
    
    /* send null pointer to generate an error */
    status = fx_file_best_effort_allocate(FX_NULL, 0xFF, FX_NULL);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(19);
    }
#endif /* FX_DISABLE_ERROR_CHECKING */

    /* Allocate half the size first...  */
    status =  fx_file_allocate(&my_file, available_bytes/2);
    
    /* Check the file allocate status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(20);
    }

    /* Pickup the available bytes in the media again.  */
    status =  fx_media_space_available(&ram_disk, &i);

    
    /* Attempt to allocate all the bytes again... but the best effort should just get us the 
       remaining bytes...  */
    status +=  fx_file_best_effort_allocate(&my_file, available_bytes, &actual);
    
    /* Check the best effort file allocate status.  */
    if ((status != FX_SUCCESS) || (actual != i))
    {

        printf("ERROR!\n");
        test_control_return(21);
    }    
    
#ifdef FX_UPDATE_FILE_SIZE_ON_ALLOCATE    
    /* Seek to the beginning of the test file.  */
    status =  fx_file_seek(&my_file, 0);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)    
    {

        printf("ERROR!\n");
        test_control_return(22);
    }
    
    /* send null pointer to generate an error */
    status = fx_file_best_effort_allocate(FX_NULL, 1500, FX_NULL);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(23);
    }
#endif /* FX_DISABLE_ERROR_CHECKING */

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
            test_control_return(24);
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
        test_control_return(25);
    }
    
#ifndef FX_DISABLE_CACHE
    /* At this point, we should invalidate the (which also flushes the cache) media to ensure that all 
       dirty sectors are written.  */
    status =  fx_media_cache_invalidate(&ram_disk);
    
    /* Check for invalidate errors.  */
    if ((status != FX_SUCCESS) || (ram_disk.fx_media_sector_cache_dirty_count))
    {

        printf("ERROR!\n");
        test_control_return(26);
    }
    
    /* See if any sectors are still valid in the cache.  */
    for (i = 0; i < FX_MAX_SECTOR_CACHE; i++)
    {
    
        /* Determine if this cache entry is still valid.  */
        if (ram_disk.fx_media_sector_cache[i].fx_cached_sector_valid)
        {
        
            printf("ERROR!\n");
            test_control_return(27);
        }
    }
#endif

    /* Now truncate half the file...  */
    status =  fx_file_truncate(&my_file, available_bytes/2);
    
    /* Check for errors... */
    if ((status) || (my_file.fx_file_current_file_size != my_file.fx_file_current_available_size/2))
    {

        printf("ERROR!\n");
        test_control_return(28);
    }
    
/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send null pointer to generate an error */
    status = fx_file_truncate(FX_NULL, 0);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(29);
    }
    
    /* send null pointer to generate an error */
    status = fx_file_truncate_release(FX_NULL, 0);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(30);
    }
#endif /* FX_DISABLE_ERROR_CHECKING */

    /* Now truncate the remainder of file...  */
    status =  fx_file_truncate(&my_file, 0);
    
    /* Check for errors... */
    if ((status) || (my_file.fx_file_current_file_size != 0))
    {

        printf("ERROR!\n");
        test_control_return(31);
    }

    /* Pickup the available bytes in the media again.  */
    status =  fx_media_space_available(&ram_disk, &i);
    
    /* Check for available bytes error.  */
    if ((status != FX_SUCCESS) || (i != 0))
    {

        printf("ERROR!\n");
        test_control_return(32);
    }

    /* At this point write the file again... */
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
            test_control_return(33);
        }
        
        /* Increment byte count.  */
        i =  i + sizeof(ULONG);
        
        /* Increment write value.  */
        write_value++;
    }
    
    /* Now truncate and release half the file...  */
    status =  fx_file_truncate_release(&my_file, available_bytes/2);

    /* Pickup the available bytes in the media again.  */
    status +=  fx_media_space_available(&ram_disk, &i);
    
    /* Check for errors... */
    if ((status) || (my_file.fx_file_current_file_size != available_bytes/2))
    {

        printf("ERROR!\n");
        test_control_return(34);
    }

    /* Now truncate and release the remaining half the file...  */
    status =  fx_file_truncate_release(&my_file, 0);

    /* Pickup the available bytes in the media again.  */
    status +=  fx_media_space_available(&ram_disk, &i);
    
    /* Check for errors... */
    if ((status) || (my_file.fx_file_current_file_size != 0) || 
            (i != available_bytes))
    {

        printf("ERROR!\n");
        test_control_return(35);
    }

    /* Test overflow of available bytes >4GB of this API.  */

    /* Set the available clusters to maximum value.  */
    temp =  ram_disk.fx_media_available_clusters;
    ram_disk.fx_media_available_clusters =  0xFFFFFFFF;

    /* Pickup the available bytes in the media again.  */
    status =  fx_media_space_available(&ram_disk, &available_bytes);
    
    /* Restore the actual available clusters.  */
    ram_disk.fx_media_available_clusters =  temp;
    
    /* Check for errors... */
    if ((status) || (available_bytes != 0xFFFFFFFF))
    {

        printf("ERROR!\n");
        test_control_return(36);
    }
 
    /* Close the file.  */
    status =  fx_file_close(&my_file);

    /* Close the media.  */
    status +=  fx_media_close(&ram_disk);   

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(37);
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
                            18000,                  // Total sectors 
                            128,                    // Sector size   
                            3,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(38);
    }
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(39);
    }

    /* Create a file called TEST.TXT in the root directory.  */
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
    status += fx_file_create(&ram_disk, "TEST11.TXT");
    status += fx_file_create(&ram_disk, "TEST12.TXT");
    status += fx_file_create(&ram_disk, "TEST13.TXT");
    status += fx_file_create(&ram_disk, "TEST14.TXT");
    status += fx_file_create(&ram_disk, "TEST15.TXT");
    status += fx_file_create(&ram_disk, "TEST16.TXT");
    status += fx_file_create(&ram_disk, "TEST17.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(40);
    }

    /* Open the test file.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &read_only, "TEST.TXT", FX_OPEN_FOR_READ);
    status += fx_file_open(&ram_disk, &my_file1, "TEST1.TXT", FX_OPEN_FOR_READ);
    status += fx_file_open(&ram_disk, &my_file2, "TEST17.TXT", FX_OPEN_FOR_READ);
   
    /* Write 2048 bytes to the file.  */
    for (i = 0; i < 4096; i++)
    {
        /* Write a byte to the file.  */
        status += fx_file_write(&my_file, " ", 1);
        status += fx_file_read(&read_only, buffer, 1, &actual);
    }

    /* Allocate some additional clusters.  */
    status +=  fx_file_allocate(&my_file, 128*6);
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(41);
    }
    
    /* Now truncate the file to a value less than the available size, but greater than the current size.  */
    status = fx_file_extended_truncate_release(&my_file, 4096 + 128);
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(42);
    }
    
    /* Now truncate the file to the available size.  */
    status = fx_file_extended_truncate_release(&my_file, 4096);
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(43);
    }

    /* Now truncate the file to half the available size.  */
    status = fx_file_extended_truncate_release(&my_file, 2048);
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(44);
    }
    
    /* Now truncate the file to nothing.  */
    status = fx_file_extended_truncate_release(&my_file, 0);
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(45);
    }
    
    /* Close everything down.  */
    status =  fx_file_close(&my_file);
    status += fx_file_close(&my_file1);
    status += fx_file_close(&my_file2);
    status += fx_file_close(&read_only);
    status += fx_media_close(&ram_disk);
    
    /* Check status.  */
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(46);
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
                            18000,                  // Total sectors 
                            128,                    // Sector size   
                            3,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(47);
    }
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(48);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(49);
    }

    /* Open the test file.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &read_only, "TEST.TXT", FX_OPEN_FOR_READ);
   
    /* Write 2048 bytes to the file.  */
    for (i = 0; i < 4096; i++)
    {
        /* Write a byte to the file.  */
        status += fx_file_write(&my_file, " ", 1);
        status += fx_file_read(&read_only, buffer, 1, &actual);
    }
    
    /* Flush the media.  */
    status += fx_media_flush(&ram_disk);
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(50);
    }
    
    /* Now truncate the file to half the available size, but force an I/O error on the directory entry write.  */
    _fx_ram_driver_io_error_request =  1;
    status = fx_file_extended_truncate_release(&my_file, 2048);
    _fx_ram_driver_io_error_request =  0;
    if (status != FX_IO_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(51);
    }
    
    /* Now truncate the file with FAT I/O Error.  */
    _fx_utility_fat_entry_read_error_request =  1;
    status = fx_file_extended_truncate_release(&my_file, 2000);
    _fx_utility_fat_entry_read_error_request =  0;
    if (status != FX_IO_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(52);
    }

    /* Now truncate the file with FAT I/O Error.  */
    _fx_utility_fat_entry_read_error_request =  6;
    status = fx_file_extended_truncate_release(&my_file, 1900);
    _fx_utility_fat_entry_read_error_request =  0;
    if (status != FX_IO_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(53);
    }

    /* Now truncate the file with FAT I/O Error.  */
    _fx_utility_fat_entry_write_error_request =  1;
    status = fx_file_extended_truncate_release(&my_file, 1700);
    _fx_utility_fat_entry_write_error_request =  0;
    if (status != FX_IO_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(54);
    }

    /* Now truncate the file with FAT I/O Error.  */
    _fx_utility_fat_entry_write_error_request =  2;
    status = fx_file_extended_truncate_release(&my_file, 1500);
    _fx_utility_fat_entry_write_error_request =  0;
    if (status != FX_IO_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(55);
    }

    /* Now truncate the file with FAT I/O Error.  */
    _fx_utility_fat_entry_read_error_request =  7;
    status = fx_file_extended_truncate_release(&my_file, 500);
    _fx_utility_fat_entry_read_error_request =  0;
    if (status != FX_IO_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(56);
    }

    /* Close everything down.  */
    status =  fx_file_close(&my_file);
    status += fx_file_close(&read_only);
    status += fx_media_close(&ram_disk);
    
    /* Check status.  */
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(54);
    }    

    /* Test the cluster values of 1 and fx_media_fat_reserved errors in the FAT chain.  */

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
                            18000,                  // Total sectors 
                            128,                    // Sector size   
                            3,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(55);
    }
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(56);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(57);
    }

    /* Open the test file.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &read_only, "TEST.TXT", FX_OPEN_FOR_READ);
   
    /* Write 2048 bytes to the file.  */
    for (i = 0; i < 4096; i++)
    {
        /* Write a byte to the file.  */
        status += fx_file_write(&my_file, " ", 1);
        status += fx_file_read(&read_only, buffer, 1, &actual);
    }
    
    /* Flush the media.  */
    status += fx_media_flush(&ram_disk);
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(58);
    }
    
    /* Now truncate one byte of the file.  */
    status = fx_file_extended_truncate_release(&my_file, 4095);

    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(59);
    }


    /* Now truncate the file but force a FAT entry of 1.  */
    _fx_utility_fat_entry_read_error_request =  10005;
    status = fx_file_extended_truncate_release(&my_file, 4094);
    _fx_utility_fat_entry_read_error_request =  0;
    if (status != FX_FILE_CORRUPT)
    {
        printf("ERROR!\n");
        test_control_return(60);
    }

    /* Now truncate the file but force a FAT entry of max value.  */
    _fx_utility_fat_entry_read_error_request =  20005;
    status = fx_file_extended_truncate_release(&my_file, 4093);
    _fx_utility_fat_entry_read_error_request =  0;
    if (status != FX_FILE_CORRUPT)
    {
        printf("ERROR!\n");
        test_control_return(61);
    }

    /* Now truncate the file but force a FAT entry of 1 to the traversal of releasing clusters.  */
    _fx_utility_fat_entry_read_error_request =  10012;
    status = fx_file_extended_truncate_release(&my_file, 4092);
    _fx_utility_fat_entry_read_error_request =  0;
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(62);
    }

    /* Now truncate the file but force a FAT entry of 1 in the traversal of open files.  */
    _fx_utility_fat_entry_read_error_request =  10025;
    status = fx_file_extended_truncate_release(&my_file, 4091);
    _fx_utility_fat_entry_read_error_request =  0;
    if (status != FX_FILE_CORRUPT)
    {
        printf("ERROR!\n");
        test_control_return(63);
    }

    /* Now truncate the file but force a FAT entry of 1 in the traversal of open files.  */
    _fx_utility_fat_entry_read_error_request =  20025;
    status = fx_file_extended_truncate_release(&my_file, 4090);
    _fx_utility_fat_entry_read_error_request =  0;
    if (status != FX_FILE_CORRUPT)
    {
        printf("ERROR!\n");
        test_control_return(64);
    }

    /* Close everything down.  */
    status =  fx_file_close(&my_file);
    status += fx_file_close(&read_only);
    status += fx_media_close(&ram_disk);
    
    /* Check status.  */
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(63);
    }    

    /* Test file extended truncate corner cases.  */

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
                            18000,                  // Total sectors 
                            128,                    // Sector size   
                            3,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(64);
    }
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(65);
    }

    /* Create a file called TEST.TXT in the root directory.  */
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
    status += fx_file_create(&ram_disk, "TEST11.TXT");
    status += fx_file_create(&ram_disk, "TEST12.TXT");
    status += fx_file_create(&ram_disk, "TEST13.TXT");
    status += fx_file_create(&ram_disk, "TEST14.TXT");
    status += fx_file_create(&ram_disk, "TEST15.TXT");
    status += fx_file_create(&ram_disk, "TEST16.TXT");
    status += fx_file_create(&ram_disk, "TEST17.TXT");
    status += fx_file_create(&ram_disk, "TEST18.TXT");
    status += fx_file_create(&ram_disk, "TEST19.TXT");
    status += fx_file_create(&ram_disk, "TEST20.TXT");
    status += fx_file_create(&ram_disk, "TEST21.TXT");
    status += fx_file_create(&ram_disk, "TEST22.TXT");
    status += fx_file_create(&ram_disk, "TEST23.TXT");
    status += fx_file_create(&ram_disk, "TEST24.TXT");
    status += fx_file_create(&ram_disk, "TEST25.TXT");
    status += fx_file_create(&ram_disk, "TEST26.TXT");
    status += fx_file_create(&ram_disk, "TEST27.TXT");
    status += fx_file_create(&ram_disk, "TEST28.TXT");
    status += fx_file_create(&ram_disk, "TEST29.TXT");
    status += fx_file_create(&ram_disk, "TEST30.TXT");


    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(67);
    }

    /* Open the test file.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &read_only, "TEST.TXT", FX_OPEN_FOR_READ);
    status += fx_file_open(&ram_disk, &my_file1, "TEST1.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &my_file2, "TEST30.TXT", FX_OPEN_FOR_WRITE);
   
    /* Write 2048 bytes to the file.  */
    for (i = 0; i < 4096; i++)
    {
        /* Write a byte to the file.  */
        status += fx_file_write(&my_file, " ", 1);
        status += fx_file_read(&read_only, buffer, 1, &actual);
    }
    
    /* Flush the media.  */
    status += fx_media_flush(&ram_disk);
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(68);
    }
    
    /* Now truncate one byte of the file.  */
    status = fx_file_extended_truncate(&my_file, 4095);

    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(69);
    }

    /* Seek to make the file offset less than our new truncate size.  */
    status =  fx_file_seek(&my_file, 4094);
    
    /* Truncate another byte of the file.  */
    status = fx_file_extended_truncate(&my_file, 4094);

    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(70);
    }
    
    /* Now truncate the another byte of the file, but with a FAT read error.  */
    _fx_utility_fat_entry_read_error_request =  1;
    status = fx_file_extended_truncate(&my_file, 4093);
    _fx_utility_fat_entry_read_error_request =  0;
    if (status != FX_IO_ERROR)   
    {
        printf("ERROR!\n");
        test_control_return(71);
    }
      
    
    /* Now truncate the file but force a FAT entry of 1.  */
    _fx_utility_fat_entry_read_error_request =  10003;
    status = fx_file_extended_truncate(&my_file, 4092);
    _fx_utility_fat_entry_read_error_request =  0;
    if (status != FX_FILE_CORRUPT)
    {
        printf("ERROR!\n");
        test_control_return(72);
    }

    /* Now truncate the file but force a FAT entry of max value.  */
    _fx_utility_fat_entry_read_error_request =  20003;
    status = fx_file_extended_truncate(&my_file, 4091);
    _fx_utility_fat_entry_read_error_request =  0;
    if (status != FX_FILE_CORRUPT)
    {
        printf("ERROR!\n");
        test_control_return(73);
    }

    /* Close everything down.  */
    status =  fx_file_close(&my_file);
    status += fx_file_close(&my_file1);
    status += fx_file_close(&my_file2);
    status += fx_file_close(&read_only);
    status += fx_media_close(&ram_disk);
    
    /* Check status.  */
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(74);
    }    
    else
    {

        printf("SUCCESS!\n");
        test_control_return(0);
    }
}

