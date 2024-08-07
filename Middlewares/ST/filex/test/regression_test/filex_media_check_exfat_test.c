/* This FileX test concentrates on the file rename operations.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_ram_driver_test.h"
#include   "fx_utility.h"
#include   "fx_directory_exFAT.h"
#include   <stdio.h>

#ifdef FX_ENABLE_EXFAT

#define     DEMO_STACK_SIZE         8192
#define     CACHE_SIZE              FX_EXFAT_SECTOR_SIZE


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD                ftest_0;
#endif
static FX_MEDIA                 ram_disk;


/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                    *ram_disk_memory;
static UCHAR                    *cache_buffer;
#else
static UCHAR                     cache_buffer[CACHE_SIZE];
#endif


/* Define thread prototypes.  */

void    filex_media_check_exfat_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);

static UCHAR    scratch_memory[256*128];
static UCHAR    buffer[1024];

/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_media_check_exfat_application_define(void *first_unused_memory)
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
ULONG       errors_detected;
FX_FILE     my_file;
#if 0
UINT        i, j;
USHORT      checksum = 0;
ULONG64     logical_sector;
#endif

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   exFAT media check test.................................");

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_exFAT_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            0,                      // Hidden sectors
                            256,                    // Total sectors 
                            FX_EXFAT_SECTOR_SIZE,   // Sector size   
                            1,                      // exFAT Sectors per cluster
                            12345,                  // Volume ID
                            0);                     // Boundary unit
    return_if_fail(FX_SUCCESS == status);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail(FX_SUCCESS == status);

    status = fx_media_check(&ram_disk, scratch_memory, sizeof(scratch_memory), 0, &errors_detected);
    return_if_fail(FX_SUCCESS == status);
    return_if_fail(0 == errors_detected);

    status = fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, (void *)buffer, 1024);
    status += fx_file_write(&my_file, (void *)buffer, 1024);
    status += fx_file_close(&my_file);
    return_if_fail(FX_SUCCESS == status);

    status = fx_file_create(&ram_disk, "AAA.TXT");
    status += fx_file_open(&ram_disk, &my_file, "AAA.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, (void *)buffer, 1024);
    status += fx_file_close(&my_file);
    return_if_fail(FX_SUCCESS == status);

    status = fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, (void *)buffer, 1024);
    status += fx_file_close(&my_file);
    return_if_fail(FX_SUCCESS == status);

    /* Check the media after creating some files. */
    status = fx_media_check(&ram_disk, scratch_memory, sizeof(scratch_memory), 0, &errors_detected);
    return_if_fail(FX_SUCCESS == status);
    return_if_fail(0 == errors_detected);

    status = fx_directory_create(&ram_disk, "/A0");
    status += fx_file_create(&ram_disk, "/A0/TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "/A0/TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, (void *)buffer, 1024);
    status += fx_file_close(&my_file);
    return_if_fail(FX_SUCCESS == status);

    status = fx_media_check(&ram_disk, scratch_memory, sizeof(scratch_memory), 0, &errors_detected);
    return_if_fail(FX_SUCCESS == status);
    return_if_fail(0 == errors_detected);

    status = fx_directory_create(&ram_disk, "/A0/A1");
    status += fx_file_create(&ram_disk, "/A0/A1/TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "/A0/A1/TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, (void *)buffer, 1024);
    status += fx_file_close(&my_file);

    /* Check the media after creating some files under sub-directories. */
    status = fx_media_check(&ram_disk, scratch_memory, sizeof(scratch_memory), 0, &errors_detected);
    return_if_fail(FX_SUCCESS == status);
    return_if_fail(0 == errors_detected);

    /* The first cluster in exFAT has index 2. */
    status = _fx_utility_exFAT_bitmap_cache_update(&ram_disk, 2);
    /* Mark the first cluster free which is occupied by Allocation Bitmap table. */
    ram_disk.fx_media_exfat_bitmap_cache[0] &= 0xfe;
    ram_disk.fx_media_exfat_bitmap_cache_dirty = FX_TRUE;
    status += _fx_utility_exFAT_bitmap_flush(&ram_disk);
    return_if_fail(FX_SUCCESS == status);

    status = fx_media_check(&ram_disk, scratch_memory, sizeof(scratch_memory), 0, &errors_detected);
    return_if_fail(FX_SUCCESS == status);
    return_if_fail(errors_detected & FX_LOST_CLUSTER_ERROR);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail(FX_SUCCESS == status);

#if 0
    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_exFAT_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            0,                      // Hidden sectors
                            256,                    // Total sectors 
                            FX_EXFAT_SECTOR_SIZE,   // Sector size   
                            1,                      // exFAT Sectors per cluster
                            12345,                  // Volume ID
                            0);                     // Boundary unit
    return_if_fail(FX_SUCCESS == status);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail(FX_SUCCESS == status);

    status = fx_media_check(&ram_disk, scratch_memory, sizeof(scratch_memory), 0, &errors_detected);
    return_if_fail(FX_SUCCESS == status);
    return_if_fail(0 == errors_detected);

    status = fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, (void *)buffer, 1024);
    status += fx_file_close(&my_file);
    return_if_fail(FX_SUCCESS == status);

    status = fx_file_create(&ram_disk, "AAA.TXT");
    status += fx_file_open(&ram_disk, &my_file, "AAA.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, (void *)buffer, 1024);
    status += fx_file_close(&my_file);
    return_if_fail(FX_SUCCESS == status);

    /* The first three dir-entries of root directory are vlume_name, Allocation Bitmap and Up-case. */
    /* Read the dir-entry of TEST.TXT. */
    i = 3;
    /* These dir-entries are all located at the first sector. */
    logical_sector = 141;
    //logical_sector = (ULONG)dir_entry_ptr -> fx_dir_entry_log_sector;
    _fx_utility_logical_sector_read(&ram_disk, (ULONG64) logical_sector, buffer, ((ULONG) 1), FX_DIRECTORY_SECTOR);
    /* Larger the file size to occupy the cluster of next file. */
    _fx_utility_64_unsigned_write(&buffer[i * FX_DIR_ENTRY_SIZE + 24], 1025);
    /* Recalculate the checksem. */
    for (j = 0; j < FX_DIR_ENTRY_SIZE; ++j)
    {
        if ((j == 2) || (j == 3))
        {
            continue;
        }
        checksum = (USHORT)(((checksum >> 1) | (checksum << 15)) + buffer[i * FX_DIR_ENTRY_SIZE + j]);
    }
    for (j = 0; j < FX_DIR_ENTRY_SIZE * 2; ++j)
    {
        checksum = (USHORT)(((checksum >> 1) | (checksum << 15)) + buffer[(i + 1) * FX_DIR_ENTRY_SIZE + j]);
    }
    //checksum = 25408;
    _fx_utility_16_unsigned_write(&buffer[i * FX_DIR_ENTRY_SIZE + 2], checksum);
    _fx_utility_logical_sector_write(&ram_disk, (ULONG64) logical_sector, buffer, ((ULONG) 1), FX_DIRECTORY_SECTOR);
    fx_media_flush(&ram_disk);

    status = fx_media_check(&ram_disk, scratch_memory, sizeof(scratch_memory), 0, &errors_detected);
    return_if_fail(FX_SUCCESS == status);
    return_if_fail(errors_detected & FX_FILE_SIZE_ERROR);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail(FX_SUCCESS == status);
#endif

    printf("SUCCESS!\n");
    test_control_return(0);
}

#endif /* FX_ENABLE_EXFAT */
