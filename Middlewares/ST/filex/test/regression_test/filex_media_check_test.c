/* This FileX test concentrates on the basic media check operation.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_utility.h"
#include   "fx_ram_driver_test.h"
#include   <stdio.h>

#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              16*128
#define     SCRATCH_MEMORY_SIZE     11000


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD                ftest_0;
#endif
static FX_MEDIA                 ram_disk;
static FX_FILE                  my_file;

static UCHAR                    raw_sector_buffer[512];
static UCHAR                    raw_sector_buffer_check[512];


/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *ram_disk_memory;
static UCHAR                  *cache_buffer;
static UCHAR                  *scratch_memory;
#else
static UCHAR                   cache_buffer[CACHE_SIZE];
static UCHAR                   scratch_memory[SCRATCH_MEMORY_SIZE];
#endif


/* Define thread prototypes.  */

void    filex_media_check_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_media_check_application_define(void *first_unused_memory)
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
    pointer =  pointer + (256*128);
    scratch_memory =  pointer;

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
ULONG       errors_detected;
UINT        i, j;
// A file name whose first byte is 0xe5.
UCHAR       specified_ascii_name[] = { 0xe5, 'a', 'b', 'c', 0};

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Media check test.......................................");

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
    return_if_fail( status == FX_SUCCESS);
    
    /* Try to check the media before the media has been opened */
    status =  fx_media_check(&ram_disk, scratch_memory, SCRATCH_MEMORY_SIZE, 0, &errors_detected);
    return_if_fail( status == FX_MEDIA_NOT_OPEN);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);
    
/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING

    /* send null pointer to generate an error */
    status = fx_media_check(FX_NULL, scratch_memory, SCRATCH_MEMORY_SIZE, 0, &errors_detected);
    return_if_fail( status == FX_PTR_ERROR);

#endif /* FX_DISABLE_ERROR_CHECKING */

    /* Check the media for errors.  */
    status =  fx_media_check(&ram_disk, scratch_memory, SCRATCH_MEMORY_SIZE, 0, &errors_detected);
    return_if_fail( (status == FX_SUCCESS) && (errors_detected == 0));

    /* Close the media and reopen.  */
    status =  fx_media_close(&ram_disk);
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);
    
    /* Invalidate the cache.  */
    status =  fx_media_cache_invalidate(&ram_disk);
    
    /* Read the first FAT sector.  */
    status += fx_media_read(&ram_disk, 1, (VOID *) raw_sector_buffer);  

    /* Check the status.  */
    return_if_fail((status == FX_SUCCESS) && 
        (raw_sector_buffer[0] == 0xF8) &&    /* _fx_media_format_media_type value set during media format */
        (raw_sector_buffer[1] == 0xFF) &&
        (raw_sector_buffer[2] == 0xFF));

    /* Change the FAT table to introduce lost clusters!  */
    raw_sector_buffer[30] =  2;
    raw_sector_buffer[31] =  3;
    raw_sector_buffer[32] =  4;
    
    /* Write the FAT sector back...  with the errors.  */
    status =  fx_media_write(&ram_disk, 1, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);
    
    /* Attempt to check the media but give it a bad scratch_memory_size to throw an error */
    status = fx_media_check(&ram_disk, scratch_memory, 0, 0, &errors_detected);
    return_if_fail( status == FX_NOT_ENOUGH_MEMORY);
    
    /* Attempt to check the media but give it a bad scratch_memory_size to throw an error */
    status = fx_media_check(&ram_disk, scratch_memory, (ram_disk.fx_media_total_clusters / 8), 0, &errors_detected);
    return_if_fail( status == FX_NOT_ENOUGH_MEMORY);

    /* Attempt to check the media but give it another bad scratch_memory_size to throw an error */
    status = fx_media_check(&ram_disk, scratch_memory, 977, 0, &errors_detected);
    return_if_fail( status == FX_NOT_ENOUGH_MEMORY);
    
    /* Check the media for errors.  */
    status =  fx_media_check(&ram_disk, scratch_memory, SCRATCH_MEMORY_SIZE, 0, &errors_detected);
    return_if_fail( (status == FX_SUCCESS) && (errors_detected == FX_LOST_CLUSTER_ERROR));
    
    /* Check the media for errors again, but correct them this time!  */
    status =  fx_media_check(&ram_disk, scratch_memory, SCRATCH_MEMORY_SIZE, FX_LOST_CLUSTER_ERROR, &errors_detected);
    return_if_fail( (status == FX_SUCCESS) && (errors_detected == FX_LOST_CLUSTER_ERROR));

    /* Check the media for errors again, but this time it should be successful.  */
    status =  fx_media_check(&ram_disk, scratch_memory, SCRATCH_MEMORY_SIZE, FX_LOST_CLUSTER_ERROR, &errors_detected);

    /* Determine if any were found - should not have found any errors at this point.  */
    return_if_fail( (status == FX_SUCCESS) && (errors_detected == 0));

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Create a file in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEXT.TXT");

    // Create the file whose name's first byte is 0xe5.
    status += fx_file_create(&ram_disk, (CHAR *)specified_ascii_name);

    status += fx_file_create(&ram_disk, "A somewhat long file name.txt");
    status += fx_file_open(&ram_disk, &my_file, "TEXT.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, raw_sector_buffer, 128);
    status += fx_file_write(&my_file, raw_sector_buffer, 128);
    status += fx_file_write(&my_file, raw_sector_buffer, 128);
    status +=  fx_media_check(&ram_disk, scratch_memory, SCRATCH_MEMORY_SIZE, 0, &errors_detected);    /* try to check the media while a file is open */
    status += fx_file_close(&my_file);
    status += fx_media_close(&ram_disk);
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
   
    /* Loop to clear the raw sector buffer... */
    for (i = 0; i < sizeof(raw_sector_buffer); i++)
    {
    
        raw_sector_buffer[i] =  0;
    } 

    /* Setup original FAT table to break the File's cluster!  */
    raw_sector_buffer[0] =  0x0F;
    raw_sector_buffer[1] =  0x0f;
    raw_sector_buffer[2] =  0xFF;
    
    /* Write the FAT sector back...  with the errors.  */
    status += fx_media_write(&ram_disk, 1, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_ACCESS_ERROR);
    
    /* Check the media for errors.  */
    status =  fx_media_check(&ram_disk, scratch_memory, SCRATCH_MEMORY_SIZE, 0, &errors_detected);

    /* Determine if any were found - should not have found any errors at this point.  */
    return_if_fail( (status == FX_SUCCESS) && (errors_detected == (FX_FAT_CHAIN_ERROR | FX_DIRECTORY_ERROR)));

    /* Check the media for errors...  and correct them this time!  */
    status =  fx_media_check(&ram_disk, scratch_memory, SCRATCH_MEMORY_SIZE, (FX_FAT_CHAIN_ERROR | FX_DIRECTORY_ERROR), &errors_detected);

    /* Determine if any were found - should not have found any errors at this point.  */
    return_if_fail( (status == FX_SUCCESS) && (errors_detected == (FX_FAT_CHAIN_ERROR | FX_DIRECTORY_ERROR)));

    /* Check the media for errors...  there should be none this time!  */
    status =  fx_media_check(&ram_disk, scratch_memory, SCRATCH_MEMORY_SIZE, (FX_FAT_CHAIN_ERROR | FX_DIRECTORY_ERROR), &errors_detected);

    /* Determine if any were found - should not have found any errors at this point.  */
    return_if_fail( (status == FX_SUCCESS) && (errors_detected == 0));

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_file_create(&ram_disk, "TEXT.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEXT.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, raw_sector_buffer, 128);
    status += fx_file_write(&my_file, raw_sector_buffer, 128);
    status += fx_file_write(&my_file, raw_sector_buffer, 128);
    status += fx_file_close(&my_file);
    status += fx_media_close(&ram_disk);
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Read the root directory's first sector.  */
    status += fx_media_read(&ram_disk, ram_disk.fx_media_root_sector_start, (VOID *) raw_sector_buffer);

    /* Set the size to 1 to generate a file size error.    */
    raw_sector_buffer[0x1C] =  1;
    raw_sector_buffer[0x1D] =  0;
    raw_sector_buffer[0x1E] =  0;
    raw_sector_buffer[0x1F] =  0;
    
    /* Write the root directory sector back...  with the errors.  */
    status += fx_media_write(&ram_disk, ram_disk.fx_media_root_sector_start, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Check the media for errors.  */
    status =  fx_media_check(&ram_disk, scratch_memory, SCRATCH_MEMORY_SIZE, 0, &errors_detected);

    /* Determine if any were found - should not have found any errors at this point.  */
    return_if_fail( (status == FX_SUCCESS) && (errors_detected == FX_FILE_SIZE_ERROR));

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);
    
    /* Clear the initial part of the RAM disk memory.  */
    for (i = 0; i < 4096; i++)
    {
        ram_disk_memory[i] =  0;
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
    return_if_fail( status == FX_SUCCESS);
    
    /* Setup a new, more complicated directory structure.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_file_create(&ram_disk, "ROOTF1.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF2.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_directory_create(&ram_disk, "\\SUB1");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "\\SUB1\\SUB1F1.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "\\SUB1\\SUB1F2.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "\\SUB1\\SUB2\\SUB2F1.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "\\SUB1\\SUB2\\SUB2F2.TXT");
    return_if_fail( status == FX_SUCCESS);
    
    /* Write data to the files...  */
    status += fx_file_open(&ram_disk, &my_file, "ROOTF1.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, "ROOT1_CONTENTS", 14);
    status += fx_file_close(&my_file);
    status += fx_media_flush(&ram_disk);

    status += fx_file_open(&ram_disk, &my_file, "ROOTF2.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, "ROOT2_CONTENTS", 14);
    status += fx_file_close(&my_file);
    status += fx_media_flush(&ram_disk);

    status += fx_file_open(&ram_disk, &my_file, "\\SUB1\\SUB1F1.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, "SUB1F1_CONTENTS", 15);
    status += fx_file_close(&my_file);
    status += fx_media_flush(&ram_disk);
        
    status += fx_file_open(&ram_disk, &my_file, "\\SUB1\\SUB1F2.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, "SUB1F2_CONTENTS", 15);
    status += fx_file_close(&my_file);
    status += fx_media_flush(&ram_disk);

    status += fx_file_open(&ram_disk, &my_file, "\\SUB1\\SUB2\\SUB2F1.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, "SUB2F1_CONTENTS", 15);
    status += fx_file_close(&my_file);
    status += fx_media_flush(&ram_disk);

    status += fx_file_open(&ram_disk, &my_file, "\\SUB1\\SUB2\\SUB2F2.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, "SUB2F2_CONTENTS", 15);
    status += fx_file_close(&my_file);
    status += fx_media_flush(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Check the media for errors.  */
    status =  fx_media_check(&ram_disk, scratch_memory, SCRATCH_MEMORY_SIZE, 0, &errors_detected);
    return_if_fail( status == FX_SUCCESS);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Clear the initial part of the RAM disk memory.  */
    for (i = 0; i < 4096; i++)
    {
        ram_disk_memory[i] =  0;
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
                            70000,                  // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
    /* Setup a new, more complicated directory structure.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_file_create(&ram_disk, "ROOTF1.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF2.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_directory_create(&ram_disk, "\\SUB1");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "\\SUB1\\SUB1F1.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "\\SUB1\\SUB1F2.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "\\SUB1\\SUB2\\SUB2F1.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "\\SUB1\\SUB2\\SUB2F2.TXT");
    return_if_fail( status == FX_SUCCESS);
    
    /* Write data to the files...  */
    status += fx_file_open(&ram_disk, &my_file, "ROOTF1.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, "ROOT1_CONTENTS", 14);
    status += fx_file_close(&my_file);
    status += fx_media_flush(&ram_disk);

    status += fx_file_open(&ram_disk, &my_file, "ROOTF2.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, "ROOT2_CONTENTS", 14);
    status += fx_file_close(&my_file);
    status += fx_media_flush(&ram_disk);

    status += fx_file_open(&ram_disk, &my_file, "\\SUB1\\SUB1F1.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, "SUB1F1_CONTENTS", 15);
    status += fx_file_close(&my_file);
    status += fx_media_flush(&ram_disk);
        
    status += fx_file_open(&ram_disk, &my_file, "\\SUB1\\SUB1F2.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, "SUB1F2_CONTENTS", 15);
    status += fx_file_close(&my_file);
    status += fx_media_flush(&ram_disk);

    status += fx_file_open(&ram_disk, &my_file, "\\SUB1\\SUB2\\SUB2F1.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, "SUB2F1_CONTENTS", 15);
    status += fx_file_close(&my_file);
    status += fx_media_flush(&ram_disk);

    status += fx_file_open(&ram_disk, &my_file, "\\SUB1\\SUB2\\SUB2F2.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, "SUB2F2_CONTENTS", 15);
    status += fx_file_close(&my_file);
    status += fx_media_flush(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Check the media for errors.  */
    status =  fx_media_check(&ram_disk, scratch_memory, 11000, 0, &errors_detected);
    return_if_fail( status == FX_SUCCESS);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Test for two files with the same FAT chain.  */

    /* Clear the initial part of the RAM disk memory.  */
    for (i = 0; i < 4096; i++)
    {
        ram_disk_memory[i] =  0;
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
                            6000,                   // Total sectors  - FAT16
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
    /* Setup a new, more complicated directory structure.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_file_create(&ram_disk, "ROOTF1.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF2.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF3.TXT");
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
    return_if_fail( status == FX_SUCCESS);

    /* Read the root directory's first sector.  */
    status += fx_media_read(&ram_disk, ram_disk.fx_media_root_sector_start, (VOID *) raw_sector_buffer);

    /* Set the second file's FAT chain to the same as the first.    */
    raw_sector_buffer[0x3A] =  raw_sector_buffer[0x1A];
    raw_sector_buffer[0x3B] =  raw_sector_buffer[0x1B];
    
    /* Write the root directory sector back...  with the errors.  */
    status += fx_media_write(&ram_disk, ram_disk.fx_media_root_sector_start, (VOID *) raw_sector_buffer);  
    status += fx_media_flush(&ram_disk);
    
    /* Read the first FAT sector.  */
    status += fx_media_read(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);

    /* Break the FAT chain of the ROOTF1.TXT file.    */
    raw_sector_buffer[14] =  0;
    raw_sector_buffer[15] =  0;
    
    /* Write the root directory sector back...  with the errors.  */
    status += fx_media_write(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);  
    status += fx_media_flush(&ram_disk);   
    return_if_fail( status == FX_SUCCESS);

    /* Check the media for errors.  */
    status =  fx_media_check(&ram_disk, scratch_memory, 4000, (FX_FAT_CHAIN_ERROR | FX_DIRECTORY_ERROR | FX_LOST_CLUSTER_ERROR), &errors_detected);
    return_if_fail((status == FX_SUCCESS) && (errors_detected == (FX_FAT_CHAIN_ERROR | FX_DIRECTORY_ERROR | FX_LOST_CLUSTER_ERROR)));

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Test I/O errors in the two files with the same FAT chain test.... */

    /* Clear the initial part of the RAM disk memory.  */
    for (i = 0; i < 4096; i++)
    {
        ram_disk_memory[i] =  0;
    }

    /* Loop to run through multiple tests...  */
    for (i = 0; i < 1000; i++)
    {    
    
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
        return_if_fail( status == FX_SUCCESS);
    
        /* Setup a new, more complicated directory structure.  */
        status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128 /*CACHE_SIZE*/);
        status += fx_file_create(&ram_disk, "ROOTF1.TXT");
        status += fx_file_create(&ram_disk, "ROOTF2.TXT");
        status += fx_file_create(&ram_disk, "ROOTF3.TXT");
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

        /* Create some more files and sub-directories to give the media check some more work to do.  */
        status += fx_file_create(&ram_disk, "ROOTF4.TXT");
        status += fx_file_create(&ram_disk, "ROOTF5.TXT");
        status += fx_file_create(&ram_disk, "ROOTF6.TXT");
        status += fx_file_create(&ram_disk, "ROOTF7.TXT");
        status += fx_file_create(&ram_disk, "ROOTF8.TXT");
        status += fx_file_create(&ram_disk, "ROOTF9.TXT");
        status += fx_file_create(&ram_disk, "ROOTF10.TXT");
        status += fx_file_create(&ram_disk, "ROOTF11.TXT");
        status += fx_file_create(&ram_disk, "ROOTF12.TXT");
        status += fx_file_create(&ram_disk, "ROOTF13.TXT");
        status += fx_file_create(&ram_disk, "ROOTF14.TXT");
        status += fx_file_create(&ram_disk, "ROOTF15.TXT");
        status += fx_file_create(&ram_disk, "ROOTF16.TXT");
        status += fx_file_create(&ram_disk, "ROOTF17.TXT");
        status += fx_file_create(&ram_disk, "ROOTF18.TXT");
        status += fx_file_create(&ram_disk, "ROOTF19.TXT");
        status += fx_file_create(&ram_disk, "ROOTF20.TXT");
        status += fx_directory_create(&ram_disk, "SUB4");
        status += fx_directory_create(&ram_disk, "SUB5");
        status += fx_directory_create(&ram_disk, "SUB6");
        status += fx_directory_create(&ram_disk, "SUB7");
        status += fx_directory_create(&ram_disk, "SUB8");
        status += fx_directory_create(&ram_disk, "SUB9");
        status += fx_directory_create(&ram_disk, "SUB10");
        status += fx_directory_create(&ram_disk, "SUB11");
        status += fx_directory_create(&ram_disk, "SUB12");
        status += fx_directory_create(&ram_disk, "SUB13");
        status += fx_file_create(&ram_disk, "\\SUB4\\ROOTF14.TXT");
        status += fx_file_create(&ram_disk, "\\SUB4\\ROOTF15.TXT");
        status += fx_file_create(&ram_disk, "\\SUB4\\ROOTF16.TXT");
        status += fx_file_create(&ram_disk, "\\SUB4\\ROOTF17.TXT");
        status += fx_file_create(&ram_disk, "\\SUB4\\ROOTF18.TXT");
        status += fx_file_create(&ram_disk, "\\SUB4\\ROOTF19.TXT");
        status += fx_file_create(&ram_disk, "\\SUB4\\ROOTF20.TXT");

        status += fx_media_flush(&ram_disk);
        return_if_fail( status == FX_SUCCESS);

        /* Read the root directory's first sector.  */
        status += fx_media_read(&ram_disk, ram_disk.fx_media_root_sector_start, (VOID *) raw_sector_buffer);

        /* Set the second file's FAT chain starting cluster to the same as the first.    */
        raw_sector_buffer[0x3A] =  raw_sector_buffer[0x1A];
        raw_sector_buffer[0x3B] =  raw_sector_buffer[0x1B];
    
        /* Write the root directory sector back...  with the errors.  */
        status += fx_media_write(&ram_disk, ram_disk.fx_media_root_sector_start, (VOID *) raw_sector_buffer);  
        status += fx_media_flush(&ram_disk);
    
        /* Read the first FAT sector.  */
        status += fx_media_read(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);

        /* Break the FAT chain of the ROOTF3.TXT file.    */
        if (i & 1)
        {
            /* Break with bad FAT value. */
            raw_sector_buffer[14] =  0;
            raw_sector_buffer[15] =  0;
        }
        else
        {
        
            /* Break with reserved fAT value.  */
            raw_sector_buffer[14] =  0xF0;
            raw_sector_buffer[15] =  0xFF;
        }
    
        /* Write the root directory sector back...  with the errors.  */
        status += fx_media_write(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);  
        status += fx_media_flush(&ram_disk);   
        return_if_fail( status == FX_SUCCESS);

        /* Create I/O errors via the RAM driver interface.  */
        _fx_utility_FAT_flush(&ram_disk);
        _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
        j = (UINT)(rand() % 20);
        if (j == 0)
           j =  1;        
        _fx_ram_driver_io_error_request =  j;
    
        /* Check the media for errors.  */
        fx_media_check(&ram_disk, scratch_memory, 4000, (((ULONG) i) % 16), &errors_detected);

        /* Clear the error generation flags.  */   
        _fx_ram_driver_io_error_request =  0;
   
        /* Abort the media.  */
        fx_media_abort(&ram_disk);
    }
    
    /* FAT32 I/O error check.  */

    /* Loop to run through multiple tests...  */
    for (i = 0; i < 1000; i++)
    {    
    
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
        return_if_fail( status == FX_SUCCESS);
    
        /* Setup a new, more complicated directory structure.  */
        status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128 /*CACHE_SIZE*/);
        status += fx_file_create(&ram_disk, "ROOTF1.TXT");
        status += fx_media_flush(&ram_disk);
        status += fx_file_create(&ram_disk, "ROOTF2.TXT");
        status += fx_media_flush(&ram_disk);
        status += fx_file_create(&ram_disk, "ROOTF3.TXT");
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

        /* Create some more files and sub-directories to give the media check some more work to do.  */
        status += fx_file_create(&ram_disk, "ROOTF4.TXT");
        status += fx_file_create(&ram_disk, "ROOTF5.TXT");
        status += fx_file_create(&ram_disk, "ROOTF6.TXT");
        status += fx_file_create(&ram_disk, "ROOTF7.TXT");
        status += fx_file_create(&ram_disk, "ROOTF8.TXT");
        status += fx_file_create(&ram_disk, "ROOTF9.TXT");
        status += fx_file_create(&ram_disk, "ROOTF10.TXT");
        status += fx_file_create(&ram_disk, "ROOTF11.TXT");
        status += fx_file_create(&ram_disk, "ROOTF12.TXT");
        status += fx_file_create(&ram_disk, "ROOTF13.TXT");
        status += fx_file_create(&ram_disk, "ROOTF14.TXT");
        status += fx_file_create(&ram_disk, "ROOTF15.TXT");
        status += fx_file_create(&ram_disk, "ROOTF16.TXT");
        status += fx_file_create(&ram_disk, "ROOTF17.TXT");
        status += fx_file_create(&ram_disk, "ROOTF18.TXT");
        status += fx_file_create(&ram_disk, "ROOTF19.TXT");
        status += fx_file_create(&ram_disk, "ROOTF20.TXT");
        status += fx_directory_create(&ram_disk, "SUB4");
        status += fx_directory_create(&ram_disk, "SUB5");
        status += fx_directory_create(&ram_disk, "SUB6");
        status += fx_directory_create(&ram_disk, "SUB7");
        status += fx_directory_create(&ram_disk, "SUB8");
        status += fx_directory_create(&ram_disk, "SUB9");
        status += fx_directory_create(&ram_disk, "SUB10");
        status += fx_directory_create(&ram_disk, "SUB11");
        status += fx_directory_create(&ram_disk, "SUB12");
        status += fx_file_create(&ram_disk, "\\SUB4\\ROOTF14.TXT");
        status += fx_file_create(&ram_disk, "\\SUB4\\ROOTF15.TXT");
        status += fx_file_create(&ram_disk, "\\SUB4\\ROOTF16.TXT");
        status += fx_file_create(&ram_disk, "\\SUB4\\ROOTF17.TXT");
        status += fx_file_create(&ram_disk, "\\SUB4\\ROOTF18.TXT");
        status += fx_file_create(&ram_disk, "\\SUB4\\ROOTF19.TXT");
        status += fx_file_create(&ram_disk, "\\SUB4\\ROOTF20.TXT");

        status += fx_media_flush(&ram_disk);
        return_if_fail( status == FX_SUCCESS);

        /* Read the root directory's first sector.  */
        status += fx_media_read(&ram_disk, ram_disk.fx_media_root_sector_start, (VOID *) raw_sector_buffer);

        /* Set the second file's FAT chain starting cluster to the same as the first.    */
        raw_sector_buffer[0x3A] =  raw_sector_buffer[0x1A];
        raw_sector_buffer[0x3B] =  raw_sector_buffer[0x1B];
    
        /* Write the root directory sector back...  with the errors.  */
        status += fx_media_write(&ram_disk, ram_disk.fx_media_root_sector_start, (VOID *) raw_sector_buffer);  
        status += fx_media_flush(&ram_disk);
    
        /* Read the first FAT sector.  */
        status += fx_media_read(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);

        /* Break the FAT chain of the ROOTF3.TXT file.    */
        if (i & 1)
        {
            /* Break with NULL FAT value.  */
            raw_sector_buffer[32] =  0;
            raw_sector_buffer[33] =  0;
            raw_sector_buffer[34] =  0;
            raw_sector_buffer[35] =  0;
        }
        else
        {
            /* Break with Reserved FAT value.  */
            raw_sector_buffer[32] =  0xF0;
            raw_sector_buffer[33] =  0xFF;
            raw_sector_buffer[34] =  0xFF;
            raw_sector_buffer[35] =  0xFF;
        }
            
        /* Write the root directory sector back...  with the errors.  */
        status += fx_media_write(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);  
        status += fx_media_flush(&ram_disk);   
        return_if_fail( status == FX_SUCCESS);

        /* Create I/O errors via the RAM driver interface.  */
        _fx_utility_FAT_flush(&ram_disk);
        _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
        j = (UINT)(rand() % 10);
        if (j == 0)
           j =  1;        
        _fx_ram_driver_io_error_request =  j;

        /* Check the media for errors.  */
        fx_media_check(&ram_disk, scratch_memory, 4000, (((ULONG) i) % 16), &errors_detected);

        /* Clear the error generation flags.  */   
        _fx_ram_driver_io_error_request =  0;

        /* Abort the media.  */
        fx_media_abort(&ram_disk);
    }     

    /* FAT32 Test for two files with the same FAT chain.  */

    /* Clear the initial part of the RAM disk memory.  */
    for (i = 0; i < 4096; i++)
    {
        ram_disk_memory[i] =  0;
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
                            70000,                  // Total sectors  - FAT32
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(33);
    }
    
    /* Setup a new, more complicated directory structure.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_file_create(&ram_disk, "ROOTF1.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF2.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF3.TXT");
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
    return_if_fail( status == FX_SUCCESS);

    /* Read the root directory's first sector.  */
    status += fx_media_read(&ram_disk, ram_disk.fx_media_root_sector_start, (VOID *) raw_sector_buffer);

    /* Set the second file's FAT chain to the same as the first.    */
    raw_sector_buffer[0x3A] =  raw_sector_buffer[0x1A];
    raw_sector_buffer[0x3B] =  raw_sector_buffer[0x1B];
    
    /* Write the root directory sector back...  with the errors.  */
    status += fx_media_write(&ram_disk, ram_disk.fx_media_root_sector_start, (VOID *) raw_sector_buffer);  
    status += fx_media_flush(&ram_disk);
    
    /* Read the first FAT sector.  */
    status += fx_media_read(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);

    /* Break the FAT chain of the ROOTF3.TXT file.    */
    raw_sector_buffer[32] =  0;
    raw_sector_buffer[33] =  0;
    raw_sector_buffer[34] =  0;
    raw_sector_buffer[35] =  0;
    
    /* Write the root directory sector back...  with the errors.  */
    status += fx_media_write(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);  
    status += fx_media_flush(&ram_disk);   
    return_if_fail( status == FX_SUCCESS);

    /* Check the media for errors.  */
    status =  fx_media_check(&ram_disk, scratch_memory, 11000, (FX_FAT_CHAIN_ERROR | FX_DIRECTORY_ERROR | FX_LOST_CLUSTER_ERROR), &errors_detected);

    /* Determine if any were found - should not have found any errors at this point.  */
    return_if_fail((status == FX_SUCCESS) && (errors_detected == (FX_FAT_CHAIN_ERROR | FX_DIRECTORY_ERROR | FX_LOST_CLUSTER_ERROR)));
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);
    
    /* FAT32 Test for two files with the same FAT chain and a broken root directory FAT chain.  */

    /* Clear the initial part of the RAM disk memory.  */
    for (i = 0; i < 4096; i++)
    {
        ram_disk_memory[i] =  0;
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
                            70000,                  // Total sectors  - FAT32
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
    /* Setup a new, more complicated directory structure.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_file_create(&ram_disk, "ROOTF1.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF2.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_create(&ram_disk, "ROOTF3.TXT");
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
    return_if_fail( status == FX_SUCCESS);

    /* Read the root directory's first sector.  */
    status += fx_media_read(&ram_disk, ram_disk.fx_media_root_sector_start, (VOID *) raw_sector_buffer);

    /* Set the second file's FAT chain to the same as the first.    */
    raw_sector_buffer[0x3A] =  raw_sector_buffer[0x1A];
    raw_sector_buffer[0x3B] =  raw_sector_buffer[0x1B];
    
    /* Write the root directory sector back...  with the errors.  */
    status += fx_media_write(&ram_disk, ram_disk.fx_media_root_sector_start, (VOID *) raw_sector_buffer);  
    status += fx_media_flush(&ram_disk);
    
    /* Read the first FAT sector.  */
    status += fx_media_read(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);

    /* Break the FAT chain of the root directory in FAT32. */
    raw_sector_buffer[8] =  0;
    raw_sector_buffer[9] =  0;
    raw_sector_buffer[10] = 0;
    raw_sector_buffer[11] = 0;

    /* Break the FAT chain of the ROOTF3.TXT file.    */
    raw_sector_buffer[32] =  0;
    raw_sector_buffer[33] =  0;
    raw_sector_buffer[34] =  0;
    raw_sector_buffer[35] =  0;    
    
    /* Write the root directory sector back...  with the errors.  */
    status += fx_media_write(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);  
    status += fx_media_flush(&ram_disk);   
    return_if_fail( status == FX_SUCCESS);

    /* Check the media for errors.  */
    status =  fx_media_check(&ram_disk, scratch_memory, 11000, (FX_FAT_CHAIN_ERROR | FX_DIRECTORY_ERROR | FX_LOST_CLUSTER_ERROR), &errors_detected);
    return_if_fail( status == FX_ERROR_NOT_FIXED);
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Test a full disk with a maximum cluster FAT chain.  */

    /* Clear the initial part of the RAM disk memory.  */
    for (i = 0; i < 4096; i++)
    {
        ram_disk_memory[i] =  0;
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
                            6000,                   // Total sectors  - FAT32
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
    /* Setup a new, more complicated directory structure.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_file_create(&ram_disk, "ROOTF1.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_open(&ram_disk, &my_file, "ROOTF1.TXT", FX_OPEN_FOR_WRITE);
    do 
    {
        status =  fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    } while (status == FX_SUCCESS);
    
    /* Determine if the write had anything other than a no more space error.  */
    return_if_fail( status == FX_NO_MORE_SPACE);

    status = fx_file_close(&my_file);
    status += fx_media_flush(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Reduce the amount of clusters just to exercise the branch in the FAT chain search.  */
    ram_disk.fx_media_total_clusters =  ram_disk.fx_media_total_clusters - 4;

    /* Check the media for errors.  */
    status =  fx_media_check(&ram_disk, scratch_memory, 11000, (FX_FAT_CHAIN_ERROR | FX_DIRECTORY_ERROR | FX_LOST_CLUSTER_ERROR), &errors_detected);

    /* Undo the cluster adjustment.   */
    ram_disk.fx_media_total_clusters =  ram_disk.fx_media_total_clusters + 4;
    return_if_fail( status == FX_SUCCESS);
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Test sub-directory corruption problems.   */

    /* Clear the initial part of the RAM disk memory.  */
    for (i = 0; i < 4096; i++)
    {
        ram_disk_memory[i] =  0;
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
                            6000,                   // Total sectors  - FAT16
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
    /* Setup a deep directory structure.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_directory_create(&ram_disk, "\\DIR1");
    status += fx_file_create(&ram_disk, "\\DIR1\\SUB1");
    status += fx_file_create(&ram_disk, "\\DIR1\\SUB2");
    status += fx_file_create(&ram_disk, "\\DIR1\\SUB3");
    status += fx_file_create(&ram_disk, "\\DIR1\\SUB4");
    status += fx_file_create(&ram_disk, "\\DIR1\\SUB5");
    status += fx_file_create(&ram_disk, "\\DIR1\\SUB6");
    status += fx_file_create(&ram_disk, "\\DIR1\\SUB7");
    status += fx_file_create(&ram_disk, "\\DIR1\\SUB8");
    status += fx_file_create(&ram_disk, "\\DIR1\\SUB9");
    status += fx_file_create(&ram_disk, "\\DIR1\\SUB10");
    status += fx_file_create(&ram_disk, "\\DIR1\\SUB11");
    status += fx_media_flush(&ram_disk);
    return_if_fail( status == FX_SUCCESS);
 
    /* Read the first FAT sector.  */
    status += fx_media_read(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);

    /* Break the FAT chain of the in the first sub directory. */
    raw_sector_buffer[10] =  0;
    raw_sector_buffer[11] =  0;
    
    /* Write the root directory sector back...  with the errors.  */
    status += fx_media_write(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);  
    status += fx_media_flush(&ram_disk);   
    return_if_fail( status == FX_SUCCESS);

    /* Check the media for errors.  */
    status =  fx_media_check(&ram_disk, scratch_memory, 4000, (FX_FAT_CHAIN_ERROR | FX_DIRECTORY_ERROR | FX_LOST_CLUSTER_ERROR), &errors_detected);

    /* Determine if any were found - should not have found any errors at this point.  */
    return_if_fail((status == FX_SUCCESS) && (errors_detected == FX_FAT_CHAIN_ERROR));

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* FAT32 Test for root directory FAT chain recovery.  */

    /* Clear the initial part of the RAM disk memory.  */
    for (i = 0; i < 4096; i++)
    {
        ram_disk_memory[i] =  0;
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
                            70000,                  // Total sectors  - FAT32
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
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
    return_if_fail( status == FX_SUCCESS);

    /* Read the first FAT sector.  */
    status += fx_media_read(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);

    /* Break the FAT chain of the root directory in FAT32. */
    raw_sector_buffer[12] =  0;
    raw_sector_buffer[13] =  0;
    raw_sector_buffer[14] = 0;
    raw_sector_buffer[15] = 0;
   
    /* Write the root directory sector back...  with the errors.  */
    status += fx_media_write(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);  
    status += fx_media_flush(&ram_disk);   
    return_if_fail( status == FX_SUCCESS);

    /* Check the media for errors.  */
    status =  fx_media_check(&ram_disk, scratch_memory, 11000, (FX_FAT_CHAIN_ERROR | FX_DIRECTORY_ERROR | FX_LOST_CLUSTER_ERROR), &errors_detected);
#if 0
    return_if_fail( status == FX_SECTOR_INVALID);
#else
    return_if_fail( status == FX_SUCCESS);
#endif
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Test a full disk with a FAT chain that has a bad pointer to itself.  */

    /* Clear the initial part of the RAM disk memory.  */
    for (i = 0; i < 4096; i++)
    {
        ram_disk_memory[i] =  0;
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
                            6000,                   // Total sectors  - FAT32
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
    /* Setup a new, more complicated directory structure.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_file_create(&ram_disk, "ROOTF1.TXT");
    status += fx_media_flush(&ram_disk);
    status += fx_file_open(&ram_disk, &my_file, "ROOTF1.TXT", FX_OPEN_FOR_WRITE);
    do 
    {
        status =  fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    } while (status == FX_SUCCESS);
    
    /* Determine if the write had anything other than a no more space error.  */
    return_if_fail( status == FX_NO_MORE_SPACE);

    status = fx_file_close(&my_file);
    status += fx_media_flush(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);

    /* Make the FAT chain recursive. */
    raw_sector_buffer[6] =  raw_sector_buffer[4];
    raw_sector_buffer[7] =  raw_sector_buffer[5];
            
    /* Write the root directory sector back...  with the errors.  */
    status += fx_media_write(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);  
    status += fx_media_flush(&ram_disk);   

    /* Check the media for errors.  */
    status +=  fx_media_check(&ram_disk, scratch_memory, 11000, (FX_FAT_CHAIN_ERROR | FX_DIRECTORY_ERROR | FX_LOST_CLUSTER_ERROR), &errors_detected);
    return_if_fail( status == FX_SUCCESS);
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Test a full disk with a sub-directory that has a bad FAT chain.  */

    /* Clear the initial part of the RAM disk memory.  */
    for (i = 0; i < 4096; i++)
    {
        ram_disk_memory[i] =  0;
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
                            6000,                   // Total sectors  - FAT32
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
    /* Setup a new, more complicated directory structure.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_directory_create(&ram_disk, "SUB1");
    status += fx_media_flush(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);

    /* Make the FAT chain recursive. */
    raw_sector_buffer[4] =  0;
    raw_sector_buffer[5] =  0;
            
    /* Write the FAT sector back...  with the errors.  */
    status += fx_media_write(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);  
    status += fx_media_flush(&ram_disk);   

    /* Read the root directory's first sector.  */
    status += fx_media_read(&ram_disk, ram_disk.fx_media_root_sector_start, (VOID *) raw_sector_buffer);

    /* Check the media for errors.  */
    status +=  fx_media_check(&ram_disk, scratch_memory, 11000, (FX_FAT_CHAIN_ERROR | FX_DIRECTORY_ERROR | FX_LOST_CLUSTER_ERROR), &errors_detected);
    return_if_fail( status == FX_SUCCESS);
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Test sub-directory depth that exceeds FX_MAX_DIRECTORY_NESTING (which is by default 20).  */

    /* Clear the initial part of the RAM disk memory.  */
    for (i = 0; i < 4096; i++)
    {
        ram_disk_memory[i] =  0;
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
                            6000,                   // Total sectors  - FAT32
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
    /* Setup a new, more complicated directory structure.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_directory_create(&ram_disk, "\\SUB1");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2\\SUB3");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2\\SUB3\\SUB4");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2\\SUB3\\SUB4\\SUB5");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2\\SUB3\\SUB4\\SUB5\\SUB6");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2\\SUB3\\SUB4\\SUB5\\SUB6\\SUB7");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2\\SUB3\\SUB4\\SUB5\\SUB6\\SUB7\\SUB8");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2\\SUB3\\SUB4\\SUB5\\SUB6\\SUB7\\SUB8\\SUB9");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2\\SUB3\\SUB4\\SUB5\\SUB6\\SUB7\\SUB8\\SUB9\\SUB10");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2\\SUB3\\SUB4\\SUB5\\SUB6\\SUB7\\SUB8\\SUB9\\SUB10\\SUB11");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2\\SUB3\\SUB4\\SUB5\\SUB6\\SUB7\\SUB8\\SUB9\\SUB10\\SUB11\\SUB12");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2\\SUB3\\SUB4\\SUB5\\SUB6\\SUB7\\SUB8\\SUB9\\SUB10\\SUB11\\SUB12\\SUB13");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2\\SUB3\\SUB4\\SUB5\\SUB6\\SUB7\\SUB8\\SUB9\\SUB10\\SUB11\\SUB12\\SUB13\\SUB14");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2\\SUB3\\SUB4\\SUB5\\SUB6\\SUB7\\SUB8\\SUB9\\SUB10\\SUB11\\SUB12\\SUB13\\SUB14\\SUB15");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2\\SUB3\\SUB4\\SUB5\\SUB6\\SUB7\\SUB8\\SUB9\\SUB10\\SUB11\\SUB12\\SUB13\\SUB14\\SUB15\\SUB16");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2\\SUB3\\SUB4\\SUB5\\SUB6\\SUB7\\SUB8\\SUB9\\SUB10\\SUB11\\SUB12\\SUB13\\SUB14\\SUB15\\SUB16\\SUB17");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2\\SUB3\\SUB4\\SUB5\\SUB6\\SUB7\\SUB8\\SUB9\\SUB10\\SUB11\\SUB12\\SUB13\\SUB14\\SUB15\\SUB16\\SUB17\\SUB18");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2\\SUB3\\SUB4\\SUB5\\SUB6\\SUB7\\SUB8\\SUB9\\SUB10\\SUB11\\SUB12\\SUB13\\SUB14\\SUB15\\SUB16\\SUB17\\SUB18\\SUB19");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2\\SUB3\\SUB4\\SUB5\\SUB6\\SUB7\\SUB8\\SUB9\\SUB10\\SUB11\\SUB12\\SUB13\\SUB14\\SUB15\\SUB16\\SUB17\\SUB18\\SUB19\\SUB20");
    status += fx_directory_create(&ram_disk, "\\SUB1\\SUB2\\SUB3\\SUB4\\SUB5\\SUB6\\SUB7\\SUB8\\SUB9\\SUB10\\SUB11\\SUB12\\SUB13\\SUB14\\SUB15\\SUB16\\SUB17\\SUB18\\SUB19\\SUB20\\SUB21");
    status += fx_media_flush(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Check the media for errors.  */
    status =  fx_media_check(&ram_disk, scratch_memory, 11000, (FX_FAT_CHAIN_ERROR | FX_DIRECTORY_ERROR | FX_LOST_CLUSTER_ERROR), &errors_detected);
    return_if_fail( status == FX_NOT_ENOUGH_MEMORY);
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Test a full disk with a sub-directory that has a bad FAT chain, but without directory correction selected.  */

    /* Clear the initial part of the RAM disk memory.  */
    for (i = 0; i < 4096; i++)
    {
        ram_disk_memory[i] =  0;
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
                            6000,                   // Total sectors  - FAT32
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
    /* Setup a new, more complicated directory structure.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    status += fx_directory_create(&ram_disk, "SUB1");
    status += fx_media_flush(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);

    /* Make the FAT chain recursive. */
    raw_sector_buffer[4] =  0;
    raw_sector_buffer[5] =  0;
            
    /* Write the FAT sector back...  with the errors.  */
    status += fx_media_write(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);  
    status += fx_media_flush(&ram_disk);   

    /* Read the root directory's first sector.  */
    status += fx_media_read(&ram_disk, ram_disk.fx_media_root_sector_start, (VOID *) raw_sector_buffer);

    /* Check the media for errors.  */
    status +=  fx_media_check(&ram_disk, scratch_memory, 11000, (FX_FAT_CHAIN_ERROR | FX_LOST_CLUSTER_ERROR), &errors_detected);
    return_if_fail( status == FX_SUCCESS);
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* FAT32 Test for sixteen files with a broken root directory FAT chain.  */

    /* Clear the initial part of the RAM disk memory.  */
    for (i = 0; i < 4096; i++)
    {
        ram_disk_memory[i] =  0;
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
                            70000,                  // Total sectors  - FAT32
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
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
    return_if_fail( status == FX_SUCCESS);

    /* Read the root directory's first sector.  */
    status += fx_media_read(&ram_disk, ram_disk.fx_media_root_sector_start, (VOID *) raw_sector_buffer);

    /* Set the second file's FAT chain to the same as the first.    */
    raw_sector_buffer[0x3A] =  raw_sector_buffer[0x1A];
    raw_sector_buffer[0x3B] =  raw_sector_buffer[0x1B];
    
    /* Write the root directory sector back...  with the errors.  */
    status += fx_media_write(&ram_disk, ram_disk.fx_media_root_sector_start, (VOID *) raw_sector_buffer);  
    status += fx_media_flush(&ram_disk);
    
    /* Read the first FAT sector.  */
    status += fx_media_read(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);

    /* Break the FAT chain of the root directory in FAT32. */
    raw_sector_buffer[12] =  0;
    raw_sector_buffer[13] =  0;
    raw_sector_buffer[14] = 0;
    raw_sector_buffer[15] = 0;
   
    /* Write the root directory sector back...  with the errors.  */
    status += fx_media_write(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);  
    status += fx_media_flush(&ram_disk);   
    return_if_fail( status == FX_SUCCESS);

    /* Check the media for errors.  */
    status =  fx_media_check(&ram_disk, scratch_memory, 11000, (FX_DIRECTORY_ERROR | FX_LOST_CLUSTER_ERROR), &errors_detected);
    return_if_fail( status == FX_ERROR_NOT_FIXED);
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

#ifndef FX_DISABLE_ERROR_CHECKING

    /* Test NULL media pointer error checking.  */
    status =  fx_media_check(FX_NULL, scratch_memory, 11000, (FX_DIRECTORY_ERROR | FX_LOST_CLUSTER_ERROR), &errors_detected);
    return_if_fail( status == FX_PTR_ERROR);

    /* Test NULL media pointer error checking.  */
    status =  fx_media_check(&ram_disk, FX_NULL, 11000, (FX_DIRECTORY_ERROR | FX_LOST_CLUSTER_ERROR), &errors_detected);
    return_if_fail( status == FX_PTR_ERROR);

#endif //FX_DISABLE_ERROR_CHECKING

    /* FAT32 Test for sixteen files with a broken root directory FAT chain and 512 byte sectors.  */

    /* Clear the initial part of the RAM disk memory.  */
    for (i = 0; i < 4096; i++)
    {
        ram_disk_memory[i] =  0;
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
                            70000,                  // Total sectors  - FAT32
                            512,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
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
    status += fx_file_create(&ram_disk, "ROOTF17.TXT");         /* Ensure the root directory exceeds the 512 byte sector size and thus has a cluster chain!  */
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
    return_if_fail( status == FX_SUCCESS);

    /* Read the root directory's first sector.  */
    status += fx_media_read(&ram_disk, ram_disk.fx_media_root_sector_start, (VOID *) raw_sector_buffer);

    /* Set the second file's FAT chain to the same as the first.    */
    raw_sector_buffer[0x3A] =  raw_sector_buffer[0x1A];
    raw_sector_buffer[0x3B] =  raw_sector_buffer[0x1B];
    
    /* Write the root directory sector back...  with the errors.  */
    status += fx_media_write(&ram_disk, ram_disk.fx_media_root_sector_start, (VOID *) raw_sector_buffer);  
    status += fx_media_flush(&ram_disk);
    
    /* Read the first FAT sector.  */
    status += fx_media_read(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);

    /* Break the FAT chain of the root directory in FAT32. */
    raw_sector_buffer[12] =  0;
    raw_sector_buffer[13] =  0;
    raw_sector_buffer[14] = 0;
    raw_sector_buffer[15] = 0;
   
    /* Write the root directory sector back...  with the errors.  */
    status += fx_media_write(&ram_disk, ram_disk.fx_media_reserved_sectors, (VOID *) raw_sector_buffer);  
    status += fx_media_flush(&ram_disk);   
    return_if_fail( status == FX_SUCCESS);

    /* Check the media for errors.  */
    status =  fx_media_check(&ram_disk, scratch_memory, 11000, (FX_DIRECTORY_ERROR | FX_LOST_CLUSTER_ERROR), &errors_detected);
    return_if_fail( status == FX_ERROR_NOT_FIXED);
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

#ifndef FX_DISABLE_ERROR_CHECKING

    /* Test NULL media pointer error checking.  */
    status =  fx_media_check(FX_NULL, scratch_memory, 11000, (FX_DIRECTORY_ERROR | FX_LOST_CLUSTER_ERROR), &errors_detected);
    return_if_fail( status == FX_PTR_ERROR);

    /* Test NULL media pointer error checking.  */
    status =  fx_media_check(&ram_disk, FX_NULL, 11000, (FX_DIRECTORY_ERROR | FX_LOST_CLUSTER_ERROR), &errors_detected);
    return_if_fail( status == FX_PTR_ERROR);

#endif //FX_DISABLE_ERROR_CHECKING

    /* FAT32 Test for formatting with I/O errors.  */

    /* Clear the initial part of the RAM disk memory.  */
    for (i = 0; i < 4096; i++)
    {
        ram_disk_memory[i] =  0;
    }
    for (i = 0; i < 4000; i++)
    {

        /* Create I/O errors via the RAM driver interface.  */
        _fx_utility_FAT_flush(&ram_disk);
        _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
        j = (UINT)(rand() % 10);
        if (j == 0)
           j =  1;        
        _fx_ram_driver_io_error_request =  j;
    
        /* Format the media.  This needs to be done before opening it!  */
        fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MYDISK",               // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            70000,                  // Total sectors  - FAT32
                            512,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 


        _fx_utility_FAT_flush(&ram_disk);
        _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
        j = (UINT)(rand() % 10);
        if (j == 0)
           j =  1;        
        _fx_ram_driver_io_error_request =  j;
  
        /* Open media with errors.  */
        fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 512);

        _fx_utility_FAT_flush(&ram_disk);
        _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
        j = (UINT)(rand() % 10);
        if (j == 0)
           j =  1;        
        _fx_ram_driver_io_error_request =  j;

        /* Attempt to close media with errors.  */
        fx_media_close(&ram_disk);

        _fx_utility_FAT_flush(&ram_disk);

        _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
        j = (UINT)(rand() % 10);
        if (j == 0)
           j =  1;        
        _fx_ram_driver_io_error_request =  j;

        /* Attempt to abort media with errors.  */
        fx_media_abort(&ram_disk);

        _fx_ram_driver_io_error_request =  0;
    }

    /* Test I/O errors in _fx_media_check_lost_cluster_check.   */
    
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
                            6000,                   // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_if_fail( status == FX_SUCCESS);
    
    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 2, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Change the FAT table to introduce lost clusters!  */
    raw_sector_buffer[30] =  2;
    raw_sector_buffer[31] =  3;
    raw_sector_buffer[32] =  4;
    
    /* Write the FAT sector back...  with the errors.  */
    status =  fx_media_write(&ram_disk, 2, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 3, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Change the FAT table to introduce lost clusters!  */
    raw_sector_buffer[30] =  5;
    raw_sector_buffer[31] =  6;
    raw_sector_buffer[32] =  7;
    
    /* Write the FAT sector back...  with the errors.  */
    status =  fx_media_write(&ram_disk, 3, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 4, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Change the FAT table to introduce lost clusters!  */
    raw_sector_buffer[30] =  8;
    raw_sector_buffer[31] =  9;
    raw_sector_buffer[32] =  10;
    
    /* Write the FAT sector back...  with the errors.  */
    status =  fx_media_write(&ram_disk, 4, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Flush and invalidate the cache.  */
    _fx_utility_logical_sector_flush(&ram_disk, 0, 4000, FX_TRUE);
    
    /* Force media errors.  */
    _fx_utility_fat_entry_read_error_request =  1;
    status =  fx_media_check(&ram_disk, scratch_memory, SCRATCH_MEMORY_SIZE, FX_LOST_CLUSTER_ERROR, &errors_detected);
    _fx_utility_fat_entry_read_error_request =  1000;
    fx_media_check(&ram_disk, scratch_memory, SCRATCH_MEMORY_SIZE, FX_LOST_CLUSTER_ERROR, &errors_detected);
    _fx_utility_fat_entry_read_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

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
                            6000,                   // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_if_fail( status == FX_SUCCESS);
    
    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 2, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Change the FAT table to introduce lost clusters!  */
    raw_sector_buffer[30] =  2;
    raw_sector_buffer[31] =  3;
    raw_sector_buffer[32] =  4;
    
    /* Write the FAT sector back...  with the errors.  */
    status =  fx_media_write(&ram_disk, 2, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 3, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Change the FAT table to introduce lost clusters!  */
    raw_sector_buffer[30] =  5;
    raw_sector_buffer[31] =  6;
    raw_sector_buffer[32] =  7;
    
    /* Write the FAT sector back...  with the errors.  */
    status =  fx_media_write(&ram_disk, 3, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 4, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Change the FAT table to introduce lost clusters!  */
    raw_sector_buffer[30] =  8;
    raw_sector_buffer[31] =  9;
    raw_sector_buffer[32] =  10;
    
    /* Write the FAT sector back...  with the errors.  */
    status =  fx_media_write(&ram_disk, 4, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Flush and invalidate the cache.  */
    _fx_utility_logical_sector_flush(&ram_disk, 0, 4000, FX_TRUE);
    
    /* Check the media for errors.  */
    _fx_utility_fat_entry_write_error_request =  1;
    status =  fx_media_check(&ram_disk, scratch_memory, SCRATCH_MEMORY_SIZE, FX_LOST_CLUSTER_ERROR, &errors_detected);
    _fx_utility_fat_entry_write_error_request =  10000;
    fx_media_check(&ram_disk, scratch_memory, SCRATCH_MEMORY_SIZE, FX_LOST_CLUSTER_ERROR, &errors_detected);
    _fx_utility_fat_entry_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Test the error paths in FAT32 root directory traversal.  */

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
                            70000,                  // Total sectors - FAT32
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_if_fail( status == FX_SUCCESS);
    
    /* Now create a series to sub-directories to expand the root directory FAT chain.  */
    status =  fx_file_create(&ram_disk, "FILE1");
    status += fx_file_create(&ram_disk, "FILE2");
    status += fx_file_create(&ram_disk, "FILE3");
    status += fx_file_create(&ram_disk, "FILE4");
    status += fx_file_create(&ram_disk, "FILE5");
    status += fx_file_create(&ram_disk, "FILE6");
    status += fx_file_create(&ram_disk, "FILE7");
    status += fx_file_create(&ram_disk, "FILE8");
    status += fx_file_create(&ram_disk, "FILE9");
    status += fx_file_create(&ram_disk, "FILE10");
    status += fx_file_create(&ram_disk, "FILE11");
    status += fx_file_create(&ram_disk, "FILE12");
    status += fx_file_create(&ram_disk, "FILE13");
    status += fx_file_create(&ram_disk, "FILE14");
    status += fx_file_create(&ram_disk, "FILE15");
    status += fx_file_create(&ram_disk, "FILE16");
    status += fx_file_create(&ram_disk, "FILE17");
    status += fx_file_create(&ram_disk, "FILE18");
    status += fx_file_create(&ram_disk, "FILE19");
    status += fx_file_create(&ram_disk, "FILE20");
    status += fx_file_create(&ram_disk, "FILE21");
    status += fx_file_create(&ram_disk, "FILE22");
    status += fx_file_create(&ram_disk, "FILE23");
    status += fx_file_create(&ram_disk, "FILE24");
    status += fx_file_create(&ram_disk, "FILE25");
    status += fx_file_create(&ram_disk, "FILE26");
    status += fx_file_create(&ram_disk, "FILE27");
    status += fx_file_create(&ram_disk, "FILE28");
    status += fx_file_create(&ram_disk, "FILE29");
    status += fx_file_create(&ram_disk, "FILE30");
    status += fx_file_create(&ram_disk, "FILE31"); 
    status += fx_media_flush(&ram_disk);

    
    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Break the FAT chain of the root directory!  */
    raw_sector_buffer[36] =  0;
    raw_sector_buffer[37] =  0;
    raw_sector_buffer[38] =  0;
    raw_sector_buffer[39] =  0;
    
    /* Write the FAT sector back...  with the errors.  */
    status =  fx_media_write(&ram_disk, 1, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Flush and invalidate the cache.  */
    fx_media_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 0, 4000, FX_TRUE);
    
    /* Check the media for errors.  */
    _fx_utility_fat_entry_read_error_request =  1;
    status =  fx_media_check(&ram_disk, scratch_memory, 11000, (FX_LOST_CLUSTER_ERROR | FX_DIRECTORY_ERROR | FX_FAT_CHAIN_ERROR), &errors_detected);
    _fx_utility_fat_entry_read_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);

    /* Check the media for errors.  */
    _fx_utility_fat_entry_write_error_request =  1;
    status =  fx_media_check(&ram_disk, scratch_memory, 11000, (FX_LOST_CLUSTER_ERROR | FX_DIRECTORY_ERROR | FX_FAT_CHAIN_ERROR), &errors_detected);
    _fx_utility_fat_entry_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Test the error paths in file FAT traversal.  */

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
                            70000,                  // Total sectors - FAT32
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_if_fail( status == FX_SUCCESS);
    
    /* Now create a series to sub-directories to expand the root directory FAT chain.  */
    status =  fx_file_create(&ram_disk, "FILE1");
    
    /* Flush and invalidate the cache.  */
    fx_media_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 0, 4000, FX_TRUE);

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);
    
    /* Open the file.  */
    status += fx_file_open(&ram_disk, &my_file, "FILE1", FX_OPEN_FOR_WRITE); 
    
    /* Write to the file.  */
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_write(&my_file, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status += fx_file_close(&my_file);
    status += fx_media_flush(&ram_disk);
    
    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Break the FAT chain of the root directory!  */
    raw_sector_buffer[36] =  0;
    raw_sector_buffer[37] =  0;
    raw_sector_buffer[38] =  0;
    raw_sector_buffer[39] =  0;
    
    /* Write the FAT sector back...  with the errors.  */
    status =  fx_media_write(&ram_disk, 1, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Flush and invalidate the cache.  */
    fx_media_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 0, 4000, FX_TRUE);
    
    /* Check the media for errors.  */
    _fx_utility_fat_entry_write_error_request =  1;
    status =  fx_media_check(&ram_disk, scratch_memory, 11000, (FX_LOST_CLUSTER_ERROR | FX_DIRECTORY_ERROR | FX_FAT_CHAIN_ERROR), &errors_detected);
    _fx_utility_fat_entry_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Test the error paths in sub-directory delete path.  */

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
                            70000,                  // Total sectors - FAT32
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_if_fail( status == FX_SUCCESS);
    
    /* Now create a sub-directory.  */
    status =  fx_directory_create(&ram_disk, "SUB1");
    
    /* Flush and invalidate the cache.  */
    fx_media_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 0, 4000, FX_TRUE);

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);
    
    /* Open the file.  */
    status += fx_file_open(&ram_disk, &my_file, "FILE1", FX_OPEN_FOR_WRITE); 
    status += fx_media_flush(&ram_disk);
    
    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Break the FAT chain of the sub-directory!  */
    raw_sector_buffer[12] =  0;
    raw_sector_buffer[13] =  0;
    raw_sector_buffer[14] =  0;
    raw_sector_buffer[15] =  0;
    
    /* Write the FAT sector back...  with the errors.  */
    status =  fx_media_write(&ram_disk, 1, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Flush and invalidate the cache.  */
    fx_media_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 0, 4000, FX_TRUE);
    
    /* Check the media for errors.  */
    _fx_ram_driver_io_error_request =  4;
    status =  fx_media_check(&ram_disk, scratch_memory, 11000, (FX_LOST_CLUSTER_ERROR | FX_DIRECTORY_ERROR | FX_FAT_CHAIN_ERROR), &errors_detected);
    _fx_ram_driver_io_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

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
    return_if_fail( status == FX_SUCCESS);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Create a file in the root directory.  */
    status =  fx_file_create(&ram_disk, "TEXT.TXT");
    status += fx_file_open(&ram_disk, &my_file, "TEXT.TXT", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&my_file, raw_sector_buffer, 128);
    status += fx_file_close(&my_file);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    status = fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);
    
    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Change the FAT table to introduce lost clusters!  */
    memcpy(raw_sector_buffer_check, raw_sector_buffer, ram_disk.fx_media_bytes_per_sector);
    raw_sector_buffer[3] = (ram_disk.fx_media_total_clusters + FX_FAT_ENTRY_START) & 0xFF;
    raw_sector_buffer[4] = (UCHAR)((ram_disk.fx_media_total_clusters + FX_FAT_ENTRY_START) >> 8);
    
    /* Write the FAT sector back...  with the errors.  */
    status =  fx_media_write(&ram_disk, 1, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);
    
    /* Attempt to check the media but give it a bad scratch_memory_size to throw an error */
    status =  fx_media_check(&ram_disk, scratch_memory, SCRATCH_MEMORY_SIZE, FX_FAT_CHAIN_ERROR,
                             &errors_detected);
    return_if_fail( (status == FX_SUCCESS) &&
                    (errors_detected == (FX_FAT_CHAIN_ERROR| FX_DIRECTORY_ERROR| FX_LOST_CLUSTER_ERROR)));
    
#if 0
    /* Read the first FAT sector again.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

    /* Compare the */
    return_if_fail( memcmp(raw_sector_buffer_check,
                           raw_sector_buffer,
                           ram_disk.fx_media_bytes_per_sector) == 0);
#endif

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    printf("SUCCESS!\n");
    test_control_return(0);
}

