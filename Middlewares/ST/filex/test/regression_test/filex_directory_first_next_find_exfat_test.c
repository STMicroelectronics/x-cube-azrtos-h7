/* This FileX test concentrates on the basic first/next entry find operations.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"

#ifdef FX_ENABLE_EXFAT

#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              FX_EXFAT_SECTOR_SIZE


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
static UCHAR                    cache_buffer[CACHE_SIZE];
#endif
static CHAR                     name[256];


/* Define thread prototypes.  */

void    filex_directory_first_next_find_exfat_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_directory_first_next_find_exfat_application_define(void *first_unused_memory)
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
UINT        attributes;
ULONG       size;
UINT        year;
UINT        month;
UINT        day;
UINT        hour;
UINT        minute;
UINT        second;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   exFAT Directory first/next entry find test.............");

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_exFAT_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            0,                      // Hidden sectors
                            1024,                   // Total sectors 
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

    /* Create a hiearchy of files and sub-directories.   */
    status =   fx_file_create(&ram_disk,      "RootFile01");
    status +=  fx_file_create(&ram_disk,      "RootFile02");
    status +=  fx_directory_create(&ram_disk, "RootDir03");
    status +=  fx_directory_create(&ram_disk, "RootDir04");
    status +=  fx_directory_create(&ram_disk, "RootDir05");
    status +=  fx_directory_create(&ram_disk, "RootDir06");
    status +=  fx_directory_create(&ram_disk, "RootDir07");
    status +=  fx_directory_create(&ram_disk, "RootDir08");
    status +=  fx_file_create(&ram_disk,      "RootFile09");
    status +=  fx_file_create(&ram_disk,      "RootFile10");
    status +=  fx_directory_create(&ram_disk, "RootDir11");
    status +=  fx_directory_create(&ram_disk, "RootDir12");
    status +=  fx_directory_create(&ram_disk, "RootDir13");
    status +=  fx_directory_create(&ram_disk, "RootDir14");
    status +=  fx_file_create(&ram_disk,      "RootFile15");
    
    /* Create the sub-directories for RootDir03.  */
    status +=  fx_directory_create(&ram_disk, "/RootDir03/RootDir03_SubDir01");
    status +=  fx_file_create(&ram_disk,      "/RootDir03/RootDir03_File02");
    status +=  fx_directory_create(&ram_disk, "/RootDir03/RootDir03_SubDir03");
    status +=  fx_file_create(&ram_disk,      "/RootDir03/RootDir03_File04");
    status +=  fx_directory_create(&ram_disk, "/RootDir03/RootDir03_SubDir05");
    status +=  fx_file_create(&ram_disk,      "/RootDir03/RootDir03_File06");
    status +=  fx_directory_create(&ram_disk, "/RootDir03/RootDir03_SubDir07");
    status +=  fx_file_create(&ram_disk,      "/RootDir03/RootDir03_File08");

    /* Create the sub-directories for RootDir04.  */
    status +=  fx_file_create(&ram_disk,      "/RootDir04/RootDir04_File01");
    status +=  fx_file_create(&ram_disk,      "/RootDir04/RootDir04_File02");
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir03");
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir04");
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir05");
    status +=  fx_file_create(&ram_disk,      "/RootDir04/RootDir04_File06");
    status +=  fx_file_create(&ram_disk,      "/RootDir04/RootDir04_File07");
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir08");

    /* Create the sub-directories for RootDir05.  */
    status +=  fx_file_create(&ram_disk,      "/RootDir05/RootDir05_File02");
    status +=  fx_file_create(&ram_disk,      "/RootDir05/RootDir05_File03");
    status +=  fx_file_create(&ram_disk,      "/RootDir05/RootDir05_File04");
    status +=  fx_directory_create(&ram_disk, "/RootDir05/RootDir05_SubDir05");
    status +=  fx_directory_create(&ram_disk, "/RootDir05/RootDir05_SubDir06");
    status +=  fx_directory_create(&ram_disk, "/RootDir05/RootDir05_SubDir07");
    status +=  fx_file_create(&ram_disk,      "/RootDir05/RootDir05_File08");
    status +=  fx_directory_create(&ram_disk, "/RootDir05/RootDir05_SubDir09");

    /* Create the sub-directories for RootDir06.  */
    status +=  fx_directory_create(&ram_disk, "/RootDir06/RootDir06_SubDir01");
    status +=  fx_directory_create(&ram_disk, "/RootDir06/RootDir06_SubDir02");
    status +=  fx_directory_create(&ram_disk, "/RootDir06/RootDir06_SubDir03");
    status +=  fx_file_create(&ram_disk,      "/RootDir06/RootDir06_File04");
    status +=  fx_file_create(&ram_disk,      "/RootDir06/RootDir06_File05");
    status +=  fx_file_create(&ram_disk,      "/RootDir06/RootDir06_File06");
    status +=  fx_file_create(&ram_disk,      "/RootDir06/RootDir06_File07");
    status +=  fx_directory_create(&ram_disk, "/RootDir06/RootDir06_SubDir08");

    /* Create the sub-directories for RootDir07.  */
    status +=  fx_directory_create(&ram_disk, "/RootDir07/RootDir07_SubDir01");
    status +=  fx_directory_create(&ram_disk, "/RootDir07/RootDir07_SubDir02");
    status +=  fx_directory_create(&ram_disk, "/RootDir07/RootDir07_SubDir03");
    status +=  fx_directory_create(&ram_disk, "/RootDir07/RootDir07_SubDir04");
    status +=  fx_file_create(&ram_disk,      "/RootDir07/RootDir07_File05");
    status +=  fx_file_create(&ram_disk,      "/RootDir07/RootDir07_File06");
    status +=  fx_file_create(&ram_disk,      "/RootDir07/RootDir07_File07");
    status +=  fx_file_create(&ram_disk,      "/RootDir07/RootDir07_File08");

    /* Create the sub-directories for RootDir08.  */
    status +=  fx_file_create(&ram_disk,      "/RootDir08/RootDir08_File01");
    status +=  fx_file_create(&ram_disk,      "/RootDir08/RootDir08_File02");
    status +=  fx_file_create(&ram_disk,      "/RootDir08/RootDir08_File03");
    status +=  fx_file_create(&ram_disk,      "/RootDir08/RootDir08_File04");
    status +=  fx_directory_create(&ram_disk, "/RootDir08/RootDir08_SubDir05");
    status +=  fx_directory_create(&ram_disk, "/RootDir08/RootDir08_SubDir06");
    status +=  fx_directory_create(&ram_disk, "/RootDir08/RootDir08_SubDir07");
    status +=  fx_directory_create(&ram_disk, "/RootDir08/RootDir08_SubDir08");

    /* Create the sub-directories for RootDir11.  */
    status +=  fx_file_create(&ram_disk,      "/RootDir11/RootDir11_File01");
    status +=  fx_file_create(&ram_disk,      "/RootDir11/RootDir11_File02");
    status +=  fx_directory_create(&ram_disk, "/RootDir11/RootDir11_SubDir03");
    status +=  fx_directory_create(&ram_disk, "/RootDir11/RootDir11_SubDir04");
    status +=  fx_file_create(&ram_disk,      "/RootDir11/RootDir11_File05");
    status +=  fx_file_create(&ram_disk,      "/RootDir11/RootDir11_File06");
    status +=  fx_directory_create(&ram_disk, "/RootDir11/RootDir11_SubDir07");
    status +=  fx_directory_create(&ram_disk, "/RootDir11/RootDir11_SubDir08");

    /* Create the sub-directories for RootDir12.  */
    status +=  fx_file_create(&ram_disk,      "/RootDir12/RootDir12_File01");
    status +=  fx_file_create(&ram_disk,      "/RootDir12/RootDir12_File02");
    status +=  fx_directory_create(&ram_disk, "/RootDir12/RootDir12_SubDir03");
    status +=  fx_directory_create(&ram_disk, "/RootDir12/RootDir12_SubDir04");
    status +=  fx_file_create(&ram_disk,      "/RootDir12/RootDir12_File05");
    status +=  fx_file_create(&ram_disk,      "/RootDir12/RootDir12_File06");
    status +=  fx_directory_create(&ram_disk, "/RootDir12/RootDir12_SubDir07");
    status +=  fx_directory_create(&ram_disk, "/RootDir12/RootDir12_SubDir08");

    /* Create the sub-directories for RootDir13.  */
    status +=  fx_file_create(&ram_disk,      "/RootDir13/RootDir13_File01");
    status +=  fx_file_create(&ram_disk,      "/RootDir13/RootDir13_File02");
    status +=  fx_directory_create(&ram_disk, "/RootDir13/RootDir13_SubDir03");
    status +=  fx_directory_create(&ram_disk, "/RootDir13/RootDir13_SubDir04");
    status +=  fx_file_create(&ram_disk,      "/RootDir13/RootDir13_File05");
    status +=  fx_file_create(&ram_disk,      "/RootDir13/RootDir13_File06");
    status +=  fx_directory_create(&ram_disk, "/RootDir13/RootDir13_SubDir07");
    status +=  fx_directory_create(&ram_disk, "/RootDir13/RootDir13_SubDir08");

    /* Create the Sub-sub directories under RootDir04.  */
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir03/RootDir04_SubDir03_SubSubDir01");
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir03/RootDir04_SubDir03_SubSubDir02");
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir03/RootDir04_SubDir03_SubSubDir03");
    status +=  fx_file_create(&ram_disk,      "/RootDir04/RootDir04_SubDir03/RootDir04_SubDir03_SubSubFile04");

    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir04/RootDir04_SubDir04_SubSubDir01");
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir04/RootDir04_SubDir04_SubSubDir02");
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir04/RootDir04_SubDir04_SubSubDir03");
    status +=  fx_file_create(&ram_disk,      "/RootDir04/RootDir04_SubDir04/RootDir04_SubDir04_SubSubFile04");

    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir05/RootDir04_SubDir05_SubSubDir01");
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir05/RootDir04_SubDir05_SubSubDir02");
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir05/RootDir04_SubDir05_SubSubDir03");
    status +=  fx_file_create(&ram_disk,      "/RootDir04/RootDir04_SubDir05/RootDir04_SubDir05_SubSubFile04");

    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir08/RootDir04_SubDir08_SubSubDir01");
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir08/RootDir04_SubDir08_SubSubDir02");
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir08/RootDir04_SubDir08_SubSubDir03");
    status +=  fx_file_create(&ram_disk,      "/RootDir04/RootDir04_SubDir05/RootDir04_SubDir08_SubSubFile04");
    
    /* Create the sub-sub-sub directories under RootDir04.  */
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir04/RootDir04_SubDir04_SubSubDir01/RootDir04_SubDir04_SubSubDir01_SubSubSubDir01");
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir04/RootDir04_SubDir04_SubSubDir01/RootDir04_SubDir04_SubSubDir01_SubSubSubDir02");
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir04/RootDir04_SubDir04_SubSubDir01/RootDir04_SubDir04_SubSubDir01_SubSubSubDir03");
    status +=  fx_file_create(&ram_disk,      "/RootDir04/RootDir04_SubDir04/RootDir04_SubDir04_SubSubDir01/RootDir04_SubDir04_SubSubDir01_SubSubSubFile04");

    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir04/RootDir04_SubDir04_SubSubDir02/RootDir04_SubDir04_SubSubDir01_SubSubSubDir01");
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir04/RootDir04_SubDir04_SubSubDir02/RootDir04_SubDir04_SubSubDir01_SubSubSubDir02");
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir04/RootDir04_SubDir04_SubSubDir02/RootDir04_SubDir04_SubSubDir01_SubSubSubDir03");
    status +=  fx_file_create(&ram_disk,      "/RootDir04/RootDir04_SubDir04/RootDir04_SubDir04_SubSubDir02/RootDir04_SubDir04_SubSubDir01_SubSubSubFile04");

    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir04/RootDir04_SubDir04_SubSubDir03/RootDir04_SubDir04_SubSubDir01_SubSubSubDir01");
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir04/RootDir04_SubDir04_SubSubDir03/RootDir04_SubDir04_SubSubDir01_SubSubSubDir02");
    status +=  fx_directory_create(&ram_disk, "/RootDir04/RootDir04_SubDir04/RootDir04_SubDir04_SubSubDir03/RootDir04_SubDir04_SubSubDir01_SubSubSubDir03");
    status +=  fx_file_create(&ram_disk,      "/RootDir04/RootDir04_SubDir04/RootDir04_SubDir04_SubSubDir03/RootDir04_SubDir04_SubSubDir01_SubSubSubFile04");

    /* Get the current path. This should be / or NULL.   */
    status +=  fx_directory_default_get(&ram_disk, &path_ptr);

    /* Check the status.  */
    if ((status != FX_SUCCESS) || (path_ptr[0]))
    {

        /* Error getting the path.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(4);
    }

    /* Set the current path. This should be / or NULL.   */
    status =  fx_directory_default_set(&ram_disk, "/");

    /* Check the status.  */
    if (status != FX_SUCCESS) 
    {

        /* Error setting the path.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(5);
    }

    /* Pickup the first entry in the root directory.  */
    status =  fx_directory_first_entry_find(&ram_disk, name);
    if (strcmp(name, "RootFile01"))
        status++;
    status += fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "RootFile02"))
        status++;
    status += fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "RootDir03"))
        status++;
    status += fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "RootDir04"))
        status++;
    status += fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "RootDir05"))
        status++;
    status += fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "RootDir06"))
        status++;
    status += fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "RootDir07"))
        status++;
    status += fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "RootDir08"))
        status++;
    status += fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "RootFile09"))
        status++;
    status += fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "RootFile10"))
        status++;
    status += fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "RootDir11"))
        status++;
    status += fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "RootDir12"))
        status++;
    status += fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "RootDir13"))
        status++;
    status += fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "RootDir14"))
        status++;
    status += fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "RootFile15"))
        status++;

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(6);
    }

    /* Now read one past the number of entries in order to get an error condition.  */
    status += fx_directory_next_entry_find(&ram_disk, name);

    /* Determine if the test was successful.  */
    if (status != FX_NO_MORE_ENTRIES)
    {

        printf("ERROR!\n");
        test_control_return(7);
    }

    /* Now test the full directory entry services.  */

    /* Pickup the first entry in the root directory.  */
    status =  fx_directory_first_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "RootFile01"))
        status++;
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "RootFile02"))
        status++;
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "RootDir03"))
        status++;
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "RootDir04"))
        status++;
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "RootDir05"))
        status++;
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "RootDir06"))
        status++;
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "RootDir07"))
        status++;
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "RootDir08"))
        status++;
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "RootFile09"))
        status++;
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "RootFile10"))
        status++;
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "RootDir11"))
        status++;
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "RootDir12"))
        status++;
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "RootDir13"))
        status++;
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "RootDir14"))
        status++;
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "RootFile15"))
        status++;

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(8);
    }

    /* Now read one past the number of entries in order to get an error condition.  */
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);

    /* Determine if the test was successful.  */
    if (status != FX_NO_MORE_ENTRIES)
    {

        printf("ERROR!\n");
        test_control_return(9);
    }

    /* Okay, now postion to the deepest sub-directory and ensure that the directory first/next 
       operations work properly there as well.  */

    /* Set the current path. This should be / or NULL.   */
    status =  fx_directory_default_set(&ram_disk, "/RootDir04/RootDir04_SubDir04/RootDir04_SubDir04_SubSubDir02");

    /* Check the status.  */
    if (status != FX_SUCCESS) 
    {

        /* Error setting the path.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(10);
    }

    /* Pickup the first entry in this sub-sub directory.  */
    status =  fx_directory_first_entry_find(&ram_disk, name);
    if (strcmp(name, "RootDir04_SubDir04_SubSubDir01_SubSubSubDir01"))
        status++;
    status += fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "RootDir04_SubDir04_SubSubDir01_SubSubSubDir02"))
        status++;
    status += fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "RootDir04_SubDir04_SubSubDir01_SubSubSubDir03"))
        status++;
    status += fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "RootDir04_SubDir04_SubSubDir01_SubSubSubFile04"))
        status++;

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(11);
    }

    /* Now read one past the number of entries in order to get an error condition.  */
    status += fx_directory_next_entry_find(&ram_disk, name);

    /* Determine if the test was successful.  */
    if (status != FX_NO_MORE_ENTRIES)
    {

        printf("ERROR!\n");
        test_control_return(12);
    }

    /* Now do the same thing but with the full find first/next.  */
    
    /* Pickup the first entry in this sub-sub directory.  */
    status =  fx_directory_first_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "RootDir04_SubDir04_SubSubDir01_SubSubSubDir01"))
        status++;
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "RootDir04_SubDir04_SubSubDir01_SubSubSubDir02"))
        status++;
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "RootDir04_SubDir04_SubSubDir01_SubSubSubDir03"))
        status++;
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "RootDir04_SubDir04_SubSubDir01_SubSubSubFile04"))
        status++;

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(13);
    }

    /* Now read one past the number of entries in order to get an error condition.  */
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);

    /* Determine if the test was successful.  */
    if (status != FX_NO_MORE_ENTRIES)
    {

        printf("ERROR!\n");
        test_control_return(14);
    }
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(15);
    }
    else
    {

        printf("SUCCESS!\n");
        test_control_return(0);
    }
}

#endif /* FX_ENABLE_EXFAT */
