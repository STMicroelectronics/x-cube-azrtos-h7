/* This FileX test concentrates on the Fault-Tolerant write large data fat interrupt test.  */
/*          
For FAT 12, 16, 32 and exFAT, one cluster size is 1024 bytes;
Check write large data fat interrupt operation:          
Step1: Format and open the media; 
Step2: Enable fault tolerant feature;   
Step3: Create new file called "TEST.TXT";    
Step4: Get the media available bytes;    
Step5: Write large bytes out of the file to fill the media 
Step7: Terminate the new thread to simulate poweroff when update the fat between undo log generated and redo log not generated.  
Step8: Open the media; 
Step9: Enable fault tolerant feature to recover the data(undo operation);    
Step10: Check the test files.
*/ 
#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#else
#define    _GNU_SOURCE
#define    _DEFAULT_SOURCE
#include   <pthread.h>
#include   <unistd.h>
#endif
#include   "fx_api.h"    
#include   "fx_system.h"
#include   "fx_fault_tolerant.h"
#include   <stdio.h>
#include   <time.h>
#include   "fx_ram_driver_test.h"               
extern void    test_control_return(UINT status);
void    filex_fault_tolerant_write_large_data_fat_interrupt_test_application_define(void *first_unused_memory);
                                         
#if defined (FX_ENABLE_FAULT_TOLERANT) && defined (FX_FAULT_TOLERANT)

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
static TX_THREAD                ftest_1;
#else
static pthread_t                ptid1; 
#endif
static FX_MEDIA                 ram_disk;
static FX_FILE                  my_file;
static UCHAR                    *pointer;

/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                    *cache_buffer;       
static UCHAR                    *fault_tolerant_buffer;  
static UCHAR                    *thread_buffer;
#else
static UCHAR                    cache_buffer[CACHE_SIZE];       
static UCHAR                    fault_tolerant_buffer[FAULT_TOLERANT_SIZE];
#endif 

static UINT                     error_couter = 0;          
static ULONG                    write_data_size;
static ULONG                    available_bytes;
static UINT                     fat_write_interrupt = FX_FALSE;    
static ULONG                    i, j;

#ifdef FX_ENABLE_EXFAT
#define TEST_COUNT              4
#else              
#define TEST_COUNT              3
#endif

/* Define thread prototypes.  */

static void    ftest_0_entry(ULONG thread_input);  
#ifndef FX_STANDALONE_ENABLE
static void    ftest_1_entry(ULONG thread_input);
#else
static void   * ftest_1_entry(void * thread_input);  
#endif        

extern void    _fx_ram_driver(FX_MEDIA *media_ptr);
extern void    test_control_return(UINT status);    
extern UINT    _filex_fault_tolerant_log_check(FX_MEDIA *media_ptr); 
extern UINT    (*driver_write_callback)(FX_MEDIA *media_ptr, UINT sector_type, UCHAR *block_ptr, UINT *operation_ptr);  
static UINT    my_driver_write(FX_MEDIA *media_ptr, UINT sector_type, UCHAR *block_ptr, UINT *operation_ptr);       


/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_write_large_data_fat_interrupt_test_application_define(void *first_unused_memory)
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
ULONG       actual;
ULONG       data_value;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Write Large Data Fat Interrupt test.....");

    /* Generate a random number for write data.  */
    srand((ULONG)time(NULL));
    data_value = (ULONG)(rand() & 0xFF) | (ULONG)((rand() & 0xFF) << 8) | (ULONG)((rand() & 0xFF) << 16) | (ULONG)((rand() & 0xFF) << 24);

    /* Genearte the write data.  */
    for (j = 0; j < large_data_buffer_size / sizeof(ULONG); j ++)
    {
        ((ULONG*)large_data_buffer)[j] = data_value++;
    }

    /* Roll back the value for later verification use.  */
    data_value -= (large_data_buffer_size / sizeof(ULONG));

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
                                     4000 * 8,               // Total sectors 
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
                                     60000 * 8,              // Total sectors 
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
                                     400000 * 8,             // Total sectors 
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
#if FX_EXFAT_SECTOR_SIZE <=512
                                            400000 * 2,             // Total sectors
#else
                                            40000 * 2,              // Total sectors
#endif
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
                                
        /* Create a file called TEST.TXT in the root directory.  */
        status =  fx_file_create(&ram_disk, "TEST.TXT");

        /* Check the create status.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(4);
        }

        /* Open the test file.  */
        status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

        /* Check the file open status.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(5);
        }

        /* Pickup the available bytes in the media.  */
        status =  fx_media_space_available(&ram_disk, &available_bytes);

        /* Check for available bytes error.  */
        if ((status != FX_SUCCESS) || (available_bytes < sizeof(ULONG)))
        {

            printf("ERROR!\n");
            test_control_return(6);
        }
            
        write_data_size = available_bytes;

        /* Create the main thread.  */
#ifndef FX_STANDALONE_ENABLE
        tx_thread_create(&ftest_1, "thread 1", ftest_1_entry, 0,  
                        thread_buffer, DEMO_STACK_SIZE, 
                        4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);
#endif

        /* directory_write_interrupt */
        fat_write_interrupt = FX_FALSE;

        /* Let the other thread run.  */
#ifndef FX_STANDALONE_ENABLE
        tx_thread_relinquish();
#else
        pthread_create(&ptid1, NULL, &ftest_1_entry, NULL);
        usleep(10);
        pthread_join(ptid1,NULL);
#endif
                        
        /* Open the ram_disk.  */
        status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);

        /* Check the status.  */
        if (status != FX_SUCCESS)
        {

            /* Error, return error code.  */
            printf("ERROR!\n");
            test_control_return(8);
        }

        /* Enable the Fault-tolerant feature.  */                                
        status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);

        /* Check status.   */
        if (status)
        {
                                                                                            
            printf("ERROR!\n");
            test_control_return(9);
        }       

        /* Pickup the available bytes in the media again.  */
        status =  fx_media_space_available(&ram_disk, &j);

        /* Check for available bytes error.  */
        if (i != 3)
        {
            if ((status != FX_SUCCESS) || (j != available_bytes))
            {

                printf("ERROR!\n");
                test_control_return(10);
            }
        }
        else
        {   
            if ((status != FX_SUCCESS) || (j != 0))
            {

                printf("ERROR!\n");
                test_control_return(10);
            }
        }
                   
        /* Open the test file.  */
        status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

        /* Check the file open status.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(11);
        }

        /* Seek to the beginning of the test file.  */
        status =  fx_file_seek(&my_file, 0);

        /* Check the file seek status.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(12);
        }

        /* Now read in all the bytes again to make sure the file contents are really there.  */
        status =  fx_file_read(&my_file, (void *) large_data_buffer, large_data_buffer_size, &actual);

        /* Check the file read status.  */   
        if (i != 3)
        {
            if ((status != FX_END_OF_FILE) || (actual != 0))
            {

                printf("ERROR!\n");
                test_control_return(13);
            }
        }
        else
        {    
            if ((status != FX_SUCCESS) || (actual != write_data_size))
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
        if ((status != FX_SUCCESS) || (fat_write_interrupt != FX_TRUE) || (error_couter))
        {

            printf("ERROR!\n");
            test_control_return(15);
        }    

        /* Delete the thread.  */
#ifndef FX_STANDALONE_ENABLE
        tx_thread_delete(&ftest_1);
#else
        pthread_cancel(ptid1);
#endif
    }

    /* Output successful.  */     
    printf("SUCCESS!\n");
    test_control_return(0);
}         
       
/* Define the test threads.  */

#ifndef FX_STANDALONE_ENABLE
static void    ftest_1_entry(ULONG thread_input)
#else
 void  *  ftest_1_entry(void * thread_input)
#endif
{

#ifdef FX_STANDALONE_ENABLE
    UINT oldtype;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);  
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
#endif

    UINT        status;  
                             
    FX_PARAMETER_NOT_USED(thread_input);

    /* Set the callback function to simulate poweoff operation when write FAT entry.  */   
    /* exFAT will not update the FAT table when write data at the beginning of the test file.  */
    if (i != 3)
        driver_write_callback = my_driver_write;    

    /* Write the data to fill the media one time.  */
    status =  fx_file_write(&my_file, (void *) large_data_buffer, write_data_size);

    /* Check the file write status.  */
    if (status != FX_SUCCESS)
    {                             
        error_couter ++;
#ifndef FX_STANDALONE_ENABLE
        return;
#else
        return NULL;
#endif
    } 
                                                                             
    /* exFAT will not update the FAT table when write data at the beginning of the test file.  */
    if (i == 3)
    {           

        /* Update the flag.  */
        fat_write_interrupt = FX_TRUE;   

        /* Close the test file.  */
        status =  fx_file_close(&my_file);

        /* Close the media.  */
        status +=  fx_media_close(&ram_disk);

        /* Determine if the test was successful.  */
        if (status != FX_SUCCESS)
        {
            error_couter ++;
#ifndef FX_STANDALONE_ENABLE
        return;
#else
        return NULL;
#endif
        }    
    }
}

static UINT my_driver_write(FX_MEDIA *media_ptr, UINT sector_type, UCHAR *block_ptr, UINT *operation_ptr)
{

    FX_PARAMETER_NOT_USED(block_ptr);

    /* Interrupt the FAT write operation after record the undo log.  */
    if ((sector_type == FX_FAT_SECTOR) && (_filex_fault_tolerant_log_check(media_ptr) & FX_FAULT_TOLERANT_LOG_UNDO_DONE))
    {

        /* Set the write interrupt operation.  */
        *operation_ptr = FX_OP_WRITE_INTERRUPT;

        /* Update the flag.  */
        fat_write_interrupt = FX_TRUE;   

        /* Clean the callback function.  */
        driver_write_callback = FX_NULL;
                        
        /* Delete the media protection structure if FX_SINGLE_THREAD is not
        defined.  */
#ifndef FX_SINGLE_THREAD   
#ifndef FX_DONT_CREATE_MUTEX

        /* Note that the protection is never released. The mutex delete
        service will handle all threads waiting access to this media
        control block.  */
        tx_mutex_delete(&(media_ptr -> fx_media_protect));
#endif
#endif

        /* Clean the media data.  */
        _fx_system_media_opened_ptr = FX_NULL;
        _fx_system_media_opened_count = 0;      

        /* Clean the media.  */
        memset(media_ptr, 0, sizeof(FX_MEDIA));   

        /* Simulate poweroff.  */
        /* First terminate the thread to ensure it is ready for deletion.  */
#ifndef FX_STANDALONE_ENABLE
        tx_thread_terminate(&ftest_1);
#else
        pthread_cancel(ptid1);
#endif
    }

    /* Return.  */
    return FX_SUCCESS;
}
#else  

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_write_large_data_fat_interrupt_test_application_define(void *first_unused_memory)
#endif
{
    
    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Write Large Data Fat Interrupt test.....N/A\n"); 

    test_control_return(255);
}
#endif
