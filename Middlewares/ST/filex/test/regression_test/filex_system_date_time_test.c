/* This FileX test concentrates on the system date/time set/get operations.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_system.h"
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

void    filex_system_date_time_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_system_date_time_application_define(void *first_unused_memory)
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
UINT        save_time;
UINT        save_date;
UINT        year, month, day, hour, minute, second;
UINT        i;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   System date/time get/set test..........................");

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
                            256,                    // Total sectors 
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
    
/* test for error checking by sending bad information */
#ifndef FX_DISABLE_ERROR_CHECKING
    
    /* check system date get pointer error */
    status = fx_system_date_get(FX_NULL, FX_NULL, FX_NULL);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(4);
    }
    
    /* check system time get pointer error */
    status = fx_system_time_get(FX_NULL, FX_NULL, FX_NULL);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(5);
    }    
    
    /* check system time set bad hours */
    status = fx_system_time_set(99, 0, 0);
    if (status != FX_INVALID_HOUR)
    {
        printf("ERROR!\n");
        test_control_return(6);
    }
    
    /* check system time set bad minutes */
    status = fx_system_time_set(0, 99, 0);
    if (status != FX_INVALID_MINUTE)
    {
        printf("ERROR!\n");
        test_control_return(7);
    }
    
    /* check system time set bad seconds */
    status = fx_system_time_set(0, 0, 99);
    if (status != FX_INVALID_SECOND)
    {
        printf("ERROR!\n");
        test_control_return(8);
    }
    
    /* check system date set bad year */
    status = fx_system_date_set(99, 6, 6);
    if (status != FX_INVALID_YEAR)
    {
        printf("ERROR!\n");
        test_control_return(9);
    }
    
    /* check system date set bad month */
    status = fx_system_date_set(2015, 99, 6);
    if (status != FX_INVALID_MONTH)
    {
        printf("ERROR!\n");
        test_control_return(10);
    }
    
    /* check good and bad inputs for each month */
    for (i = 1; i <= 12; i++)
    {
        /* valid input for month i */
        status = fx_system_date_set(2015, i, 6);
        if (status != FX_SUCCESS)
        {
            printf("ERROR!\n");
            test_control_return(11);
        }
        
        /* invalid input for month i */
        status = fx_system_date_set(2015, i, 99);
        if (status != FX_INVALID_DAY)
        {
            printf("ERROR!\n");
            test_control_return(12);
        }
    }
    
    /* check bad input for feb on a leap year */
    status = fx_system_date_set(2016, 2, 99);
    if (status != FX_INVALID_DAY)
    {
        printf("ERROR!\n");
        test_control_return(13);
    }
    
    
#endif

    /* Set the date and time.  */
    status =  fx_system_date_set(1999, 12, 31); 
   
    /* Check for successful date set.  */  
    if (status != FX_SUCCESS)
    {

        /* Error setting date.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(4);
    }
    
    status =  fx_system_time_set(23, 59, 58);

    /* Check for successful time set.  */  
    if (status != FX_SUCCESS)
    {

        /* Error setting time.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(14);
    }
    
    /* Call the internal system date get with NULL values.  */
    status =  _fx_system_date_get(FX_NULL, FX_NULL, FX_NULL);
    
    /* Get the date.  */
    status +=  fx_system_date_get(&year, &month, &day);
    
    /* Check for successful date get.  */  
    if ((status != FX_SUCCESS) ||
        (year != 1999) ||
        (month != 12) ||
        (day != 31))
    {

        /* Error getting date.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(15);
    }
    
    /* Call the internal system time get.  */
    status =  _fx_system_time_get(FX_NULL, FX_NULL, FX_NULL);
    
    /* Get the time.  */
    status +=  fx_system_time_get(&hour, &minute, &second);

    /* Check for successful time get.  */
    if ((status != FX_SUCCESS) ||
        (hour != 23) ||
        (minute != 59) ||
        (second != 58))
    {

        /* Error getting time.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(16);
    }
#ifndef FX_DISABLE_ERROR_CHECKING
    /* Get the time, but with a NULL value for month.  */
    status =  fx_system_time_get(&hour, FX_NULL, &second);

    /* Check for successful time get.  */
    if (status != FX_PTR_ERROR)
    {

        /* Error getting time.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(17);
    }

    /* Get the time, but with a NULL value for second.  */
    status =  fx_system_time_get(&hour, &minute, FX_NULL);

    /* Check for successful time get.  */
    if (status != FX_PTR_ERROR)
    {

        /* Error getting time.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(18);
    }

    /* Get the date, but with a NULL value for month.  */
    status =  fx_system_date_get(&year, FX_NULL, &day);

    /* Check for successful date get.  */
    if (status != FX_PTR_ERROR)
    {

        /* Error getting date.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(19);
    }

    /* Get the date, but with a NULL value for day.  */
    status =  fx_system_date_get(&year, &month, FX_NULL);

    /* Check for successful date get.  */
    if (status != FX_PTR_ERROR)
    {

        /* Error getting date.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(20);
    }

    /* Set the date, but with an invalid year over the max.  */
    status =  fx_system_date_set(FX_MAXIMUM_YEAR+1, 12, 1);

    /* Check for successful date set.  */
    if (status != FX_INVALID_YEAR)
    {

        /* Error setting date.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(21);
    }

    /* Set the date, but with an invalid day (0).  */
    status =  fx_system_date_set(2016, 12, 0);

    /* Check for successful date set.  */
    if (status != FX_INVALID_DAY)
    {

        /* Error setting date.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(22);
    }

    /* Set the date, but with a leap year an invalid day 30 for February.  */
    status =  fx_system_date_set(2016, 2, 30);

    /* Check for successful date set.  */
    if (status != FX_INVALID_DAY)
    {

        /* Error setting date.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(22);
    }

    /* Set the date, but with a leap year and a valid day 29 for February.  */
    status =  fx_system_date_set(2016, 2, 29);

    /* Check for successful date set.  */
    if (status != FX_SUCCESS)
    {

        /* Error setting date.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(23);
    }
#endif
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Drive the system time logic.  */
    
    /* Save the system date/time.  */
    save_date =  _fx_system_date;
    save_time =  _fx_system_time;
    
    /* First call the system timer entry function with an invalid ID.  */
    _fx_system_timer_entry(0);
    
    /* Now setup an invalid month to cause the error logic to hit.  */
    _fx_system_date =  _fx_system_date | 0x1E0;  /* Set month to all ones - invalid > 12  */
    
    /* Now loop for the amount of updates (10 seconds per update) to cause the day to wrap.  */
    for (i = 0; i < 86400; i++)
    {   
        /* Adjust the time by calling the time update function.  */
        _fx_system_timer_entry(FX_TIMER_ID);
    }

    /* Restore the system date/time.  */
    _fx_system_date =  save_date;
    _fx_system_time =  save_time;

    /* Now call the system timer entry to walk through the maximum system time.  */
    for (i = 0; i < 315360000; i++)
    {
    
        /* Adjust the time by calling the time update function.  */
        _fx_system_timer_entry(FX_TIMER_ID);
    }

    /* Restore the system date/time.  */
    _fx_system_date =  save_date;
    _fx_system_time =  save_time;

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(10);
    }
    else
    {

        printf("SUCCESS!\n");
        test_control_return(0);
    }
}

