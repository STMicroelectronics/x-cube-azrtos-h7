/* This FileX test concentrates on the file rename operations.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_ram_driver_test.h"
#include   "fx_fault_tolerant.h"
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
static FX_FILE                  my_file;


/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                    *ram_disk_memory;
static UCHAR                    *cache_buffer;
static UCHAR                    *fault_tolerant_buffer;   
#else
static UCHAR                     cache_buffer[CACHE_SIZE];
static UCHAR                     fault_tolerant_buffer[FAULT_TOLERANT_SIZE];   
#endif


/* Define thread prototypes.  */

void    filex_file_rename_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_file_rename_application_define(void *first_unused_memory)
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
FX_FILE     open_file;
CHAR        long_name1[FX_MAX_LONG_NAME_LEN+1];
CHAR        long_name2[FX_MAX_LONG_NAME_LEN+1];
UINT        i;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   File rename test.......................................");

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
/* Allocated a larger disk to enable fault tolerant feature. */                            
#ifdef FX_ENABLE_FAULT_TOLERANT
                            512 * 8,                // Total sectors 
                            256,                    // Sector size   
                            8,                      // Sectors per cluster
#else
                            512,                    // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
#endif
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
    /* try to rename a file to something invalid */
    status = fx_file_rename(&ram_disk, "MYTEST", "");
    return_if_fail( status == FX_INVALID_NAME);
    
    /* try to rename a file before the media is opened */
    status = fx_file_rename(&ram_disk, "MYTEST", "OURTEST");
    return_if_fail( status == FX_MEDIA_NOT_OPEN);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);
    
#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Enable fault tolerant if FX_ENABLE_FAULT_TOLERANT is defined. */
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);
#endif

    /* try to rename a file while the media is write protected */
    ram_disk.fx_media_driver_write_protect = FX_TRUE;
    status = fx_file_rename(&ram_disk, "MYTEST", "OURTEST");
    ram_disk.fx_media_driver_write_protect = FX_FALSE;
    return_if_fail( status == FX_WRITE_PROTECT);
    
    /* try to rename a file that cant be found */
    status = fx_file_rename(&ram_disk, "MYTEST", "OURTEST");
    return_if_fail( status != FX_SUCCESS);
    
/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING

    /* send null pointer to generate an error */
    status = fx_file_rename(FX_NULL, "", "");
    return_if_fail( status == FX_PTR_ERROR);

#endif /* FX_DISABLE_ERROR_CHECKING */

    /* Simple 8.3 rename in the root directory.  */
    status =  fx_file_create(&ram_disk, "MYTEST");
    status += fx_file_rename(&ram_disk, "MYTEST", "OURTEST");
    return_if_fail( status == FX_SUCCESS);
    
    /* rename the files to lower case and upper case versions of themselves */
    status  = fx_file_rename(&ram_disk, "OURTEST", "ourtest");
    status += fx_file_rename(&ram_disk, "ourtest", "OURTEST");
    return_if_fail( status == FX_SUCCESS);
    
    /* try to rename a file into a subdirectory that does not exist */
    status = fx_file_rename(&ram_disk, "OURTEST", "DOES_NOT_EXITS/OURTEST");
    return_if_fail( status == FX_INVALID_NAME);

    /* Now create a 2 sub-directories.  */
    status  = fx_directory_create(&ram_disk, "/subdir1");
    status += fx_directory_create(&ram_disk, "/subdir2");
    return_if_fail( status == FX_SUCCESS);
    
    /* try to rename a file to something that already exists */
    status = fx_file_rename(&ram_disk, "OURTEST", "subdir1");
    return_if_fail( status == FX_ALREADY_CREATED);
    
#ifndef FX_DONT_UPDATE_OPEN_FILES

    /* rename a file while it is open and while another is opened to get code coverage */
    status  = fx_file_create(&ram_disk, "OPEN_FILE");
    status += fx_file_open(&ram_disk, &open_file, "OPEN_FILE", FX_OPEN_FOR_WRITE);
    status += fx_file_rename(&ram_disk, "OURTEST", "NEWTEST");
    status += fx_file_rename(&ram_disk, "NEWTEST", "OURTEST");
    status += fx_file_rename(&ram_disk, "OPEN_FILE", "NEW_OPEN_FILE");
    return_if_fail( status == FX_SUCCESS);

#endif

    /* move the file through the subdirectories */
    status  = fx_file_rename(&ram_disk, "OURTEST", "/subdir1/NEWOURTEST");
    status += fx_file_rename(&ram_disk, "/subdir1/NEWOURTEST", "/subdir2/OURTEST");
    status += fx_file_rename(&ram_disk, "/subdir2/OURTEST", "OURTEST");
    status += fx_file_rename(&ram_disk, "OURTEST", ".OURTEST");
    status += fx_file_delete(&ram_disk, ".OURTEST");    
    return_if_fail( status == FX_SUCCESS);
    
    /* try to rename a file that isnt a file */
    status = fx_file_rename(&ram_disk, "subdir1", "OURTEST");
    return_if_fail( status == FX_NOT_A_FILE);

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
                            128,                    // Directory Entries
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

    /* Loop to create a very long name.  */
    for (i = 0; i < FX_MAX_LONG_NAME_LEN; i++)
    {
        long_name1[i] =  'a';
        long_name2[i] =  'a';
    }
    
    /* Put NULL and special character at the end of the file.  */
    long_name1[i]   =  0;
    long_name1[i-1] =  0;
    long_name1[i-2] =  '~';
    long_name2[i] =    0;
    long_name2[i-1] =  0;
    long_name2[i-2] =  '~';
    
    /* Create a super short file name.  */
    status =  fx_file_create(&ram_disk, "sname");
    status += fx_file_create(&ram_disk, long_name1);
    return_if_fail( status == FX_SUCCESS);

    /* Attempt to change the short name with leading spaces.  */
    status =  fx_file_rename(&ram_disk, "sname", "    name");
    return_if_fail( status == FX_SUCCESS);
    
    /* Now attempt to change the short name with middle spaces.  */
    status =  fx_file_rename(&ram_disk, "name", "\\new name");
    return_if_fail( status == FX_SUCCESS);

    /* Make the second long name too big, to test the max size of the new long file name.  */
    long_name2[FX_MAX_LONG_NAME_LEN-1] = '~';
    status =  fx_file_rename(&ram_disk, long_name1, long_name2);
    return_if_fail( status == FX_INVALID_NAME);

    /* Make the second long file name the correct max length again.  */
    long_name2[FX_MAX_LONG_NAME_LEN-1] =  0;

    /* Change the name of the second long file name.  */
    long_name2[FX_MAX_LONG_NAME_LEN-3] = 'a';       
    status =  fx_file_rename(&ram_disk, long_name1, long_name2);
    return_if_fail( status == FX_SUCCESS);

    /* Change back to the first long file name.  */
    status =  fx_file_rename(&ram_disk, long_name2, long_name1);
    return_if_fail( status == FX_SUCCESS);

    /* Now open the long file name so we can get that logic excercised in the rename logic.  */
    status =  fx_file_open(&ram_disk, &my_file, long_name1, FX_OPEN_FOR_WRITE);
    status += fx_file_rename(&ram_disk, long_name1, long_name2);
    return_if_fail( status == FX_SUCCESS);

    /* Close the file.  */
    fx_file_close(&my_file);
    
    /* Now test the directory free search logic when there are no more entries.  */

    /* First, fill up the root directory.  */
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1000");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1001");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1002");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1003");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1004");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1005");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1006");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1007");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1008");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1009");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1010");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1011");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1012");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1013");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1014");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1015");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1016");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1017");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1018");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1019");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1020");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1021");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1022");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1023");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1024");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1025");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1026");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1027");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1028");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1029");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1030");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1031");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1032");
    status +=  fx_file_create(&ram_disk, "aaaaaaaaaaaaa1033");
 
    /* Now attempt to rename the long file again.  */    
    status += fx_file_rename(&ram_disk, long_name2, long_name1);

    /* Check status - this should fail because we do not have enough room in the root directory.  */    
    return_if_fail( status == FX_NO_MORE_SPACE);
    
    /* Now test I/O error on directory entry write inside of the rename processing.  */
    fx_media_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    _fx_utility_logical_sector_write_error_request =  1;
    status =  fx_file_rename(&ram_disk, "new name", "sname");
    _fx_utility_logical_sector_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);
    
    printf("SUCCESS!\n");
    test_control_return(0);
}
