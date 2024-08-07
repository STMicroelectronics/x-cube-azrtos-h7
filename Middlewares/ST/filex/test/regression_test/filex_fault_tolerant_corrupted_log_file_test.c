/* This FileX test concentrates on validating the buffer overflow when Fault-Tolerant is enabled.  */
#if 0
*Description*
The Fault Tolerant feature of Azure RTOS FileX includes integer under and overflows which may be exploited to achieve buffer overflow and modify memory contents.
When a valid log file with correct ID and checksum is detected by the _fx_fault_tolerant_enable function an attempt to recover the previous failed write operation is taken by call of _fx_fault_tolerant_apply_logs. This function iterates through the log entries and performs required recovery operations. When properly crafted a log including entries of type FX_FAULT_TOLERANT_DIR_LOG_TYPE may be utilized to introduce unexpected behavior. Multiple values are retrieved from the log file thus controlled by a potential attacker, these include log total size (size variable), number of log entries (remaining_logs), entry type (log_type), length (log_len), copy_size etc. Certain combinations of values may be utilized to bypass validation by exploiting a integer overflow (and optionally underflow).
For example the comparison of copy_offset and copy_size against media_ptr -&gt; fx_media_memory_size may be bypassed by using a value of log_len smaller than FX_FAULT_TOLERANT_DIR_LOG_ENTRY_SIZE to cause a integer underflow and set copy_size to a unexpectedly large value. With correct selection of copy_offset the sum of copy_offset and copy_size will result in an integer overflow resulting in check bypass. Alternatively one may manipulate copy_offset and a not-underflown value of copy_size to again cause integer overflow of the sum. With a substantially large offset value the destination address of memcpy should also be overflown.
With the condition bypassed one may force a buffer overflow in the memcpy call with the possibility to manipulate both the destination address with copy_offset and amount of copied data with copy_size.
For example with log_len set to 15 copy_size will have the value of 4294967295 due to integer underflow,
with copy_offset set to 1 the result of sum of copy_size and copy_offset will be 0 and will result in bypass of the comparison against media_ptr -&gt; fx_media_memory_size allowing a buffer overflow.
```
    case FX_FAULT_TOLERANT_DIR_LOG_TYPE:
        /* This is a DIR log. */
        dir_log = (FX_FAULT_TOLERANT_DIR_LOG *)current_ptr;
        log_sector = _fx_utility_64_unsigned_read((UCHAR *)&amp;dir_log -&gt; fx_fault_tolerant_dir_log_sector);
        /* Get the destination sector. */
        status =  _fx_utility_logical_sector_read(media_ptr,
                                                    log_sector,
                                                    media_ptr -&gt; fx_media_memory_buffer,
                                                    ((ULONG) 1), FX_DATA_SECTOR);
        if (status != FX_SUCCESS)
        {
            /* Return the error status.  */
            return(status);
        }
        /* Set copy information. */
        copy_offset = _fx_utility_32_unsigned_read((UCHAR *)&amp;dir_log -&gt; fx_fault_tolerant_dir_log_offset);
        copy_size = log_len - FX_FAULT_TOLERANT_DIR_LOG_ENTRY_SIZE;
        if ((copy_offset + copy_size) &gt; media_ptr -&gt; fx_media_memory_size)
        {
            return(FX_FILE_CORRUPT);
        }
        /* Copy data into destination sector. */
        memcpy(media_ptr -&gt; fx_media_memory_buffer + copy_offset, /* Use case of memcpy is verified. */
                current_ptr + FX_FAULT_TOLERANT_DIR_LOG_ENTRY_SIZE, copy_size);
```
#endif

#include   "fx_api.h"    
#include   "fx_utility.h"
#include   "fx_fault_tolerant.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"               
extern void    test_control_return(UINT status);
void    filex_fault_tolerant_corrupted_log_file_test_application_define(void *first_unused_memory);
                                         
#if defined (FX_ENABLE_FAULT_TOLERANT) && defined (FX_FAULT_TOLERANT)

#define DEMO_STACK_SIZE         4096
#define CACHE_SIZE              2048
#define FAULT_TOLERANT_SIZE     FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD                ftest_0;
#endif
static FX_MEDIA                 ram_disk;
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
void    filex_fault_tolerant_corrupted_log_file_test_application_define(void *first_unused_memory)
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

UINT        status, temp = 0;
UINT        i = 0;
ULONG       sector;
FX_FAULT_TOLERANT_DIR_LOG *dir_log;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Corrupted Log File test.................");
                  
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
                                            512,                   // Sector size   
                                            4,                      // exFAT Sectors per cluster
                                            12345,                  // Volume ID
                                            0);                     // Boundary unit
        }
#endif
        return_if_fail( status == FX_SUCCESS);

        /* Open the ram_disk.  */
        status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
        return_if_fail( status == FX_SUCCESS);
        return_if_fail(ram_disk.fx_media_fault_tolerant_enabled != FX_TRUE);
        
        /* Enable the Fault-tolerant feature.  */                            
        status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
        return_if_fail( status == FX_SUCCESS);

        /* Start transaction. */
        _fx_fault_tolerant_transaction_start(&ram_disk);

        /* Save log pointer. */
        dir_log = (FX_FAULT_TOLERANT_DIR_LOG *)(ram_disk.fx_media_fault_tolerant_memory_buffer +
                                                ram_disk.fx_media_fault_tolerant_file_size);

        /* Create a DIR log entry with data that are no sense. */
        sector = ((ULONG)ram_disk.fx_media_data_sector_start) +
                 (((ULONG64)(ram_disk.fx_media_fault_tolerant_start_cluster + 1 - FX_FAT_ENTRY_START)) *
                  ((ULONG)ram_disk.fx_media_sectors_per_cluster));
        _fx_fault_tolerant_add_dir_log(&ram_disk, sector, 1, (UCHAR *)&temp, 1);

        /* Manipulate the log size to 15.  */
        _fx_utility_16_unsigned_write((UCHAR *)&dir_log -> fx_fault_tolerant_dir_log_size, 15);

        /* End transaction. */
        status = _fx_fault_tolerant_transaction_end(&ram_disk);
        return_if_fail( status == FX_FILE_CORRUPT);

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
void    filex_fault_tolerant_corrupted_log_file_test_application_define(void *first_unused_memory)
#endif
{

    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Corrupted Log File test.................N/A\n");

    test_control_return(255);
}
#endif
