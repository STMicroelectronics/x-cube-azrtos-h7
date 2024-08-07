/* This FileX test concentrates on the available clusters after file seek and write operation.  */
/* A fix-me is in this file. */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_utility.h"
#include   "fx_ram_driver_test.h"
#include   "fx_fault_tolerant.h"
#include   <stdio.h>

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

struct step
{
    ULONG offset;
    UINT seek;
    UINT size;
};

#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *ram_disk_memory;
static UCHAR                  *cache_buffer;
#else
static UCHAR                   cache_buffer[CACHE_SIZE];
#endif
static UCHAR                   test_buffer[4096];
#ifdef FX_ENABLE_FAULT_TOLERANT
static UCHAR                   fault_tolerant_buffer[FAULT_TOLERANT_SIZE];
#endif /* FX_ENABLE_FAULT_TOLERANT */
static struct step steps[] = 
{
    {0, FX_SEEK_FORWARD, 1136},
    {4 * sizeof(test_buffer), FX_SEEK_FORWARD, sizeof(test_buffer)},
    {112, FX_SEEK_END, sizeof(test_buffer)},
    {(2 * sizeof(test_buffer)), FX_SEEK_BACK, sizeof(test_buffer)},
};

/* Define thread prototypes.  */

void    filex_file_write_available_cluster_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_file_write_available_cluster_application_define(void *first_unused_memory)
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
#ifdef FX_ENABLE_FAULT_TOLERANT
UINT        enable_fault_tolerant_this_time = 0;
UINT        do_it_again = 0;
ULONG       actual;
#endif /* FX_ENABLE_FAULT_TOLERANT */

ULONG       used_clusters;
ULONG       total_bytes;
ULONG       total_clusters;
ULONG       i, j;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   File write available cluster test......................");

#ifdef FX_ENABLE_FAULT_TOLERANT
TEST_START:
#endif /* FX_ENABLE_FAULT_TOLERANT */

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
    return_if_fail( status == FX_SUCCESS);

#ifdef FX_ENABLE_FAULT_TOLERANT
    if ( enable_fault_tolerant_this_time)
    {
        /* Enable the Fault-tolerant feature.  */
        status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
        return_if_fail( status == FX_SUCCESS);
    }
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");
    return_if_fail( status == FX_SUCCESS);

    /* Open the test file.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_SUCCESS);

    total_clusters = ram_disk.fx_media_available_clusters;

    /* Run test steps. */
    total_bytes = 9216;
    for (j = 0; j < 100; j++)
    {
        i = j & 0x03;
        fx_file_relative_seek (&my_file, steps[i].offset, steps[i].seek);
        fx_file_write (&my_file, test_buffer, steps[i].size);

        if (i == 3)
        {

            /* Check available clusters. */
            used_clusters = total_bytes / 4096 + 1;
            return_if_fail( total_clusters == ram_disk.fx_media_available_clusters + used_clusters);

            /* Increase 8080 bytes each round. */
            total_bytes += 8080;
        }
    }
#ifdef FX_ENABLE_FAULT_TOLERANT
    if ( enable_fault_tolerant_this_time)
    {
        /* Make IO error while looking for copy_head_cluster by _fx_utility_FAT_entry_read. */
        _fx_utility_fat_entry_read_error_request = 1;
        status = fx_file_relative_seek( &my_file, steps[0].offset, steps[0].seek);
        status += fx_file_write( &my_file, test_buffer, steps[0].size);
        return_if_fail( status == FX_IO_ERROR);

        /* Make IO error while looking for copy_tail_cluster by _fx_utility_FAT_entry_read. */
        _fx_utility_fat_entry_read_error_request = 94;
        status = fx_file_relative_seek( &my_file, steps[3].offset, steps[3].seek);
        status += fx_file_write( &my_file, test_buffer, steps[3].size);
        return_if_fail( status == FX_IO_ERROR);

        /* Make IO error while copying sectors in replaced clusters at the back in fx_file_write.c.*/
        _fx_ram_driver_io_error_request = 17;
        status = fx_file_relative_seek( &my_file, steps[3].offset, steps[3].seek);
        status += fx_file_write( &my_file, test_buffer, steps[3].size);
        return_if_fail( status == FX_IO_ERROR);

        /* Make IO error while writing back the current logical sector at Line 1599 in fx_file_write.c. */
        _fx_ram_driver_io_error_request = 18;
        status = fx_file_relative_seek( &my_file, steps[3].offset, steps[3].seek);
        status += fx_file_write( &my_file, test_buffer, steps[3].size);
        return_if_fail( status == FX_IO_ERROR);

#if !defined(FX_ENABLE_FAULT_TOLERANT) && !defined(FX_DISABLE_CACHE)
        /*Make IO error while copying sectors in replaced clusters at the front in fx_file_write.c.*/
        _fx_ram_driver_io_error_request = 3;
        status = fx_file_relative_seek( &my_file, steps[3].offset, steps[3].seek);
        status += fx_file_write( &my_file, test_buffer, steps[3].size);
        return_if_fail( status == FX_IO_ERROR);
#endif

        /* Make IO error while writing back the current logical sector at Line 1215 in fx_file_write.c. */
        _fx_ram_driver_io_error_request = 4;
        status = fx_file_relative_seek( &my_file, steps[3].offset, steps[3].seek);
        status += fx_file_write( &my_file, test_buffer, steps[3].size);
        return_if_fail( status == FX_IO_ERROR);

        /* Make IO error while getting the cluster next to the copy tail. */
        _fx_utility_fat_entry_read_error_request = 75;
        status = fx_file_relative_seek( &my_file, steps[3].offset, steps[3].seek);
        status += fx_file_write( &my_file, test_buffer, steps[3].size);

        /* Make IO error while _fx_file_write is calling _fx_fault_tolerant_set_FAT__chain. */
        _fx_ram_driver_io_error_request = 1;
        status = fx_file_relative_seek( &my_file, steps[0].offset, steps[0].seek);
        status += fx_file_write( &my_file, test_buffer, steps[0].size);
        return_if_fail( status == FX_IO_ERROR);

        /* Backup original data. */
        actual = my_file.fx_file_first_physical_cluster;

/* Please fix me:
 * The following code is determing to cover a branch in fx_file_write.c, which need to get three condition:
 * 1. fx_file_total_clusters - fx_file_current_relative_cluster > 0
 * 2. fx_file_current_logical_offset >= fx_media_bytes_per_sector
 * 3. fx_file_current_relative_sector >= fx_media_sectors_per_cluster - 1
 *
 * In other words, it need to set fx_file_current_logical_offset at the end of a cluster which is in the middle of a file.
 * I manually modify FILE_STRUCT and satify the condition for coverage. Should this branch be removed?
 */
        /* Produce wrong first_physical_cluster. */
        status = fx_file_relative_seek( &my_file, steps[0].offset, steps[0].seek);
        my_file.fx_file_first_physical_cluster = 0;
        status += fx_file_write( &my_file, test_buffer, steps[0].size);
        return_if_fail( status == FX_NOT_FOUND);

        status = fx_file_relative_seek( &my_file, steps[0].offset, steps[0].seek);
        my_file.fx_file_first_physical_cluster = ram_disk.fx_media_fat_reserved;
        status += fx_file_write( &my_file, test_buffer, steps[0].size);
        return_if_fail( status == FX_NOT_FOUND);
        
        /* Restore data we just modified. */
        my_file.fx_file_first_physical_cluster = actual;

        if ( do_it_again)
        {
            goto EXTRA_TEST;
        }

        /* Attempt to cover the branch that offset is at the end of a sector but not the end of a file. */
        /* No head cluster need to be replaced. */
        status = fx_file_relative_seek( &my_file, steps[2].offset, steps[2].seek);
        my_file.fx_file_current_logical_offset = 512;
        status += fx_file_write( &my_file, test_buffer, steps[2].size);

        /* Attempt to cover the branch that offset is at the end of a cluster but not the end of a file. */
        /* No head cluster need to be replaced. */
        status = fx_file_relative_seek( &my_file, steps[2].offset, steps[2].seek);
        my_file.fx_file_current_logical_offset = 512;
        my_file.fx_file_current_relative_sector = 7;
        my_file.fx_file_total_clusters = 50;
        status += fx_file_write( &my_file, test_buffer, steps[2].size);

        /* Close the file.  */
        status =  fx_file_close(&my_file);
        status += fx_media_close( &ram_disk);
        return_if_fail( status == FX_SUCCESS);
        do_it_again = 1;
        goto TEST_START;

EXTRA_TEST:
        /* Covered the branch dealing with the condition that offset is at the end of a cluster but not the end of a file. */
        /* Head cluster need to be replaced is existed. */
        status = fx_file_relative_seek( &my_file, steps[2].offset, steps[2].seek);
        my_file.fx_file_current_logical_offset = 512;
        my_file.fx_file_current_relative_sector = 7;
        status += fx_file_write( &my_file, test_buffer, steps[2].size);

    }
#endif

    /* Close the file.  */
    status =  fx_file_close(&my_file);
    status += fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);
    
#ifdef FX_ENABLE_FAULT_TOLERANT
    if ( enable_fault_tolerant_this_time == 0)
    {
        enable_fault_tolerant_this_time = 1;
        goto TEST_START;
    }
#endif
    
#if defined(FX_ENABLE_FAULT_TOLERANT) && !defined(FX_DISABLE_CACHE)
FX_CACHED_SECTOR cache_entry;

    ram_disk.fx_media_sector_cache_hashed = 0;
    ram_disk.fx_media_memory_buffer = test_buffer;
    ram_disk.fx_media_sector_cache_end = test_buffer;
    ram_disk.fx_media_fault_tolerant_enabled = 1;
    ram_disk.fx_media_fault_tolerant_state |= FX_FAULT_TOLERANT_STATE_STARTED;
    ram_disk.fx_media_sector_cache_list_ptr = &cache_entry;
    ram_disk.fx_media_sector_cache_size = 1;
    cache_entry.fx_cached_sector_memory_buffer = NULL;
    cache_entry.fx_cached_sector_next_used = NULL;

    _fx_utility_logical_sector_write(&ram_disk, (ULONG64)3, test_buffer, 0, FX_DATA_SECTOR);

#endif /* defined(FX_ENABLE_FAULT_TOLERANT) && !defined(FX_DISABLE_CACHE) */

    printf("SUCCESS!\n");
    test_control_return(0);
}

