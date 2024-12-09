/* This FileX test concentrates on the basic media volume get/set operation.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_media.h"
#include   "fx_utility.h"
#include   "fx_ram_driver_test.h"
#include   <stdio.h>

#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              16*128


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

void    filex_media_volume_get_set_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_media_volume_get_set_application_define(void *first_unused_memory)
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
    printf("FileX Test:   Media volume get/set test..............................");

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
                            256,                    // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(2);
    }
    
    /* try to set the media volume name before the media is opened */
    status =  fx_media_volume_set(&ram_disk, "NEW");
    if (status != FX_MEDIA_NOT_OPEN) 
    {
        printf("ERROR!\n");
        test_control_return(11);
    }
    
    /* try to get the media volume name before the media is opened */
    status =  fx_media_volume_get(&ram_disk, volume_buffer, FX_BOOT_SECTOR);
    if (status != FX_MEDIA_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(11);
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
    
/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send null pointer to generate an error */
    status = fx_media_volume_get(FX_NULL, volume_buffer, FX_DIRECTORY_SECTOR);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(11);
    }

    /* send null pointer to generate an error for extended version */
    status = fx_media_volume_get_extended(FX_NULL, volume_buffer, 0, FX_DIRECTORY_SECTOR);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(11);
    }

    /* send null pointer to generate an error */
    status = fx_media_volume_set(FX_NULL, "");
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(11);
    }
#else
    /* try to set the media volume name to something invalid */
    status =  fx_media_volume_set(&ram_disk, "");
    if (status != FX_INVALID_NAME) 
    {
        printf("ERROR!\n");
        test_control_return(11);
    }
#endif /* FX_DISABLE_ERROR_CHECKING */
    
    /* Get the current volume name - from boot record.  */
    status =  fx_media_volume_get(&ram_disk, volume_buffer, FX_BOOT_SECTOR);
    
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
    
    /* Set volume_buffer to known state.  */
    memcpy(volume_buffer, "ORIGINALSTRING", 15);

    /* Get the current volume name - from boot record, using extended version.  */
    status = fx_media_volume_get_extended(&ram_disk, volume_buffer, 2, FX_BOOT_SECTOR);

    /* Determine if there was an error getting the volume name.  */
    if ((status != FX_BUFFER_ERROR) ||
        (volume_buffer[0] != 'M') ||
        (volume_buffer[1] != '\0') ||
        (volume_buffer[2] != 'I') ||
        (volume_buffer[3] != 'G'))
    {

        /* Error getting the volume name.  */
        printf("ERROR!\n");
        test_control_return(4);
    }


    /* Get the current volume name - from root directory.  */
    status =  fx_media_volume_get(&ram_disk, volume_buffer, FX_DIRECTORY_SECTOR);
            
    /* Determine if we found a volume name in the root directory.
     * When not, the value from boot sector will be returned. */
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
        test_control_return(5);
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
    status =  fx_media_volume_get(&ram_disk, volume_buffer, FX_BOOT_SECTOR);
    
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
    
    /* Get the current volume name - from root directory.  */
    status =  fx_media_volume_get(&ram_disk, volume_buffer, FX_DIRECTORY_SECTOR);
            
    /* Determine if we found a volume name in the root directory... should not since our
       format doesn't do that!  */
    if ((status != FX_SUCCESS) ||
        (volume_buffer[0] != 'N') ||
        (volume_buffer[1] != 'E') ||
        (volume_buffer[2] != 'W') ||
        (volume_buffer[3] != (CHAR) 0))
    {
    
        /* Error getting the volume name.  */
        printf("ERROR!\n");
        test_control_return(8);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(10);
    }

    /* Format the media in FAT32.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            70000,                   // Total sectors - FAT 32
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(11);
    }
    

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(12);
    }

    /* Set the volume name to a NULL volume name.   */
    volume_buffer[0] =  0;
    status =  _fx_media_volume_set(&ram_disk, volume_buffer);
    if (status != FX_INVALID_NAME) 
    {
        printf("ERROR!\n");
        test_control_return(13);
    }
    
    /* Now set the volume name to something longer than 11 characters that are lower case.  */
    status =  fx_media_volume_set(&ram_disk, "thisisareallylongvolumename");
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(14);
    }

    /* Now set the volume name to something small with lower case.  */
    status =  fx_media_volume_set(&ram_disk, "sm");
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(15);
    }

    /* Now get the volume name from the boot sector.  */
    status =  fx_media_volume_get(&ram_disk, volume_buffer, FX_BOOT_SECTOR);
    if ((status != FX_SUCCESS) || (volume_buffer[0] != 'S') || (volume_buffer[1] != 'M') || (volume_buffer[2] != 0))
    {
        printf("ERROR!\n");
        test_control_return(16);
    }

    /* Now get the volume name from the directory sector.  */
    status =  fx_media_volume_get(&ram_disk, volume_buffer, FX_DIRECTORY_SECTOR);
    if ((status != FX_SUCCESS) || (volume_buffer[0] != 'S') || (volume_buffer[1] != 'M') || (volume_buffer[2] != 0))
    {
        printf("ERROR!\n");
        test_control_return(17);
    }

    /* Set volume_buffer to known state.  */
    memcpy(volume_buffer, "ORIGINALSTRING", 15);

    /* Get the current volume name - from root directory.  */
    status = fx_media_volume_get_extended(&ram_disk, volume_buffer, 2, FX_DIRECTORY_SECTOR);

    /* Determine if we found a volume name in the root directory.
     * When not, the value from boot sector will be returned. */
    if ((status != FX_BUFFER_ERROR) ||
        (volume_buffer[0] != 'S') ||
        (volume_buffer[1] != '\0') ||
        (volume_buffer[2] != 'I') ||
        (volume_buffer[3] != 'G'))
    {

        /* Error getting the volume name.  */
        printf("ERROR!\n");
        test_control_return(17);
    }
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(18);
    }
    

    /* Format the media in FAT16.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            16,                     // Directory Entries
                            0,                      // Hidden sectors
                            7000,                   // Total sectors - FAT 16
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(19);
    }
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(20);
    }

    /* Fill up the root directory (16 entries), so the volume set won't find an empty entry.  */
    status =   fx_file_create(&ram_disk, "FILE1.TXT");
    status +=  fx_file_create(&ram_disk, "FILE2.TXT");
    status +=  fx_file_create(&ram_disk, "FILE3.TXT");
    status +=  fx_file_create(&ram_disk, "FILE4.TXT");
    status +=  fx_file_create(&ram_disk, "FILE5.TXT");
    status +=  fx_file_create(&ram_disk, "FILE6.TXT");
    status +=  fx_file_create(&ram_disk, "FILE7.TXT");
    status +=  fx_file_create(&ram_disk, "FILE8.TXT");
    status +=  fx_file_create(&ram_disk, "FILE9.TXT");
    status +=  fx_file_create(&ram_disk, "FILE10.TXT");
    status +=  fx_file_create(&ram_disk, "FILE11.TXT");
    status +=  fx_file_create(&ram_disk, "FILE12.TXT");
    status +=  fx_file_create(&ram_disk, "FILE13.TXT");
    status +=  fx_file_create(&ram_disk, "FILE14.TXT");
    status +=  fx_file_create(&ram_disk, "FILE15.TXT");
    status +=  fx_file_create(&ram_disk, "FILE16.TXT");

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(21);
    }
    
    /* Now set the volume name.  */
    status =  fx_media_volume_set(&ram_disk, "MINE ");
    if (status != FX_MEDIA_INVALID)
    {
        printf("ERROR!\n");
        test_control_return(22);
    }

    /* Now delete the first entry so we can find it.  */
    status =  fx_file_delete(&ram_disk, "FILE1.TXT");
    
    /* And set the volume name again.  */
    status +=  fx_media_volume_set(&ram_disk, "MINE ");
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(23);
    }

    /* Set the volume name again - should just use the same directory entry.  */
    status =  fx_media_volume_set(&ram_disk, "MINE1 ");
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(24);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(25);
    }

    /* Format the media in FAT16.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            16,                     // Directory Entries
                            0,                      // Hidden sectors
                            7000,                   // Total sectors - FAT 16
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(26);
    }
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(27);
    }

    /* Fill up the root directory (16 entries), so the volume set won't find an empty entry.  */
    status =   fx_file_create(&ram_disk, "FILE1.TXT");
    status +=  fx_file_create(&ram_disk, "FILE2.TXT");
    status +=  fx_file_create(&ram_disk, "FILE3.TXT");
    status +=  fx_file_create(&ram_disk, "FILE4.TXT");
    status +=  fx_file_create(&ram_disk, "FILE5.TXT");
    status +=  fx_file_create(&ram_disk, "FILE6.TXT");
    status +=  fx_file_create(&ram_disk, "FILE7.TXT");
    status +=  fx_file_create(&ram_disk, "FILE8.TXT");
    status +=  fx_file_create(&ram_disk, "FILE9.TXT");
    status +=  fx_file_create(&ram_disk, "FILE10.TXT");
    status +=  fx_file_create(&ram_disk, "FILE11.TXT");
    status +=  fx_file_create(&ram_disk, "FILE12.TXT");
    status +=  fx_file_create(&ram_disk, "FILE13.TXT");
    status +=  fx_file_create(&ram_disk, "FILE14.TXT");
    status +=  fx_file_create(&ram_disk, "FILE15.TXT");
    status +=  fx_file_create(&ram_disk, "FILE16.TXT");

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(28);
    }
    
    /* Now delete the first entry so we can find it.  */
    status =  fx_file_delete(&ram_disk, "FILE16.TXT");
    
    /* And set the volume name again.  */
    status +=  fx_media_volume_set(&ram_disk, "mine~ ");
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(29);
    }

    /* Set the volume name again - should just use the same directory entry.  */
    status =  fx_media_volume_set(&ram_disk, "MINE1 ");
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(30);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(31);
    }
    
    /* Format the media in FAT16.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            16,                     // Directory Entries
                            0,                      // Hidden sectors
                            7000,                   // Total sectors - FAT 16
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(32);
    }
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(33);
    }

    /* Almost fill up the root directory (15 entries), so the volume set won't find an empty entry.  */
    status =   fx_file_create(&ram_disk, "FILE1.TXT");
    status +=  fx_file_create(&ram_disk, "FILE2.TXT");
    status +=  fx_file_create(&ram_disk, "FILE3.TXT");
    status +=  fx_file_create(&ram_disk, "FILE4.TXT");
    status +=  fx_file_create(&ram_disk, "FILE5.TXT");
    status +=  fx_file_create(&ram_disk, "FILE6.TXT");
    status +=  fx_file_create(&ram_disk, "FILE7.TXT");
    status +=  fx_file_create(&ram_disk, "FILE8.TXT");
    status +=  fx_file_create(&ram_disk, "FILE9.TXT");
    status +=  fx_file_create(&ram_disk, "FILE10.TXT");
    status +=  fx_file_create(&ram_disk, "FILE11.TXT");
    status +=  fx_file_create(&ram_disk, "FILE12.TXT");
    status +=  fx_file_create(&ram_disk, "FILE13.TXT");
    status +=  fx_file_create(&ram_disk, "FILE14.TXT");
    status +=  fx_file_create(&ram_disk, "FILE15.TXT");

    /* Leave the last entry free for search test.  */
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(34);
    }
     
    /* Set the volume name.  */
    status +=  fx_media_volume_set(&ram_disk, "MY ");
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(35);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(36);
    }    

    /* Format the media in FAT16.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            16,                     // Directory Entries
                            0,                      // Hidden sectors
                            7000,                   // Total sectors - FAT 16
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(37);
    }
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(38);
    }

    /* Almost fill up the root directory (15 entries), so the volume set won't find an empty entry.  */
    status =   fx_file_create(&ram_disk, "FILE1.TXT");
    status +=  fx_file_create(&ram_disk, "FILE2.TXT");
    status +=  fx_file_create(&ram_disk, "FILE3.TXT");
    status +=  fx_file_create(&ram_disk, "FILE4.TXT");
    status +=  fx_file_create(&ram_disk, "FILE5.TXT");
    status +=  fx_file_create(&ram_disk, "FILE6.TXT");
    status +=  fx_file_create(&ram_disk, "FILE7.TXT");
    status +=  fx_file_create(&ram_disk, "FILE8.TXT");
    status +=  fx_file_create(&ram_disk, "FILE9.TXT");
    status +=  fx_file_create(&ram_disk, "FILE10.TXT");
    status +=  fx_file_create(&ram_disk, "FILE11.TXT");
    status +=  fx_file_create(&ram_disk, "FILE12.TXT");
    status +=  fx_file_create(&ram_disk, "FILE13.TXT");
    status +=  fx_file_create(&ram_disk, "FILE14.TXT");
    status +=  fx_file_create(&ram_disk, "FILE15.TXT");

    /* Leave the last entry free for search test... and delete file 5 and 6 to create some available slots.  */
    status +=  fx_file_delete(&ram_disk, "FILE6.TXT");
    status +=  fx_file_delete(&ram_disk, "FILE7.TXT");
       
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(39);
    }
     
    /* Set the volume name.  */
    status =  fx_media_volume_set(&ram_disk, "MY ");
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(40);
    }

    /* Set the volume name again.  */
    status =  fx_media_volume_set(&ram_disk, "MY ");
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(41);
    }

    /* Test the I/O errors.  */
    
    /* Cause an error reading sector 0.  */
    _fx_utility_logical_sector_flush(&ram_disk, 0, 60000, FX_TRUE);
    _fx_ram_driver_io_error_request =  1;
    status =  fx_media_volume_set(&ram_disk, "MY ");
    _fx_ram_driver_io_error_request =  0;
    if (status != FX_IO_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(42);
    }

    /* Cause an error reading the boot sector.  */
    _fx_utility_logical_sector_flush(&ram_disk, 0, 60000, FX_TRUE);
    _fx_ram_driver_io_error_request =  2;
    status =  fx_media_volume_set(&ram_disk, "MY ");
    _fx_ram_driver_io_error_request =  0;
    if (status != FX_IO_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(43);
    }

    /* Cause an error writing the boot sector.  */
    _fx_utility_logical_sector_flush(&ram_disk, 0, 60000, FX_TRUE);
    _fx_ram_driver_io_error_request =  2;
    status =  fx_media_volume_set(&ram_disk, "MY ");
    _fx_ram_driver_io_error_request =  0;
    if (status != FX_IO_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(44);
    }

    /* Cause an error reading the first directory entry.  */
    _fx_utility_logical_sector_flush(&ram_disk, 0, 60000, FX_TRUE);
    _fx_ram_driver_io_error_request =  4;
    status =  fx_media_volume_set(&ram_disk, "MY ");
    _fx_ram_driver_io_error_request =  0;
    if (status != FX_IO_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(45);
    }

    /* Cause an error reading the logical directory sector.  */
    _fx_utility_logical_sector_flush(&ram_disk, 0, 60000, FX_TRUE);
    _fx_utility_logical_sector_read_error_request =  8;
    status =  fx_media_volume_set(&ram_disk, "MY ");
    _fx_utility_logical_sector_write_error_request =  0;
    if (status != FX_IO_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(46);
    }

    /* Cause an error reading sector 0.  */
    _fx_utility_logical_sector_flush(&ram_disk, 0, 60000, FX_TRUE);
    _fx_ram_driver_io_error_request =  1;
    status =  fx_media_volume_get(&ram_disk, volume_buffer, FX_BOOT_SECTOR);
    _fx_ram_driver_io_error_request =  0;
    if (status != FX_IO_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(47);
    }

    /* Cause an error reading the boot sector.  */
    _fx_utility_logical_sector_flush(&ram_disk, 0, 60000, FX_TRUE);
    _fx_ram_driver_io_error_request =  2;
    status =  fx_media_volume_get(&ram_disk, volume_buffer, FX_BOOT_SECTOR);
    _fx_ram_driver_io_error_request =  0;
    if (status != FX_IO_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(48);
    }

    /* Cause an error reading the directory sector.  */
    _fx_utility_logical_sector_flush(&ram_disk, 0, 60000, FX_TRUE);
    _fx_ram_driver_io_error_request =  1;
    status =  fx_media_volume_get(&ram_disk, volume_buffer, FX_DIRECTORY_SECTOR);
    _fx_ram_driver_io_error_request =  0;
    if (status != FX_IO_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(49);
    }

    /* Set a maximum length VOLUME NAME and retrieve it.  */
    status =  fx_media_volume_set(&ram_disk, "MYVOLUME123");
    status += fx_media_volume_get(&ram_disk, volume_buffer, FX_DIRECTORY_SECTOR);

    /* Check for status.  */
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(50);
    }

    /* Set a maximum length VOLUME NAME and retrieve it.  */
    status =  fx_media_volume_set(&ram_disk, "           ");
    status += fx_media_volume_get(&ram_disk, volume_buffer, FX_BOOT_SECTOR);

    /* Check for status.  */
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(53);
    }

    status = fx_media_volume_get(&ram_disk, volume_buffer, FX_DIRECTORY_SECTOR);
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(51);
    }    

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(52);
    }    

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
                            256,                    // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail(status == FX_SUCCESS);

    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail(status == FX_SUCCESS);

    status =  fx_media_volume_set(&ram_disk, "NEW");
    return_if_fail(status == FX_SUCCESS);

    /* Fail to read the directory entry of the volume. */
    _fx_utility_logical_sector_read_error_request = 2;
    status = fx_media_volume_get(&ram_disk, volume_buffer, FX_DIRECTORY_SECTOR);
    return_if_fail(status == FX_IO_ERROR);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(53);
    }  

    /* This test is added to check if fx_media_volume_get() returns volume entry that was marked free */
    memset(ram_disk_memory,'\0',sizeof(ram_disk_memory));

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "NO NAME",              // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            70000 * 8,              // Total sectors 
                            512,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail(status == FX_SUCCESS);

    status =  fx_media_open(&ram_disk, "RAMDISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail(status == FX_SUCCESS);;

    /* volume_buffer will contain "NO NAME" */
    status = fx_media_volume_get(&ram_disk, volume_buffer, FX_DIRECTORY_SECTOR);
    return_if_fail(status == FX_SUCCESS);

    status =  fx_media_volume_set(&ram_disk, "56781234");
    return_if_fail(status == FX_SUCCESS);
    fx_media_cache_invalidate(&ram_disk);

    /* volume_buffer will contain "56781234" */
    status = fx_media_volume_get(&ram_disk, volume_buffer, FX_BOOT_SECTOR);
    return_if_fail(status == FX_SUCCESS);

    /* volume_buffer will contain "56781234" */
    status = fx_media_volume_get(&ram_disk, volume_buffer, FX_DIRECTORY_SECTOR);
    return_if_fail(status == FX_SUCCESS);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(54);
    }  

    /* Modify first character of the volume label entry in boot sector. This is done to verify 
    if the volume entry marked free is returned or not */
    ram_disk_memory[71] = 0x39;

    /* First Character of Volume label in data section 
    The absolute address was found by looping accross the ram_disk_memory to find the Volume name in data section */
    ram_disk_memory[2241024] = FX_DIR_ENTRY_FREE;

    memset(cache_buffer,'\0',sizeof(cache_buffer));
    
    status =  fx_media_open(&ram_disk, "RAMDISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail(status == FX_SUCCESS);;
    status = fx_media_volume_get(&ram_disk, volume_buffer, FX_DIRECTORY_SECTOR);
    return_if_fail(status == FX_SUCCESS);
        
    if ((status != FX_SUCCESS) || (volume_buffer[0] != '9'))
    {
        printf("ERROR!\n");
        test_control_return(55);
    }  

    printf("SUCCESS!\n");
    test_control_return(0);
}

