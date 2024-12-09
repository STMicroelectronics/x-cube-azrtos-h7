/* This FileX test concentrates on the file create/delete operations.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_fault_tolerant.h"
#include   "fx_media.h"
#include   "fx_ram_driver_test.h"
#include   "fx_utility.h"
#include   <stdio.h>

#define     DEMO_STACK_SIZE         8192
#define     CACHE_SIZE              16*128
#ifdef FX_ENABLE_FAULT_TOLERANT
#define     FAULT_TOLERANT_SIZE     FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE
#else
#define     FAULT_TOLERANT_SIZE     0
#endif


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD                ftest_0;
#endif
static FX_MEDIA                 ram_disk;
static FX_FILE                  file_1;
static FX_FILE                  file_2;
static FX_FILE                  file_3;
static FX_FILE                  file_4;
static FX_FILE                  file_5;
static FX_FILE                  file_6;
static FX_FILE                  file_7;



/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                    *ram_disk_memory;
static UCHAR                    *cache_buffer;
static UCHAR                    *fault_tolerant_buffer;   
#else
static UCHAR                    cache_buffer[CACHE_SIZE];
static UCHAR                    fault_tolerant_buffer[FAULT_TOLERANT_SIZE]; 
#endif
static UCHAR                    fat_buffer[128];
static UCHAR                    name_buffer[FX_MAX_LONG_NAME_LEN+1];


/* Define thread prototypes.  */

void    filex_file_create_delete_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_file_create_delete_application_define(void *first_unused_memory)
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
    fault_tolerant_buffer = pointer;
    pointer += FAULT_TOLERANT_SIZE;
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
UCHAR       buffer[30];
ULONG       actual;
ULONG64     actual64;
UINT        i;
ULONG64     temp;
UINT        temp_attr;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   File create/delete test................................");

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
                            512,                    // Total sectors 
                            256,                    // Sector size   
                            8,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
    /* Attempt to invalidate the media cache before the media is opened to generate an error */
    status = fx_media_cache_invalidate(&ram_disk);
    return_if_fail( status == FX_MEDIA_NOT_OPEN);
    
    /* Attempt to get space available information before the media is opened to generate an error */
    status = fx_media_extended_space_available(&ram_disk, &temp);
    return_if_fail( status == FX_MEDIA_NOT_OPEN);
    
    /* Attempt to allocate space before the media is opened to generate an error */
    status = fx_file_extended_allocate(&file_1, 0);
    return_if_fail( status == FX_NOT_OPEN);
    
    /* Attempt to allocate space before the media is opened to generate an error */
    status = fx_file_extended_best_effort_allocate(&file_1, 0, &temp);
    return_if_fail( status == FX_NOT_OPEN);
    
    /* try to create a file before the media has been opened to generate an error */
    status = fx_file_create(&ram_disk, "asdf");
    return_if_fail( status == FX_MEDIA_NOT_OPEN);
    
    /* try to close a file before the file has been opened to generate an error */
    status = fx_file_close(&file_1);
    return_if_fail( status == FX_NOT_OPEN);
    
    /* try to delete a file before the media has been opened to generate an error */
    status = fx_file_delete(&ram_disk, "asdf");
    return_if_fail( status == FX_MEDIA_NOT_OPEN);
    
    /* this will be caught by _fxe_file_open instead of _fx_file_open if DISABLE_ERROR_CHECKING is not defined */
    /* Attempt to open a file before the media has been opened to generate an error */
    status = fx_file_open(&ram_disk, &file_5, "rootname2", FX_OPEN_FOR_WRITE);
    return_if_fail( (status == FX_MEDIA_NOT_OPEN) || (status == FX_PTR_ERROR));

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);
    
#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Enable fault tolerant if FX_ENABLE_FAULT_TOLERANT is defined. */
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);
#endif

    /* try to create a file with an illegal name to generate an error */
    status = fx_file_create(&ram_disk, "");
    return_if_fail( status == FX_INVALID_NAME);
    
    /* try to create a file with an illegal path to generate an error */
    status = fx_file_create(&ram_disk, "/subdir/root");
    return_if_fail( status == FX_INVALID_PATH);

    /* Simple 8.3 rename in the root directory.  */
    status =  fx_directory_create(&ram_disk, "subdir");
    status += fx_file_create(&ram_disk, "rootname");
    status += fx_file_create(&ram_disk, "rootname1");
    status += fx_file_create(&ram_disk, "rootname2");
    status += fx_file_create(&ram_disk, "/subdir/rootname");
    status += fx_file_create(&ram_disk, "/subdir/rootname1");
    return_if_fail( status == FX_SUCCESS);
    
    /* Attempt to open a file that is not a file to generate an error */
    status = fx_file_open(&ram_disk, &file_5, "subdir", FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_NOT_A_FILE);
    
    /* try to create and delete a file when the media is write protected to generate an error */
    ram_disk.fx_media_driver_write_protect = FX_TRUE;
    status = fx_file_create(&ram_disk, "asdf");
    return_if_fail( status == FX_WRITE_PROTECT);

    status = fx_file_delete(&ram_disk, "rootname");
    return_if_fail( status == FX_WRITE_PROTECT);
    
    /* Attempt to open a file while the media is write protected to generate an error */
    status = fx_file_open(&ram_disk, &file_5, "rootname2", FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_WRITE_PROTECT);
    ram_disk.fx_media_driver_write_protect = FX_FALSE;

    /* Attempt to create the same file again. This should cause an error!  */
    status =  fx_file_create(&ram_disk, "rootname");
    return_if_fail( status == FX_ALREADY_CREATED);
    
    /* attempt to open a file with an invalid open type to generate an error */
    /* This code is executing differently on local vs server. Disabled until cause is explored
    status =  fx_file_open(&ram_disk, &file_5, "rootname2", 3);
    return_if_fail( status == FX_ACCESS_ERROR);
    */
    
    /* try to delete a file in a directory that is read only */
    status  = fx_file_attributes_read(&ram_disk, "rootname2", &temp_attr);
    status += fx_file_attributes_set(&ram_disk, "rootname2", FX_READ_ONLY);
    status += fx_file_delete(&ram_disk, "rootname2");
    status += fx_file_attributes_set(&ram_disk, "rootname2", temp_attr);
    return_if_fail( status == FX_WRITE_PROTECT);
    
    /* Open all the files.  */
    status =   fx_file_open(&ram_disk, &file_1, "rootname", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_2, "rootname1", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_3, "/subdir/rootname", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_4, "/subdir/rootname1", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_5, "rootname2", FX_OPEN_FOR_READ);
    return_if_fail( status == FX_SUCCESS);
    
    /* attempt to open a file that is already open */
    status =  fx_file_open(&ram_disk, &file_6, "rootname", FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_ACCESS_ERROR);
    
    /* try to create and delete a file when the media is write protected to generate an error */
    ram_disk.fx_media_driver_write_protect = FX_TRUE;
    status = fx_file_create(&ram_disk, "asdf");
    return_if_fail( status == FX_WRITE_PROTECT);
    status = fx_file_delete(&ram_disk, "asdf");
    return_if_fail( status == FX_WRITE_PROTECT);
    
    /* Attempt to allocate space while the media is write protected to generate an error */
    status = fx_file_extended_allocate(&file_1, 1);
    return_if_fail( status == FX_WRITE_PROTECT);
    status = fx_file_extended_best_effort_allocate(&file_1, 1, &temp);
    return_if_fail( status == FX_WRITE_PROTECT);
    ram_disk.fx_media_driver_write_protect = FX_FALSE;
    
/* test the error checking */
#ifndef FX_DISABLE_ERROR_CHECKING
    
    /* send a null pointer to generate an error */
    status =  fx_media_extended_space_available(FX_NULL, FX_NULL);
    return_if_fail( status == FX_PTR_ERROR);
    
    /* send a null pointer to generate an error */
    status = fx_file_extended_best_effort_allocate(FX_NULL, 0, FX_NULL);
    
    /* send null pointer to generate an error */
    status = fx_file_create(FX_NULL, "rootname");
    return_if_fail( status == FX_PTR_ERROR);
    
    /* send null pointer to generate an error */
    status = fx_file_open(FX_NULL, FX_NULL, "rootname", 0);
    return_if_fail( status == FX_PTR_ERROR);
    
    /* attempt to open an already open file to generate an error */
    status = fx_file_open(&ram_disk, &file_1, "rootname", FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_PTR_ERROR);
    
    /* Attempt to allocate space for a file that is not open to write to generate an error */
    status = fx_file_open(&ram_disk, &file_5, "rootname2", FX_OPEN_FOR_READ);
    status = fx_file_extended_allocate(&file_5, 1);
    return_if_fail( status == FX_ACCESS_ERROR);
    
    /* Attempt to allocate space for a file that is not open to write to generate an error */
    status = fx_file_extended_best_effort_allocate(&file_5, 0, &temp);
    return_if_fail( status == FX_ACCESS_ERROR);
    
    /* Allocate 0 space for a file */
    status = fx_file_extended_allocate(&file_1, 0);
    return_if_fail( status == FX_SUCCESS);
    
    /* Allocate 0 space for a file */
    status = fx_file_extended_best_effort_allocate(&file_1, 0, &temp);
    return_if_fail( status == FX_SUCCESS);
    
    /* Attempt to allocate too much space for a file */
    status = fx_file_extended_allocate(&file_1, 0xFFFFFFFFFFFFFFFF);
    return_if_fail( status == FX_NO_MORE_SPACE);

    /* Corrupt the media.  */
    ram_disk.fx_media_bytes_per_sector = 0;
    status = fx_file_extended_allocate(&file_1, 1);
    return_if_fail(status == FX_MEDIA_INVALID);

    status = fx_file_extended_best_effort_allocate(&file_1, 1, &actual64);
    return_if_fail(status == FX_MEDIA_INVALID);

    ram_disk.fx_media_bytes_per_sector = 256;
    
#endif /* FX_DISABLE_ERROR_CHECKING */
    
    /* try to delete an open file to generate an error */
    status = fx_file_delete(&ram_disk, "rootname");
    return_if_fail( status == FX_ACCESS_ERROR);
    
    /* try to delete something that is not a file to generate an error */
    status = fx_file_delete(&ram_disk, "subdir");
    return_if_fail( status == FX_NOT_A_FILE);

    /* Now write a buffer to each file.   */
    status =  fx_file_write(&file_1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status +=  fx_file_write(&file_2, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status +=  fx_file_write(&file_3, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    status +=  fx_file_write(&file_4, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    return_if_fail( status == FX_SUCCESS);

    /* Seek to the beginning of each file.  */
    status +=  fx_file_seek(&file_1, 0);
    status +=  fx_file_seek(&file_2, 0);
    status +=  fx_file_seek(&file_3, 0);
    status +=  fx_file_seek(&file_4, 0);
    return_if_fail( status == FX_SUCCESS);

    /* Now read the buffer.  */
    status =  fx_file_read(&file_1, buffer, 30, &actual);
    return_if_fail( (status == FX_SUCCESS) && (actual == 26));
    
/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send null pointer to generate an error */
    status = fx_file_read(FX_NULL, buffer, 30, &actual);
    return_if_fail(status == FX_PTR_ERROR);
#endif /* FX_DISABLE_ERROR_CHECKING */

    /* Now read the buffer.  */
    status =  fx_file_read(&file_2, buffer, 30, &actual);
    return_if_fail( (status == FX_SUCCESS) && (actual == 26));

    /* Now read the buffer.  */
    status =  fx_file_read(&file_3, buffer, 30, &actual);
    return_if_fail( (status == FX_SUCCESS) && (actual == 26));

    /* Now read the buffer.  */
    status =  fx_file_read(&file_4, buffer, 30, &actual);
    return_if_fail( (status == FX_SUCCESS) && (actual == 26));

    /* Close all files.  */
    status =   fx_file_close(&file_1);
    status +=  fx_file_close(&file_2);
    status +=  fx_file_close(&file_3);
    status +=  fx_file_close(&file_4);
    status +=  fx_file_close(&file_5);
    return_if_fail( status == FX_SUCCESS);
    
/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send null pointer to generate an error */
    status = fx_file_close(FX_NULL);
    return_if_fail(status == FX_PTR_ERROR);
    
    /* send null pointer to generate an error */
    status = fx_file_delete(FX_NULL, "rootname");
    return_if_fail(status == FX_PTR_ERROR);
#endif /* FX_DISABLE_ERROR_CHECKING */

    /* Delete all files.  */
    status = fx_file_delete(&ram_disk, "rootname");
    status += fx_file_delete(&ram_disk, "rootname1");
    status += fx_file_delete(&ram_disk, "/subdir/rootname");
    status += fx_file_delete(&ram_disk, "/subdir/rootname1");
    status += fx_file_delete(&ram_disk, "rootname2");
    status += fx_directory_delete(&ram_disk, "/subdir");
    return_if_fail( status == FX_SUCCESS);

    /* Attempt to delete an already deleted file.  */
    status += fx_file_delete(&ram_disk, "rootname");
    return_if_fail( status == FX_NOT_FOUND);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);
    
    /* Test corner cases in extended best effort allocate.  */
        
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
                            7000,                   // Total sectors - FAT 16
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_if_fail( status == FX_SUCCESS);
    
    /* Create a file.  */
    status = fx_file_create(&ram_disk, "TEST.TXT");
    
    /* Create a secondary file.  */
    status += fx_file_create(&ram_disk, "TEST1.TXT");
    return_if_fail( status == FX_SUCCESS);
    
    /* Open the file. */
    status =  fx_file_open(&ram_disk, &file_6, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_5, "TEST1.TXT", FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_SUCCESS);

    /* Write one cluster of information to the TEST1.TXT file.  */
    status =  fx_file_write(&file_5, buffer, 128);
    
    /* Close the secondary file.  */
    status += fx_file_close(&file_5);

    /* Loop to take up the entire ram disk by writing to this file.  */
    while (ram_disk.fx_media_available_clusters)
    {
        /* Write a one cluster block.  */
        status += fx_file_write(&file_6, buffer, 128);
    }
    return_if_fail( status == FX_SUCCESS);
    
    /* Attempt to allocate when there is nothing available.  */
    status =  fx_file_extended_best_effort_allocate(&file_6, 128, &actual64);
    return_if_fail( status == FX_NO_MORE_SPACE);
    
    /* Now release the first cluster to create a hole at the front.  */
    status =  fx_file_delete(&ram_disk, "TEST1.TXT");
    
    /* Now flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }

    /* Read the first FAT sector.  */
    status += fx_media_read(&ram_disk, 1, (VOID *) fat_buffer);  

    /* Add a FAT entry randomly in the FAT table.  */
    fat_buffer[4] =  0x03;
    fat_buffer[5] =  0x00;

    /* Write the FAT corruption out.  */
    status += fx_media_write(&ram_disk, 1, (VOID *) fat_buffer);
    return_if_fail( status == FX_SUCCESS);
    
    /* Attempt to allocate when there is nothing available - with a lost cluster.  */
    status =  fx_file_extended_best_effort_allocate(&file_6, 128, &actual64);
    return_if_fail( status == FX_NO_MORE_SPACE);

    /* Attempt to allocate when there is an I/O error.  */
    _fx_utility_fat_entry_read_error_request =  1;
    status =  fx_file_extended_best_effort_allocate(&file_6, 128, &actual64);
    _fx_utility_fat_entry_read_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);

    /* Now flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) fat_buffer);  

    /* Make the first entry available.  */
    fat_buffer[4] =  0x00;
    fat_buffer[5] =  0x00;

    /* Write the FAT corruption out.  */
    status += fx_media_write(&ram_disk, 1, (VOID *) fat_buffer);
    return_if_fail( status == FX_SUCCESS);

    /* Allocate with an exact fit.  */
    status =  fx_file_extended_best_effort_allocate(&file_6, 128, &actual64);
    return_if_fail( status == FX_SUCCESS);

    /* Release all the clusters for this file.  */
    status =  fx_file_extended_truncate_release(&file_6, 0);
    status +=  fx_file_write(&file_6, fat_buffer, 128);
    status +=  fx_file_write(&file_6, fat_buffer, 128);
    status +=  fx_file_write(&file_6, fat_buffer, 128);
    status +=  fx_file_write(&file_6, fat_buffer, 128);   
    
    /* Allocate 4 clusters.  */
    status +=  fx_file_extended_best_effort_allocate(&file_6, 512, &actual64);
    return_if_fail( status == FX_SUCCESS);

    /* Release all the clusters for this file.  */
    status =  fx_file_extended_truncate_release(&file_6, 0);
  
    /* Allocate 4 clusters with no clusters in the file.  */
    status +=  fx_file_extended_best_effort_allocate(&file_6, 512, &actual64);
    return_if_fail( status == FX_SUCCESS);

    /* Write 512 bytes of data out.  */
    status +=  fx_file_write(&file_6, fat_buffer, 128);
    status +=  fx_file_write(&file_6, fat_buffer, 128);
    status +=  fx_file_write(&file_6, fat_buffer, 128);
    status +=  fx_file_write(&file_6, fat_buffer, 128);   

    /* Now allocate perform allocations with I/O errors on building the new FAT chain.  */
    _fx_utility_fat_entry_write_error_request =  1;
    status +=  fx_file_extended_best_effort_allocate(&file_6, 512, &actual64);
    _fx_utility_fat_entry_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);
    
    /* Now allocate perform allocations with I/O error on writing the EOF.  */
    _fx_utility_fat_entry_write_error_request =  4;
    status =  fx_file_extended_best_effort_allocate(&file_6, 512, &actual64);
    _fx_utility_fat_entry_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);
    
    /* Now allocate perform allocations with I/O error on linking to last cluster.  */
    _fx_utility_fat_entry_write_error_request =  5;
    status =  fx_file_extended_best_effort_allocate(&file_6, 512, &actual64);
    _fx_utility_fat_entry_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);
    
    /* Release all the clusters for this file.  */
    status =  fx_file_extended_truncate_release(&file_6, 0);

    /* Now flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }

#ifndef FX_ENABLE_FAULT_TOLERANT
    /* Now allocate perform allocations with I/O error on the directory write request.  */
    _fx_utility_logical_sector_write_error_request =  1;
    status =  fx_file_extended_best_effort_allocate(&file_6, 128, &actual64);
    _fx_utility_logical_sector_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);
#endif
    
    /* Close the file and the media.  */
    status =  fx_file_close(&file_6);
    status += fx_media_close(&ram_disk);
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
                            21000,                  // Total sectors - FAT 16
                            128,                    // Sector size   
                            3,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_if_fail( status == FX_SUCCESS);
    
    /* Create a file.  */
    status = fx_file_create(&ram_disk, "TEST.TXT");
    return_if_fail( status == FX_SUCCESS);
    
    /* Open the file. */
    status =  fx_file_open(&ram_disk, &file_6, "TEST.TXT", FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_SUCCESS);

    /* Write data to the TEST.TXT file.  */
    status =  fx_file_write(&file_6, buffer, 128);
    status += fx_file_write(&file_6, buffer, 128);
    status += fx_file_write(&file_6, buffer, 64);

    /* Allocate more clusters but with the offset on a sector boundary rather than a cluster boundary.  */
    status =  fx_file_extended_best_effort_allocate(&file_6, 512, &actual64);
    return_if_fail( status == FX_SUCCESS);

    /* Truncate the file.  */
    status =  fx_file_extended_truncate_release(&file_6, 0);

    /* Write data to the TEST.TXT file.  */
    status +=  fx_file_write(&file_6, buffer, 128);

    /* Allocate more clusters but with the offset on a sector boundary rather than a cluster boundary.  */
    status +=  fx_file_extended_best_effort_allocate(&file_6, 512, &actual64);
    return_if_fail( status == FX_SUCCESS);

    /* Close the file and the media.  */
    status =  fx_file_close(&file_6);
    status += fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Test corner cases in extended allocate.  */
        
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
                            7000,                   // Total sectors - FAT 16
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_if_fail( status == FX_SUCCESS);
    
    /* Create a file.  */
    status = fx_file_create(&ram_disk, "TEST.TXT");
    
    /* Create a secondary file.  */
    status += fx_file_create(&ram_disk, "TEST1.TXT");
    return_if_fail( status == FX_SUCCESS);
    
    /* Open the file. */
    status =  fx_file_open(&ram_disk, &file_6, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status =  fx_file_open(&ram_disk, &file_5, "TEST1.TXT", FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_SUCCESS);

    /* Write one cluster of information to the TEST1.TXT file.  */
    status =  fx_file_write(&file_5, buffer, 128);
    
    /* Close the secondary file.  */
    status += fx_file_close(&file_5);

    /* Loop to take up the entire ram disk by writing to this file.  */
    while (ram_disk.fx_media_available_clusters)
    {
    
        /* Write a one cluster block.  */
        status += fx_file_write(&file_6, buffer, 128);
    }
    return_if_fail( status == FX_SUCCESS);
    
    /* Attempt to allocate when there is nothing available.  */
    status =  fx_file_extended_allocate(&file_6, 128);
    return_if_fail( status == FX_NO_MORE_SPACE);
    
    /* Now release the first cluster to create a hole at the front.  */
    status =  fx_file_delete(&ram_disk, "TEST1.TXT");
    
    /* Now flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }

    /* Read the first FAT sector.  */
    status += fx_media_read(&ram_disk, 1, (VOID *) fat_buffer);  

    /* Add a FAT entry randomly in the FAT table.  */
    fat_buffer[4] =  0x03;
    fat_buffer[5] =  0x00;

    /* Write the FAT corruption out.  */
    status += fx_media_write(&ram_disk, 1, (VOID *) fat_buffer);
    return_if_fail( status == FX_SUCCESS);
    
    /* Attempt to allocate when there is nothing available - with a lost cluster.  */
    status =  fx_file_extended_allocate(&file_6, 128);
    return_if_fail( status == FX_NO_MORE_SPACE);

    /* Attempt to allocate when there is an I/O error.  */
    _fx_utility_fat_entry_read_error_request =  1;
    status =  fx_file_extended_allocate(&file_6, 128);
    _fx_utility_fat_entry_read_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);

    /* Now flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }

    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) fat_buffer);  

    /* Make the first entry available.  */
    fat_buffer[4] =  0x00;
    fat_buffer[5] =  0x00;

    /* Write the FAT corruption out.  */
    status += fx_media_write(&ram_disk, 1, (VOID *) fat_buffer);
    return_if_fail( status == FX_SUCCESS);

    /* Allocate with an exact fit.  */
    status =  fx_file_extended_allocate(&file_6, 128);
    return_if_fail( status == FX_SUCCESS);

    /* Release all the clusters for this file.  */
    status =  fx_file_extended_truncate_release(&file_6, 0);
    status +=  fx_file_write(&file_6, fat_buffer, 128);
    status +=  fx_file_write(&file_6, fat_buffer, 128);
    status +=  fx_file_write(&file_6, fat_buffer, 128);
    status +=  fx_file_write(&file_6, fat_buffer, 128);   
    
    /* Allocate 4 clusters.  */
    status +=  fx_file_extended_allocate(&file_6, 512);
    return_if_fail( status == FX_SUCCESS);

    /* Release all the clusters for this file.  */
    status =  fx_file_extended_truncate_release(&file_6, 0);
  
    /* Allocate 4 clusters with no clusters in the file.  */
    status +=  fx_file_extended_allocate(&file_6, 512);
    return_if_fail( status == FX_SUCCESS);

    /* Write 512 bytes of data out.  */
    status +=  fx_file_write(&file_6, fat_buffer, 128);
    status +=  fx_file_write(&file_6, fat_buffer, 128);
    status +=  fx_file_write(&file_6, fat_buffer, 128);
    status +=  fx_file_write(&file_6, fat_buffer, 128);   

    /* Now allocate perform allocations with I/O errors on building the new FAT chain.  */
    _fx_utility_fat_entry_write_error_request =  1;
    status +=  fx_file_extended_allocate(&file_6, 512);
    _fx_utility_fat_entry_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);
    
    /* Now allocate perform allocations with I/O error on writing the EOF.  */
    _fx_utility_fat_entry_write_error_request =  4;
    status =  fx_file_extended_allocate(&file_6, 512);
    _fx_utility_fat_entry_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);
    
    /* Now allocate perform allocations with I/O error on linking to last cluster.  */
    _fx_utility_fat_entry_write_error_request =  5;
    status =  fx_file_extended_allocate(&file_6, 512);
    _fx_utility_fat_entry_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);
    
    /* Release all the clusters for this file.  */
    status =  fx_file_extended_truncate_release(&file_6, 0);

    /* Now flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }

#ifndef FX_ENABLE_FAULT_TOLERANT

    /* Now allocate perform allocations with I/O error on the directory write request.  */
    _fx_utility_logical_sector_write_error_request =  1;
    status =  fx_file_extended_allocate(&file_6, 128);
    _fx_utility_logical_sector_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);
#endif
    
    /* Close the file and the media.  */
    status =  fx_file_close(&file_6);
    status += fx_media_close(&ram_disk);
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
                            21000,                  // Total sectors - FAT 16
                            128,                    // Sector size   
                            3,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_if_fail( status == FX_SUCCESS);
    
    /* Create a file.  */
    status = fx_file_create(&ram_disk, "TEST.TXT");
    return_if_fail( status == FX_SUCCESS);
    
    /* Open the file. */
    status =  fx_file_open(&ram_disk, &file_6, "TEST.TXT", FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_SUCCESS);

    /* Write one cluster of information to the TEST1.TXT file.  */
    status =  fx_file_write(&file_6, buffer, 128);
    status += fx_file_write(&file_6, buffer, 128);
    status += fx_file_write(&file_6, buffer, 64);

    /* Allocate more clusters but with the offset on a sector boundary rather than a cluster boundary.  */
    status =  fx_file_extended_allocate(&file_6, 512);
    return_if_fail( status == FX_SUCCESS);

    /* Truncate the file.  */
    status =  fx_file_extended_truncate_release(&file_6, 0);

    /* Write data to the TEST.TXT file.  */
    status +=  fx_file_write(&file_6, buffer, 128);

    /* Allocate more clusters but with the offset on a sector boundary rather than a cluster boundary.  */
    status +=  fx_file_extended_allocate(&file_6, 512);
    return_if_fail( status == FX_SUCCESS);

    /* Now test the maximum allocate size.  */
    actual64 =  file_6.fx_file_current_available_size;
    file_6.fx_file_current_available_size =  0xFFFFFFF8;
    
    /* Allocate more clusters but with the file near maximum size to test the maximum size logic.  */
    status  =  fx_file_extended_allocate(&file_6, 512);
    file_6.fx_file_current_available_size =  actual64;
    return_if_fail( status == FX_NO_MORE_SPACE);

    /* Now test the maximum allocate size.  */
    actual64 =  file_6.fx_file_current_available_size;
    file_6.fx_file_current_available_size =  0xFFFFFFFFFFFFFFF8;
    
    /* Allocate more clusters but with the file near maximum size to test the maximum size logic.  */
    status  =  fx_file_extended_allocate(&file_6, 512);
    file_6.fx_file_current_available_size =  actual64;
    return_if_fail( status == FX_NO_MORE_SPACE);

    /* Now test the maximum allocate size.  */
    actual64 =  file_6.fx_file_current_available_size;
    file_6.fx_file_current_available_size =  0xFFFFFFF8;
    
    /* Allocate more clusters but with the file near maximum size to test the maximum size logic.  */
    status  =  fx_file_extended_best_effort_allocate(&file_6, 512, &actual64);
    file_6.fx_file_current_available_size =  actual64;
    return_if_fail( status == FX_NO_MORE_SPACE);

    /* Now test the maximum allocate size.  */
    actual64 =  file_6.fx_file_current_available_size;
    file_6.fx_file_current_available_size =  0xFFFFFFFFFFFFFFF8;
    
    /* Allocate more clusters but with the file near maximum size to test the maximum size logic.  */
    status  =  fx_file_extended_best_effort_allocate(&file_6, 512, &actual64);
    file_6.fx_file_current_available_size =  actual64;
    return_if_fail( status == FX_NO_MORE_SPACE);

    /* Now test a maximium size allocation.  */
    status  =  fx_file_extended_best_effort_allocate(&file_6, 0xFFFFFFFFFFFFFFF8ULL, &actual64);
    return_if_fail( status == FX_NO_MORE_SPACE);
    
    /* Close the file and the media.  */
    status =  fx_file_close(&file_6);
    status += fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);
    
    /* Check the corner cases of file delete.  */
    
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
                            7000,                   // Total sectors - FAT 16
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_if_fail( status == FX_SUCCESS);
    
    /* Create a set of files.  */
    status =  fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_create(&ram_disk, "TEST1.TXT");
    status += fx_file_create(&ram_disk, "TEST2.TXT");
    status += fx_file_create(&ram_disk, "TEST3.TXT");
    status += fx_file_create(&ram_disk, "TEST4.TXT");
    status += fx_file_create(&ram_disk, "TEST5.TXT");
    status += fx_file_create(&ram_disk, "TEST6.TXT");
    status += fx_file_create(&ram_disk, "TEST7.TXT");
    status += fx_file_create(&ram_disk, "TEST8.TXT");
    status += fx_file_create(&ram_disk, "TEST9.TXT");
    status += fx_file_create(&ram_disk, "TEST10.TXT");
    status += fx_file_create(&ram_disk, "TEST11.TXT");
    status += fx_file_create(&ram_disk, "TEST12.TXT");
    status += fx_file_create(&ram_disk, "TEST13.TXT");
    status += fx_file_create(&ram_disk, "TEST14.TXT");
    status += fx_file_create(&ram_disk, "TEST15.TXT");
    status += fx_file_create(&ram_disk, "TEST16.TXT");
    status += fx_file_create(&ram_disk, "TEST17.TXT");
    status += fx_file_create(&ram_disk, "TEST18.TXT");
    status += fx_file_create(&ram_disk, "TEST19.TXT");
    status += fx_file_create(&ram_disk, "TEST20.TXT");
    status += fx_file_create(&ram_disk, "TEST21.TXT");
    status += fx_file_create(&ram_disk, "TEST22.TXT");
    status += fx_file_create(&ram_disk, "TEST23.TXT");
    status += fx_file_create(&ram_disk, "TEST24.TXT");
    status += fx_file_create(&ram_disk, "TEST25.TXT");
    status += fx_file_create(&ram_disk, "TEST26.TXT");
    status += fx_file_create(&ram_disk, "TEST27.TXT");
    status += fx_file_create(&ram_disk, "TEST28.TXT");
    status += fx_file_create(&ram_disk, "TEST29.TXT");
    status += fx_file_create(&ram_disk, "TEST30.TXT");
    return_if_fail( status == FX_SUCCESS);
    
    /* Open a couple files.... */
    status =   fx_file_open(&ram_disk, &file_4, "TEST.TXT", FX_OPEN_FOR_WRITE);
    status =   fx_file_open(&ram_disk, &file_5, "TEST1.TXT", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_6, "TEST30.TXT", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_7, "TEST29.TXT", FX_OPEN_FOR_WRITE);
    status +=  fx_file_write(&file_4, fat_buffer, 128);
    status +=  fx_file_write(&file_4, fat_buffer, 128);
    status +=  fx_file_write(&file_4, fat_buffer, 128);
    status +=  fx_file_write(&file_4, fat_buffer, 128);
    status +=  fx_file_write(&file_5, fat_buffer, 128);
    status +=  fx_file_write(&file_5, fat_buffer, 128);
    status +=  fx_file_write(&file_5, fat_buffer, 128);
    status +=  fx_file_write(&file_5, fat_buffer, 128);
    status +=  fx_file_write(&file_6, fat_buffer, 128);
    status +=  fx_file_write(&file_6, fat_buffer, 128);
    status +=  fx_file_write(&file_6, fat_buffer, 128);
    status +=  fx_file_write(&file_6, fat_buffer, 128);
    status +=  fx_file_write(&file_7, fat_buffer, 128);
    status +=  fx_file_write(&file_7, fat_buffer, 128);
    status +=  fx_file_write(&file_7, fat_buffer, 128);
    status +=  fx_file_write(&file_7, fat_buffer, 128);
    return_if_fail( status == FX_SUCCESS);
    
    /* Now flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }
    
    /* Close TEST.TXT.  */
    status =  fx_file_close(&file_4);
    
    /* Attempt to delete the file, but with open files on a different offset/sector and a write I/O error.  */
    _fx_utility_logical_sector_read_error_request =  2;
    status +=  fx_file_delete(&ram_disk, "TEST.TXT");
    _fx_utility_logical_sector_read_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);

    /* Close all the open files.  */
    status = fx_file_close(&file_5);
    status += fx_file_close(&file_6);
    status += fx_file_close(&file_7);

    /* Now flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }
    
    /* Attempt to delete the file, but with a FAT read error.  */
    _fx_utility_fat_entry_read_error_request =  1;
    status += fx_file_delete(&ram_disk, "TEST1.TXT");
    _fx_utility_fat_entry_read_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);
        
    /* Now flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }
    
    /* Attempt to delete the file, but with a FAT write error.  */
    _fx_utility_fat_entry_write_error_request =  1;
    status = fx_file_delete(&ram_disk, "TEST30.TXT");
    _fx_utility_fat_entry_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);
    
    /* Now flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }
    
    /* Read the first FAT sector.  */
    status = fx_media_read(&ram_disk, 1, (VOID *) fat_buffer);  

    /* Make the first entry available.  */
    fat_buffer[4] =  0x02;
    fat_buffer[5] =  0x00;

    /* Write the FAT corruption out.  */
    status += fx_media_write(&ram_disk, 1, (VOID *) fat_buffer);
    return_if_fail( status == FX_SUCCESS);

    /* Attempt to delete the file, but with a bad FAT chain.  */
    status = fx_file_delete(&ram_disk, "TEST.TXT");
    return_if_fail( status == FX_FAT_READ_ERROR);
    
    /* Attempt to delete the file that exceeds the cluster clount (that we modify to be small).  */
    actual =  ram_disk.fx_media_total_clusters;
    ram_disk.fx_media_total_clusters =  1;
    status =  fx_file_delete(&ram_disk, "TEST29.TXT");   
    ram_disk.fx_media_total_clusters =  actual;
    return_if_fail( status == FX_FAT_READ_ERROR);
    
    /* Build a long file name, exceeding FX_MAX_LONG_NAME_LEN.  */
    for (i = 0; i < (FX_MAX_LONG_NAME_LEN+1); i++)
    {
        name_buffer[i] =  'a';
    }
    name_buffer[FX_MAX_LONG_NAME_LEN] =  0;
    
    /* Attempt to create a file with this extra long name... this is expected to fail.  */
    status =  fx_file_create(&ram_disk, (CHAR *)name_buffer);
    return_if_fail( status == FX_INVALID_NAME);
    
    /* Now open and write to a file so we can test file close I/O error condition.  */
    status =  fx_file_open(&ram_disk, &file_7, "TEST20.TXT", FX_OPEN_FOR_WRITE);
    status +=  fx_file_write(&file_7, fat_buffer, 128);

    /* Close the file with an I/O error.  */
    _fx_utility_logical_sector_read_error_request =  1;
    status += fx_file_close(&file_7);
    _fx_utility_logical_sector_read_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);
    
    /* Now call the best effort allocate with a closed file handle.  */
    status =  fx_file_best_effort_allocate(&file_7, 1, &actual);
    return_if_fail( status == FX_NOT_OPEN);
    
    /* Close the media.  */
    status = fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    printf("SUCCESS!\n");
    test_control_return(0);
}
