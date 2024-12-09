/* This FileX test concentrates on the basic media format, open, close operation.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_system.h"
#include   "fx_utility.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"

#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              256*128


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD               ftest_0;
#endif
static FX_MEDIA                ram_disk;
static FX_FILE                 my_file;
static FX_MEDIA                ram_disk1;
static FX_FILE                 my_file1;
static FX_FILE                 my_file2;
static FX_FILE                 my_file3;
static FX_FILE                 my_file4;
static FX_FILE                 my_file5;
static FX_FILE                 my_file6;
static FX_FILE                 my_file7;
static FX_FILE                 my_file8;
static FX_FILE                 my_file9;
static FX_FILE                 my_file10;
static FX_FILE                 my_file11;


/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *ram_disk_memory;
static UCHAR                  *ram_disk_memory1;
static UCHAR                  *cache_buffer;
static UCHAR                  *cache_buffer1;
#else
static UCHAR                   cache_buffer[CACHE_SIZE];
static UCHAR                   cache_buffer1[CACHE_SIZE];
#endif


/* Define thread prototypes.  */

void    filex_media_multiple_format_open_close_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);


static ULONG open_count;
static ULONG close_count;

static void  ram_disk_open(FX_MEDIA *media_ptr)
{

    FX_PARAMETER_NOT_USED(media_ptr);

    open_count++;
}

static void  ram_disk_close(FX_MEDIA *media_ptr)
{

    FX_PARAMETER_NOT_USED(media_ptr);

    close_count++;
}



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_media_multiple_format_open_close_application_define(void *first_unused_memory)
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
    pointer =  pointer + (256*128) /* CACHE_SIZE */;
    cache_buffer1 =  pointer;
    pointer =  pointer + CACHE_SIZE;
    ram_disk_memory =  pointer;
    pointer =  pointer + (256*128);
    ram_disk_memory1 =  pointer;

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

UINT        status, status1;
ULONG       actual;
UINT        i, j;
UCHAR       local_buffer[32];
UCHAR       raw_sector_buffer[512];
UCHAR       volume_buffer[100];
ULONG       temp;
ULONG       temp1;
ULONG       temp2;
ULONG       temp3;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Multiple Media format, open and close test.............");

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

    /* Format the second media. This needs to be done before opening it!  */
    status +=  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory1,        // RAM disk memory pointer
                            cache_buffer1,           // Media buffer pointer
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

    /* Open each ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_media_open(&ram_disk1,"RAM DISK", _fx_ram_driver, ram_disk_memory1,cache_buffer1,CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(3);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");
    status1=  fx_file_create(&ram_disk1, "TEST.TXT");

    /* Check the first create status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(3);
    }

    /* Check the second create status.  */
    if (status1 != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(31);
    }

    /* Open the test files.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk1, &my_file1, "TEST.TXT", FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(4);
    }

    /* Seek to the beginning of the test files.  */
    status =  fx_file_seek(&my_file, 0);
    status += fx_file_seek(&my_file1, 0);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(5);
    }

    /* Write a string to the test files.  */
    status =  fx_file_write(&my_file, " ABCDEFGHIJKLMNOPQRSTUVWXYZ\n", 28);
    status += fx_file_write(&my_file1, " ABCDEFGHIJKLMNOPQRSTUVWXYZ\n", 28);

    /* Check the file write status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(6);
    }

    /* Seek to the beginning of the test files.  */
    status =  fx_file_seek(&my_file, 0);
    status += fx_file_seek(&my_file1, 0);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(7);
    }

    /* Read the first 28 bytes of the test files.  */
    status =  fx_file_read(&my_file, local_buffer, 28, &actual);
    status += fx_file_read(&my_file1, local_buffer, 28, &actual);

    /* Check the file read status.  */
    if ((status != FX_SUCCESS) || (actual != 28))
    {

        printf("ERROR!\n");
        test_control_return(8);
    }

    /* Close the test files.  */
    status =  fx_file_close(&my_file);
    status += fx_file_close(&my_file1);

    /* Check the file close status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(9);
    }

    /* Close the medias.  */
    status =  fx_media_close(&ram_disk);
    status += fx_media_close(&ram_disk1);

    /* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send null pointer to generate an error */
    status = fx_media_close(FX_NULL);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(11);
    }
#endif /* FX_DISABLE_ERROR_CHECKING */

    /* Re-Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_media_open(&ram_disk1,"RAM DISK", _fx_ram_driver, ram_disk_memory1,cache_buffer1,CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(3);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");
    status1=  fx_file_create(&ram_disk1, "TEST.TXT");

    /* Check for an already created status.  This is not fatal, just 
       let the user know.  */
    if (status != FX_ALREADY_CREATED)
    {

        printf("ERROR!\n");
        test_control_return(3);
    }

    /* Check for an already created status.  This is not fatal, just 
       let the user know.  */
    if (status1 != FX_ALREADY_CREATED)
    {

        printf("ERROR!\n");
        test_control_return(31);
    }

    /* Open the test files.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk1, &my_file1, "TEST.TXT", FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(4);
    }

    /* Seek to the beginning of the test files.  */
    status =  fx_file_seek(&my_file, 0);
    status += fx_file_seek(&my_file1, 0);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(5);
    }

    /* Write a string to the test files.  */
    status =  fx_file_write(&my_file, " ABCDEFGHIJKLMNOPQRSTUVWXYZ\n", 28);
    status += fx_file_write(&my_file1, " ABCDEFGHIJKLMNOPQRSTUVWXYZ\n", 28);

    /* Check the file write status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(6);
    }

    /* Seek to the beginning of the test files.  */
    status =  fx_file_seek(&my_file, 0);
    status += fx_file_seek(&my_file1, 0);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(7);
    }

    /* Read the first 28 bytes of the test files.  */
    status =  fx_file_read(&my_file, local_buffer, 28, &actual);
    status += fx_file_read(&my_file1, local_buffer, 28, &actual);

    /* Check the file read status.  */
    if ((status != FX_SUCCESS) || (actual != 28))
    {

        printf("ERROR!\n");
        test_control_return(8);
    }

    /* Close the test files.  */
    status =  fx_file_close(&my_file);
    status += fx_file_close(&my_file1);

    /* Check the file close status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(9);
    }

    /* Close the medias.  */
    status =  fx_media_close(&ram_disk);
    status += fx_media_close(&ram_disk1);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(10);
    }

    /* Test multiple files open at time of close.  */

    /* Re-Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_media_open(&ram_disk1,"RAM DISK", _fx_ram_driver, ram_disk_memory1,cache_buffer1,CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(10);
    }

    /* Create a file called TEST.TXT in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");
    status1=  fx_file_create(&ram_disk1, "TEST.TXT");

    /* Check for an already created status.  This is not fatal, just 
       let the user know.  */
    if (status != FX_ALREADY_CREATED)
    {

        printf("ERROR!\n");
        test_control_return(11);
    }

    /* Check for an already created status.  This is not fatal, just 
       let the user know.  */
    if (status1 != FX_ALREADY_CREATED)
    {

        printf("ERROR!\n");
        test_control_return(12);
    }

    /* Open the test files.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_READ);
    status += fx_file_open(&ram_disk1, &my_file1, "TEST.TXT", FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(13);
    }

    /* Seek to the beginning of the test files.  */
    status =  fx_file_seek(&my_file, 0);
    status += fx_file_seek(&my_file1, 0);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(14);
    }

    /* Write a string to the test files.  */
    status = fx_file_write(&my_file1, " ABCDEFGHIJKLMNOPQRSTUVWXYZ\n", 28);

    /* Check the file write status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(15);
    }

    /* Seek to the beginning of the test files.  */
    status =  fx_file_seek(&my_file, 0);
    status += fx_file_seek(&my_file1, 0);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(16);
    }

    /* Read the first 28 bytes of the test files.  */
    status =  fx_file_read(&my_file, local_buffer, 28, &actual);
    status += fx_file_read(&my_file1, local_buffer, 28, &actual);

    /* Check the file read status.  */
    if ((status != FX_SUCCESS) || (actual != 28))
    {

        printf("ERROR!\n");
        test_control_return(17);
    }

    /* Close the medias.  */
    status =  fx_media_close(&ram_disk1);
    status += fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(18);
    }

    /* FAT32 Test for sixteen files and multiple open/close calls.  */

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
            70000,                  // Total sectors  - FAT32
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

    /* Setup a new, more complicated directory structure.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_file_create(&ram_disk, "ROOTF1.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF2.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF3.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF4.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF5.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF6.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF7.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF8.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF9.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF10.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF11.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF12.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF13.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF14.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF15.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF16.TXT");
    status += fx_media_flush(&ram_disk);

    status += fx_file_open(&ram_disk, &my_file, "ROOTF1.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, "ROOTF1_CONTENTS", 130);
    status += fx_file_close(&my_file);
    status += fx_media_flush(&ram_disk);

    status += fx_file_open(&ram_disk, &my_file, "ROOTF2.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, "ROOTF2_CONTENTS", 130);
    status += fx_file_close(&my_file);
    status += fx_media_flush(&ram_disk);

    status += fx_file_open(&ram_disk, &my_file, "ROOTF3.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, "ROOTF3_CONTENTS", 130);
    status += fx_file_close(&my_file);
    status += fx_media_flush(&ram_disk);

    /* Determine if the files were written properly.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(20);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Open the media again.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* And close it again.  */    
    status +=  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(21);
    }   

    /* Test the notify registration functions.  */

    /* Setup notify routines for open and close.  */    
    status =  fx_media_open_notify_set(&ram_disk, ram_disk_open);
    status += fx_media_close_notify_set(&ram_disk,ram_disk_close);

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(22);
    }

    /* Setup a new, more complicated directory structure.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Determine if the files were written properly.  */
    if ((status) || (open_count != 1))
    {

        printf("ERROR!\n");
        test_control_return(23);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    status += fx_media_open_notify_set(&ram_disk,  FX_NULL);
    status += fx_media_close_notify_set(&ram_disk, FX_NULL);

    /* Determine if the test was successful.  */
    if ((status != FX_SUCCESS) || (close_count != 1))
    {

        printf("ERROR!\n");
        test_control_return(24);
    }   

    /* FAT32 Test for the additional information sector.  */

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
            6000,                   // Total sectors  - FAT16
            512,                    // Sector size   
            1,                      // Sectors per cluster
            1,                      // Heads
            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(24);
    }

    /* Clear the raw sector buffer.  */
    for (i = 0; i < 512; i++)
    {
        raw_sector_buffer[i] =  0;
    }

    /* Setup a new file that we will use for the additional information sector.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_file_create(&ram_disk, "TEXT.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEXT.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, raw_sector_buffer, 512);
    status += fx_media_flush(&ram_disk);

    /* Determine if the files were written properly.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(25);
    }

    /* Now setup the media's additional information sector.  */
    ram_disk.fx_media_FAT32_additional_info_sector =  (my_file.fx_file_first_physical_cluster - 2) + ram_disk.fx_media_data_sector_start;    

    /* Now set the last available clusters to the current available clusters.  */
    ram_disk.fx_media_FAT32_additional_info_last_available =  ram_disk.fx_media_available_clusters;

    /* Flush the media.  */
    status +=  fx_media_flush(&ram_disk);

    /* Now setup the media's additional information sector.  */
    ram_disk.fx_media_FAT32_additional_info_sector =  (my_file.fx_file_first_physical_cluster - 2) + ram_disk.fx_media_data_sector_start;    

    /* Now set the last available clusters to the current available clusters.  */
    ram_disk.fx_media_FAT32_additional_info_last_available =  ram_disk.fx_media_available_clusters;

    /* Close the media.  */
    status +=  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(26);
    }   

    /* Open the media again.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_file_open(&ram_disk, &my_file, "TEXT.TXT", FX_OPEN_FOR_WRITE);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(27);
    }   

    /* Now setup the media's additional information sector.  */
    ram_disk.fx_media_FAT32_additional_info_sector =  (my_file.fx_file_first_physical_cluster - 2) + ram_disk.fx_media_data_sector_start;    

    /* Now clear the last available clusters to the current available clusters.  */
    ram_disk.fx_media_FAT32_additional_info_last_available =  0;

    /* Now temporarily set the write protect flag.  */
    ram_disk.fx_media_driver_write_protect =  FX_FALSE;

    /* Flush the media.  */
    status +=  fx_media_flush(&ram_disk);

    /* Now setup the media's additional information sector.  */
    ram_disk.fx_media_FAT32_additional_info_sector =  (my_file.fx_file_first_physical_cluster - 2) + ram_disk.fx_media_data_sector_start;    

    /* Now clear the last available clusters to the current available clusters.  */
    ram_disk.fx_media_FAT32_additional_info_last_available =  0;

    /* Now temporarily set the write protect flag.  */
    ram_disk.fx_media_driver_write_protect =  FX_TRUE;

    /* Close the media.  */
    status +=  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(28);
    }   

    /* Open the media again.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_file_open(&ram_disk, &my_file, "TEXT.TXT", FX_OPEN_FOR_WRITE);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(27);
    }   

    /* Now setup the media's additional information sector.  */
    ram_disk.fx_media_FAT32_additional_info_sector =  (my_file.fx_file_first_physical_cluster - 2) + ram_disk.fx_media_data_sector_start;    

    /* Now clear the last available clusters to the current available clusters.  */
    ram_disk.fx_media_FAT32_additional_info_last_available =  0;

    /* Clear the write protect flag.  */
    ram_disk.fx_media_driver_write_protect =  FX_FALSE;

    /* Flush the media.  */
    status +=  fx_media_flush(&ram_disk);

    /* Now setup the media's additional information sector.  */
    ram_disk.fx_media_FAT32_additional_info_sector =  (my_file.fx_file_first_physical_cluster - 2) + ram_disk.fx_media_data_sector_start;    

    /* Now clear the last available clusters to the current available clusters.  */
    ram_disk.fx_media_FAT32_additional_info_last_available =  0;

    /* Clear the write protect flag.  */
    ram_disk.fx_media_driver_write_protect =  FX_FALSE;

    /* Close the media... This will attempt to look for the additional information sector but fail since the signature is not present.  */
    status +=  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(29);
    }   

    /* Open the media again.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_file_open(&ram_disk, &my_file, "TEXT.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_seek(&my_file, 0);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(30);
    }   

    /* Now setup the media's additional information sector.  */
    ram_disk.fx_media_FAT32_additional_info_sector =  (my_file.fx_file_first_physical_cluster - 2) + ram_disk.fx_media_data_sector_start;    

    /* Now clear the last available clusters to the current available clusters.  */
    ram_disk.fx_media_FAT32_additional_info_last_available =  0;

    /* Now temporarily set the write protect flag.  */
    ram_disk.fx_media_driver_write_protect =  FX_FALSE;

    /* Read the file contents.  */
    status =  fx_file_read(&my_file, raw_sector_buffer, 512, &actual);

    /* Write the first signature.  */
    _fx_utility_32_unsigned_write(&raw_sector_buffer[0], 0x41615252);

    /* Write the signature back to the file.  */
    status += fx_file_seek(&my_file, 0);
    status += fx_file_write(&my_file, raw_sector_buffer, 512);

    /* Flush the media.  */
    status +=  fx_media_flush(&ram_disk);

    /* Now setup the media's additional information sector.  */
    ram_disk.fx_media_FAT32_additional_info_sector =  (my_file.fx_file_first_physical_cluster - 2) + ram_disk.fx_media_data_sector_start;    

    /* Now clear the last available clusters to the current available clusters.  */
    ram_disk.fx_media_FAT32_additional_info_last_available =  0;

    /* Now temporarily set the write protect flag.  */
    ram_disk.fx_media_driver_write_protect =  FX_FALSE;

    /* Close the media... This will attempt to look for the additional information sector but fail since the second signature is not present.  */
    status +=  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(31);
    }   

    /* Open the media again.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_file_open(&ram_disk, &my_file, "TEXT.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_seek(&my_file, 0);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(32);
    }   

    /* Now setup the media's additional information sector.  */
    ram_disk.fx_media_FAT32_additional_info_sector =  (my_file.fx_file_first_physical_cluster - 2) + ram_disk.fx_media_data_sector_start;    

    /* Now clear the last available clusters to the current available clusters.  */
    ram_disk.fx_media_FAT32_additional_info_last_available =  0;

    /* Now clear the write protect flag.  */
    ram_disk.fx_media_driver_write_protect =  FX_FALSE;

    /* Read the file contents.  */
    status =  fx_file_read(&my_file, raw_sector_buffer, 512, &actual);

    /* Write the last signature.  */
    fx_file_seek(&my_file, 0);
    _fx_utility_32_unsigned_write(&raw_sector_buffer[484], 0x61417272);

    /* Write the signature back to the file.  */
    status += fx_file_seek(&my_file, 0);
    status += fx_file_write(&my_file, raw_sector_buffer, 512);

    /* Flush the media.  */
    status +=  fx_media_flush(&ram_disk);

    /* Now clear the last available clusters to the current available clusters.  */
    ram_disk.fx_media_FAT32_additional_info_last_available =  0;

    /* Now clear the write protect flag.  */
    ram_disk.fx_media_driver_write_protect =  FX_FALSE;

    /* Close the media... This will attempt to look for the additional information sector and succeed in updating the additional sector. */
    status +=  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(33);
    }   

    /* Create some additional files.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_file_create(&ram_disk, "TEXT1.TXT");
    status += fx_file_create(&ram_disk, "TEXT2.TXT");
    status += fx_file_create(&ram_disk, "TEXT3.TXT");
    status += fx_file_create(&ram_disk, "TEXT4.TXT");
    status += fx_file_create(&ram_disk, "TEXT5.TXT");
    status += fx_file_create(&ram_disk, "TEXT6.TXT");
    status += fx_file_create(&ram_disk, "TEXT7.TXT");
    status += fx_file_create(&ram_disk, "TEXT8.TXT");
    status += fx_file_create(&ram_disk, "TEXT9.TXT");
    status += fx_file_create(&ram_disk, "TEXT10.TXT");
    status += fx_file_create(&ram_disk, "TEXT11.TXT");
    status += fx_media_close(&ram_disk);    

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(34);
    }   

    /* Loop to create the environment for I/O errors...  */
    for (i = 0; i < 1000; i++)
    {

        /* Open the media again.  */
        status = fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 512);
        status += fx_file_open(&ram_disk, &my_file, "TEXT.TXT", FX_OPEN_FOR_READ);
        status += fx_file_open(&ram_disk, &my_file1, "TEXT1.TXT", FX_OPEN_FOR_WRITE);
        status += fx_file_open(&ram_disk, &my_file2, "TEXT2.TXT", FX_OPEN_FOR_WRITE);
        status += fx_file_open(&ram_disk, &my_file3, "TEXT3.TXT", FX_OPEN_FOR_WRITE);
        status += fx_file_open(&ram_disk, &my_file4, "TEXT4.TXT", FX_OPEN_FOR_WRITE);
        status += fx_file_open(&ram_disk, &my_file5, "TEXT5.TXT", FX_OPEN_FOR_WRITE);
        status += fx_file_open(&ram_disk, &my_file6, "TEXT6.TXT", FX_OPEN_FOR_WRITE);
        status += fx_file_open(&ram_disk, &my_file7, "TEXT7.TXT", FX_OPEN_FOR_WRITE);
        status += fx_file_open(&ram_disk, &my_file8, "TEXT8.TXT", FX_OPEN_FOR_WRITE);
        status += fx_file_open(&ram_disk, &my_file9, "TEXT9.TXT", FX_OPEN_FOR_WRITE);
        status += fx_file_open(&ram_disk, &my_file10, "TEXT10.TXT", FX_OPEN_FOR_WRITE);
        status += fx_file_open(&ram_disk, &my_file11, "TEXT11.TXT", FX_OPEN_FOR_WRITE);
        raw_sector_buffer[0] = (UCHAR)i;
        status += fx_file_write(&my_file1, raw_sector_buffer, 1);
        status += fx_file_write(&my_file2, raw_sector_buffer, 1);
        status += fx_file_write(&my_file3, raw_sector_buffer, 1);
        status += fx_file_write(&my_file4, raw_sector_buffer, 1);
        status += fx_file_write(&my_file5, raw_sector_buffer, 1);
        status += fx_file_write(&my_file6, raw_sector_buffer, 1);
        status += fx_file_write(&my_file7, raw_sector_buffer, 1);
        status += fx_file_write(&my_file8, raw_sector_buffer, 1);
        status += fx_file_write(&my_file9, raw_sector_buffer, 1);
        status += fx_file_write(&my_file10, raw_sector_buffer, 1);
        status += fx_file_write(&my_file11, raw_sector_buffer, 1);

        /* Determine if the test was successful.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(34);
        }   

        /* Now temporarily set the write protect flag.  */
        ram_disk.fx_media_driver_write_protect =  FX_FALSE;
        status = fx_media_flush(&ram_disk);
        status += fx_media_cache_invalidate(&ram_disk);

        /* Determine if the test was successful.  */
        if (status != FX_SUCCESS)
        {

            printf("ERROR!\n");
            test_control_return(34);
        }   

        /* Now setup the media's additional information sector.  */
        ram_disk.fx_media_FAT32_additional_info_sector =  (my_file.fx_file_first_physical_cluster - 2) + ram_disk.fx_media_data_sector_start;    

        /* Now clear the last available clusters to the current available clusters.  */
        ram_disk.fx_media_FAT32_additional_info_last_available =  0;

        /* Create I/O errors via the RAM driver interface.  */
        j = (UINT)(rand() % 10);
        if (j == 0)
            j =  1;        
        _fx_ram_driver_io_error_request =  j;

        fx_media_flush(&ram_disk);

        /* Now setup the media's additional information sector.  */
        ram_disk.fx_media_FAT32_additional_info_sector =  (my_file.fx_file_first_physical_cluster - 2) + ram_disk.fx_media_data_sector_start;    

        /* Now clear the last available clusters to the current available clusters.  */
        ram_disk.fx_media_FAT32_additional_info_last_available =  0;

        /* Create I/O errors via the RAM driver interface.  */
        j = (UINT)(rand() % 10);
        if (j == 0)
            j =  1;        
        _fx_ram_driver_io_error_request =  j;

        /* Close the media... This will attempt to look for the additional information sector and succeed in updating the additional sector. */        
        fx_media_close(&ram_disk);       

        /* Create I/O errors via the RAM driver interface.  */
        j = (UINT)(rand() % 10);
        if (j == 0)
            j =  1;        
        _fx_ram_driver_io_error_request =  j;

        /* Set the volume.  */
        fx_media_volume_set(&ram_disk, "MYDISK");

        /* Create I/O errors via the RAM driver interface.  */
        j = (UINT)(rand() % 10);
        if (j == 0)
            j =  1;        
        _fx_ram_driver_io_error_request =  j;

        /* Get the volume.  */
        if (i & 1)
            fx_media_volume_get(&ram_disk, (CHAR *)volume_buffer, FX_BOOT_SECTOR);
        else
            fx_media_volume_get(&ram_disk, (CHAR *)volume_buffer, FX_DIRECTORY_SECTOR);

        _fx_ram_driver_io_error_request =  0;
    }

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(33);
    }   


    /* Additional media open/close corner case tests... including I/O errors.  */

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
            6000,                   // Total sectors  - FAT16
            128,                    // Sector size   
            1,                      // Sectors per cluster
            1,                      // Heads
            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(34);
    }

    /* Clear the raw sector buffer.  */
    for (i = 0; i < 512; i++)
    {
        raw_sector_buffer[i] =  0;
    }

    /* Save the options.  */
    temp =  _fx_system_build_options_1;
    temp1 = _fx_system_build_options_2;
    temp2 = _fx_system_build_options_3;
    temp3 = (ULONG)_fx_version_id[0];

    /* Set all these to 0 to cause the error.  */
    _fx_system_build_options_1 =  0;
    _fx_system_build_options_2 =  0;
    _fx_system_build_options_3 =  0;

    /* Setup a new file that we will use for the additional information sector.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check for the not implemented error, which is expected.  */
    if (status != FX_NOT_IMPLEMENTED)
    {

        printf("ERROR!\n");
        test_control_return(35);
    }

    /* Restore build options and set version_id to 0 to cause the error.  */
    _fx_system_build_options_1 =  temp;
    _fx_system_build_options_2 =  temp1;
    _fx_system_build_options_3 =  temp2;
    _fx_version_id[0] =           0;

    /* Setup a new file that we will use for the additional information sector.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check for the not implemented error, which is expected.  */
    if (status != FX_NOT_IMPLEMENTED)
    {

        printf("ERROR!\n");
        test_control_return(35);
    }

    /* Restore version id.  */
    _fx_version_id[0] =           (CHAR) temp3;


#ifndef FX_DISABLE_CACHE
    /* Now attempt to call media open with a 0 sized cache.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 0);

    /* Check for the buffer error, which is expected.  */
    if (status != FX_BUFFER_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(36);
    }
#endif

    /* Now attempt to call media open with an over-sized cache... this will be reduced automatically and should open fine.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 257*128);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(37);
    }

    /* Abort the media since it wasn't opened properly.  */
    fx_media_abort(&ram_disk);

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
            70000,                  // Total sectors  - FAT32
            512,                    // Sector size   
            1,                      // Sectors per cluster
            1,                      // Heads
            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(38);
    }

    /* Setup a new file that we will use for the additional information sector.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 512);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(39);
    }

    /* Loop to create a bogus FAT chain for the root directory to test the total cluster logic in root directory total entries calculation.  */
    for (i = 2; i < ram_disk.fx_media_total_clusters+10; i++)
    {
        /* Write a FAT chain link that exceeds the FAT area. */
        status += _fx_utility_FAT_entry_write(&ram_disk, i, i+1);
    }   

    /* Now close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(40);
    }

    /* Open the media again to get the FAT read error.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 512);

    /* Check for error.  */
    if (status != FX_FAT_READ_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(41);
    }

    /* Abort the media.  */
    fx_media_abort(&ram_disk);


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
            70000,                  // Total sectors  - FAT32
            512,                    // Sector size   
            1,                      // Sectors per cluster
            1,                      // Heads
            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(42);
    }

    /* Open the media again.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 512);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(43);
    }

     /* Write a FAT chain link to check (cluster_number == FAT_entry). */
    status = _fx_utility_FAT_entry_write(&ram_disk, 2, 2+1);
    
    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(46);
    }

    /* Now close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(40);
    }

    /* Open the media again to get the FAT read error.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 512);

    /* Check for error.  */
    if (status != FX_FAT_READ_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(41);
    }
    
    /* Abort the media.  */
    fx_media_abort(&ram_disk);
 
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
            70000,                  // Total sectors  - FAT32
            512,                    // Sector size   
            1,                      // Sectors per cluster
            1,                      // Heads
            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(42);
    }

    /* Open the media again.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 512);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(43);
    }

    /* Pickup the additional information sector.  */
    temp =  ram_disk.fx_media_FAT32_additional_info_sector;

    /* Read the additional information sector.  */
    fx_media_read(&ram_disk, temp, raw_sector_buffer);

    /* Modify the second signature to exercise the error logic in media open.  */
    _fx_utility_32_unsigned_write(&raw_sector_buffer[484], 0);

    /* Write the second signature back.  */
    fx_media_write(&ram_disk, temp, raw_sector_buffer);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(44);
    }

    /* Now try to re-open with the second signature bad.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 512);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(45);
    }

    /* Fix the signature, but corrupt the available clusters.  */

    /* Read the additional information sector.  */
    fx_media_read(&ram_disk, temp, raw_sector_buffer);

    /* Modify the second signature to exercise the error logic in media open.  */
    _fx_utility_32_unsigned_write(&raw_sector_buffer[484], 0x61417272);

    /* Save the search start and available clusters.  */
    temp1 =  _fx_utility_32_unsigned_read(&raw_sector_buffer[488]);
    temp2 =  _fx_utility_32_unsigned_read(&raw_sector_buffer[492]);

    /* Modify the search start... beyond the total clusters */
    _fx_utility_32_unsigned_write(&raw_sector_buffer[492], 0xF0F0F0F0);

    /* Write the search start back.  */
    fx_media_write(&ram_disk, temp, raw_sector_buffer);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(46);
    }

    /* Now try to re-open with a bad search cluster value - greater than total.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 512);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(47);
    }

    /* Read the additional information sector.  */
    fx_media_read(&ram_disk, temp, raw_sector_buffer);

    /* Modify the search start... before the starting cluster. */
    _fx_utility_32_unsigned_write(&raw_sector_buffer[492], 0);

    /* Write the search start back.  */
    fx_media_write(&ram_disk, temp, raw_sector_buffer);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(48);
    }

    /* Now try to re-open with a bad search cluster value - less than the total.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 512);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(49);
    }

    /* Read the additional information sector.  */
    fx_media_read(&ram_disk, temp, raw_sector_buffer);

    /* Restore the search start... before the starting cluster. */
    _fx_utility_32_unsigned_write(&raw_sector_buffer[492], temp2);

    /* Make the available clusters too big.  */
    _fx_utility_32_unsigned_write(&raw_sector_buffer[488], ram_disk.fx_media_total_clusters+1);

    /* Write the search start back.  */
    fx_media_write(&ram_disk, temp, raw_sector_buffer);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(50);
    }

    /* Now try to re-open with a bad available clusters value - more than the total.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 512);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(51);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(52);
    }

    /* Format the media in FAT12 for testing I/O error on available clusters in media open.  */
    status =  fx_media_format(&ram_disk, 
            _fx_ram_driver,         // Driver entry
            ram_disk_memory,        // RAM disk memory pointer
            cache_buffer,           // Media buffer pointer
            CACHE_SIZE,             // Media buffer size 
            "MY_RAM_DISK",          // Volume Name
            1,                      // Number of FATs
            32,                     // Directory Entries
            0,                      // Hidden sectors
            2000,                   // Total sectors  - FAT12
            128,                    // Sector size   
            1,                      // Sectors per cluster
            1,                      // Heads
            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(53);
    }

    /* Open the media again, but this time introduce an I/O error to cause the FAT read to fail.  */
    _fx_ram_driver_io_error_request =  5;
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    _fx_ram_driver_io_error_request =  0;

    /* Check for error.  */
    if (status != FX_FAT_READ_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(54);
    }

    /* Abort the media.  */
    fx_media_abort(&ram_disk);   

    /* Format the media in FAT16 for testing I/O error on available clusters in media open.  */
    status =  fx_media_format(&ram_disk, 
            _fx_ram_driver,         // Driver entry
            ram_disk_memory,        // RAM disk memory pointer
            cache_buffer,           // Media buffer pointer
            CACHE_SIZE,             // Media buffer size 
            "MY_RAM_DISK",          // Volume Name
            1,                      // Number of FATs
            32,                     // Directory Entries
            0,                      // Hidden sectors
            7000,                   // Total sectors  - FAT16
            128,                    // Sector size   
            1,                      // Sectors per cluster
            1,                      // Heads
            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(55);
    }

    /* Open the media again, but this time introduce an I/O error to cause the FAT read to fail.  */
    _fx_ram_driver_io_error_request =  5;
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    _fx_ram_driver_io_error_request =  0;

    /* Check for error.  */
    if (status != FX_FAT_READ_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(56);
    }

    /* Abort the media.  */
    fx_media_abort(&ram_disk);

    /* Open the media again to setup for a media close I/O error.  */    
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(57);
    }

    /* Create a file.  */    
    status =  fx_file_create(&ram_disk, "TEST.TXT");

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(58);
    }

    /* Open the test files.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(59);
    }

    /* Write to the file.  */
    status =  fx_file_write(&my_file, "1234567890", 10);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(60);
    }

    /* Now attemp to close the media, but with an I/O error introduced so the close will fail trying to write out the directory entry of the open file.  */
    _fx_ram_driver_io_error_request =  1;
    status =  fx_media_close(&ram_disk);
    _fx_ram_driver_io_error_request =  0;

    /* Check for the I/O error.  */
    if (status != FX_IO_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(61);
    }

    /* Abort media.  */
    fx_media_abort(&ram_disk);

    /* Format the media in FAT16 for testing I/O error in media close.  */
    status =  fx_media_format(&ram_disk, 
            _fx_ram_driver,         // Driver entry
            ram_disk_memory,        // RAM disk memory pointer
            cache_buffer,           // Media buffer pointer
            CACHE_SIZE,             // Media buffer size 
            "MY_RAM_DISK",          // Volume Name
            1,                      // Number of FATs
            32,                     // Directory Entries
            0,                      // Hidden sectors
            7000,                   // Total sectors  - FAT16
            128,                    // Sector size   
            1,                      // Sectors per cluster
            1,                      // Heads
            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(62);
    }

    /* Open the media again to setup for a media close I/O error.  */    
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(63);
    }

    /* Create a file.  */    
    status =  fx_file_create(&ram_disk, "TEST.TXT");

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(64);
    }

    /* Open the test files.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(65);
    }

    /* Write to the file.  */
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_media_read(&ram_disk, 2000, raw_sector_buffer);
    status +=  fx_media_write(&ram_disk, 2000, raw_sector_buffer);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(66);
    }    

#ifndef FX_DISABLE_CACHE
#ifdef FX_FAULT_TOLERANT
    /* While FX__FAULT_TOLERANT is defined, non data sector will flush directly in _fx_utility_logical_sector_write rather than set dirty flag. */
    /* For this reason, driver won't be called in _fx_utility_logical_sector_flush which is different from non FAULT_TOLERANT code. */
    _fx_utility_logical_sector_flush_error_request = 1;    
    status =  fx_media_close(&ram_disk);
#else
    /* Now attemp to close the media, but with an I/O error introduced so the close will fail trying to write out the directory entry of the open file.  */
    _fx_ram_driver_io_error_request =  17;
    status =  fx_media_close(&ram_disk);
    _fx_ram_driver_io_error_request =  0;
#endif

    /* Check for the I/O error.  */
    if (status != FX_IO_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(67);
    }
#endif

    /* Finallay, abort the media.  */
    fx_media_abort(&ram_disk);
    
    /* Format the media in FAT16 for testing I/O error in media flush.  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            7000,                   // Total sectors  - FAT16
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(68);
    }

    /* Open the media again to setup for a media close I/O error.  */    
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {
   
        printf("ERROR!\n");
        test_control_return(69);
    }

    /* Create a file.  */    
    status =  fx_file_create(&ram_disk, "TEST.TXT");

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(70);
    }

    /* Open the test files.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    
    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(71);
    }
    
    /* Write to the file.  */
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_media_read(&ram_disk, 2000, raw_sector_buffer);
    status +=  fx_media_write(&ram_disk, 2000, raw_sector_buffer);
    
    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(72);
    }    
    
#if defined(FX_FAULT_TOLERANT) && !defined(FX_DISABLE_CACHE)
    /* While FX__FAULT_TOLERANT is defined, non data sector will flush directly in _fx_utility_logical_sector_write rather than set dirty flag. */
    /* For this reason, driver won't be called in _fx_utility_logical_sector_flush which is different from non FAULT_TOLERANT code. */
    _fx_utility_logical_sector_flush_error_request = 1;    
    status =  fx_media_close(&ram_disk);
#else 
    /* Now attemp to flush the media, but with an I/O error introduced so the close will fail trying to write out the directory entry of the open file.  */
    _fx_ram_driver_io_error_request =  1;
    status =  fx_media_flush(&ram_disk);
    _fx_ram_driver_io_error_request =  0;
#endif
    /* Check for the I/O error.  */
    if (status != FX_IO_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(73);
    }

    /* Finallay, abort the media.  */
    fx_media_abort(&ram_disk);


    /* Format the media in FAT16 for testing I/O error in media flush.  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            7000,                   // Total sectors  - FAT16
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(68);
    }

    /* Open the media again to setup for a media close I/O error.  */    
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {
   
        printf("ERROR!\n");
        test_control_return(69);
    }

    /* Create a file.  */    
    status =  fx_file_create(&ram_disk, "TEST.TXT");

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(70);
    }

    /* Open the test files.  */
    status =  fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);
    
    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(71);
    }
    
    /* Write to the file.  */
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_file_write(&my_file, "1234567890", 10);
    status +=  fx_media_read(&ram_disk, 2000, raw_sector_buffer);
    status +=  fx_media_write(&ram_disk, 2000, raw_sector_buffer);
    
    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(72);
    }    
    
#ifndef FX_DISABLE_CACHE
#ifdef FX_FAULT_TOLERANT
    /* While FX__FAULT_TOLERANT is defined, non data sector will flush directly in _fx_utility_logical_sector_write rather than set dirty flag. */
    /* For this reason, driver won't be called in _fx_utility_logical_sector_flush which is different from non FAULT_TOLERANT code. */
    _fx_utility_logical_sector_flush_error_request = 1;    
    status =  fx_media_close(&ram_disk);
#else
    /* Now attemp to flush the media, but with an I/O error introduced so the close will fail trying to flush logical sectors out.  */
    _fx_ram_driver_io_error_request =  17;
    status =  fx_media_flush(&ram_disk);
    _fx_ram_driver_io_error_request =  0;
#endif
    
    /* Check for the I/O error.  */
    if (status != FX_IO_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(73);
    }
#endif

    /* Finallay, abort the media.  */
    fx_media_abort(&ram_disk);
    
    /* Now see if we can test the FAT32 format with I/O corner cases.  */
    _fx_ram_driver_io_error_request =  2211;
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            70657,                  // Total sectors  - FAT32
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    _fx_ram_driver_io_error_request =  0;

    /* Determine if the format had an error.  */
    if (status != FX_IO_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(74);
    }



    printf("SUCCESS!\n");
    test_control_return(0);
}

