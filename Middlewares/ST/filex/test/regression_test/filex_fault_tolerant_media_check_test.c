/* This FileX test concentrates on the basic media check operation.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_utility.h"
#include   "fx_fault_tolerant.h"
#include   "fx_ram_driver_test.h"
#include   <stdio.h>
extern void  test_control_return(UINT status);
void    filex_fault_tolerant_media_check_application_define(void *first_unused_memory);

#if defined (FX_ENABLE_FAULT_TOLERANT) && defined (FX_FAULT_TOLERANT)
#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              2048
#define     SCRATCH_MEMORY_SIZE     2000


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD                ftest_0;
#endif
static FX_MEDIA                 ram_disk;
static UCHAR                    fault_tolerant_buffer[FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE];


/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                    ram_disk_memory[2048 * 256];
static UCHAR                    cache_buffer[CACHE_SIZE];
static UCHAR                    *scratch_memory;
#else
static UCHAR                    cache_buffer[CACHE_SIZE];
static UCHAR                    scratch_memory[SCRATCH_MEMORY_SIZE];
#endif




/* Define thread prototypes.  */

static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_media_check_application_define(void *first_unused_memory)
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
    scratch_memory =  pointer;
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
ULONG       errors_detected;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Media Check Test........................");

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            sizeof(cache_buffer),   // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            256,                    // Total sectors 
                            2048,                   // Sector size   
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
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, sizeof(cache_buffer));

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(3);
    }

    /* Enable fault tolerant. */
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, sizeof(fault_tolerant_buffer));

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(4);
    }

    /* Check the media for errors.  */
    status =  fx_media_check(&ram_disk, scratch_memory, SCRATCH_MEMORY_SIZE, FX_LOST_CLUSTER_ERROR, &errors_detected);

    /* Determine if any were found - should not have found any errors at this point.  */
    if ((status != FX_SUCCESS) || (errors_detected))
    {

        /* Error in the media.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(5);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(6);
    }

    printf("SUCCESS!\n");
    test_control_return(0);
}

#else  
#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_media_check_application_define(void *first_unused_memory)
#endif
{
    
    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Media Check Test........................N/A\n");

    test_control_return(255);
}
#endif
