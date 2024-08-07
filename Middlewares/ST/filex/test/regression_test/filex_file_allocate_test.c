/* This FileX test concentrates on the file allocate test.  */
/*
Test steps:
1. Format and open the media;
2. Enable fault tolerant feature if it is compiled;
3. Create new file called "TEST.TXT";
4. Allocate the file with available bytes and seek to the beginning of file;
5. Write 1000 bytes out of the file to fill the media;
6. Close the file;
7. Open the file and read data from it;
*/
#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_fault_tolerant.h"
#include   <stdio.h>
#include   <time.h>
#include   "fx_ram_driver_test.h"
extern void    test_control_return(UINT status);

#define DEMO_STACK_SIZE         4096

#ifdef FX_ENABLE_EXFAT
#define CACHE_SIZE              FX_EXFAT_SECTOR_SIZE
#else
#define CACHE_SIZE              2048
#endif

#ifdef FX_ENABLE_FAULT_TOLERANT
#ifdef FX_ENABLE_EXFAT
#define FAULT_TOLERANT_SIZE     FX_EXFAT_SECTOR_SIZE
#else
#define FAULT_TOLERANT_SIZE     FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE
#endif
#else
#define     FAULT_TOLERANT_SIZE     0
#endif

/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD                ftest_0;
#endif
static FX_MEDIA                 ram_disk;
static FX_FILE                  my_file;
static UCHAR                    *pointer;

/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                    *cache_buffer;
#ifdef FX_ENABLE_FAULT_TOLERANT
static UCHAR                    *fault_tolerant_buffer;
#endif /* FX_ENABLE_FAULT_TOLERANT */
#else
static UCHAR                     cache_buffer[CACHE_SIZE];
#ifdef FX_ENABLE_FAULT_TOLERANT
static UCHAR                     fault_tolerant_buffer[FAULT_TOLERANT_SIZE]; 
#endif /* FX_ENABLE_FAULT_TOLERANT */
#endif


#ifdef FX_ENABLE_EXFAT
#define TEST_COUNT              4
#else
#define TEST_COUNT              3
#endif

/* Define thread prototypes.  */

void    filex_file_allocate_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);
extern void    _fx_ram_driver(FX_MEDIA *media_ptr);
extern void    test_control_return(UINT status);


/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_file_allocate_application_define(void *first_unused_memory)
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
#ifdef FX_ENABLE_FAULT_TOLERANT
    fault_tolerant_buffer = pointer;
    pointer += FAULT_TOLERANT_SIZE;
#endif /* FX_ENABLE_FAULT_TOLERANT */

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
ULONG       file_size = 1000;
ULONG       i, j;
ULONG       data_value;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   File Allocate Test.....................................");

    /* Generate a random number for write data.  */
    srand((ULONG)time(NULL));
    data_value = (ULONG)(rand() & 0xFF) | (ULONG)((rand() & 0xFF) << 8) | (ULONG)((rand() & 0xFF) << 16) | (ULONG)((rand() & 0xFF) << 24);

    /* Genearte the write data.  */
    for (j = 0; j < large_data_buffer_size / sizeof(ULONG); j ++)
    {
        ((ULONG*)large_data_buffer)[j] = data_value++;
    }

    /* Roll back the value for later verification use.  */
    data_value -= (large_data_buffer_size / sizeof(ULONG));

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
                                     4000 * 8,               // Total sectors
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
                                     60000 * 8,              // Total sectors
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
                                     400000 * 8,             // Total sectors
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
                                            _fx_ram_driver,             // Driver entry
                                            ram_disk_memory_large,      // RAM disk memory pointer
                                            cache_buffer,               // Media buffer pointer
                                            CACHE_SIZE,                 // Media buffer size
                                            "MY_RAM_DISK",              // Volume Name
                                            1,                          // Number of FATs
                                            0,                          // Hidden sectors
                                            40000* 2,                   // Total sectors
                                            FX_EXFAT_SECTOR_SIZE,  // Sector size.
                                            4,                          // exFAT Sectors per cluster
                                            12345,                      // Volume ID
                                            0);                         // Boundary unit
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

#ifdef FX_ENABLE_FAULT_TOLERANT
        /* Enable the Fault-tolerant feature.  */
        status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);

        /* Check status.   */
        if (status)
        {

            printf("ERROR!\n");
            test_control_return(3);
        }
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Create a file called TEST.TXT in the root directory.  */
        status =  fx_file_create(&ram_disk, "TEST.TXT");

        /* Check the create status.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(4);
        }

        /* Open the test file.  */
        status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

        /* Check the file open status.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(5);
        }

        status = fx_file_allocate(&my_file, file_size);
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(7);
        }

        status = fx_file_seek(&my_file, 0);
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(8);
        }

        /* Write the data to fill the media one time.  */
        status =  fx_file_write(&my_file, (void *) large_data_buffer, file_size);

        /* Check the file write status.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(9);
        }

        /* Close the test file.  */
        status =  fx_file_close(&my_file);

        /* Check the file close status.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(10);
        }

        /* Open the test file.  */
        status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

        /* Check the file open status.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(12);
        }

        /* Seek to the beginning of the test file.  */
        status =  fx_file_seek(&my_file, 0);

        /* Check the file seek status.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(13);
        }

        /* Now read in all the bytes again to make sure the file contents are really there.  */
        status =  fx_file_read(&my_file, (void *) large_data_buffer, large_data_buffer_size, &actual);

        /* Check the file read status.  */
        if ((status != FX_SUCCESS) || (actual != file_size))
        {

            printf("ERROR!\n");
            test_control_return(14);
        }

        /* Close the test file.  */
        status =  fx_file_close(&my_file);

        /* Check the file close status.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(15);
        }

        /* Close the media.  */
        status =  fx_media_close(&ram_disk);

        /* Determine if the test was successful.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(16);
        }
    }

    /* Output successful.  */
    printf("SUCCESS!\n");
    test_control_return(0);
}
