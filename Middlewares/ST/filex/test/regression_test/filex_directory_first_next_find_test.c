/* This FileX test concentrates on the basic first/next entry find operations.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_utility.h"
#include   "fx_ram_driver_test.h"
#include   <stdio.h>

#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              16*128

/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD                ftest_0;
#endif
static FX_MEDIA                 ram_disk;


/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *ram_disk_memory;
static UCHAR                  *cache_buffer;
#else
static UCHAR                  cache_buffer[CACHE_SIZE];
#endif
static CHAR                   name[256];


/* Define thread prototypes.  */

void    filex_directory_first_next_find_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_directory_first_next_find_application_define(void *first_unused_memory)
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
ULONG       temp;
UINT        year;
UINT        month;
UINT        day;
UINT        hour;
UINT        minute;
UINT        second;
FX_LOCAL_PATH local_path;
UINT        i;
// A file name whose first byte is 0xe5.
UCHAR       specified_ascii_name[] = { 0xe5, 'a', 'b', 'c', 0};

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Directory first/next entry find test...................");

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
    
    /* Attempt to get the next entry before the media is opened to generate an error */
    status = fx_directory_next_entry_find(&ram_disk, "/A0");
    if (status != FX_MEDIA_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(2);
    }
    
    /* Attempt to get the next full entry before the media is opened to generate an error */
    status = fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (status != FX_MEDIA_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(3);
    }
    
    /* Attempt to get the first entry before the media is opened to generate an error */
    status = fx_directory_first_entry_find(&ram_disk, "/A0");
    if (status != FX_MEDIA_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(4);
    }
    
    /* Attempt to get the first full entry before the media is opened to generate an error */
    status = fx_directory_first_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (status != FX_MEDIA_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(5);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(6);
    }
    
/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send null pointer to generate an error */
    status = fx_directory_first_entry_find(FX_NULL, "/A0");
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(6);
    }
    
    /* send null pointer to generate an error */
    status = fx_directory_first_full_entry_find(FX_NULL, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(7);
    }
#endif /* FX_DISABLE_ERROR_CHECKING */

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
        test_control_return(8);
    }

    /* Set the current path. This should be / or NULL.   */
    status =  fx_directory_default_set(&ram_disk, "/");

    /* Check the status.  */
    if (status != FX_SUCCESS) 
    {

        /* Error setting the path.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(9);
    }
    
/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send null pointer to generate an error */
    status = fx_directory_next_entry_find(FX_NULL, name);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(10);
    }
#endif /* FX_DISABLE_ERROR_CHECKING */

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

    /* Now test the full directory entry services.  */
    
/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send null pointer to generate an error */
    status = fx_directory_next_full_entry_find(FX_NULL, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(13);
    }
#endif /* FX_DISABLE_ERROR_CHECKING */

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
        test_control_return(14);
    }

    /* Now read one past the number of entries in order to get an error condition.  */
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);

    /* Determine if the test was successful.  */
    if (status != FX_NO_MORE_ENTRIES)
    {

        printf("ERROR!\n");
        test_control_return(15);
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
        test_control_return(16);
    }

    /* Pickup the first entry in this sub-sub directory.  */
    status =  fx_directory_first_entry_find(&ram_disk, name);
    if (strcmp(name, "."))
        status++;
    status =  fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, ".."))
        status++;
    status =  fx_directory_next_entry_find(&ram_disk, name);
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
        test_control_return(17);
    }

    /* Now read one past the number of entries in order to get an error condition.  */
    status += fx_directory_next_entry_find(&ram_disk, name);

    /* Determine if the test was successful.  */
    if (status != FX_NO_MORE_ENTRIES)
    {

        printf("ERROR!\n");
        test_control_return(18);
    }

    /* Now do the same thing but with the full find first/next.  */
    
/*  pick up the first full entry with a local path defined to get to all of the code */
#ifndef FX_NO_LOCAL_PATH
    status  =  fx_directory_local_path_set(&ram_disk, &local_path, "/RootDir04/RootDir04_SubDir03");
    status +=  fx_directory_first_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "."))
        status++;
    status +=  fx_directory_local_path_clear(&ram_disk);
    status +=  fx_directory_local_path_set(&ram_disk, &local_path, "/");
    status +=  fx_directory_first_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    status +=  fx_directory_local_path_clear(&ram_disk);
    
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(19);
    }
#endif
    
    /* Pickup the first entry in this sub-sub directory.  */
    status =  fx_directory_first_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, "."))
        status++;
    status =  fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);
    if (strcmp(name, ".."))
        status++;
    status =  fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
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
        test_control_return(20);
    }

    /* Now read one past the number of entries in order to get an error condition.  */
    status += fx_directory_next_full_entry_find(&ram_disk, name, &attributes, 
                &size, &year, &month, &day, &hour, &minute, &second);

    /* Determine if the test was successful.  */
    if (status != FX_NO_MORE_ENTRIES)
    {

        printf("ERROR!\n");
        test_control_return(21);
    }
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(22);
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
                            4096,                   // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(23);
    }
    
    /* Open the ram_disk - 1 sector cache.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(24);
    }

    /* Call first entry find with nothing in the root directory.  */
    status =  fx_directory_first_entry_find(&ram_disk, name);
    
    /* This should return no more entries.  */
    if (status != FX_NO_MORE_ENTRIES)
    {
        printf("ERROR!\n");
        test_control_return(25);
    }   

    /* Create a directory structure that we can traverse.  */
    status =  fx_directory_create(&ram_disk, "\\SUB1");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST1.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST2.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST3.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST4.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST5.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST6.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST7.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST8.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST9.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST10.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST11.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST12.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST13.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST14.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST15.TXT");
    status += fx_file_delete(&ram_disk, "\\SUB1\\TEST7.TXT");       /* Leave a free hole in the directory.  */
    status += fx_directory_default_set(&ram_disk, "\\SUB1");
    /* See if we can add a 255 character name.  */
    for (i = 0; i < 256; i++)
    {
        name[i] =  'a';
    }
    name[255] = 0;
    
    /* Create a max length named file in this sub-directory.  */
    status +=  fx_file_create(&ram_disk, name);

    /* Check for any errors.  */
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(26);
    }

    /* Now get the first entry... with a cluster of 0 error first.  Note this calls next entry find inside.  */
    _fx_utility_fat_entry_read_error_request = 10001;
    status = fx_directory_first_entry_find(&ram_disk, name);
    _fx_utility_fat_entry_read_error_request =  0;
    
    /* This should return no more entries.  */
    if (status != FX_FAT_READ_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(27);
    }   
    
    /* Now get the first entry... with a cluster equal to itself first.  Note this calls next entry find inside.  */
    _fx_utility_fat_entry_read_error_request = 30001;
    status = fx_directory_first_entry_find(&ram_disk, name);
    _fx_utility_fat_entry_read_error_request =  0;
    
    /* This should return no more entries.  */
    if (status != FX_FAT_READ_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(28);
    }   

    /* Now get the first entry... with an I/O error on the FAT read.  Note this calls next entry find inside.  */
    _fx_utility_fat_entry_read_error_request = 1;
    status = fx_directory_first_entry_find(&ram_disk, name);
    _fx_utility_fat_entry_read_error_request =  0;
    
    /* This should return no more entries.  */
    if (status != FX_IO_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(29);
    }   

    /* Now get the first entry... with a smaller value of total clusters.  Note this calls next entry find inside.  */
    temp =  ram_disk.fx_media_total_clusters;
    ram_disk.fx_media_total_clusters =  1;
    status = fx_directory_first_entry_find(&ram_disk, name);
    ram_disk.fx_media_total_clusters =  temp;
    
    /* This should return no more entries.  */
    if (status != FX_FAT_READ_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(30);
    }   

    /* Now get the first entry... successfully.  Note this calls next entry find inside.  */
    status = fx_directory_first_entry_find(&ram_disk, name);    /* TEST.TXT  */

    /* Check for any errors.  */
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(31);
    }

    /* Now flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }

    /* Now get the first entry... with an I/O error on the directory read.  Note this calls next entry find inside.  */
    _fx_utility_logical_sector_read_error_request = 1;
    status = fx_directory_next_entry_find(&ram_disk, name);
    _fx_utility_logical_sector_read_error_request = 0;
    
    /* This should return no more entries.  */
    if (status != FX_IO_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(32);
    }   
    
    /* Now get all the directory entires.   */
    status =  fx_directory_next_entry_find(&ram_disk, name);    /* .          */
    status =  fx_directory_next_entry_find(&ram_disk, name);    /* ..          */
    status =  fx_directory_next_entry_find(&ram_disk, name);    /* TEST1.TXT  */
    status += fx_directory_next_entry_find(&ram_disk, name);    /* TEST2.TXT  */
    status += fx_directory_next_entry_find(&ram_disk, name);    /* TEST3.TXT  */
    status += fx_directory_next_entry_find(&ram_disk, name);    /* TEST4.TXT  */
    status += fx_directory_next_entry_find(&ram_disk, name);    /* TEST5.TXT  */
    status += fx_directory_next_entry_find(&ram_disk, name);    /* TEST6.TXT  */
    status += fx_directory_next_entry_find(&ram_disk, name);    /* TEST8.TXT  */
    status += fx_directory_next_entry_find(&ram_disk, name);    /* TEST9.TXT  */
    status += fx_directory_next_entry_find(&ram_disk, name);    /* TEST10.TXT  */
    status += fx_directory_next_entry_find(&ram_disk, name);    /* TEST11.TXT  */
    status += fx_directory_next_entry_find(&ram_disk, name);    /* TEST12.TXT  */
    status += fx_directory_next_entry_find(&ram_disk, name);    /* TEST13.TXT  */
    status += fx_directory_next_entry_find(&ram_disk, name);    /* TEST14.TXT  */
    status += fx_directory_next_entry_find(&ram_disk, name);    /* TEST15.TXT  */
    status += fx_directory_next_entry_find(&ram_disk, name);    /* 255 character file name....  */

    /* Check for any errors.  */
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(33);
    }

    /* Now make another request to get the FX_NO_MORE_ENTRIES error.  */
    status =  fx_directory_next_entry_find(&ram_disk, name);    /* END of Directory! */
    
    /* This should return no more entries.  */
    if (status != FX_NO_MORE_ENTRIES)
    {
        printf("ERROR!\n");
        test_control_return(34);
    }

    /* Now make another request to get the FX_NO_MORE_ENTRIES error, but artifically move the current entry ahead.  */
    temp =  ram_disk.fx_media_default_path.fx_path_current_entry;
    ram_disk.fx_media_default_path.fx_path_current_entry =  100;
    status =  fx_directory_next_entry_find(&ram_disk, name);    /* END of Directory! */
    ram_disk.fx_media_default_path.fx_path_current_entry =  temp;
    
    /* This should return no more entries.  */
    if (status != FX_NO_MORE_ENTRIES)
    {
        printf("ERROR!\n");
        test_control_return(35);
    }

    /* Now create enough sub-directories to exceed the maximum path.  */
    status =  fx_directory_create(&ram_disk, "subdirectory1");
    status += fx_directory_default_set(&ram_disk, "subdirectory1");
    status += fx_directory_create(&ram_disk, "subdirectory2");
    status += fx_directory_default_set(&ram_disk, "subdirectory2");
    status += fx_directory_create(&ram_disk, "subdirectory3");
    status += fx_directory_default_set(&ram_disk, "subdirectory3");
    status += fx_directory_create(&ram_disk, "subdirectory4");
    status += fx_directory_default_set(&ram_disk, "subdirectory4");
    status += fx_directory_create(&ram_disk, "subdirectory5");
    status += fx_directory_default_set(&ram_disk, "subdirectory5");
    status += fx_directory_create(&ram_disk, "subdirectory6");
    status += fx_directory_default_set(&ram_disk, "subdirectory6");
    status += fx_directory_create(&ram_disk, "subdirectory7");
    status += fx_directory_default_set(&ram_disk, "subdirectory7");
    status += fx_directory_create(&ram_disk, "subdirectory8");
    status += fx_directory_default_set(&ram_disk, "subdirectory8");
    status += fx_directory_create(&ram_disk, "subdirectory9");
    status += fx_directory_default_set(&ram_disk, "subdirectory9");
    status += fx_directory_create(&ram_disk, "subdirectory10");
    status += fx_directory_default_set(&ram_disk, "subdirectory10");
    status += fx_directory_create(&ram_disk, "subdirectory11");
    status += fx_directory_default_set(&ram_disk, "subdirectory11");
    status += fx_directory_create(&ram_disk, "subdirectory12");
    status += fx_directory_default_set(&ram_disk, "subdirectory12");
    status += fx_directory_create(&ram_disk, "subdirectory13");
    status += fx_directory_default_set(&ram_disk, "subdirectory13");
    status += fx_directory_create(&ram_disk, "subdirectory14");
    status += fx_directory_default_set(&ram_disk, "subdirectory14");
    status += fx_directory_create(&ram_disk, "subdirectory15");
    status += fx_directory_default_set(&ram_disk, "subdirectory15");
    status += fx_directory_create(&ram_disk, "subdirectory16");
    status += fx_directory_default_set(&ram_disk, "subdirectory16");
    status += fx_directory_create(&ram_disk, "subdirectory17");
    status += fx_directory_default_set(&ram_disk, "subdirectory17");
    status += fx_directory_create(&ram_disk, "subdirectory18");
    status += fx_directory_default_set(&ram_disk, "subdirectory18");
    status += fx_directory_create(&ram_disk, "subdirectory19");
    status += fx_directory_default_set(&ram_disk, "subdirectory19");
    status += fx_directory_create(&ram_disk, "subdirectory20");
    status += fx_directory_default_set(&ram_disk, "subdirectory20");
    status += fx_directory_create(&ram_disk, "subdirectory21");
    status += fx_directory_default_set(&ram_disk, "subdirectory21");
    status += fx_directory_create(&ram_disk, "subdirectory22");
    status += fx_directory_default_set(&ram_disk, "subdirectory22");
    status += fx_directory_create(&ram_disk, "subdirectory23");
    status += fx_directory_default_set(&ram_disk, "subdirectory23");
    status += fx_directory_create(&ram_disk, "subdirectory24");
    status += fx_directory_default_set(&ram_disk, "subdirectory24");
    status += fx_directory_create(&ram_disk, "subdirectory25");
    status += fx_directory_default_set(&ram_disk, "subdirectory25");
    status += fx_directory_create(&ram_disk, "subdirectory26");
    status += fx_directory_default_set(&ram_disk, "subdirectory26");
    status += fx_directory_create(&ram_disk, "subdirectory27");
    status += fx_directory_default_set(&ram_disk, "subdirectory27");
    status += fx_directory_create(&ram_disk, "subdirectory28");
    status += fx_directory_default_set(&ram_disk, "subdirectory28");
    status += fx_directory_create(&ram_disk, "subdirectory29");
    status += fx_directory_default_set(&ram_disk, "subdirectory29");
    status += fx_directory_first_entry_find(&ram_disk, name);    /* Should be . */
    status += fx_directory_next_entry_find(&ram_disk, name);     /* Should be .. */

    /* Check for any errors.  */
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(36);
    }
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(37);
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
                            4096,                   // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(38);
    }
    
    /* Open the ram_disk - 1 sector cache.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(39);
    }

    /* Call first entry find with nothing in the root directory.  */
    status =  fx_directory_first_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);
    
    /* This should return no more entries.  */
    if (status != FX_NO_MORE_ENTRIES)
    {
        printf("ERROR!\n");
        test_control_return(40);
    }   

    /* Create a directory structure that we can traverse.  */
    status =  fx_directory_create(&ram_disk, "\\SUB1");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST1.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST2.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST3.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST4.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST5.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST6.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST7.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST8.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST9.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST10.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST11.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST12.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST13.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST14.TXT");
    status += fx_file_create(&ram_disk, "\\SUB1\\TEST15.TXT");
    status += fx_file_delete(&ram_disk, "\\SUB1\\TEST7.TXT");       /* Leave a free hole in the directory.  */
    status += fx_directory_default_set(&ram_disk, "\\SUB1");
    /* See if we can add a 255 character name.  */
    for (i = 0; i < 256; i++)
    {
        name[i] =  'a';
    }
    name[255] = 0;
    
    /* Create a max length named file in this sub-directory.  */
    status +=  fx_file_create(&ram_disk, name);

    /* Check for any errors.  */
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(41);
    }

    /* Now get the first entry... with a cluster of 0 error first.  Note this calls next entry find inside.  */
    _fx_utility_fat_entry_read_error_request = 10001;
    status = fx_directory_first_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &minute, FX_NULL);
    _fx_utility_fat_entry_read_error_request =  0;
    
    /* This should return no more entries.  */
    if (status != FX_FAT_READ_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(42);
    }   
    
    /* Now get the first entry... with a cluster equal to itself first.  Note this calls next entry find inside.  */
    _fx_utility_fat_entry_read_error_request = 30001;
    status = fx_directory_first_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);
    _fx_utility_fat_entry_read_error_request =  0;
    
    /* This should return no more entries.  */
    if (status != FX_FAT_READ_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(43);
    }   

    /* Now get the first entry... with an I/O error on the FAT read.  Note this calls next entry find inside.  */
    _fx_utility_fat_entry_read_error_request = 1;
    status = fx_directory_first_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);
    _fx_utility_fat_entry_read_error_request =  0;
    
    /* This should return no more entries.  */
    if (status != FX_IO_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(44);
    }   

    /* Now get the first entry... with a smaller value of total clusters.  Note this calls next entry find inside.  */
    temp =  ram_disk.fx_media_total_clusters;
    ram_disk.fx_media_total_clusters =  1;
    status = fx_directory_first_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);
    ram_disk.fx_media_total_clusters =  temp;
    
    /* This should return no more entries.  */
    if (status != FX_FAT_READ_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(45);
    }   

    /* Now get the first entry... successfully.  Note this calls next entry find inside.  */
    status = fx_directory_first_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* TEST.TXT  */

    /* Check for any errors.  */
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(46);
    }

    /* Now flush everything out.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }

    /* Now get the first entry... with an I/O error on the directory read.  Note this calls next entry find inside.  */
    _fx_utility_logical_sector_read_error_request = 1;
    status = fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);
    _fx_utility_logical_sector_read_error_request = 0;
    
    /* This should return no more entries.  */
    if (status != FX_IO_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(47);
    }   
    
    /* Now get all the directory entires.   */
    status =  fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* .          */
    status =  fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &minute, FX_NULL);    /* ..         */
    status =  fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* TEST1.TXT  */
    status += fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* TEST2.TXT  */
    status += fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* TEST3.TXT  */
    status += fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* TEST4.TXT  */
    status += fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* TEST5.TXT  */
    status += fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* TEST6.TXT  */
    status += fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* TEST8.TXT  */
    status += fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* TEST9.TXT  */
    status += fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* TEST10.TXT  */
    status += fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* TEST11.TXT  */
    status += fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* TEST12.TXT  */
    status += fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* TEST13.TXT  */
    status += fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* TEST14.TXT  */
    status += fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* TEST15.TXT  */
    status += fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* 255 character file name....  */

    /* Check for any errors.  */
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(48);
    }

    /* Now make another request to get the FX_NO_MORE_ENTRIES error.  */
    status =  fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* END of Directory! */
    
    /* This should return no more entries.  */
    if (status != FX_NO_MORE_ENTRIES)
    {
        printf("ERROR!\n");
        test_control_return(49);
    }

    /* Now make another request to get the FX_NO_MORE_ENTRIES error, but artifically move the current entry ahead.  */
    temp =  ram_disk.fx_media_default_path.fx_path_current_entry;
    ram_disk.fx_media_default_path.fx_path_current_entry =  100;
    status =  fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* END of Directory! */
    ram_disk.fx_media_default_path.fx_path_current_entry =  temp;
    
    /* This should return no more entries.  */
    if (status != FX_NO_MORE_ENTRIES)
    {
        printf("ERROR!\n");
        test_control_return(50);
    }

    /* Now create enough sub-directories to exceed the maximum path.  */
    status =  fx_directory_create(&ram_disk, "subdirectory1");
    status += fx_directory_default_set(&ram_disk, "subdirectory1");

    // Create the file whose name's first byte is 0xe5.
    status = fx_file_create(&ram_disk, (CHAR *)specified_ascii_name);

    status += fx_directory_create(&ram_disk, "subdirectory2");
    status += fx_directory_default_set(&ram_disk, "subdirectory2");
    status += fx_directory_create(&ram_disk, "subdirectory3");
    status += fx_directory_default_set(&ram_disk, "subdirectory3");
    status += fx_directory_create(&ram_disk, "subdirectory4");
    status += fx_directory_default_set(&ram_disk, "subdirectory4");
    status += fx_directory_create(&ram_disk, "subdirectory5");
    status += fx_directory_default_set(&ram_disk, "subdirectory5");
    status += fx_directory_create(&ram_disk, "subdirectory6");
    status += fx_directory_default_set(&ram_disk, "subdirectory6");
    status += fx_directory_create(&ram_disk, "subdirectory7");
    status += fx_directory_default_set(&ram_disk, "subdirectory7");
    status += fx_directory_create(&ram_disk, "subdirectory8");
    status += fx_directory_default_set(&ram_disk, "subdirectory8");
    status += fx_directory_create(&ram_disk, "subdirectory9");
    status += fx_directory_default_set(&ram_disk, "subdirectory9");
    status += fx_directory_create(&ram_disk, "subdirectory10");
    status += fx_directory_default_set(&ram_disk, "subdirectory10");
    status += fx_directory_create(&ram_disk, "subdirectory11");
    status += fx_directory_default_set(&ram_disk, "subdirectory11");
    status += fx_directory_create(&ram_disk, "subdirectory12");
    status += fx_directory_default_set(&ram_disk, "subdirectory12");
    status += fx_directory_create(&ram_disk, "subdirectory13");
    status += fx_directory_default_set(&ram_disk, "subdirectory13");
    status += fx_directory_create(&ram_disk, "subdirectory14");
    status += fx_directory_default_set(&ram_disk, "subdirectory14");
    status += fx_directory_create(&ram_disk, "subdirectory15");
    status += fx_directory_default_set(&ram_disk, "subdirectory15");
    status += fx_directory_create(&ram_disk, "subdirectory16");
    status += fx_directory_default_set(&ram_disk, "subdirectory16");
    status += fx_directory_create(&ram_disk, "subdirectory17");
    status += fx_directory_default_set(&ram_disk, "subdirectory17");
    status += fx_directory_create(&ram_disk, "subdirectory18");
    status += fx_directory_default_set(&ram_disk, "subdirectory18");
    status += fx_directory_create(&ram_disk, "subdirectory19");
    status += fx_directory_default_set(&ram_disk, "subdirectory19");
    status += fx_directory_create(&ram_disk, "subdirectory20");
    status += fx_directory_default_set(&ram_disk, "subdirectory20");
    status += fx_directory_create(&ram_disk, "subdirectory21");
    status += fx_directory_default_set(&ram_disk, "subdirectory21");
    status += fx_directory_create(&ram_disk, "subdirectory22");
    status += fx_directory_default_set(&ram_disk, "subdirectory22");
    status += fx_directory_create(&ram_disk, "subdirectory23");
    status += fx_directory_default_set(&ram_disk, "subdirectory23");
    status += fx_directory_create(&ram_disk, "subdirectory24");
    status += fx_directory_default_set(&ram_disk, "subdirectory24");
    status += fx_directory_create(&ram_disk, "subdirectory25");
    status += fx_directory_default_set(&ram_disk, "subdirectory25");
    status += fx_directory_create(&ram_disk, "subdirectory26");
    status += fx_directory_default_set(&ram_disk, "subdirectory26");
    status += fx_directory_create(&ram_disk, "subdirectory27");
    status += fx_directory_default_set(&ram_disk, "subdirectory27");
    status += fx_directory_create(&ram_disk, "subdirectory28");
    status += fx_directory_default_set(&ram_disk, "subdirectory28");
    status += fx_directory_create(&ram_disk, "subdirectory29");
    status += fx_directory_default_set(&ram_disk, "subdirectory29");
    status += fx_directory_first_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);    /* Should be . */
    status += fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);     /* Should be .. */

    /* Check for any errors.  */
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(51);
    }
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
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
                            4096,                   // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Open the ram_disk.  */
    status +=  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_value_if_fail(status == FX_SUCCESS, 53);

    status = fx_file_create(&ram_disk, (CHAR *)specified_ascii_name);
    return_value_if_fail(status == FX_SUCCESS, 54);

    specified_ascii_name[1]++;
    status = fx_file_create(&ram_disk, (CHAR *)specified_ascii_name);
    return_value_if_fail(status == FX_SUCCESS, 55);

    // Test the file whose name's first byte is 0xe5.
    status = fx_directory_first_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);

    /* Now get all the directory entires.   */
    status +=  fx_directory_next_full_entry_find(&ram_disk, name, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &second);
    return_value_if_fail(status == FX_SUCCESS, 56);

    // Try fx_directory_first_entry_find this time.
    status = fx_directory_first_entry_find(&ram_disk, name);
    return_value_if_fail(status == FX_SUCCESS, 57);

    // Try fx_media_volume_set this time.
    status = fx_media_volume_set(&ram_disk, (CHAR *)specified_ascii_name);
    return_value_if_fail(status == FX_SUCCESS, 58);

    printf("SUCCESS!\n");
    test_control_return(0);
}

