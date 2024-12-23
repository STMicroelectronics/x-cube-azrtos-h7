/* This FileX test concentrates on the Fault-Tolerant file seek operation.  */
/*
1. Format and open the media; 
2. Enable fault tolerant feature;     
3. Create and write 1024 bytes into TEST.TXT; 
4. Seek to the beginning of the file;
5. Write 640 bytes into TEST.TXT;
6. Verify the data;
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
void    filex_fault_tolerant_file_seek_test_application_define(void *first_unused_memory);

#if defined (FX_ENABLE_FAULT_TOLERANT) && defined (FX_FAULT_TOLERANT) && defined (FX_FAULT_TOLERANT_DATA)

#define     DEMO_STACK_SIZE         4096
#ifdef FX_ENABLE_EXFAT
#define CACHE_SIZE                  FX_EXFAT_SECTOR_SIZE
#define FAULT_TOLERANT_SIZE         FX_EXFAT_SECTOR_SIZE
#define TOTAL_SIZE                  FX_EXFAT_SECTOR_SIZE
#define OVERWRITE_SIZE              FX_EXFAT_SECTOR_SIZE
#else
#define CACHE_SIZE                   (16 * 1024)
#define FAULT_TOLERANT_SIZE         FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE
#define TOTAL_SIZE                  1024
#define OVERWRITE_SIZE              512
#endif




/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD                ftest_0;  
#endif
static FX_MEDIA                 ram_disk;
static FX_FILE                  my_file;
static UCHAR                    *pointer;

/* Define the counters used in the test application...  */
                                                         
static UCHAR                    cache_buffer[CACHE_SIZE];       
static UCHAR                    fault_tolerant_buffer[FAULT_TOLERANT_SIZE];  
static UCHAR                    *thread_buffer;
static CHAR                     write_buffer[2][TOTAL_SIZE];          
static CHAR                     read_buffer[TOTAL_SIZE];          
                                                         
#ifdef FX_ENABLE_EXFAT
#define TEST_COUNT              4            /* FAT12, 16, 32 and exFAT.  */
#else              
#define TEST_COUNT              3            /* FAT12, 16, 32.  */
#endif

/* Define thread prototypes.  */

static void    ftest_0_entry(ULONG thread_input);  
extern void    _fx_ram_driver(FX_MEDIA *media_ptr);
extern void    test_control_return(UINT status);     



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_file_seek_test_application_define(void *first_unused_memory)
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
ULONG       actual;  
UINT        i;
UINT        j;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant File Seek Test..........................");
               
    /* Loop to test FAT 12, 16, 32 and exFAT.   */
    for (i = 0; i < TEST_COUNT; i ++)
    {

        /* Initialize the write and read buffer. */
        for (j = 0; j < sizeof(write_buffer[0]); j++)
        {
            write_buffer[0][j] = (CHAR)(rand() & 0xFF);
            write_buffer[1][j] = (CHAR)(~write_buffer[0][j] & 0xFF);
        }

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
                                     512,                    // Sector size   
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
                                     512,                    // Sector size   
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
                                     512,                    // Sector size   
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
                                            8,                      // exFAT Sectors per cluster
                                            12345,                  // Volume ID
                                            0);                     // Boundary unit
        }
#endif
        return_if_fail(status == FX_SUCCESS);

        /* Open the ram_disk.  */
        status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
        return_if_fail(status == FX_SUCCESS);

        /* Enable the Fault-tolerant feature.  */                             
        status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
        return_if_fail(status == FX_SUCCESS);

        /* Prepare a file for test. */
        status = fx_file_create(&ram_disk, "TEST.TXT");
        return_if_fail(status == FX_SUCCESS);

        status = fx_file_create(&ram_disk, "TEST.TXT");
        return_if_fail(status == FX_ALREADY_CREATED);

        status = fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
        return_if_fail(status == FX_SUCCESS);

        /* Write data. */
        status = fx_file_write(&my_file, write_buffer[0], TOTAL_SIZE);
        return_if_fail(status == FX_SUCCESS);

        /* Seek to the beginning of file. */
        status = fx_file_relative_seek(&my_file, 0, FX_SEEK_BEGIN);
        return_if_fail(status == FX_SUCCESS);

        /* Overwrite the beginning data. */
        status = fx_file_write(&my_file, write_buffer[1], OVERWRITE_SIZE);
        return_if_fail(status == FX_SUCCESS);

        status = fx_file_relative_seek(&my_file, 0, FX_SEEK_BEGIN);
        return_if_fail(status == FX_SUCCESS);

        /* Read the bytes of the test file.  */
        memset(read_buffer, 0xFF, sizeof(read_buffer));
        status = fx_file_read(&my_file, read_buffer, sizeof(read_buffer), &actual);
        return_if_fail((status == FX_SUCCESS) && (actual == sizeof(read_buffer)));

        /* Check the data of overwrite part. */
        return_if_fail(memcmp(&read_buffer[0], write_buffer[1], OVERWRITE_SIZE) == 0);

        /* Check the data of remaining part. */
        return_if_fail(memcmp(&read_buffer[OVERWRITE_SIZE],
                              &write_buffer[0][OVERWRITE_SIZE],
                              (TOTAL_SIZE - OVERWRITE_SIZE)) == 0);

        /* Close the file. */
        status = fx_file_close(&my_file);
        return_if_fail(status == FX_SUCCESS);

        /* Close the media.  */
        status =  fx_media_close(&ram_disk);
        return_if_fail(status == FX_SUCCESS);
    }      

    /* Output successful.  */     
    printf("SUCCESS!\n");
    test_control_return(0);
}         
#else  

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_file_seek_test_application_define(void *first_unused_memory)
#endif
{
    
    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant File Seek Test..........................N/A\n");

    test_control_return(255);
}
#endif

