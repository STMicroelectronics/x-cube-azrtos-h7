/* This FileX test concentrates on the directory long/short name get operations.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"

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
static UCHAR                    cache_buffer[CACHE_SIZE];
#endif
static CHAR                     name[50];
static CHAR                     max_name[FX_MAX_LONG_NAME_LEN+1];
static CHAR                     return_name[FX_MAX_LONG_NAME_LEN+1];


/* Define thread prototypes.  */

void    filex_directory_long_short_get_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_directory_long_short_get_application_define(void *first_unused_memory)
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
UINT        i;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Directory long/short name get test.....................");

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
                            2048,                   // Total sectors 
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

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(2);
    }
    
    /* Attempt to get a directory that does not exist */
    status =  fx_directory_short_name_get(&ram_disk, "does_not_exist", "does_not_exist");
    if (status == FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(3);
    }

    /* Simple 8.3 rename in the root directory.  */
    status =  fx_directory_create(&ram_disk, "subdir");
    status +=  fx_file_create(&ram_disk, "name");
    status += fx_file_create(&ram_disk,  "longnametest");
    status += fx_file_create(&ram_disk,  "subdir/SNAME.TXT");
    status += fx_file_create(&ram_disk,  "subdir/longnametest");
    
    /* Check the directory/file create status.  */
    if (status != FX_SUCCESS)
    {

        /* Error creating files/directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(4);
    }
    
/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send null pointer to generate an error */
    status = fx_directory_short_name_get(FX_NULL, "subdir", name);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(5);
    }
    /* send null pointer to generate an error */
    status = fx_directory_short_name_get_extended(FX_NULL, "subdir", name, 1);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(5);
    }
#endif /* FX_DISABLE_ERROR_CHECKING */
    
    /* attempt to get the long name of something that doesnt exist */
    status = fx_directory_long_name_get(&ram_disk, "does_not_exist", "does_not_exist");
    if (status == FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(6);
    }

    /* Get the short name of a long name.  */
    status =  fx_directory_short_name_get(&ram_disk, "subdir", name);
    status += fx_directory_long_name_get(&ram_disk, name, name);

    /* Check for errors.  */
    if ((status != FX_SUCCESS) || (strcmp(name, "subdir")))
    {

        /* Error getting long/short name.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(7);
    }
   
    /* Set name to known state.  */
    memcpy(name, "ORIGINALSTRING", 15);

    /* Test extended short name get with short buffer length.  */
    status = fx_directory_short_name_get_extended(&ram_disk, "subdir", name, 2);

    /* Check for errors.  */
    if ((status != FX_BUFFER_ERROR) || (memcmp(name, "S\0IGINALSTRING\0", 15)))
    {

        /* Error getting short name extended.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(7);
    }

    /* Get the long name from short name.  */
    status = fx_directory_long_name_get(&ram_disk, "NAME", name);
    status += fx_directory_short_name_get(&ram_disk, name, name);

    /* Check for errors.  */
    if ((status != FX_SUCCESS) || (strcmp(name, "NAME")))
    {

        /* Error getting long/short name.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(8);
    }

    /* Get the short name of a long name.  */
    status =  fx_directory_short_name_get(&ram_disk, "longnametest", name);
    status += fx_directory_long_name_get(&ram_disk, name, name);

    /* Check for errors.  */
    if ((status != FX_SUCCESS) || (strcmp(name, "longnametest")))
    {

        /* Error getting long/short name.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(9);
    }

    /* Move to sub-directory.  */
    status +=  fx_directory_default_set(&ram_disk, "subdir");

    /* Get the long name from short name.  */
    status += fx_directory_long_name_get(&ram_disk, "SNAME.TXT", name);
    status += fx_directory_short_name_get(&ram_disk, name, name);

    /* Check for errors.  */
    if ((status != FX_SUCCESS) || (strcmp(name, "SNAME.TXT")))
    {

        /* Error getting long/short name.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(10);
    }

    /* Set name to known state.  */
    memcpy(name, "ORIGINALSTRING", 15);

    /* Get the long name from short name using extended version.  */
    status = fx_directory_long_name_get_extended(&ram_disk, "SNAME.TXT", name, 2);

    /* Check for error.  */
    if ((status != FX_BUFFER_ERROR) || (memcmp(name, "S\0IGINALSTRING\0", 15)))
    {

        /* Error getting long/short name.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(10);
    }

    /* Set name to known state.  */
    memcpy(name, "ORIGINALSTRING", 15);
    status = fx_directory_short_name_get_extended(&ram_disk, "SNAME.TXT", name, 2);

    /* Check for error.  */
    if ((status != FX_BUFFER_ERROR) || (memcmp(name, "S\0IGINALSTRING\0", 15)))
    {

        /* Error getting long/short name.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(10);
    }

    /* Get the short name from long name.  */
    status = fx_directory_short_name_get(&ram_disk, "longnametest", name);
    status += fx_directory_long_name_get(&ram_disk, name, name);

    /* Check for errors.  */
    if ((status != FX_SUCCESS) || (strcmp(name, "longnametest")))
    {

        /* Error getting long/short name.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(11);
    }
    
/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send null pointer to generate an error */
    status = fx_directory_long_name_get(FX_NULL, "NAME", name);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(12);
    }
    /* send null pointer to generate an error */
    status = fx_directory_long_name_get_extended(FX_NULL, "NAME", name, 1);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(12);
    }
#endif /* FX_DISABLE_ERROR_CHECKING */
    
    
    /* Create a maximum size short file name.  */
    status =  fx_file_create(&ram_disk, "MAX_SIZE.TXT");

    /* Get the short name.  */
    status += fx_directory_short_name_get(&ram_disk, "MAX_SIZE.TXT", name);

    /* Check for errors.  */
    if ((status != FX_SUCCESS) || (strcmp(name, "MAX_SIZE.TXT")))
    {

        /* Error getting long/short name.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(13);
    }  
    
    /* Build a maximum sized long file name.  */
    for (i = 0; i < (FX_MAX_LONG_NAME_LEN-1); i++)
    {
    
        /* Set a character in the file name.  */
        max_name[i] =  'a';
    }
    max_name[i] =  0;
    max_name[i-4] = '.';
    
    /* Create a maximum size long file name.  */
    status =  fx_file_create(&ram_disk, max_name);

    /* Get the short name.  */
    status += fx_directory_short_name_get(&ram_disk, max_name, return_name);

    /* Check for errors.  */
    if ((status != FX_SUCCESS) || (strcmp(return_name, "AAA~001A.AAA")))
    {

        /* Error getting long/short name.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(14);
    }  

    /* Get the long name.  */
    status += fx_directory_long_name_get(&ram_disk, return_name, return_name);

    /* Check for errors.  */
    if ((status != FX_SUCCESS) || (strcmp(return_name, max_name)))
    {

        /* Error getting long/short name.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(15);
    }  
    
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS) 
    {

        printf("ERROR!\n");
        test_control_return(16);
    }
    else
    {

        printf("SUCCESS!\n");
        test_control_return(0);
    }
}
