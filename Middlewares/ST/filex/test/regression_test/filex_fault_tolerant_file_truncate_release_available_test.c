/* This FileX test concentrates on the Fault-Tolerant truncate release operation.  */
/*
For FAT 12, 16, 32 and exFAT, one cluster size is 1024 bytes;
                              
Check directory interrupt for fx_file_truncate_release(): 
1: Format and open the media; 
2. Create and write 4K bytes into TEST.TXT;
3. Close file and close media;
4. Open the media;
5. Enable fault tolerant feature;     
6. Check available bytes;
7. Truncate release to size 0 of TEST.TXT;
8. Write 4K into TEST.TXT;
9. Check available bytes. It should be the same as the one at (6);
10. Close the media;
11. Open the media;
12. Check available bytes. It should be the same as the one at (6).
*/

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_system.h"
#include   "fx_fault_tolerant.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"               
extern void    test_control_return(UINT status);
void    filex_fault_tolerant_file_truncate_release_available_test_application_define(void *first_unused_memory);
                                            
#if defined (FX_ENABLE_FAULT_TOLERANT) && defined (FX_FAULT_TOLERANT) && defined (FX_FAULT_TOLERANT_DATA)

#define     DEMO_STACK_SIZE         4096
#ifdef FX_ENABLE_EXFAT
#define CACHE_SIZE                  FX_EXFAT_SECTOR_SIZE
#define FAULT_TOLERANT_SIZE         FX_EXFAT_SECTOR_SIZE
#else
#define CACHE_SIZE                  2048
#define FAULT_TOLERANT_SIZE         FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE
#endif


#define     FILE_SIZE               4096


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD               ftest_0;  
#endif
static FX_MEDIA                ram_disk;
static FX_FILE                 my_file;
static UCHAR                   *pointer;

/* Define the counters used in the test application...  */
                                                        
#ifndef FX_STANDALONE_ENABLE
static UCHAR                   *cache_buffer;
static UCHAR                   *fault_tolerant_buffer;
static UCHAR                   *thread_buffer;
#else
static UCHAR                   cache_buffer[CACHE_SIZE];
static UCHAR                   fault_tolerant_buffer[FAULT_TOLERANT_SIZE];
#endif 
static CHAR                    write_buffer[FILE_SIZE];

#ifdef FX_ENABLE_EXFAT
#define TEST_COUNT              4
#else              
#define TEST_COUNT              3
#endif

/* Define thread prototypes.  */

static void    ftest_0_entry(ULONG thread_input);  
                                                                                                   
extern void    _fx_ram_driver(FX_MEDIA *media_ptr);
extern void    test_control_return(UINT status); 
extern UINT    _filex_fault_tolerant_log_check(FX_MEDIA *media_ptr); 


/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_file_truncate_release_available_test_application_define(void *first_unused_memory)
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
    thread_buffer = pointer;
    pointer += DEMO_STACK_SIZE;
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
UINT        i;
ULONG       available_bytes[3];

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant File Truncate Release Available Test....");
                 
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

        /* Determine if the format had an error.  */
        return_if_fail(status == FX_SUCCESS);

        /* Open the ram_disk.  */
        status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
        return_if_fail(status == FX_SUCCESS);

        /* Create a file called TEST.TXT in the root directory.  */
        status =  fx_file_create(&ram_disk, "TEST.TXT");
        return_if_fail(status == FX_SUCCESS);

        /* Open the test file.  */
        status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
        return_if_fail(status == FX_SUCCESS);

        /* Write 4K bytes into the test file.  */
        status =  fx_file_write(&my_file, write_buffer, sizeof(write_buffer));
        return_if_fail(status == FX_SUCCESS);

        /* Close the test file.  */
        status =  fx_file_close(&my_file);
        return_if_fail(status == FX_SUCCESS);

        /* Close the media.  */
        status =  fx_media_close(&ram_disk);
        return_if_fail(status == FX_SUCCESS);

        /* Open the ram_disk.  */
        status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
        return_if_fail(status == FX_SUCCESS);

        /* Enable the Fault-tolerant feature.  */                                 
        status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
        return_if_fail(status == FX_SUCCESS);

        /* Get available_bytes before truncate release. */
        status = fx_media_space_available(&ram_disk, &available_bytes[0]);
        return_if_fail(status == FX_SUCCESS);

        /* Open the test file.  */
        status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
        return_if_fail(status == FX_SUCCESS);

        /* Truncate the "TEST.TXT" file from 4K to 0.  */
        fx_file_truncate_release(&my_file, 0);
        return_if_fail(status == FX_SUCCESS);

        /* Write 4K bytes into the test file.  */
        status =  fx_file_write(&my_file, write_buffer, sizeof(write_buffer));
        return_if_fail(status == FX_SUCCESS);

        /* Close the test file.  */
        status =  fx_file_close(&my_file);
        return_if_fail(status == FX_SUCCESS);

        /* Get available_bytes before truncate release. */
        status = fx_media_space_available(&ram_disk, &available_bytes[1]);
        return_if_fail(status == FX_SUCCESS);

        /* Check available bytes. */
        return_if_fail(available_bytes[0] == available_bytes[1]);

        /* Close the media.  */
        status =  fx_media_close(&ram_disk);
        return_if_fail(status == FX_SUCCESS);

        /* Open the ram_disk.  */
        status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
        return_if_fail(status == FX_SUCCESS);

        /* Enable the Fault-tolerant feature.  */                                 
        status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
        return_if_fail(status == FX_SUCCESS);

        /* Get available_bytes after write. */
        status = fx_media_space_available(&ram_disk, &available_bytes[2]);
        return_if_fail(status == FX_SUCCESS);

        /* Close the media.  */
        status =  fx_media_close(&ram_disk);
        return_if_fail(status == FX_SUCCESS);

        /* Check available bytes. */
        return_if_fail(available_bytes[0] == available_bytes[2]);
    }      

    /* for coverage */

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
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    status += fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    status += fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_allocate(&my_file, 4 * 512);
    return_if_fail(status == FX_SUCCESS);

    /* Fail while setting undo log. */
    _fx_utility_logical_sector_write_error_request = 1;
    status = fx_file_truncate_release(&my_file, 0);
    return_if_fail(status == FX_IO_ERROR);
    _fx_utility_logical_sector_write_error_request = 0;

    fx_file_close(&my_file);
    fx_media_close(&ram_disk);

    for (i = 1; i < 100; i++)
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
        status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
        status += fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
        status += fx_file_create(&ram_disk, "TEST.TXT");
        status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
        status += fx_file_write(&my_file, write_buffer, sizeof(write_buffer));
        status += fx_file_close(&my_file);
        status += fx_file_create(&ram_disk, "AAA.TXT");
        status += fx_file_open(&ram_disk, &my_file, "AAA.TXT", FX_OPEN_FOR_WRITE);
        status += fx_file_write(&my_file, write_buffer, sizeof(write_buffer));
        status += fx_file_close(&my_file);
        status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
        status += fx_file_allocate(&my_file, 512);
        return_if_fail(status == FX_SUCCESS);

        _fx_utility_fat_entry_read_error_request = i;
        status = fx_file_truncate_release(&my_file, 256);
        return_if_fail((status == FX_IO_ERROR) || (i > 6));
        _fx_utility_fat_entry_read_error_request = 0;

        fx_file_close(&my_file);
        fx_media_close(&ram_disk);

        if (status != FX_IO_ERROR)
        {
            break;
        }
    }

    /* Output successful.  */     
    printf("SUCCESS!\n");
    test_control_return(0);
}         
#else  

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_file_truncate_release_available_test_application_define(void *first_unused_memory)
#endif
{
    
    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant File Truncate Release Available Test....N/A\n");

    test_control_return(255);
}
#endif

