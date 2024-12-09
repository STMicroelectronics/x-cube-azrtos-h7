/* This FileX test concentrates on the bug as per
 * JIRA: https://expresslogic.atlassian.net/browse/FIL-4.
 * In fx_fault_tolerant_cleanup_FAT_chain.c, a group of the FAT entries are updated in the cache
 * before it is flushed into media system. The design requires the FAT entries updated in reverse
 * order in FAT chain. Due to the FAT cache, the FAT entries may be updated in any order.
 * That will break the fault tolerant design. The expected issue is, the FAT chain could
 * not be released completely.*/   
/*
For FAT 32 one cluster size is 512 bytes and one sector per cluster;
                   
Step1: Format and open the media; 
Step2: Enable fault tolerant feature;     
Step3: Create and allocate 118 clusters for A.TXT.
Step6: Create a new thread and append data to A.TXT to occupy cluster 127 and 128.
Step7: Terminate the new thread to simulate poweroff while writing the fat entry of cluster 127
(the fat entry of cluster 128 is flushed first).
Step8: Open the media.
Step9: Enable fault tolerant feature to revert the FAT chain(revert operation).
Step10: Perform media check.
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
#include   "fx_utility.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"               
extern void    test_control_return(UINT status);
void    filex_fault_tolerant_file_write_fat_crossover_2_test_application_define(void *first_unused_memory);
                                              
#ifdef FX_ENABLE_FAULT_TOLERANT

#define DEMO_STACK_SIZE         4096
#define CACHE_SIZE              2048
#define FAULT_TOLERANT_SIZE     FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE


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
static UINT                     fat_write_interrupt = FX_FALSE;
static UCHAR                    scratch_memory[16 * 1024];
static UINT                     interrupt_count_down;


/* Define thread prototypes.  */

static void    ftest_0_entry(ULONG thread_input);  
#ifndef FX_STANDALONE_ENABLE
static void    ftest_1_entry(ULONG thread_input);
#else
static void   * ftest_1_entry(void * thread_input);  
#endif   

extern void  _fx_ram_driver(FX_MEDIA *media_ptr);
extern void  test_control_return(UINT status);      
extern UINT  _filex_fault_tolerant_log_check(FX_MEDIA *media_ptr); 
extern UINT  (*driver_write_callback)(FX_MEDIA *media_ptr, UINT sector_type, UCHAR *block_ptr, UINT *operation_ptr);  
static UINT  my_driver_write(FX_MEDIA *media_ptr, UINT sector_type, UCHAR *block_ptr, UINT *operation_ptr);



/* Define what the initial system looks like.  */
#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_file_write_fat_crossover_2_test_application_define(void *first_unused_memory)
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
ULONG       errors_detected;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant File Write Fat Crossover 2 Test.........");

    /* Format the media with FAT32.  This needs to be done before opening it!  */
    status = fx_media_format(&ram_disk, 
                             _fx_ram_driver,         // Driver entry            
                             ram_disk_memory_large,  // RAM disk memory pointer
                             cache_buffer,           // Media buffer pointer
                             CACHE_SIZE,             // Media buffer size 
                             "MY_RAM_DISK",          // Volume Name
                             1,                      // Number of FATs
                             32,                     // Directory Entries
                             0,                      // Hidden sectors
                             70000,                  // Total sectors 
                             512,                    // Sector size   
                             1,                      // Sectors per cluster
                             1,                      // Heads
                             1);                     // Sectors per track 
    status +=  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    status += fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail(FX_SUCCESS == status);

    /* One cluster for A.TXT. */
    status = fx_file_create(&ram_disk, "A.TXT");
    status += fx_file_open(&ram_disk, &my_file, "A.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_allocate(&my_file, 512 * 118);     
    status += fx_file_close(&my_file);
    return_if_fail(FX_SUCCESS == status);

    /* Ensure no lost clusters. */
    status = fx_media_check(&ram_disk, scratch_memory, sizeof(scratch_memory), FX_LOST_CLUSTER_ERROR, &errors_detected);
    return_if_fail((status == FX_SUCCESS) && (0 == errors_detected));

#ifndef FX_STANDALONE_ENABLE
    /* Create the main thread.  */
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
    return_if_fail(FX_TRUE == fat_write_interrupt);

    status = fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    status += fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail(FX_SUCCESS == status);

    status =  fx_media_check(&ram_disk, scratch_memory, sizeof(scratch_memory), FX_LOST_CLUSTER_ERROR, &errors_detected);
    return_if_fail(FX_SUCCESS == status);
    return_if_fail(errors_detected == 0);

    fx_media_close(&ram_disk);

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
    UINT        status;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Set the callback function to simulate poweoff operation when write FAT entry.  */   
    driver_write_callback = my_driver_write;
                                                  
    /* Open the test file.  */
    status = fx_file_open(&ram_disk, &my_file, "A.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_seek(&my_file, 118 * 512);
    return_if_fail(FX_SUCCESS == status);

    interrupt_count_down = 2;

    /*  The FAT sector of the fat entries of cluster 127 is not the FAT sector for cluster 128.
        Since the record of cluster 128 is located at fat_cache[0], the fat sector of 128 will be flushed first.
        Terminate the new thread to simulate poweroff while writing the fat entry of cluster 127.
    */
    fx_file_write(&my_file, scratch_memory, 512 * 2);
}

static UINT my_driver_write(FX_MEDIA *media_ptr, UINT sector_type, UCHAR *block_ptr, UINT *operation_ptr)
{

    FX_PARAMETER_NOT_USED(block_ptr);

    /* Interrupt the FAT write operation after record the undo log.  */
    if (sector_type == FX_FAT_SECTOR)
    {

        interrupt_count_down--;
        if (interrupt_count_down == 0)
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
    }

    /* Return.  */
    return FX_SUCCESS;
}
#else  

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_file_write_fat_crossover_2_test_application_define(void *first_unused_memory)
#endif
{
    
    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant File Write Fat Crossover 2 Test.........N/A\n");

    test_control_return(255);
}
#endif

