/* This FileX test concentrates on the basic directory create/delete operations.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_directory.h"
#include   "fx_ram_driver_test.h"
#include   "fx_fault_tolerant.h"
#include   "fx_utility.h"
#include   <stdio.h>

#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              16*128
#ifdef FX_ENABLE_FAULT_TOLERANT
#define     FAULT_TOLERANT_SIZE     FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE
#else
#define     FAULT_TOLERANT_SIZE     0
#endif


#define DIR_PATH "MYDIR\\"
#define DIR_PATH_SIZE (sizeof(DIR_PATH) - 1)

/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD                ftest_0;
#endif
static FX_MEDIA                 ram_disk;
static CHAR                     name[FX_MAX_LONG_NAME_LEN+50];
static CHAR                     volume_name[FX_MAX_LONG_NAME_LEN+1];
static FX_FILE                  file_1;
static FX_FILE                  file_2;
static FX_FILE                  file_3;
static FX_FILE                  file_4;
static FX_FILE                  file_5;
static FX_FILE                  file_6;
static FX_FILE                  file_7;
static FX_FILE                  file_8;
static FX_FILE                  file_9;
static FX_FILE                  file_10;
static FX_FILE                  file_11;
static FX_FILE                  file_12;
static FX_FILE                  file_13;
static FX_FILE                  file_14;
static FX_FILE                  file_15;
static FX_FILE                  file_16;
static FX_FILE                  file_17;
static FX_FILE                  file_18;
static FX_FILE                  file_19;
static FX_FILE                  file_20;
static FX_FILE                  file_21;
static FX_FILE                  file_22;
static FX_FILE                  file_23;
static FX_FILE                  file_24;
static FX_FILE                  file_25;
static FX_FILE                  file_26;
static FX_FILE                  file_27;
static FX_FILE                  file_28;
static FX_FILE                  file_29;
static FX_FILE                  file_30;
static FX_FILE                  file_31;
static FX_FILE                  file_32;
static FX_FILE                  file_33;
static FX_FILE                  file_34;
static FX_FILE                  file_35;
static FX_FILE                  file_36;
static FX_FILE                  file_37;
static FX_FILE                  file_38;
static FX_FILE                  file_39;
static FX_FILE                  file_40;
static FX_FILE                  file_41;
static FX_FILE                  file_42;
static FX_FILE                  file_43;
static FX_FILE                  file_44;
static UCHAR                    buffer[128];


static UCHAR my_unicode_name[] =
{0xFF,0xC1, 0xFF,0x00, 0xFF,0x00, 0xFF,0x02,
0xFF,0xD6, 0xFF,0xC7, 0xFF,0x00, 0xFF,0xC7,
0xFF,0xB9, 0xFF,0x00, 0xFF,0xC7, 0xFF,0xB2,
0xFF,0xD6, 0xFF,0xC7, 0xFF,0x00, 0x00,0x00};

static UCHAR my_unicode_name1[] =
{0x00,0xFF, 0x00,0xFF, 0x00,0xFF, 0x00,0xFF,
0x00,0xFF, 0x00,0xFF, 0x00,0xFF, 0x00,0xFF,
0x00,0xFF, 0x00,0xFF, 0x00,0xFF, 0x00,0xFF,
0x00,0xFF, 0x00,0xFF, 0x00,0xFF, 0x00,0x00};




/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                    *ram_disk_memory;
static UCHAR                    *cache_buffer;
static UCHAR                    *fault_tolerant_buffer;   
#else
static UCHAR                    cache_buffer[CACHE_SIZE];
static UCHAR                    fault_tolerant_buffer[FAULT_TOLERANT_SIZE]; 
#endif


extern ULONG   _fx_ram_driver_copy_default_format;


/* Define thread prototypes.  */

void    filex_directory_create_delete_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);




/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_directory_create_delete_application_define(void *first_unused_memory)
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
    fault_tolerant_buffer = pointer;
    pointer += FAULT_TOLERANT_SIZE;
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
ULONG           temp;
UINT            attributes;
UINT            i;
#ifdef EXTENDED_NAME_MANGLE_TEST    
UINT            j, k, l;
#endif
CHAR            *name_ptr;
FX_DIR_ENTRY    dir_entry;
FX_DIR_ENTRY    search_directory;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Directory create/delete test...........................");

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

/* We need a larger disk to test the feature of fault tolerant. */                            
#ifdef FX_ENABLE_FAULT_TOLERANT
                            512 * 8,                // Total sectors 
                            256,                    // Sector size
                            8,                      // Sectors per cluster
#else
                            512,                    // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
#endif

                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
    /* Attempt to create a directory before the media is opened to generate an error */
    status = fx_directory_create(&ram_disk, "/A0");
    return_if_fail( status == FX_MEDIA_NOT_OPEN);
    
    /* Attempt to delete a directory before the media is opened to generate an error */
    status = fx_directory_delete(&ram_disk, "/A0");
    return_if_fail( status == FX_MEDIA_NOT_OPEN);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);
    
/* We need a larger disk to test the feature of fault tolerant. */                            
#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Enable fault tolerant if FX_ENABLE_FAULT_TOLERANT is defined. */
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);
#endif

/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING

    /* send null pointer to directory create to generate an error */
    status = fx_directory_create(FX_NULL, "/A0");
    return_if_fail( status == FX_PTR_ERROR);
    
    /* directory delete */
    status = fx_directory_delete(FX_NULL, "/A0");
    return_if_fail( status == FX_PTR_ERROR);

#endif /* FX_DISABLE_ERROR_CHECKING */
    
    /* Attempt to create a directory with an invalid name to generate an error */
    status = fx_directory_create(&ram_disk, "");
    return_if_fail( status == FX_INVALID_NAME);
    
    /* Attempt to create a directory with an invalid name to generate an error */
    status = fx_directory_create(&ram_disk, "/A0/error\\error");
    return_if_fail( status == FX_INVALID_PATH);
    
    /* Attempt to create a directory while the media is write protected to generate an error */
    ram_disk.fx_media_driver_write_protect = FX_TRUE;
    status = fx_directory_create(&ram_disk, "/A0");
    return_if_fail( status == FX_WRITE_PROTECT);
    
    /* try to create a directory while the media is write protected to generate an error */
    status = fx_directory_delete(&ram_disk, "/A0");
    return_if_fail( status == FX_WRITE_PROTECT);
    ram_disk.fx_media_driver_write_protect = FX_FALSE;
    
    /* Attempt to create a directory when there is no space to generate an error */
    temp = ram_disk.fx_media_available_clusters;
    ram_disk.fx_media_available_clusters = 0;
    status = fx_directory_create(&ram_disk, "/A0");
    return_if_fail( status == FX_NO_MORE_SPACE);
    ram_disk.fx_media_available_clusters = temp;

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
    status +=  fx_file_create(&ram_disk, "not_a_dir");
    return_if_fail( status == FX_SUCCESS);

    /* Attempt to create the same directory again.  */
    status =   fx_directory_create(&ram_disk, "/A0");
    return_if_fail( status == FX_ALREADY_CREATED);

    /* Create the next level of sub-directories.  */
    status =   fx_directory_create(&ram_disk, "/A0/A1");
    status +=  fx_directory_create(&ram_disk, "/A0/A2");
    status +=  fx_directory_create(&ram_disk, "/A0/A3");
    status +=  fx_directory_create(&ram_disk, "/A0/A4");
    status +=  fx_directory_create(&ram_disk, "/A0/A5");
    
    status +=  fx_directory_create(&ram_disk, "/B0/B1");
    status +=  fx_directory_create(&ram_disk, "/B0/B2");
    status +=  fx_directory_create(&ram_disk, "/B0/B3");
    status +=  fx_directory_create(&ram_disk, "/B0/B4");
    status +=  fx_directory_create(&ram_disk, "/B0/B5");
    
    status +=  fx_directory_create(&ram_disk, "/C0/C1");
    status +=  fx_directory_create(&ram_disk, "/C0/C2");
    status +=  fx_directory_create(&ram_disk, "/C0/C3");
    status +=  fx_directory_create(&ram_disk, "/C0/C4");
    status +=  fx_directory_create(&ram_disk, "/C0/C5");

    status +=  fx_directory_create(&ram_disk, "/D0/D1");
    status +=  fx_directory_create(&ram_disk, "/D0/D2");
    status +=  fx_directory_create(&ram_disk, "/D0/D3");
    status +=  fx_directory_create(&ram_disk, "/D0/D4");
    status +=  fx_directory_create(&ram_disk, "/D0/D5");

    status +=  fx_directory_create(&ram_disk, "/E0/E1");
    status +=  fx_directory_create(&ram_disk, "/E0/E2");
    status +=  fx_directory_create(&ram_disk, "/E0/E3");
    status +=  fx_directory_create(&ram_disk, "/E0/E4");
    status +=  fx_directory_create(&ram_disk, "/E0/E5");

    status +=  fx_directory_create(&ram_disk, "/F0/F1");
    status +=  fx_directory_create(&ram_disk, "/F0/F2");
    status +=  fx_directory_create(&ram_disk, "/F0/F3");
    status +=  fx_directory_create(&ram_disk, "/F0/F4");
    status +=  fx_directory_create(&ram_disk, "/F0/F5");
    
    status +=  fx_directory_create(&ram_disk, "/G0/G1");
    status +=  fx_directory_create(&ram_disk, "/G0/G2");
    status +=  fx_directory_create(&ram_disk, "/G0/G3");
    status +=  fx_directory_create(&ram_disk, "/G0/G4");
    status +=  fx_directory_create(&ram_disk, "/G0/G5");

    status +=  fx_directory_create(&ram_disk, "/H0/H1");
    status +=  fx_directory_create(&ram_disk, "/H0/H2");
    status +=  fx_directory_create(&ram_disk, "/H0/H3");
    status +=  fx_directory_create(&ram_disk, "/H0/H4");
    status +=  fx_directory_create(&ram_disk, "/H0/H5");

    status +=  fx_directory_create(&ram_disk, "/I0/I1");
    status +=  fx_directory_create(&ram_disk, "/I0/I2");
    status +=  fx_directory_create(&ram_disk, "/I0/I3");
    status +=  fx_directory_create(&ram_disk, "/I0/I4");
    status +=  fx_directory_create(&ram_disk, "/I0/I5");
    return_if_fail( status == FX_SUCCESS);
    
    /* Now create the third level of sub-directories... */
    status =   fx_directory_create(&ram_disk, "/A0/A1/A00");
    status +=  fx_directory_create(&ram_disk, "/B0/B2/B00");
    status +=  fx_directory_create(&ram_disk, "/C0/C3/C00");
    status +=  fx_directory_create(&ram_disk, "/D0/D4/D00");
    status +=  fx_directory_create(&ram_disk, "/E0/E5/E00");
    status +=  fx_directory_create(&ram_disk, "/F0/F1/F00");
    status +=  fx_directory_create(&ram_disk, "/G0/G1/G00");
    status +=  fx_directory_create(&ram_disk, "/H0/H2/H00");
    status +=  fx_directory_create(&ram_disk, "/I0/I3/I00");
    return_if_fail( status == FX_SUCCESS);
    
    /* Attempt to delete something that is not a directory */
    status = fx_directory_delete(&ram_disk, "not_a_dir");
    return_if_fail( status == FX_NOT_DIRECTORY);
    
    /* Attempt to delete a directory that is not empty */
    status = fx_directory_delete(&ram_disk, "/A0");
    return_if_fail( status == FX_DIR_NOT_EMPTY);
    
    /* Attempt to delete a directory that is read only */
    status =  fx_directory_attributes_read(&ram_disk, "/A0/A1/A00", &attributes);
    status += fx_directory_attributes_set(&ram_disk, "/A0/A1/A00", FX_READ_ONLY);
    status += fx_directory_delete(&ram_disk, "/A0/A1/A00");
    return_if_fail( status == FX_WRITE_PROTECT);
    status += fx_directory_attributes_set(&ram_disk, "/A0/A1/A00", attributes);

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
    return_if_fail( status == FX_SUCCESS);

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
    return_if_fail( status == FX_SUCCESS);

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
    return_if_fail( status == FX_SUCCESS);

    /* Attempt to delete a directory again.  */
    status =   fx_directory_delete(&ram_disk, "/A0");
    return_if_fail( status == FX_NOT_FOUND);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

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
                            2,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_if_fail( status == FX_SUCCESS);

    /* Create a directory name that is too large.  */
    for (i = 0; i < FX_MAX_LONG_NAME_LEN+1; i++)
    {
    
        name[i] = 'a';
    }
    name[FX_MAX_LONG_NAME_LEN + 1] =  0;

    /* Attempt to create a directory with too long of a name.  */
    status = fx_directory_create(&ram_disk, name);
    return_if_fail( status == FX_INVALID_NAME);
    
    /* Attempt to create a directory with no more clusters.  */
    temp =  ram_disk.fx_media_total_clusters;
    ram_disk.fx_media_total_clusters =  0;
    status = fx_directory_create(&ram_disk, "sub1");
    ram_disk.fx_media_total_clusters =  temp;
    return_if_fail( status == FX_NO_MORE_SPACE);
       
    /* Attempt to create a directory with a FAT read error.  */
    _fx_utility_fat_entry_read_error_request =  1;
    status = fx_directory_create(&ram_disk, "sub1");
    _fx_utility_fat_entry_read_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);

    /* Create a subdirectory with a search pointer that must wrap after the allocation.  */
    ram_disk.fx_media_cluster_search_start =  ram_disk.fx_media_total_clusters + (FX_FAT_ENTRY_START - 1);
    status = fx_directory_create(&ram_disk, "sub1");
    return_if_fail( status == FX_SUCCESS);

    /* Create a subdirectory with a search pointer that must wrap after the allocation.  */
    ram_disk.fx_media_cluster_search_start =  ram_disk.fx_media_total_clusters + (FX_FAT_ENTRY_START - 1);
    status = fx_directory_create(&ram_disk, "sub2");
    return_if_fail( status == FX_SUCCESS);

    /* Create a subdirectory with a search pointer that starts on a non-free cluster.  */
    ram_disk.fx_media_cluster_search_start =  FX_FAT_ENTRY_START;
    status = fx_directory_create(&ram_disk, "sub3");
    return_if_fail( status == FX_SUCCESS);

    /* Create a subdirectory with a logical sector read error.  */
    _fx_utility_logical_sector_read_error_request =  13;
    status = fx_directory_create(&ram_disk, "sub4");
    _fx_utility_logical_sector_read_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);

#ifndef FX_DISABLE_CACHE
    /* Create a subdirectory with a logical flush error.  */
    _fx_utility_logical_sector_flush_error_request =  1;
    status = fx_directory_create(&ram_disk, "sub5");
    _fx_utility_logical_sector_flush_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);
#endif

    /* Create a subdirectory with an error clearing the additional sectors of a cluster.  */
    _fx_ram_driver_io_error_request =  8;
    status = fx_directory_create(&ram_disk, "sub6");
    _fx_ram_driver_io_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);

    /* Create a subdirectory with an error on writing the FAT entry.  */
    _fx_utility_fat_entry_write_error_request =  1;
    status = fx_directory_create(&ram_disk, "sub7");
    _fx_utility_fat_entry_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);

    /* Create a subdirectory with an error on writing the first director entry.  */
    _fx_directory_entry_write_error_request =  1;
    status = fx_directory_create(&ram_disk, "sub8");
    _fx_directory_entry_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);

    /* Create a subdirectory with an error on writing the second director entry.  */
    _fx_directory_entry_write_error_request =  2;
    status = fx_directory_create(&ram_disk, "sub9");
    _fx_directory_entry_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);
    
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
                            2,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_if_fail( status == FX_SUCCESS);
    
    /* Create a series of sub-directories... in preparation for our directory delete checking.  */
    status =  fx_directory_create(&ram_disk, "SUB1");
    status += fx_directory_create(&ram_disk, "SUB2");
    status += fx_directory_create(&ram_disk, "SUB3");
    status += fx_directory_create(&ram_disk, "SUB4");
    status += fx_directory_create(&ram_disk, "SUB5");
    status += fx_directory_create(&ram_disk, "SUB6");
    status += fx_directory_create(&ram_disk, "SUB7");
    status += fx_directory_create(&ram_disk, "SUB8");
    return_if_fail( status == FX_SUCCESS);

    /* Delete a directory but with a FAT read error on the first cluster.  */
    _fx_utility_fat_entry_read_error_request =  1;
    status =  fx_directory_delete(&ram_disk, "SUB8");
    _fx_utility_fat_entry_read_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);
    
    /* Delete a directory but with a cluster error - bad small value.  */
    _fx_utility_fat_entry_read_error_request =  10001;
    status =  fx_directory_delete(&ram_disk, "SUB8");
    _fx_utility_fat_entry_read_error_request =  0;
    return_if_fail( status == FX_FAT_READ_ERROR);
    
    /* Delete a directory but with a cluster error - bad total clusters.  */
    temp =  ram_disk.fx_media_total_clusters;
    ram_disk.fx_media_total_clusters =  0;
    status =  fx_directory_delete(&ram_disk, "SUB8");
    ram_disk.fx_media_total_clusters =  temp;
    return_if_fail( status == FX_FAT_READ_ERROR);
    
    /* Delete a directory but with a cluster error - same value.  */
    _fx_utility_fat_entry_read_error_request =  30001;
    status =  fx_directory_delete(&ram_disk, "SUB8");
    _fx_utility_fat_entry_read_error_request =  0;
    return_if_fail( status == FX_FAT_READ_ERROR);
    
    /* Delete a directory but with a directory entry read error.  */
    _fx_directory_entry_read_error_request =  1;
    status =  fx_directory_delete(&ram_disk, "SUB8");
    _fx_directory_entry_read_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);
    
    /* Delete a directory but with a directory entry write error.  */
    _fx_directory_entry_write_error_request =  1;
    status =  fx_directory_delete(&ram_disk, "SUB8");
    _fx_directory_entry_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);

    /* Delete a directory wtih large error request values just to exercise those paths - should be successful.  */
    _fx_directory_entry_read_error_request =  10000;
    _fx_directory_entry_write_error_request =  10000;
    status =  fx_directory_delete(&ram_disk, "SUB8");
    _fx_directory_entry_read_error_request =  0;
    _fx_directory_entry_write_error_request =  0;
    return_if_fail( status == FX_SUCCESS);

    /* Delete a directory with an error in the freeing of the cluster traversal FAT read.  */
    _fx_utility_fat_entry_read_error_request =  2;
    status =  fx_directory_delete(&ram_disk, "SUB7");
    _fx_utility_fat_entry_read_error_request  =  0;
    return_if_fail( status == FX_IO_ERROR);

    /* Delete a directory but with a cluster error in releasing cluster - bad small value.  */
    _fx_utility_fat_entry_read_error_request =  10002;
    status =  fx_directory_delete(&ram_disk, "SUB6");
    _fx_utility_fat_entry_read_error_request =  0;
    return_if_fail( status == FX_FAT_READ_ERROR);
    
    /* Delete a directory but with a cluster error in releasing cluster - bad total clusters.  */
    temp =  ram_disk.fx_media_total_clusters;
    _fx_utility_fat_entry_read_error_request = 40002;
    status =  fx_directory_delete(&ram_disk, "SUB5");
    ram_disk.fx_media_total_clusters =  temp;
    return_if_fail( status == FX_FAT_READ_ERROR);
    
    /* Delete a directory but with a cluster error in releasing cluster - same value.  */
    _fx_utility_fat_entry_read_error_request =  30002;
    status =  fx_directory_delete(&ram_disk, "SUB4");
    _fx_utility_fat_entry_read_error_request =  0;
    return_if_fail( status == FX_FAT_READ_ERROR);
    
    /* Delete a directory but with a FAT write error in releasing cluster.  */
    _fx_utility_fat_entry_write_error_request =  1;
    status =  fx_directory_delete(&ram_disk, "SUB3");
    _fx_utility_fat_entry_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

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
                            14000,                  // Total sectors 
                            128,                    // Sector size   
                            2,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_if_fail( status == FX_SUCCESS);

    /* Create a sub-directory to work from.  */
    status =  fx_directory_create(&ram_disk, "SUB1");
    
    /* Set the default path here.  */
    status +=  fx_directory_default_set(&ram_disk, "SUB1");

    /* Now try to create some unusual file name to test the internal free search logic.  */
    status +=  fx_file_create(&ram_disk, "..test");
    return_if_fail( status == FX_INVALID_NAME);

    /* Create a name with a character greater than 127.  */
    name[0] =  'n';
    name[1] =  'a';
    name[2] =  'm';
    name[3] =  'e';
    name[4] =  '\'';
    name[5] =  ')';
    name[6] =  '(';
    name[7] =  '`';
    name[8] =  (CHAR) 128;
    name[9] =  0;
    
    status =  fx_file_create(&ram_disk, name);
    status += fx_file_open(&ram_disk, &file_1, name, FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_SUCCESS);
    
    /* Create a name with a bunch of special characters.  */
    status =  fx_file_create(&ram_disk, "name-a_b}{()'!#$&@^%+,;=[]");
    status += fx_file_open(&ram_disk, &file_2, "name-a_b}{()'!#$&@^%+,;=[]", FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_SUCCESS);
    
    /* Create a name with an invalid character.  */
    name[0] =  'n';
    name[1] =  'a';
    name[2] =  'm';
    name[3] =  'e';
    name[4] =  (CHAR) 5;
    name[5] =  '\'';
    name[6] =  0;
    
    status =  fx_file_create(&ram_disk, name);
    return_if_fail( status == FX_INVALID_NAME);

    /* Create a long file name by position of the dot. */
    status =  fx_file_create(&ram_disk, "MYLONGERN.AM");
    status += fx_file_create(&ram_disk, "MY.TXT");
    status += fx_file_open(&ram_disk, &file_3, "MYLONGERN.AM", FX_OPEN_FOR_WRITE);
    status += fx_file_open(&ram_disk, &file_4, "MY.TXT", FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_SUCCESS);
    
    /* Create a set of very short long file name.  */
    status =  fx_file_create(&ram_disk, "n");
    status +=  fx_file_create(&ram_disk, "nW");
    status +=  fx_file_create(&ram_disk, "nWW");
    status +=  fx_file_create(&ram_disk, "nWWW");
    status +=  fx_file_create(&ram_disk, "nWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_create(&ram_disk, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW");
    status +=  fx_file_open(&ram_disk, &file_5, "n", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_6, "nW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_7, "nWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_8, "nWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_9, "nWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_10, "nWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_11, "nWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_12, "nWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_13, "nWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_14, "nWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_15, "nWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_16, "nWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_17, "nWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_18, "nWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_19, "nWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_20, "nWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_21, "nWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_22, "nWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_23, "nWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_24, "nWWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_25, "nWWWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_26, "nWWWWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_27, "nWWWWWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_28, "nWWWWWWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_29, "nWWWWWWWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_30, "nWWWWWWWWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_31, "nWWWWWWWWWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_32, "nWWWWWWWWWWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_33, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_34, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_35, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_36, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_37, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_38, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_39, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_40, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_41, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    status +=  fx_file_open(&ram_disk, &file_42, "nWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW", FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_SUCCESS);

    /* Build a super long file name that will require additional cluster allocation in the sub-directory.  */
    for (i = 0; i < FX_MAX_LONG_NAME_LEN; i++)
    {
        name[i] =  'a';
    }
    name[FX_MAX_LONG_NAME_LEN-2] =  0;

    /* Test the wrap of the FAT table by allocating at the end.  */
    ram_disk.fx_media_cluster_search_start =  ram_disk.fx_media_total_clusters + (FX_FAT_ENTRY_START-1);
    status =  fx_file_create(&ram_disk, name);
    status +=  fx_file_open(&ram_disk, &file_43, name, FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_SUCCESS);

    /* Change the name to create a new file name.  */
    name[FX_MAX_LONG_NAME_LEN-3] = '0';

    /* Test the wrap of the FAT table by allocating at the end.  */
    ram_disk.fx_media_cluster_search_start =  ram_disk.fx_media_total_clusters + (FX_FAT_ENTRY_START-1);
    status +=  fx_file_create(&ram_disk, name);
    status +=  fx_file_open(&ram_disk, &file_44, name, FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_SUCCESS);

    name[FX_MAX_LONG_NAME_LEN-3] = '1';

    /* Now try to allocate an entry with no more available clusters.  */
    temp =  ram_disk.fx_media_available_clusters;
    ram_disk.fx_media_available_clusters =  0;
    status +=  fx_file_create(&ram_disk, name);
    ram_disk.fx_media_available_clusters =  temp;
    return_if_fail( status == FX_NO_MORE_SPACE);

    name[FX_MAX_LONG_NAME_LEN-3] = '2';

    /* Now try to allocate an entry with no more total clusters.  */
    temp =  ram_disk.fx_media_total_clusters;
    _fx_utility_fat_entry_read_error_request = 40064;
    ram_disk.fx_media_cluster_search_start =  ram_disk.fx_media_total_clusters + (FX_FAT_ENTRY_START-1);
    status =  fx_file_create(&ram_disk, name);
    _fx_utility_fat_entry_read_error_request =  0;    
    ram_disk.fx_media_total_clusters =  temp;
    return_if_fail( status == FX_NO_MORE_SPACE);

    /* Now try to allocate a new cluster with a FAT read error in fx_directory_free_search.  */
    temp =  ram_disk.fx_media_total_clusters;
    _fx_utility_fat_entry_read_error_request = 65;
    ram_disk.fx_media_cluster_search_start =  ram_disk.fx_media_total_clusters + (FX_FAT_ENTRY_START-1);
    status =  fx_file_create(&ram_disk, name);
    _fx_utility_fat_entry_read_error_request =  0;
    ram_disk.fx_media_total_clusters =  temp;
    return_if_fail( status == FX_IO_ERROR);

    /* Now try to allocate a new cluster with a logical sector read error in fx_directory_free_search.  */
    _fx_utility_logical_sector_read_error_request = 172;
    ram_disk.fx_media_cluster_search_start =  ram_disk.fx_media_total_clusters + (FX_FAT_ENTRY_START-1);
    status =  fx_file_create(&ram_disk, name);
    _fx_utility_logical_sector_read_error_request = 0;
    return_if_fail( status == FX_IO_ERROR);

#ifndef FX_DISABLE_CACHE
    /* Now try to allocate a new cluster with a logical sector flush error in fx_directory_free_search.  */
    _fx_utility_logical_sector_flush_error_request = 1;
    ram_disk.fx_media_cluster_search_start =  ram_disk.fx_media_total_clusters + (FX_FAT_ENTRY_START-1);
    status =  fx_file_create(&ram_disk, name);
    _fx_utility_logical_sector_flush_error_request = 0;
    return_if_fail( status == FX_IO_ERROR);
#endif

    /* Now try to allocate a new cluster with a write error when clearing additional sector clusters from fx_directory_free_search.  */
    _fx_ram_driver_io_error_request = 91;
    ram_disk.fx_media_cluster_search_start =  ram_disk.fx_media_total_clusters + (FX_FAT_ENTRY_START-1);
    status =  fx_file_create(&ram_disk, name);
    _fx_ram_driver_io_error_request = 0;
    return_if_fail( status == FX_IO_ERROR);

    /* Now try to allocate a new cluster with a write error when writing the FAT link from fx_directory_free_search.  */
    _fx_utility_fat_entry_write_error_request = 1;
    ram_disk.fx_media_cluster_search_start =  ram_disk.fx_media_total_clusters + (FX_FAT_ENTRY_START-1);
    status =  fx_file_create(&ram_disk, name);
    _fx_utility_fat_entry_write_error_request = 0;
    return_if_fail( status == FX_IO_ERROR);

    /* Now try to allocate a new cluster with a write error when writing the EOF FAT link from fx_directory_free_search.  */
    _fx_utility_fat_entry_write_error_request = 3;
    ram_disk.fx_media_cluster_search_start =  ram_disk.fx_media_total_clusters + (FX_FAT_ENTRY_START-1);
    status =  fx_file_create(&ram_disk, name);
    _fx_utility_fat_entry_write_error_request = 0;
    return_if_fail( status == FX_IO_ERROR);

    /* Now try to allocate a new cluster with a logical sector write error when writing the new sub-directory sector in fx_directory_free_search.  */
    _fx_utility_logical_sector_write_error_request = 1;
    ram_disk.fx_media_cluster_search_start =  ram_disk.fx_media_total_clusters + (FX_FAT_ENTRY_START-1);
    status =  fx_file_create(&ram_disk, name);
    _fx_utility_logical_sector_write_error_request = 0;
    return_if_fail( status == FX_IO_ERROR);

    /* Now try to allocate a new cluster with a total cluster of 0 in fx_directory_free_search.  */
    _fx_utility_fat_entry_read_error_request = 92;
    ram_disk.fx_media_cluster_search_start =  ram_disk.fx_media_total_clusters + (FX_FAT_ENTRY_START-1);
    status =  fx_file_create(&ram_disk, name);
    _fx_utility_fat_entry_read_error_request = 0;
    return_if_fail( status == FX_IO_ERROR);

    /* Now try to allocate a new cluster with a bad read value (1) of intial cluster in sub-directory in fx_directory_free_search.  */
    _fx_utility_fat_entry_read_error_request = 10095;
    ram_disk.fx_media_cluster_search_start =  ram_disk.fx_media_total_clusters + (FX_FAT_ENTRY_START-1);
    status =  fx_file_create(&ram_disk, name);
    _fx_utility_fat_entry_read_error_request = 0;
    return_if_fail( status == FX_FAT_READ_ERROR);

    /* Now try to allocate a new cluster with a bad read value (same as initial cluster) of intial cluster in sub-directory in fx_directory_free_search.  */
    _fx_utility_fat_entry_read_error_request = 30098;
    ram_disk.fx_media_cluster_search_start =  ram_disk.fx_media_total_clusters + (FX_FAT_ENTRY_START-1);
    status =  fx_file_create(&ram_disk, name);
    _fx_utility_fat_entry_read_error_request = 0;
    return_if_fail( status == FX_FAT_READ_ERROR);

    /* Now try to allocate a new cluster with a bad read value (greater than max) of intial cluster in sub-directory in fx_directory_free_search.  */
    temp =  ram_disk.fx_media_total_clusters;
    _fx_utility_fat_entry_read_error_request = 40101;
    ram_disk.fx_media_cluster_search_start =  ram_disk.fx_media_total_clusters + (FX_FAT_ENTRY_START-1);
    status =  fx_file_create(&ram_disk, name);
    _fx_utility_fat_entry_read_error_request = 0;
    ram_disk.fx_media_total_clusters =  temp;
    return_if_fail( status == FX_FAT_READ_ERROR);

    /* Now try to allocate a new cluster with an error linking the new cluster chain with the old one in sub-directory in fx_directory_free_search.  */
    _fx_utility_fat_entry_write_error_request = 4;
    ram_disk.fx_media_cluster_search_start =  ram_disk.fx_media_total_clusters + (FX_FAT_ENTRY_START-1);
    status =  fx_file_create(&ram_disk, name);
    _fx_utility_fat_entry_write_error_request = 0;
    return_if_fail( status == FX_IO_ERROR);

    /* Now write to each file before we close them.  */
    status = fx_file_write(&file_1, "1", 1);
    status += fx_file_write(&file_2, "1", 1);
    status += fx_file_write(&file_3, "1", 1);
    status += fx_file_write(&file_4, "1", 1);
    status += fx_file_write(&file_5, "1", 1);
    status += fx_file_write(&file_6, "1", 1);
    status += fx_file_write(&file_7, "1", 1);
    status += fx_file_write(&file_8, "1", 1);
    status += fx_file_write(&file_9, "1", 1);
    status += fx_file_write(&file_10, "1", 1);
    status += fx_file_write(&file_11, "1", 1);
    status += fx_file_write(&file_12, "1", 1);
    status += fx_file_write(&file_13, "1", 1);
    status += fx_file_write(&file_14, "1", 1);
    status += fx_file_write(&file_15, "1", 1);
    status += fx_file_write(&file_16, "1", 1);
    status += fx_file_write(&file_17, "1", 1);
    status += fx_file_write(&file_18, "1", 1);
    status += fx_file_write(&file_19, "1", 1);
    status += fx_file_write(&file_20, "1", 1);
    status += fx_file_write(&file_21, "1", 1);
    status += fx_file_write(&file_22, "1", 1);
    status += fx_file_write(&file_23, "1", 1);
    status += fx_file_write(&file_24, "1", 1);
    status += fx_file_write(&file_25, "1", 1);
    status += fx_file_write(&file_26, "1", 1);
    status += fx_file_write(&file_27, "1", 1);
    status += fx_file_write(&file_28, "1", 1);
    status += fx_file_write(&file_29, "1", 1);
    status += fx_file_write(&file_30, "1", 1);
    status += fx_file_write(&file_31, "1", 1);
    status += fx_file_write(&file_32, "1", 1);
    status += fx_file_write(&file_33, "1", 1);
    status += fx_file_write(&file_34, "1", 1);
    status += fx_file_write(&file_35, "1", 1);
    status += fx_file_write(&file_36, "1", 1);
    status += fx_file_write(&file_37, "1", 1);
    status += fx_file_write(&file_38, "1", 1);
    status += fx_file_write(&file_39, "1", 1);
    status += fx_file_write(&file_40, "1", 1);
    status += fx_file_write(&file_41, "1", 1);
    status += fx_file_write(&file_42, "1", 1);
    status += fx_file_write(&file_43, "1", 1);
    status += fx_file_write(&file_44, "1", 1);
    status += fx_file_close(&file_1);
    status += fx_file_close(&file_2);
    status += fx_file_close(&file_3);
    status += fx_file_close(&file_4);
    status += fx_file_close(&file_5);
    status += fx_file_close(&file_6);
    status += fx_file_close(&file_7);
    status += fx_file_close(&file_8);
    status += fx_file_close(&file_9);
    status += fx_file_close(&file_10);
    status += fx_file_close(&file_11);
    status += fx_file_close(&file_12);
    status += fx_file_close(&file_13);
    status += fx_file_close(&file_14);
    status += fx_file_close(&file_15);
    status += fx_file_close(&file_16);
    status += fx_file_close(&file_17);
    status += fx_file_close(&file_18);
    status += fx_file_close(&file_19);
    status += fx_file_close(&file_20);
    status += fx_file_close(&file_21);
    status += fx_file_close(&file_22);
    status += fx_file_close(&file_23);
    status += fx_file_close(&file_24);
    status += fx_file_close(&file_25);
    status += fx_file_close(&file_26);
    status += fx_file_close(&file_27);
    status += fx_file_close(&file_28);
    status += fx_file_close(&file_29);
    status += fx_file_close(&file_30);
    status += fx_file_close(&file_31);
    status += fx_file_close(&file_32);
    status += fx_file_close(&file_33);
    status += fx_file_close(&file_34);
    status += fx_file_close(&file_35);
    status += fx_file_close(&file_36);
    status += fx_file_close(&file_37);
    status += fx_file_close(&file_38);
    status += fx_file_close(&file_39);
    status += fx_file_close(&file_40);
    status += fx_file_close(&file_41);
    status += fx_file_close(&file_42);
    status += fx_file_close(&file_43);
    status += fx_file_close(&file_44);
    return_if_fail( status == FX_SUCCESS);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);
    
    /* Set the flag to copy the default format to get NT file name, orphan long file name, and super long file names.  */
    _fx_ram_driver_copy_default_format =  1;

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    _fx_ram_driver_copy_default_format =  0;
    return_if_fail( status == FX_SUCCESS);

    /* Get the first file name, this should be "ntfile" for exercising the NT file name capability.  */
    status =  fx_directory_first_entry_find(&ram_disk, name);
    status += fx_file_open(&ram_disk, &file_1, name, FX_OPEN_FOR_WRITE);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_file_open(&ram_disk, &file_2, name, FX_OPEN_FOR_WRITE);
    status += fx_directory_next_entry_find(&ram_disk, name);
    
    /* Write to both files, write to them and close them!  */
    status += fx_file_write(&file_1, "abcd", 4);
    status += fx_file_write(&file_2, "abcd", 4);
    status += fx_file_close(&file_1);
    status += fx_file_close(&file_2);
    return_if_fail( status == FX_SUCCESS);
    
    /* Now look move to the first sub-directory.  */
    status =  fx_directory_default_set(&ram_disk, name);
    status +=  fx_directory_first_entry_find(&ram_disk, name);  /* .  */
    status +=  fx_directory_next_entry_find(&ram_disk, name);   /* ..  */
    status +=  fx_directory_next_entry_find(&ram_disk, name);   /* Too long of a file name.  */
    status +=  fx_file_delete(&ram_disk, name);
    status +=  fx_directory_next_entry_find(&ram_disk, name);   /* Next too long sub-directory.  */
    status +=  fx_directory_default_set(&ram_disk, name);       /* Move to next sub-directory. */
    return_if_fail( status == FX_SUCCESS);

    /* Move back to the root directory.  */
    status += fx_directory_default_set(&ram_disk, "\\");

    /* Set an all blank volume name.   */
    status += fx_media_volume_set(&ram_disk, "           ");
    status += fx_media_volume_get(&ram_disk, volume_name, FX_DIRECTORY_SECTOR);
    status += fx_directory_first_entry_find(&ram_disk, name);  /* ntfile  */
    status += fx_directory_next_entry_find(&ram_disk, name);   /* nt-file~.txt */
    status += fx_directory_next_entry_find(&ram_disk, name);   
    status += fx_directory_next_entry_find(&ram_disk, name);   
    status += fx_directory_next_entry_find(&ram_disk, name);   
    status += fx_directory_next_entry_find(&ram_disk, name);   
    status += fx_directory_next_entry_find(&ram_disk, name);   
    status += fx_directory_next_entry_find(&ram_disk, name);   

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);
    
    /* Set the flag to copy the default format to get NT file name, orphan long file name, and super long file names.  */
    _fx_ram_driver_copy_default_format =  1;

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    _fx_ram_driver_copy_default_format =  0;
    return_if_fail( status == FX_SUCCESS);

    /* Get the first shorted file name.  */
    status =  fx_directory_first_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_file_open(&ram_disk, &file_2, name, FX_OPEN_FOR_WRITE);
    
    /* Write to the shorted file.  */
    status += fx_file_write(&file_2, "abcd", 4);

    /* Now close the file, which will write the shorted file entry out.  */
    _fx_utility_logical_sector_write_error_request =  1;
    status += fx_file_close(&file_2);
    _fx_utility_logical_sector_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Set the flag to copy the default format to get NT file name, orphan long file name, and super long file names.  */
    _fx_ram_driver_copy_default_format =  1;

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    _fx_ram_driver_copy_default_format =  0;
    return_if_fail( status == FX_SUCCESS);

    /* Get the first shorted file name.  */
    status =  fx_directory_first_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_file_open(&ram_disk, &file_2, name, FX_OPEN_FOR_WRITE);
    
    /* Write to the shorted file.  */
    status += fx_file_write(&file_2, "abcd", 4);

    /* Now close the file, which will write the shorted file entry out.  */
    temp =  ram_disk.fx_media_data_sector_start;
    ram_disk.fx_media_data_sector_start =  ram_disk.fx_media_root_sector_start+2;
    status += fx_file_close(&file_2);
    ram_disk.fx_media_data_sector_start =  temp;
    return_if_fail( status == FX_FILE_CORRUPT);
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Set the flag to copy the default format to get NT file name, orphan long file name, and super long file names.  */
    _fx_ram_driver_copy_default_format =  1;

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    _fx_ram_driver_copy_default_format =  0;
    return_if_fail( status == FX_SUCCESS);

    /* Get the first shorted file name.  */
    status =  fx_directory_first_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_file_open(&ram_disk, &file_2, name, FX_OPEN_FOR_WRITE);
    status += fx_directory_next_entry_find(&ram_disk, name);

    /* Write to the shorted file.  */
    status += fx_file_write(&file_2, "abcd", 4);

    /* Now close the file, which will write the shorted file entry out.  */
    _fx_utility_logical_sector_read_error_request =  2;
    status += fx_file_close(&file_2);
    _fx_utility_logical_sector_read_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Set the flag to copy the default format to get NT file name, orphan long file name, and super long file names.  */
    _fx_ram_driver_copy_default_format =  1;

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    _fx_ram_driver_copy_default_format =  0;
    return_if_fail( status == FX_SUCCESS);

    /* Get the first shorted file name.  */
    status =  fx_directory_first_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_file_open(&ram_disk, &file_2, name, FX_OPEN_FOR_WRITE);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_media_flush(&ram_disk);
    status += fx_media_read(&ram_disk, ram_disk.fx_media_root_sector_start+1, buffer);
    buffer[0x40] =  0x1f;
    status += fx_media_write(&ram_disk, ram_disk.fx_media_root_sector_start+1, buffer);

    /* Write to the shorted file.  */
    status += fx_file_write(&file_2, "abcd", 4);

    /* Now close the file, which will write the shorted file entry out.... not that the shorted flag is not in the actual directory entry.  */
    status += fx_file_close(&file_2);
    return_if_fail( status == FX_SUCCESS);
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Set the flag to copy the default format to get NT file name, orphan long file name, and super long file names.  */
    _fx_ram_driver_copy_default_format =  1;

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    _fx_ram_driver_copy_default_format =  0;
    return_if_fail( status == FX_SUCCESS);

    /* Get the first shorted file name.  */
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);

    /* Switch to this directory.  */
    status += fx_directory_default_set(&ram_disk, name);
    status =  fx_directory_first_entry_find(&ram_disk, name);    /* . */
    status += fx_directory_next_entry_find(&ram_disk, name);     /* .. */
    status += fx_directory_next_entry_find(&ram_disk, name);     /* shorted long file name!  */
    status += fx_file_open(&ram_disk, &file_2, name, FX_OPEN_FOR_WRITE);

    /* Write to the shorted file.  */
    status += fx_file_write(&file_2, "abcd", 4);

    /* Now close the file, which will write the shorted file entry out.  */
    _fx_utility_fat_entry_read_error_request =  1;
    status += fx_file_close(&file_2);
    _fx_utility_fat_entry_read_error_request =  0;

    /* Check the status - should be an error here.  */
    if (status != FX_IO_ERROR)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(97);
    }
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(98);
    }       


    /* Set the flag to copy the default format to get NT file name, orphan long file name, and super long file names.  */
    _fx_ram_driver_copy_default_format =  1;

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    _fx_ram_driver_copy_default_format =  0;

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(99);
    }

    /* Get the first shorted file name.  */
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);

    /* Switch to this directory.  */
    status += fx_directory_default_set(&ram_disk, name);
    status =  fx_directory_first_entry_find(&ram_disk, name);    /* . */
    status += fx_directory_next_entry_find(&ram_disk, name);     /* .. */
    status += fx_directory_next_entry_find(&ram_disk, name);     /* shorted long file name!  */
    status += fx_file_open(&ram_disk, &file_2, name, FX_OPEN_FOR_WRITE);

    /* Write to the shorted file.  */
    status += fx_file_write(&file_2, "abcd", 4);

    /* Now close the file, which will write the shorted file entry out.  */
    _fx_utility_fat_entry_read_error_request =  10001;
    status += fx_file_close(&file_2);
    _fx_utility_fat_entry_read_error_request =  0;

    /* Check the status - should be an error here.  */
    if (status != FX_FILE_CORRUPT)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(100);
    }
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(101);
    }       

    /* Set the flag to copy the default format to get NT file name, orphan long file name, and super long file names.  */
    _fx_ram_driver_copy_default_format =  1;

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    _fx_ram_driver_copy_default_format =  0;

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(102);
    }

    /* Get the first shorted file name.  */
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);

    /* Switch to this directory.  */
    status += fx_directory_default_set(&ram_disk, name);
    status =  fx_directory_first_entry_find(&ram_disk, name);    /* . */
    status += fx_directory_next_entry_find(&ram_disk, name);     /* .. */
    status += fx_directory_next_entry_find(&ram_disk, name);     /* shorted long file name!  */
    status += fx_file_open(&ram_disk, &file_2, name, FX_OPEN_FOR_WRITE);

    /* Write to the shorted file.  */
    status += fx_file_write(&file_2, "abcd", 4);

    /* Now close the file, which will write the shorted file entry out.  */
    _fx_utility_fat_entry_read_error_request =  20001;
    status += fx_file_close(&file_2);
    _fx_utility_fat_entry_read_error_request =  0;

    /* Check the status - should be an error here.  */
    if (status != FX_FILE_CORRUPT)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(103);
    }
    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(104);
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
                            14000,                  // Total sectors 
                            128,                    // Sector size   
                            2,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(105);
    }   
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(106);
    }

    /* Build a sub-directory structure to further test diretory entry read.  */
    status =  fx_directory_create(&ram_disk, "SUB1");
    status += fx_directory_default_set(&ram_disk, "SUB1");
    status += fx_file_create(&ram_disk, "S1_FL1.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL2.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL3.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL4.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL5.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL6.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL7.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL8.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL9.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL10.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL11.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL12.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL13.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL14.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL15.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL16.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL17.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL18.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL19.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL20.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL21.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL22.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL23.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL24.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL25.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL26.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL27.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL28.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL29.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL30.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL31.TXT");    
    status += fx_file_create(&ram_disk, "S1_FL32.TXT");    
    
    /* Now taverse the directory tree and open files.  */
    status += fx_directory_first_entry_find(&ram_disk, name);  
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    ram_disk.fx_media_default_path.fx_path_directory.fx_dir_entry_last_search_relative_cluster =  100;
    status += fx_directory_next_entry_find(&ram_disk, name);
    ram_disk.fx_media_default_path.fx_path_directory.fx_dir_entry_last_search_log_sector =  1000;
    status += fx_directory_next_entry_find(&ram_disk, name);
    ram_disk.fx_media_default_path.fx_path_directory.fx_dir_entry_last_search_byte_offset =  10000;
    status += fx_directory_next_entry_find(&ram_disk, name);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(107);
    }       

    /* Now go back to the beginning of the directory.  */
    status += fx_directory_first_entry_find(&ram_disk, name);  
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    
    /* Search with I/O read error.  */
    ram_disk.fx_media_default_path.fx_path_directory.fx_dir_entry_last_search_cluster =  0;  /* Cause cluster traversal.  */
    _fx_utility_fat_entry_read_error_request =  1;
    status += fx_directory_next_entry_find(&ram_disk, name);
    _fx_utility_fat_entry_read_error_request =  0;
    
    /* Determine if the expected I/O error is present.  */
    if (status != FX_IO_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(108);
    }       
    
    /* Search with bad FAT error  - too small.  */
    ram_disk.fx_media_default_path.fx_path_directory.fx_dir_entry_last_search_cluster =  0;  /* Cause cluster traversal.  */
    _fx_utility_fat_entry_read_error_request =  10001;
    status = fx_directory_next_entry_find(&ram_disk, name);
    _fx_utility_fat_entry_read_error_request =  0;
    
    /* Determine if the expected I/O error is present.  */
    if (status != FX_FILE_CORRUPT)
    {

        printf("ERROR!\n");
        test_control_return(109);
    }       
    
    /* Search with bad FAT error  - too small.  */
    ram_disk.fx_media_default_path.fx_path_directory.fx_dir_entry_last_search_cluster =  0;  /* Cause cluster traversal.  */
    _fx_utility_fat_entry_read_error_request =  20001;
    status = fx_directory_next_entry_find(&ram_disk, name);
    _fx_utility_fat_entry_read_error_request =  0;
    
    /* Determine if the expected I/O error is present.  */
    if (status != FX_FILE_CORRUPT)
    {

        printf("ERROR!\n");
        test_control_return(110);
    }         

    /* Corrupt the media to test zero divisor checking.  */
    ram_disk.fx_media_bytes_per_sector = 0;
    status = fx_directory_next_entry_find(&ram_disk, name);

    /* Determine if the expected error is present.  */
    if (status != FX_MEDIA_INVALID)
    {

        printf("ERROR!\n");
        test_control_return(110);
    }
    ram_disk.fx_media_bytes_per_sector = 128;

    /* Build a large file name.  */
    for (i = 0; i < 255; i++)
    {
        name[i] =  'a';
    }
    name[255] =  0;

    /* Move back to the root directory.  */
    status =  fx_directory_default_set(&ram_disk, "\\");
    status += fx_unicode_file_create(&ram_disk, my_unicode_name, 15, (CHAR *)buffer);
    status += fx_unicode_file_create(&ram_disk, my_unicode_name1, 15, (CHAR *)buffer);
    status += fx_file_create(&ram_disk, name);
    
    /* Set the search to the beginning of the root directory.  */
    status += fx_directory_first_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(111);
    }       
    
    /* Now generate an error on the root directory size.  */
    temp =  ram_disk.fx_media_root_sectors;
    ram_disk.fx_media_root_sectors  =  1;
    status = fx_directory_next_entry_find(&ram_disk, name);
    ram_disk.fx_media_root_sectors =  temp;
    
    /* Determine if the expected I/O error is present.  */
    if (status != FX_FILE_CORRUPT)
    {

        printf("ERROR!\n");
        test_control_return(112);
    }         

    /* Now generate an error with a logical sector read error.  */
    _fx_utility_logical_sector_read_error_request =  5;
    status = fx_directory_next_entry_find(&ram_disk, name);
    _fx_utility_logical_sector_read_error_request =  0;
      
    /* Determine if the expected I/O error is present.  */
    if (status != FX_IO_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(113);
    }         

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(114);
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
                            14000,                  // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(115);
    }   
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(116);
    }

    /* Build a large file name.  */
    for (i = 0; i < 255; i++)
    {
        name[i] =  'a';
    }
    name[255] =  0;

    /* Create a sub-directory.  */
    status =  fx_directory_create(&ram_disk, "SUB1");
    status += fx_directory_default_set(&ram_disk, "SUB1");
    status += fx_file_create(&ram_disk, name);
    status += fx_directory_first_entry_find(&ram_disk, (CHAR *)buffer);
    status += fx_directory_next_entry_find(&ram_disk, (CHAR *)buffer);
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(117);
    }
    
    /* Read the first entry, but with a FAT entry error.  */
    _fx_utility_fat_entry_read_error_request =  1;
    status = fx_directory_next_entry_find(&ram_disk, (CHAR *)buffer);
    _fx_utility_fat_entry_read_error_request =  0;
    
    /* Determine if the expected I/O error is present.  */
    if (status != FX_IO_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(118);
    }         

    /* Read the first entry, but with a FAT entry error - too small.  */
    _fx_utility_fat_entry_read_error_request =  10001;
    status =  fx_directory_next_entry_find(&ram_disk, (CHAR *)buffer);
    _fx_utility_fat_entry_read_error_request =  0;
    
    /* Determine if the expected I/O error is present.  */
    if (status != FX_FILE_CORRUPT)
    {

        printf("ERROR!\n");
        test_control_return(119);
    }         
    
    /* Read the first entry, but with a FAT entry error - too big.  */
    _fx_utility_fat_entry_read_error_request =  20001;
    status =  fx_directory_next_entry_find(&ram_disk, (CHAR *)buffer);
    _fx_utility_fat_entry_read_error_request =  0;
    
    /* Determine if the expected I/O error is present.  */
    if (status != FX_FILE_CORRUPT)
    {

        printf("ERROR!\n");
        test_control_return(120);
    }         

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(121);
    }       

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            512,                    // Directory Entries
                            0,                      // Hidden sectors
                            14000,                  // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(122);
    }   
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(123);
    }

    /* Build a large file name.  */
    for (i = 0; i < 255; i++)
    {
        name[i] =  'a';
    }
    name[255] =  0;

    /* Create a large file name in the root directory.  */
    _fx_utility_logical_sector_write_error_request =  22;
    status =  fx_file_create(&ram_disk, name);
    _fx_utility_logical_sector_write_error_request =  0;
    
    /* Setup an I/O error on the first logical sectory write of the directory entry.  */
    status += fx_media_flush(&ram_disk);
    
    /* Determine if have the expected I/O error.  */
    if (status != FX_IO_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(124);
    }       

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(125);
    }       


    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            512,                    // Directory Entries
                            0,                      // Hidden sectors
                            14000,                  // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(126);
    }   
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(127);
    }

    /* Build a large file name.  */
    for (i = 0; i < 255; i++)
    {
        name[i] =  'a';
    }
    name[255] =  0;

    /* Create a large file name in the root directory.  */
    _fx_utility_logical_sector_read_error_request =  45;
    status =  fx_file_create(&ram_disk, name);
    _fx_utility_logical_sector_read_error_request =  0;
    
    /* Setup an I/O error on the first logical sectory write of the directory entry.  */
    status += fx_media_flush(&ram_disk);
    
    /* Determine if have the expected I/O error.  */
    if (status != FX_IO_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(128);
    }       

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(129);
    }       

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            512,                    // Directory Entries
                            0,                      // Hidden sectors
                            14000,                  // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(130);
    }   
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(131);
    }

    /* Build a large file name.  */
    for (i = 0; i < 255; i++)
    {
        name[i] =  'a';
    }
    name[255] =  0;

    /* Create a large file name in the root directory.  */
    temp =  ram_disk.fx_media_data_sector_start;
    ram_disk.fx_media_data_sector_start =  ram_disk.fx_media_root_sector_start+1;
    status =  fx_file_create(&ram_disk, name);
    ram_disk.fx_media_data_sector_start =  temp;
    
    /* Setup an I/O error on the first logical sectory write of the directory entry.  */
    status += fx_media_flush(&ram_disk);
    
    /* Determine if have the expected error.  */
    if (status != FX_FILE_CORRUPT)
    {

        printf("ERROR!\n");
        test_control_return(132);
    }       

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(133);
    }       

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            512,                    // Directory Entries
                            0,                      // Hidden sectors
                            14000,                  // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(134);
    }   
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(135);
    }

    /* Create a sub-directory so we can generate errors writing to the sub-directory file entry.  */
    status =  fx_directory_create(&ram_disk, "SUB1");
    status += fx_directory_default_set(&ram_disk, "SUB1");

    /* Setup an I/O error on the first logical sectory write of the directory entry.  */
    status += fx_media_flush(&ram_disk);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(136);
    }

    /* Build a large file name.  */
    for (i = 0; i < 255; i++)
    {
        name[i] =  'a';
    }
    name[255] =  0;

    /* Create a large file name in the sub-directory with a FAT entry read error.  */
    _fx_utility_fat_entry_read_error_request =  14;
    status =  fx_file_create(&ram_disk, name);
    _fx_utility_fat_entry_read_error_request =  0;
    
    /* Setup an I/O error on the first logical sectory write of the directory entry.  */
    status += fx_media_flush(&ram_disk);
    
    /* Determine if have the expected error.  */
    if (status != FX_IO_ERROR)
    {

        printf("ERROR!\n");
        test_control_return(137);
    }       

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(138);
    }       

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            512,                    // Directory Entries
                            0,                      // Hidden sectors
                            14000,                  // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(139);
    }   
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(140);
    }

    /* Create a sub-directory so we can generate errors writing to the sub-directory file entry.  */
    status =  fx_directory_create(&ram_disk, "SUB1");
    status += fx_directory_default_set(&ram_disk, "SUB1");

    /* Setup an I/O error on the first logical sectory write of the directory entry.  */
    status += fx_media_flush(&ram_disk);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(141);
    }

    /* Build a large file name.  */
    for (i = 0; i < 255; i++)
    {
        name[i] =  'a';
    }
    name[255] =  0;

    /* Create a large file name in the sub-directory with a FAT entry read error.  */
    _fx_utility_fat_entry_read_error_request =  10014;
    status =  fx_file_create(&ram_disk, name);
    _fx_utility_fat_entry_read_error_request =  0;
    
    /* Setup an I/O error on the first logical sectory write of the directory entry.  */
    status += fx_media_flush(&ram_disk);
    
    /* Determine if have the expected error.  */
    if (status != FX_FILE_CORRUPT)
    {

        printf("ERROR!\n");
        test_control_return(142);
    }       

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(143);
    }       

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            512,                    // Directory Entries
                            0,                      // Hidden sectors
                            14000,                  // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(144);
    }   
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(145);
    }

    /* Create a sub-directory so we can generate errors writing to the sub-directory file entry.  */
    status =  fx_directory_create(&ram_disk, "SUB1");
    status += fx_directory_default_set(&ram_disk, "SUB1");

    /* Setup an I/O error on the first logical sectory write of the directory entry.  */
    status += fx_media_flush(&ram_disk);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(146);
    }

    /* Build a large file name.  */
    for (i = 0; i < 255; i++)
    {
        name[i] =  'a';
    }
    name[255] =  0;

    /* Create a large file name in the sub-directory with a FAT entry read error.  */
    _fx_utility_fat_entry_read_error_request =  20014;
    status =  fx_file_create(&ram_disk, name);
    _fx_utility_fat_entry_read_error_request =  0;
    
    /* Setup an I/O error on the first logical sectory write of the directory entry.  */
    status += fx_media_flush(&ram_disk);
    
    /* Determine if have the expected error.  */
    if (status != FX_FILE_CORRUPT)
    {

        printf("ERROR!\n");
        test_control_return(147);
    }       

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(148);
    }       

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            10000,                  // Directory Entries
                            0,                      // Hidden sectors
                            64000,                  // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(149);
    }   
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(150);
    }
    
    /* Create a file name that execises the short name with a 0xE5 and a dot less than the eighth character.  */
    status =  fx_file_create(&ram_disk, "a.a");
    status += fx_media_flush(&ram_disk);
    status += fx_media_read(&ram_disk, ram_disk.fx_media_root_sector_start, buffer);
    buffer[0x20] =  0x8f;
    buffer[0x21] =  0xe5;
    buffer[0x22] =  '.';
    buffer[0x23] =  'a';
    buffer[0x24] =  '~';
    status += fx_media_write(&ram_disk, ram_disk.fx_media_root_sector_start, buffer);
    status += fx_media_flush(&ram_disk);
    status += fx_directory_short_name_get(&ram_disk, "a.a", name);
    status += fx_file_open(&ram_disk, &file_1, "a.a", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&file_1, "a", 1);
    file_1.fx_file_dir_entry.fx_dir_entry_short_name[0] = (CHAR)0xE5;
    status += fx_media_flush(&ram_disk);
    status += fx_file_close(&file_1);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(151);
    }  

    /* Create some files to test the logic in fx_directory_entry_read  */
    status =   fx_file_create(&ram_disk, "a");
    status +=  fx_directory_short_name_get(&ram_disk, "a", name);
    status +=  fx_file_create(&ram_disk, "aa");
    status +=  fx_directory_short_name_get(&ram_disk, "aa", name);
    status +=  fx_file_create(&ram_disk, "aaa");
    status +=  fx_directory_short_name_get(&ram_disk, "aaa", name);
    status +=  fx_file_create(&ram_disk, "aaaa");
    status +=  fx_directory_short_name_get(&ram_disk, "aaaa", name);
    status +=  fx_file_create(&ram_disk, "aaaaa");
    status +=  fx_directory_short_name_get(&ram_disk, "aaaaa", name);
    status +=  fx_file_create(&ram_disk, "aaaaaa");
    status +=  fx_directory_short_name_get(&ram_disk, "aaaaaa", name);
    status +=  fx_file_create(&ram_disk, "aaaaaaa");
    status +=  fx_directory_short_name_get(&ram_disk, "aaaaaaa", name);
    status +=  fx_file_create(&ram_disk, "aaaaaaaa");
    status +=  fx_directory_short_name_get(&ram_disk, "aaaaaaaa", name);
    status +=  fx_file_create(&ram_disk, "aaaaaaaa.a");
    status +=  fx_directory_short_name_get(&ram_disk, "aaaaaaaa.a", name);
    status +=  fx_file_create(&ram_disk, "aaaaaaaa.aa");
    status +=  fx_directory_short_name_get(&ram_disk, "aaaaaaaa.aa", name);
    status +=  fx_file_create(&ram_disk, "aaaaaaaa.aaa");
    status +=  fx_directory_short_name_get(&ram_disk, "aaaaaaaa.aaa", name);
    status +=  fx_file_create(&ram_disk, "a.");
    status +=  fx_directory_short_name_get(&ram_disk, "a.", name);
    status +=  fx_file_create(&ram_disk, "aa.");
    status +=  fx_directory_short_name_get(&ram_disk, "aa.", name);
    status +=  fx_file_create(&ram_disk, "aaa.");
    status +=  fx_directory_short_name_get(&ram_disk, "aaa.", name);
    status +=  fx_file_create(&ram_disk, "aaaa.");
    status +=  fx_directory_short_name_get(&ram_disk, "aaaa.", name);
    status +=  fx_file_create(&ram_disk, "aaaaa.");
    status +=  fx_directory_short_name_get(&ram_disk, "aaaaa.", name);
    status +=  fx_file_create(&ram_disk, "aaaaaa.");
    status +=  fx_directory_short_name_get(&ram_disk, "aaaaaa.", name);
    status +=  fx_file_create(&ram_disk, "aaaaaaa.");
    status +=  fx_directory_short_name_get(&ram_disk, "aaaaaaa.", name);
    status +=  fx_file_create(&ram_disk, "aaaaaaaa.");
    status +=  fx_directory_short_name_get(&ram_disk, "aaaaaaaa.", name);
    status +=  fx_file_delete(&ram_disk, "aaaaaa.");
    status +=  fx_file_delete(&ram_disk, "aaaaaaa.");
    status +=  fx_file_delete(&ram_disk, "aaaaaaaa.");
    
    /* Create a name with an 0xe5 in the front.  */
    name[0] =  (CHAR)0xE5;
    name[1] =  'a';
    name[2] =  'b';
    name[3] =  '.';
    name[4] =  't';
    name[5] =  'x';
    name[6] =  't';
    name[7] =  0;
    status += fx_file_create(&ram_disk, name);
    status += fx_directory_short_name_get(&ram_disk, name, (CHAR *)buffer);
    status += fx_file_open(&ram_disk, &file_1, name, FX_OPEN_FOR_WRITE);
    status += fx_file_write(&file_1, "1", 1);
    status += fx_file_close(&file_1);

    status += fx_file_create(&ram_disk, "abcdefgh.txt");
    status += fx_directory_short_name_get(&ram_disk, "abcdefgh.txt", (CHAR *)buffer);
    status += fx_file_open(&ram_disk, &file_1, "abcdefgh.txt", FX_OPEN_FOR_WRITE);
    status += fx_file_write(&file_1, "1", 1);
    status += fx_file_close(&file_1);

#if EXTENDED_NAME_MANGLE_TEST    
    name[0] = 'a';
    name[1] = 'B';
    name[2] = 'C';
    name[3] = 'D';
    name[4] = 'E';
    name[5] = 'F';
    name[6] = 'G';
    name[7] = '0';
    name[8] = '0';
    name[9] = '0';
    name[10] = '0';
    name[11] = 0;
    for (i = 0; i < 3; i++)
    {

        name[7] = '0' + i;

        for (j = 0; j < 10; j++)
        {
    
            name[8] =  '0' + j;
    
            for (k = 0; k < 10; k++)
            {

                name[9] =  '0' + k;
        
                for (l = 0; l < 10; l++)
                {
                    if (status)
                        break;
                    name[10] =  '0' + l;
                    status += fx_file_create(&ram_disk, name);
                    if (status)
                        break;
                    status += fx_directory_short_name_get(&ram_disk, name, buffer);
                    if (status)
                        break;
                }
            }
        }
    }
#endif
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(152);
    }

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(153);
    }       
    
    
    /* Test corner cases on fx_directory_search.  */
    
    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            256,                    // Directory Entries
                            0,                      // Hidden sectors
                            14000,                  // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(154);
    }   
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(155);
    }
    
    /* Build a long directory structure.  */
    status =  fx_directory_create(&ram_disk, "/sub-directory_depth_00001");
    status += fx_directory_create(&ram_disk, "sub-directory_depth_00001/sub-directory_depth_00002");
    status += fx_directory_create(&ram_disk, "sub-directory_depth_00001/sub-directory_depth_00002/sub-directory_depth_00003");
    status += fx_directory_create(&ram_disk, "sub-directory_depth_00001/sub-directory_depth_00002/sub-directory_depth_00003/sub-directory_depth_00004");
    status += fx_directory_create(&ram_disk, "sub-directory_depth_00001/sub-directory_depth_00002/sub-directory_depth_00003/sub-directory_depth_00004/sub-directory_depth_00005");
    status += fx_directory_create(&ram_disk, "sub-directory_depth_00001/sub-directory_depth_00002/sub-directory_depth_00003/sub-directory_depth_00004/sub-directory_depth_00005/sub-directory_depth_00006");
    status += fx_directory_create(&ram_disk, "sub-directory_depth_00001/sub-directory_depth_00002/sub-directory_depth_00003/sub-directory_depth_00004/sub-directory_depth_00005/sub-directory_depth_00006/sub-directory_depth_00007");
    status += fx_directory_create(&ram_disk, "sub-directory_depth_00001/sub-directory_depth_00002/sub-directory_depth_00003/sub-directory_depth_00004/sub-directory_depth_00005/sub-directory_depth_00006/sub-directory_depth_00007/sub-directory_depth_00008");
    status += fx_directory_create(&ram_disk, "sub-directory_depth_00001/sub-directory_depth_00002/sub-directory_depth_00003/sub-directory_depth_00004/sub-directory_depth_00005/sub-directory_depth_00006/sub-directory_depth_00007/sub-directory_depth_00008/sub-directory_depth_00009");
    status += fx_directory_create(&ram_disk, "sub-directory_depth_00001/sub-directory_depth_00002/sub-directory_depth_00003/sub-directory_depth_00004/sub-directory_depth_00005/sub-directory_depth_00006/sub-directory_depth_00007/sub-directory_depth_00008/sub-directory_depth_00009/sub-directory_depth_00010");
    status += fx_directory_create(&ram_disk, "sub-directory_depth_00001/sub-directory_depth_00002/sub-directory_depth_00003/sub-directory_depth_00004/sub-directory_depth_00005/sub-directory_depth_00006/sub-directory_depth_00007/sub-directory_depth_00008/sub-directory_depth_00009/sub-directory_depth_00010/sub-directory_depth_00011");
    status += fx_file_create(&ram_disk, "sub-directory_depth_00001/sub-directory_depth_00002/sub-directory_depth_00003/sub-directory_depth_00004/sub-directory_depth_00005/sub-directory_depth_00006/sub-directory_depth_00007/sub-directory_depth_00008/sub-directory_depth_00009/sub-directory_depth_00010/sub-directory_depth_00011/file1.txt");
    status += fx_file_open(&ram_disk, &file_1, "sub-directory_depth_00001/sub-directory_depth_00002/sub-directory_depth_00003/sub-directory_depth_00004/sub-directory_depth_00005/sub-directory_depth_00006/sub-directory_depth_00007/sub-directory_depth_00008/sub-directory_depth_00009/sub-directory_depth_00010/sub-directory_depth_00011/file1.txt", FX_OPEN_FOR_WRITE);

    /* Now create an invalid path by using a file as a directory entry.  */
    status += fx_file_create(&ram_disk, "sub-directory_depth_00001/sub-directory_depth_00002/sub-directory_depth_00003/sub-directory_depth_00004/sub-directory_depth_00005/sub-directory_depth_00006/sub-directory_depth_00007/sub-directory_depth_00008/sub-directory_depth_00009/sub-directory_depth_00010/sub-directory_depth_00011/file1.txt/baddir");

    /* Determine if the test was successful.  */
    if (status != FX_INVALID_PATH)
    {

        printf("ERROR!\n");
        test_control_return(156);
    }       
    
    status = fx_file_create(&ram_disk, "/sub-directory_depth_00001/sub-directory_depth_00002/sub-directory_depth_00003/sub-directory_depth_00004/sub-directory_depth_00005/sub-directory_depth_00006/sub-directory_depth_00007/sub-directory_depth_00008/sub-directory_depth_00009/sub-directory_depth_00010/sub-directory_depth_00011/file2.txt");
    status += fx_file_open(&ram_disk, &file_2, "/sub-directory_depth_00001/sub-directory_depth_00002/sub-directory_depth_00003/sub-directory_depth_00004/sub-directory_depth_00005/sub-directory_depth_00006/sub-directory_depth_00007/sub-directory_depth_00008/sub-directory_depth_00009/sub-directory_depth_00010/sub-directory_depth_00011/file2.txt", FX_OPEN_FOR_WRITE);
    status += fx_directory_default_set(&ram_disk, "sub-directory_depth_00001/sub-directory_depth_00002/sub-directory_depth_00003/sub-directory_depth_00004/sub-directory_depth_00005/sub-directory_depth_00006/sub-directory_depth_00007/sub-directory_depth_00008/sub-directory_depth_00009/sub-directory_depth_00010/sub-directory_depth_00011");
    status += fx_directory_first_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);
    status += fx_directory_next_entry_find(&ram_disk, name);

    /* Close the media.  */
    status +=  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(157);
    }       

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            256,                    // Directory Entries
                            0,                      // Hidden sectors
                            14000,                  // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(158);
    }   
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(159);
    }

    /* Build a long sub-directory name...  This name will be too long to cache in fx_directory_search.  */
    for (i = 0; i < FX_MAX_LONG_NAME_LEN; i++)
    {
        name[i] =  'a';
    }
    name[0] =  '/';
    name[256] = 0;
    
    status =  fx_directory_create(&ram_disk, name);
    
    name[256] = '/';
    name[257] = 'f';
    name[258] = 'i';
    name[259] = 'l';
    name[260] = 'e';
    name[261] = 0;
   
    status += fx_file_create(&ram_disk, name);   
    status += fx_file_open(&ram_disk, &file_1, name, FX_OPEN_FOR_WRITE);
    status += fx_directory_short_name_get(&ram_disk, name, (CHAR *)buffer);
    status += fx_directory_short_name_get(&ram_disk, name, (CHAR *)buffer);
    status += fx_directory_short_name_get(&ram_disk, name, (CHAR *)buffer);
    status += fx_directory_short_name_get(&ram_disk, name, (CHAR *)buffer);
    status += fx_directory_short_name_get(&ram_disk, name, (CHAR *)buffer);
    status += fx_file_close(&file_1);
    status += fx_file_open(&ram_disk, &file_1, &name[1], FX_OPEN_FOR_WRITE);
    status += fx_file_close(&file_1);
    status += fx_file_open(&ram_disk, &file_1, name, FX_OPEN_FOR_WRITE);
    status += fx_file_close(&file_1);
    status += fx_file_open(&ram_disk, &file_1, &name[1], FX_OPEN_FOR_WRITE);
    status += fx_file_close(&file_1);
    status += fx_file_delete(&ram_disk, name);   

    /* Build a perfect fit long sub-directory name and file name...  This name will fit exactly into the fx_directory_search cache.  */
    for (i = 0; i < FX_MAX_LONG_NAME_LEN; i++)
    {
        name[i] =  'a';
    }
    name[0] =  '/';
    name[250] = 0;
    
    status += fx_directory_create(&ram_disk, name);
    
    name[250] = '/';
    name[251] = 'f';
    name[252] = 'i';
    name[253] = 'l';
    name[254] = 'e';
    name[255] = 0;
   
    status =  fx_file_create(&ram_disk, name);   
    status += fx_file_open(&ram_disk, &file_1, name, FX_OPEN_FOR_WRITE);
    status += fx_directory_short_name_get(&ram_disk, name, (CHAR *)buffer);
    status += fx_directory_short_name_get(&ram_disk, name, (CHAR *)buffer);
    status += fx_directory_short_name_get(&ram_disk, name, (CHAR *)buffer);
    status += fx_directory_short_name_get(&ram_disk, name, (CHAR *)buffer);
    status += fx_directory_short_name_get(&ram_disk, name, (CHAR *)buffer);
    status += fx_file_close(&file_1);
    status += fx_file_open(&ram_disk, &file_1, &name[1], FX_OPEN_FOR_WRITE);
    status += fx_file_close(&file_1);
    status += fx_file_open(&ram_disk, &file_1, name, FX_OPEN_FOR_WRITE);
    status += fx_file_close(&file_1);
    status += fx_file_open(&ram_disk, &file_1, &name[1], FX_OPEN_FOR_WRITE);
    status += fx_file_close(&file_1);
    status =  fx_file_delete(&ram_disk, name);   

    
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(160);
    }       


    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            256,                    // Directory Entries
                            0,                      // Hidden sectors
                            14000,                  // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(161);
    }   
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(162);
    }


    /* Build a perfect fit long sub-directory name and file name...  This name will fit exactly into the fx_directory_search cache.  */
    for (i = 0; i < FX_MAX_LONG_NAME_LEN; i++)
    {
        name[i] =  'a';
    }
    name[0] =  '/';
    name[255] =  0;
    status += fx_directory_create(&ram_disk, name);
   
    
    name[253] = 0;
    
    status += fx_directory_create(&ram_disk, name);
    
    name[253] = '/';
    name[254] = 'a';
    name[255] = 0;
   
    status =  fx_file_create(&ram_disk, name);   
    status += fx_file_open(&ram_disk, &file_1, name, FX_OPEN_FOR_WRITE);
    status += fx_directory_short_name_get(&ram_disk, name, (CHAR *)buffer);
    status += fx_file_close(&file_1);

    /* Setup info for direct call to directory search.  */
    dir_entry.fx_dir_entry_name =  ram_disk.fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;
    search_directory.fx_dir_entry_name =  ram_disk.fx_media_name_buffer + FX_MAX_LONG_NAME_LEN * 2;
    dir_entry.fx_dir_entry_short_name[0] =        0;
    search_directory.fx_dir_entry_short_name[0] = 0;

    /* Lets make the name bigger than the maximun name.  */
    name[255] = 'a';
    name[256] = 'a';
    name[257] = 'a';
    name[258] = 'a';
    name[259] = 0;
    _fx_directory_search(&ram_disk, name, &dir_entry, &search_directory, &name_ptr);
    
    
    /* Set the name back to the original size.  */
    name[255] =  0;
    status += fx_file_open(&ram_disk, &file_1, name, FX_OPEN_FOR_WRITE);
    status += fx_directory_short_name_get(&ram_disk, name, (CHAR *)buffer);
    status += fx_file_close(&file_1);

    /* Now make the name long again to test relative, no path.  */
    name[255] =  'a';
    
    /* Now let's perform a maximum relative search.  */
    _fx_directory_search(&ram_disk, &name[1], &dir_entry, &search_directory, &name_ptr);
        

    /* Now lets set a path and test that out.  */
    name[253] =  0;
    status += fx_directory_default_set(&ram_disk, name);
    
    /* Now set the name back to the original size.  */
    name[253] =  '/';
    name[255] =  0;
    
    status += fx_file_open(&ram_disk, &file_1, name, FX_OPEN_FOR_WRITE);
    status += fx_directory_short_name_get(&ram_disk, name, (CHAR *)buffer);
    status += fx_file_close(&file_1);

    /* Now make the name long again to test relative, with path.  */
    name[255] =  'a';
    
    /* Now let's perform a maximum relative search.  */
    _fx_directory_search(&ram_disk, &name[1], &dir_entry, &search_directory, &name_ptr);

    /* Now lets set a path, modify the found path and test that out.  */
    name[253] =  'a';
    name[254] =  'a';
    name[255] =  0;
    status += fx_directory_default_set(&ram_disk, name);
    
    /* Now set the name back to the original size.  */
    name[253] =  '/';
    name[255] =  0;
    
    status += fx_file_open(&ram_disk, &file_1, name, FX_OPEN_FOR_WRITE);
    status += fx_directory_short_name_get(&ram_disk, name, (CHAR *)buffer);
    status += fx_file_close(&file_1);

    /* Test the maximum size of the default path lookup.  */
    for (i = 0; i < 256; i++)
    {
        ram_disk.fx_media_last_found_name[i] =  ram_disk.fx_media_default_path.fx_path_name_buffer[i];
    }
    ram_disk.fx_media_last_found_name[0] =  '/';
    name[255] =  'a';
    name[256] =  0;  

    /* Now let's perform a maximum relative search.  */
    _fx_directory_search(&ram_disk, &name[1], &dir_entry, &search_directory, &name_ptr);
                
    /* Now let's setup a directory search with a NULL path, a maximum found file name, and alternate directory separators.  */
    ram_disk.fx_media_default_path.fx_path_name_buffer[0] =  0;
    strcpy(&ram_disk.fx_media_last_found_name[0], "\\abc\\def\\ghi\\abc");
    for (i = 0; i < 256; i++)
    {
        ram_disk.fx_media_last_found_file_name[i] =  'a';
        ram_disk.fx_media_last_found_directory.fx_dir_entry_name[i] = 'a';
    }
    ram_disk.fx_media_last_found_file_name[255] =  0;    
    ram_disk.fx_media_last_found_directory.fx_dir_entry_name[255] = 0;
    ram_disk.fx_media_last_found_directory_valid =  FX_TRUE;
    _fx_directory_search(&ram_disk, "abc\\def\\ghi\\abc", &dir_entry, &search_directory, &name_ptr);

    /* Now let's setup a directory search with a NULL path, a medium sized found file name, and alternate directory separators.  */
    ram_disk.fx_media_default_path.fx_path_name_buffer[0] =  0;
    strcpy(&ram_disk.fx_media_last_found_name[0], "\\abc\\def\\ghi\\abc");
    for (i = 0; i < 256; i++)
    {
        ram_disk.fx_media_last_found_file_name[i] =  'a';
        ram_disk.fx_media_last_found_directory.fx_dir_entry_name[i] = 'a';
    }
    ram_disk.fx_media_last_found_file_name[255] =  0;    
    ram_disk.fx_media_last_found_directory.fx_dir_entry_name[25] = 0;
    ram_disk.fx_media_last_found_directory_valid =  FX_TRUE;
    _fx_directory_search(&ram_disk, "abc\\def\\ghi\\abc", &dir_entry, &search_directory, &name_ptr);
                
                
    /* Close the media.  */
    status =  fx_media_close(&ram_disk);

    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(163);
    }       


    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            256,                    // Directory Entries
                            0,                      // Hidden sectors
                            14000,                  // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Determine if the format had an error.  */
    if (status)
    {

        printf("ERROR!\n");
        test_control_return(164);
    }   
    
    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);

    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(165);
    }

    /* Create a directory path.  */
    status =  fx_directory_create(&ram_disk, "/abc");
    status += fx_directory_create(&ram_disk, "/abc/def");
    status += fx_directory_create(&ram_disk, "/abc/def/ghi");
    status += fx_directory_create(&ram_disk, "/abc/def/ghi/jkl");
    status += fx_file_create(&ram_disk, "/abc/def/ghi/jkl/file.txt");   
    status += fx_file_create(&ram_disk, "/abc/def/ghi/jkl/file2.txt");   
    status += fx_file_create(&ram_disk, "/abc/def/ghi/jkl/file3.txt");   
    status += fx_file_create(&ram_disk, "/abc/def/ghi/jkl/file4.txt");   
    status += fx_file_create(&ram_disk, "/abc/def/ghi/jkl/file5.txt");   
    status += fx_file_create(&ram_disk, "/abc/def/ghi/jkl/file6.txt");   
    status += fx_file_create(&ram_disk, "/abc/def/ghi/jkl/file7.txt");   
    status += fx_file_create(&ram_disk, "/abc/def/ghi/jkl/file8.txt");   
    status += fx_file_create(&ram_disk, "/abc/def/ghi/jkl/file9.txt");   
    status += fx_file_create(&ram_disk, "/abc/def/ghi/jkl/file10.txt");   
    status += fx_file_create(&ram_disk, "/abc/def/ghi/jkl/file11.txt");   
    status += fx_directory_default_set(&ram_disk, "/abc");
    status += fx_directory_first_entry_find(&ram_disk, name);
    
    /* Now open the file to get it into the search cache.  */
    status += fx_file_open(&ram_disk, &file_1, "/abc/def/ghi/jkl/file.txt", FX_OPEN_FOR_WRITE);
    status += fx_file_close(&file_1);
    status += fx_file_open(&ram_disk, &file_1, "/abc/def/ghi/jkl/file.txt", FX_OPEN_FOR_WRITE);
    status += fx_file_close(&file_1);
    status += fx_file_open(&ram_disk, &file_1, "def/ghi/jkl/file.txt", FX_OPEN_FOR_WRITE);
    status += fx_file_close(&file_1);
    status += fx_directory_default_set(&ram_disk, "/");
    status += fx_file_open(&ram_disk, &file_1, "abc/def/ghi/jkl/file.txt", FX_OPEN_FOR_WRITE);
    status += fx_file_close(&file_1);  
    status += fx_directory_next_entry_find(&ram_disk, name);

    /* Now test for FAT read errors.  */
    _fx_utility_fat_entry_read_error_request =  1;
    status += fx_file_open(&ram_disk, &file_2, "abc/def/ghi/jkl/file2.txt", FX_OPEN_FOR_WRITE);
    _fx_utility_fat_entry_read_error_request =  0;
    
    /* Check the status... should be an I/O error at this point.  */
    if (status != FX_IO_ERROR)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(166);
    }
    
    /* Now test a FAT entry value that is too small.  */
    _fx_utility_fat_entry_read_error_request =  10001;
    status = fx_file_open(&ram_disk, &file_2, "abc/def/ghi/jkl/file2.txt", FX_OPEN_FOR_WRITE);
    _fx_utility_fat_entry_read_error_request =  0;
    
    /* Check the status... should be a FAT read error.  */
    if (status != FX_FAT_READ_ERROR)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(167);
    }
    
    /* Now test a FAT entry value that is too large.  */
    temp =  ram_disk.fx_media_total_clusters;
    _fx_utility_fat_entry_read_error_request =  40001;
    status = fx_file_open(&ram_disk, &file_2, "abc/def/ghi/jkl/file2.txt", FX_OPEN_FOR_WRITE);
    _fx_utility_fat_entry_read_error_request =  0;
    ram_disk.fx_media_total_clusters =  temp;
    
    /* Check the status... should be a FAT read error.  */
    if (status != FX_FAT_READ_ERROR)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(168);
    }
    
    /* Now test a FAT entry value that is the same as the cluster itself.  */
    _fx_utility_fat_entry_read_error_request =  30001;
    status = fx_file_open(&ram_disk, &file_2, "abc/def/ghi/jkl/file2.txt", FX_OPEN_FOR_WRITE);
    _fx_utility_fat_entry_read_error_request =  0;
    
    /* Check the status... should be a FAT read error.  */
    if (status != FX_FAT_READ_ERROR)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(169);
    }
    
    /* Now perform a good open.  */
    status = fx_file_open(&ram_disk, &file_2, "abc/def/ghi/jkl/file2.txt", FX_OPEN_FOR_WRITE);
    status += fx_file_close(&file_2);  
    
    /* Check the status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, return error code.  */
        printf("ERROR!\n");
        test_control_return(166);
    }
    
    /* Setup info for direct call to directory search.  */
    dir_entry.fx_dir_entry_name =  ram_disk.fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;
    search_directory.fx_dir_entry_name =  ram_disk.fx_media_name_buffer + FX_MAX_LONG_NAME_LEN * 2;
    dir_entry.fx_dir_entry_short_name[0] =        0;
    search_directory.fx_dir_entry_short_name[0] = 0;

    /* Lets perform a search that from the root directory.  */
    _fx_directory_search(&ram_disk, "/abc/..", &dir_entry, &search_directory, &name_ptr);
    
    /* Now setup a maximum default path.  */
    for (i = 0; i < FX_MAX_LAST_NAME_LEN; i++)
    {
        name[i] =  'a';
    }
    name[255] =  0;
    
    /* Create a sub-directory of maximum lenght.  */
    status =  fx_directory_create(&ram_disk, name);
    
    /* Set the default path to this name.  */
    status += fx_directory_default_set(&ram_disk, name);
    
    /* Create a file in the sub-directory path.  */
    status += fx_file_create(&ram_disk, "file1.txt");
   
    /* Open the file to test the directory search cache name build.  */
    status += fx_file_open(&ram_disk, &file_1, "file1.txt", FX_OPEN_FOR_WRITE);
    
    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(167);
    }       
    

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(168);
    }

    /* Test for https://github.com/azure-rtos/filex/issues/26 */
    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            4,                      // Directory Entries
                            0,                      // Hidden sectors
                            128,                    // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_if_fail( status == FX_SUCCESS);

    FX_FILE my_file;
    ULONG available_clusters;
    uint8_t buff[128];
    int num_of_files = 0;
    char dummy_file_path[DIR_PATH_SIZE + 3];
    memcpy(dummy_file_path, DIR_PATH, DIR_PATH_SIZE);
    /* Create a file called TEST.TXT.*/

    status +=  fx_directory_create(&ram_disk, "/MYDIR");
    status +=  fx_directory_create(&ram_disk, "MYDIR/MYDIR1");
    status +=  fx_directory_create(&ram_disk, "MYDIR/MYDIR1/MYDIR2");
    status +=  fx_directory_create(&ram_disk, "MYDIR/MYDIR1/MYDIR2/MYDIR3");
    return_if_fail( status == FX_SUCCESS);
    
    status =  fx_file_create(&ram_disk, "MYDIR/MYDIR1/MYDIR2/MYDIR3/TEST.TXT");  
    return_if_fail( status == FX_SUCCESS);

    available_clusters = ram_disk.fx_media_available_clusters;
    /*
     * Ensure there is no spare space in the cluster used for the directory
     */
    dummy_file_path[sizeof(DIR_PATH) - 1] = 'A';
    dummy_file_path[sizeof(DIR_PATH)] = '\0';
    for (num_of_files = 0; available_clusters == ram_disk.fx_media_available_clusters; num_of_files ++)
    {
        dummy_file_path[sizeof(DIR_PATH) - 1] ++;
        status =  fx_file_create(&ram_disk, dummy_file_path);
        return_if_fail( status == FX_SUCCESS);
    }
    // remove the file that allocated new cluster
    status =  fx_file_delete(&ram_disk, dummy_file_path);
    return_if_fail( status == FX_SUCCESS);

    /* Open the test file.  */
    status =  fx_file_open(&ram_disk, &my_file, "MYDIR/MYDIR1/MYDIR2/MYDIR3/TEST.TXT", FX_OPEN_FOR_WRITE);
    return_if_fail( status == FX_SUCCESS);

    available_clusters = ram_disk.fx_media_available_clusters;
    for (int i = 0; i < available_clusters - 1; i++)
    {
        /* Write a one cluster size to the test file.  */
        status =  fx_file_write(&my_file, buff, sizeof(buff));
        return_if_fail( status == FX_SUCCESS);
    }

    /* The dir entry should be created on new cluster */
    status =fx_file_rename(&ram_disk, "MYDIR/MYDIR1/MYDIR2/MYDIR3/TEST.TXT", "MYDIR/MYDIR1/MYDIR2/MYDIR3/TEST_RENAME.TXT");
    return_if_fail (status == FX_SUCCESS);

    return_if_fail(ram_disk.fx_media_available_clusters == 0);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    /* Determine if the test was successful.  */
    if (status != FX_SUCCESS)
    {

        printf("ERROR!\n");
        test_control_return(169);
    }       
    else
    {

        printf("SUCCESS!\n");
        test_control_return(0);
    }
}

