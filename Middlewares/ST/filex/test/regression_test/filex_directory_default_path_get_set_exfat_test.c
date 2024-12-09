/* This FileX test concentrates on the basic directory default path get/set operations.  */

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
static UCHAR                  *ram_disk_memory;
static UCHAR                  *cache_buffer;
#else
static UCHAR                  cache_buffer[CACHE_SIZE];
#endif


/* Define thread prototypes.  */

void    filex_directory_default_get_set_exfat_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_directory_default_get_set_exfat_application_define(void *first_unused_memory)
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
CHAR        path_name_buffer[32];
CHAR        test_path_name_buffer[1];

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   exFAT Directory default get/set test...................");

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
    status =   fx_directory_create(&ram_disk, "/A0");
    status +=  fx_directory_create(&ram_disk, "/B0");
    status +=  fx_directory_create(&ram_disk, "/C0");
    status +=  fx_directory_create(&ram_disk, "/D0");
    status +=  fx_directory_create(&ram_disk, "/E0");
    status +=  fx_directory_create(&ram_disk, "/F0");
    status +=  fx_directory_create(&ram_disk, "/G0");
    status +=  fx_directory_create(&ram_disk, "/H0");
    status +=  fx_directory_create(&ram_disk, "/I0");

    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error creating directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(4);
    }

    /* Attempt to create the same directory again.  */
    status =   fx_directory_create(&ram_disk, "/A0");

    /* Check for errors...  */
    if (status != FX_ALREADY_CREATED)
    {

        /* Error creating same directory twice.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(41);
    }

    /* Create the next level of sub-directories.  */
    status =   fx_directory_default_set(&ram_disk, "A0");
    status +=  fx_directory_create(&ram_disk, "A1");
    status +=  fx_directory_create(&ram_disk, "A2");
    status +=  fx_directory_create(&ram_disk, "A3");
    status +=  fx_directory_create(&ram_disk, "A4");
    status +=  fx_directory_create(&ram_disk, "A5");
    status +=  fx_directory_default_get(&ram_disk, &path_ptr);

    
    status +=  fx_directory_default_set(&ram_disk, "/B0");
    status +=  fx_directory_create(&ram_disk, "B1");
    status +=  fx_directory_create(&ram_disk, "B2");
    status +=  fx_directory_create(&ram_disk, "B3");
    status +=  fx_directory_create(&ram_disk, "B4");
    status +=  fx_directory_create(&ram_disk, "B5");
    status +=  fx_directory_default_get(&ram_disk, &path_ptr);
    
    status +=  fx_directory_default_set(&ram_disk, "/C0");
    status +=  fx_directory_create(&ram_disk, "C1");
    status +=  fx_directory_create(&ram_disk, "C2");
    status +=  fx_directory_create(&ram_disk, "C3");
    status +=  fx_directory_create(&ram_disk, "C4");
    status +=  fx_directory_create(&ram_disk, "C5");
    status +=  fx_directory_default_get(&ram_disk, &path_ptr);

    status +=  fx_directory_default_set(&ram_disk, "/D0");
    status +=  fx_directory_create(&ram_disk, "D1");
    status +=  fx_directory_create(&ram_disk, "D2");
    status +=  fx_directory_create(&ram_disk, "D3");
    status +=  fx_directory_create(&ram_disk, "D4");
    status +=  fx_directory_create(&ram_disk, "D5");
    status +=  fx_directory_default_get(&ram_disk, &path_ptr);

    status +=  fx_directory_default_set(&ram_disk, "/E0");
    status +=  fx_directory_create(&ram_disk, "E1");
    status +=  fx_directory_create(&ram_disk, "E2");
    status +=  fx_directory_create(&ram_disk, "E3");
    status +=  fx_directory_create(&ram_disk, "E4");
    status +=  fx_directory_create(&ram_disk, "E5");
    status +=  fx_directory_default_get(&ram_disk, &path_ptr);

    status +=  fx_directory_default_set(&ram_disk, "/F0");
    status +=  fx_directory_create(&ram_disk, "F1");
    status +=  fx_directory_create(&ram_disk, "F2");
    status +=  fx_directory_create(&ram_disk, "F3");
    status +=  fx_directory_create(&ram_disk, "F4");
    status +=  fx_directory_create(&ram_disk, "F5");
    status +=  fx_directory_default_get(&ram_disk, &path_ptr);
    
    status +=  fx_directory_default_set(&ram_disk, "/G0");
    status +=  fx_directory_create(&ram_disk, "G1");
    status +=  fx_directory_create(&ram_disk, "G2");
    status +=  fx_directory_create(&ram_disk, "G3");
    status +=  fx_directory_create(&ram_disk, "G4");
    status +=  fx_directory_create(&ram_disk, "G5");
    status +=  fx_directory_default_get(&ram_disk, &path_ptr);

    status +=  fx_directory_default_set(&ram_disk, "/H0");
    status +=  fx_directory_create(&ram_disk, "H1");
    status +=  fx_directory_create(&ram_disk, "H2");
    status +=  fx_directory_create(&ram_disk, "H3");
    status +=  fx_directory_create(&ram_disk, "H4");
    status +=  fx_directory_create(&ram_disk, "H5");
    status +=  fx_directory_default_get(&ram_disk, &path_ptr);

    status +=  fx_directory_default_set(&ram_disk, "/I0");
    status +=  fx_directory_create(&ram_disk, "I1");
    status +=  fx_directory_create(&ram_disk, "I2");
    status +=  fx_directory_create(&ram_disk, "I3");
    status +=  fx_directory_create(&ram_disk, "I4");
    status +=  fx_directory_create(&ram_disk, "I5");
    status +=  fx_directory_default_get(&ram_disk, &path_ptr);

    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error creating sub-directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(4);
    }
    
    /* Now create the third level of sub-directories... */
    
    status +=  fx_directory_default_set(&ram_disk, "/A0/A1");
    status +=  fx_directory_create(&ram_disk, "A00");
    status +=  fx_directory_default_set(&ram_disk, "/B0");
    status +=  fx_directory_create(&ram_disk, "B2/B00");
    status +=  fx_directory_create(&ram_disk, "/C0/C3/C00");
    status +=  fx_directory_default_set(&ram_disk, "/D0/D4");
    status +=  fx_directory_create(&ram_disk, "D00");
    status +=  fx_directory_default_set(&ram_disk, "/E0");
    status +=  fx_directory_create(&ram_disk, "E5/E00");
    status +=  fx_directory_create(&ram_disk, "/F0/F1/F00");
    status +=  fx_directory_default_set(&ram_disk, "/G0/G1");
    status +=  fx_directory_create(&ram_disk, "G00");
    status +=  fx_directory_default_set(&ram_disk, "/H0");
    status +=  fx_directory_create(&ram_disk, "H2/H00");
    status +=  fx_directory_create(&ram_disk, "/I0/I3/I00");
    status +=  fx_directory_default_get(&ram_disk, &path_ptr);
    
    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error creating sub-sub-directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(5);
    }

    /* Now delete all the directories.  */
    status =   fx_directory_delete(&ram_disk, "/A0/A1/A00");
    status +=  fx_directory_delete(&ram_disk, "/B0/B2/B00");
    status +=  fx_directory_delete(&ram_disk, "/C0/C3/C00");
    status +=  fx_directory_delete(&ram_disk, "/D0/D4/D00");
    status +=  fx_directory_delete(&ram_disk, "/E0/E5/E00");
    status +=  fx_directory_delete(&ram_disk, "/F0/F1/F00");
    status +=  fx_directory_delete(&ram_disk, "/G0/G1/G00");
    status +=  fx_directory_delete(&ram_disk, "/H0/H2/H00");
    status +=  fx_directory_delete(&ram_disk, "/I0/I3/I00");

    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error deleting sub-sub-directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(6);
    }

    /* Delete the next level of sub-directories.  */
    status +=  fx_directory_delete(&ram_disk, "/A0/A1");
    status +=  fx_directory_delete(&ram_disk, "/A0/A2");
    status +=  fx_directory_delete(&ram_disk, "/A0/A3");
    status +=  fx_directory_delete(&ram_disk, "/A0/A4");
    status +=  fx_directory_delete(&ram_disk, "/A0/A5");
    
    status +=  fx_directory_delete(&ram_disk, "/B0/B1");
    status +=  fx_directory_delete(&ram_disk, "/B0/B2");
    status +=  fx_directory_delete(&ram_disk, "/B0/B3");
    status +=  fx_directory_delete(&ram_disk, "/B0/B4");
    status +=  fx_directory_delete(&ram_disk, "/B0/B5");
    
    status +=  fx_directory_delete(&ram_disk, "/C0/C1");
    status +=  fx_directory_delete(&ram_disk, "/C0/C2");
    status +=  fx_directory_delete(&ram_disk, "/C0/C3");
    status +=  fx_directory_delete(&ram_disk, "/C0/C4");
    status +=  fx_directory_delete(&ram_disk, "/C0/C5");

    status +=  fx_directory_delete(&ram_disk, "/D0/D1");
    status +=  fx_directory_delete(&ram_disk, "/D0/D2");
    status +=  fx_directory_delete(&ram_disk, "/D0/D3");
    status +=  fx_directory_delete(&ram_disk, "/D0/D4");
    status +=  fx_directory_delete(&ram_disk, "/D0/D5");

    status +=  fx_directory_delete(&ram_disk, "/E0/E1");
    status +=  fx_directory_delete(&ram_disk, "/E0/E2");
    status +=  fx_directory_delete(&ram_disk, "/E0/E3");
    status +=  fx_directory_delete(&ram_disk, "/E0/E4");
    status +=  fx_directory_delete(&ram_disk, "/E0/E5");

    status +=  fx_directory_delete(&ram_disk, "/F0/F1");
    status +=  fx_directory_delete(&ram_disk, "/F0/F2");
    status +=  fx_directory_delete(&ram_disk, "/F0/F3");
    status +=  fx_directory_delete(&ram_disk, "/F0/F4");
    status +=  fx_directory_delete(&ram_disk, "/F0/F5");
    
    status +=  fx_directory_delete(&ram_disk, "/G0/G1");
    status +=  fx_directory_delete(&ram_disk, "/G0/G2");
    status +=  fx_directory_delete(&ram_disk, "/G0/G3");
    status +=  fx_directory_delete(&ram_disk, "/G0/G4");
    status +=  fx_directory_delete(&ram_disk, "/G0/G5");

    status +=  fx_directory_delete(&ram_disk, "/H0/H1");
    status +=  fx_directory_delete(&ram_disk, "/H0/H2");
    status +=  fx_directory_delete(&ram_disk, "/H0/H3");
    status +=  fx_directory_delete(&ram_disk, "/H0/H4");
    status +=  fx_directory_delete(&ram_disk, "/H0/H5");

    status +=  fx_directory_delete(&ram_disk, "/I0/I1");
    status +=  fx_directory_delete(&ram_disk, "/I0/I2");
    status +=  fx_directory_delete(&ram_disk, "/I0/I3");
    status +=  fx_directory_delete(&ram_disk, "/I0/I4");
    status +=  fx_directory_delete(&ram_disk, "/I0/I5");
    
    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error deleting sub-directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(7);
    }

    /* Delete a series of directories...  */
    status =   fx_directory_delete(&ram_disk, "/A0");
    status +=  fx_directory_delete(&ram_disk, "/B0");
    status +=  fx_directory_delete(&ram_disk, "/C0");
    status +=  fx_directory_delete(&ram_disk, "/D0");
    status +=  fx_directory_delete(&ram_disk, "/E0");
    status +=  fx_directory_delete(&ram_disk, "/F0");
    status +=  fx_directory_delete(&ram_disk, "/G0");
    status +=  fx_directory_delete(&ram_disk, "/H0");
    status +=  fx_directory_delete(&ram_disk, "/I0");

    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error deleting directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(8);
    }

    /* Attempt to delete a directory again.  */
    status =   fx_directory_delete(&ram_disk, "/A0");

    /* This should be an error.  */
    if (status != FX_NOT_FOUND)
    {

        /* Error deleting same directory twice.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(9);
    }

    /* Test to return FX_BUFFER_ERROR from _fx_directory_default_get_copy() */
    status =  fx_directory_default_get_copy(&ram_disk, test_path_name_buffer, sizeof(test_path_name_buffer));
    
    /* Determine if the test was successful.  */
    if (status != FX_BUFFER_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(10);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(12);
    }
    else
    {

        printf("SUCCESS!\n");
        test_control_return(0);
    }
}

#endif /* FX_ENABLE_EXFAT */
