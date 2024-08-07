/* This FileX test concentrates on the basic directory naming operations.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_directory.h"
#include   "fx_utility.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"

#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              16*128


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD                ftest_0;
#endif
static FX_MEDIA                 ram_disk;
static CHAR                     max_name[FX_MAX_LONG_NAME_LEN+2];
static CHAR                     return_name[FX_MAX_LONG_NAME_LEN+1];


/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *ram_disk_memory;
static UCHAR                  *cache_buffer;
#else
static UCHAR                  cache_buffer[CACHE_SIZE];
#endif


/* Define thread prototypes.  */

void    filex_directory_naming_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_directory_naming_application_define(void *first_unused_memory)
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
CHAR        *path_ptr;
CHAR        special_name[10];
UINT        i;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Directory naming test..................................");

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
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(1);
    }
    
    /* Try to rename a directory before the media is opened to generate an error */
    status = fx_directory_rename(&ram_disk, "/A0", "/A1");
    if (status != FX_MEDIA_NOT_OPEN)
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

    /* Get the current path. This should be / or NULL.   */
    status =  fx_directory_default_get(&ram_disk, &path_ptr);

    /* Check the status.  */
    if ((status != FX_SUCCESS) || (path_ptr[0]))
    {

        /* Error getting the path.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(4);
    }
    
    /* Test a directory name that does not exist to generate an error */
/*    status = fx_directory_name_test(&ram_disk, "/A0");
    if (status == FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(5);
    }
*/
    
    /* test some directory names to test the directory search function */
/*    status  = fx_directory_create(&ram_disk, "\\TEST");
    status += fx_directory_name_test(&ram_disk, "\\TEST");
    status += fx_directory_delete(&ram_disk, "\\TEST");
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(6);
    }
*/
    

    /* Create a series of directories...  */
    status =   fx_directory_create(&ram_disk, "/A0");
    status +=  fx_directory_create(&ram_disk, "/b0");
    status +=  fx_directory_create(&ram_disk, "/C0");
    status +=  fx_directory_create(&ram_disk, "/d0");

    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error creating directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(7);
    }

    /* Attempt to create the same directory again.  */
    status =   fx_directory_create(&ram_disk, "/a0");

    /* Check for errors...  */
    if (status != FX_ALREADY_CREATED)
    {

        /* Error creating same directory twice.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(8);
    }

    /* Create sub-directories with the E5 character.   */
    special_name[0] =  (CHAR)0xE5;
    special_name[1] =  0x2E;
    special_name[2] =  0x54;
    special_name[3] =  0x58;
    special_name[4] =  0x54;
    special_name[5] =  0;
    status =  fx_directory_create(&ram_disk, special_name);
    status += fx_media_flush(&ram_disk);
    status += fx_directory_delete(&ram_disk, special_name);

    /* Check for errors.  */
    if (status != FX_SUCCESS)
    {

        /* Error creating special name.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(9);
    } 

    /* Create another special file name.  */
    special_name[0] =  (CHAR)0xE5;
    special_name[1] =  0x31;
    special_name[2] =  0x2E;
    special_name[3] =  0x54;
    special_name[4] =  0x58;
    special_name[5] =  0x54;
    special_name[6] =  0;
    status =  fx_directory_create(&ram_disk, special_name);
    status += fx_media_flush(&ram_disk);
    status += fx_directory_delete(&ram_disk, special_name);

    /* Check for errors.  */
    if (status != FX_SUCCESS)
    {

        /* Error creating special name.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(10);
    } 

    /* Create the next level of sub-directories.... with the interesting names...  */
    status =   fx_directory_default_set(&ram_disk, "/A0");

    status =   fx_directory_create(&ram_disk, "a");
    status +=  fx_directory_create(&ram_disk, "this...ismytestdir");
    status +=  fx_directory_create(&ram_disk, "this ... is my test dir");
    status +=  fx_directory_create(&ram_disk, "ThisIsTheFirstVeryLongDirNameForThisTestABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZfirstfile111111111111111100000000000000000000000000000000000000000000000000000000");
    status +=  fx_directory_create(&ram_disk, "       c");
    status +=  fx_directory_create(&ram_disk, "       c/    e");
    status +=  fx_directory_create(&ram_disk, "       c/    e/    f");
    
    /* Check for errors.  */
    if (status != FX_SUCCESS)
    {

        /* Error creating interesting directory names.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(11);
    } 

    /* Now attempt to create a file in each of these sub-directories.  */
    status =   fx_file_create(&ram_disk, "/A0/a/test.txt");
    status +=  fx_file_create(&ram_disk, "/A0/this...ismytestdir/test.txt");
    status +=  fx_file_create(&ram_disk, "/A0/this ... is my test dir/test.txt");
    status +=  fx_file_create(&ram_disk, "/A0/ThisIsTheFirstVeryLongDirNameForThisTestABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZfirstfile111111111111111100000000000000000000000000000000000000000000000000000000/test.txt");
    fx_media_flush(&ram_disk);
    status +=  fx_file_create(&ram_disk, "/A0    /    c/    e/    f/test.txt");
    fx_media_flush(&ram_disk);
    
    /* Check for errors.  */
    if (status != FX_SUCCESS)
    {

        /* Error creating test files in the interesting sub-directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(12);
    } 

    /* Now attempt to delete the file in each of these sub-directories.  */
    status =   fx_file_delete(&ram_disk, "/A0/a/test.txt");
    status +=  fx_file_delete(&ram_disk, "/A0/this...ismytestdir/test.txt");
    status +=  fx_file_delete(&ram_disk, "/A0/this ... is my test dir/test.txt");
    status +=  fx_file_delete(&ram_disk, "/A0/ThisIsTheFirstVeryLongDirNameForThisTestABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZfirstfile111111111111111100000000000000000000000000000000000000000000000000000000/test.txt");
    status +=  fx_file_delete(&ram_disk, "/A0    /    c/    e/    f/test.txt");

    /* Check for errors.  */
    if (status != FX_SUCCESS)
    {

        /* Error deleting test files in the interesting sub-directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(13);
    } 

    /* Now attempt to delete the interesting sub-directories.  */
    status =   fx_directory_delete(&ram_disk, "/A0/a");
    status +=  fx_directory_delete(&ram_disk, "/A0/this...ismytestdir");
    status +=  fx_directory_delete(&ram_disk, "/A0/this ... is my test dir");
    status +=  fx_directory_delete(&ram_disk, "/A0/ThisIsTheFirstVeryLongDirNameForThisTestABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZfirstfile111111111111111100000000000000000000000000000000000000000000000000000000");
    status +=  fx_directory_delete(&ram_disk, "/A0/c/e/f");
    status +=  fx_directory_delete(&ram_disk, "/A0/c/e");
    status +=  fx_directory_delete(&ram_disk, "/A0/c");

    /* Check for errors.  */
    if (status != FX_SUCCESS)
    {

        /* Error deleting the interesting sub-directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(14);
    } 

    /* Delete a series of directories...  Use short name for b0 and d0!  */
    status =   fx_directory_delete(&ram_disk, "/A0");
    status +=  fx_directory_delete(&ram_disk, "/B0");
    status +=  fx_directory_delete(&ram_disk, "/C0");
    status +=  fx_directory_delete(&ram_disk, "/D0");

    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error deleting directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(15);
    }

    /* Set default to root directory.  */
    status =   fx_directory_default_set(&ram_disk, "/");

    /* Flush the media... should be empty at this point.  */
    status += fx_media_flush(&ram_disk);

    /* Now fill the media's root directory until we get an error...  We will use 8.3 names 
       to attempt to create 32 names.    */
    status +=  fx_directory_create(&ram_disk, "A01");
    status +=  fx_directory_create(&ram_disk, "A02");
    status +=  fx_directory_create(&ram_disk, "A03");
    status +=  fx_directory_create(&ram_disk, "A04");
    status +=  fx_directory_create(&ram_disk, "A05");
    status +=  fx_directory_create(&ram_disk, "A06");
    status +=  fx_directory_create(&ram_disk, "A07");
    status +=  fx_directory_create(&ram_disk, "A08");
    status +=  fx_directory_create(&ram_disk, "A09");
    status +=  fx_directory_create(&ram_disk, "A10");
    status +=  fx_directory_create(&ram_disk, "A11");
    status +=  fx_directory_create(&ram_disk, "A12");
    status +=  fx_directory_create(&ram_disk, "A13");
    status +=  fx_directory_create(&ram_disk, "A14");
    status +=  fx_directory_create(&ram_disk, "A15");
    status +=  fx_directory_create(&ram_disk, "A16");
    status +=  fx_directory_create(&ram_disk, "A17");
    status +=  fx_directory_create(&ram_disk, "A18");
    status +=  fx_directory_create(&ram_disk, "A19");
    status +=  fx_directory_create(&ram_disk, "A20");
    status +=  fx_directory_create(&ram_disk, "A21");
    status +=  fx_directory_create(&ram_disk, "A22");
    status +=  fx_directory_create(&ram_disk, "A23");
    status +=  fx_directory_create(&ram_disk, "A24");
    status +=  fx_directory_create(&ram_disk, "A25");
    status +=  fx_directory_create(&ram_disk, "A26");
    status +=  fx_directory_create(&ram_disk, "A27");
    status +=  fx_directory_create(&ram_disk, "A28");
    status +=  fx_directory_create(&ram_disk, "A29");
    status +=  fx_directory_create(&ram_disk, "A30");
    status +=  fx_directory_create(&ram_disk, "A31");
    status +=  fx_directory_create(&ram_disk, "A32");

fx_media_flush(&ram_disk);

    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error creating directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(16);
    }

    /* Attempt to create 33rd name... this should fail.  */
    status =  fx_directory_create(&ram_disk, "A33");

    /* Check for errors...  */
    if (status != FX_NO_MORE_SPACE)
    {

        /* Error creating directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(17);
    }

    /* Flush the media.  */
    status =  fx_media_flush(&ram_disk);

    /* Now delete all the names.  */
    status +=  fx_directory_delete(&ram_disk, "A01");
    status +=  fx_directory_delete(&ram_disk, "A02");
    status +=  fx_directory_delete(&ram_disk, "A03");
    status +=  fx_directory_delete(&ram_disk, "A04");
    status +=  fx_directory_delete(&ram_disk, "A05");
    status +=  fx_directory_delete(&ram_disk, "A06");
    status +=  fx_directory_delete(&ram_disk, "A07");
    status +=  fx_directory_delete(&ram_disk, "A08");
    status +=  fx_directory_delete(&ram_disk, "A09");
    status +=  fx_directory_delete(&ram_disk, "A10");
    status +=  fx_directory_delete(&ram_disk, "A11");
    status +=  fx_directory_delete(&ram_disk, "A12");
    status +=  fx_directory_delete(&ram_disk, "A13");
    status +=  fx_directory_delete(&ram_disk, "A14");
    status +=  fx_directory_delete(&ram_disk, "A15");
    status +=  fx_directory_delete(&ram_disk, "A16");
    status +=  fx_directory_delete(&ram_disk, "A17");
    status +=  fx_directory_delete(&ram_disk, "A18");
    status +=  fx_directory_delete(&ram_disk, "A19");
    status +=  fx_directory_delete(&ram_disk, "A20");
    status +=  fx_directory_delete(&ram_disk, "A21");
    status +=  fx_directory_delete(&ram_disk, "A22");
    status +=  fx_directory_delete(&ram_disk, "A23");
    status +=  fx_directory_delete(&ram_disk, "A24");
    status +=  fx_directory_delete(&ram_disk, "A25");
    status +=  fx_directory_delete(&ram_disk, "A26");
    status +=  fx_directory_delete(&ram_disk, "A27");
    status +=  fx_directory_delete(&ram_disk, "A28");
    status +=  fx_directory_delete(&ram_disk, "A29");
    status +=  fx_directory_delete(&ram_disk, "A30");
    status +=  fx_directory_delete(&ram_disk, "A31");
    status +=  fx_directory_delete(&ram_disk, "A32");
    
    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error deleting directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(18);
    }

    /* Flush the media.  */
    status =  fx_media_flush(&ram_disk);
    
    /* Now do the same thing, except with 2 entry long names.  */
    status +=  fx_directory_create(&ram_disk, "b01");
    status +=  fx_directory_create(&ram_disk, "b02");
    status +=  fx_directory_create(&ram_disk, "b03");
    status +=  fx_directory_create(&ram_disk, "b04");
    status +=  fx_directory_create(&ram_disk, "b05");
    status +=  fx_directory_create(&ram_disk, "b06");
    status +=  fx_directory_create(&ram_disk, "b07");
    status +=  fx_directory_create(&ram_disk, "b08");
    status +=  fx_directory_create(&ram_disk, "b09");
    status +=  fx_directory_create(&ram_disk, "b10");
    status +=  fx_directory_create(&ram_disk, "b11");
    status +=  fx_directory_create(&ram_disk, "b12");
    status +=  fx_directory_create(&ram_disk, "b13");
    status +=  fx_directory_create(&ram_disk, "b14");
    status +=  fx_directory_create(&ram_disk, "b15");
    status +=  fx_directory_create(&ram_disk, "b16");

    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error creating directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(19);
    }

    /* Attempt to create 17th long name... this should fail.  */
    status =  fx_directory_create(&ram_disk, "b17");

    /* Check for errors...  */
    if (status != FX_NO_MORE_SPACE)
    {

        /* Error creating directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(20);
    }

    /* Flush the media.  */
    status =  fx_media_flush(&ram_disk);

    /* Now delete all the names.  */
    status +=  fx_directory_delete(&ram_disk, "b01");
    status +=  fx_directory_delete(&ram_disk, "b02");
    status +=  fx_directory_delete(&ram_disk, "b03");
    status +=  fx_directory_delete(&ram_disk, "b04");
    status +=  fx_directory_delete(&ram_disk, "b05");
    status +=  fx_directory_delete(&ram_disk, "b06");
    status +=  fx_directory_delete(&ram_disk, "b07");
    status +=  fx_directory_delete(&ram_disk, "b08");
    status +=  fx_directory_delete(&ram_disk, "b09");
    status +=  fx_directory_delete(&ram_disk, "b10");
    status +=  fx_directory_delete(&ram_disk, "b11");
    status +=  fx_directory_delete(&ram_disk, "b12");
    status +=  fx_directory_delete(&ram_disk, "b13");
    status +=  fx_directory_delete(&ram_disk, "b14");
    status +=  fx_directory_delete(&ram_disk, "b15");
    status +=  fx_directory_delete(&ram_disk, "b16");
    
    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error deleting directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(21);
    }
    
#if 0
    /* test the directory free search function for proper error handing with invalid names */
    FX_DIR_ENTRY dir_entry;
    dir_entry.fx_dir_entry_name = (char*) malloc(0x100 * sizeof(char));
    dir_entry.fx_dir_entry_name[0] = '.';
    dir_entry.fx_dir_entry_name[1] = '.';
    dir_entry.fx_dir_entry_name[2] = (char)0;
    status = _fx_directory_free_search(&ram_disk, &dir_entry, &dir_entry);
    if (status != FX_INVALID_NAME)
    {
        printf("ERROR!\n");
        test_control_return(22);
    }
    
    /* test the directory free search function for proper error handling of special characters */
/* This code is executing differently on local vs server. Disabled until cause is explored */
    dir_entry.fx_dir_entry_name[0] = (char)128;
    dir_entry.fx_dir_entry_name[1] = (char)128;
    dir_entry.fx_dir_entry_name[2] = (char)0;
    status = _fx_directory_free_search(&ram_disk, &dir_entry, &dir_entry);
    if (status == FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(23);
    }
    
    /* test the directory free search function for proper error handling of special characters */
    dir_entry.fx_dir_entry_name[0] = '%';
    dir_entry.fx_dir_entry_name[1] = '%';
    dir_entry.fx_dir_entry_name[2] = (char)0;
    status = _fx_directory_free_search(&ram_disk, &dir_entry, &dir_entry);
    if (status == FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(24);
    }
    
    /* test the directory free search function for proper error handling of special characters */
    dir_entry.fx_dir_entry_name[0] = ']';
    dir_entry.fx_dir_entry_name[1] = ']';
    dir_entry.fx_dir_entry_name[2] = (char)0;
    status = _fx_directory_free_search(&ram_disk, &dir_entry, &dir_entry);
    if (status == FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(25);
    }
#endif
    
    /* Create a directory.  */
    status =  fx_directory_create(&ram_disk, "b16");

    status += fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }

    /* Create an I/O error for the directory search inside of fx_directory_name_test.   */
    _fx_utility_logical_sector_read_error_request =  1;
    status =  fx_directory_name_test(&ram_disk, "b16");
    _fx_utility_logical_sector_read_error_request =  0;
    
    /* Check for the I/O error.  */
    if (status != FX_IO_ERROR)
    {
    
        printf("ERROR!\n");
        test_control_return(26);
    }

    /* Test the directory name.  */
    _fx_directory_name_extract("\\", return_name);
    
    /* Check for a good return name.  */
    if (return_name[0] != 0)
    {
   
        printf("ERROR!\n");
        test_control_return(27);
    }

    /* Now test the maximum length of a name.  */
    for (i = 0; i < FX_MAX_LONG_NAME_LEN+1; i++)
    {
        max_name[i] = 'a';
    }

    /* Test for maximum size... and removal of blank spaces.  */
    _fx_directory_name_extract(max_name, return_name);

    /* Was the name truncated?  */
    if (return_name[FX_MAX_LONG_NAME_LEN - 1] != 0)
    {
   
        printf("ERROR!\n");
        test_control_return(28);
    }
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(29);
    }
    else
    {

        printf("SUCCESS!\n");
        test_control_return(0);
    }
}

