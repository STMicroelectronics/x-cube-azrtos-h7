/* This FileX test concentrates on the basic directory default path get/set operations.  */

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
static CHAR                     name[256];


/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *ram_disk_memory;
static UCHAR                  *cache_buffer;
#else
static UCHAR                  cache_buffer[CACHE_SIZE];
#endif


/* Define thread prototypes.  */

void    filex_directory_default_get_set_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_directory_default_get_set_application_define(void *first_unused_memory)
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

UINT            status;
CHAR            *path_ptr;
CHAR            path_name_buffer[32];
CHAR            test_path_name_buffer[1];
ULONG           temp;
FX_LOCAL_PATH   local_path;
FX_LOCAL_PATH   local_path1;
UINT            i;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Directory default get/set test.........................");

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
    
    /* Attempt to set the default directory before the media has been opened to generate an error */
    status = fx_directory_default_set(&ram_disk, "/A0");
    if (status != FX_MEDIA_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(2);
    }
    
    /* Attempt to get the default directory before the media has been opened to generate an error */
    status = fx_directory_default_get(&ram_disk, &path_ptr);
    if (status != FX_MEDIA_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(3);
    }

    /* Attempt to get copy of the default directory before the media has been opened to generate an error */
    status =  fx_directory_default_get_copy(&ram_disk, test_path_name_buffer, sizeof(test_path_name_buffer));
    
    /* Determine if the test was successful.  */
    if (status != FX_MEDIA_NOT_OPEN)
    {

        printf("ERROR!\n");
        test_control_return(52);
    }

#ifndef FX_NO_LOCAL_PATH
    /* Attempt to get copy of the directory local path before the media has been opened to generate an error */
    status =  _fx_directory_local_path_get_copy(&ram_disk, test_path_name_buffer, sizeof(test_path_name_buffer));
    
    /* Determine if the test was successful.  */
    if (status != FX_MEDIA_NOT_OPEN)
    {

        printf("ERROR!\n");
        test_control_return(53);
    }
#endif /* FX_NO_LOCAL_PATH */

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(4);
    }

    /* Get the current path. This should be / or NULL.   */
    status =  fx_directory_default_get(&ram_disk, &path_ptr);

    /* Check the status.  */
    if ((status != FX_SUCCESS) || (path_ptr[0]))
    {

        /* Error getting the path.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(5);
    }
    
    temp = ram_disk.fx_media_id;
    ram_disk.fx_media_id = 0;
    
    /* Give a media pointer with bad data to cause an error */
    status = fx_directory_default_get(&ram_disk, &path_ptr);
    
    /* Check the status */
    if (status != FX_MEDIA_NOT_OPEN)
    {
      
      /* Error opening the media */
      printf("ERROR!\n");
      test_control_return(6);
    }
    
    ram_disk.fx_media_id = temp;
    
#ifndef FX_DISABLE_ERROR_CHECKING
    /* Give a bad ram pointer to cause an error   */
    status =  fx_directory_default_get(NULL, &path_ptr);

    /* Check the status.  */
    if (status != FX_PTR_ERROR)
    {

        /* Error getting the path.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(7);
    }
    
    /* Give a bad path pointer to cause an error   */
    status =  fx_directory_default_get(&ram_disk, NULL);

    /* Check the status.  */
    if (status != FX_PTR_ERROR)
    {

        /* Error getting the path.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(8);
    }
#endif

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
    status +=  fx_file_create(&ram_disk, "NOT_A_DIR");

    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error creating directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(9);
    }
    
    /* Attempt to set the default directory to something that is not a directory to generate an error */
    status = fx_directory_default_set(&ram_disk, "NOT_A_DIR");
    if (status != FX_INVALID_PATH)
    {
        printf("ERROR!\n");
        test_control_return(10);
    }
    status =  fx_file_delete(&ram_disk, "NOT_A_DIR");
    if (status != FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(11);
    }

    /* Attempt to create the same directory again.  */
    status =   fx_directory_create(&ram_disk, "/A0");

    /* Check for errors...  */
    if (status != FX_ALREADY_CREATED)
    {

        /* Error creating same directory twice.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(12);
    }
    
/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send null pointer to default directory set to generate an error */
    status = fx_directory_default_set(FX_NULL, "/A0");
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(13);
    }
#endif /* FX_DISABLE_ERROR_CHECKING */

    /* Create the next level of sub-directories.  */
    status =   fx_directory_default_set(&ram_disk, "/A0");
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
    status +=  fx_directory_default_get_copy(&ram_disk, path_name_buffer, sizeof(path_name_buffer));

    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error creating sub-directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(14);
    }
    
    if (strcmp(path_name_buffer, "/I0"))
    {
        printf("ERROR!\n");
        test_control_return(233);
    }

    /* Now create the third level of sub-directories... */

#ifndef FX_NO_LOCAL_PATH
    /* create these with a local path defined in threadx */
    status  =  fx_directory_local_path_set(&ram_disk, &local_path, "/A0");
    status +=  fx_directory_default_set(&ram_disk, "/A0");
    status +=  fx_directory_create(&ram_disk, "test");
    status +=  fx_directory_delete(&ram_disk, "test");
    status +=  fx_directory_default_set(&ram_disk, "A1");
    status +=  fx_directory_create(&ram_disk, "test");
    status +=  fx_directory_delete(&ram_disk, "test");
    status +=  fx_directory_local_path_clear(&ram_disk);
    
    /* Check for errors...  */
    if (status != FX_SUCCESS)
    {

        /* Error creating sub-sub-directories.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(15);
    }
#endif
    
    /* create the rest as normal */
    status  =  fx_directory_default_set(&ram_disk, "/A0/A1");
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
        test_control_return(16);
    }

    /* Now delete all the directories.  */
    status  =  fx_directory_delete(&ram_disk, "/A0/A1/A00");
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
        test_control_return(17);
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
        test_control_return(18);
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
        test_control_return(19);
    }

    /* Attempt to delete a directory again.  */
    status =   fx_directory_delete(&ram_disk, "/A0");

    /* This should be an error.  */
    if (status != FX_NOT_FOUND)
    {

        /* Error deleting same directory twice.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(20);
    }
                                          
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(21);
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
        test_control_return(22);
    }
    
    /* Open the ram_disk - with one logical sector cache!  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(23);
    }

    /* Create a sub-directory.  */
    status =  fx_directory_create(&ram_disk, "sub-directory-level_001");
    
    /* Attempt to set a path with a NULL path name.  */
    status +=  _fx_directory_default_set(&ram_disk, FX_NULL);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(24);
    }

    /* Flush and invalidate the caches.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }
    
    /* Try to set the current path but with an I/O error coming from the directory search.  */
    _fx_utility_logical_sector_read_error_request =  1;
    status =  fx_directory_default_set(&ram_disk, "sub-directory-level_001");
    _fx_utility_logical_sector_read_error_request =  0;
    
    /* Check the status - should be an I/O error!  */
    if (status != FX_INVALID_PATH)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(25);
    }

    /* Try to set the current path to the first sub-directory.  */
    status =  fx_directory_default_set(&ram_disk, "sub-directory-level_001");
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(26);
    }
    
    /* Try to set the current path to the same relative sub-directory.  */
    status =  fx_directory_default_set(&ram_disk, ".");
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(27);
    }

    /* Try to set the current path back to root directory relative to the current directory.  */
    status =  fx_directory_default_set(&ram_disk, "..");
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(28);
    }

    /* Create a set of sub-directories that exceeds the maximum path size.  */
    status =  fx_directory_default_set(&ram_disk, "sub-directory-level_001");
    status += fx_directory_create(&ram_disk, "sub-directory-level_002");     
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_002");
    status += fx_directory_create(&ram_disk, "sub-directory-level_003");     
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_003");
    status += fx_directory_create(&ram_disk, "sub-directory-level_004");     
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_004");
    status += fx_directory_create(&ram_disk, "sub-directory-level_005");     
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_005");
    status += fx_directory_create(&ram_disk, "sub-directory-level_006");     
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_006");
    status += fx_directory_create(&ram_disk, "sub-directory-level_007");     
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_007");
    status += fx_directory_create(&ram_disk, "sub-directory-level_008");     
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_008");
    status += fx_directory_create(&ram_disk, "sub-directory-level_009");     
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_009");
    status += fx_directory_create(&ram_disk, "sub-directory-level_010");     
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_010");
    status += fx_directory_create(&ram_disk, "sub-directory-level_011");     
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_011");
    status += fx_directory_create(&ram_disk, "sub-directory-level_012");     
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_012");
    status += fx_directory_create(&ram_disk, "sub-directory-level_013");     
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_013");
    status += fx_directory_create(&ram_disk, "sub-directory-level_014");     
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_014");
    status += fx_directory_create(&ram_disk, "sub-directory-level_015");     
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_015");

    /* Now backup to the root directory.  */
    status += fx_directory_default_set(&ram_disk, "../../../../../../../../../../../../../../..");
    status += fx_directory_first_entry_find(&ram_disk, name);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(29);
    }

    /* Move back to a maximum path situation.  */
    status =  fx_directory_default_set(&ram_disk, "sub-directory-level_001");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_002");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_003");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_004");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_005");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_006");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_007");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_008");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_009");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_010");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_011");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_012");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_013");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_014");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_015");

    /* Now backup to the root directory - one sub-directory at a time.  */
    status += fx_directory_default_set(&ram_disk, "..");
    status += fx_directory_default_set(&ram_disk, "..");
    status += fx_directory_default_set(&ram_disk, "..");
    status += fx_directory_default_set(&ram_disk, "..");
    status += fx_directory_default_set(&ram_disk, "..");
    status += fx_directory_default_set(&ram_disk, "..");
    status += fx_directory_default_set(&ram_disk, "..");
    status += fx_directory_default_set(&ram_disk, "..");
    status += fx_directory_default_set(&ram_disk, "..");
    status += fx_directory_default_set(&ram_disk, "..");
    status += fx_directory_default_set(&ram_disk, "..");
    status += fx_directory_default_set(&ram_disk, "..");
    status += fx_directory_default_set(&ram_disk, "..");
    status += fx_directory_default_set(&ram_disk, "..");
    status += fx_directory_default_set(&ram_disk, "..");
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(30);
    }

    /* Move back to a maximum path situation.  */
    status =  fx_directory_default_set(&ram_disk, "sub-directory-level_001");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_002");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_003");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_004");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_005");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_006");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_007");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_008");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_009");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_010");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_011");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_012");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_013");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_014");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_015");

    /* Now backup to the root directory - one sub-directory at a time.  */
    status += fx_directory_default_set(&ram_disk, "..\\..\\..\\..\\..\\..\\..\\..\\..\\..\\..\\..\\..\\..\\..");
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(31);
    }

    /* Move to an exact fit of the 256 path size situation.  */
    status =  fx_directory_default_set(&ram_disk, "sub-directory-level_001");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_002");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_003");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_004");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_005");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_006");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_007");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_008");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_009");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_010");
    status += fx_directory_create(&ram_disk, "perfect_fit_dr");
    status += fx_directory_default_set(&ram_disk, "perfect_fit_dr");
    status += fx_directory_create(&ram_disk, "perfect_fit_subdir");
    status += fx_directory_default_set(&ram_disk, "perfect_fit_subdir");

    /* Now backup to the root directory - one sub-directory at a time.  */
    status += fx_directory_default_set(&ram_disk, "..\\..\\..\\..\\..\\..\\..\\..\\..\\..\\..\\..");
    status += fx_directory_first_entry_find(&ram_disk, name);
    status += fx_directory_first_entry_find(&ram_disk, name);
    status += fx_directory_first_entry_find(&ram_disk, name);
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(32);
    }
    
    /* Now try to have a new path relative back to the root directory, and then back to itself.  */
    status =  fx_directory_default_set(&ram_disk, "sub-directory-level_001");

    /* Update the directory symbol at the root of the path string.  */
    ram_disk.fx_media_default_path.fx_path_string[0] =  '/';
    status += fx_directory_default_set(&ram_disk, "../sub-directory-level_001");

    /* Update the directory symbol at the root of the path string to cause a maximum path condition.  */
    ram_disk.fx_media_default_path.fx_path_string[0] =  ' ';
    status += fx_directory_default_set(&ram_disk, "../sub-directory-level_001");

    status += fx_directory_default_set(&ram_disk, "..");   
    status += fx_directory_first_entry_find(&ram_disk, name);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(33);
    }

    /* Test to return FX_BUFFER_ERROR from _fx_directory_default_get_copy() */
    status =  fx_directory_default_get_copy(&ram_disk, test_path_name_buffer, sizeof(test_path_name_buffer));
    
    /* Determine if the test was successful.  */
    if (status != FX_BUFFER_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(52);
    }
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(34);
    }

    /* Now perform the same corner case testing with the local path set.  */

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
        test_control_return(22);
    }
    
    /* Open the ram_disk - with one logical sector cache!  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(35);
    }

    /* Create a sub-directory.  */
    status =  fx_directory_create(&ram_disk, "sub-directory-level_001");
   
    /* Skip local_path test */
#ifndef FX_NO_LOCAL_PATH 
    /* Attempt to set a path with a NULL path name.  */
    status +=  _fx_directory_local_path_set(&ram_disk, &local_path, FX_NULL);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(36);
    }

    /* Flush and invalidate the caches.  */
    fx_media_flush(&ram_disk);
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
    for (i = 0; i < FX_MAX_FAT_CACHE; i++)
    {
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_cluster =  0;
        ram_disk.fx_media_fat_cache[i].fx_fat_cache_entry_value =  0;
    }
    
    /* Try to set the current path but with an I/O error coming from the directory search.  */
    _fx_utility_logical_sector_read_error_request =  1;
    status =  fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_001");
    _fx_utility_logical_sector_read_error_request =  0;
    
    /* Check the status - should be an I/O error!  */
    if (status != FX_INVALID_PATH)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(37);
    }

    /* Try to set the current path to the first sub-directory.  */
    status =  fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_001");
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(38);
    }
    
    /* Try to set the current path to the same relative sub-directory.  */
    status =  fx_directory_local_path_set(&ram_disk, &local_path, ".");
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(39);
    }

    /* Try to set the current path back to root directory relative to the current directory.  */
    status =  fx_directory_local_path_set(&ram_disk, &local_path, "..");
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(40);
    }

    /* Create a set of sub-directories that exceeds the maximum path size.  */
    status =  fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_001");
    status += fx_directory_create(&ram_disk, "sub-directory-level_002");     
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_002");
    status += fx_directory_create(&ram_disk, "sub-directory-level_003");     
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_003");
    status += fx_directory_create(&ram_disk, "sub-directory-level_004");     
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_004");
    status += fx_directory_create(&ram_disk, "sub-directory-level_005");     
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_005");
    status += fx_directory_create(&ram_disk, "sub-directory-level_006");     
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_006");
    status += fx_directory_create(&ram_disk, "sub-directory-level_007");     
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_007");
    status += fx_directory_create(&ram_disk, "sub-directory-level_008");     
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_008");
    status += fx_directory_create(&ram_disk, "sub-directory-level_009");     
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_009");
    status += fx_directory_create(&ram_disk, "sub-directory-level_010");     
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_010");
    status += fx_directory_create(&ram_disk, "sub-directory-level_011");     
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_011");
    status += fx_directory_create(&ram_disk, "sub-directory-level_012");     
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_012");
    status += fx_directory_create(&ram_disk, "sub-directory-level_013");     
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_013");
    status += fx_directory_create(&ram_disk, "sub-directory-level_014");     
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_014");
    status += fx_directory_create(&ram_disk, "sub-directory-level_015");     
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_015");

    /* Now backup to the root directory.  */
    status += fx_directory_local_path_set(&ram_disk, &local_path, "../../../../../../../../../../../../../../..");
    status += fx_directory_first_entry_find(&ram_disk, name);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(41);
    }

    /* Move back to a maximum path situation.  */
    status =  fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_001");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_002");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_003");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_004");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_005");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_006");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_007");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_008");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_009");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_010");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_011");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_012");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_013");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_014");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_015");

    /* Now backup to the root directory - one sub-directory at a time.  */
    status += fx_directory_local_path_set(&ram_disk, &local_path, "..");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "..");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "..");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "..");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "..");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "..");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "..");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "..");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "..");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "..");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "..");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "..");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "..");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "..");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "..");
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(42);
    }

    /* Move back to a maximum path situation.  */
    status =  fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_001");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_002");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_003");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_004");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_005");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_006");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_007");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_008");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_009");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_010");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_011");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_012");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_013");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_014");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_015");

    /* Now backup to the root directory - one sub-directory at a time.  */
    status += fx_directory_local_path_set(&ram_disk, &local_path, "..\\..\\..\\..\\..\\..\\..\\..\\..\\..\\..\\..\\..\\..\\..");
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(43);
    }

    /* Move to an exact fit of the 256 path size situation.  */
    status =  fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_001");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_002");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_003");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_004");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_005");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_006");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_007");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_008");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_009");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_010");
    status += fx_directory_create(&ram_disk, "perfect_fit_dr");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "perfect_fit_dr");
    status += fx_directory_create(&ram_disk, "perfect_fit_subdir");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "perfect_fit_subdir");

    /* Now backup to the root directory - one sub-directory at a time.  */
    status += fx_directory_local_path_set(&ram_disk, &local_path, "..\\..\\..\\..\\..\\..\\..\\..\\..\\..\\..\\..");
    status += fx_directory_first_entry_find(&ram_disk, name);
    status += fx_directory_first_entry_find(&ram_disk, name);
    status += fx_directory_first_entry_find(&ram_disk, name);
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(44);
    }
    
    /* Now try to have a new path relative back to the root directory, and then back to itself.  */
    status =  fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_001");

    /* Update the directory symbol at the root of the path string.  */
    local_path.fx_path_string[0] =  '/';
    status += fx_directory_local_path_set(&ram_disk, &local_path, "../sub-directory-level_001");

    /* Update the directory symbol at the root of the path string to cause a maximum path condition.  */
    local_path.fx_path_string[0] =  ' ';
    status += fx_directory_local_path_set(&ram_disk, &local_path, "../sub-directory-level_001");

    status += fx_directory_local_path_set(&ram_disk, &local_path, "..");   
    status += fx_directory_first_entry_find(&ram_disk, name);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(45);
    }

    /* Create a file at the root directory.  */
    status +=  fx_file_create(&ram_disk, "TEST.TXT");
    
    /* Attempt to set the default to a file name.  */
    status +=  fx_directory_local_path_set(&ram_disk, &local_path, "TEST.TXT");

    /* Check the status.  */
    if (status != FX_INVALID_PATH)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(46);
    }

    /* Build a maximum local path.  */   
    status =  fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_001");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_002");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_003");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_004");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_005");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_006");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_007");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_008");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_009");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_010");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_011");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_012");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_013");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_014");
    status += fx_directory_local_path_set(&ram_disk, &local_path, "sub-directory-level_015");

    /* Now set a new local path that will inherit this local path.  */
    status += fx_directory_local_path_set(&ram_disk, &local_path1, "..\\sub-directory-level_015");
    
    /* Now set an absolute path.  */
    status += fx_directory_local_path_set(&ram_disk, &local_path1, "\\sub-directory-level_001");
    status += fx_directory_local_path_set(&ram_disk, &local_path1, "..");
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(47);
    }
    
    /* Clear the local path.  */
    status += fx_directory_local_path_clear(&ram_disk);
    
    /* Build a maximum path default (global) path.  */
    status =  fx_directory_default_set(&ram_disk, "\\");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_001");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_002");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_003");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_004");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_005");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_006");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_007");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_008");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_009");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_010");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_011");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_012");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_013");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_014");
    status += fx_directory_default_set(&ram_disk, "sub-directory-level_015");
    
    /* Now set the local path again, which will inherit the global path.  */
    status += fx_directory_local_path_set(&ram_disk, &local_path, "../sub-directory-level_015");

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(48);
    }

    /* Test to return FX_BUFFER_ERROR from _fx_directory_local_path_get_copy() */
    status =  _fx_directory_local_path_get_copy(&ram_disk, test_path_name_buffer, sizeof(test_path_name_buffer));
    
    /* Determine if the test was successful.  */
    if (status != FX_BUFFER_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(49);
    }
    
    /* Test to each (return_path_name_buffer[0] = '\0') line in _fx_directory_local_path_get_copy() */
    fx_directory_local_path_clear(&ram_disk);
    path_name_buffer[0] = 'A';
    status =  _fx_directory_local_path_get_copy(&ram_disk, path_name_buffer, sizeof(path_name_buffer));
    status += strcmp(&path_name_buffer[0], "\0");
    
    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(50);
    }

#endif /* FX_NO_LOCAL_PATH */

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(51);
    }
    else
    {

        printf("SUCCESS!\n");
        test_control_return(0);
    }
}

