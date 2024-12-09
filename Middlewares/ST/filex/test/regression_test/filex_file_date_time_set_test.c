/* This FileX test concentrates on the file date/time set operation.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"

#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              128*128


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD               ftest_0;
#endif
static FX_MEDIA                ram_disk;
static FX_FILE                 file_0;
static FX_FILE                 file_1;
static FX_FILE                 file_2;
static FX_FILE                 file_3;
static FX_FILE                 file_4;
static FX_FILE                 file_5;
static FX_FILE                 file_6;


/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *ram_disk_memory;
static UCHAR                  *cache_buffer;
#else
static UCHAR                   cache_buffer[CACHE_SIZE];
#endif


/* Define thread prototypes.  */

void    filex_file_date_time_set_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_file_date_time_set_application_define(void *first_unused_memory)
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
UINT        attributes;
ULONG       size;
UINT        year, month, day, hour, minute, second;
#ifndef FX_DISABLE_ERROR_CHECKING
UINT        i;
#endif /* FX_DISABLE_ERROR_CHECKING */
FX_FILE     my_file;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   File date/time set test................................");

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
    
    /* try to retrieve directory information before the media has been opened to generate an error */
    status =  fx_directory_information_get(&ram_disk, "DOES_NOT_EXIST", &attributes, &size,
                                        &year, &month, &day, &hour, &minute, &second);
    if (status != FX_MEDIA_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(2);
    }
    
    /* Attempt to set the date and time for a file before the media is opened to generate an error */
    status = fx_file_date_time_set(&ram_disk, "TEST.TXT", 2000, 1, 1, 1, 1, 1);
    if (status != FX_MEDIA_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(3);
    }

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(4);
    }

    /* Create a few files file in the root directory.  */
    status  = fx_file_create(&ram_disk, "TEST.TXT");
    status += fx_file_create(&ram_disk, "OPENED_FOR_WRITE.TXT");
    status += fx_file_create(&ram_disk, "NOT_OPENED.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(5);
    }
    
/* test error checking */
#ifndef FX_DISABLE_ERROR_CHECKING
    
    /* send a null pointer to generate an error */
    status =  fx_directory_information_get(&ram_disk, "DOES_NOT_EXIST", FX_NULL, FX_NULL,
                                        FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(6);
    }
    
    /* send null pointer to generate an error */
    status = fx_file_date_time_set(FX_NULL, "TEST.TXT", 0, 0, 0, 0, 0, 0);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(7);
    }
    
    /* send an invalid year to generate an error */
    status = fx_file_date_time_set(&ram_disk, "TEST.TXT", 0, 0, 0, 0, 0, 0);
    if (status != FX_INVALID_YEAR)
    {
        printf("ERROR!\n");
        test_control_return(8);
    }
    
    /* send an invalid month to generate an error */
    status = fx_file_date_time_set(&ram_disk, "TEST.TXT", 2015, 0, 1, 0, 0, 0);
    if (status != FX_INVALID_MONTH)
    {
        printf("ERROR!\n");
        test_control_return(9);
    }
    
    /* send a valid and invalid day for each month to generate an error */
    for (i = 1; i <= 12; i++)
    {
        status = fx_file_date_time_set(&ram_disk, "TEST.TXT", 2015, i, 0, 0, 0, 0);
        if (status != FX_INVALID_DAY)
        {
            printf("ERROR!\n");
            test_control_return(10);
        }
        
        status = fx_file_date_time_set(&ram_disk, "TEST.TXT", 2015, i, 1, 1, 1, 1);
        if (status != FX_SUCCESS)
        {
            printf("ERROR!\n");
            test_control_return(11);
        }
    }
    
    /* send an invalid day for feb of a leap year */
    status = fx_file_date_time_set(&ram_disk, "TEST.TXT", 2016, 2, 0, 0, 0, 0);
    if (status != FX_INVALID_DAY)
    {
        printf("ERROR!\n");
        test_control_return(12);
    }
    
    /* send an invalid hour to generate an error */
    status = fx_file_date_time_set(&ram_disk, "TEST.TXT", 2000, 1, 1, 99, 0, 0);
    if (status != FX_INVALID_HOUR)
    {
        printf("ERROR!\n");
        test_control_return(13);
    }
    
    /* send an invalid minute to generate an error */
    status = fx_file_date_time_set(&ram_disk, "TEST.TXT", 2000, 1, 1, 1, 99, 0);
    if (status != FX_INVALID_MINUTE)
    {
        printf("ERROR!\n");
        test_control_return(14);
    }
    
    /* send an invalid second to generate an error */
    status = fx_file_date_time_set(&ram_disk, "TEST.TXT", 2000, 1, 1, 1, 1, 99);
    if (status != FX_INVALID_SECOND)
    {
        printf("ERROR!\n");
        test_control_return(15);
    }

#endif
    
    /* Attempt to set the date and time for a file that does not exist to generate an error */
    status = fx_file_date_time_set(&ram_disk, "DOES_NOT_EXIST", 2000, 1, 1, 1, 1, 1);
    if (status == FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(16);
    }

    /* Invalidate the media cache.  */
    fx_media_cache_invalidate(&ram_disk);

    /* Set the date and time for the file.  */
    status =  fx_file_date_time_set(&ram_disk, "TEST.TXT", 
                1999, 12, 31, 23, 59, 58);

    /* Check the date/time set status.  */
    if (status != FX_SUCCESS) 
    {

        printf("ERROR!\n");
        test_control_return(17);
    }
    
    /* try to retrieve directory information for something that doesnt exist to generate an error */
    status =  fx_directory_information_get(&ram_disk, "DOES_NOT_EXIST", &attributes, &size,
                                        &year, &month, &day, &hour, &minute, &second);
    if (status == FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(18);
    }

    /* Invalidate the media cache.  */
    fx_media_cache_invalidate(&ram_disk);

    /* Now pickup the date/time for the file.  */
    status =  fx_directory_information_get(&ram_disk, "TEST.TXT", &attributes, &size,
                                        &year, &month, &day, &hour, &minute, &second);

    /* Check the date/time status.  */
    if ((status != FX_SUCCESS) || (attributes != FX_ARCHIVE) || (size != 0) || 
        (year != 1999) || (month != 12) || (day != 31) || (hour != 23) || (minute != 59) || (second != 58))
    {

        printf("ERROR!\n");
        test_control_return(19);
    }
    
    /* get the date/time for the other files while some are opened to get better code coverage */
    status  = fx_file_open(&ram_disk, &my_file, "OPENED_FOR_WRITE.TXT", FX_OPEN_FOR_WRITE);
    status += fx_directory_information_get(&ram_disk, "OPENED_FOR_WRITE.TXT", &attributes, &size,
                                        &year, &month, &day, &hour, &minute, &second);
    status += fx_directory_information_get(&ram_disk, "NOT_OPENED.TXT", &attributes, &size,
                                        &year, &month, &day, &hour, &minute, &second);
    if (status != FX_SUCCESS)
    {
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
#ifndef FX_DISABLE_ERROR_CHECKING
    /* Set the date and time for with an invalid file name.  */
    status =  fx_file_date_time_set(&ram_disk, NULL, 
                1999, 12, 31, 23, 59, 58);

    /* Check the date/time set status.  */
    if (status != FX_PTR_ERROR) 
    {

        printf("ERROR!\n");
        test_control_return(22);
    }

    /* Set the date and time for with an invalid year.  */
    status =  fx_file_date_time_set(&ram_disk, "TEST.TXT", 
                FX_MAXIMUM_YEAR+1, 12, 31, 23, 59, 58);

    /* Check the date/time set status.  */
    if (status != FX_INVALID_YEAR) 
    {

        printf("ERROR!\n");
        test_control_return(23);
    }

    /* Set the date and time for with an invalid day for January.  */
    status =  fx_file_date_time_set(&ram_disk, "TEST.TXT", 
                2016, 1, 32, 23, 59, 58);

    /* Check the date/time set status.  */
    if (status != FX_INVALID_DAY) 
    {

        printf("ERROR!\n");
        test_control_return(24);
    }

    /* Set the date and time for with an invalid day for Febuary leap year.  */
    status =  fx_file_date_time_set(&ram_disk, "TEST.TXT", 
                2016, 2, 30, 23, 59, 58);

    /* Check the date/time set status.  */
    if (status != FX_INVALID_DAY) 
    {

        printf("ERROR!\n");
        test_control_return(25);
    }

    /* Set the date and time for with a valid day for Febuary leap year.  */
    status =  fx_file_date_time_set(&ram_disk, "TEST.TXT", 
                2016, 2, 29, 24, 59, 58);

    /* Check the date/time set status.  */
    if (status != FX_INVALID_HOUR) 
    {

        printf("ERROR!\n");
        test_control_return(26);
    }

    /* Set the date and time for with an invalid day for Febuary.  */
    status =  fx_file_date_time_set(&ram_disk, "TEST.TXT", 
                2017, 2, 29, 23, 59, 58);

    /* Check the date/time set status.  */
    if (status != FX_INVALID_DAY) 
    {

        printf("ERROR!\n");
        test_control_return(27);
    }

    /* Set the date and time for with an invalid day for March.  */
    status =  fx_file_date_time_set(&ram_disk, "TEST.TXT", 
                2017, 3, 32, 23, 59, 58);

    /* Check the date/time set status.  */
    if (status != FX_INVALID_DAY) 
    {

        printf("ERROR!\n");
        test_control_return(28);
    }

    /* Set the date and time for with an invalid day for April.  */
    status =  fx_file_date_time_set(&ram_disk, "TEST.TXT", 
                2017, 4, 31, 23, 59, 58);

    /* Check the date/time set status.  */
    if (status != FX_INVALID_DAY) 
    {

        printf("ERROR!\n");
        test_control_return(29);
    }

    /* Set the date and time for with an invalid day for May.  */
    status =  fx_file_date_time_set(&ram_disk, "TEST.TXT", 
                2017, 5, 32, 23, 59, 58);

    /* Check the date/time set status.  */
    if (status != FX_INVALID_DAY) 
    {

        printf("ERROR!\n");
        test_control_return(30);
    }

    /* Set the date and time for with an invalid day for June.  */
    status =  fx_file_date_time_set(&ram_disk, "TEST.TXT", 
                2017, 6, 31, 23, 59, 58);

    /* Check the date/time set status.  */
    if (status != FX_INVALID_DAY) 
    {

        printf("ERROR!\n");
        test_control_return(31);
    }

    /* Set the date and time for with an invalid day for July.  */
    status =  fx_file_date_time_set(&ram_disk, "TEST.TXT", 
                2017, 7, 32, 23, 59, 58);

    /* Check the date/time set status.  */
    if (status != FX_INVALID_DAY) 
    {

        printf("ERROR!\n");
        test_control_return(32);
    }

    /* Set the date and time for with an invalid day for August.  */
    status =  fx_file_date_time_set(&ram_disk, "TEST.TXT", 
                2017, 8, 32, 23, 59, 58);

    /* Check the date/time set status.  */
    if (status != FX_INVALID_DAY) 
    {

        printf("ERROR!\n");
        test_control_return(33);
    }

    /* Set the date and time for with an invalid day for September.  */
    status =  fx_file_date_time_set(&ram_disk, "TEST.TXT", 
                2017, 9, 31, 23, 59, 58);

    /* Check the date/time set status.  */
    if (status != FX_INVALID_DAY) 
    {

        printf("ERROR!\n");
        test_control_return(34);
    }

    /* Set the date and time for with an invalid day for October.  */
    status =  fx_file_date_time_set(&ram_disk, "TEST.TXT", 
                2017, 10, 32, 23, 59, 58);

    /* Check the date/time set status.  */
    if (status != FX_INVALID_DAY) 
    {

        printf("ERROR!\n");
        test_control_return(35);
    }

    /* Set the date and time for with an invalid day for November.  */
    status =  fx_file_date_time_set(&ram_disk, "TEST.TXT", 
                2017, 11, 31, 23, 59, 58);

    /* Check the date/time set status.  */
    if (status != FX_INVALID_DAY) 
    {

        printf("ERROR!\n");
        test_control_return(36);
    }

    /* Set the date and time for with an invalid day for December.  */
    status =  fx_file_date_time_set(&ram_disk, "TEST.TXT", 
                2017, 12, 32, 23, 59, 58);

    /* Check the date/time set status.  */
    if (status != FX_INVALID_DAY) 
    {

        printf("ERROR!\n");
        test_control_return(37);
    }
#endif
    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            64,                     // Directory Entries
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

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(39);
    }
    
    /* Create a bunch of files.  */
    status =   fx_file_create(&ram_disk, "TEST.TXT");
    status +=  fx_file_create(&ram_disk, "TEST1.TXT");
    status +=  fx_file_create(&ram_disk, "TEST2.TXT");
    status +=  fx_file_create(&ram_disk, "TEST3.TXT");
    status +=  fx_file_create(&ram_disk, "TEST4.TXT");
    status +=  fx_file_create(&ram_disk, "TEST5.TXT");
    status +=  fx_file_create(&ram_disk, "TEST6.TXT");
    status +=  fx_file_create(&ram_disk, "TEST7.TXT");
    status +=  fx_file_create(&ram_disk, "TEST8.TXT");
    status +=  fx_file_create(&ram_disk, "TEST9.TXT");
    status +=  fx_file_create(&ram_disk, "TEST10.TXT");
    status +=  fx_file_create(&ram_disk, "TEST11.TXT");
    status +=  fx_file_create(&ram_disk, "TEST12.TXT");
    status +=  fx_file_create(&ram_disk, "TEST13.TXT");
    status +=  fx_file_create(&ram_disk, "TEST14.TXT");
    status +=  fx_file_create(&ram_disk, "TEST15.TXT");
    status +=  fx_file_create(&ram_disk, "TEST16.TXT");
    status +=  fx_file_create(&ram_disk, "TEST17.TXT");
    status +=  fx_file_create(&ram_disk, "TEST18.TXT");
    status +=  fx_file_create(&ram_disk, "TEST19.TXT");
    status +=  fx_file_create(&ram_disk, "TEST20.TXT");
    status +=  fx_file_create(&ram_disk, "TEST21.TXT");
    status +=  fx_file_create(&ram_disk, "TEST22.TXT");
    status +=  fx_file_create(&ram_disk, "TEST23.TXT");
    status +=  fx_file_create(&ram_disk, "TEST24.TXT");
    status +=  fx_file_create(&ram_disk, "TEST25.TXT");
    status +=  fx_file_create(&ram_disk, "TEST26.TXT");
    status +=  fx_file_create(&ram_disk, "TEST27.TXT");
    status +=  fx_file_create(&ram_disk, "TEST28.TXT");
    status +=  fx_file_create(&ram_disk, "TEST29.TXT");
    status +=  fx_file_create(&ram_disk, "TEST30.TXT");
    status +=  fx_file_create(&ram_disk, "TEST31.TXT");

    /* Now open a set of files so we get all combinations through the open count check in fx_directory_information_get.c.  */
    status +=  fx_file_open(&ram_disk, &file_0, "TEST.TXT", FX_OPEN_FOR_READ);
    status +=  fx_file_open(&ram_disk, &file_1, "TEST1.TXT", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_2, "TEST2.TXT", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_3, "TEST9.TXT", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_4, "TEST11.TXT", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_5, "TEST19.TXT", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_6, "TEST30.TXT", FX_OPEN_FOR_WRITE);

    /* Now get information on the first file... requesting on the "second" parameter.  */
    status +=  fx_directory_information_get(&ram_disk, "TEST.TXT", FX_NULL, FX_NULL, 
                                                        FX_NULL, FX_NULL, FX_NULL,
                                                        FX_NULL, FX_NULL, &second);

    /* Determine if there was an error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(40);
    }

    /* Now get information on the first file... requesting on the "minute" parameter.  */
    status +=  fx_directory_information_get(&ram_disk, "TEST.TXT", FX_NULL, FX_NULL, 
                                                        FX_NULL, FX_NULL, FX_NULL,
                                                        FX_NULL, &minute, FX_NULL);

    /* Determine if there was an error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(41);
    }
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    
    /* Determine if there was an error.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(42);
    }
    else
    {

        printf("SUCCESS!\n");
        test_control_return(0);
    }
}

