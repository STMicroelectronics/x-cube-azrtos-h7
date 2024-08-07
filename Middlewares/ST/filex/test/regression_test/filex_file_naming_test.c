/* This FileX test concentrates on the basic file naming operations.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"

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
static UCHAR                   cache_buffer[CACHE_SIZE];
#endif


/* Define thread prototypes.  */

void    filex_file_naming_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_file_naming_application_define(void *first_unused_memory)
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

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   File naming test.......................................");

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

    /* Create a series of directories...  */
    status =   fx_directory_create(&ram_disk, "A0");
    status +=  fx_directory_create(&ram_disk, ".b0");
    status +=  fx_directory_create(&ram_disk, "C0");
    status +=  fx_directory_create(&ram_disk, ".d0.dir");
    status +=  fx_file_create(&ram_disk, ".FOO.BAR");

    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error creating directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(4);
    }

    /* Attempt to create the same directory again.  */
    status =   fx_directory_create(&ram_disk, "/a0");

    /* Check for errors...  */
    if (status != FX_ALREADY_CREATED)
    {

        /* Error creating same directory twice.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(5);
    }

    /* Create files with the E5 character.   */
    special_name[0] =  (CHAR)0xE5;
    special_name[1] =  0x2E;
    special_name[2] =  0x54;
    special_name[3] =  0x58;
    special_name[4] =  0x54;
    special_name[5] =  0;
    status =  fx_file_create(&ram_disk, special_name);
    status += fx_media_flush(&ram_disk);
    status += fx_file_delete(&ram_disk, special_name);

    /* Check for errors.  */
    if (status != FX_SUCCESS)
    {

        /* Error creating special name.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(5);
    } 

    /* Create another special file name.  */
    special_name[0] =  (CHAR)0xE5;
    special_name[1] =  0x31;
    special_name[2] =  0x2E;
    special_name[3] =  0x54;
    special_name[4] =  0x58;
    special_name[5] =  0x54;
    special_name[6] =  0;
    status =  fx_file_create(&ram_disk, special_name);
    status += fx_media_flush(&ram_disk);
    status += fx_file_delete(&ram_disk, special_name);

    /* Check for errors.  */
    if (status != FX_SUCCESS)
    {

        /* Error creating special name.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(6);
    } 

    /* Create the next level of sub-directories.... with the interesting names...  */
    status =   fx_directory_default_set(&ram_disk, "/A0");

    status =   fx_file_create(&ram_disk, ".a.a.a.");
    status +=  fx_file_create(&ram_disk, "this...ismytestfile");
    status +=  fx_file_create(&ram_disk, "this ... is my test file");
    status +=  fx_file_create(&ram_disk, "ThisIsTheFirstVeryLongFileNameForThisTestABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZfirstfile11111111111111110000000000000000000000000000000000000000000000000000000");
    status +=  fx_directory_create(&ram_disk, "       c");
    status +=  fx_directory_create(&ram_disk, "       c/    e");
    status +=  fx_directory_create(&ram_disk, "       c/    e/    f");
    status +=  fx_file_create(&ram_disk, "/A0    /    c/    e/    f/test.txt");
    status +=  fx_file_create(&ram_disk, "/.d0.dir/test.txt");
    status +=  fx_file_create(&ram_disk, "/.b0/TEST.TXT");
    status +=  fx_media_flush(&ram_disk);
    
    /* Check for errors.  */
    if (status != FX_SUCCESS)
    {

        /* Error creating interesting file names.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(7);
    } 

    /* Now attempt to delete the files in each of these sub-directories.  */
    status =   fx_file_delete(&ram_disk, "/A0/.a.a.a.");
    status +=  fx_file_delete(&ram_disk, "/A0/this...ismytestfile");
    status +=  fx_file_delete(&ram_disk, "/A0/this ... is my test file");
    status +=  fx_file_delete(&ram_disk, "/A0/ThisIsTheFirstVeryLongFileNameForThisTestABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZfirstfile11111111111111110000000000000000000000000000000000000000000000000000000");
    status +=  fx_file_delete(&ram_disk, "/A0    /    c/    e/    f/test.txt");
    status +=  fx_directory_delete(&ram_disk, "/A0  /c  /e/  f");
    status +=  fx_directory_delete(&ram_disk, "/A0  /c  /e");
    status +=  fx_directory_delete(&ram_disk, "/A0  /c ");
    status +=  fx_file_delete(&ram_disk, "/.d0.dir/test.txt");
    status +=  fx_file_delete(&ram_disk, "/.b0/TEST.TXT");
    status +=  fx_directory_delete(&ram_disk, "/A0");
    status +=  fx_directory_delete(&ram_disk, "/     .b0");
    status +=  fx_directory_delete(&ram_disk, "/C0        ");
    status +=  fx_directory_delete(&ram_disk, "/.d0.dir");
    status +=  fx_file_delete(&ram_disk, "/.FOO.BAR");
    status +=  fx_media_flush(&ram_disk);

    /* Check for errors.  */
    if (status != FX_SUCCESS)
    {

        /* Error deleting test files in the interesting sub-directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(8);
    } 

    /* Set default to root directory.  */
    status =   fx_directory_default_set(&ram_disk, "/");

    /* Flush the media... should be empty at this point.  */
    status += fx_media_flush(&ram_disk);

    /* Now fill the media's root directory until we get an error...  We will use 8.3 names 
       to attempt to create 32 names.    */
    status +=  fx_file_create(&ram_disk, "A01");
    status +=  fx_file_create(&ram_disk, "A02");
    status +=  fx_file_create(&ram_disk, "A03");
    status +=  fx_file_create(&ram_disk, "A04");
    status +=  fx_file_create(&ram_disk, "A05");
    status +=  fx_file_create(&ram_disk, "A06");
    status +=  fx_file_create(&ram_disk, "A07");
    status +=  fx_file_create(&ram_disk, "A08");
    status +=  fx_file_create(&ram_disk, "A09");
    status +=  fx_file_create(&ram_disk, "A10");
    status +=  fx_file_create(&ram_disk, "A11");
    status +=  fx_file_create(&ram_disk, "A12");
    status +=  fx_file_create(&ram_disk, "A13");
    status +=  fx_file_create(&ram_disk, "A14");
    status +=  fx_file_create(&ram_disk, "A15");
    status +=  fx_file_create(&ram_disk, "A16");
    status +=  fx_file_create(&ram_disk, "A17");
    status +=  fx_file_create(&ram_disk, "A18");
    status +=  fx_file_create(&ram_disk, "A19");
    status +=  fx_file_create(&ram_disk, "A20");
    status +=  fx_file_create(&ram_disk, "A21");
    status +=  fx_file_create(&ram_disk, "A22");
    status +=  fx_file_create(&ram_disk, "A23");
    status +=  fx_file_create(&ram_disk, "A24");
    status +=  fx_file_create(&ram_disk, "A25");
    status +=  fx_file_create(&ram_disk, "A26");
    status +=  fx_file_create(&ram_disk, "A27");
    status +=  fx_file_create(&ram_disk, "A28");
    status +=  fx_file_create(&ram_disk, "A29");
    status +=  fx_file_create(&ram_disk, "A30");
    status +=  fx_file_create(&ram_disk, "A31");
    status +=  fx_file_create(&ram_disk, "A32");

    fx_media_flush(&ram_disk);

    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error creating files.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(9);
    }

    /* Attempt to create 33rd name... this should fail.  */
    status =  fx_file_create(&ram_disk, "A33");

    /* Check for errors...  */
    if (status != FX_NO_MORE_SPACE)
    {

        /* Error creating files.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(10);
    }

    /* Flush the media.  */
    status =  fx_media_flush(&ram_disk);

    /* Now delete all the names.  */
    status +=  fx_file_delete(&ram_disk, "A01");
    status +=  fx_file_delete(&ram_disk, "A02");
    status +=  fx_file_delete(&ram_disk, "A03");
    status +=  fx_file_delete(&ram_disk, "A04");
    status +=  fx_file_delete(&ram_disk, "A05");
    status +=  fx_file_delete(&ram_disk, "A06");
    status +=  fx_file_delete(&ram_disk, "A07");
    status +=  fx_file_delete(&ram_disk, "A08");
    status +=  fx_file_delete(&ram_disk, "A09");
    status +=  fx_file_delete(&ram_disk, "A10");
    status +=  fx_file_delete(&ram_disk, "A11");
    status +=  fx_file_delete(&ram_disk, "A12");
    status +=  fx_file_delete(&ram_disk, "A13");
    status +=  fx_file_delete(&ram_disk, "A14");
    status +=  fx_file_delete(&ram_disk, "A15");
    status +=  fx_file_delete(&ram_disk, "A16");
    status +=  fx_file_delete(&ram_disk, "A17");
    status +=  fx_file_delete(&ram_disk, "A18");
    status +=  fx_file_delete(&ram_disk, "A19");
    status +=  fx_file_delete(&ram_disk, "A20");
    status +=  fx_file_delete(&ram_disk, "A21");
    status +=  fx_file_delete(&ram_disk, "A22");
    status +=  fx_file_delete(&ram_disk, "A23");
    status +=  fx_file_delete(&ram_disk, "A24");
    status +=  fx_file_delete(&ram_disk, "A25");
    status +=  fx_file_delete(&ram_disk, "A26");
    status +=  fx_file_delete(&ram_disk, "A27");
    status +=  fx_file_delete(&ram_disk, "A28");
    status +=  fx_file_delete(&ram_disk, "A29");
    status +=  fx_file_delete(&ram_disk, "A30");
    status +=  fx_file_delete(&ram_disk, "A31");
    status +=  fx_file_delete(&ram_disk, "A32");
    
    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error deleting files.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(11);
    }

    /* Flush the media.  */
    status =  fx_media_flush(&ram_disk);
    
    /* Now do the same thing, except with 2 entry long names.  */
    status +=  fx_file_create(&ram_disk, "b01");
    status +=  fx_file_create(&ram_disk, "b02");
    status +=  fx_file_create(&ram_disk, "b03");
    status +=  fx_file_create(&ram_disk, "b04");
    status +=  fx_file_create(&ram_disk, "b05");
    status +=  fx_file_create(&ram_disk, "b06");
    status +=  fx_file_create(&ram_disk, "b07");
    status +=  fx_file_create(&ram_disk, "b08");
    status +=  fx_file_create(&ram_disk, "b09");
    status +=  fx_file_create(&ram_disk, "b10");
    status +=  fx_file_create(&ram_disk, "b11");
    status +=  fx_file_create(&ram_disk, "b12");
    status +=  fx_file_create(&ram_disk, "b13");
    status +=  fx_file_create(&ram_disk, "b14");
    status +=  fx_file_create(&ram_disk, "b15");
    status +=  fx_file_create(&ram_disk, "b16");

    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error creating files.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(12);
    }

    /* Attempt to create 17th long name... this should fail.  */
    status =  fx_file_create(&ram_disk, "b17");

    /* Check for errors...  */
    if (status != FX_NO_MORE_SPACE)
    {

        /* Error creating files.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(13);
    }

    /* Flush the media.  */
    status =  fx_media_flush(&ram_disk);

    /* Now delete all the names.  */
    status +=  fx_file_delete(&ram_disk, "b01");
    status +=  fx_file_delete(&ram_disk, "b02");
    status +=  fx_file_delete(&ram_disk, "b03");
    status +=  fx_file_delete(&ram_disk, "b04");
    status +=  fx_file_delete(&ram_disk, "b05");
    status +=  fx_file_delete(&ram_disk, "b06");
    status +=  fx_file_delete(&ram_disk, "b07");
    status +=  fx_file_delete(&ram_disk, "b08");
    status +=  fx_file_delete(&ram_disk, "b09");
    status +=  fx_file_delete(&ram_disk, "b10");
    status +=  fx_file_delete(&ram_disk, "b11");
    status +=  fx_file_delete(&ram_disk, "b12");
    status +=  fx_file_delete(&ram_disk, "b13");
    status +=  fx_file_delete(&ram_disk, "b14");
    status +=  fx_file_delete(&ram_disk, "b15");
    status +=  fx_file_delete(&ram_disk, "b16");
    
    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error deleting files.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(14);
    }

    /* Create a file with non alphabet character and space. */
    status =   fx_file_create(&ram_disk, "TEST.T~ ");

    /* Now delete this file.  */
    status +=  fx_file_delete(&ram_disk, "TEST.T~ ");
    
    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error deleting files.  Return to caller.  */
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

