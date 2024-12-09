#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#include   "tx_thread.h"
#include   "tx_timer.h"
#endif
#include   "fx_api.h"    
#include   "fx_directory.h"    
#include   "fx_utility.h"
#include   "fx_fault_tolerant.h"
#include   <stdio.h>
#include   <string.h>
#include   "fx_ram_driver_test.h"               
extern void    test_control_return(UINT status);
void    filex_fault_tolerant_enable_3_test_application_define(void *first_unused_memory);
                                         
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
static UCHAR                    *ram_disk_memory;
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
void    filex_fault_tolerant_enable_3_test_application_define(void *first_unused_memory)
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
    ram_disk_memory = pointer;
    pointer += (256 * 128);
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

ULONG64     actual_64;
UINT        status, length, new_length, old_length;
ULONG       actual;
ULONG       temp;
UCHAR       buffer[2048];   
CHAR        destination_name[100];
UCHAR       unicode_name_A[] = { 'A', 0, 0, 0};
UCHAR       unicode_name_1[] = { 1, 0, 0, 0};
UCHAR       unicode_name_2[] = { 1, 0, 1, 0, 0, 0};
UCHAR       unicode_name_3[] = { 1, 0, 1, 0, 1, 0, 0, 0};
UCHAR       unicode_name_14[] = { 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 0, 0};
UCHAR       unicode_name_A1[] = { 'A', 0, 1, 0, 0, 0};
UCHAR       unicode_name_A3[] = { 'A', 1, 2, 1, 0, 0};
UCHAR       long_unicode_name[300];
FX_FILE     my_file;
FX_LOCAL_PATH local_path;
FX_DIR_ENTRY dir_entry;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Enable 3 Test...........................");
                  
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

    /* Prepare a file for our test. */
    length = fx_unicode_length_get( unicode_name_2);
    status = fx_unicode_file_create( &ram_disk, unicode_name_2, length, destination_name);
    status += fx_file_open( &ram_disk, &my_file, destination_name, FX_OPEN_FOR_WRITE);
    status += fx_file_allocate( &my_file, 2048);
    status += fx_file_seek( &my_file, 2048);
    return_if_fail( status == FX_SUCCESS);

    /* Write some data append to the new file, which will call fx_fault_tolerant_cleanup_FAT_chain */
    /* Make IO error while reading FAT chain. */
    _fx_utility_fat_entry_read_error_request = 2;
    status += fx_file_write( &my_file, "1234567890", 10);
    return_if_fail( status == FX_IO_ERROR);

    status = fx_file_close( &my_file);
    return_if_fail( status == FX_SUCCESS);

    /* Make IO error while _fx_fault_tolerant_recover is calling _fx_fault_tolerant_cleanup_FAT_chain. */
    ram_disk.fx_media_driver_write_protect = FX_TRUE;
    /*_fx_utility_fat_entry_read_error_request = 1;*/
    status = fx_file_rename(&ram_disk, "MYTEST", "OURTEST");
    ram_disk.fx_media_driver_write_protect = FX_FALSE;
    return_if_fail( status == FX_WRITE_PROTECT);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            70000,                  // Total sectors - FAT32
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_if_fail( status == FX_SUCCESS);

    /* Now create a series to sub-directories to expand the root directory FAT chain.  */
    status =  fx_file_create(&ram_disk, "FILE1");
    
    /* Open the file.  */
    status += fx_file_open(&ram_disk, &my_file, "FILE1", FX_OPEN_FOR_WRITE); 
    
    /* Write to the file.  */
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO error while flushing dirty buffer in fx_utility_logical_sector_read.c. */
    _fx_ram_driver_io_error_request = 4;
    status = fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    return_if_fail( status == FX_IO_ERROR);

    status = fx_file_close( &my_file);
    status += fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);
    
    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            4200 * 8,               // Total sectors - FAT32
                            256,                    // Sector size   
                            8,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Enable fault tolerant. */
    status = fx_fault_tolerant_enable( &ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Attempt to truncate file which is open for read while fault tolerant is enabled. */
    status = fx_file_create( &ram_disk, "FILE2");
    status += fx_file_open( &ram_disk, &my_file, "FILE2", FX_OPEN_FOR_READ);
    status += fx_file_truncate_release( &my_file, 0);
    return_if_fail( status == FX_ACCESS_ERROR);

    /* Attempt to truncate file while fault tolerant is enabled and the disk is write protected. */
    status = fx_file_close( &my_file);
    status += fx_file_open( &ram_disk, &my_file, "FILE2", FX_OPEN_FOR_WRITE);
    ram_disk.fx_media_driver_write_protect = 1;
    status += fx_file_truncate_release( &my_file, 0);
    ram_disk.fx_media_driver_write_protect = 0;
    return_if_fail( status == FX_WRITE_PROTECT);

    status = fx_file_allocate( &my_file, 2048 * 2);
    return_if_fail( status == FX_SUCCESS);

    /* Make IO error in _fx_directory_entry_write. */
    _fx_utility_logical_sector_read_error_request = 1;
    status = fx_file_truncate_release( &my_file, 1000);
    return_if_fail( status == FX_IO_ERROR);

    /* Size expected is less the original file size. */
    status = fx_file_truncate_release( &my_file, 2048 * 3);
    status += fx_file_seek( &my_file, 2048 * 3);
    status += fx_file_truncate_release( &my_file, 2048 * 2);
    status += fx_file_close( &my_file);
    return_if_fail( status == FX_SUCCESS);

    /* Prepare a file for our test. */
    status = fx_file_create( &ram_disk, "FILE3");
    status += fx_file_open( &ram_disk, &my_file, "FILE3", FX_OPEN_FOR_WRITE);
    status += fx_file_allocate( &my_file, 2048 * 3);
    return_if_fail( status == FX_SUCCESS);

    /* Make IO error while first reading current cluster entry from FAT chain. */
    _fx_utility_fat_entry_read_error_request = 3;
    status = fx_file_truncate_release( &my_file, 2048);
    return_if_fail( status == FX_IO_ERROR);

    /* Allocate 5 clusters for our new file. */
    status = fx_file_allocate( &my_file, 5 * 2048);
    return_if_fail( status == FX_SUCCESS);

    /* Make IO error while reading more cluster entries from FAT chain. */
    _fx_utility_fat_entry_read_error_request = 4;
    status = fx_file_truncate_release( &my_file, 2048);
    return_if_fail( status == FX_IO_ERROR);

    /* Allocate 5 clusters for our new file. */
    status = fx_file_allocate( &my_file, 5 * 2048);
    return_if_fail( status == FX_SUCCESS);

    /* Make IO error while setting undo log in FAT chain. */
    _fx_utility_logical_sector_write_error_request = 1;
    status = fx_file_truncate_release( &my_file, 2048);
    return_if_fail( status == FX_IO_ERROR);

    /* Allocate 5 clusters for our new file. */
    status = fx_file_allocate( &my_file, 5 * 2048);
    return_if_fail( status == FX_SUCCESS);

    /* Make IO error while setting the end of the FAT chain. */
    _fx_utility_fat_entry_write_error_request = 1;
    status = fx_file_truncate_release( &my_file, 2048);
    return_if_fail( status == FX_IO_ERROR);

    /* Allocate 5 clusters for our new file. */
    status = fx_file_allocate( &my_file, 5 * 2048);
    return_if_fail( status == FX_SUCCESS);

    /* Make IO error while flushing cached FAT entries. */
    _fx_utility_logical_sector_read_error_request = 2;
    status = fx_file_truncate_release( &my_file, 2048);
    return_if_fail( status == FX_IO_ERROR);

    /* Allocate 5 clusters for our new file. */
    status = fx_file_allocate( &my_file, 5 * 2048);
    return_if_fail( status == FX_SUCCESS);

    /* Make IO error at the end of fx_file_truncate_release while writing directory entries to the media. */
    _fx_utility_logical_sector_read_error_request = 3;
    status = fx_file_truncate_release( &my_file, 2048);
    return_if_fail( status == FX_IO_ERROR);

    /* Allocate 5 clusters for our new file. */
    status = fx_file_allocate( &my_file, 5 * 2048);
    return_if_fail( status == FX_SUCCESS);

    /* Make IO error in fx_file_truncate_release while calling _fx_fault_tolerant_transaction_end. */
    _fx_utility_logical_sector_read_error_request =  4;
    status = fx_file_truncate_release( &my_file, 2048);
    return_if_fail( status == FX_IO_ERROR);

    status = fx_file_close( &my_file);
    status += fx_media_close( &ram_disk);

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            4200 * 8,               // Total sectors - FAT32
                            256,                    // Sector size   
                            8,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    status = fx_file_create( &ram_disk, "TEST");
    status += fx_file_open( &ram_disk, &my_file, "TEST", FX_OPEN_FOR_WRITE);
    status += fx_file_allocate( &my_file, 2048 * 5);
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO error at the end of fx_file_extended_truncate_release while calling _fx_utility_FAT_flush. */
    /* which can only be reached when FX_FAULT_TOLERANT is defined but not enabled since fat entries will never be set as dirty while fault tolerant is enabled. */
    /* We add fat log instead of setting fx_fat_cache_entry_dirty while fault tolerant is enabled. */
    _fx_utility_logical_sector_read_error_request = 3;
    status = fx_file_truncate_release( &my_file, 2048);
    return_if_fail( status == FX_IO_ERROR);

    status = fx_file_close( &my_file);
    status += fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            4200 * 8,               // Total sectors - FAT32
                            256,                    // Sector size   
                            8,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Enable fault tolerant. */
    status = fx_fault_tolerant_enable( &ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO error in fx_directory_create while writing back directory sectors. */
    _fx_utility_logical_sector_write_error_request = 1;
    status = fx_directory_create( &ram_disk, "dev");
    return_if_fail( status == FX_IO_ERROR);

#ifndef FX_DISABLE_CACHE
    /* Make an IO error in fx_directory_create while flushing internal logical sector cache. */
    _fx_utility_logical_sector_flush_error_request = 1;
    status = fx_directory_create( &ram_disk, "dev");
    return_if_fail( status == FX_IO_ERROR);
#endif

    /* Make an IO error at the end of fx_directory_create while writing back new directory sector. */
    _fx_utility_logical_sector_read_error_request = 10;
    status = fx_directory_create( &ram_disk, "dev");
    return_if_fail( status == FX_IO_ERROR);

    status = fx_file_create( &ram_disk, "TEST");
    status += fx_file_open( &ram_disk, &my_file, "TEST", FX_OPEN_FOR_WRITE);
    status += fx_file_allocate( &my_file, 2048 * 3);
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO error at the end of fx_file_extended_truncate while calling _fx_directory_entry_write. */
    _fx_utility_logical_sector_read_error_request = 1;
    status = fx_file_extended_truncate( &my_file, 2048 * 2);
    return_if_fail( status == FX_IO_ERROR);

    /* Make an IO error in fx_file_extended_truncate while calling _fx_fault_tolerant_transaction_end. */
    _fx_utility_logical_sector_read_error_request = 2;
    status = fx_file_extended_truncate( &my_file, 2048);
    return_if_fail( status == FX_IO_ERROR);

    status = fx_file_allocate( &my_file, 2048 * 2);
    status += fx_file_close( &my_file);
    return_if_fail( status == FX_SUCCESS);

    /* Open the file we just closed which will update fx_file_maximum_size_used of the file. */
    status = fx_file_open( &ram_disk, &my_file, "TEST", FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_SUCCESS);

    /* Truncated the file size less than fx_file_maximum_size_used. */
    status = fx_file_extended_truncate( &my_file, 1024);
    return_if_fail( status == FX_SUCCESS);

    status = fx_file_close( &my_file);
    return_if_fail( status == FX_SUCCESS);

    status = fx_file_create( &ram_disk, "FILE");
    status += fx_file_open( &ram_disk, &my_file, "FILE", FX_OPEN_FOR_WRITE);
    status += fx_file_allocate( &my_file, 2048 * 3);
    status += fx_file_extended_truncate_release( &my_file, 0);
    return_if_fail( status == FX_SUCCESS);

    /* Extend the file which have no clusters, which need to sep FAT chain. */
    status = fx_file_extended_best_effort_allocate( &my_file, 2048 * 4, &actual_64);
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO error while writing the directory back to the media. */
    _fx_utility_logical_sector_read_error_request = 7;
    status = fx_file_extended_best_effort_allocate( &my_file, 2048 * 5, &actual_64);
    return_if_fail( status == FX_IO_ERROR);

#ifndef FX_DISABLE_CACHE
    /* Make an IO error at the end of fx_file_extended_best_effort while flushing sectors. */
    _fx_utility_logical_sector_flush_error_request = 3;
    status = fx_file_extended_best_effort_allocate( &my_file, 2048 * 5, &actual_64);
    return_if_fail( status == FX_IO_ERROR);
#endif

    /* Make an IO error in fx_unicode_file_create while calling _fx_directory_search. */
    length = fx_unicode_length_get( unicode_name_A);
    _fx_utility_logical_sector_read_error_request = 14;
    status = fx_unicode_file_create( &ram_disk, unicode_name_A, length, destination_name);
    return_if_fail( status == FX_IO_ERROR);

    /* Make an IO error in fx_unicode_directory_create while calling _fx_directory_search. */
    length = fx_unicode_length_get( unicode_name_A);
    _fx_utility_logical_sector_read_error_request = 23;
    status = fx_unicode_directory_create( &ram_disk, unicode_name_A, length, destination_name);
    return_if_fail( status == FX_IO_ERROR);

#ifndef FX_DISABLE_CACHE
    /* Make an IO error in fx_unicode_file_create while calling _fx_unicode_directory_entry_change. */
    length = fx_unicode_length_get( unicode_name_A3);
    _fx_utility_logical_sector_read_error_request = 25;
    status = fx_unicode_file_create( &ram_disk, unicode_name_A3, length, destination_name);
    return_if_fail( status == FX_IO_ERROR);

    /* Make an IO error in fx_unicode_directory_create while calling _fx_unicode_directory_entry_change. */
    unicode_name_A3[0]++;
    length = fx_unicode_length_get( unicode_name_A3);
    _fx_utility_logical_sector_read_error_request = 32;
    status = fx_unicode_directory_create( &ram_disk, unicode_name_A3, length, destination_name);
    return_if_fail( status == FX_IO_ERROR);
#endif

    status = fx_file_close( &my_file);
    status += fx_file_create( &ram_disk, "HELLO");
    status += fx_file_open( &ram_disk, &my_file, "HELLO", FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO error in fx_file_extended_allocate while calling _fx_directory_entry_write. */
    _fx_utility_logical_sector_read_error_request = 2;
    status = fx_file_extended_allocate( &my_file, 200);
    return_if_fail( status == FX_IO_ERROR);

#ifndef FX_DISABLE_CACHE
    /* Make an IO error in fx_file_extended_allocate while calling _fx_utility_logical_sector_flush. */
    _fx_utility_logical_sector_flush_error_request = 3;
    status = fx_file_extended_allocate( &my_file, 200);
    return_if_fail( status == FX_IO_ERROR);
#endif

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            4200 * 8,               // Total sectors - FAT32
                            256,                    // Sector size   
                            8,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Enable fault tolerant. */
    status = fx_fault_tolerant_enable( &ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);

    length = fx_unicode_length_get( unicode_name_1);
    status = fx_unicode_file_create( &ram_disk, unicode_name_1, length , destination_name);
    return_if_fail( status == FX_SUCCESS);

    old_length = fx_unicode_length_get( unicode_name_1);
    new_length = fx_unicode_length_get( unicode_name_2);

    /* Make an IO error at the end of fx_unicode_file_rename while calling _fx_directory_search. */
    _fx_utility_logical_sector_read_error_request = 14;
    status = fx_unicode_file_rename( &ram_disk, unicode_name_1, old_length, unicode_name_2, new_length, destination_name);
    return_if_fail( status == FX_IO_ERROR);
    
    length = fx_unicode_length_get( unicode_name_A);
    status = fx_unicode_directory_create( &ram_disk, unicode_name_A, length, destination_name);
    return_if_fail( status == FX_SUCCESS);

    old_length = fx_unicode_length_get( unicode_name_A);
    new_length = fx_unicode_length_get( unicode_name_A1);

    /* Make an IO error at the end of fx_unicode_directory_rename while calling _fx_directory_search. */
    _fx_utility_logical_sector_read_error_request = 20;
    status = fx_unicode_directory_rename( &ram_disk, unicode_name_A, old_length, unicode_name_A1, new_length, destination_name);
    return_if_fail( status == FX_IO_ERROR);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            4200 * 8,               // Total sectors - FAT32
                            256,                    // Sector size   
                            8,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Enable fault tolerant. */
    status = fx_fault_tolerant_enable( &ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);

    length = fx_unicode_length_get( unicode_name_1);
    status = fx_unicode_file_create( &ram_disk, unicode_name_1, length , destination_name);
    return_if_fail( status == FX_SUCCESS);

    old_length = fx_unicode_length_get( unicode_name_1);
    new_length = fx_unicode_length_get( unicode_name_2);

    /* Make an IO error at the end of fx_unicode_file_rename while calling _fx_unicode_directory_entry_change. */
    _fx_utility_logical_sector_read_error_request = 17;
    status = fx_unicode_file_rename( &ram_disk, unicode_name_1, old_length, unicode_name_2, new_length, destination_name);
    return_if_fail( status == FX_IO_ERROR);

    length = fx_unicode_length_get( unicode_name_A);
    status = fx_unicode_directory_create( &ram_disk, unicode_name_A, length, destination_name);
    return_if_fail( status == FX_SUCCESS);

    old_length = fx_unicode_length_get( unicode_name_A);
    new_length = fx_unicode_length_get( unicode_name_A1);

#ifndef FX_DISABLE_CACHE
    /* Make an IO error at the end of fx_unicode_directory_rename while calling _fx_unicode_directory_entry_change. */
    _fx_utility_logical_sector_read_error_request = 24;
    status = fx_unicode_directory_rename( &ram_disk, unicode_name_A, old_length, unicode_name_A1, new_length, destination_name);
    return_if_fail( status == FX_IO_ERROR);

    status = fx_directory_create( &ram_disk, "src");
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO error at the end of fx_file_delete while calling _fx_utility_logical_sector_flush. */
    _fx_utility_logical_sector_flush_error_request = 1;
    status += fx_directory_delete( &ram_disk, "src");
    return_if_fail( status == FX_IO_ERROR);
#endif

    status = fx_directory_create( &ram_disk, "dev");
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO error at the end of fx_directory_attributes_set while calling _fx_directory_entry_write. */
    _fx_utility_logical_sector_read_error_request = 2;
    status = fx_directory_attributes_set( &ram_disk, "dev", FX_DIRECTORY | FX_ARCHIVE | FX_SYSTEM | FX_READ_ONLY | FX_HIDDEN);
    return_if_fail( status == FX_IO_ERROR);

    status = fx_file_create( &ram_disk, "test.bat");
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO error at the end of fx_file_attributes_set while calling _fx_directory_entry_write. */
    _fx_utility_logical_sector_read_error_request = 5;
    status = fx_file_attributes_set( &ram_disk, "test.bat", FX_READ_ONLY);
    return_if_fail( status == FX_IO_ERROR);

    status = fx_file_create( &ram_disk, "new_file");
    status += fx_file_open( &ram_disk, &my_file, "new_file", FX_OPEN_FOR_WRITE);
    status += fx_file_write( &my_file, "1234567890", 10);
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO error in fx_media_flush while calling _fx_directory_entry_write. */
    _fx_utility_logical_sector_read_error_request = 1;
    status = fx_media_flush( &ram_disk);
    return_if_fail( status == FX_IO_ERROR);

#ifndef FX_DISABLE_CACHE
    /* Make an IO error in fx_media_flush while calling _fx_utility_logical_sector_flush. */
    _fx_utility_logical_sector_flush_error_request = 1;
    status = fx_media_flush( &ram_disk);
    return_if_fail( status == FX_IO_ERROR);
#endif

    status = fx_directory_create( &ram_disk, "root");
#ifndef FX_STANDALONE_ENABLE
    status += fx_directory_local_path_set( &ram_disk, &local_path,  "root");
#else
    status +=  fx_directory_default_set(&ram_disk,  "/root");
#endif
    return_if_fail( status == FX_SUCCESS);

    /* Create enough directory to overflow one single cluster. */
    length = fx_unicode_length_get( unicode_name_1);
    for ( UINT i = 0; i < 26; i++)
    {
        status = fx_unicode_directory_create( &ram_disk, unicode_name_1, length, destination_name);
        unicode_name_1[0]++;
        return_if_fail( status == FX_SUCCESS);
    }

    length = fx_unicode_length_get( unicode_name_2);
    for ( UINT i = 0; i < 6; i++)
    {
        status = fx_unicode_directory_create( &ram_disk, unicode_name_2, length, destination_name);
        unicode_name_2[0]++;
        return_if_fail( status == FX_SUCCESS);
    }

    /* Make an IO error in _fx_unicode_directory_entry_read while calling _fx_utility_FAT_entry_read to access fat chain. */
    _fx_utility_fat_entry_read_error_request = 3;
    length = fx_unicode_length_get( unicode_name_3);
    status = fx_unicode_directory_create( &ram_disk, unicode_name_3, length, destination_name);
    return_if_fail( status == FX_SUCCESS);

    status = fx_directory_create( &ram_disk, "root");
#ifndef FX_STANDALONE_ENABLE
    status += fx_directory_local_path_set( &ram_disk, &local_path,  "root");
#else
    status +=  fx_directory_default_set(&ram_disk,  "/root");
#endif
    return_if_fail( status == FX_SUCCESS);

    memset( long_unicode_name, 1, 270);
    long_unicode_name[270] = 0;
    long_unicode_name[271] = 0;
    length = fx_unicode_length_get( long_unicode_name);
    return_if_fail( length = 135);

    /* Created six directories in a subdir including 12 dir_entries each to overflow a single cluster. */
    temp = long_unicode_name[0];
    for ( UINT i = 0; i < 6; i++)
    {
        status = fx_unicode_directory_create( &ram_disk, long_unicode_name, length, destination_name);
        return_if_fail( status == FX_SUCCESS);
        long_unicode_name[0]++;
    }

    /* Access the directory divided into two clusters and make an IO error while getting next cluster in _fx_unicode_directory_entry_read. */
    /* We still create the directory successfully. */
    _fx_utility_fat_entry_read_error_request = 3;
    status = fx_unicode_directory_create( &ram_disk, long_unicode_name, length, destination_name);
    return_if_fail( status == FX_SUCCESS);

    /* Switch to root directory. */
#ifndef FX_STANDALONE_ENABLE
    status += fx_directory_local_path_set( &ram_disk, &local_path,  "/");
#else
    status +=  fx_directory_default_set(&ram_disk,  "/");
#endif
    status += fx_unicode_directory_create( &ram_disk, long_unicode_name, length, destination_name);
    long_unicode_name[0]++;
    return_if_fail( status == FX_SUCCESS);

    /* There is no sectors in root directory. */
    temp = ram_disk.fx_media_root_sectors;
    ram_disk.fx_media_root_sectors = 0;
    status = fx_unicode_directory_create( &ram_disk, long_unicode_name, length, destination_name);
    ram_disk.fx_media_root_sectors = temp;
    return_if_fail( status == FX_FILE_CORRUPT);

    status = fx_file_close( &my_file);
    return_if_fail( status == FX_SUCCESS);

    status = fx_file_create( &ram_disk, "config.h");
    status += fx_file_open( &ram_disk, &my_file, "config.h", FX_OPEN_FOR_WRITE);
    status += fx_file_write( &my_file, read_buffer, 2048 * 2);
    status += fx_file_seek( &my_file, 2047);

    /* Make an IO error in fx_file_read while calling _fx_utility_FAT_entry_read. */
    _fx_utility_fat_entry_read_error_request = 1;
    status += fx_file_read( &my_file, read_buffer, 10, &actual);
    return_if_fail( status == FX_IO_ERROR);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            4200 * 8,               // Total sectors - FAT32
                            256,                    // Sector size   
                            8,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    length = fx_unicode_length_get( unicode_name_14);
    for ( UINT i = 0; i < 2; i++)
    {
        status = fx_unicode_file_create( &ram_disk, unicode_name_14, length, destination_name);
        return_if_fail( status == FX_SUCCESS);
        unicode_name_14[0]++;
    }

    /* Make an IO error in fx_unicode_directory_entry_change while calling _fx_utility_logical_sector_write. */
    _fx_utility_logical_sector_write_error_request = 6;
    length = fx_unicode_length_get( unicode_name_14);
    status = fx_unicode_file_create( &ram_disk, unicode_name_14, length, destination_name);
    unicode_name_14[0]++;
    return_if_fail( status == FX_IO_ERROR);

    length = fx_unicode_length_get( unicode_name_14);
    for ( UINT i = 0; i < 2; i++)
    {
        status = fx_unicode_file_create( &ram_disk, unicode_name_14, length, destination_name);
        return_if_fail( status == FX_SUCCESS);
        unicode_name_14[0]++;
    }

    /* Make an IO error in fx_unicode_directory_entry_change while reading next logical sector. */
    _fx_utility_logical_sector_read_error_request = 38;
    status = fx_unicode_file_create( &ram_disk, unicode_name_14, length, destination_name);
    return_if_fail( status == FX_IO_ERROR);
    unicode_name_14[0]++;

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            4200 * 8,               // Total sectors - FAT32
                            256,                    // Sector size   
                            8,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Enable fault tolerant. */
    status = fx_fault_tolerant_enable( &ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);
    
    status = fx_file_create( &ram_disk, "src");
    status += fx_file_open( &ram_disk, &my_file, "src", FX_OPEN_FOR_WRITE);
    status += fx_file_allocate( &my_file, 2048 * 2);
    status += fx_file_seek( &my_file, 2047);
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO error in _fx_fault_tolerant_cleanup_FAT_chain while calling _fx_utility_FAT_entry_write. */
    _fx_utility_fat_entry_write_error_request = 3;
    status += fx_file_write( &my_file, "hello", 5);
    status += fx_file_close( &my_file);
    return_if_fail( status == FX_IO_ERROR);

    status = fx_file_create( &ram_disk, "world");
    status += fx_file_open( &ram_disk, &my_file, "world", FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_SUCCESS);

    /* Make IO error in _fx_fault_tolerant_transaction_end while flushing first cluster. */
    _fx_ram_driver_io_error_request = 5;
    status = fx_file_write( &my_file, "hello", 5);
    return_if_fail( status == FX_IO_ERROR);

    status = fx_file_close( &my_file);
    return_if_fail( status == FX_SUCCESS);

#ifndef FX_DISABLE_CACHE
    /* Make IO error in _fx_fault_tolerant_transaction_end while writing back log file. */
    _fx_ram_driver_io_error_request = 11;
    status = fx_directory_create( &ram_disk, "dev");
    return_if_fail( status == FX_IO_ERROR);
#endif

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            4200 * 8,               // Total sectors - FAT32
                            256,                    // Sector size   
                            8,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Enable fault tolerant. */
    status = fx_fault_tolerant_enable( &ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);
    
    status = fx_file_create( &ram_disk, "test.txt");
    status += fx_file_open( &ram_disk, &my_file, "test.txt", FX_OPEN_FOR_WRITE);
    status += fx_file_extended_best_effort_allocate( &my_file, 2048 * 4, &actual_64);

    /* Attempt to cover the branch in _fx_file_write which needs file_ptr -> fx_file_current_relative_cluster < file_ptr -> fx_file_consecutive_cluster. */
    /* Current offset is in the cluster in the middle of some consecutive cluster at the head of file. */
    status = fx_file_seek( &my_file, 2048 + 20);
    status += fx_file_write( &my_file, "hello", 5);
    return_if_fail( status == FX_SUCCESS);

    status = fx_file_close( &my_file);
    return_if_fail( status == FX_SUCCESS);
    
    status = fx_file_create( &ram_disk, "test2.txt");
    status += fx_file_open( &ram_disk, &my_file, "test2.txt", FX_OPEN_FOR_WRITE);
    status += fx_file_extended_best_effort_allocate( &my_file, 2048 * 4, &actual_64);
    return_if_fail( status == FX_SUCCESS);

    status = fx_file_close( &my_file);
    return_if_fail( status == FX_SUCCESS);

    /* Covered the branch in fx_file_write.c while adding dir log. */
    ram_disk.fx_media_fault_tolerant_state |= FX_FAULT_TOLERANT_STATE_STARTED;
    dir_entry.fx_dir_entry_long_name_present = 0;
    dir_entry.fx_dir_entry_short_name[0] = 'A';
    dir_entry.fx_dir_entry_short_name[1] = 0;
    dir_entry.fx_dir_entry_name = (CHAR *)buffer;
    dir_entry.fx_dir_entry_name[0] = 'A';
    dir_entry.fx_dir_entry_name[1] = 0;
    dir_entry.fx_dir_entry_byte_offset = 512;
    dir_entry.fx_dir_entry_log_sector = ram_disk.fx_media_data_sector_start + 1;
    dir_entry.fx_dir_entry_long_name_shorted = 1;
    _fx_directory_entry_write( &ram_disk, &dir_entry);
    
    status += fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            70000,               // Total sectors - FAT32
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_read( &ram_disk, 1, (VOID *)read_buffer);
    return_if_fail( status == FX_SUCCESS);

    /* Format the disk as FAT32 and assign the FAT entry of root cluster as itself. */
    read_buffer[8] = 2;
    read_buffer[9] = 0;
    read_buffer[10] = 0;
    read_buffer[11] = 0;
    status = fx_media_write( &ram_disk, 1, (VOID *)read_buffer);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_close( &ram_disk);
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_FAT_READ_ERROR);
    
    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
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
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Enable fault tolerant. */
    status = fx_fault_tolerant_enable( &ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);

    status = fx_file_create( &ram_disk, "test");
    status += fx_file_open( &ram_disk, &my_file, "test", FX_OPEN_FOR_WRITE);
    status += fx_file_allocate( &my_file, 2048 * 6);
    status += fx_file_seek( &my_file, 1);

    /* Make a mistake(cluster < FX_FAT_ENTRY_START) while looping the link of FAT to find the previous cluster of copy_head_cluster. */
    ram_disk.fx_media_fat_cache[12].fx_fat_cache_entry_cluster = 3;
    ram_disk.fx_media_fat_cache[12].fx_fat_cache_entry_value = 1;
    status = fx_file_write( &my_file, read_buffer, 2048 * 3);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
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
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Enable fault tolerant. */
    status = fx_fault_tolerant_enable( &ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);

    status = fx_file_create( &ram_disk, "test");
    status += fx_file_open( &ram_disk, &my_file, "test", FX_OPEN_FOR_WRITE);
    status += fx_file_allocate( &my_file, 2048 * 6);
    status += fx_file_seek( &my_file, 1);

    /* Make a mistake(cluster > media_ptr -> fx_media_fat_reserved) while looping the link of FAT to find the previous cluster of copy_head_cluster. */
    ram_disk.fx_media_fat_cache[12].fx_fat_cache_entry_cluster = 3;
    ram_disk.fx_media_fat_cache[12].fx_fat_cache_entry_value = ram_disk.fx_media_fat_reserved + 1;
    status = fx_file_write( &my_file, read_buffer, 2048 * 3);

    /* Make an IO error in fx_file_rename while calling _fx_directory_entry_write. */
    _fx_utility_logical_sector_read_error_request = 24;
    status = fx_file_create( &ram_disk, "test1");
    status += fx_file_rename( &ram_disk, "test1", "test2");
    return_if_fail( status == FX_IO_ERROR);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

#ifndef FX_ENABLE_EXFAT
    /* Format a FAT32 disk with 512 bytes sector.  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            70000 * 8,               // Total sectors
                            512,                    // Sector size   
                            8,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO error while reading FAT chain. */
    _fx_ram_driver_io_error_request = 1;
    status = fx_fault_tolerant_enable( &ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    _fx_ram_driver_io_error_request = 0;
    return_if_fail( status == FX_FAT_READ_ERROR);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format a FAT16 disk with 512 bytes sector.  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            70000 * 8,               // Total sectors
                            512,                    // Sector size   
                            8,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO error while reading FAT chain. */
    ram_disk.fx_media_cluster_search_start = 0;
    status = fx_fault_tolerant_enable( &ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);
#endif /* ifndef FX_ENABLE_EXFAT */

    /* Output successful.  */
    printf("SUCCESS!\n");
    test_control_return(0);
}         

#else  

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_enable_3_test_application_define(void *first_unused_memory)
#endif
{
    
    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Enable 3 Test...........................N/A\n");  

    test_control_return(255);
}
#endif
