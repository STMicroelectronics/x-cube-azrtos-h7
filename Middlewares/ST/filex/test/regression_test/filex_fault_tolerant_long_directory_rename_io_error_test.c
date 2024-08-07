/* This FileX test concentrates on the Fault-Tolerant directory write interrupt operation.  */
/*   
For FAT 12, 16, 32 and exFAT, one cluster size is 3072 bytes;
                               
Check directory interrupt for fx_directory_rename(): 
Step1: Format and open the media; 
Step2: Enable fault tolerant feature;     
Step3: Create directories and sub-directories; 
Step4: Traverse the directory;       
Step5: Create new thread to rename directory from directory_name_1 to directory_name_2;
Step6: Force IO error while generating fault tolerant log file.
*/

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_system.h"
#include   "fx_fault_tolerant.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"               
extern void    test_control_return(UINT status);
void    filex_fault_tolerant_long_directory_rename_io_error_test_application_define(void *first_unused_memory);
                                            
#if defined (FX_ENABLE_FAULT_TOLERANT) && defined (FX_FAULT_TOLERANT) && defined (FX_FAULT_TOLERANT_DATA) && (FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE >= 3072)

#define     DEMO_STACK_SIZE         4096
#ifdef FX_ENABLE_EXFAT
#define CACHE_SIZE                  FX_EXFAT_SECTOR_SIZE
#define FAULT_TOLERANT_SIZE         FX_EXFAT_SECTOR_SIZE
#else
#define CACHE_SIZE                  2048
#define FAULT_TOLERANT_SIZE         FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE
#endif




/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD               ftest_0;  
#endif
static FX_MEDIA                ram_disk;
static UCHAR                   *pointer;    

#if (FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE >= 3072)
#define TEST_COUNT              3
#else

#ifdef FX_ENABLE_EXFAT
#define TEST_COUNT              5
#else              
#define TEST_COUNT              4
#endif
#endif

/* Define the counters used in the test application...  */
                                                   
#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *cache_buffer;
static UCHAR                  *fault_tolerant_buffer;
static UCHAR                  *thread_buffer;
#else
static UCHAR                  cache_buffer[CACHE_SIZE];
static UCHAR                  fault_tolerant_buffer[FAULT_TOLERANT_SIZE];
#endif 
static UINT                   error_counter = 0;
static CHAR                   directory_name_1[300] = "/D0/D40000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
static CHAR                   directory_name_2[300] = "/D0/D50000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
                                             
                                                     
static UINT                   expected_index =  0;
static CHAR *                 expected_name[] =  {
                              
                        "A0",
                        ".",
                        "..", 
                        "A1",   
                        ".",
                        "..",   
                        "A00",    
                        ".",
                        "..",   
                        "A2",     
                        ".",
                        "..",   
                        "A3",     
                        ".",
                        "..",  
                        "B0",
                        ".",
                        "..", 
                        "B1",   
                        ".",
                        "..",   
                        "B2",     
                        ".",
                        "..",   
                        "B00",    
                        ".",
                        "..",   
                        "B3",     
                        ".",
                        "..", 
                        "C0",
                        ".",
                        "..", 
                        "C1",   
                        ".",
                        "..",   
                        "C2",     
                        ".",
                        "..",  
                        "C3",     
                        ".",
                        "..",   
                        "C00",    
                        ".",
                        "..",  
                        "D0",
                        ".",
                        "..", 
                        "D1",   
                        ".",
                        "..",   
                        "D2",     
                        ".",
                        "..",  
                        "D3",     
                        ".",
                        "..",  
                        directory_name_1 + 4,     
                        ".",
                        "..",    
                        "END"};
                             
static CHAR *                 expected_name_exfat[] =  {
                               
                        "A0",
                        "A1",   
                        "A00",    
                        "A2",     
                        "A3",     
                        "B0",
                        "B1",   
                        "B2",     
                        "B00",    
                        "B3",     
                        "C0",
                        "C1",   
                        "C2",    
                        "C3",     
                        "C00",    
                        "D0",
                        "D1",   
                        "D2",    
                        "D3",     
                        directory_name_1 + 4,
                        "END"};

/* Define thread prototypes.  */

static void    ftest_0_entry(ULONG thread_input);  
extern void    _fx_ram_driver(FX_MEDIA *media_ptr);
extern void    test_control_return(UINT status);         
static void    traverse_directory(CHAR *directory_name);   
static void    traverse_directory_exfat(CHAR *directory_name); 
extern UINT    _filex_fault_tolerant_log_check(FX_MEDIA *media_ptr); 
extern UINT    (*driver_write_callback)(FX_MEDIA *media_ptr, UINT sector_type, UCHAR *block_ptr, UINT *operation_ptr);  


/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_long_directory_rename_io_error_test_application_define(void *first_unused_memory)
#endif
{

    
#ifndef FX_STANDALONE_ENABLE
    /* Setup the working pointer.  */
    pointer =  (UCHAR *) first_unused_memory;

    /* Create the main thread.  */

    tx_thread_create(&ftest_0, "thread 0", ftest_0_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer =  pointer + DEMO_STACK_SIZE;

    /* Setup memory for the RAM disk and the sector cache.  */
    cache_buffer =  pointer;
    pointer += CACHE_SIZE;
    fault_tolerant_buffer = pointer;
    pointer += FAULT_TOLERANT_SIZE;    
    thread_buffer = pointer;
    pointer += DEMO_STACK_SIZE;
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
    printf("FileX Test:   Fault Tolerant Long Dir Rename IO Error Test...........");
               
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
                                     384,                    // Sector size   
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
                                     384,                    // Sector size   
                                     8,                      // Sectors per cluster
                                     1,                      // Heads
                                     1);                     // Sectors per track 
        }  
        else if (i == 2)
        {
            /* Format the media with FAT16 with sector size 2048 bytes.  This needs to be done before opening it!  */
            status =  fx_media_format(&ram_disk,
                                     _fx_ram_driver,         // Driver entry
                                     ram_disk_memory_large,  // RAM disk memory pointer
                                     cache_buffer,           // Media buffer pointer
                                     CACHE_SIZE,             // Media buffer size
                                     "MY_RAM_DISK",          // Volume Name
                                     1,                      // Number of FATs
                                     32,                     // Directory Entries
                                     0,                      // Hidden sectors
                                     4200 * 16,              // Total sectors
                                     1024,                   // Sector size
                                     2,                      // Sectors per cluster
                                     1,                      // Heads
                                     1);                     // Sectors per track
        }
        else if (i == 3)
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
                                     384,                    // Sector size   
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
                                            768,                    // Sector size   
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

        /* Open the ram_disk.  */
        status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);

        /* Check the status.  */
        if (status != FX_SUCCESS)
        {

            /* Error, return error code.  */
            printf("ERROR!\n");
            test_control_return(2);
        }

        /* Enable the Fault-tolerant feature to recover the media.  */         
        status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);

        /* Check status.   */
        if (status)
        {

            printf("ERROR!\n");
            test_control_return(3);
        }

        /* Create a series of directories...  */
        status =   fx_directory_create(&ram_disk, "/A0");
        status +=  fx_directory_create(&ram_disk, "/B0");
        status +=  fx_directory_create(&ram_disk, "/C0");
        status +=  fx_directory_create(&ram_disk, "/D0");

        /* Check for errors...  */
        if (status != FX_SUCCESS)
        {

            /* Error creating directories.  Return to caller.  */
            printf("ERROR!\n");
            test_control_return(4);
        }

        /* Create the next level of sub-directories.  */
        status =   fx_directory_create(&ram_disk, "/A0/A1");
        status +=  fx_directory_create(&ram_disk, "/A0/A2");
        status +=  fx_directory_create(&ram_disk, "/A0/A3");

        status +=  fx_directory_create(&ram_disk, "/B0/B1");
        status +=  fx_directory_create(&ram_disk, "/B0/B2");
        status +=  fx_directory_create(&ram_disk, "/B0/B3");

        status +=  fx_directory_create(&ram_disk, "/C0/C1");
        status +=  fx_directory_create(&ram_disk, "/C0/C2");
        status +=  fx_directory_create(&ram_disk, "/C0/C3");

        status +=  fx_directory_create(&ram_disk, "/D0/D1");
        status +=  fx_directory_create(&ram_disk, "/D0/D2");
        status +=  fx_directory_create(&ram_disk, "/D0/D3");   
        status +=  fx_directory_create(&ram_disk, directory_name_1);

        /* Check for errors...  */
        if (status != FX_SUCCESS)
        {

            /* Error creating sub-directories.  Return to caller.  */
            printf("ERROR!\n");
            test_control_return(5);
        }

        /* Now create the third level of sub-directories... */
        status =   fx_directory_create(&ram_disk, "/A0/A1/A00");
        status +=  fx_directory_create(&ram_disk, "/B0/B2/B00");
        status +=  fx_directory_create(&ram_disk, "/C0/C3/C00");

        /* Check for errors...  */
        if (status != FX_SUCCESS)
        {

            /* Error creating sub-sub-directories.  Return to caller.  */
            printf("ERROR!\n");
            test_control_return(6);
        }      

        /* Set the index as zero before traverse.  */
        expected_index = 0;
                                       
        /* Traverse the directory structure recursively.  */
        if (i < 4)            
            traverse_directory(FX_NULL);
        else                  
            traverse_directory_exfat(FX_NULL);
                                        
#if !defined(FX_DISABLE_CACHE)                      
        /* Rename the directory directory_name_1 to directory_name_2.  */
        _fx_ram_driver_io_error_request =  4;
        status = fx_directory_rename(&ram_disk, directory_name_1, directory_name_2);

        /* Determine if the result fails.  */
        if (status != FX_IO_ERROR)
        {

            printf("ERROR!\n");
            test_control_return(7);
        }
#endif

        /* Close the media.  */
        status =  fx_media_close(&ram_disk);

        /* Determine if the test was successful.  */
        if ((status != FX_SUCCESS) || (error_counter))
        {

            printf("ERROR!\n");
            test_control_return(11);
        }    
    }      

    /* Output successful.  */     
    printf("SUCCESS!\n");
    test_control_return(0);
}         
                                                                         
/* Define the FAT12, 16 and 32 directory traversal routine for the local path test.  */
#ifndef FX_STANDALONE_ENABLE 
static void  traverse_directory(CHAR *directory_name)
{

FX_LOCAL_PATH   local_path;
FX_FILE         file;
UINT            status;
CHAR            name[300];
UINT            skip;


    /* Determine if we are at the start.  */
    if (directory_name == FX_NULL)
    {
        status =  fx_directory_local_path_set(&ram_disk, &local_path, "\\");
        skip =  0;
    }
    else
    {
        status =  fx_directory_local_path_set(&ram_disk, &local_path, directory_name);
        skip =  2;
    }

    /* Get the first directory entry in the root path.  */
    status =  fx_directory_first_entry_find(&ram_disk, name);

    /* Loop through the directory entries for this path.  */
    while (status == FX_SUCCESS)
    {

        /* Compare with what is expected.  */
        if (strcmp(name, expected_name[expected_index++]))
            error_counter++;
       
        /* Determine if this name is a directory or a file.  */
        status =  fx_directory_name_test(&ram_disk, name);

        /* What is it?  */
        if (status == FX_NOT_DIRECTORY)
        {

            /* This is a file, open it to test its integrity.  */
            status =  fx_file_open(&ram_disk, &file, name, FX_OPEN_FOR_READ);
            if (status != FX_SUCCESS)
                error_counter++;
            status =  fx_file_close(&file);
            if (status != FX_SUCCESS)
                error_counter++;
        
            /* Pickup the next directory entry.  */
            status =  fx_directory_next_entry_find(&ram_disk, name);
        }
        else if (status == FX_SUCCESS)
        {

            /* Skip the first two entries...  */
            if (skip)
                skip--;
            else
            {
                /* Recursive call to traverse directory.  */
                traverse_directory(name);
            
                /* Restore path.  */
                status =  fx_directory_local_path_restore(&ram_disk, &local_path);        
            }

            status =  fx_directory_next_entry_find(&ram_disk, name);
        } 
    } 

    fx_directory_local_path_clear(&ram_disk);
} 
#else

static void  traverse_directory(CHAR *directory_name)
{
UINT            status;
CHAR            name[300];

    /* Pickup the first entry in the root directory.  */
    status =  fx_directory_first_entry_find(&ram_disk, name);
    if (strcmp(name, "A0"))
        status++;
    status =  fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "B0"))
        status++;
    status =  fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "C0"))
        status++;
    status =  fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "D0"))
        status++;

    /* Okay, now postion to the deepest sub-directory and ensure that the directory first/next 
       operations work properly there as well.  */

    /* Set the current path */
    status =  fx_directory_default_set(&ram_disk, "/C0/C3");

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
    if (strcmp(name, "C00"))
        status++;

    /* Now read one past the number of entries in order to get an error condition.  */
    status += fx_directory_next_entry_find(&ram_disk, name);

    /* Determine if the test was successful.  */
    if (status != FX_NO_MORE_ENTRIES)
    {

        printf("ERROR!\n");
        test_control_return(18);
    }

}

#endif

/* Define the exFAT directory traversal routine for the local path test.  */
#ifndef FX_STANDALONE_ENABLE
void  traverse_directory_exfat(CHAR *directory_name)
{

FX_LOCAL_PATH   local_path;
FX_FILE         file;
UINT            status;
CHAR            name[300];


    /* Determine if we are at the start.  */
    if (directory_name == FX_NULL)
    {
        status =  fx_directory_local_path_set(&ram_disk, &local_path, "\\");
    }
    else
    {
        status =  fx_directory_local_path_set(&ram_disk, &local_path, directory_name);
    }

    /* Get the first directory entry in the root path.  */
    status =  fx_directory_first_entry_find(&ram_disk, name);

    /* Loop through the directory entries for this path.  */
    while (status == FX_SUCCESS)
    {

        /* Compare with what is expected.  */
        if (strcmp(name, expected_name_exfat[expected_index++]))
            error_counter++;
       
        /* Determine if this name is a directory or a file.  */
        status =  fx_directory_name_test(&ram_disk, name);

        /* What is it?  */
        if (status == FX_NOT_DIRECTORY)
        {

            /* This is a file, open it to test its integrity.  */
            status =  fx_file_open(&ram_disk, &file, name, FX_OPEN_FOR_READ);
            if (status != FX_SUCCESS)
                error_counter++;
            status =  fx_file_close(&file);
            if (status != FX_SUCCESS)
                error_counter++;
        
            /* Pickup the next directory entry.  */
            status =  fx_directory_next_entry_find(&ram_disk, name);
        }
        else if (status == FX_SUCCESS)
        {

            /* Recursive call to traverse directory.  */
            traverse_directory_exfat(name);
        
            /* Restore path.  */
            status =  fx_directory_local_path_restore(&ram_disk, &local_path);        

            status =  fx_directory_next_entry_find(&ram_disk, name);
        } 
    } 

    fx_directory_local_path_clear(&ram_disk);
}
#else
void  traverse_directory_exfat(CHAR *directory_name)
{
FX_FILE         file;
UINT            status;
CHAR            name[300];

    /* Pickup the first entry in the root directory.  */
    status =  fx_directory_first_entry_find(&ram_disk, name);
    if (strcmp(name, "A0"))
        status++;
    status =  fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "B0"))
        status++;
    status =  fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "C0"))
        status++;
    status =  fx_directory_next_entry_find(&ram_disk, name);
    if (strcmp(name, "D0"))
        status++;

    /* Okay, now postion to the deepest sub-directory and ensure that the directory first/next 
       operations work properly there as well.  */

    /* Set the current path */
    status =  fx_directory_default_set(&ram_disk, "/C0/C3/");

    /* Check the status.  */
    if (status != FX_SUCCESS) 
    {

        /* Error setting the path.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(16);
    }

    /* Pickup the first entry in this sub-sub directory.  */
    status =  fx_directory_first_entry_find(&ram_disk, name);
    if (strcmp(name, "C00"))
        status++;
}
#endif

#else  

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_long_directory_rename_io_error_test_application_define(void *first_unused_memory)
#endif
{
    
    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Long Dir Rename IO Error Test...........N/A\n");

    test_control_return(255);
}
#endif

