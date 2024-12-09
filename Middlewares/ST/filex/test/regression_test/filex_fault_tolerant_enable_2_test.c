#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#include   "tx_thread.h"
#include   "tx_timer.h"
#endif
#include   "fx_api.h"    
#include   "fx_unicode.h"
#include   "fx_utility.h"
#include   "fx_fault_tolerant.h"
#include   <stdio.h>
#include   <string.h>
#include   "fx_ram_driver_test.h"               
extern void    test_control_return(UINT status);
void    filex_fault_tolerant_enable_2_test_application_define(void *first_unused_memory);
                                         
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
static CHAR                     read_buffer[10240];          

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
void    filex_fault_tolerant_enable_2_test_application_define(void *first_unused_memory)
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

FX_DIR_ENTRY dir_entry1, dir_entry2;
ULONG       uentry;
ULONG       unicode_size;
UINT        status, length;
ULONG       actual;
UCHAR       buffer[2048];   
CHAR        destination_name[100];
UCHAR       unicode_name_A[] = { 'A', 0, 0, 0};
UCHAR       unicode_name_1[] = { 1, 0, 0, 0};
UCHAR       unicode_name_2[] = { 1, 0, 1, 0, 0, 0};
UCHAR       unicode_name_A1[] = { 'A', 0, 1, 0, 0, 0};
UCHAR       *source_buffer;
USHORT      log_length;
FX_FILE     my_file;
ULONG       clusters;
ULONG       bytes_per_cluster;
ULONG       cluster;
FX_LOCAL_PATH local_path;
FX_FAULT_TOLERANT_FAT_CHAIN *FAT_chain;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Enable 2 Test...........................");
                  
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
    status +=  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Enable fault tolerant. */
    status = fx_fault_tolerant_enable( &ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);

    status = fx_file_create( &ram_disk, "TEST.TXT");
    status += fx_file_open( &ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_allocate( &my_file, 2048);
    return_if_fail( status == FX_SUCCESS);

    /* Set the state of fault tolerant as FX_FAULT_TOLERANT_STATE_STARTED and add an extra transaction record. */
    ram_disk.fx_media_fault_tolerant_state = FX_FAULT_TOLERANT_STATE_STARTED;
    ram_disk.fx_media_fault_tolerant_transaction_count = 1;

    /* Overflow the size of log file. */
    source_buffer = fault_tolerant_buffer + FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET + FX_FAULT_TOLERANT_LOG_CONTENT_HEADER_SIZE;
    log_length = (USHORT)_fx_utility_16_unsigned_read( source_buffer + 2);
    _fx_utility_16_unsigned_write( source_buffer + 2, 0xffff);
    status = fx_file_truncate_release( &my_file, 26);
    return_if_fail( status == FX_FILE_CORRUPT);

    /* Reset the log file to revert our modification and close transaction. */
    _fx_fault_tolerant_reset_log_file( &ram_disk);
    status = fx_file_truncate_release( &my_file, 26);
    return_if_fail( status == FX_SUCCESS);

    /* Deal with the tail. */
    status = fx_file_close( &my_file);
    status += fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

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
    status +=  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Enable fault tolerant. */
    status = fx_fault_tolerant_enable( &ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);

    length = fx_unicode_length_get( unicode_name_A);
    status = fx_unicode_directory_create( &ram_disk, unicode_name_A, length, destination_name);
#ifndef FX_STANDALONE_ENABLE
    status += fx_directory_local_path_set( &ram_disk, &local_path, destination_name);
#else
    status +=  fx_directory_default_set(&ram_disk, destination_name);
#endif
    return_if_fail( status == FX_SUCCESS);

    /* Create a lot of directries which makes the first level subdir occupy space more than a single cluster, which will leave fat logs in log file. */

    /* For our special implement of fx_unicode_xx_create, we can only 26 files/directories with the same length of unicode name. */
    for ( UINT i = 0; i < 26; i++)
    {
        length = fx_unicode_length_get( unicode_name_1);
        status = fx_unicode_directory_create( &ram_disk, unicode_name_1, length, destination_name);
        return_if_fail( status == FX_SUCCESS);
        unicode_name_1[0]++;
    }

    /* When more directories is attempted to be created, fat logs will be read in fx_directory_search and fx_directory_free_search. */
    for ( UINT i = 0; i < 6; i++)
    {
        length = fx_unicode_length_get( unicode_name_2);
        status = fx_unicode_directory_create( &ram_disk, unicode_name_2, length, destination_name);
        return_if_fail( status == FX_SUCCESS);
        unicode_name_2[0]++;
    }

    /* Make an I/O error while _fx_fault_tolerant_apply_log is calling _fx_utility_FAT_entry_write. */
    _fx_utility_fat_entry_write_error_request = 1;
    status = fx_unicode_directory_create( &ram_disk, unicode_name_2, length, destination_name);
    return_if_fail( status == FX_IO_ERROR);

    /* Make an I/O error while _fx_fault_tolerant_apply_log is calling _fx_utility_logical_sector_read. */
    _fx_utility_logical_sector_read_error_request = 339;
    status = fx_unicode_directory_create( &ram_disk, unicode_name_2, length, destination_name);
    return_if_fail( status == FX_IO_ERROR);

    /* Make an I/O error while _fx_fault_tolerant_apply_log is calling _fx_utility_logical_sector_write. */
    _fx_utility_logical_sector_write_error_request = 4;
    status = fx_unicode_directory_create( &ram_disk, unicode_name_2, length, destination_name);
    return_if_fail( status == FX_IO_ERROR);

#ifndef FX_DISABLE_CACHE
    /* Make an I/O error while _fx_fault_tolerant_apply_log is calling _fx_utility_logical_sector_flush. */
    _fx_utility_logical_sector_flush_error_request = 6;
    status = fx_unicode_directory_create( &ram_disk, unicode_name_2, length, destination_name);
    return_if_fail( status == FX_IO_ERROR);
#endif

    /* Start transaction manually. */
    ram_disk.fx_media_fault_tolerant_state = FX_FAULT_TOLERANT_STATE_STARTED;
    ram_disk.fx_media_fault_tolerant_transaction_count = 1;
    source_buffer = fault_tolerant_buffer + FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET + FX_FAULT_TOLERANT_LOG_CONTENT_HEADER_SIZE;

    /* Modify the length of the log to overflow the size of log file, which will be caught by _fx_fault_tolerant_apply_logs. */
    log_length = (USHORT)_fx_utility_16_unsigned_read( source_buffer + 2);
    _fx_utility_16_unsigned_write( source_buffer + 2, 0xffff);
    status = _fx_fault_tolerant_transaction_end( &ram_disk);
    return_if_fail( status == FX_FILE_CORRUPT);

    /* Recover log length. */
    _fx_utility_16_unsigned_write( source_buffer + 2, log_length);

    /* Start transaction manually. */
    ram_disk.fx_media_fault_tolerant_state = FX_FAULT_TOLERANT_STATE_STARTED;
    ram_disk.fx_media_fault_tolerant_transaction_count = 1;
    source_buffer = fault_tolerant_buffer + FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET + FX_FAULT_TOLERANT_LOG_CONTENT_HEADER_SIZE;

    /* Modify the type of the log as an undefined number, which will be caught by _fx_fault_tolerant_apply_logs. */
    _fx_utility_16_unsigned_write( source_buffer, 0xffff);
    status = _fx_fault_tolerant_transaction_end( &ram_disk);
    return_if_fail( status == FX_SECTOR_INVALID);

    /* The size of a dir log is larger than a sector which is invalid, which will be caught by _fx_fault_tolerant_apply_logs. */
    _fx_utility_16_unsigned_write( ram_disk.fx_media_fault_tolerant_memory_buffer + 4, 0xffff);
    source_buffer = ram_disk.fx_media_fault_tolerant_memory_buffer + FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET + FX_FAULT_TOLERANT_LOG_CONTENT_HEADER_SIZE;
    _fx_utility_16_unsigned_write( source_buffer, FX_FAULT_TOLERANT_DIR_LOG_TYPE);
    _fx_utility_16_unsigned_write( source_buffer + 2, 4096);
    _fx_utility_32_unsigned_write( source_buffer + 4, 0); /* sector_offset = 0 */
    _fx_utility_64_unsigned_write( source_buffer + 8, 0); /* log sector = 0 */
    status = _fx_fault_tolerant_apply_logs( &ram_disk);
    return_if_fail( status == FX_FILE_CORRUPT);

    /* _fx_fault_tolerant_add_dir_log always return FX_SUCCESS when data_size(the fourth argument) is zero. */
    status = _fx_fault_tolerant_add_dir_log( &ram_disk, 0, 0, FX_NULL, 0);
    return_if_fail( status == FX_SUCCESS);

    /* The size of a log overflow the size of log file. */
    ram_disk.fx_media_fault_tolerant_total_logs = 1;
    source_buffer = ram_disk.fx_media_fault_tolerant_memory_buffer + FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET + FX_FAULT_TOLERANT_LOG_CONTENT_HEADER_SIZE;
    _fx_utility_16_unsigned_write( source_buffer + 2, 0xffff);
    status = _fx_fault_tolerant_read_directory_sector( &ram_disk, 0, FX_NULL, 0);
    return_if_fail( status == FX_FILE_CORRUPT);

    /* The size of a dir log is larger than a sector which is invalid. */
    ram_disk.fx_media_fault_tolerant_file_size = 1024;
    ram_disk.fx_media_fault_tolerant_total_logs = 1;
    source_buffer = ram_disk.fx_media_fault_tolerant_memory_buffer + FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET + FX_FAULT_TOLERANT_LOG_CONTENT_HEADER_SIZE;
    _fx_utility_16_unsigned_write( source_buffer, FX_FAULT_TOLERANT_DIR_LOG_TYPE);
    _fx_utility_16_unsigned_write( source_buffer + 2, 512);
    _fx_utility_32_unsigned_write( source_buffer + 4, 0); /* sector_offset = 0 */
    _fx_utility_64_unsigned_write( source_buffer + 8, 0); /* log sector = 0 */
    status = _fx_fault_tolerant_read_directory_sector( &ram_disk, 0, FX_NULL, 1);
    return_if_fail( status == FX_FILE_CORRUPT);

    /* The size of a log overflow the size of log file. */
    ram_disk.fx_media_fault_tolerant_total_logs = 1;
    ram_disk.fx_media_fault_tolerant_state |= FX_FAULT_TOLERANT_STATE_STARTED;
    source_buffer = ram_disk.fx_media_fault_tolerant_memory_buffer + FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET + FX_FAULT_TOLERANT_LOG_CONTENT_HEADER_SIZE;
    _fx_utility_16_unsigned_write( source_buffer + 2, 0xffff);
    status = _fx_utility_logical_sector_read( &ram_disk, 2, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
    return_if_fail( status == FX_FILE_CORRUPT);

    _fx_fault_tolerant_recover( &ram_disk);

    /* Make a mistake in fx_utility_sector_read.c the last time calling _fx_utility_logical_sector_cache_entry_read. */
    _fx_utility_logical_sector_read_1_error_request = 1;
    status = _fx_utility_logical_sector_read( &ram_disk, 99, buffer, 1, FX_DIRECTORY_SECTOR);
    return_if_fail( status == FX_SUCCESS);

    status = fx_file_create( &ram_disk, "test.txt");
    status += fx_file_open( &ram_disk, &my_file, "test.txt", FX_OPEN_FOR_WRITE);
    status += fx_file_allocate( &my_file, 2048 * 3);
    status += fx_file_close( &my_file);
    return_if_fail( status == FX_SUCCESS);

    /* Register a bad fat entry in cache for the last cluster of the file. */
    ram_disk.fx_media_fat_cache[28].fx_fat_cache_entry_cluster = 39;
    ram_disk.fx_media_fat_cache[28].fx_fat_cache_entry_value = 1;

    /* File corruption is found while calling fx_file_open. */
    status = fx_file_open( &ram_disk, &my_file, "test.txt", FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_FILE_CORRUPT);

    status = fx_file_create( &ram_disk, "aa");
    status += fx_file_open( &ram_disk, &my_file, "aa", FX_OPEN_FOR_WRITE);
    status += fx_file_write( &my_file, read_buffer, 2048 * 3);
    return_if_fail( status == FX_SUCCESS);

    bytes_per_cluster = ram_disk.fx_media_bytes_per_sector * ram_disk.fx_media_sectors_per_cluster;
    clusters = (FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE + bytes_per_cluster - 1) / bytes_per_cluster;
    cluster = 40 + clusters;

#ifndef FX_DISABLE_CACHE
    /* Register a bad fat entry in cache in the middle of the file. */
    ram_disk.fx_media_fat_cache[(cluster & FX_FAT_CACHE_HASH_MASK) * FX_FAT_CACHE_DEPTH].fx_fat_cache_entry_cluster = cluster;
    ram_disk.fx_media_fat_cache[(cluster & FX_FAT_CACHE_HASH_MASK) * FX_FAT_CACHE_DEPTH].fx_fat_cache_entry_value = 0xffff;
    status += fx_file_seek( &my_file, 2047);
    status += fx_file_read( &my_file, read_buffer, 5, &actual);
    return_if_fail( status == FX_FILE_CORRUPT);
#endif

    status = fx_file_close( &my_file);
    return_if_fail( status == FX_SUCCESS);

    status = fx_file_create( &ram_disk, "tt");
    status += fx_file_open( &ram_disk, &my_file, "tt", FX_OPEN_FOR_WRITE);
    status += fx_file_allocate( &my_file, 2048 * 3);
    return_if_fail( status == FX_SUCCESS);

    /* Attempt to cover the branch at the end of fx_file_extended_truncate */
    /* that size >= file_ptr -> fx_file_maximum_size_used. */
    my_file.fx_file_maximum_size_used = 0;
    status = fx_file_extended_truncate( &my_file, 2048);
    status += fx_file_close( &my_file);
    return_if_fail( status == FX_SUCCESS);

    status = fx_file_create( &ram_disk, "test");
    status += fx_file_open( &ram_disk, &my_file, "test", FX_OPEN_FOR_WRITE);
    status += fx_file_allocate( &my_file, 2048);
    return_if_fail( status == FX_SUCCESS);

    /* Cover the branch in fx_file_extended_truncate_release that fx_file_current_offset <= fx_file_current_file_size. */
    status = fx_file_seek( &my_file, 0);
    status += fx_file_extended_truncate_release( &my_file, 1024);
    return_if_fail( status == FX_SUCCESS);

    /* Just cover the branch of FX_UTILITY_LOGICAL_SECTOR_READ_EXTENSION_1. */
    _fx_utility_logical_sector_read_1_error_request = 2;
    status = _fx_utility_logical_sector_read( &ram_disk, 99, buffer, 1, FX_DIRECTORY_SECTOR);

    /* Attempt to cover the last branch in _fx_unicode_directory_entry_read that cluster < FX_FAT_ENTRY_START. */
    uentry = 129;
    dir_entry1.fx_dir_entry_cluster = 2;
    unicode_size = fx_unicode_length_get( unicode_name_A1);

    /* Register an uncorrent fat cache to satisfy cluster < FX_FAT_ENTRY_START. */
    ram_disk.fx_media_fat_cache[8].fx_fat_cache_entry_cluster = 2;
    ram_disk.fx_media_fat_cache[8].fx_fat_cache_entry_value = 1;

    /* Make an IO error to return from the function earlier avoiding core dump. */
    _fx_utility_logical_sector_read_error_request = 1;
    status = _fx_unicode_directory_entry_read( &ram_disk, &dir_entry1, &uentry, &dir_entry2,
           unicode_name_A1, &unicode_size);
    return_if_fail( status == FX_FILE_CORRUPT);
    
    /* Modify fault tolerant FAT chain in buffer. */
    FAT_chain = (FX_FAULT_TOLERANT_FAT_CHAIN *)(ram_disk.fx_media_fault_tolerant_memory_buffer + FX_FAULT_TOLERANT_FAT_CHAIN_OFFSET);
    _fx_utility_32_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_head_new, 3);
    _fx_utility_32_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_next_deletion, ram_disk.fx_media_fat_reserved + 1);

    /* Get fx_fault_tolerant_FAT_chain_next_deletion while looping to cleanup FAT entries. */
    status = _fx_fault_tolerant_cleanup_FAT_chain( &ram_disk, FX_FAULT_TOLERANT_FAT_CHAIN_RECOVER);
    return_if_fail( status == FX_SUCCESS);
    
    /* Modify fault tolerant FAT chain in buffer. */
    FAT_chain = (FX_FAULT_TOLERANT_FAT_CHAIN *)(ram_disk.fx_media_fault_tolerant_memory_buffer + FX_FAULT_TOLERANT_FAT_CHAIN_OFFSET);
    _fx_utility_32_unsigned_write((UCHAR *)&FAT_chain -> fx_fault_tolerant_FAT_chain_head_new, 3);
    _fx_utility_FAT_entry_write(&ram_disk, 3, 1);

    /* Get fx_fault_tolerant_FAT_chain_next_deletion while looping to cleanup FAT entries. */
    status = _fx_fault_tolerant_cleanup_FAT_chain( &ram_disk, FX_FAULT_TOLERANT_FAT_CHAIN_RECOVER);
    return_if_fail( status == FX_SUCCESS);

    /* Output successful.  */     
    printf("SUCCESS!\n");
    test_control_return(0);
}         

#else  

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_enable_2_test_application_define(void *first_unused_memory)
#endif
{
    
    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Enable 2 Test...........................N/A\n");  

    test_control_return(255);
}
#endif
