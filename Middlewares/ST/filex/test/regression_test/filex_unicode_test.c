/* This FileX test concentrates on the unicode operations.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_utility.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"

#define     DEMO_STACK_SIZE         8192
#define     CACHE_SIZE              16*128

/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD                ftest_0;
#endif
static FX_MEDIA                 ram_disk;


static UCHAR                    short_unicode_name[] =  {1, 0, 0, 0};
static UCHAR                    directory_name[] =      {3, 0, 4, 0, 5, 0, 6, 0, 0, 0};
static UCHAR                    long_unicode_name[] =   {2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 15, 0, 0, 0};
static UCHAR                    destination_name[100];
static UCHAR                    does_not_exist[] =      {1, 1, 1, 1};

static UCHAR                    unicode_temp_long_file_name[FX_MAX_LONG_NAME_LEN];

/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                    *ram_disk_memory;
static UCHAR                    *cache_buffer;
#else
static UCHAR                     cache_buffer[CACHE_SIZE];
#endif

/* Define thread prototypes.  */

void    filex_unicode_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);

#ifndef FX_ENABLE_FAULT_TOLERANT
/* Calculate the times driver was called to raise error at a particular time. */
static INT                      driver_called_counter = 0;

/* Create a terrible driver. */
static void _fx_terrible_driver(FX_MEDIA *media_ptr)
{
    driver_called_counter++;

    /* Make IO error to cover the branch at Line 247 in fx_unicode_file_create.c */
    if ( driver_called_counter != 61)
        (* _fx_ram_driver)(media_ptr);
    else
        media_ptr -> fx_media_driver_status = FX_IO_ERROR;

    return;
}
#endif /* FX_ENABLE_FAULT_TOLERANT */

/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_unicode_application_define(void *first_unused_memory)
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
ULONG       length;
ULONG       ul_temp;
UCHAR       buffer[512];
UINT        i;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Unicode test...........................................");

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            100,                    // Directory Entries
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
    
    /* try to do all the unicode commands before the media is opened to generate an error */
    length = 1;
    
    /* short name get */
    status = fx_unicode_short_name_get(&ram_disk, directory_name, length, (CHAR *) destination_name);
    if (status != FX_MEDIA_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(2);
    }
    
    /* Call extended version of short name get.  */
    status = fx_unicode_short_name_get_extended(&ram_disk, directory_name, length, (CHAR*)destination_name, sizeof(destination_name));
    if (status != FX_MEDIA_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(2);
    }

    /* name get */
    status = fx_unicode_name_get(&ram_disk, (CHAR *) destination_name, destination_name, &length);
    if (status != FX_MEDIA_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(3);
    }

    /* Call extended version of name get.  */
    status = fx_unicode_name_get_extended(&ram_disk, (CHAR*)destination_name, destination_name, &length, sizeof(destination_name));
    if (status != FX_MEDIA_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(3);
    }

    /* create */
    status = fx_unicode_directory_create(&ram_disk,  directory_name, length, (CHAR *) destination_name);
    if (status != FX_MEDIA_NOT_OPEN)
    {
        printf("ERROR!\n");
        test_control_return(4);
    }

    /* file create */
    status =  fx_unicode_file_create(&ram_disk, short_unicode_name, length, (CHAR *) destination_name);
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
    
    /* try to create a directory while the media is write protected */
    ram_disk.fx_media_driver_write_protect = FX_TRUE;
    status = fx_unicode_directory_create(&ram_disk,  directory_name, length, (CHAR *) destination_name);
    if (status != FX_WRITE_PROTECT)
    {
        printf("ERROR!\n");
        test_control_return(7);
    }
    
    /* try to create a file while the media is write protected */
    status =  fx_unicode_file_create(&ram_disk, short_unicode_name, length, (CHAR *) destination_name);
    if (status != FX_WRITE_PROTECT)
    {
        printf("ERROR!\n");
        test_control_return(8);
    }
    ram_disk.fx_media_driver_write_protect = FX_FALSE;

    /* only test this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send a null pointer to generate an error */
    /* short name get */
    status = fx_unicode_short_name_get(FX_NULL, directory_name, length, (CHAR *) destination_name);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(9);
    }
    
    /* name get */
    status = fx_unicode_name_get(FX_NULL, (CHAR *) destination_name, destination_name, &length);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(10);
    }

    /* file create */
    status = fx_unicode_file_create(FX_NULL, short_unicode_name, length, (CHAR *) destination_name);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(11);
    }
#endif /* FX_DISABLE_ERROR_CHECKING */

    /* only test this if exFAT is enabled */
#ifdef FX_ENABLE_EXFAT
//omitted because FX_ENABLE_EXFAT is not defined in the src
    
    /* set media attribute to exFAT and get to generate an error */
//    UCHAR uc_temp = ram_disk.fx_media_FAT_type;
//    ram_disk.fx_media_FAT_type = FX_exFAT;
//    ram_disk.fx_media_id = FX_MEDIA_ID;
//    status = fx_unicode_short_name_get(&ram_disk, directory_name, length, (CHAR *) destination_name);
//    if (status != FX_NOT_IMPLEMENTED)
//    {
//        printf("ERROR!\n");
//        test_control_return(12);
//    }
//    status = fx_unicode_name_get(&ram_disk, (CHAR *) destination_name, destination_name, &length);
//    if (status != FX_NOT_IMPLEMENTED)
//    {
//        printf("ERROR!\n");
//        test_control_return(23);
//    }
//    ram_disk.fx_media_FAT_type = uc_temp;
#endif /* FX_ENABLE_EXFAT */
    
    /* Create the a short and long unicode file name.  */
    length =  fx_unicode_length_get(short_unicode_name);
    status =  fx_unicode_file_create(&ram_disk, short_unicode_name, length, (CHAR *) destination_name);
    length =  fx_unicode_length_get(long_unicode_name);
    status += fx_unicode_file_create(&ram_disk, long_unicode_name, length, (CHAR *) destination_name);
    status += fx_file_create(&ram_disk, "abcdefghijklmnop");
    status += fx_directory_short_name_get(&ram_disk, "abcdefghijklmnop", (CHAR *) destination_name);
    status += fx_media_flush(&ram_disk);

    /* Check for erros.  */
    if (status != FX_SUCCESS)
    {

        /* Error creating unicode file names.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(13);
    }

    /* Try to create the same name again - this should result in an error!  */
    length =  fx_unicode_length_get(long_unicode_name);
    status += fx_unicode_file_create(&ram_disk, long_unicode_name, length, (CHAR *) destination_name);
    
    /* Check for expected error.  */
    if (status != FX_ALREADY_CREATED)
    {

        /* Error creating unicode file names.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(14);
    }

    /* Try creating a unicode name twice... this should result in an error as well.  */    
    length =  fx_unicode_length_get(short_unicode_name);
    status =  fx_unicode_file_create(&ram_disk, short_unicode_name, length, (CHAR *) destination_name);

    /* Check for expected error.  */
    if (status != FX_ALREADY_CREATED)
    {

        /* Error creating unicode file names.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(15);
    }

    /* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING
    /* send null pointer to generate an error */
    status = fx_unicode_directory_create(FX_NULL,  directory_name, length, (CHAR *) destination_name);
    if (status != FX_PTR_ERROR)
    {
        printf("ERROR!\n");
        test_control_return(16);
    }
#endif /* FX_DISABLE_ERROR_CHECKING */

    /* Create the unicode directory name to create the same unicode file names in the sub directory.  */
    length =  fx_unicode_length_get(directory_name);
    status =  fx_unicode_directory_create(&ram_disk,  directory_name, length, (CHAR *) destination_name);
    status += fx_file_create(&ram_disk, "qrstuvwxyz");
    
    /* Check for erros.  */
    if (status != FX_SUCCESS)
    {

        /* Error creating unicode directory.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(17);
    }

    /* Attempt to create the unicode sub-directory again.  */
    status =  fx_unicode_directory_create(&ram_disk,  directory_name, length, (CHAR *) destination_name);

    /* Check for expected error.  */
    if (status != FX_ALREADY_CREATED)
    {

        /* Error creating unicode directory name.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(18);
    }

    /* Ask for the name of something that does not exist to generate an error. */
    ul_temp = length;
    status = fx_unicode_short_name_get(&ram_disk, does_not_exist, length, (CHAR *) destination_name);
    if (status == FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(19);
    }
    status = fx_unicode_name_get(&ram_disk, (CHAR *) does_not_exist, destination_name, &length);
    if (status == FX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(20);
    }
    length = ul_temp;

    /* Now, pickup the short name for the unicode directory so we can set the default path there and
       do the same thing from a sub-directory.  */
    status =  fx_unicode_short_name_get(&ram_disk, directory_name, length, (CHAR *) destination_name);
    status += fx_directory_default_set(&ram_disk, (CHAR *) destination_name);
    length =  fx_unicode_length_get(short_unicode_name);
    status =  fx_unicode_file_create(&ram_disk, short_unicode_name, length, (CHAR *) destination_name);
    length =  fx_unicode_length_get(long_unicode_name);
    status += fx_unicode_file_create(&ram_disk, long_unicode_name, length, (CHAR *) destination_name);
    status += fx_file_create(&ram_disk, "abcdefghijklmnop");
    length =  fx_unicode_length_get(directory_name);
    status += fx_unicode_directory_create(&ram_disk,  directory_name, length, (CHAR *) destination_name);
    status += fx_file_create(&ram_disk, "qrstuvwxyz");
    status += fx_media_flush(&ram_disk);

    /* Check for erros.  */
    if (status != FX_SUCCESS)
    {

        /* Error creating unicode file and directory names in a sub-directory.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(21);
    }


    /* Test the short/long name get routines with the short unicode name.  */
    length =  fx_unicode_length_get(short_unicode_name);
    status += fx_unicode_short_name_get(&ram_disk, short_unicode_name, length, (CHAR *) destination_name);
    status += fx_unicode_name_get(&ram_disk, (CHAR *) destination_name, destination_name, &length);
    
    /* Check for errors.  */
    if ((status) || (length != fx_unicode_length_get(short_unicode_name)))
    {

        /* Error getting unicode file and short names in a sub-directory.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(22);
    }

    /* Test the short/long name get routines with the long unicode name.  */
    length =  fx_unicode_length_get(long_unicode_name);
    status += fx_unicode_short_name_get(&ram_disk, long_unicode_name, length, (CHAR *) destination_name);
    status += fx_unicode_name_get(&ram_disk, (CHAR *) destination_name, destination_name, &length);
    
    /* Check for errors.  */
    if ((status) || (length != fx_unicode_length_get(long_unicode_name)))
    {

        /* Error getting unicode file and short names in a sub-directory.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(23);
    }

    /* Test the short/long name get routines with the directory unicode name.  */
    length =  fx_unicode_length_get(directory_name);
    status += fx_unicode_short_name_get(&ram_disk, directory_name, length, (CHAR *) destination_name);
    status += fx_unicode_name_get(&ram_disk, (CHAR *) destination_name, destination_name, &length);
    
    /* Check for errors.  */
    if ((status) || (length != fx_unicode_length_get(directory_name)))
    {

        /* Error getting unicode file and short names in a sub-directory.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(24);
    }

    /* Now delete everything in the sub-directory.  */
    status += fx_unicode_short_name_get(&ram_disk, directory_name, length, (CHAR *) destination_name);
    status += fx_directory_delete(&ram_disk, (CHAR *) destination_name);
    status += fx_file_delete(&ram_disk, "qrstuvwxyz");
    status += fx_file_delete(&ram_disk, "abcdefghijklmnop");
    length =  fx_unicode_length_get(long_unicode_name);
    status += fx_unicode_short_name_get(&ram_disk, long_unicode_name, length, (CHAR *) destination_name);
    status += fx_file_delete(&ram_disk, (CHAR *) destination_name);    
    length =  fx_unicode_length_get(short_unicode_name);
    status += fx_unicode_short_name_get(&ram_disk, short_unicode_name, length, (CHAR *) destination_name);
    status += fx_file_delete(&ram_disk, (CHAR *) destination_name);             
    
    /* Check for erros.  */
    if (status != FX_SUCCESS)
    {

        /* Error deleting unicode file and directory names in a sub-directory.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(25);
    }


   /* Move the directory default back to the root.  */    
    status += fx_directory_default_set(&ram_disk, "/");

    /* Test the short/long name get routines with the short unicode name.  */
    length =  fx_unicode_length_get(short_unicode_name);
    status += fx_unicode_short_name_get(&ram_disk, short_unicode_name, length, (CHAR *) destination_name);
    status += fx_unicode_name_get(&ram_disk, (CHAR *) destination_name, destination_name, &length);
    
    /* Check for errors.  */
    if ((status) || (length != fx_unicode_length_get(short_unicode_name)))
    {

        /* Error getting unicode file and short names in the root directory.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(26);
    }

    /* Test the short/long name get routines with the long unicode name.  */
    length =  fx_unicode_length_get(long_unicode_name);
    status += fx_unicode_short_name_get(&ram_disk, long_unicode_name, length, (CHAR *) destination_name);
    status += fx_unicode_name_get(&ram_disk, (CHAR *) destination_name, destination_name, &length);
    
    /* Check for errors.  */
    if ((status) || (length != fx_unicode_length_get(long_unicode_name)))
    {

        /* Error getting unicode file and short names in the root directory.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(27);
    }

    /* Test the short/long name get routines with the directory unicode name.  */
    length =  fx_unicode_length_get(directory_name);
    status += fx_unicode_short_name_get(&ram_disk, directory_name, length, (CHAR *) destination_name);
    status += fx_unicode_name_get(&ram_disk, (CHAR *) destination_name, destination_name, &length);
    
    /* Check for errors.  */
    if ((status) || (length != fx_unicode_length_get(directory_name)))
    {

        /* Error getting unicode file and short names in the root directory.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(28);
    }

    /* Delete the root directory contents.  */
    status += fx_unicode_short_name_get(&ram_disk, directory_name, length, (CHAR *) destination_name);
    status += fx_directory_delete(&ram_disk, (CHAR *) destination_name);
    status += fx_file_delete(&ram_disk, "qrstuvwxyz");
    status += fx_file_delete(&ram_disk, "abcdefghijklmnop");
    length =  fx_unicode_length_get(long_unicode_name);
    status += fx_unicode_short_name_get(&ram_disk, long_unicode_name, length, (CHAR *) destination_name);
    status += fx_file_delete(&ram_disk, (CHAR *) destination_name);    
    length =  fx_unicode_length_get(short_unicode_name);
    status += fx_unicode_short_name_get(&ram_disk, short_unicode_name, length, (CHAR *) destination_name);
    status += fx_file_delete(&ram_disk, (CHAR *) destination_name);

    /* Check for errors.  */
    if (status != FX_SUCCESS)
    {

        /* Error getting unicode file and short names in the root directory.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(29);
    }

    length = fx_unicode_length_get(short_unicode_name);
    status = fx_unicode_file_create(&ram_disk, short_unicode_name, length, (CHAR*)destination_name);
    length = fx_unicode_length_get(long_unicode_name);
    status += fx_unicode_file_create(&ram_disk, long_unicode_name, length, (CHAR*)destination_name);

    length = fx_unicode_length_get((UCHAR*)"Z\01\02\03\0\0");
    status += fx_file_create(&ram_disk, (CHAR*)"test");

    /* Check for errors.  */
    if (status != FX_SUCCESS)
    {

        /* Error creating unicode files.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(29);
    }

    length = fx_unicode_length_get(long_unicode_name);
    status = fx_unicode_short_name_get_extended(&ram_disk, long_unicode_name, length, (CHAR*)destination_name, 14);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        /* Error getting short name.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(29);
    }

    /* Set unicode_temp_long_file_name and length to known state.  */
    memcpy(unicode_temp_long_file_name, "ORIGINALSTRING", 15);
    length = 0;
    status = fx_unicode_name_get_extended(&ram_disk, (CHAR*)destination_name, (UCHAR*)unicode_temp_long_file_name, &length, 4);

    /* Check for error.  */
    if ((status != FX_SUCCESS) || (length != 14) || (memcmp(unicode_temp_long_file_name, "\2\0\0\0INALSTRING\0", 15)))
    {

        /* Error getting short name.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(29);
    }

    status = fx_unicode_short_name_get_extended(&ram_disk, (UCHAR*)"t\0e\0s\0t\0\0", 4, (CHAR*)destination_name, 14);

    /* Check for error.  */
    if (status != FX_SUCCESS)
    {

        /* Error getting short name.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(29);
    }

    /*  At the circumstance of the same lenth of unicode name,there are no more than 26 unicode names
        with the first character ranging from 'a'to 'z'.If you create one more unicode name, an error will
        occur.Now test this case */
    length = fx_unicode_length_get(long_unicode_name);
    unicode_temp_long_file_name[0] = 'z';

    for (i = 1; i < length; i++)
    {

        /* Build temporary long file name.  */
        unicode_temp_long_file_name[i] = (UCHAR)('0' + (i % 9));
    }
    unicode_temp_long_file_name[i] = FX_NULL;
    for (i = 1; i < 27; i++)
    {

        /* creat the same lenth  files but Them differ in the first character of file name.  */
        status = fx_file_create(&ram_disk, (CHAR *)unicode_temp_long_file_name);
        if (status)
            break;
        unicode_temp_long_file_name[0] = (UCHAR)('z' - i);
    }
    status = fx_unicode_file_create(&ram_disk, long_unicode_name, length, (CHAR *)destination_name);
    if (status != FX_ALREADY_CREATED)
    {
        /* No spare name for unicode file names.  Return to caller.  */
        printf("ERROR!\n");
        test_control_return(30);
    }

    /* Test the unicode length get that exceeds the maximum value.  */
    for (i = 0; i < 512; i++)
    {
        buffer[i] = 'A';
    }
    length =  fx_unicode_length_get(buffer);
    if (length != 128)
    {

        printf("ERROR!\n");
        test_control_return(31);
    }

    /* Now test the NULL bytes being in different locations.  */
    buffer[0] =  0;
    buffer[3] =  0;
    buffer[8] =  0;
    buffer[9] =  0;

    length =  fx_unicode_length_get(buffer);
    if (length != 4)
    {

        printf("ERROR!\n");
        test_control_return(32);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_value_if_fail( status == FX_SUCCESS, 33);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_value_if_fail( status == FX_SUCCESS, 34);

#ifndef FX_ENABLE_FAULT_TOLERANT
    /* Register our terrible driver. */
    ram_disk.fx_media_driver_entry = _fx_terrible_driver;
    short_unicode_name[0]++;
    length = fx_unicode_length_get(short_unicode_name);
    status = fx_unicode_file_create(&ram_disk, long_unicode_name, length, (CHAR *)destination_name);
    ram_disk.fx_media_driver_entry = _fx_ram_driver;
    return_value_if_fail( status == FX_IO_ERROR, 35);
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    return_value_if_fail( status == FX_SUCCESS, 36);

    printf("SUCCESS!\n");
    test_control_return(0);
}
