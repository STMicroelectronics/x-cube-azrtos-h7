/* This FileX test concentrates on the basic media volume get/set operation.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"

#ifdef FX_ENABLE_EXFAT

#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              FX_EXFAT_SECTOR_SIZE


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD               ftest_0;
#endif
static FX_MEDIA                ram_disk;


/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *ram_disk_memory;
static UCHAR                  *cache_buffer;
#else
static UCHAR                   cache_buffer[CACHE_SIZE];
#endif


/* Define thread prototypes.  */

void    filex_media_volume_get_set_exfat_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_media_volume_get_set_exfat_application_define(void *first_unused_memory)
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
CHAR        volume_buffer[32];

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   exFAT Media volume get/set test........................");

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

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(2);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(3);
    }
    
    /* Get the current volume name - from boot record.  */
    status =  fx_media_volume_get(&ram_disk, volume_buffer, FX_DIRECTORY_SECTOR);
    
    /* Determine if there was an error getting the volume name.  */
    if ((status != FX_SUCCESS) ||
        (volume_buffer[0] != 'M') ||
        (volume_buffer[1] != 'Y') ||
        (volume_buffer[2] != '_') ||
        (volume_buffer[3] != 'R') ||
        (volume_buffer[4] != 'A') ||
        (volume_buffer[5] != 'M') ||
        (volume_buffer[6] != '_') ||
        (volume_buffer[7] != 'D') ||
        (volume_buffer[8] != 'I') ||
        (volume_buffer[9] != 'S') ||
        (volume_buffer[10] != 'K'))
    {

        /* Error getting the volume name.  */
        printf("ERROR!\n");
        test_control_return(4);
    }

    /* It is now time to set the volume name.  */
    status =  fx_media_volume_set(&ram_disk, "NEW");

    /* Determine if the volume name set was successful...  */
    if (status != FX_SUCCESS) 
    {
    
        /* Error setting the volume name.  */
        printf("ERROR!\n");
        test_control_return(6);
    }

    /* Flush and invalidate the media...   */
    fx_media_cache_invalidate(&ram_disk);

    /* Get the current volume name - from boot record.  */
    status =  fx_media_volume_get(&ram_disk, volume_buffer, FX_DIRECTORY_SECTOR);
    
    /* Determine if there was an error getting the volume name.  */
    if ((status != FX_SUCCESS) ||
        (volume_buffer[0] != 'N') ||
        (volume_buffer[1] != 'E') ||
        (volume_buffer[2] != 'W') ||
        (volume_buffer[3] != (CHAR) 0))
    {

        /* Error getting the volume name.  */
        printf("ERROR!\n");
        test_control_return(7);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(10);
    }
    else
    {

        printf("SUCCESS!\n");
        test_control_return(0);
    }
}

#endif /* FX_ENABLE_EXFAT */
