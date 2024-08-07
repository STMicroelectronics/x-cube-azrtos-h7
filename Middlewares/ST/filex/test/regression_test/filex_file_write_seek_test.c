/* This FileX test concentrates on the file seek and write operation.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_utility.h"
#include   <stdio.h>
#include   "fx_fault_tolerant.h"
#include    "fx_ram_driver_test.h"

#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              128*128
#ifdef FX_ENABLE_FAULT_TOLERANT
#define     FAULT_TOLERANT_SIZE     FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE
#else
#define     FAULT_TOLERANT_SIZE     0
#endif


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD               ftest_0;
#endif
static FX_MEDIA                ram_disk;
static FX_FILE                 my_file;

/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *ram_disk_memory;
static UCHAR                  *cache_buffer;
#else
static UCHAR                   cache_buffer[CACHE_SIZE];
#endif
static UCHAR                   write_data[65535];
static UCHAR                   read_data[65535];
#ifdef FX_ENABLE_FAULT_TOLERANT
static UCHAR                   fault_tolerant_buffer[FAULT_TOLERANT_SIZE];
#endif /* FX_ENABLE_FAULT_TOLERANT */


/* Define thread prototypes.  */

void    filex_file_write_seek_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_file_write_seek_application_define(void *first_unused_memory)
#endif
{

#ifndef FX_STANDALONE_ENABLE
UCHAR    *pointer;

    
    /* Setup the working pointer.  */
    pointer =  (UCHAR *) first_unused_memory;

    /* Create the main thread.  */
    tx_thread_create(&ftest_0, "thread 0", ftest_0_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer =  pointer + DEMO_STACK_SIZE;

    /* Setup memory for the RAM disk and the sector cache.  */
    cache_buffer =  pointer;
    pointer =  pointer + CACHE_SIZE;
    ram_disk_memory =  pointer;

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
ULONG       i;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   File write seek test...................................");

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
                            512,                    // Total sectors 
                            512,                    // Sector size   
                            8,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail(FX_SUCCESS == status);

#ifdef FX_ENABLE_FAULT_TOLERANT

    /* Enable the Fault-tolerant feature.  */
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail(FX_SUCCESS == status);
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Create a file called TEST.TXT in the root directory.  */
    status += fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    return_if_fail(FX_SUCCESS == status);

    /* Initialize the data for write and read. */
    for (i = 0; i < sizeof(write_data); i++)
    {
        write_data[i] = (UCHAR)i;
        read_data[i] = 0;
    }

    /* Write 4116 bytes to the file.  */
    status =  fx_file_write(&my_file, write_data, 4116);
    return_if_fail(FX_SUCCESS == status);

    /* Seek to the beginning of the test file.  */
    status =  fx_file_seek(&my_file, 16464);
    return_if_fail(FX_SUCCESS == status);

    /* Write 4116 bytes to the file.  */
    status =  fx_file_write(&my_file, write_data + 4116, 4116);
    return_if_fail(FX_SUCCESS == status);

    /* Seek to the beginning of the test file.  */
    status =  fx_file_seek(&my_file, 112);
    return_if_fail(FX_SUCCESS == status);

    /* Write 1 byte to the file.  */
    status =  fx_file_write(&my_file, write_data + 112, 1);
    return_if_fail(FX_SUCCESS == status);

    /* Seek to the beginning of the test file.  */
    status =  fx_file_seek(&my_file, 32928);
    return_if_fail(FX_SUCCESS == status);

    /* Write 4116 bytes to the file.  */
    status =  fx_file_write(&my_file, write_data + 8232, 4116);
    return_if_fail(FX_SUCCESS == status);

    fx_file_close(&my_file);
    fx_media_close(&ram_disk);

    /* For the coverage of fx_file_write.c. */

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
                            70000,                    // Total sectors 
                            512,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail(FX_SUCCESS == status);

#ifdef FX_ENABLE_FAULT_TOLERANT

    /* Enable the Fault-tolerant feature.  */
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail(FX_SUCCESS == status);
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Create a file called TEST.TXT in the root directory.  */
    status += fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    return_if_fail(FX_SUCCESS == status);

    /* fx_file_current_relative_cluster < fx_file_consecutive_cluster. */
    status = fx_file_write(&my_file, write_data, 4 * 512);
    status += fx_file_seek(&my_file, 2 * 512);
    my_file.fx_file_current_relative_cluster = 0;
    status += fx_file_write(&my_file, write_data, 4 * 512);
    return_if_fail(FX_SUCCESS == status);

    fx_file_close(&my_file);
    fx_media_close(&ram_disk);

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
                            70000,                    // Total sectors 
                            512,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail(FX_SUCCESS == status);

#ifdef FX_ENABLE_FAULT_TOLERANT

    /* Enable the Fault-tolerant feature.  */
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail(FX_SUCCESS == status);
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Create a file called TEST.TXT in the root directory.  */
    status += fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    return_if_fail(FX_SUCCESS == status);

    status = fx_file_write(&my_file, write_data, 4 * 512);
    status += fx_file_seek(&my_file, 0);
    return_if_fail(FX_SUCCESS == status);

    /* data_append == FALSE && invalid copy_head_cluster */
    my_file.fx_file_current_physical_cluster = 1;
    status = fx_file_write(&my_file, write_data, 2 * 512);

    fx_file_close(&my_file);
    fx_media_close(&ram_disk);

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
                            70000,                    // Total sectors 
                            512,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail(FX_SUCCESS == status);

#ifdef FX_ENABLE_FAULT_TOLERANT

    /* Enable the Fault-tolerant feature.  */
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail(FX_SUCCESS == status);
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Create a file called TEST.TXT in the root directory.  */
    status += fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    return_if_fail(FX_SUCCESS == status);

    status = fx_file_write(&my_file, write_data, 4 * 512);
    status += fx_file_seek(&my_file, 0);
    return_if_fail(FX_SUCCESS == status);

    /* data_append == FALSE && invalid copy_head_cluster */
    my_file.fx_file_current_physical_cluster = (ULONG)-1;
    status = fx_file_write(&my_file, write_data, 2 * 512);

    my_file.fx_file_current_physical_cluster = 1;
    status = fx_file_write(&my_file, write_data, 2 * 512);

    fx_file_close(&my_file);
    fx_media_close(&ram_disk);

    /* Format the media by FAT16 which cluster 0 is available. */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            7000,                    // Total sectors 
                            512,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail(FX_SUCCESS == status);

#ifdef FX_ENABLE_FAULT_TOLERANT

    /* Enable the Fault-tolerant feature.  */
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail(FX_SUCCESS == status);
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Create a file called TEST.TXT in the root directory.  */
    status += fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    return_if_fail(FX_SUCCESS == status);

    status = fx_file_write(&my_file, write_data, 512);
    return_if_fail(FX_SUCCESS == status);

    /* Mark the cluster of root directory as Free and give it to the file being written. */
    ram_disk.fx_media_cluster_search_start = 0;
    _fx_utility_FAT_entry_write(&ram_disk, 0, FX_FREE_CLUSTER);
    status = fx_file_write(&my_file, write_data, 512);
    return_if_fail(FX_SECTOR_INVALID == status);

    fx_file_close(&my_file);
    fx_media_close(&ram_disk);

    printf("SUCCESS!\n");
    test_control_return(0);
}

