/* This FileX test concentrates on the media with hidden sectors.  */

#include   "fx_api.h"
#include   "fx_ram_driver_test.h"

#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              128*128

#define     HIDDEN_SECTORS          8

#ifdef FX_ENABLE_EXFAT
#define     SECTOR_SIZE             FX_EXFAT_SECTOR_SIZE
#else
#define     SECTOR_SIZE             512
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


/* Define thread prototypes.  */

void    filex_media_hidden_sectors_test_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);
static VOID  hidden_sectors_driver(FX_MEDIA *media_ptr);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_media_hidden_sectors_test_application_define(void *first_unused_memory)
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
    printf("FileX Test:   Media Hidden Sectors Test..............................");

    /* Format the media.  This needs to be done before opening it!  */
#ifdef FX_ENABLE_EXFAT
    status =  fx_media_exFAT_format(&ram_disk, 
                            hidden_sectors_driver,  // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            HIDDEN_SECTORS,         // Hidden sectors
                            256,                    // Total sectors 
                            SECTOR_SIZE,            // Sector size   
                            1,                      // exFAT Sectors per cluster
                            12345,                  // Volume ID
                            0);                     // Boundary unit
#else
    status =  fx_media_format(&ram_disk, 
                            hidden_sectors_driver,  // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            HIDDEN_SECTORS,         // Hidden sectors
                            256,                    // Total sectors 
                            SECTOR_SIZE,            // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
#endif

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(2);
    }

    memset(&ram_disk, 0, sizeof(ram_disk));

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", hidden_sectors_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(3);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(4);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);   

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(5);
    }

    memset(&ram_disk, 0, sizeof(ram_disk));

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", hidden_sectors_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(6);
    }

    /* Open the test file.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(7);
    }

    /* Close the file.  */
    status =  fx_file_close(&my_file);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(8);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);   

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(9);
    }

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

static VOID  hidden_sectors_driver(FX_MEDIA *media_ptr)
{
UCHAR       *source_buffer;
UCHAR       *destination_buffer;
UINT        bytes_per_sector;   

    if (media_ptr -> fx_media_driver_request == FX_DRIVER_BOOT_READ)
    {

        /* Read the boot record and return to the caller.  */

        /* Calculate the RAM disk boot sector offset, which is at the very beginning of the
           RAM disk. Note the RAM disk memory is pointed to by the fx_media_driver_info pointer, 
           which is supplied by the application in the call to fx_media_open.  */
        source_buffer =  (UCHAR *) media_ptr -> fx_media_driver_info;
        source_buffer += SECTOR_SIZE * HIDDEN_SECTORS;

        /* For RAM driver, determine if the boot record is valid.  */
        if ((source_buffer[0] != (UCHAR) 0xEB)  ||
           ((source_buffer[1] != (UCHAR) 0x34)  &&
            (source_buffer[1] != (UCHAR) 0x76)) ||      /* exFAT jump code.  */
            (source_buffer[2] != (UCHAR) 0x90))
        {

            /* Invalid boot record, return an error!  */
            media_ptr -> fx_media_driver_status =  FX_MEDIA_INVALID;
            return;
        }

        /* For RAM disk only, pickup the bytes per sector.  */
        bytes_per_sector =  _fx_utility_16_unsigned_read(&source_buffer[FX_BYTES_SECTOR]);

#ifdef FX_ENABLE_EXFAT
        /* if byte per sector is zero, then treat it as exFAT volume.  */
        if (bytes_per_sector == 0 && (source_buffer[1] == (UCHAR) 0x76))
        {

            /* Pickup the byte per sector shift, and calculate byte per sector.  */
            bytes_per_sector = (UINT)(1 << source_buffer[FX_EF_BYTE_PER_SECTOR_SHIFT]);
        }
#endif /* FX_ENABLE_EXFAT */

        /* Ensure this is less than the destination.  */

        /* Copy the RAM boot sector into the destination.  */
        _fx_utility_memory_copy(source_buffer, media_ptr -> fx_media_driver_buffer, bytes_per_sector);

        /* Successful driver request.  */
        media_ptr -> fx_media_driver_status =  FX_SUCCESS;
    }
    else if (media_ptr -> fx_media_driver_request == FX_DRIVER_BOOT_WRITE)
    {

        /* Write the boot record and return to the caller.  */

        /* Calculate the RAM disk boot sector offset, which is at the very beginning of the
           RAM disk. Note the RAM disk memory is pointed to by the fx_media_driver_info pointer, 
           which is supplied by the application in the call to fx_media_open.  */
        destination_buffer =  (UCHAR *) media_ptr -> fx_media_driver_info;
        destination_buffer += SECTOR_SIZE * HIDDEN_SECTORS;

        /* Copy the RAM boot sector into the destination.  */
        _fx_utility_memory_copy(media_ptr -> fx_media_driver_buffer, destination_buffer, media_ptr -> fx_media_bytes_per_sector);

        /* Successful driver request.  */
        media_ptr -> fx_media_driver_status =  FX_SUCCESS;
    }
    else
    {
        if ((media_ptr -> fx_media_driver_request == FX_DRIVER_WRITE) || (media_ptr -> fx_media_driver_request == FX_DRIVER_READ))
        {
            if (media_ptr -> fx_media_hidden_sectors == 0)
            {
                media_ptr -> fx_media_driver_status =  FX_IO_ERROR;
                return;
            }
        }
        _fx_ram_driver(media_ptr);
    }
}
