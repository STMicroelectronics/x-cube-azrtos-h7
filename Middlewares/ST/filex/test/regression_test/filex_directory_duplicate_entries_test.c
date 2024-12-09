/* This FileX test concentrates on recovery operation when there are duplicate entries.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_directory.h"
#include   "fx_ram_driver_test.h"
#include   "fx_fault_tolerant.h"
#include   "fx_utility.h"
#include   <stdio.h>

#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              16*128
#ifdef FX_ENABLE_FAULT_TOLERANT
#define     FAULT_TOLERANT_SIZE     FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE
#else
#define     FAULT_TOLERANT_SIZE     0
#endif

#define     DIRECTORY_NAME          "DUP_DIR"
#define     FILE_NAME               "abc"


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD                ftest_0;
#endif
static FX_MEDIA                 ram_disk;
static CHAR                     name[256];

/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                    ram_disk_memory[1024 * 1024];
static UCHAR                    *cache_buffer;
static UCHAR                    *fault_tolerant_buffer;   
#else
static UCHAR                    cache_buffer[CACHE_SIZE];
static UCHAR                    fault_tolerant_buffer[FAULT_TOLERANT_SIZE]; 
#endif

extern ULONG   _fx_ram_driver_copy_default_format;


/* Define thread prototypes.  */

void    filex_directory_duplicate_entries_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);




/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_directory_duplicate_entries_application_define(void *first_unused_memory)
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
    fault_tolerant_buffer = pointer;
    pointer += FAULT_TOLERANT_SIZE;

#endif

    /* Initialize the FileX system.  */
    fx_system_initialize();
#ifdef FX_STANDALONE_ENABLE
    ftest_0_entry(0);
#endif
}

static void    delete_duplicate_entry(FX_MEDIA *media_ptr,
                                      CHAR *parent_directory,
                                      CHAR *duplicate_entry_name)
{
FX_DIR_ENTRY    dir_entry;

    fx_directory_default_set(media_ptr, parent_directory);

    /* Get the dir entry of DIRECTORY_NAME. */
    FX_PROTECT

    dir_entry.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;
    dir_entry.fx_dir_entry_short_name[0] =  0;
    _fx_directory_search(media_ptr, duplicate_entry_name, &dir_entry, FX_NULL, FX_NULL);

    /* Now try to remove one of the duplicate dir entries. */
    dir_entry.fx_dir_entry_name[0] =  (CHAR)FX_DIR_ENTRY_FREE;
    dir_entry.fx_dir_entry_short_name[0] =  (CHAR)FX_DIR_ENTRY_FREE;
#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {
        _fx_directory_exFAT_entry_write(media_ptr, &dir_entry, UPDATE_FILE);
    }
    else
#endif /* FX_ENABLE_EXFAT */
    {
        _fx_directory_entry_write(media_ptr, &dir_entry);
    }
    FX_UNPROTECT
}


/* Define the test threads.  */

static void    ftest_0_entry(ULONG thread_input)
{

UINT            status;
FX_DIR_ENTRY    dir_entry;
FX_MEDIA       *media_ptr = &ram_disk;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Directory duplicate entries test.......................");

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

/* We need a larger disk to test the feature of fault tolerant. */
#ifdef FX_ENABLE_FAULT_TOLERANT
                            512 * 8,                // Total sectors
                            256,                    // Sector size
                            8,                      // Sectors per cluster
#else
                            512,                    // Total sectors
                            128,                    // Sector size
                            1,                      // Sectors per cluster
#endif

                            1,                      // Heads
                            1);                     // Sectors per track
    return_if_fail( status == FX_SUCCESS);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

/* We need a larger disk to test the feature of fault tolerant. */
#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Enable fault tolerant if FX_ENABLE_FAULT_TOLERANT is defined. */
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);
#endif

    /* Create a directory.  */
    status =  fx_directory_create(&ram_disk, DIRECTORY_NAME);
    return_if_fail( status == FX_SUCCESS);

    status =  fx_directory_default_set(&ram_disk, DIRECTORY_NAME);
    return_if_fail( status == FX_SUCCESS);

    /* Create a file in subdirectory. */
    status =  fx_file_create(&ram_disk, FILE_NAME);
    return_if_fail( status == FX_SUCCESS);

    status =  fx_directory_default_set(&ram_disk, "/");
    return_if_fail( status == FX_SUCCESS);

    /* Get the dir entry of DIRECTORY_NAME. */
    FX_PROTECT
    dir_entry.fx_dir_entry_name =  ram_disk.fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;
    dir_entry.fx_dir_entry_short_name[0] =  0;
    status =  _fx_directory_search(&ram_disk, DIRECTORY_NAME, &dir_entry, FX_NULL, FX_NULL);
    return_if_fail( status == FX_SUCCESS);

    /* Duplicate the dir entry. */
    dir_entry.fx_dir_entry_byte_offset += FX_DIR_ENTRY_SIZE;
#ifdef FX_ENABLE_EXFAT
    if (ram_disk.fx_media_FAT_type == FX_exFAT)
    {
        status =  _fx_directory_exFAT_entry_write(&ram_disk, &dir_entry, UPDATE_FILE);
    }
    else
#endif /* FX_ENABLE_EXFAT */
    {
        status =  _fx_directory_entry_write(&ram_disk, &dir_entry);
    }
    return_if_fail( status == FX_SUCCESS);
    FX_UNPROTECT

    /* Make sure there are duplicate dir entries. */
    status =  fx_directory_first_entry_find(&ram_disk, name);
    return_if_fail( status == FX_SUCCESS);
    return_if_fail( strcmp(name, DIRECTORY_NAME) == 0);

    status =  fx_directory_next_entry_find(&ram_disk, name);
    return_if_fail( status == FX_SUCCESS);
    return_if_fail( strcmp(name, DIRECTORY_NAME) == 0);

    delete_duplicate_entry(&ram_disk, "/", DIRECTORY_NAME);

    /* Make sure there are no duplicate dir entries. */
    status =  fx_directory_first_entry_find(&ram_disk, name);
    return_if_fail( status == FX_SUCCESS);
    return_if_fail( strcmp(name, DIRECTORY_NAME) == 0);

    status =  fx_directory_next_entry_find(&ram_disk, name);
    return_if_fail( status != FX_SUCCESS);

    /* Check the file in DIRECTORY_NAME is still valid. */
    status =  fx_directory_default_set(&ram_disk, DIRECTORY_NAME);
    return_if_fail( status == FX_SUCCESS);

    /* Skip '.' and '..'. */
    status =  fx_directory_first_entry_find(&ram_disk, name);
    return_if_fail( status == FX_SUCCESS);
    status =  fx_directory_next_entry_find(&ram_disk, name);
    return_if_fail( status == FX_SUCCESS);

    /* Check the filename. */
    status =  fx_directory_next_entry_find(&ram_disk, name);
    return_if_fail( status == FX_SUCCESS);
    return_if_fail( strcmp(name, FILE_NAME) == 0);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(254);
    }
    else
    {

        printf("SUCCESS!\n");
        test_control_return(0);
    }
}
