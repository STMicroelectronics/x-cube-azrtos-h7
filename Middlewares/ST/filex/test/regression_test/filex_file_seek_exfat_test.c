/* This FileX test concentrates on the file seek operation.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"
#include   "fx_file.h"

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

void    filex_file_seek_exfat_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_file_seek_exfat_application_define(void *first_unused_memory)
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
    printf("FileX Test:   exFAT File seek test...................................");

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
    
    /* Check for invalidate errors.  */
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
            test_control_return(9);
        }
    }
#endif
    
    /* Seek to the beginning of the test file.  */
    status =  fx_file_seek(&my_file, 0);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(10);
    }

    /* Read the 4 bytes at the front of the file... should be 0!  */
    status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != 0) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(11);
    }

    /* Read the next 4 bytes at the front of the file... should be 1!  */
    status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != 1) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(12);
    }

    /* Seek to near the last 4 bytes of the file.  */
    status =  fx_file_seek(&my_file, available_bytes - 4);
    
    /* Read the last 4 bytes of the file...  should be available_bytes/sizeof(ULONG)!  */
    status +=  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != (available_bytes/sizeof(ULONG) - 1)) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(13);
    }
    
    /* Read the past the end of the file...  should get an error in this case.   */
    status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if (status != FX_END_OF_FILE)
    {
    
        printf("ERROR!\n");
        test_control_return(14);
    }

    /* Seek to the middle of the file.  */
    status =  fx_file_seek(&my_file, available_bytes/2);
    
    /* Read the middle 4 bytes of the file...  should be (available_bytes/2)/sizeof(ULONG)!  */
    status +=  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != (available_bytes/(2*sizeof(ULONG)))) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(15);
    }

    /* Seek to the end of the file.  */
    status =  fx_file_seek(&my_file, 0xFFFFFFFF);

    /* Determine if it is correct.  */
    if (status != FX_SUCCESS)
    {
    
        printf("ERROR!\n");
        test_control_return(16);
    }

    /* Close the file.  */
    status +=  fx_file_close(&my_file);

    /* Open the test file for fast reading.  */
    status +=  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_READ_FAST);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(17);
    }

    /* Read the 4 bytes at the front of the file... should be 0!  */
    status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != 0) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(18);
    }

    /* Read the next 4 bytes at the front of the file... should be 1!  */
    status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != 1) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(19);
    }

    /* Seek to near the last 4 bytes of the file.  */
    status =  fx_file_seek(&my_file, available_bytes - 4);
    
    /* Read the last 4 bytes of the file...  should be available_bytes/sizeof(ULONG)!  */
    status +=  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != (available_bytes/sizeof(ULONG) - 1)) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(20);
    }
    
    /* Read the past the end of the file...  should get an error in this case.   */
    status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if (status != FX_END_OF_FILE)
    {
    
        printf("ERROR!\n");
        test_control_return(21);
    }

    /* Seek to the middle of the file.  */
    status =  fx_file_seek(&my_file, available_bytes/2);
    
    /* Read the middle 4 bytes of the file...  should be (available_bytes/2)/sizeof(ULONG)!  */
    status +=  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != (available_bytes/(2*sizeof(ULONG)))) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(22);
    }

    /* Seek to the end of the file.  */
    status =  fx_file_seek(&my_file, 0xFFFFFFFF);

    /* Determine if it is correct.  */
    if (status != FX_SUCCESS)
    {
    
        printf("ERROR!\n");
        test_control_return(23);
    }

    /* Close the file.  */
    status +=  fx_file_close(&my_file);

    /* Open the test file for reading.  */
    status +=  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_READ);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(24);
    }

    /* Read the 4 bytes at the front of the file... should be 0!  */
    status =  fx_file_relative_seek(&my_file, 0, FX_SEEK_BEGIN);
    status +=  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != 0) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(25);
    }

    /* Read the next 4 bytes at the front of the file... should be 1!  */
    status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if ((status) || (read_value != 1) || (actual != sizeof(ULONG)))
    {
    
        printf("ERROR!\n");
        test_control_return(26);
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
        test_control_return(27);
    }
    
    /* Read the past the end of the file...  should get an error in this case.   */
    status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);
    
    /* Determine if it is correct.  */
    if (status != FX_END_OF_FILE)
    {
    
        printf("ERROR!\n");
        test_control_return(28);
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
        test_control_return(29);
    }

    /* Seek to the end of the file.  */
    status =  fx_file_relative_seek(&my_file, 0, FX_SEEK_END);
 
    /* Determine if it is correct.  */
    if (status != FX_SUCCESS)
    {
    
        printf("ERROR!\n");
        test_control_return(30);
    }

    /* Close the file.  */
    status +=  fx_file_close(&my_file);

    /* Close the media.  */
    status +=  fx_media_close(&ram_disk);   

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(31);
    }
    
    /* Test to verify relative clustor calculation on larger file size, 
       FX_MEDIA id configured to simulate the the seek operation for larger size address*/
  
    
    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_exFAT_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory_large,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            0,                      // Hidden sectors
                            42000,                    // Total sectors 
                            FX_EXFAT_SECTOR_SIZE,   // Sector size   
                            64,                      // exFAT Sectors per cluster
                            12345,                  // Volume ID
                            0);                     // Boundary unit


    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(32);
    }
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(33);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(34);
    }

    /* Open the test file.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(35);
    }

    ULONG64 seek_offset[2];

    ULONG64 relative_cluster[2];
    seek_offset[0] = 0x000001000ULL;
    seek_offset[1] = 0x100001000ULL;
    
    /* Set the below parameters to access the else condition for directly access the desired cluster  Line 284
        file_ptr -> fx_file_current_relative_cluster = (ULONG)byte_offset / bytes_per_cluster;                         */ 
    my_file.fx_file_current_file_size = 5368709120; 
    my_file.fx_file_consecutive_cluster =163840;
    
    for(i=0; i<2; i++)
    {
        /* Seek to the beginning of the test file.  */
        status =  _fx_file_extended_seek(&my_file, seek_offset[i]);

        /* Check the file seek status.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(36);
        }
        
        relative_cluster[i] = my_file.fx_file_current_relative_cluster;
    
    }
  
    if(relative_cluster[0]==relative_cluster[1])
    {
        printf("ERROR!\n");
        test_control_return(37);
    }
    
    /* Open the file for read */
    status = fx_file_close(&my_file);
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(38);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);   

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(39);
    }
    else
    {

        printf("SUCCESS!\n");
        test_control_return(0);
    }
}

#endif /* FX_ENABLE_EXFAT */
