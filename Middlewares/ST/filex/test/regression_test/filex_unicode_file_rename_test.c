/* This FileX test concentrates on the unicode file rename operations.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_ram_driver_test.h"
#include   <stdio.h>

#define     DEMO_STACK_SIZE         8192
#define     CACHE_SIZE              16*128

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
static UCHAR                     cache_buffer[CACHE_SIZE];
#endif
/* Notice that old_file_name is the same as new_file_name1.  */
static UCHAR                    old_file_name[]  =  {3, 0, 4, 0, 5, 0, 6, 0, 0, 0}; 
static UCHAR                    new_file_name1[] =  {3, 0, 4, 0, 5, 0, 6, 0, 0, 0};
static UCHAR                    new_file_name2[] =  {3, 0, 4, 0, 5, 0, 0, 0};
static UCHAR                    new_file_name3[] =  {3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 0, 0};
static UCHAR                    new_file_name4[] =  {2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 15, 0, 0, 0}; 
static UCHAR                    new_file_name5[] =  {'a', 0, 'b', 0, 'c', 0, 0, 0}; 
static UCHAR                    new_file_name5_bak[] =  {'a', 0, 'b', 0, 'c', 0, 0, 0}; 
static UCHAR                    new_file_name6[] =  {'a', 0, 'y', 0, 'c', 0, 0, 0}; 
static UCHAR                    new_file_name7[] =  {'a', 1, 'z', 0, 'c', 0, 0, 0}; 
static UCHAR                    new_file_name8[] =  {'z' + 1, 0, 'z', 0, 'c', 0, 'd', 0, 0, 0}; 
static UCHAR                    new_file_name9[] =  {'z' + 1, 1, 'z', 0, 'c', 0, 'd', 0, 0, 0}; 
static UCHAR                    destination_name[100];

#ifdef FX_ENABLE_EXFAT
#define TEST_COUNT              3   /* exFAT does not yet support Unicode.  */
#else              
#define TEST_COUNT              3
#endif

/* Define thread prototypes.  */

void    filex_unicode_file_rename_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_unicode_file_rename_application_define(void *first_unused_memory)
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

UINT        i;
UINT        status, count;
ULONG       length;
ULONG       old_length; 
ULONG       new_length;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Unicode File Rename Test...............................");
                      
    /* Loop to test FAT 12, 16, 32 and exFAT.   */
    for (i = 0; i < TEST_COUNT; i ++)
    {
        if (i == 0)
        {
            /* Format the media with FAT12.  This needs to be done before opening it!  */
            status =  fx_media_format(&ram_disk, 
                                     _fx_ram_driver,         // Driver entry
                                     ram_disk_memory_large,  // RAM disk memory pointer
                                     cache_buffer,           // Media buffer pointer
                                     CACHE_SIZE,             // Media buffer size 
                                     "MY_RAM_DISK",          // Volume Name
                                     1,                      // Number of FATs
                                     32,                     // Directory Entries
                                     0,                      // Hidden sectors
                                     256,                    // Total sectors 
                                     256,                    // Sector size   
                                     8,                      // Sectors per cluster
                                     1,                      // Heads
                                     1);                     // Sectors per track 
        }     
        else if (i == 1)
        {
            /* Format the media with FAT16.  This needs to be done before opening it!  */
            status =  fx_media_format(&ram_disk, 
                                     _fx_ram_driver,         // Driver entry            
                                     ram_disk_memory_large,  // RAM disk memory pointer
                                     cache_buffer,           // Media buffer pointer
                                     CACHE_SIZE,             // Media buffer size 
                                     "MY_RAM_DISK",          // Volume Name
                                     1,                      // Number of FATs
                                     32,                     // Directory Entries
                                     0,                      // Hidden sectors
                                     4200 * 8,               // Total sectors 
                                     256,                    // Sector size   
                                     8,                      // Sectors per cluster
                                     1,                      // Heads
                                     1);                     // Sectors per track 
        }  
        else if (i == 2)
        {
            /* Format the media with FAT32.  This needs to be done before opening it!  */
            status =  fx_media_format(&ram_disk, 
                                     _fx_ram_driver,         // Driver entry            
                                     ram_disk_memory_large,  // RAM disk memory pointer
                                     cache_buffer,           // Media buffer pointer
                                     CACHE_SIZE,             // Media buffer size 
                                     "MY_RAM_DISK",          // Volume Name
                                     1,                      // Number of FATs
                                     32,                     // Directory Entries
                                     0,                      // Hidden sectors
                                     70000 * 8,              // Total sectors 
                                     256,                    // Sector size   
                                     8,                      // Sectors per cluster
                                     1,                      // Heads
                                     1);                     // Sectors per track 
        }  
#ifdef FX_ENABLE_EXFAT
        else
        {

            /* Format the media with exFAT.  This needs to be done before opening it!  */
            status =  fx_media_exFAT_format(&ram_disk, 
                                            _fx_ram_driver,         // Driver entry            
                                            ram_disk_memory_large,  // RAM disk memory pointer
                                            cache_buffer,           // Media buffer pointer
                                            CACHE_SIZE,             // Media buffer size 
                                            "MY_RAM_DISK",          // Volume Name
                                            1,                      // Number of FATs
                                            0,                      // Hidden sectors
                                            256,                    // Total sectors 
                                            512,                   // Sector size   
                                            4,                      // exFAT Sectors per cluster
                                            12345,                  // Volume ID
                                            0);                     // Boundary unit
        }
#endif

        /* Determine if the format had an error.  */
        if (status)
        {

            printf("ERROR!\n");
            test_control_return(1);
        }
        
        /* Attempt to rename a file before the media has been opened */
        status = fx_unicode_file_rename(&ram_disk, (UCHAR *)"name", 1, (UCHAR *)"name", 1, (CHAR *) destination_name);
        if (status != FX_MEDIA_NOT_OPEN)
        {
            printf("ERROR!\n");
            test_control_return(2);
        }

        /* Open the ram_disk.  */
        status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);

        /* Check the status.  */
        if (status != FX_SUCCESS)
        {

            /* Error, return error code.  */
            printf("ERROR!\n");
            test_control_return(3);
        }

        /* Attempt to rename a file that does not exist */
        status = fx_unicode_file_rename(&ram_disk, (UCHAR *)"does_not_exist", 1, (UCHAR *)"does_not_exist", 1, (CHAR *) destination_name);
        if (status == FX_SUCCESS)
        {
            printf("ERROR!\n");
            test_control_return(4);
        }

        /* Create the unicode file name to create the same unicode file names in the sub directory.  */
        length =  fx_unicode_length_get(old_file_name);
        status =  fx_unicode_file_create(&ram_disk, old_file_name, length, (CHAR *) destination_name);

        /* Check for expected error.  */
        if (status)
        {

            /* Error creating unicode file names.  Return to caller.  */
            printf("ERROR!\n");
            test_control_return(5);
        }

        /* Rename the old file name to new_file_name1.  */
        old_length =  fx_unicode_length_get(old_file_name);
        new_length =  fx_unicode_length_get(new_file_name1);
        status =  fx_unicode_file_rename(&ram_disk, old_file_name, old_length, new_file_name1, new_length, (CHAR *) destination_name);

        /* Check for expected error.  */
        if (status)
        {

            /* Error creating unicode file names.  Return to caller.  */
            printf("ERROR!\n");
            test_control_return(6);
        }

        /* Rename the new_file_name1 to new_file_name2.  */
        old_length =  fx_unicode_length_get(new_file_name1);
        new_length =  fx_unicode_length_get(new_file_name2);
        status =  fx_unicode_file_rename(&ram_disk, new_file_name1, old_length, new_file_name2, new_length, (CHAR *) destination_name);

        /* Check for expected error.  */
        if (status)
        {

            /* Error creating unicode file names.  Return to caller.  */
            printf("ERROR!\n");
            test_control_return(7);
        }

        /* Rename the new_file_name2 to new_file_name3.  */
        old_length =  fx_unicode_length_get(new_file_name2);
        new_length =  fx_unicode_length_get(new_file_name3);
        status =  fx_unicode_file_rename(&ram_disk, new_file_name2, old_length, new_file_name3, new_length, (CHAR *) destination_name);

        /* Check for expected error.  */
        if (status)
        {

            /* Error creating unicode file names.  Return to caller.  */
            printf("ERROR!\n");
            test_control_return(8);
        }

        /* Rename the new_file_name3 to new_file_name4.  */
        old_length =  fx_unicode_length_get(new_file_name3);
        new_length =  fx_unicode_length_get(new_file_name4);
        status =  fx_unicode_file_rename(&ram_disk, new_file_name3, old_length, new_file_name4, new_length, (CHAR *) destination_name);

        /* Check for expected error.  */
        if (status)
        {

            /* Error creating unicode file names.  Return to caller.  */
            printf("ERROR!\n");
            test_control_return(9);
        }

        /* Attempt to create the old_file_name again.  */  
        length =  fx_unicode_length_get(old_file_name);
        status =  fx_unicode_file_create(&ram_disk,  old_file_name, length, (CHAR *) destination_name);

        /* Check for expected error.  */
        if (status)
        {

            /* Error creating unicode file name.  Return to caller.  */
            printf("ERROR!\n");
            test_control_return(10);
        }

        /* Attempt to create the new_file_name1 same as old_file_name again.  */  
        length =  fx_unicode_length_get(new_file_name1);
        status =  fx_unicode_file_create(&ram_disk,  new_file_name1, length, (CHAR *) destination_name);

        /* Check for expected error.  */
        if (status != FX_ALREADY_CREATED)
        {

            /* Error creating unicode directory name.  Return to caller.  */
            printf("ERROR!\n");
            test_control_return(11);
        }


        /* Attempt to create the new_file_name2 again.  */  
        length =  fx_unicode_length_get(new_file_name2);
        status =  fx_unicode_file_create(&ram_disk,  new_file_name2, length, (CHAR *) destination_name);

        /* Check for expected error.  */
        if (status)
        {

            /* Error creating unicode directory name.  Return to caller.  */
            printf("ERROR!\n");
            test_control_return(12);
        }

        /* Attempt to create the new_file_name1 again.  */  
        length =  fx_unicode_length_get(new_file_name3);
        status =  fx_unicode_file_create(&ram_disk,  new_file_name3, length, (CHAR *) destination_name);

        /* Check for expected error.  */
        if (status)
        {

            /* Error creating unicode directory name.  Return to caller.  */
            printf("ERROR!\n");
            test_control_return(13);
        }

        /* Attempt to create the new_file_name4 again.  */  
        length =  fx_unicode_length_get(new_file_name4);
        status =  fx_unicode_file_create(&ram_disk,  new_file_name4, length, (CHAR *) destination_name);

        /* Check for expected error.  */
        if (status != FX_ALREADY_CREATED)
        {

            /* Error creating unicode directory name.  Return to caller.  */
            printf("ERROR!\n");
            test_control_return(14);
        }
        
#ifndef FX_DISABLE_ERROR_CHECKING
        /* send a null pointer to generate an error */
        status = fx_unicode_file_rename(FX_NULL, old_file_name, old_length, new_file_name1, new_length, (CHAR *) destination_name);
        if (status != FX_PTR_ERROR)
        {
            printf("ERROR!\n");
            test_control_return(15);
        }
#endif /* FX_DISABLE_ERROR_CHECKING */

        /* Create a file whose name is consists of lowcase characters. */
        length = fx_unicode_length_get(new_file_name5);
        status = fx_unicode_file_create(&ram_disk, new_file_name5, length, (CHAR *)destination_name);

        /* Check for expected error. */
        return_value_if_fail( status == FX_SUCCESS, 16);

        /* Rename as the same file name. */
        length = fx_unicode_length_get(new_file_name5);
        status = fx_unicode_file_rename(&ram_disk, new_file_name5, length, new_file_name5, length, (CHAR *)destination_name);

        /* Check for expected error. */
        return_value_if_fail( status == FX_SUCCESS, 17);

        length = fx_unicode_length_get(new_file_name6);
        status = fx_unicode_file_create(&ram_disk, new_file_name6, length, (CHAR *)destination_name);

        /* Check for expected error. */
        return_value_if_fail( status == FX_SUCCESS, 18);

        /* Attempt to  rename new_file_name6 as existed new_file_name5 */
        old_length = fx_unicode_length_get(new_file_name6);
        new_length = fx_unicode_length_get(new_file_name5);
        status = fx_unicode_file_rename(&ram_disk, new_file_name6, old_length, new_file_name5, new_length, (CHAR *)destination_name);

        /* Check for expected error. */
        return_value_if_fail( status == FX_ALREADY_CREATED, 19);

        /* Attempt to  rename new_file_name6 as new_file_name7 */
        old_length = fx_unicode_length_get(new_file_name6);
        new_length = fx_unicode_length_get(new_file_name7);
        status = fx_unicode_file_rename(&ram_disk, new_file_name6, old_length, new_file_name7, new_length, (CHAR *)destination_name);

        /* Check for expected error. */
        return_value_if_fail( status == FX_SUCCESS, 20);

        /* Attempt to  rename new_file_name5 as exised new_file_name7 */
        old_length = fx_unicode_length_get(new_file_name5);
        new_length = fx_unicode_length_get(new_file_name7);
        status = fx_unicode_file_rename(&ram_disk, new_file_name5, old_length, new_file_name7, new_length, (CHAR *)destination_name);

        /* Check for expected error. */
        return_value_if_fail( status == FX_ALREADY_CREATED, 21);

        /* Attempt to  rename new_file_name7 as itself. */
        old_length = fx_unicode_length_get(new_file_name7);
        new_length = fx_unicode_length_get(new_file_name7);
        status = fx_unicode_file_rename(&ram_disk, new_file_name7, old_length, new_file_name7, new_length, (CHAR *)destination_name);

        /* Check for expected error. */
        return_value_if_fail( status == FX_SUCCESS, 22);

        /* Created  two files whose name's first byte is 'z'+1. */
        length = fx_unicode_length_get(new_file_name8);
        status = fx_unicode_file_create(&ram_disk, new_file_name8, length, (CHAR *)destination_name);
        length = fx_unicode_length_get(new_file_name9);
        status += fx_unicode_file_create(&ram_disk, new_file_name9, length, (CHAR *)destination_name);
        return_value_if_fail( status == FX_SUCCESS, 23);

        /* Attempt to rename new_file_name9 as existed new_file_name8 */
        old_length = fx_unicode_length_get(new_file_name9);
        new_length = fx_unicode_length_get(new_file_name8);
        status = fx_unicode_file_rename(&ram_disk, new_file_name9, old_length, new_file_name8, new_length, (CHAR *)destination_name);
        return_value_if_fail( status == FX_ALREADY_CREATED, 24);

        /* Attempt to rename new_file_name9 as existed new_file_name7 */
        old_length = fx_unicode_length_get(new_file_name9);
        new_length = fx_unicode_length_get(new_file_name7);
        status = fx_unicode_file_rename(&ram_disk, new_file_name9, old_length, new_file_name7, new_length, (CHAR *)destination_name);
        return_value_if_fail( status == FX_ALREADY_CREATED, 25);

        /* Attempt to create a lot of directories with similar names to overflow. */
        length = fx_unicode_length_get(new_file_name5);
        for ( count = 0; count < 27; count++)
        {
            new_file_name5[0]++;
            status = fx_unicode_directory_create(&ram_disk, new_file_name5, length, (CHAR *)destination_name);
            return_value_if_fail( (status == FX_SUCCESS) || (status == FX_ALREADY_CREATED) || (status == FX_NO_MORE_SPACE), 24 + count);
        }
        old_length = fx_unicode_length_get(new_file_name5_bak);
        new_length = fx_unicode_length_get(new_file_name5);
        status = fx_unicode_directory_rename(&ram_disk, new_file_name5_bak, old_length, new_file_name5, new_length, (CHAR *)destination_name);
        return_value_if_fail( (status == FX_NOT_FOUND) || ( status == FX_NOT_DIRECTORY), 53);

        /* Close the media.  */
        status =  fx_media_close(&ram_disk);

        /* Determine if the test was successful.  */
        return_value_if_fail( status == FX_SUCCESS, 51);
    }

    printf("SUCCESS!\n");
    test_control_return(0);
}
