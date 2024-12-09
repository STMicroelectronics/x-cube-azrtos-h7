/* This FileX test concentrates on the file read/write operation.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_utility.h"
#include   "fx_ram_driver_test.h"
#include   <stdio.h>

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
static FX_FILE                 read_only;
static ULONG                   my_buffer[128];
static UCHAR                   fat_buffer[128];
static UCHAR                   buffer[128*3];


/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *ram_disk_memory;
static UCHAR                  *cache_buffer;
#else
static UCHAR                   cache_buffer[CACHE_SIZE];
#endif


/* Define thread prototypes.  */

void    filex_file_read_write_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_file_read_write_application_define(void *first_unused_memory)
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
ULONG       actual1;
ULONG       actual2;
ULONG       temp;
ULONG       temp1;
ULONG       read_value;
ULONG       write_value;
ULONG       available_bytes;
ULONG       i, j;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   File read/write test...................................");

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
                            511,                    // Total sectors 
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

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(21);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_create(&ram_disk, "READ_ONLY.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(3);
    }
    
    /* try to write to a file before it has been opened  */
    status =  fx_file_write(&my_file, (void *) &write_value, sizeof(ULONG));
    if (status != FX_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(23);
    }

    /* Open the test file.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &read_only, "READ_ONLY.TXT", FX_OPEN_FOR_READ);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(4);
    }
    
    /* try to write to a file while it is write protected  */
    ram_disk.fx_media_driver_write_protect = FX_TRUE;
    status =  fx_file_write(&my_file, (void *) &write_value, sizeof(ULONG));
    if (status != FX_WRITE_PROTECT)
    {
        printf("ERROR!\n");
        test_control_return(23);
    }
    ram_disk.fx_media_driver_write_protect = FX_FALSE;
    
    /* try to write to a file that is not opened for writing  */
    status =  fx_file_write(&read_only, (void *) &write_value, sizeof(ULONG));
    if (status != FX_ACCESS_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(23);
    }
    
    ram_disk.fx_media_bytes_per_sector = 0;

    /* Try to write to a file when media is corrupted.  */
    status = fx_file_write(&my_file, (void*)& write_value, sizeof(ULONG));
    if (status != FX_MEDIA_INVALID)
    {
        printf("ERROR!\n");
        test_control_return(23);
    }

    ram_disk.fx_media_bytes_per_sector = 128;

    /* Pickup the available bytes in the media.  */
    status =  fx_media_space_available(&ram_disk, &available_bytes);
    
    /* Check for available bytes error.  */
    if ((status != FX_SUCCESS) || (available_bytes < sizeof(ULONG)))
    {

        printf("ERROR!\n");
        test_control_return(5);
    }
    
/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send null pointer to generate an error */
    status = fx_file_write(FX_NULL, (void *) &write_value, 0);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(11);
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
            test_control_return(6);
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
        test_control_return(7);
    }
    
#ifndef FX_DISABLE_CACHE
    /* At this point, we should invalidate the (which also flushes the cache) media to ensure that all 
       dirty sectors are written.  */
    status =  fx_media_cache_invalidate(&ram_disk);
    
    /* Check for flush errors.  */
    if ((status != FX_SUCCESS) || (ram_disk.fx_media_sector_cache_dirty_count))
    {

        printf("ERROR!\n");
        test_control_return(8);
    }
    
    /* See if any sectors are still valid in the cache.  */
    for (i = 0; i < FX_MAX_SECTOR_CACHE; i++)
    {
    
        /* Determine if this cache entry is still valid.  */
        if (ram_disk.fx_media_sector_cache[i].fx_cached_sector_valid)
        {
        
            printf("ERROR!\n");
            test_control_return(81);
        }
    }
#endif
    
    /* Seek to the beginning of the test file.  */
    status =  fx_file_seek(&my_file, 0);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(9);
    }
    
/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send null pointer to generate an error */
    status = fx_file_seek(FX_NULL, 0);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(11);
    }
#endif /* FX_DISABLE_ERROR_CHECKING */

    /* Now read in all the bytes again to make sure the file contents are really there.  */
    i =  0;
    read_value =  0;
    while (i < available_bytes)
    {
    
        /* Read 4 bytes from the file.  */
        status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);

        /* Check the file read status.  */
        if ((status != FX_SUCCESS) || (actual != 4) || (read_value != i/4))
        {

            printf("ERROR!\n");
            test_control_return(10);
        }

        /* Increment byte count.  */
        i =  i + sizeof(ULONG);
    }

    /* Close the test file.  */
    status =  fx_file_close(&my_file);

    /* Check the file close status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(11);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Check the media close status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(12);
    }

    /* Reformat the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            511,                    // Total sectors (ensure clusters divisible by 4)
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

    /* Open the ram_disk, but do so to ensure non-hashed algorithm is used by supplying CACHE_SIZE-1.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE-1);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(14);
    }
    
    /* Read 4 bytes from the file.  */
    status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    if (status != FX_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(23);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(15);
    }

    /* Open the test file.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(16);
    }

    /* Pickup the available bytes in the media.  */
    status =  fx_media_space_available(&ram_disk, &available_bytes);
    
    /* Check for available bytes error.  */
    if ((status != FX_SUCCESS) || (available_bytes < sizeof(ULONG)))
    {

        printf("ERROR!\n");
        test_control_return(17);
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
            test_control_return(18);
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
        test_control_return(19);
    }
    
#ifndef FX_DISABLE_CACHE
    /* At this point, we should invalidate the media to ensure that all 
       dirty sectors are written.  */
    status =  fx_media_cache_invalidate(&ram_disk);
    
    /* Check for flush errors.  */
    if ((status != FX_SUCCESS) || (ram_disk.fx_media_sector_cache_dirty_count))
    {

        printf("ERROR!\n");
        test_control_return(20);
    }

    /* See if any sectors are still valid in the cache.  */
    for (i = 0; i < ram_disk.fx_media_sector_cache_size; i++)
    {
    
        /* Determine if this cache entry is still valid.  */
        if (ram_disk.fx_media_sector_cache[i].fx_cached_sector_valid)
        {
        
            printf("ERROR!\n");
            test_control_return(81);
        }
    }
#endif
   
    /* Seek to the beginning of the test file.  */
    status =  fx_file_seek(&my_file, 0);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(22);
    }

    /* Now read in all the bytes again to make sure the file contents are really there.  */
    i =  0;
    read_value =  0;
    while (i < available_bytes)
    {
    
        /* Read 4 bytes from the file.  */
        status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);

        /* Check the file read status.  */
        if ((status != FX_SUCCESS) || (actual != 4) || (read_value != i/4))
        {

            printf("ERROR!\n");
            test_control_return(23);
        }

        /* Increment byte count.  */
        i =  i + sizeof(ULONG);
    }

    /* Close the test file.  */
    status =  fx_file_close(&my_file);

    /* Check the file close status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(24);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(25);
    }

    /* Open the file.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_READ);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        /* Error opening file.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(26);
    }
    
    /* Now read in all the bytes again to make sure the file contents are really there.  */
    i =  0;
    read_value =  0;
    while (i < available_bytes)
    {
    
        /* Read as much as 4 sectors full of bytes from the file.  */
        status =  fx_file_read(&my_file, (void *) my_buffer, sizeof(my_buffer), &actual);

        /* Check the file read status.  */
        if (status != FX_SUCCESS) 
        {

            printf("ERROR!\n");
            test_control_return(27);
        }

        /* Determine if the contents are what is expected.  */
        for (j = 0; j < actual/sizeof(ULONG); j++)
        {
        
            /* Determine if the buffer is correct.  */
            if (read_value != my_buffer[j])
            {
            
                printf("ERROR!\n");
                test_control_return(28);
            }
            
            read_value++;
        }

        /* Increment byte count.  */
        i =  i + actual;
    }

    /* Close the file.  */
    status =  fx_file_close(&my_file);
    
    /* Open the file again but with the fast option.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_READ_FAST);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        /* Error opening file.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(29);
    }
    
    /* Now read in all the bytes again to make sure the file contents are really there.  */
    i =  0;
    read_value =  0;
    while (i < available_bytes)
    {
    
        /* Read as much as 4 sectors full of bytes from the file.  */
        status =  fx_file_read(&my_file, (void *) my_buffer, sizeof(my_buffer), &actual);

        /* Check the file read status.  */
        if (status != FX_SUCCESS) 
        {

            printf("ERROR!\n");
            test_control_return(30);
        }

        /* Determine if the contents are what is expected.  */
        for (j = 0; j < actual/sizeof(ULONG); j++)
        {
        
            /* Determine if the buffer is correct.  */
            if (read_value != my_buffer[j])
            {
            
                printf("ERROR!\n");
                test_control_return(31);
            }
            
            read_value++;
        }

        /* Increment byte count.  */
        i =  i + actual;
    }

    /* Close the file.  */
    status =  fx_file_close(&my_file);

    /* Check the file close status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(32);
    }

    /* Delete the file.  */
    status =  fx_file_delete(&ram_disk, "TEST.TXT");
    
    /* Check the file delete status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(33);
    }

    /* Write the file in blocks and then read ulong at a time.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    
    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(34);
    }
    
    /* Now write the big buffer at the same time.  */
    /* Now read in all the bytes again to make sure the file contents are really there.  */
    i =  0;
    read_value =  0;
    while (i < available_bytes)
    {

        /* Build the buffer.  */
        for (j = 0; j < actual/sizeof(ULONG); j++)
        {
        
            /* Build the buffer entry.  */
            my_buffer[j] =  read_value++;
        }

    
        /* Write 4 sectors at a time.  */
        status =  fx_file_write(&my_file, (void *) my_buffer, sizeof(my_buffer));

        /* Check the file write status.  */
        if (status != FX_SUCCESS) 
        {

            printf("ERROR!\n");
            test_control_return(35);
        }

        /* Increment byte count.  */
        i =  i + sizeof(my_buffer);
    }
    
    /* At this point, seek to the beginning of the file and read every 4 bytes.  */
    status =  fx_file_seek(&my_file, 0);
    
    i =  0;
    read_value =  0;
    while (i < available_bytes)
    {
    
        /* Read 4 bytes from the file.  */
        status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);

        /* Check the file read status.  */
        if ((status != FX_SUCCESS) || (actual != 4) || (read_value != i/4))
        {

            printf("ERROR!\n");
            test_control_return(36);
        }

        /* Increment byte count.  */
        i =  i + sizeof(ULONG);
    }

    /* Close the file.  */
    status +=  fx_file_close(&my_file);
    
    /* Close the media.  */
    status +=  fx_media_close(&ram_disk);   

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(37);
    }

    /* Test the update of files open for reading while the write is happening.  */ 

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
                            6000,                   // Total sectors - FAT16
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
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
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_READ);
    status += fx_file_open(&ram_disk, &my_file1, "TEST.TXT", FX_OPEN_FOR_READ);
    status += fx_file_open(&ram_disk, &my_file2, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &my_file3, "TEST1.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &my_file4, "TEST2.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file4, my_buffer, 128);    
    
    /* Now loop through the maximum of clusters to fill and read the file.  */
    i =  0;
    while (ram_disk.fx_media_available_clusters)
    {
    
        if (i == 4)
        {
            status += fx_file_close(&my_file4);
            status += fx_file_delete(&ram_disk, "TEST2.TXT");
        }
    
        /* Write to the writable file.  */
        status +=  fx_file_write(&my_file2, my_buffer, 128);
        
        /* Read the data in the file from the other 2 file handles.  */
        status +=  fx_file_read(&my_file1, my_buffer, 128, &actual1);
        status +=  fx_file_read(&my_file, my_buffer, 128, &actual2);
        
        /* Check the status.  */
        if (status)
            break;
        i++;
    }
    
    /* Close the files and the media.  */
    status += fx_file_close(&my_file);
    status += fx_file_close(&my_file1);
    status += fx_file_close(&my_file2);
    status += fx_media_close(&ram_disk);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(40);
    }

    /* Test the update of files open for reading while the write is happening with multiple sectors per cluster.  */ 

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
                            6000,                   // Total sectors - FAT16
                            128,                    // Sector size   
                            2,                      // Sectors per cluster
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
    status += fx_file_create(&ram_disk, "TEST1.TXT");
    status += fx_file_create(&ram_disk, "TEST2.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_READ);
    status += fx_file_open(&ram_disk, &my_file1, "TEST.TXT", FX_OPEN_FOR_READ);
    status += fx_file_open(&ram_disk, &my_file2, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &my_file3, "TEST1.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &my_file4, "TEST2.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file4, my_buffer, 128);    
    
    /* Now loop through the maximum of clusters to fill and read the file.  */
    i =  0;
    while (ram_disk.fx_media_available_clusters)
    {
    
        if (i == 4)
        {
            status += fx_file_close(&my_file4);
            status += fx_file_delete(&ram_disk, "TEST2.TXT");
        }
    
        /* Write to the writable file.  */
        status +=  fx_file_write(&my_file2, my_buffer, 128);
        status +=  fx_file_write(&my_file2, my_buffer, 128);
        
        /* Read the data in the file from the other 2 file handles.  */
        status +=  fx_file_read(&my_file1, my_buffer, 128, &actual1);
        status +=  fx_file_read(&my_file, my_buffer, 128, &actual2);
        status +=  fx_file_read(&my_file1, my_buffer, 128, &actual1);
        status +=  fx_file_read(&my_file, my_buffer, 128, &actual2);
        
        /* Check the status.  */
        if (status)
            break;
        i++;
    }

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(43);
    }

    /* Now force a wrap of the FAT search.  */
    ram_disk.fx_media_available_clusters++;
    
    status =  fx_file_write(&my_file2, my_buffer, 128);
    
    /* Did we get an error?  */
    if (status != FX_NO_MORE_SPACE)
    {
    
        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(44);    
    }
    
    /* Close the files and the media.  */
    status = fx_file_close(&my_file);
    status += fx_file_close(&my_file1);
    status += fx_file_close(&my_file2);
    status += fx_media_close(&ram_disk);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(43);
    }

    /* Test the update of files open for reading while the write is happening - with random errors!  */ 

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
                            6000,                   // Total sectors - FAT16
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(44);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(45);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    fx_file_create(&ram_disk, "TEST.TXT");
    fx_file_create(&ram_disk, "TEST1.TXT");
    fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_READ);
    fx_file_open(&ram_disk, &my_file1, "TEST.TXT", FX_OPEN_FOR_READ);
    fx_file_open(&ram_disk, &my_file2, "TEST.TXT", FX_OPEN_FOR_WRITE);
    fx_file_open(&ram_disk, &my_file3, "TEST1.TXT", FX_OPEN_FOR_WRITE);  
    
    /* Now loop through the maximum of clusters to fill and read the file.  */
    i =  0;
    while (ram_disk.fx_media_available_clusters)
    {
     
        /* Flush the media.  */
        fx_media_flush(&ram_disk);
        _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
        
        /* Setup the random I/O Error.  */
        _fx_ram_driver_io_error_request =  (rand() & 4);
        if (_fx_ram_driver_io_error_request == 0)
            _fx_ram_driver_io_error_request = 1;
        _fx_utility_fat_entry_write_error_request =  _fx_ram_driver_io_error_request;
        _fx_utility_fat_entry_read_error_request =  _fx_ram_driver_io_error_request;
        _fx_utility_logical_sector_write_error_request =  _fx_ram_driver_io_error_request;
        _fx_utility_logical_sector_read_error_request =  _fx_ram_driver_io_error_request;
               
        /* Write to the writable file.  */
        if (i & 1)
        {
            fx_file_write(&my_file2, my_buffer, 128);
        }
        else
        {
            fx_file_write(&my_file2, my_buffer, 32);
            fx_file_write(&my_file2, my_buffer, 32);
            fx_file_write(&my_file2, my_buffer, 32);
            fx_file_write(&my_file2, my_buffer, 32);        
        }

        /* Setup the random I/O Error.  */
        _fx_ram_driver_io_error_request =  (rand() & 4);
        if (_fx_ram_driver_io_error_request == 0)
            _fx_ram_driver_io_error_request = 1;
        _fx_utility_fat_entry_write_error_request =  _fx_ram_driver_io_error_request;
        _fx_utility_fat_entry_read_error_request =  _fx_ram_driver_io_error_request;
        _fx_utility_logical_sector_write_error_request =  _fx_ram_driver_io_error_request;
        _fx_utility_logical_sector_read_error_request =  _fx_ram_driver_io_error_request;
        
        /* Read the data in the file from the other 2 file handles.  */
        if (i & 1)
        {
            fx_file_read(&my_file1, my_buffer, 128, &actual1);
        }
        else
        {
            fx_file_read(&my_file1, my_buffer, 32, &actual1);
            fx_file_read(&my_file1, my_buffer, 32, &actual1);
            fx_file_read(&my_file1, my_buffer, 32, &actual1);
            fx_file_read(&my_file1, my_buffer, 32, &actual1);        
        }

        /* Setup the random I/O Error.  */
        _fx_ram_driver_io_error_request =  (rand() & 4);
        if (_fx_ram_driver_io_error_request == 0)
            _fx_ram_driver_io_error_request = 1;
        _fx_utility_fat_entry_write_error_request =  _fx_ram_driver_io_error_request;
        _fx_utility_fat_entry_read_error_request =  _fx_ram_driver_io_error_request;
        _fx_utility_logical_sector_write_error_request =  _fx_ram_driver_io_error_request;
        _fx_utility_logical_sector_read_error_request =  _fx_ram_driver_io_error_request;

        fx_file_read(&my_file, my_buffer, 128, &actual2);

        _fx_ram_driver_io_error_request = 0;
        _fx_utility_fat_entry_write_error_request =  _fx_ram_driver_io_error_request;
        _fx_utility_fat_entry_read_error_request =  _fx_ram_driver_io_error_request;
        _fx_utility_logical_sector_write_error_request =  _fx_ram_driver_io_error_request;
        _fx_utility_logical_sector_read_error_request =  _fx_ram_driver_io_error_request;

        i++;
    }
    
    /* Close the files and the media.  */
    fx_file_close(&my_file);
    fx_file_close(&my_file1);
    fx_file_close(&my_file2);
    fx_media_close(&ram_disk);


    /* Test the write of partial cluster and direct I/O over non-contigous clusters.  */ 


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
                            6000,                   // Total sectors - FAT16
                            128,                    // Sector size   
                            3,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(46);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(47);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_create(&ram_disk, "TEST1.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &my_file4,"TEST1.TXT", FX_OPEN_FOR_WRITE);
    
    /* Write a small amount of data first to cause an unalignment and partial sector write.  */
    status += fx_file_write(&my_file, my_buffer, 32);
    status += fx_file_write(&my_file4, my_buffer, 32);
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);   
    
    /* Now loop through the maximum of clusters to fill up the rest of the FAT table.  */
    i =  0;
    while (ram_disk.fx_media_available_clusters)
    {
    
    
        /* Write to the writable file.  */
        status +=  fx_file_write(&my_file, my_buffer, 32);
        status +=  fx_file_write(&my_file4, my_buffer, 32);
        i++;
    }

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(48);
    }

    /* Now read the file in as big of chunks as possible to excersise the partial read paths.  */
    status =  fx_file_seek(&my_file, 0);
    do
    {
        /* Read chunks of the file.  */
        status =  fx_file_read(&my_file, my_buffer, 128*4, &actual);
    
    } while (status != FX_END_OF_FILE);
   

    /* Now delete the second file to leave holes in the FAT table.  */
    status += fx_file_close(&my_file4);
    status += fx_file_delete(&ram_disk, "TEST1.TXT");

    /* Finally, perform a direct write with that can't be done with contigous clusters.  */   
    status =  fx_file_write(&my_file, my_buffer, 128*4);
    
    /* Did we get an error?  */
    if (status != FX_SUCCESS)
    {
    
        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(49);    
    }
    
    /* Close the files and the media.  */
    status = fx_file_close(&my_file);
    status += fx_media_close(&ram_disk);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(50);
    }


    /* Test the write of partial cluster and direct I/O when the FAT chain is broken.  */ 

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
                            14000,                   // Total sectors - FAT16
                            128,                    // Sector size   
                            2,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(51);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(52);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    
    /* Write a data first to build a FAT chain.  */
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);    
    status += fx_file_seek(&my_file, 0);
    status += fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }
    
    /* Read the first FAT sector.  */
    status += fx_media_read(&ram_disk, 1, (VOID *) fat_buffer);  

    /* Add a FAT entry randomly in the FAT table.  */
    fat_buffer[6] =  1;
    fat_buffer[7] =  0;
    fat_buffer[8] =  1;
    fat_buffer[9] =  0;

    /* Write the FAT corruption out.  */
    status += fx_media_write(&ram_disk, 1, (VOID *) fat_buffer);

    /* See if we are okay.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(53);
    }

    /* Now attempt to write a partial sector.  */
    status =  fx_file_write(&my_file, my_buffer, 128*4);
    
    /* See if we get the file corrupt error.  */
    if (status != FX_FILE_CORRUPT)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(54);
    }
    
    status =  fx_file_write(&my_file, my_buffer, 128*4);
    status +=  fx_file_write(&my_file, my_buffer, 128*4);

    /* See if we get the file corrupt error.  */
    if (status != FX_FILE_CORRUPT)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(55);
    }
    
   
    /* Close the files and the media.  */
    status = fx_file_close(&my_file);
    status += fx_media_close(&ram_disk);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(56);
    }

    /* Test the maximum write size.  */ 

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
                            1000,                   // Total sectors - FAT12
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(57);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(58);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    
    /* Write a data first to build a FAT chain.  */
    status += fx_file_write(&my_file, my_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(59);
    }

    /* Now manually setup the file offset to force an error.  */ 
    temp =  (ULONG)my_file.fx_file_current_file_offset;
    my_file.fx_file_current_file_offset =  0xFFFFFFF0;

    /* Write a data first to force the overflow.  */
    status = fx_file_write(&my_file, my_buffer, 32);

    /* Check the status.  */
    if (status != FX_NO_MORE_SPACE)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(60);
    }

    /* Also save, adjust the current available so that one new cluster will be written.  */
    my_file.fx_file_current_available_size = 0xFFFFFFF0;
    temp1 = (ULONG)my_file.fx_file_current_available_size;
    
    /* Now attempt to write a massive file to exercise the maximum available file size logic.  */
    status = fx_file_write(&my_file, my_buffer, 1);
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(61);
    }
    
    /* Now restore the original offset.  */
    my_file.fx_file_current_file_offset =  temp;
    my_file.fx_file_current_available_size =  temp1;
    
    /* Close the file.  */
    status =  fx_file_close(&my_file);
    status += fx_media_close(&ram_disk);
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(62);
    }


    /* Test the remaining I/O error paths in file write.  */ 

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
                            1000,                   // Total sectors - FAT12
                            128,                    // Sector size   
                            3,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(63);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(64);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status = fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(65);
    }

    /* Write a data first to build a FAT chain, with an I/O error on liking the FAT chain.  */
    _fx_utility_fat_entry_write_error_request =  1;
    status = fx_file_write(&my_file, my_buffer, 128*4);
    _fx_utility_fat_entry_write_error_request =  0;

    /* Check the status.  */
    if (status != FX_IO_ERROR)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(66);
    }
    
    /* Close the file.  */
    status =  fx_file_close(&my_file);
    status += fx_media_close(&ram_disk);
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(67);
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
                            1000,                   // Total sectors - FAT12
                            128,                    // Sector size   
                            3,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(68);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(69);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status = fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(70);
    }

    /* Write a data first to build a FAT chain, with an I/O error on writing EOF at the end of the FAT chain.  */
    _fx_utility_fat_entry_write_error_request =  2;
    status = fx_file_write(&my_file, my_buffer, 128*4);
    _fx_utility_fat_entry_write_error_request =  0;

    /* Check the status.  */
    if (status != FX_IO_ERROR)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(71);
    }
    
    /* Close the file.  */
    status =  fx_file_close(&my_file);
    status += fx_media_close(&ram_disk);
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(72);
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
                            1000,                   // Total sectors - FAT12
                            128,                    // Sector size   
                            3,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(73);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(74);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status = fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, my_buffer, 128*4);
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(75);
    }

    /* Write a data first to build a FAT chain, with an I/O error on linking the new FAT chain to existing FAT chain.  */
    _fx_utility_fat_entry_write_error_request =  2;
    status = fx_file_write(&my_file, my_buffer, 128*4);
    _fx_utility_fat_entry_write_error_request =  0;

    /* Check the status.  */
    if (status != FX_IO_ERROR)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(76);
    }
    
    /* Close the file.  */
    status =  fx_file_close(&my_file);
    status += fx_media_close(&ram_disk);
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(77);
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
                            1000,                   // Total sectors - FAT12
                            128,                    // Sector size   
                            3,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(78);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(79);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status = fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, my_buffer, 128+64);
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(80);
    }

    /* Write data that will require another cluster, even though we have only written one sector of the first allocated cluster.  */
    status = fx_file_write(&my_file, my_buffer, 128*4);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(81);
    }
    
    /* Close the file.  */
    status =  fx_file_close(&my_file);
    status += fx_media_close(&ram_disk);
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(82);
    }   

    /* Test the read of partial cluster and direct I/O when the FAT chain is broken.  */ 

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
                            14000,                   // Total sectors - FAT16
                            128,                    // Sector size   
                            2,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(83);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(84);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    
    /* Write a data first to build a FAT chain.  */
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);    
    status += fx_file_seek(&my_file, 0);
    status += fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }
    
    /* Read the first FAT sector.  */
    status += fx_media_read(&ram_disk, 1, (VOID *) fat_buffer);  

    /* Add a FAT entry randomly in the FAT table.  */
    fat_buffer[6] =  1;
    fat_buffer[7] =  0;
    fat_buffer[8] =  1;
    fat_buffer[9] =  0;

    /* Write the FAT corruption out.  */
    status += fx_media_write(&ram_disk, 1, (VOID *) fat_buffer);

    /* See if we are okay.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(85);
    }

    /* Read the file to get the file corruption error.  */
    status =  fx_file_read(&my_file, my_buffer, 128*4, &actual);

    /* See if the file is corrupted - it should be.  */
    if (status != FX_FILE_CORRUPT)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(86);
    }
    
    /* Close everything down.  */
    fx_file_close(&my_file);
    fx_media_abort(&ram_disk);    

    /* Test the read of partial cluster and direct I/O when the FAT chain is broken - at the beginning of the FAT chain!  */ 

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
                            14000,                   // Total sectors - FAT16
                            128,                    // Sector size   
                            2,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(87);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(88);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    
    /* Write a data first to build a FAT chain.  */
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);    
    status += fx_file_seek(&my_file, 0);
    status += fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }
    
    /* Read the first FAT sector.  */
    status += fx_media_read(&ram_disk, 1, (VOID *) fat_buffer);  

    /* Add a FAT entry randomly in the FAT table.  */
    fat_buffer[4] =  1;
    fat_buffer[5] =  0;
    fat_buffer[6] =  1;
    fat_buffer[7] =  0;
    fat_buffer[8] =  1;
    fat_buffer[9] =  0;

    /* Write the FAT corruption out.  */
    status += fx_media_write(&ram_disk, 1, (VOID *) fat_buffer);

    /* See if we are okay.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(89);
    }

    /* Read the file to get the file corruption error.  */
    status =  fx_file_read(&my_file, my_buffer, 128*4, &actual);

    /* See if the file is corrupted - it should be.  */
    if (status != FX_FILE_CORRUPT)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(90);
    }
    
    /* Close everything down.  */
    fx_file_close(&my_file);
    fx_media_abort(&ram_disk);    


    /* Test the read of partial cluster and direct I/O when the FAT chain is broken - at the beginning of the FAT chain and with FFs instead of 1!  */ 

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
                            14000,                   // Total sectors - FAT16
                            128,                    // Sector size   
                            2,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(91);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(92);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    
    /* Write a data first to build a FAT chain.  */
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);    
    status += fx_file_seek(&my_file, 0);
    status += fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }
    
    /* Read the first FAT sector.  */
    status += fx_media_read(&ram_disk, 1, (VOID *) fat_buffer);  

    /* Add a FAT entry randomly in the FAT table.  */
    fat_buffer[4] =  0xFF;
    fat_buffer[5] =  0xFF;
    fat_buffer[6] =  0xFF;
    fat_buffer[7] =  0xFF;
    fat_buffer[8] =  0xFF;
    fat_buffer[9] =  0xFF;

    /* Write the FAT corruption out.  */
    status += fx_media_write(&ram_disk, 1, (VOID *) fat_buffer);

    /* See if we are okay.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(93);
    }

    /* Read the file to get the file corruption error.  */
    status =  fx_file_read(&my_file, my_buffer, 128*4, &actual);

    /* See if the file is corrupted - it should be.  */
    if (status != FX_FILE_CORRUPT)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(94);
    }
    
    /* Close everything down.  */
    fx_file_close(&my_file);
    fx_media_abort(&ram_disk);    


    /* Test the read with I/O FAT read error.  */ 

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
                            14000,                   // Total sectors - FAT16
                            128,                    // Sector size   
                            2,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(95);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(96);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    
    /* Write a data first to build a FAT chain.  */
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);    
    status += fx_file_seek(&my_file, 0);
    status += fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }
    
    /* Read the file to get the file IO error.  */
    _fx_utility_fat_entry_read_error_request =  1;
    status =  fx_file_read(&my_file, my_buffer, 128*4, &actual);
    _fx_utility_fat_entry_read_error_request =  0;

    /* See if the file is corrupted - it should be.  */
    if (status != FX_IO_ERROR)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(97);
    }
    
    /* Close everything down.  */
    fx_file_close(&my_file);
    fx_media_abort(&ram_disk);    


    /* Test the read with I/O logical sector read error.  */ 

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
                            14000,                   // Total sectors - FAT16
                            128,                    // Sector size   
                            2,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(95);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(96);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    
    /* Write a data first to build a FAT chain.  */
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);    
    status += fx_file_seek(&my_file, 0);
    status += fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }
    
    /* Read the file to get the file IO error.  */
    _fx_ram_driver_io_error_request =  2;
    status =  fx_file_read(&my_file, my_buffer, 128*4, &actual);
    _fx_ram_driver_io_error_request =  0;

    /* See if the file is corrupted - it should be.  */
    if (status != FX_IO_ERROR)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(97);
    }
    
    /* Close everything down.  */
    fx_file_close(&my_file);
    fx_media_abort(&ram_disk);    


    /* Test the file open with I/O read errors when walking the FAT chain.  */ 

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
                            7000,                   // Total sectors - FAT16
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(98);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(99);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_create(&ram_disk, "TEST1.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &my_file1, "TEST1.TXT", FX_OPEN_FOR_WRITE);
    
    /* Write a data first to build a FAT chain.  */
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);
    status += fx_file_write(&my_file, my_buffer, 128*4);    
    status += fx_file_write(&my_file1, my_buffer, 128*4);    
    status += fx_file_write(&my_file, my_buffer, 128*4);    
    
    /* Close the file.  */
    status += fx_file_close(&my_file);
    status += fx_file_close(&my_file1);

    /* Check status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(100);
    }

    /* Now flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }
    
    
    /* Now open the file with an I/O error on the FAT entry read when walking the FAT chain.  */
    _fx_utility_fat_entry_read_error_request =  1;
    status = fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    _fx_utility_fat_entry_read_error_request =  0;
    
    /* See if we got the I/O Error.  */
    if (status != FX_IO_ERROR)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(101);
    }
    
    /* Now break the FAT chain and try to open the file.  */

    /* Now flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) fat_buffer);  

    /* Add a FAT entry randomly in the FAT table.  */
    fat_buffer[4] =  2;

    /* Write the FAT corruption out.  */
    status += fx_media_write(&ram_disk, 1, (VOID *) fat_buffer);

    /* See if we are okay.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(102);
    }

    /* Now open the file with a corrupted FAT entry which will cause an error when walking the FAT chain.  */
    status = fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    
    /* See if we got the FAT chain.  */
    if (status != FX_FAT_READ_ERROR)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(103);
    }

    /* Now flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) fat_buffer);  

    /* Add a FAT entry randomly in the FAT table.  */
    fat_buffer[4] =  0xF0;
    fat_buffer[5] =  0xFF;

    /* Write the FAT corruption out.  */
    status += fx_media_write(&ram_disk, 1, (VOID *) fat_buffer);

    /* See if we are okay.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(104);
    }
  
    /* Now open the file with a corrupted FAT entry which will cause an error when walking the FAT chain.  */
    status = fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    
    /* See if we got the file corrupt error.  */
    if (status != FX_FILE_CORRUPT)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(105);
    }

    /* Now flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) fat_buffer);  

    /* Fix the FAT chain.  */
    fat_buffer[4] =  3;
    fat_buffer[5] =  0;

    /* Write the FAT corruption out.  */
    status += fx_media_write(&ram_disk, 1, (VOID *) fat_buffer);

    /* See if we are okay.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(106);
    }

    /* Now test the total clusters check when traversing the FAT chain.  */
    temp =  ram_disk.fx_media_total_clusters;
    ram_disk.fx_media_total_clusters =  4;
    status = fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    ram_disk.fx_media_total_clusters =  temp;
    
    /* Check status.  */
    if (status != FX_FAT_READ_ERROR)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(107);
    }

    /* Now open the file with a good FAT chain.  */
    status = fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    
    /* Check status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(108);
    }
    
    /* Now allocate another cluster to the end of the file.  */
    status =  fx_file_allocate(&my_file, 256);
    status += fx_file_close(&my_file);
    
    /* Check status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(109);
    }
    
    /* Now open the file again with an extra cluster at the end of the file...  */
    status = fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    
    /* Check status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(110);
    }   
    
    /* Close the file again.  */
    fx_file_close(&my_file);
    
    /* Now flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) fat_buffer);  

    /* Fix the FAT chain.  */
    fat_buffer[4] =  0;
    fat_buffer[5] =  0;

    /* Write the FAT corruption out.  */
    status += fx_media_write(&ram_disk, 1, (VOID *) fat_buffer);

    /* See if we are okay.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(111);
    }

    /* Now open the file again with an invalid last cluster at the end of the file...  */
    status = fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    
    /* Check status.  */
    if (status != FX_FILE_CORRUPT)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(112);
    }   

    /* Now flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) fat_buffer);  

    /* Fix the FAT chain.  */
    fat_buffer[4] =  3;
    fat_buffer[5] =  0;
    fat_buffer[0x32] =  0;
    fat_buffer[0x33] =  0;

    /* Write the FAT corruption out.  */
    fx_media_write(&ram_disk, 1, (VOID *) fat_buffer);

    /* Now open the file again with an invalid last cluster at the end of the file...  */
    status = fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    
    /* Check status.  */
    if (status != FX_FILE_CORRUPT)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(113);
    }   

    /* Now flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) fat_buffer);  

    /* Fix the FAT chain.  */
    fat_buffer[0x32] =  0xff;
    fat_buffer[0x33] =  0xff;

    /* Write the FAT corruption out.  */
    fx_media_write(&ram_disk, 1, (VOID *) fat_buffer);

    /* Open the file.  */
    status = fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, buffer, 128*3);
    status += fx_file_seek(&my_file, 0);
    
    /* Set the flag to cause the cache update to bypass.  */
    _fx_utility_logical_sector_read_1_error_request =  1;
    status += fx_file_read(&my_file, buffer, 128*3, &actual);
    _fx_utility_logical_sector_read_1_error_request =  0;

    /* Set the flag to cause the cache update to bypass - this time with a larger error value to make sure that path is taken.  */
    _fx_utility_logical_sector_read_1_error_request =  1000000;
    status += fx_file_read(&my_file, buffer, 128*3, &actual);
    _fx_utility_logical_sector_read_1_error_request =  0;
    
#ifndef FX_ENABLE_FAULT_TOLERANT
    /* Call the logical sector flush with a large error value to get that path taken as well.  */
    _fx_utility_logical_sector_flush_error_request =   1000000;
    status +=  _fx_utility_logical_sector_flush(&ram_disk, 0, 60000, FX_FALSE);
    _fx_utility_logical_sector_flush_error_request =   0;
    
    /* Check status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(116);
    }   
#endif

    /* Close everything down.  */
    fx_file_close(&my_file);
    fx_media_abort(&ram_disk);    

    /* FAT32. */
    status = fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            70000,                  // Total sectors - FAT16
                            512,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_file_create(&ram_disk, "TEST.TXT");
    return_if_fail(FX_SUCCESS == status);

    status = fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_allocate(&my_file, 512);
    status += fx_file_close(&my_file);
    return_if_fail(FX_SUCCESS == status);

    /* Open the file with a bad FAT chain. */
    _fx_utility_FAT_entry_write(&ram_disk, 3, 0);
    status = fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    return_if_fail(FX_FILE_CORRUPT == status);

    fx_file_close(&my_file);
    fx_media_close(&ram_disk);

    printf("SUCCESS!\n");
    test_control_return(0);
}

