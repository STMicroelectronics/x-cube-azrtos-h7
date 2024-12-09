/* This test case verifies _fx_utility_exFAT_bitmap_flush() return failure when the driver does nothing. */
#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#include   "tx_thread.h"
#endif
#include   "fx_api.h"
#include   "fx_utility.h"
#include   "fx_ram_driver_test.h"
#include    <stdio.h>
#include    <string.h>

#ifdef FX_ENABLE_EXFAT
#define     DEMO_STACK_SIZE         8192
/* Set the cache size as the size of one sector causing frequently IO operation. */
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
static UCHAR                    cache_buffer[CACHE_SIZE];
#endif

/* Define thread prototypes.  */

void    filex_bitmap_flush_exfat_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);
static void  bad_driver(FX_MEDIA *media_ptr);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);
UINT  _fx_utility_exFAT_bitmap_flush(FX_MEDIA *media_ptr);

/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_bitmap_flush_exfat_application_define(void *first_unused_memory)
#endif
{

#ifndef FX_STANDALONE_ENABLE
    UCHAR    *pointer;


    /* Setup the working pointer.  */
    pointer = (UCHAR *)first_unused_memory;

    /* Create the main thread.  */
    tx_thread_create(&ftest_0, "thread 0", ftest_0_entry, 0,
        pointer, DEMO_STACK_SIZE,
        4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer = pointer + DEMO_STACK_SIZE;

    /* Setup memory for the RAM disk and the sector cache.  */
    cache_buffer = pointer;
    pointer = pointer + CACHE_SIZE;
    ram_disk_memory = pointer;
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
UINT status;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Bitmap Flush exFAT test................................");

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
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);


    /* Set the driver to bad one which does nothing on any operations. */
    ram_disk.fx_media_driver_entry = bad_driver;

    /* Mark bitmap cache as dirty. */
    ram_disk.fx_media_exfat_bitmap_cache_dirty = FX_TRUE;

    /* Flush bitmap. */
    status = _fx_utility_exFAT_bitmap_flush(&ram_disk);

    /* Expect error status since driver is bad. */
    return_if_fail( status != FX_SUCCESS);

    /* Recover driver. */
    ram_disk.fx_media_driver_entry = _fx_ram_driver;

    printf("SUCCESS!\n");
    test_control_return(0);
}

static void  bad_driver(FX_MEDIA *media_ptr)
{
    
    /* This driver does nothing. */
    FX_PARAMETER_NOT_USED(media_ptr);
}
#endif /* FX_ENABLE_EXFAT */
