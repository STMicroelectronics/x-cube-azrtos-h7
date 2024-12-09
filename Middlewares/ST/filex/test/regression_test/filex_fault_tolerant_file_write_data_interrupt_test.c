/* This FileX test concentrates on the Fault-Tolerant data write interrupt operation.  */
/*
For FAT 12, 16, 32, and exFAT one cluster size is 1024 bytes;
         1024         1024        1024        1024         1024         1024
    |------------|------------|-----------|------------|------------|------------|
    |  TEST.TXT  |  TEST1.TXT | TEST1.TXT |  TEST2.TXT |            |            |
    |------------|------------|-----------|------------|------------|------------|
                 |      |     |    |    |
                 |      |     |    |    |
               Test1    |     |    |    |
                       Test2  |    |    |
                            Test3  |    |
                                 Test4  |
                                        |
                                      Test5
                        
Check data interrupt for fx_file_write() :                        
Step1: Format and open the media; 
Step2: Enable fault tolerant feature;     
Step3: Create and write 28 bytes into TEST.TXT; 
Step4: Create and write 1500 bytes into TEST1.TXT, (two cluster);
Step5: Create and write 19 bytes into TEST2.TXT;             
Step6: Create new thread to continue to write 1024 data into TEST1.TXT:
       Test1: Seek to the file: index 0;
       Test2: Seek to the file: index 1023;    
       Test3: Seek to the file: index 1024;
       Test4: Seek to the file: index 1025; 
       Test5: Seek to the file: index 1500;
Step7: Terminate the new thread to simulate poweroff when write the data between undo log generated and redo log not generated.  
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
#include   "fx_ram_driver_test.h"               
extern void    test_control_return(UINT status);
void    filex_fault_tolerant_file_write_data_interrupt_test_application_define(void *first_unused_memory);

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
static UINT                     data_write_interrupt = FX_FALSE;    
static UINT                     write_index; 
static UINT                     data_size = 0;
static UINT                     i;
#ifdef FX_ENABLE_EXFAT
static CHAR                     write_buffer[4*FX_EXFAT_SECTOR_SIZE];          
static UINT                     write_buffer_size = 4*FX_EXFAT_SECTOR_SIZE;
static CHAR                     read_buffer[4*FX_EXFAT_SECTOR_SIZE];
static UINT                     read_buffer_size = 4*FX_EXFAT_SECTOR_SIZE;  
static UCHAR                    data_buffer[4*FX_EXFAT_SECTOR_SIZE];
#else
static CHAR                     write_buffer[2048];          
static UINT                     write_buffer_size = 2048;
static CHAR                     read_buffer[4096];
static UINT                     read_buffer_size = 4096;  
static UCHAR                    data_buffer[4096];
#endif                                  

#define SEEK_COUNT              5                   
#ifdef FX_ENABLE_EXFAT
#define FAT_COUNT               4            /* FAT12, 16, 32 and exFAT.  */
#else              
#define FAT_COUNT               3            /* FAT12, 16, 32.  */
#endif
#define TEST_COUNT              FAT_COUNT * SEEK_COUNT

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
void    filex_fault_tolerant_file_write_data_interrupt_test_application_define(void *first_unused_memory)
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

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant File Write DATA Interrupt Test..........");
               
    /* Loop to test FAT 12, 16, 32 and exFAT.   */
    for (i = 0; i < TEST_COUNT; i ++)
    {
        if (i < 5)
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
        else if (i < 10)
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
        else if (i < 15)
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

        /* Prepare a file for test. */
        status = fx_file_create(&ram_disk, "TEST.TXT");
        status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
        status += fx_file_write(&my_file, " ABCDEFGHIJKLMNOPQRSTUVWXYZ\n", 28);
        status += fx_file_seek(&my_file, 0);
        return_if_fail( status == FX_SUCCESS);

        /* Read the bytes of the test file.  */
        memset(read_buffer, 0, read_buffer_size);  
        status = fx_file_read(&my_file, read_buffer, read_buffer_size, &actual);
        return_if_fail( (status == FX_SUCCESS) && (actual == 28));

        /* Close the test file.  */
        status = fx_file_close(&my_file);
        return_if_fail( status == FX_SUCCESS);

        /* Create a file called TEST1.TXT in the root directory.  */
        status = fx_file_create(&ram_disk, "TEST1.TXT");
        status += fx_file_open(&ram_disk, &my_file, "TEST1.TXT", FX_OPEN_FOR_WRITE);
        return_if_fail( status == FX_SUCCESS);

        /* Write a string to the test file.  */

        /* Random genearte the write data.  */
        for (write_index = 0; write_index < 1500; write_index ++)
        {
            write_buffer[write_index] = (CHAR)rand();

            /* Store the write data to compare the new data after Data write interrupt.  */
            data_buffer[write_index] = (UCHAR)write_buffer[write_index];               
            data_size = 1500;
        }

        /* Write the data into file.  */
        status = fx_file_write(&my_file, (void *) write_buffer, data_size);
        status += fx_file_seek(&my_file, 0);
        return_if_fail( status == FX_SUCCESS);

        /* Read the bytes of the test file.  */
        memset(read_buffer, 0, read_buffer_size);  
        status = fx_file_read(&my_file, read_buffer, read_buffer_size, &actual);
        return_if_fail( (status == FX_SUCCESS) && (actual == data_size));

        /* Close the test file.  */
        status = fx_file_close(&my_file);
        return_if_fail( status == FX_SUCCESS);

        /* Create a file called TEST2.TXT in the root directory.  */
        status = fx_file_create(&ram_disk, "TEST2.TXT");
        status += fx_file_open(&ram_disk, &my_file, "TEST2.TXT", FX_OPEN_FOR_WRITE);
        status += fx_file_write(&my_file, " EXPRESSLOGIC_TEST\n", 19);
        status += fx_file_seek(&my_file, 0);
        return_if_fail( status == FX_SUCCESS);

        /* Read the bytes of the test file.  */
        memset(read_buffer, 0, read_buffer_size);  
        status = fx_file_read(&my_file, read_buffer, read_buffer_size, &actual);
        return_if_fail( (status == FX_SUCCESS) && (actual == 19));

        /* Close the test file.  */
        status = fx_file_close(&my_file);
        return_if_fail( status == FX_SUCCESS);

        /* Create the main thread.  */
#ifndef FX_STANDALONE_ENABLE
        tx_thread_create(&ftest_1, "thread 1", ftest_1_entry, 0,  
                        thread_buffer, DEMO_STACK_SIZE, 
                        4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);
#endif

        /* directory_write_interrupt */
        data_write_interrupt = FX_FALSE;

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
        return_if_fail( status == FX_SUCCESS);

        /* Enable the Fault-tolerant feature to recover the media.  */          
        status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
        return_if_fail( status == FX_SUCCESS);

        /* Open the test file.  */
        status = fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
        status += fx_file_seek(&my_file, 0);
        return_if_fail( status == FX_SUCCESS);

        /* Read the bytes of the test file.  */
        memset(read_buffer, 0, read_buffer_size);  
        status = fx_file_read(&my_file, read_buffer, read_buffer_size, &actual);
        return_if_fail((status == FX_SUCCESS) && (actual == 28) && !memcmp(read_buffer, " ABCDEFGHIJKLMNOPQRSTUVWXYZ\n", actual));

        /* Close the test file.  */
        status = fx_file_close(&my_file);
        return_if_fail( status == FX_SUCCESS);

        /* Open the test file.  */
        status = fx_file_open(&ram_disk, &my_file, "TEST1.TXT", FX_OPEN_FOR_WRITE);
        status += fx_file_seek(&my_file, 0);
        return_if_fail( status == FX_SUCCESS);

        /* Read the bytes of the test file.  */
        memset(read_buffer, 0, read_buffer_size);  
        status = fx_file_read(&my_file, read_buffer, read_buffer_size, &actual);
        return_if_fail((status == FX_SUCCESS) && (actual == data_size) && !memcmp(read_buffer, data_buffer, actual));

        /* Close the test file.  */
        status = fx_file_close(&my_file);
        return_if_fail( status == FX_SUCCESS);

        /* Open the test file.  */
        status = fx_file_open(&ram_disk, &my_file, "TEST2.TXT", FX_OPEN_FOR_WRITE);
        status += fx_file_seek(&my_file, 0);
        return_if_fail( status == FX_SUCCESS);

        /* Read the bytes of the test file.  */
        memset(read_buffer, 0, read_buffer_size);
        status =  fx_file_read(&my_file, read_buffer, read_buffer_size, &actual);
        return_if_fail((status == FX_SUCCESS) && (actual == 19) && !memcmp(read_buffer, " EXPRESSLOGIC_TEST\n", actual));

        /* Close the test file.  */
        status = fx_file_close(&my_file);
        return_if_fail( status == FX_SUCCESS);

        /* Close the media.  */
        status =  fx_media_close(&ram_disk);
        return_if_fail((status == FX_SUCCESS) && (data_write_interrupt == FX_TRUE) && (error_couter == 0));

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
    driver_write_callback = my_driver_write;
                                         
    /* Open the test file.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST1.TXT", FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)           
    {
        error_couter ++;
#ifndef FX_STANDALONE_ENABLE
        return;
#else
        return NULL;
#endif
    }              

    /* Seek to the file. FAT 12, 16, 32 and exFAT.  */
    if ((i % SEEK_COUNT) == 0)
        status = fx_file_seek(&my_file, 0);
    else if ((i % SEEK_COUNT) == 1)
        status = fx_file_seek(&my_file, 1023);
    else if ((i % SEEK_COUNT) == 2)
        status = fx_file_seek(&my_file, 1024);
    else if ((i % SEEK_COUNT) == 3)
        status = fx_file_seek(&my_file, 1025);
    else if ((i % SEEK_COUNT) == 4)
        status = fx_file_seek(&my_file, 1500);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)    
    {
        error_couter ++;
#ifndef FX_STANDALONE_ENABLE
        return;
#else
        return NULL;
#endif
    }  

    /* Random genearte the write data.  */
    for (write_index = 0; write_index < write_buffer_size; write_index ++)
    {
        write_buffer[write_index] = (CHAR)rand();    
    }

    /* Write 1024 bytes to the file, then update the FAT table.  (bytes should be greate than one cluster).  */
    fx_file_write(&my_file, (void *) write_buffer, write_buffer_size);
}

static UINT my_driver_write(FX_MEDIA *media_ptr, UINT sector_type, UCHAR *block_ptr, UINT *operation_ptr)
{

    FX_PARAMETER_NOT_USED(block_ptr);

    /* Interrupt the Data write operation.  */
    if ((sector_type == FX_DATA_SECTOR) && (_filex_fault_tolerant_log_check(media_ptr) & FX_FAULT_TOLERANT_LOG_UNDO_DONE))
    {

        /* Set the write interrupt operation.  */
        *operation_ptr = FX_OP_WRITE_INTERRUPT;

        /* Update the flag.  */
        data_write_interrupt = FX_TRUE;   
                                             
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
void    filex_fault_tolerant_file_write_data_interrupt_test_application_define(void *first_unused_memory)
#endif
{
    
    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant File Write DATA Interrupt Test..........N/A\n"); 

    test_control_return(255);
}
#endif

