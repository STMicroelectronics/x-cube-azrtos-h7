/* This FileX test concentrates on the utility APIs.  */

#include   "fx_api.h"
#include   "fx_ram_driver_test.h"
#include   "fx_utility.h"
#include   <stdio.h>

void  filex_utility_application_define(void *first_unused_memory);
void  test_control_return(UINT status);

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
static UCHAR                     cache_buffer[CACHE_SIZE];
#endif
extern UCHAR                    _fx_media_format_oem_name[8];
extern UCHAR                    _fx_media_format_media_type;
extern ULONG                    _fx_media_format_volume_id;

static UCHAR partition_sector[] = {
  0xfa, 0xb8, 0x00, 0x00, 0x8e, 0xd0, 0xbc, 0x00, 0x7c, 0x8b, 0xf4, 0x50, 0x07, 0x50, 0x1f, 0xfb,
  0xfc, 0xbf, 0x00, 0x06, 0xb9, 0x00, 0x01, 0xf3, 0xa5, 0xea, 0x1e, 0x06, 0x00, 0x00, 0xbe, 0xbe,
  0x07, 0x80, 0x3c, 0x80, 0x74, 0x02, 0xcd, 0x18, 0x56, 0x53, 0x06, 0xbb, 0x00, 0x7c, 0xb9, 0x01,
  0x00, 0xba, 0x00, 0x00, 0xb8, 0x01, 0x02, 0xcd, 0x13, 0x07, 0x5b, 0x5e, 0xb2, 0x80, 0x72, 0x0b,
  0xbf, 0xbc, 0x7d, 0x81, 0x3d, 0x55, 0x53, 0x75, 0x02, 0xb2, 0x00, 0xbf, 0xeb, 0x06, 0x88, 0x15,
  0x8a, 0x74, 0x01, 0x8b, 0x4c, 0x02, 0x8b, 0xee, 0xeb, 0x15, 0xbe, 0x9b, 0x06, 0xac, 0x3c, 0x00,
  0x74, 0x0b, 0x56, 0xbb, 0x07, 0x00, 0xb4, 0x0e, 0xcd, 0x10, 0x5e, 0xeb, 0xf0, 0xeb, 0xfe, 0xbb,
  0x00, 0x7c, 0xb8, 0x01, 0x02, 0xcd, 0x13, 0x73, 0x05, 0xbe, 0xb3, 0x06, 0xeb, 0xdf, 0xbe, 0xd2,
  0x06, 0xbf, 0xfe, 0x7d, 0x81, 0x3d, 0x55, 0xaa, 0x75, 0xd3, 0xbf, 0x24, 0x7c, 0xbe, 0xeb, 0x06,
  0x8a, 0x04, 0x88, 0x05, 0x8b, 0xf5, 0xea, 0x00, 0x7c, 0x00, 0x00, 0x49, 0x6e, 0x76, 0x61, 0x6c,
  0x69, 0x64, 0x20, 0x70, 0x61, 0x72, 0x74, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x74, 0x61, 0x62,
  0x6c, 0x65, 0x00, 0x45, 0x72, 0x72, 0x6f, 0x72, 0x20, 0x6c, 0x6f, 0x61, 0x64, 0x69, 0x6e, 0x67,
  0x20, 0x6f, 0x70, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6e, 0x67, 0x20, 0x73, 0x79, 0x73, 0x74, 0x65,
  0x6d, 0x00, 0x4d, 0x69, 0x73, 0x73, 0x69, 0x6e, 0x67, 0x20, 0x6f, 0x70, 0x65, 0x72, 0x61, 0x74,
  0x69, 0x6e, 0x67, 0x20, 0x73, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x1a, 0xb8, 0x5e, 0x00, 0x00, 0x00, 0x14,
  0x35, 0x00, 0x0c, 0xfe, 0xff, 0xe9, 0x20, 0x05, 0x00, 0x00, 0xe0, 0xa2, 0xf5, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xaa
};

#ifdef FX_ENABLE_EXFAT
#define TEST_COUNT              6
#else              
#define TEST_COUNT              5
#endif

/* Define thread prototypes.  */

static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
UINT  fx_media_format_oem_name_set(UCHAR new_oem_name[8]);
UINT  fx_media_format_type_set(UCHAR new_media_type);
UINT  fx_media_format_volume_id_set(ULONG new_volume_id);
UINT  _fx_partition_offset_calculate(void  *partition_sector, UINT partition,
                                     ULONG *partition_start, ULONG *partition_size);

/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_utility_application_define(void *first_unused_memory)
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


static void    no_partition_test()
{
UINT i;
UINT status;
ULONG total_sectors;
ULONG partition_start;
ULONG partition_size;
                  
    /* Loop to test FAT 12, 16, 32 and exFAT.   */
    for (i = 0; i < TEST_COUNT; i ++)
    {
        if (i == 0)
        {
            /* Format the media with FAT12.  This needs to be done before opening it!  */
            total_sectors = 256;
            status =  fx_media_format(&ram_disk, 
                                     _fx_ram_driver,         // Driver entry
                                     ram_disk_memory_large,  // RAM disk memory pointer
                                     cache_buffer,           // Media buffer pointer
                                     CACHE_SIZE,             // Media buffer size 
                                     "MY_RAM_DISK",          // Volume Name
                                     1,                      // Number of FATs
                                     32,                     // Directory Entries
                                     0,                      // Hidden sectors
                                     total_sectors,          // Total sectors 
                                     512,                    // Sector size   
                                     8,                      // Sectors per cluster
                                     1,                      // Heads
                                     1);                     // Sectors per track 
        }     
        else if (i == 1)
        {
            /* Format the media with FAT16.  This needs to be done before opening it!  */
            total_sectors = 4200 * 8;
            status =  fx_media_format(&ram_disk, 
                                     _fx_ram_driver,         // Driver entry            
                                     ram_disk_memory_large,  // RAM disk memory pointer
                                     cache_buffer,           // Media buffer pointer
                                     CACHE_SIZE,             // Media buffer size 
                                     "MY_RAM_DISK",          // Volume Name
                                     1,                      // Number of FATs
                                     32,                     // Directory Entries
                                     0,                      // Hidden sectors
                                     total_sectors,          // Total sectors 
                                     512,                    // Sector size   
                                     8,                      // Sectors per cluster
                                     1,                      // Heads
                                     1);                     // Sectors per track 
        }  
        else if (i == 2)
        {
            /* Format the media with FAT32.  This needs to be done before opening it!  */
            total_sectors = 70000 * 8;
            status =  fx_media_format(&ram_disk, 
                                     _fx_ram_driver,         // Driver entry            
                                     ram_disk_memory_large,  // RAM disk memory pointer
                                     cache_buffer,           // Media buffer pointer
                                     CACHE_SIZE,             // Media buffer size 
                                     "MY_RAM_DISK",          // Volume Name
                                     1,                      // Number of FATs
                                     32,                     // Directory Entries
                                     0,                      // Hidden sectors
                                     total_sectors,          // Total sectors 
                                     512,                    // Sector size   
                                     8,                      // Sectors per cluster
                                     1,                      // Heads
                                     1);                     // Sectors per track 
        }
        else if (i == 3)
        {
            /* Format the media with FAT12.  This needs to be done before opening it!  */
            total_sectors = 256;
            status =  fx_media_format(&ram_disk, 
                                     _fx_ram_driver,         // Driver entry
                                     ram_disk_memory_large,  // RAM disk memory pointer
                                     cache_buffer,           // Media buffer pointer
                                     CACHE_SIZE,             // Media buffer size 
                                     "MY_RAM_DISK",          // Volume Name
                                     1,                      // Number of FATs
                                     32,                     // Directory Entries
                                     0,                      // Hidden sectors
                                     total_sectors,          // Total sectors 
                                     128,                    // Sector size   
                                     8,                      // Sectors per cluster
                                     1,                      // Heads
                                     1);                     // Sectors per track 
        }  
        else if (i == 4)
        {
            /* Format the media with FAT12.  This needs to be done before opening it!  */
            total_sectors = 256;
            status =  fx_media_format(&ram_disk, 
                                     _fx_ram_driver,         // Driver entry
                                     ram_disk_memory_large,  // RAM disk memory pointer
                                     cache_buffer,           // Media buffer pointer
                                     CACHE_SIZE,             // Media buffer size 
                                     "MY_RAM_DISK",          // Volume Name
                                     1,                      // Number of FATs
                                     32,                     // Directory Entries
                                     0,                      // Hidden sectors
                                     total_sectors,          // Total sectors 
                                     1024,                   // Sector size   
                                     8,                      // Sectors per cluster
                                     1,                      // Heads
                                     1);                     // Sectors per track 
        }
#ifdef FX_ENABLE_EXFAT
        else
        {

            /* Format the media with exFAT.  This needs to be done before opening it!  */
            total_sectors = 256;
            status =  fx_media_exFAT_format(&ram_disk, 
                                            _fx_ram_driver,         // Driver entry            
                                            ram_disk_memory_large,  // RAM disk memory pointer
                                            cache_buffer,           // Media buffer pointer
                                            CACHE_SIZE,             // Media buffer size 
                                            "MY_RAM_DISK",          // Volume Name
                                            1,                      // Number of FATs
                                            0,                      // Hidden sectors
                                            total_sectors,          // Total sectors 
                                            512,                   // Sector size   
                                            4,                      // exFAT Sectors per cluster
                                            12345,                  // Volume ID
                                            0);                     // Boundary unit
        }
#endif
        return_if_fail( status == FX_SUCCESS);

        /* Get partition offset and size. */
        _fx_partition_offset_calculate(ram_disk_memory_large, 0, &partition_start, &partition_size);

        /* Check partition offset and size. */
        return_value_if_fail(partition_start == 0, 6);
        return_value_if_fail(partition_size == total_sectors, 7);
    }
}


/* Define the test threads.  */

static void    ftest_0_entry(ULONG thread_input)
{

UCHAR       oem_name[8] = {'O', 'E', 'M', ' ', 'N', 'A', 'M', 'E'};
ULONG       partition_start;
ULONG       partition_size;
ULONG       FAT_sector;
ULONG64     value;
CHAR        buffer[16];
UINT        size;
UINT        status;
UINT        partition_sector_value;
FX_MEDIA    tmp_media;
ULONG       temp;
ULONG       temp1;
ULONG       temp2;
ULONG       temp3;
ULONG       temp4;
ULONG       temp5;
ULONG       temp6;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Utility test...........................................");

    /* Modify OEM_NAME. */
    fx_media_format_oem_name_set(oem_name);

    /* Check OEM_NAME. */
    return_value_if_fail(memcmp(oem_name, _fx_media_format_oem_name, sizeof(oem_name)) == 0, 1);

    /* Modify media_type. */
    fx_media_format_type_set(0xF0);

    /* Check media_type. */
    return_value_if_fail(_fx_media_format_media_type == 0xF0, 2);

    /* Modify volume_id. */
    fx_media_format_volume_id_set(0xFF);

    /* Check media_type. */
    return_value_if_fail(_fx_media_format_volume_id == 0xFF, 3);

    /* Get partition offset and size. */
    _fx_partition_offset_calculate(partition_sector, 0, &partition_start, &partition_size);

    /* Check partition offset and size. */
    return_value_if_fail(partition_start == 0x0520, 4);
    return_value_if_fail(partition_size == 0xF5A2E0, 5);

    /* Check for error in _fx_partition_offset_calculate when buffer is invalid.  */

    /* Induce error in buffer */
    partition_sector[510] = 0xFF;

    status = _fx_partition_offset_calculate(partition_sector, 0, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_NOT_FOUND)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate!\n");
        test_control_return(1);
    }

    /* Restore buffer */
    partition_sector[510] = 0x55;

    /* BRANCH COVERAGE TESTS START */

    /* Induce error in buffer */
    partition_sector[511] = 0xFF;

    status = _fx_partition_offset_calculate(partition_sector, 0, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_NOT_FOUND)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate!\n");
        test_control_return(2);
    }

    /* Restore buffer */
    partition_sector[511] = 0xAA;

    partition_sector_value = partition_sector[0];

    /* Induce error in buffer */
    partition_sector[0] = 0xFF;

    status = _fx_partition_offset_calculate(partition_sector, 100, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_NOT_FOUND)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(3);
    }

    /* Restore buffer */
    partition_sector[0] = partition_sector_value;

    partition_sector_value = partition_sector[2];

    /* Induce error in buffer */
    partition_sector[2] = 0xFF;

    status = _fx_partition_offset_calculate(partition_sector, 100, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_NOT_FOUND)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(4);
    }

    /* Restore buffer */
    partition_sector[2] = partition_sector_value;

    partition_sector_value = partition_sector[0];

    /* Induce error in buffer */
    partition_sector[0] = 0xe9;

    status = _fx_partition_offset_calculate(partition_sector, 0, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_SUCCESS)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(5);
    }

    /* Restore buffer */
    partition_sector[0] = partition_sector_value;


    partition_sector_value = partition_sector[0];

    /* Induce error in buffer */
    partition_sector[0] = 0xeb;

    status = _fx_partition_offset_calculate(partition_sector, 100, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_NOT_FOUND)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(5);
    }

    /* Restore buffer */
    partition_sector[0] = partition_sector_value;
    
    partition_sector_value = partition_sector[0];
    temp = partition_sector[2];

    /* Induce error in buffer */
    partition_sector[0] = 0xeb;
    partition_sector[2] = 0x90;

    status = _fx_partition_offset_calculate(partition_sector, 0, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_SUCCESS)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(6);
    }

    /* Restore buffer */
    partition_sector[0] = partition_sector_value;
    partition_sector[2] = temp;

    partition_sector_value = partition_sector[0];
    temp = partition_sector[2];

    /* Induce error in buffer */
    partition_sector[0] = 0xe9;
    partition_sector[2] = 0x90;

    status = _fx_partition_offset_calculate(partition_sector, 0, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_SUCCESS)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(6);
    }

    /* Restore buffer */
    partition_sector[0] = partition_sector_value;
    partition_sector[2] = temp;

    /* Test to check the partition_sector values, if there are good values for sectors per FAT. 
       We want the test code to return FX_NOT_FOUND after _fx_utility_partition_get() is called */

    /* Set real boot sector check to true */
    temp6 = partition_sector[0];
    partition_sector[0] = 0xe9;

    temp  = partition_sector[0x16];
    temp1 = partition_sector[0x17];
    temp2 = partition_sector[0x24];
    temp3 = partition_sector[0x25];
    temp4 = partition_sector[0x26];
    temp5 = partition_sector[0x27];

    /* Induce error in buffer */
    partition_sector[0x16] = 0;
    partition_sector[0x17] = 0;
    partition_sector[0x24] = 0;
    partition_sector[0x25] = 0;
    partition_sector[0x26] = 0;
    partition_sector[0x27] = 0;
    
    status = _fx_partition_offset_calculate(partition_sector, 100, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_NOT_FOUND)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(7);
    }

    /* Restore buffer */
    partition_sector[0x16] = temp;
    partition_sector[0x17] = temp1;
    partition_sector[0x24] = temp2;
    partition_sector[0x25] = temp3;
    partition_sector[0x26] = temp4;
    partition_sector[0x27] = temp5;

    /* Induce error in buffer */
    partition_sector[0x17] = 0;
    partition_sector[0x24] = 0;
    partition_sector[0x25] = 0;
    partition_sector[0x26] = 0;
    partition_sector[0x27] = 0;
    
    status = _fx_partition_offset_calculate(partition_sector, 0, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_SUCCESS)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(8);
    }

    /* Restore buffer */
    partition_sector[0x17] = temp1;
    partition_sector[0x24] = temp2;
    partition_sector[0x25] = temp3;
    partition_sector[0x26] = temp4;
    partition_sector[0x27] = temp5;

    /* Induce error in buffer */
    partition_sector[0x16] = 0;
    partition_sector[0x24] = 0;
    partition_sector[0x25] = 0;
    partition_sector[0x26] = 0;
    partition_sector[0x27] = 0;
    
    status = _fx_partition_offset_calculate(partition_sector, 0, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_SUCCESS)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(9);
    }

    /* Restore buffer */
    partition_sector[0x16] = temp;
    partition_sector[0x24] = temp2;
    partition_sector[0x25] = temp3;
    partition_sector[0x26] = temp4;
    partition_sector[0x27] = temp5;

    /* Induce error in buffer */
    partition_sector[0x16] = 0;
    partition_sector[0x17] = 0;
    partition_sector[0x25] = 0;
    partition_sector[0x26] = 0;
    partition_sector[0x27] = 0;
    
    status = _fx_partition_offset_calculate(partition_sector, 0, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_SUCCESS)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(10);
    }

    /* Restore buffer */
    partition_sector[0x16] = temp;
    partition_sector[0x17] = temp1;
    partition_sector[0x25] = temp3;
    partition_sector[0x26] = temp4;
    partition_sector[0x27] = temp5;

    /* Induce error in buffer */
    partition_sector[0x16] = 0;
    partition_sector[0x17] = 0;
    partition_sector[0x24] = 0;
    partition_sector[0x26] = 0;
    partition_sector[0x27] = 0;
    
    status = _fx_partition_offset_calculate(partition_sector, 0, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_SUCCESS)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(11);
    }

    /* Restore buffer */
    partition_sector[0x16] = temp;
    partition_sector[0x17] = temp1;
    partition_sector[0x24] = temp2;
    partition_sector[0x26] = temp4;
    partition_sector[0x27] = temp5;

    /* Induce error in buffer */
    partition_sector[0x16] = 0;
    partition_sector[0x17] = 0;
    partition_sector[0x24] = 0;
    partition_sector[0x25] = 0;
    partition_sector[0x27] = 0;
    
    status = _fx_partition_offset_calculate(partition_sector, 0, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_SUCCESS)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(12);
    }

    /* Restore buffer */
    partition_sector[0x16] = temp;
    partition_sector[0x17] = temp1;
    partition_sector[0x24] = temp2;
    partition_sector[0x25] = temp3;
    partition_sector[0x27] = temp5;

    /* Induce error in buffer */
    partition_sector[0x16] = 0;
    partition_sector[0x17] = 0;
    partition_sector[0x24] = 0;
    partition_sector[0x25] = 0;
    partition_sector[0x26] = 0;
    
    status = _fx_partition_offset_calculate(partition_sector, 0, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_SUCCESS)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(13);
    }

    /* Restore buffer */
    partition_sector[0x16] = temp;
    partition_sector[0x17] = temp1;
    partition_sector[0x24] = temp2;
    partition_sector[0x25] = temp3;
    partition_sector[0x26] = temp4;

    temp = partition_sector[0x20];
    temp1 = partition_sector[0x21];
    temp2 = partition_sector[0x22];
    temp3 = partition_sector[0x23];

    /* Set calculatation of the total sectors based of FAT32 */
    temp4 = partition_sector[0x13];
    temp5 = partition_sector[0x14];
    partition_sector[0x13] = 0;
    partition_sector[0x14] = 0;

    /* Induce error in buffer */
    partition_sector[0x20] = 0;
    partition_sector[0x21] = 0;
    partition_sector[0x22] = 0;
    partition_sector[0x23] = 0;

    status = _fx_partition_offset_calculate(partition_sector, 100, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_NOT_FOUND)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(14);
    }

    /* Restore buffer */
    partition_sector[0x20] = temp;
    partition_sector[0x21] = temp1;
    partition_sector[0x22] = temp2;
    partition_sector[0x23] = temp3;

    /* Induce error in buffer */
    partition_sector[0x21] = 0;
    partition_sector[0x22] = 0;
    partition_sector[0x23] = 0;

    status = _fx_partition_offset_calculate(partition_sector, 0, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_SUCCESS)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(15);
    }

    /* Restore buffer */
    partition_sector[0x21] = temp1;
    partition_sector[0x22] = temp2;
    partition_sector[0x23] = temp3;

    /* Induce error in buffer */
    partition_sector[0x20] = 0;
    partition_sector[0x22] = 0;
    partition_sector[0x23] = 0;

    status = _fx_partition_offset_calculate(partition_sector, 0, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_SUCCESS)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(16);
    }

    /* Restore buffer */
    partition_sector[0x20] = temp;
    partition_sector[0x22] = temp2;
    partition_sector[0x23] = temp3;
    
    /* Induce error in buffer */
    partition_sector[0x20] = 0;
    partition_sector[0x21] = 0;
    partition_sector[0x23] = 0;

    status = _fx_partition_offset_calculate(partition_sector, 0, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_SUCCESS)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(17);
    }

    /* Restore buffer */
    partition_sector[0x20] = temp;
    partition_sector[0x21] = temp1;
    partition_sector[0x23] = temp3;
    
    /* Induce error in buffer */
    partition_sector[0x20] = 0;
    partition_sector[0x21] = 0;
    partition_sector[0x22] = 0;

    status = _fx_partition_offset_calculate(partition_sector, 0, &partition_start, &partition_size);

    /* Check for the error.  */
    if (status != FX_SUCCESS)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(18);
    }
    
    /* Check for partition_start and partition_size to be NULL */
    status = _fx_partition_offset_calculate(partition_sector, 0,  FX_NULL, FX_NULL);

    /* Check for the error.  */
    if (status != FX_SUCCESS)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(19);
    }
    
    /* Restore buffer */
    partition_sector[0x20] = temp;
    partition_sector[0x21] = temp1;
    partition_sector[0x22] = temp2;

    /* Restore real boot sector and the total sector check to default */
    partition_sector[0] = temp6;
    partition_sector[0x13] = temp4;
    partition_sector[0x14] = temp5;

    /* Check for partition_start and partition_size to be NULL  to cover line 236 and 242 */
    status = _fx_partition_offset_calculate(partition_sector, 0,  FX_NULL, FX_NULL);

    /* Check for the error.  */
    if (status != FX_SUCCESS)
    {
        /* Print error message.  */
        printf("Error in checking of _fx_partition_offset_calculate! %d\n",status);
        test_control_return(20);
    }

    /* BRANCH COVERAGE TESTS END */

    /* Invoke _fx_utility_memory_set and _fx_utility_string_length_get. */
    _fx_utility_memory_set((UCHAR*)buffer, 0, 0);
    _fx_utility_memory_set((UCHAR*)buffer, 0, sizeof(buffer));
    size = _fx_utility_string_length_get(buffer, sizeof(buffer));
    return_if_fail(size == 0);

    buffer[0] = 'a';
    size = _fx_utility_string_length_get(buffer, sizeof(buffer));
    return_if_fail(size == 1);

    _fx_utility_memory_set((UCHAR*)buffer, 'a', sizeof(buffer));
    size = _fx_utility_string_length_get(buffer, sizeof(buffer));
    return_if_fail(size == sizeof(buffer));

    _fx_utility_memory_set((UCHAR*)buffer, 'a', sizeof(buffer));
    size = _fx_utility_string_length_get(buffer, 1);
    return_if_fail(size == 1);

    /* Tests for utility_64_unsigned_read, utility_64_unsigned_write and utility_FAT_sector_get. */
    _fx_utility_64_unsigned_write((UCHAR*)buffer, 0x0001);
    value = _fx_utility_64_unsigned_read((UCHAR*)buffer);
    return_if_fail(0x0001 == value);

    /* Tests for utility_FAT_sector_get. */
    tmp_media.fx_media_bytes_per_sector = 512;
    tmp_media.fx_media_reserved_sectors = 0;

    /* 12-bit FAT. */
#ifdef FX_ENABLE_EXFAT
    tmp_media.fx_media_FAT_type = FX_FAT12;
#else
    tmp_media.fx_media_12_bit_FAT = 1;
#endif /* FX_ENABLE_EXFAT */

    /* The FAT entry of cluster 400 is located at the second sector. */
    FAT_sector = _fx_utility_FAT_sector_get(&tmp_media, 400);
    return_if_fail(1 == FAT_sector);

    /* 16-bit FAT. */
#ifdef FX_ENABLE_EXFAT
    tmp_media.fx_media_FAT_type = FX_FAT16;
#else
    tmp_media.fx_media_12_bit_FAT = 0;
    tmp_media.fx_media_32_bit_FAT = 0;
#endif /* FX_ENABLE_EXFAT */

    /* The FAT entry of cluster 400 is located at the second sector. */
    FAT_sector = _fx_utility_FAT_sector_get(&tmp_media, 400);
    return_if_fail(1 == FAT_sector);

    /* 32-bit FAT or exFAT. */
#ifdef FX_ENABLE_EXFAT
    tmp_media.fx_media_FAT_type = FX_FAT32;
#else
    tmp_media.fx_media_12_bit_FAT = 0;
    tmp_media.fx_media_32_bit_FAT = 1;
#endif /* FX_ENABLE_EXFAT */

    /* The FAT entry of cluster 400 is located at the fourth sector. */
    FAT_sector = _fx_utility_FAT_sector_get(&tmp_media, 400);
    return_if_fail(3 == FAT_sector);

    no_partition_test();

    printf("SUCCESS!\n");
    test_control_return(0);
}
