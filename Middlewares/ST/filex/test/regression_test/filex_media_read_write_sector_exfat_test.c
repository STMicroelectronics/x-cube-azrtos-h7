/* This FileX test concentrates on the basic media read/write sector operation.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_utility.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"

void  test_control_return(UINT status);
#ifndef FX_DISABLE_CACHE
#ifdef FX_ENABLE_EXFAT

#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              128*128


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD               ftest_0;
#endif
static FX_MEDIA                ram_disk;

static UCHAR                   raw_sector_buffer[FX_EXFAT_SECTOR_SIZE];


/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *ram_disk_memory;
static UCHAR                  *cache_buffer;
#else
static UCHAR                   cache_buffer[CACHE_SIZE];
#endif


/* Define thread prototypes.  */

void    filex_media_read_write_sector_exfat_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_media_read_write_sector_exfat_application_define(void *first_unused_memory)
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

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   exFAT Media read/write sector test.....................");

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

    /* Read the backup boot sector.  */
    status =  fx_media_read(&ram_disk, 12, (VOID *) raw_sector_buffer);  

    /* Check the status.  */
    if ((status != FX_SUCCESS) || 
        (raw_sector_buffer[0] != 0xEB) ||
        (raw_sector_buffer[1] != 0x76) ||
        (raw_sector_buffer[2] != 0x90))
    {

        /* Error reading FAT sector.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(4);
    }
    
    /* Write the same, unchanged sector back.  */
    status =  fx_media_write(&ram_disk, 1, (VOID *) raw_sector_buffer);  

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error writting FAT sector.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(5);
    }

    ULONG64     logical_sector;
    ULONG64     temp_total_sectors;
    temp_total_sectors = ram_disk. fx_media_total_sectors;
    
    /* Test for valid sector.  */
    ram_disk. fx_media_total_sectors = 4294967298 ;
    logical_sector =  512;
    status =  _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
    if (status != FX_SUCCESS)
    {

        /* Error writting FAT sector.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(6);
    }

    /* Test for invalid sector.  */
    ram_disk. fx_media_total_sectors = 4294967298 ;
    logical_sector =  4294967299;
    status =  _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
    if (status != FX_SECTOR_INVALID)
    {

        /* Error writting FAT sector.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(7);
    }

    /* Test for valid sector.  */
    ram_disk. fx_media_total_sectors = 4294967298 ;
    logical_sector =  512;
    status =  _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
    if (status != FX_SUCCESS)
    {

        /* Error writting FAT sector.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(8);
    }

    /* Test for invalid sector.  */
    ram_disk. fx_media_total_sectors = 4294967298 ;
    logical_sector =  4294967299;
    status =  _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
    if (status != FX_SECTOR_INVALID)
    {

        /* Error writting FAT sector.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(9);
    }
    ram_disk. fx_media_total_sectors = temp_total_sectors;


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



#else
#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_media_read_write_sector_exfat_application_define(void *first_unused_memory)
#endif
{

    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   exFAT Media read/write sector test.....................N/A\n");

    test_control_return(255);
}
#endif