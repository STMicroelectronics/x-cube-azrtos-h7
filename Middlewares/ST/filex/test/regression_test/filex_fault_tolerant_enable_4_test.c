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
void    filex_fault_tolerant_enable_4_test_application_define(void *first_unused_memory);
                                            
#if defined (FX_ENABLE_FAULT_TOLERANT) && defined (FX_FAULT_TOLERANT) && defined (FX_FAULT_TOLERANT_DATA)

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
static UINT                     directory_write_interrupt = FX_FALSE; 
static CHAR                     write_buffer[2048];          
static UCHAR                    read_buffer[4096];          

/* Define thread prototypes.  */

static void    ftest_0_entry(ULONG thread_input);  
#ifndef FX_STANDALONE_ENABLE
static void    ftest_1_entry(ULONG thread_input);
#else
static void    *ftest_1_entry(void * thread_input);  
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
void    filex_fault_tolerant_enable_4_test_application_define(void *first_unused_memory)
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

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Enable 4 Test...........................");
              
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
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail(status == FX_SUCCESS);

    /* Enable the Fault-tolerant feature.  */                               
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail(status == FX_SUCCESS);

    /* Create a file called TEST1.TXT in the root directory.  */
    status = fx_file_create(&ram_disk, "TEST1.TXT");
    return_if_fail(status == FX_SUCCESS);
#ifndef FX_STANDALONE_ENABLE
    /* Create the main thread.  */
    tx_thread_create(&ftest_1, "thread 1", ftest_1_entry, 0,  
                    thread_buffer, DEMO_STACK_SIZE, 
                    4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);
#endif
    
    /* directory_write_interrupt */
    directory_write_interrupt = FX_FALSE;

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
    return_if_fail(status == FX_SUCCESS);

    /**** Modify the checksum of log content so that the log will not apply. ****/
    _fx_utility_logical_sector_read( &ram_disk, 1 + ram_disk.fx_media_sectors_per_FAT + ram_disk.fx_media_root_sectors, read_buffer, 8, FX_DATA_SECTOR);
    read_buffer[FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET]++;
    _fx_utility_logical_sector_write( &ram_disk, 1 + ram_disk.fx_media_sectors_per_FAT + ram_disk.fx_media_root_sectors, read_buffer, 8, FX_DATA_SECTOR);

    /* Enable the Fault-tolerant feature to recover the media.  */           
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail(status == FX_SUCCESS);

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

    /* Open the test file.  */
    fx_file_open(&ram_disk, &my_file, "TEST1.TXT", FX_OPEN_FOR_WRITE);
    fx_file_seek(&my_file, 0);

    /* Write something to the file. */
    fx_file_write(&my_file, (void *) write_buffer, 2048);
}

static UINT my_driver_write(FX_MEDIA *media_ptr, UINT sector_type, UCHAR *block_ptr, UINT *operation_ptr)
{

    FX_PARAMETER_NOT_USED(block_ptr);

    /* Interrupt the Directory write operation after record the redo log.  */
    if ((sector_type == FX_DIRECTORY_SECTOR) && (_filex_fault_tolerant_log_check(media_ptr) & FX_FAULT_TOLERANT_LOG_REDO_DONE))
    {

        /* Set the write interrupt operation.  */
        *operation_ptr = FX_OP_WRITE_INTERRUPT;

        /* Update the flag.  */
        directory_write_interrupt = FX_TRUE;   
                                             
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
void    filex_fault_tolerant_enable_4_test_application_define(void *first_unused_memory)
#endif
{
    
    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Enable 4................................N/A\n");  

    test_control_return(255);
}
#endif

