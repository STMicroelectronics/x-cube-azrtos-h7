/* In this test point: */
/* Send null pointer to generate an error in fxe_fault_tolerant_enable */
/* Register an invalid cluster number for fault_tolerant log file and enable the Fault-tolerant feature.  */                            
/* Register an invalid cluster number for fault_tolerant log file and enable the Fault-tolerant feature, but 0xffffffff this time.  */
/* Enable fault tolerant and make an IO ERROR while reading the FAT entry log file. */
/* Enable fault tolerant and make an IO ERROR while reading log file. */
/* Insert an ucorrect FAT information in cache before enable fault tolerant feature. */
/* Modify header checksum in log file before enable fault tolerant. */
/* Modify fat chain checksum in log file before enable fault tolerant. */
/* Make an IO ERROR in fx_fault_tolerant_create_log_file while reading FAT entries. */
/* Make an IO ERROR in fx_fault_tolerant_create_log_file while writiing FAT entries in the method of FX_UTILITY_FAT_ENTRY_WRITE_EXTENSION. */
/* Make an IO ERROR in fx_fault_tolerant_create_log_file while writing start cluster of log file into boot sector. */
/* Set fx_media_cluster_search_start to enforce filesystem to search wrapping the disk for a free cluster for log file. */
/* Access all the fat chain to get the last free cluster for log file and filesystem set fx_media_cluster_search_start as FX_FAT_ENTRY_START. */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#include   "tx_thread.h"
#include   "tx_timer.h"
#endif
#include   "fx_api.h"    
#include   "fx_utility.h"
#include   "fx_fault_tolerant.h"
#include   <stdio.h>
#include   <string.h>
#include   "fx_ram_driver_test.h"               
extern void    test_control_return(UINT status);
void    filex_fault_tolerant_enable_1_test_application_define(void *first_unused_memory);
                                         
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
void    filex_fault_tolerant_enable_1_test_application_define(void *first_unused_memory)
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
    pointer += FAULT_TOLERANT_SIZE + 1024;
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
UCHAR       buffer[2048];   
ULONG       clusters;
ULONG       bytes_per_cluster;
FX_FILE     my_file;
FX_LOCAL_PATH local_path;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Enable 1 Test...........................");
                  
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

/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING

    /* send null pointer to generate an error */
    status = fx_fault_tolerant_enable( FX_NULL, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_PTR_ERROR);

    /* send null pointer to generate an error */
    status = fx_fault_tolerant_enable( &ram_disk, FX_NULL, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_PTR_ERROR);

#endif /* FX_DISABLE_ERROR_CHECKING */

    /* Register an invalid cluster number for fault_tolerant log file and enable the Fault-tolerant feature.  */                            
    _fx_utility_32_unsigned_write((UCHAR *)ram_disk.fx_media_driver_info + FX_FAULT_TOLERANT_BOOT_INDEX, (ULONG)1);
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_close( &ram_disk);
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

    /* Register an invalid cluster number for fault_tolerant log file and enable the Fault-tolerant feature, but 0xffffffff this time.  */
    _fx_utility_32_unsigned_write((UCHAR *)ram_disk.fx_media_driver_info + FX_FAULT_TOLERANT_BOOT_INDEX, (ULONG)0xffffffff);
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_close( &ram_disk);
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
                             12,                     // Sectors per cluster
                             1,                      // Heads
                             1);                     // Sectors per track 
    status +=  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Register an invalid cluster number for fault_tolerant log file and enable the Fault-tolerant feature, but the last FAT entry does not link to fat last.  */
    _fx_utility_32_unsigned_write((UCHAR *)ram_disk.fx_media_driver_info + FX_FAULT_TOLERANT_BOOT_INDEX, (ULONG)2);
    _fx_utility_FAT_entry_write( &ram_disk, 2, 3);
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format the media with FAT16 and enable fault tolerant. */
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
    status += fx_media_open( &ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    status += fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    status += fx_media_close( &ram_disk);
    status += fx_media_open( &ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Enable fault tolerant and make an IO ERROR while reading the FAT entry log file. */
    _fx_ram_driver_io_error_request = 2;
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_IO_ERROR);

    /* Enable fault tolerant and make an IO ERROR while reading log file. */
    _fx_ram_driver_io_error_request = 3;
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_IO_ERROR);

    /* Insert an ucorrect FAT information in cache. */
    ram_disk.fx_media_fat_cache[8].fx_fat_cache_entry_cluster = 2;
    ram_disk.fx_media_fat_cache[8].fx_fat_cache_entry_value = 5;

    /* Enable fault tolerant still successfully but original log file is lost. */
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format the media with FAT16 and enable fault tolerant. */
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
    status += fx_media_open( &ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    status += fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    status += fx_media_close( &ram_disk);
    status += fx_media_open( &ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* By default, log file locate at the first sector of user area. So we change the data in the log file to make checksum a non zero number. */
    _fx_utility_logical_sector_read( &ram_disk, 1 + ram_disk.fx_media_sectors_per_FAT + ram_disk.fx_media_root_sectors, buffer, (ULONG64)1, FX_DATA_SECTOR);

    /* Modify header checksum. */
    buffer[5]++;

    /* Write out our modification. */
    _fx_utility_logical_sector_write( &ram_disk, 1 + ram_disk.fx_media_sectors_per_FAT + ram_disk.fx_media_root_sectors, buffer, (ULONG64)1, FX_DATA_SECTOR);

    /* Access our log file now. */
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_open( &ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Modify log file again, but fat chain this time. */
    _fx_utility_logical_sector_read( &ram_disk, 1 + ram_disk.fx_media_sectors_per_FAT + ram_disk.fx_media_root_sectors, buffer, (ULONG64)1, FX_DATA_SECTOR);

    /* Modify FAT chain checksum. */
    buffer[FX_FAULT_TOLERANT_LOG_HEADER_SIZE]++;
    _fx_utility_logical_sector_write( &ram_disk, 1 + ram_disk.fx_media_sectors_per_FAT + ram_disk.fx_media_root_sectors, buffer, (ULONG64)1, FX_DATA_SECTOR);
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format the media with FAT16 and enable fault tolerant. */
    status = fx_media_format(&ram_disk, 
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
    status += fx_media_open( &ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Make an IO ERROR in fx_fault_tolerant_create_log_file while reading FAT entries. */
    _fx_ram_driver_io_error_request = 2;
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_IO_ERROR);

    /* Make an IO ERROR in fx_fault_tolerant_create_log_file while writiing FAT entries in the method of FX_UTILITY_FAT_ENTRY_WRITE_EXTENSION. */
    /* Honestly, _fx_utility_FAT_entry_write will not call driver at this particular position. */
    _fx_utility_fat_entry_write_error_request = 1;
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_IO_ERROR);

    /* Make an IO ERROR in fx_fault_tolerant_create_log_file while writing start cluster of log file into boot sector. */
    _fx_ram_driver_io_error_request = 3;
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_BOOT_ERROR);

    /* Make an IO ERROR in fx_fault_tolerant_create_log_file while writiing FAT entries in the method of FX_UTILITY_FAT_ENTRY_WRITE_EXTENSION. */
    /* Honestly, _fx_utility_FAT_entry_write will not call driver at this particular position. */
    _fx_utility_fat_entry_write_error_request = 2;
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_IO_ERROR);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format the media with FAT16 and enable fault tolerant. */
    status = fx_media_format(&ram_disk, 
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
    status += fx_media_open( &ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Create files in a subdir to fill user data area. */
    status = fx_directory_create( &ram_disk, "root");
#ifndef FX_STANDALONE_ENABLE
    status += fx_directory_local_path_set( &ram_disk, &local_path, "root");
#else
    status +=  fx_directory_default_set(&ram_disk, "/root");
#endif
    /* Fill all space so that there is no space left for fault tolerant log file. */
    status += fx_file_create( &ram_disk, "dev");
    status += fx_file_open( &ram_disk, &my_file, "dev", FX_OPEN_FOR_WRITE);
    status += fx_file_allocate( &my_file, 256 * 8 * 4000);
    status += fx_file_close( &my_file);
    status += fx_file_create( &ram_disk, "src");
    status += fx_file_open( &ram_disk, &my_file, "src", FX_OPEN_FOR_WRITE);
    status += fx_file_allocate( &my_file, 256 * 8 * 194);
    return_if_fail( status == FX_SUCCESS);

    /* No space is left! */
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_NO_MORE_SPACE);

    /* Delete a file to enable fault tolerant. */
    status = fx_file_delete( &ram_disk, "dev");
    return_if_fail( status == FX_SUCCESS);

    /* Set cluster_search_start as a cluster which is occupied by "src" to enforce filesystem to search wrapping the disk for a free cluster. */
    ram_disk.fx_media_cluster_search_start = 4100;
    status += fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format the media with FAT16 and enable fault tolerant. */
    status = fx_media_format(&ram_disk, 
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
    status += fx_media_open( &ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Create files in a subdir to fill user data area. */
    status = fx_directory_create( &ram_disk, "root");
#ifndef FX_STANDALONE_ENABLE
    status += fx_directory_local_path_set( &ram_disk, &local_path, "/root");
#else
    status +=  fx_directory_default_set(&ram_disk, "/root");
#endif
    return_if_fail( status == FX_SUCCESS);

    bytes_per_cluster = ram_disk.fx_media_bytes_per_sector * ram_disk.fx_media_sectors_per_cluster;
    clusters = (FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE + bytes_per_cluster - 1) / bytes_per_cluster;

    /* Fill user data area. Only the last cluster is left. */
    status = fx_file_create( &ram_disk, "dev");
    status += fx_file_open( &ram_disk, &my_file, "dev", FX_OPEN_FOR_WRITE);
    status += fx_file_allocate( &my_file, 256 * 8 * (4194 - clusters));
    return_if_fail( status == FX_SUCCESS);

    /* Access all the fat chain to get the last free cluster for log file and filesystem set fx_media_cluster_search_start as FX_FAT_ENTRY_START. */
    status = fx_fault_tolerant_enable( &ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Enable fault tolerant with buffer size larger than required. */
    status = fx_media_format(&ram_disk, 
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
                             4,                      // Sectors per cluster
                             1,                      // Heads
                             1);                     // Sectors per track 

    /* Create files in a subdir to fill user data area. */
    status += fx_media_open( &ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    status = fx_fault_tolerant_enable( &ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE + 1024);
    return_if_fail( status == FX_SUCCESS);

    return_if_fail(ram_disk.fx_media_fault_tolerant_memory_buffer_size == FAULT_TOLERANT_SIZE);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Output successful.  */     
    printf("SUCCESS!\n");
    test_control_return(0);
}         

#else  

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_enable_1_test_application_define(void *first_unused_memory)
#endif
{
    
    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Enable 1 Test...........................N/A\n");  

    test_control_return(255);
}
#endif
