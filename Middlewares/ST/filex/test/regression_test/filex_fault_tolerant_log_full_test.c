/* This FileX test concentrates on the Fault-Tolerant redo log write interrupt operation.
 * The available bytes before/after file write should be decreased write_buffer_size only. */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_system.h"
#include   "fx_fault_tolerant.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"
extern void    test_control_return(UINT status);
void    filex_fault_tolerant_log_full_test_application_define(void *first_unused_memory);

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
static CHAR                     write_buffer[2048];
static UINT                     i;

#ifdef FX_ENABLE_EXFAT
#define FAT_COUNT               4            /* FAT12, 16, 32 and exFAT.  */
#else
#define FAT_COUNT               3            /* FAT12, 16, 32.  */
#endif
#define TEST_COUNT              FAT_COUNT

/* Define thread prototypes.  */

static void    ftest_0_entry(ULONG thread_input);
extern void    _fx_ram_driver(FX_MEDIA *media_ptr);
extern void    test_control_return(UINT status);
extern UINT    _filex_fault_tolerant_log_check(FX_MEDIA *media_ptr);
extern UINT    (*driver_write_callback)(FX_MEDIA *media_ptr, UINT sector_type, UCHAR *block_ptr, UINT *operation_ptr);
static UINT    my_driver_write(FX_MEDIA *media_ptr, UINT sector_type, UCHAR *block_ptr, UINT *operation_ptr);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_log_full_test_application_define(void *first_unused_memory)
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
    printf("FileX Test:   Fault Tolerant Log Full Tes............................");

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
                                     70000 * 4,              // Total sectors
                                     512,                    // Sector size
                                     4,                      // Sectors per cluster
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

        /* Determine if the format had an error.  */
        if (status)
        {

            printf("ERROR!\n");
            test_control_return(1);
        }

        /* Open the ram_disk.  */
        status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);

        /* Check the status.  */
        if (status != FX_SUCCESS)
        {

            /* Error, return error code.  */
            printf("ERROR!\n");
            test_control_return(2);
        }

        /* Enable the Fault-tolerant feature.  */
        status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);

        /* Check the create status.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(4);
        }

        /* Create a file.  */
        status = fx_file_create(&ram_disk, "TEST.TXT");
        status += fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

        /* Check the create status.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(5);
        }

        /* Set the callback function to modify size of fault tolerant log.  */
        driver_write_callback = my_driver_write;

        /* Write one cluster of information to the TEST.TXT file.  */
        status =  fx_file_write(&my_file, write_buffer, sizeof(write_buffer));

        /* Check the create status.  */
        if (status != FX_NO_MORE_SPACE)
        {

            printf("ERROR!\n");
            test_control_return(6);
        }

        /* Close the secondary file.  */
        status = fx_file_close(&my_file);

        /* Check the create status.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(7);
        }

        /* Close the media.  */
        status =  fx_media_close(&ram_disk);

        /* Determine if the test was successful.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(8);
        }

        /* Delete the thread.  */
#ifndef FX_STANDALONE_ENABLE
        tx_thread_delete(&ftest_1);
#endif
    }

    /* Output successful.  */
    printf("SUCCESS!\n");
    test_control_return(0);
}

static UINT my_driver_write(FX_MEDIA *media_ptr, UINT sector_type, UCHAR *block_ptr, UINT *operation_ptr)
{

    FX_PARAMETER_NOT_USED(media_ptr);
    FX_PARAMETER_NOT_USED(sector_type);
    FX_PARAMETER_NOT_USED(block_ptr);
    FX_PARAMETER_NOT_USED(operation_ptr);

    if (ram_disk.fx_media_fault_tolerant_state == FX_FAULT_TOLERANT_STATE_STARTED)
    {
#ifdef FX_ENABLE_EXFAT
        /* Modify the size of fault tolerant log file. */
        ram_disk.fx_media_fault_tolerant_file_size = FX_EXFAT_SECTOR_SIZE;
#else
        ram_disk.fx_media_fault_tolerant_file_size = FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE;
#endif
    }

    /* Return.  */
    return FX_SUCCESS;
}

#else

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_log_full_test_application_define(void *first_unused_memory)
#endif
{

    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Log Full Tes............................N/A\n");

    test_control_return(255);
}
#endif

