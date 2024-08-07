/* This FileX test concentrates on the Fault-Tolerant data loop write operation.  */
/*                 
For FAT 12, 16, 32 and exFAT, one cluster size is 1024 bytes;

         1024        1024          1024       1024         1024         1024
    |------------|------------|-----------|------------|------------|------------|
    |            |  TEST.TXT  | TEST.TXT  |            |            |            |
    |------------|------------|-----------|------------|------------|------------|
 
Check loop write data operation:                                                                  
Step1: Format and open the media; 
Step2: Enable fault tolerant feature;      
Step3: Create new file called "TEST.TXT";      
Step4: Check the media available bytes;
Step5: Write 1500 bytes to "TEST.TXT"(two clusters);
Step6: Check the media available bytes;
Step7: Loop to write random size bytes to "TEST.TXT"(less than two clusters); 
Step8: Check the media available bytes every write operation.
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
void    filex_fault_tolerant_loop_write_data_test_application_define(void *first_unused_memory);

#if defined (FX_ENABLE_FAULT_TOLERANT) && defined (FX_FAULT_TOLERANT) && defined (FX_FAULT_TOLERANT_DATA)

#define     DEMO_STACK_SIZE         4096
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
static CHAR                     write_buffer[2048];          
static UINT                     write_buffer_size = 2048;    
static UINT                     write_index;
static CHAR                     read_buffer[4096];          
static UINT                     read_buffer_size = 4096;  
static UINT                     i;
                                                         
#ifdef FX_ENABLE_EXFAT
#define TEST_COUNT              4            /* FAT12, 16, 32 and exFAT.  */
#else              
#define TEST_COUNT              3            /* FAT12, 16, 32.  */
#endif

/* Define thread prototypes.  */

static void    ftest_0_entry(ULONG thread_input);  
extern void    _fx_ram_driver(FX_MEDIA *media_ptr);
extern void    test_control_return(UINT status);
extern UINT    (*driver_write_callback)(FX_MEDIA *media_ptr, UINT sector_type, UCHAR *block_ptr, UINT *operation_ptr);  


/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_loop_write_data_test_application_define(void *first_unused_memory)
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
UINT        loop_count;
UINT        seek_index;
UINT        write_size;
UINT        two_cluster_size;
ULONG       file_size;
ULONG       current_available_bytes;
ULONG       available_bytes;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Loop Write DATA Test....................");

    /* Random genearte the write data.  */
    for (write_index = 0; write_index < write_buffer_size; write_index ++)
        write_buffer[write_index] = (CHAR)rand();
                 
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
        if (status)
        {

            printf("ERROR!\n");
            test_control_return(1);
        }

        /* Initialize the parmeters.  */      
        file_size = 1500; 
        two_cluster_size = 2048;

        /* Open the ram_disk.  */
        status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);

        /* Check the status.  */
        if (status != FX_SUCCESS)
        {

            /* Error, return error code.  */
            printf("ERROR!\n");
            test_control_return(2);
        }

        /* Enable the Fault-tolerant feature.  */                                 
        status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);

        /* Check status.   */
        if (status)
        {

            printf("ERROR!\n");
            test_control_return(3);
        }
                                   
        /* Pickup the available bytes in the media.  */
        status =  fx_media_space_available(&ram_disk, &available_bytes);
                                        
        /* Check for available bytes error.  */
        if (status != FX_SUCCESS)
        {

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

        /* Write a string to the test file, (two cluster).  */
        status =  fx_file_write(&my_file, (void *) write_buffer, file_size);

        /* Check the file write status.  */
        if (status != FX_SUCCESS)    
        {

            printf("ERROR!\n");
            test_control_return(7);
        }
                                        
        /* Pickup the available bytes in the media.  */
        status =  fx_media_space_available(&ram_disk, &current_available_bytes);
                                        
        /* Check for available bytes error.  */
        if ((status != FX_SUCCESS) && (available_bytes != (current_available_bytes + two_cluster_size * 2)))
        {

            printf("ERROR!\n");
            test_control_return(8);
        }   

        /* Update the avalibale_bytes.  */
        available_bytes = current_available_bytes;

        /* Loop to write the data to test file.  */
        for (loop_count = 0; loop_count < 1000; loop_count ++)
        {

            /* Random generate the seek index (0~file_size).  */
            seek_index = (ULONG)rand() % (file_size + 1);

            /* Seek to the beginning of the test file.  */
            status =  fx_file_seek(&my_file, seek_index);

            /* Check the file seek status.  */
            if (status != FX_SUCCESS)    
            {

                printf("ERROR!\n");
                test_control_return(9);
            }  

            /* Random generate the write size, (0 ~ two cluster size - seek_index);  */
            write_size = (UINT)rand() % (two_cluster_size - seek_index + 1);

            /* Write a string to the test file.  */
            status =  fx_file_write(&my_file, (void *) write_buffer, write_size);

            /* Check the file write status.  */
            if (status != FX_SUCCESS)    
            {

                printf("ERROR!\n");
                test_control_return(10);
            }

            /* Update the file_size.  */
            if (seek_index + write_size > file_size)
                file_size = seek_index + write_size;

            /* Seek to the beginning of the test file.  */
            status =  fx_file_seek(&my_file, 0);

            /* Check the file seek status.  */
            if (status != FX_SUCCESS)    
            {

                printf("ERROR!\n");
                test_control_return(11);
            }             

            /* Read the bytes of the test file.  */
            status =  fx_file_read(&my_file, read_buffer, read_buffer_size, &actual);

            /* Check the file read status.  */
            if ((status != FX_SUCCESS) || (actual != file_size))   
            {

                printf("ERROR!\n");
                test_control_return(12);
            }

            /* Pickup the available bytes in the media.  */
            status =  fx_media_space_available(&ram_disk, &available_bytes);

            /* Check for available bytes error.  */
            if ((status != FX_SUCCESS) || (available_bytes != current_available_bytes))
            {

                printf("ERROR!\n");
                test_control_return(13);
            }     
        }

        /* Close the test file.  */
        status =  fx_file_close(&my_file);

        /* Check the file close status.  */
        if (status != FX_SUCCESS)        
        {

            printf("ERROR!\n");
            test_control_return(14);
        }        

        /* Close the media.  */
        status =  fx_media_close(&ram_disk);

        /* Determine if the test was successful.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(15);
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
void    filex_fault_tolerant_loop_write_data_test_application_define(void *first_unused_memory)
#endif
{
    
    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Loop Write DATA Test....................N/A\n");  

    test_control_return(255);
}
#endif

