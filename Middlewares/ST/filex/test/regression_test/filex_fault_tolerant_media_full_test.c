/* This FileX test concentrates on the Fault-Tolerant Media Full test.  */
/*          
For FAT 12, 16, 32 and exFAT, one cluster size is 1024 bytes;
Check media full operation:          
Step1: Format and open the media; 
Step2: Enable fault tolerant feature;   
Step3: Create new file called "TEST.TXT";    
Step4: Get the media available bytes;    
Step5: Loop to write bytes out of the file to fill the media;
Step6: Check the media available bytes. 
Step7: Loop to read and check the file data from the media;
*/ 
#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_fault_tolerant.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"               
extern void    test_control_return(UINT status);
void    filex_fault_tolerant_media_full_test_application_define(void *first_unused_memory);
                                         
#if defined (FX_ENABLE_FAULT_TOLERANT) && defined (FX_FAULT_TOLERANT)

#define DEMO_STACK_SIZE         4096
#ifdef FX_ENABLE_EXFAT
#define CACHE_SIZE                  FX_EXFAT_SECTOR_SIZE
#define FAULT_TOLERANT_SIZE         FX_EXFAT_SECTOR_SIZE
#else
#define CACHE_SIZE                  2048
#define FAULT_TOLERANT_SIZE         FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE
#endif


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD                ftest_0;  
#endif
static FX_MEDIA                 ram_disk;
static FX_FILE                  my_file;
static UCHAR                    *pointer;

/* Define the counters used in the test application...  */
                                                          
#ifndef FX_STANDALONE_ENABLE
static UCHAR                    *cache_buffer;
static UCHAR                    *fault_tolerant_buffer;
#else
static UCHAR                    cache_buffer[CACHE_SIZE];
static UCHAR                    fault_tolerant_buffer[FAULT_TOLERANT_SIZE];
#endif 

#ifdef FX_ENABLE_EXFAT
#define TEST_COUNT              4
#else              
#define TEST_COUNT              3
#endif

/* Define thread prototypes.  */

static void    ftest_0_entry(ULONG thread_input);     
extern void    _fx_ram_driver(FX_MEDIA *media_ptr);
extern void    test_control_return(UINT status);           


/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_media_full_test_application_define(void *first_unused_memory)
#endif
{

    
#ifndef FX_STANDALONE_ENABLE
    /* Setup the working pointer.  */
    pointer =  (UCHAR *) first_unused_memory;

    /* Create the main thread.  */

    tx_thread_create(&ftest_0, "thread 0", ftest_0_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer =  pointer + DEMO_STACK_SIZE;

    /* Setup memory for the RAM disk and the sector cache.  */    
    cache_buffer =  pointer;
    pointer += CACHE_SIZE;
    fault_tolerant_buffer = pointer;
    pointer += FAULT_TOLERANT_SIZE;
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
    printf("FileX Test:   Fault Tolerant Media Full Test.........................");
                      
    /* Loop to test FAT 12, 16, 32 and exFAT.   */
    for (i = 0; i < TEST_COUNT; i ++)
    {
        if (i == 0)
        {
            /* Format the media with FAT12.  This needs to be done before opening it!  */
            status =  fx_media_format(&ram_disk, 
                                     _fx_ram_driver,         // Driver entry
                                     ram_disk_memory_large,  // RAM disk memory pointer
                                     cache_buffer,           // Media buffer pointer
                                     CACHE_SIZE,             // Media buffer size 
                                     "MY_RAM_DISK",          // Volume Name
                                     1,                      // Number of FATs
                                     32,                     // Directory Entries
                                     0,                      // Hidden sectors
                                     256,                    // Total sectors 
                                     256,                    // Sector size   
                                     8,                      // Sectors per cluster
                                     1,                      // Heads
                                     1);                     // Sectors per track 
        }     
        else if (i == 1)
        {
            /* Format the media with FAT16.  This needs to be done before opening it!  */
            status =  fx_media_format(&ram_disk, 
                                     _fx_ram_driver,         // Driver entry            
                                     ram_disk_memory_large,  // RAM disk memory pointer
                                     cache_buffer,           // Media buffer pointer
                                     CACHE_SIZE,             // Media buffer size 
                                     "MY_RAM_DISK",          // Volume Name
                                     1,                      // Number of FATs
                                     32,                     // Directory Entries
                                     0,                      // Hidden sectors
                                     4200 * 8,               // Total sectors 
                                     256,                    // Sector size   
                                     8,                      // Sectors per cluster
                                     1,                      // Heads
                                     1);                     // Sectors per track 
        }  
        else if (i == 2)
        {
            /* Format the media with FAT32.  This needs to be done before opening it!  */
            status =  fx_media_format(&ram_disk, 
                                     _fx_ram_driver,         // Driver entry            
                                     ram_disk_memory_large,  // RAM disk memory pointer
                                     cache_buffer,           // Media buffer pointer
                                     CACHE_SIZE,             // Media buffer size 
                                     "MY_RAM_DISK",          // Volume Name
                                     1,                      // Number of FATs
                                     32,                     // Directory Entries
                                     0,                      // Hidden sectors
                                     70000 * 8,              // Total sectors 
                                     256,                    // Sector size   
                                     8,                      // Sectors per cluster
                                     1,                      // Heads
                                     1);                     // Sectors per track 
        }  
#ifdef FX_ENABLE_EXFAT
        else
        {

            /* Format the media with exFAT.  This needs to be done before opening it!  */
            status =  fx_media_exFAT_format(&ram_disk, 
                                            _fx_ram_driver,         // Driver entry            
                                            ram_disk_memory_large,  // RAM disk memory pointer
                                            cache_buffer,           // Media buffer pointer
                                            CACHE_SIZE,             // Media buffer size 
                                            "MY_RAM_DISK",          // Volume Name
                                            1,                      // Number of FATs
                                            0,                      // Hidden sectors
                                            256,                    // Total sectors 
                                            FX_EXFAT_SECTOR_SIZE,   // Sector size
                                            4,                      // exFAT Sectors per cluster
                                            12345,                  // Volume ID
                                            0);                     // Boundary unit
        }
#endif

        return_if_fail( status == FX_SUCCESS);

        /* Open the ram_disk.  */
        status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
        return_if_fail( status == FX_SUCCESS);
    
        /* Enable the Fault-tolerant feature.  */                             
        status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
        return_if_fail( status == FX_SUCCESS);
                                
        /* Create a file called TEST.TXT in the root directory.  */
        status =  fx_file_create(&ram_disk, "TEST.TXT");
        return_if_fail( status == FX_SUCCESS);

        /* Open the test file.  */
        status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
        return_if_fail( status == FX_SUCCESS);

        /* Pickup the available bytes in the media.  */
        status =  fx_media_space_available(&ram_disk, &available_bytes);
        return_if_fail( (status == FX_SUCCESS) && (available_bytes >= sizeof(ULONG)));

        /* Loop to write successive bytes out to the file.... to fill the media!  */
        j =  0;
        write_value =  0;
        while (j < available_bytes)
        {

            /* Write 4 bytes to the file.  */
            status =  fx_file_write(&my_file, (void *) &write_value, sizeof(ULONG));
            return_if_fail( status == FX_SUCCESS);

            /* Increment byte count.  */
            j =  j + sizeof(ULONG);

            /* Increment write value.  */
            write_value++;
        }

        /* Pickup the available bytes in the media again.  */
        status =  fx_media_space_available(&ram_disk, &j);
        return_if_fail( (status == FX_SUCCESS) && (j == 0));

#ifndef FX_DISABLE_CACHE
        /* At this point, we should invalidate the (which also flushes the cache) media to ensure that all 
        dirty sectors are written.  */
        status =  fx_media_cache_invalidate(&ram_disk);
        return_if_fail((status == FX_SUCCESS) && (ram_disk.fx_media_sector_cache_dirty_count == 0));

        /* See if any sectors are still valid in the cache.  */
        for (j = 0; j < ram_disk.fx_media_sector_cache_size; j++)
        {

            /* Determine if this cache entry is still valid.  */
            return_if_fail(ram_disk.fx_media_sector_cache[j].fx_cached_sector_valid == 0);
        }
#endif /* FX_DISABLE_CACHE */

        /* Seek to the beginning of the test file.  */
        status =  fx_file_seek(&my_file, 0);
        return_if_fail( status == FX_SUCCESS);

        /* Now read in all the bytes again to make sure the file contents are really there.  */
        j =  0;
        read_value =  0;   
        write_value =  0;
        while (j < available_bytes)
        {

            /* Read 4 bytes from the file.  */
            status =  fx_file_read(&my_file, (void *) &read_value, sizeof(ULONG), &actual);

            /* Check the file read status.  */
            return_if_fail( (status == FX_SUCCESS) && (actual == 4) && (read_value == write_value));

            /* Increment byte count.  */
            j =  j + sizeof(ULONG);
            
            /* Increment write value.  */
            write_value++;
        }

        /* Close the test file.  */
        status =  fx_file_close(&my_file);
        return_if_fail( status == FX_SUCCESS);

        /* Close the media.  */
        status =  fx_media_close(&ram_disk);
        return_if_fail( status == FX_SUCCESS);
    }

    /* Output successful.  */     
    printf("SUCCESS!\n");
    test_control_return(0);
}         

#else  

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_media_full_test_application_define(void *first_unused_memory)
#endif
{
    
    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Media Full Test.........................N/A\n"); 

    test_control_return(255);
}
#endif
