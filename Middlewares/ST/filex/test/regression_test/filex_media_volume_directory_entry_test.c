#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#include   "tx_thread.h"
#endif
#include   "fx_api.h"
#include   "fx_utility.h"
#include   "fx_ram_driver_test.h"
#include    <stdio.h>
#include    <string.h>

#define     DEMO_STACK_SIZE         8192
/* Set the cache size as the size of one sector causing frequently IO operation. */
#define     CACHE_SIZE              128
#ifdef FX_ENABLE_EXFAT
#define     CACHE_SIZE1             FX_EXFAT_SECTOR_SIZE
#else
#define     CACHE_SIZE1             512
#endif

#define     VOLUME_LABEL            "Vlabel"

/* Define the global variable we may use in the future. */
extern ULONG _fx_ram_driver_copy_default_format;
extern UCHAR large_file_name_format[];

/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD                ftest_0;
#endif
static FX_MEDIA                 ram_disk;

/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                    *ram_disk_memory;
static UCHAR                    *cache_buffer;
static UCHAR                    *cache_buffer1;
#else
static UCHAR                     cache_buffer[CACHE_SIZE];
static UCHAR                     cache_buffer1[CACHE_SIZE1];
#endif

/* Define thread prototypes.  */

void    filex_media_volume_directory_entry_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);

/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_media_volume_directory_entry_application_define(void *first_unused_memory)
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
    cache_buffer1 = pointer;
    pointer = pointer + CACHE_SIZE1;
    ram_disk_memory = pointer;

#endif

    /* Initialize the FileX system.  */
    fx_system_initialize();
#ifdef FX_STANDALONE_ENABLE
    ftest_0_entry(0);
#endif
}

static UCHAR specified_unicode_file_name[] = { 'Z', 0, '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '.', 0, 't', 0, 'x', 0, 't', 0, 0,  0 };

/* Define the test threads.  */

static void    ftest_0_entry(ULONG thread_input)
{
    UINT status;
    ULONG ulength;
    UCHAR destination_name[128];
    UCHAR name_buffer[128];

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Media volume directory entry test......................");

    /* Format the media.  This needs to be done before opening it!  */
    status = fx_media_format(&ram_disk,
        _fx_ram_driver,         // Driver entry            
        ram_disk_memory_large,  // RAM disk memory pointer
        cache_buffer,           // Media buffer pointer
        CACHE_SIZE,             // Media buffer size 
        "MY_RAM_DISK",          // Volume Name
        1,                      // Number of FATs
        32,                     // Directory Entries
        0,                      // Hidden sectors
        70000,                  // Total sectors 
        128,                    // Sector size   
        1,                      // Sectors per cluster
        1,                      // Heads
        1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Create a file with long file name. */
    ulength = fx_unicode_length_get( specified_unicode_file_name);
    status = fx_unicode_file_create( &ram_disk, specified_unicode_file_name, ulength, (CHAR *)destination_name);
    return_if_fail( status == FX_SUCCESS);

    /* Set and get the volume name. */
    status = fx_media_volume_set( &ram_disk, "NO NAME");
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_volume_get( &ram_disk, (CHAR *)name_buffer, FX_DIRECTORY_SECTOR);
    return_if_fail( ( status == FX_SUCCESS) && ( 0 == strcmp( (CHAR *)name_buffer, "NO NAME")));

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Test for creating direcoty with same name as volume" */
    
    /* Format the media.  This needs to be done before opening it!  */
    status = fx_media_format(&ram_disk,
        _fx_ram_driver,         // Driver entry            
        ram_disk_memory_large,  // RAM disk memory pointer
        cache_buffer,           // Media buffer pointer
        CACHE_SIZE,             // Media buffer size 
        "MY_RAM_DISK",          // Volume Name
        1,                      // Number of FATs
        32,                     // Directory Entries
        0,                      // Hidden sectors
        70000,                  // Total sectors 
        128,                    // Sector size   
        1,                      // Sectors per cluster
        1,                      // Heads
        1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);
    
    status =  fx_media_volume_set(&ram_disk, VOLUME_LABEL);
    return_if_fail( status == FX_SUCCESS);

    status =  fx_media_flush(&ram_disk);
    return_if_fail( status == FX_SUCCESS);
    
    status =   fx_directory_create(&ram_disk, VOLUME_LABEL);
    return_if_fail( status == FX_SUCCESS);
    
    status =  fx_media_flush(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);


#ifdef FX_ENABLE_EXFAT
    /* Test for setting volume name same as directory created previously, 
       when disk is originally formated without volume name */

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_exFAT_format(&ram_disk, 
                            _fx_ram_driver,             // Driver entry
                            ram_disk_memory,            // RAM disk memory pointer
                            cache_buffer1,              // Media buffer pointer
                            CACHE_SIZE1,                // Media buffer size 
                            "",                         // Volume Name
                            1,                          // Number of FATs
                            0,                          // Hidden sectors
                            7000,                       // Total sectors 
                            FX_EXFAT_SECTOR_SIZE,  // Sector size   
                            1,                          // exFAT Sectors per cluster
                            12345,                      // Volume ID
                            0);                         // Boundary unit

    return_if_fail( status == FX_SUCCESS);

    status = fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer1, CACHE_SIZE1);
    return_if_fail( status == FX_SUCCESS);
    
    status =   fx_directory_create(&ram_disk, VOLUME_LABEL);
    return_if_fail( status == FX_SUCCESS);
    
    status =  fx_media_flush(&ram_disk);
    return_if_fail( status == FX_SUCCESS);
    
    status =  fx_media_volume_set(&ram_disk, VOLUME_LABEL);
    return_if_fail( status == FX_SUCCESS);

    status =  fx_media_flush(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Test for creating directory with same name as volume, 
       when disk is originally formated without volume name */

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_exFAT_format(&ram_disk, 
                            _fx_ram_driver,             // Driver entry
                            ram_disk_memory,            // RAM disk memory pointer
                            cache_buffer1,              // Media buffer pointer
                            CACHE_SIZE1,                // Media buffer size 
                            "",                         // Volume Name
                            1,                          // Number of FATs
                            0,                          // Hidden sectors
                            7000,                       // Total sectors 
                            FX_EXFAT_SECTOR_SIZE,  // Sector size   
                            1,                          // exFAT Sectors per cluster
                            12345,                      // Volume ID
                            0);                         // Boundary unit

    return_if_fail( status == FX_SUCCESS);

    status = fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer1, CACHE_SIZE1);
    return_if_fail( status == FX_SUCCESS);
    
    status =  fx_media_volume_set(&ram_disk, VOLUME_LABEL);
    return_if_fail( status == FX_SUCCESS);

    status =  fx_media_flush(&ram_disk);
    return_if_fail( status == FX_SUCCESS);
    
    status =   fx_directory_create(&ram_disk, VOLUME_LABEL);
    return_if_fail( status == FX_SUCCESS);
    
    status =  fx_media_flush(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_close( &ram_disk);
    return_if_fail( status == FX_SUCCESS);

#endif

    printf("SUCCESS!\n");
    test_control_return(0);
}
