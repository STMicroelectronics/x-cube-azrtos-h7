/* This FileX test concentrates on the Fault-Tolerant FAT write interrupt operation.  */
/*       
For FAT 12, one cluster size is 1024 bytes;
                                          
Check fat interrupt for fx_file_delete():        
Step1: Format and open the media; 
Step2: Enable fault tolerant feature;     
Step3: Create and allocate 330 clusters (8-337) for TEST.TXT; 
Step4: Create and allocate 10 clusters (338-347) for TEST2.TXT;
Step5: Adjust FAT chain of TEST2.TXT;              
Step6: Create new thread to delete TEST2.TXT;
Step7: Terminate the new thread to simulate poweroff when update the FAT entry 0x155 after record the redo log.  
Step8: Open the media; 
Step9: Enable fault tolerant feature to recover the data(redo operation);    
Step10: Check the TEST.TXT.
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
#include   "fx_utility.h"
#include   "fx_fault_tolerant.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"               
extern void    test_control_return(UINT status);
void    filex_fault_tolerant_file_delete_fat_multple_sectors_test_application_define(void *first_unused_memory);
                                            
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
static TX_THREAD               ftest_0;  
static TX_THREAD               ftest_1;
#else
static pthread_t               ptid1; 
#endif
static FX_MEDIA                ram_disk;
static FX_FILE                 my_file;
static UCHAR                   *pointer;

/* Define the counters used in the test application...  */
                                                    
#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *cache_buffer;
static UCHAR                  *fault_tolerant_buffer;
static UCHAR                  *thread_buffer;
#else
static UCHAR                  cache_buffer[CACHE_SIZE];
static UCHAR                  fault_tolerant_buffer[FAULT_TOLERANT_SIZE];
#endif 
static UINT                   error_couter = 0;
static UINT                   fat_write_interrupt = FX_FALSE; 
static CHAR                   read_buffer[1024];          
static UINT                   read_buffer_size = 1024;

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
void    filex_fault_tolerant_file_delete_fat_interrupt_test_application_define(void *first_unused_memory)
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
UINT        i;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant File Delete FAT Multiple Sectors Test...");
               

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
                                512,                    // Total sectors 
                                512,                    // Sector size   
                                1,                      // Sectors per cluster
                                1,                      // Heads
                                1);                     // Sectors per track 

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

    /* Allocate file for 335 clusters.  */
    status = fx_file_allocate(&my_file, 330 * 512);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)           
    {

        printf("ERROR!\n");
        test_control_return(5);
    }

    /* Close the test file.  */
    status =  fx_file_close(&my_file);

    /* Check the file close status.  */
    if (status != FX_SUCCESS)        
    {

        printf("ERROR!\n");
        test_control_return(9);
    }

    /* Create another file called TEST2.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST2.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(4);
    }

    /* Open the test file.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST2.TXT", FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)           
    {

        printf("ERROR!\n");
        test_control_return(5);
    }

    /* Allocate file for 10 clusters.
       The cluster 341 will be occupied which spans two sectors.  */
    status = fx_file_allocate(&my_file, 10 * 512);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)           
    {

        printf("ERROR!\n");
        test_control_return(5);
    }

    /* Close the test file.  */
    status =  fx_file_close(&my_file);

    /* Check the file close status.  */
    if (status != FX_SUCCESS)        
    {

        printf("ERROR!\n");
        test_control_return(9);
    }

    /* Adjust FAT chain to trigger the issue.
       Original FAT chain: 0x152->0x153->0x154->0x155(multiple sectors)->0x156->0x157...
       Modified FAT chain: 0x152->0x153->0x154->0x156->0x155(multiple sectors)->0x157... */
    _fx_utility_FAT_entry_write(&ram_disk, 0x154, 0x156);
    _fx_utility_FAT_entry_write(&ram_disk, 0x156, 0x155);
    _fx_utility_FAT_entry_write(&ram_disk, 0x155, 0x157);
                    
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

    /* After write interrupt, reread the files.  */ 

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(22);
    }
    
    /* Validate bug here. When the FAT entry spans two sectors and not flushed at the same time, the FAT chain will be like this:
       0x152->0x153->0x154->0x156->0x155(multiple sectors)->0x150(error)... 
       FAT entry 0x150 belongs to TEST.TXT thus the file is corrupted.  */

    /* Enable the Fault-tolerant feature to recover the media.  */       
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);

    /* Check status.   */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(23);
    }

    /* Open the test file and make sure it is not corrupted.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)           
    {

        printf("ERROR!\n");
        test_control_return(24);
    }

    /* Close the test file.  */
    status =  fx_file_close(&my_file);

    /* Check the file close status.  */
    if (status != FX_SUCCESS)        
    {

        printf("ERROR!\n");
        test_control_return(25);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if ((status != FX_SUCCESS) || (fat_write_interrupt != FX_TRUE) || (error_couter))
    {

        printf("ERROR!\n");
        test_control_return(33);
    }    

    /* Delete the thread.  */
#ifndef FX_STANDALONE_ENABLE
    tx_thread_delete(&ftest_1);
#else
    pthread_cancel(ptid1);
#endif  

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

    FX_PARAMETER_NOT_USED(thread_input);

    /* Set the callback function to simulate poweoff operation when write FAT entry.  */
    driver_write_callback = my_driver_write;
                 
    /* Create a file called TEST.TXT in the root directory.  */
    fx_file_delete(&ram_disk, "TEST2.TXT");
}

static UINT my_driver_write(FX_MEDIA *media_ptr, UINT sector_type, UCHAR *block_ptr, UINT *operation_ptr)
{
static UCHAR *last_block_ptr = FX_NULL;
                                       
    FX_PARAMETER_NOT_USED(block_ptr);

    /* Interrupt the FAT write operation after record the redo log.  */
    if ((sector_type == FX_FAT_SECTOR) && (_filex_fault_tolerant_log_check(media_ptr) & FX_FAULT_TOLERANT_LOG_REDO_DONE))
    {

        /* Skip FAT operations not spaning two sectors.  */
        if (last_block_ptr != (ram_disk_memory_large + 512))
        {
            last_block_ptr = block_ptr;
            return FX_SUCCESS;
        }
        last_block_ptr = block_ptr;
        if (block_ptr != (ram_disk_memory_large + 512 * 2))
        {
            return FX_SUCCESS;
        }

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
void    filex_fault_tolerant_file_delete_fat_multple_sectors_test_application_define(void *first_unused_memory)
#endif
{
    
    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant File Delete FAT Multiple Sectors Test...N/A\n"); 

    test_control_return(255);
}
#endif

