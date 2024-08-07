/* This FileX test concentrates on the file read/write operation.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_ram_driver_test.h"
#include   <stdio.h>

#ifdef FX_ENABLE_EXFAT

#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              128*128


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD               ftest_0;
#endif
static FX_MEDIA                ram_disk;
static FX_FILE                 my_file;
static ULONG                   my_buffer[128];

/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *ram_disk_memory;
static UCHAR                  *cache_buffer;
#else
static UCHAR                   cache_buffer[CACHE_SIZE];
#endif


/* Define thread prototypes.  */

void    filex_file_read_write_exfat_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_file_read_write_exfat_application_define(void *first_unused_memory)
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
ULONG       i, j;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   exFAT File read/write test.............................");

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
        test_control_return(21);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
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

    /* Pickup the available bytes in the media.  */
    status =  fx_media_space_available(&ram_disk, &available_bytes);
    
    /* Check for available bytes error.  */
    if ((status != FX_SUCCESS) || (available_bytes < sizeof(ULONG)))
    {

        printf("ERROR!\n");
        test_control_return(5);
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
        test_control_return(13);
    }

    /* Open the ram_disk, but do so to ensure non-hashed algorithm is used by supplying CACHE_SIZE-1.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE-1);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR! %d\n",status);
        test_control_return(14);
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
    else
    {

        printf("SUCCESS!\n");
        test_control_return(0);
    }
}

#endif /* FX_ENABLE_EXFAT */
