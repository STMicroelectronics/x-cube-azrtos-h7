/* This FileX test concentrates on the Fault-Tolerant enable feature.  */
/* 
For FAT 12, 16, 32 and exFAT, one cluster size is 1024 bytes;
Check Fault tolerrant Log file:           
Step1: Format and open the media; 
Step2: Check the fault tolerant enable flag and boot index value; 
Step3: Enable fault tolerant feature;  
Step4: Check the fault tolerant enable flag, boot index value and fault tolerant header value; 
*/

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#include   "tx_thread.h"
#include   "tx_timer.h"
#endif
#include   "fx_api.h"    
#include   "fx_utility.h"
#include   "fx_fault_tolerant.h"
#include   <stdio.h>
#include   "fx_ram_driver_test.h"               
extern void    test_control_return(UINT status);
void    filex_fault_tolerant_enable_test_application_define(void *first_unused_memory);
                                         
#if defined (FX_ENABLE_FAULT_TOLERANT) && defined (FX_FAULT_TOLERANT)

#define DEMO_STACK_SIZE         4096
#define CACHE_SIZE              2048
#define FAULT_TOLERANT_SIZE     FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD                ftest_0;
#endif
static FX_MEDIA                 ram_disk;
static UCHAR                    *pointer;

/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                    *cache_buffer;
static UCHAR                    *fault_tolerant_buffer;
#else
static UCHAR                    cache_buffer[CACHE_SIZE];
static UCHAR                    fault_tolerant_buffer[FAULT_TOLERANT_SIZE];
#endif

#ifdef FX_ENABLE_EXFAT
#define TEST_COUNT              4
#else              
#define TEST_COUNT              3
#endif

/* Define thread prototypes.  */

static void    ftest_0_entry(ULONG thread_input);     
extern void    _fx_ram_driver(FX_MEDIA *media_ptr);
extern void    test_control_return(UINT status);           


/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_enable_test_application_define(void *first_unused_memory)
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

UINT        status, temp1;
UINT        i = 0, j; 
ULONG       start_cluster;  
ULONG       clusters;
ULONG       bytes_per_cluster;
ULONG       value;
ULONG       start_sector, temp;
UCHAR       buffer[2048];   
UCHAR       *source_buffer;
ULONG       checksum;
UCHAR       flag;
UINT        size;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Enable Test.............................");
                  
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
                                     256,                    // Sector size   
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
                                     256,                    // Sector size   
                                     8,                      // Sectors per cluster
                                     1,                      // Heads
                                     1);                     // Sectors per track 
        }  
        else if (i == 2)
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
                                     256,                    // Sector size   
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
                                            512,                   // Sector size   
                                            4,                      // exFAT Sectors per cluster
                                            12345,                  // Volume ID
                                            0);                     // Boundary unit
        }
#endif
        return_if_fail( status == FX_SUCCESS);

        /* Open the ram_disk.  */
        status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large, cache_buffer, CACHE_SIZE);
        return_if_fail( status == FX_SUCCESS);
        return_if_fail(ram_disk.fx_media_fault_tolerant_enabled != FX_TRUE);

        /* Read the boot sector.  */
        source_buffer = ((UCHAR *) ram_disk.fx_media_driver_info);

        /* Copy the RAM sector into the destination.  */
        _fx_utility_memory_copy(source_buffer, buffer, ram_disk.fx_media_bytes_per_sector);         

        /* Check whether the boot index is used. */
        start_cluster = _fx_utility_32_unsigned_read(buffer + FX_FAULT_TOLERANT_BOOT_INDEX);  
        return_if_fail( start_cluster == 0);
    
        /* Atempt to call fx_fault_tolerant_enable with invalid parameters. */
        temp = ram_disk.fx_media_sectors_per_cluster;
        ram_disk.fx_media_sectors_per_cluster = 0;
        status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
        ram_disk.fx_media_sectors_per_cluster = temp;
        return_if_fail( status == FX_MEDIA_INVALID);
        
        status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, 0);
        return_if_fail( status == FX_NOT_ENOUGH_MEMORY);

        /* Make an IO ERROR while reading boot sector. */
        _fx_ram_driver_io_error_request = 1;
        status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
        return_if_fail( status == FX_BOOT_ERROR);
        
        /* What will happen if there is no space? */
        temp = ram_disk.fx_media_available_clusters;
        ram_disk.fx_media_available_clusters = 0;
        status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
        return_if_fail( status == FX_NO_MORE_SPACE);
        ram_disk.fx_media_available_clusters = temp;

        temp = ram_disk.fx_media_total_clusters;
        ram_disk.fx_media_total_clusters = 0;
        status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
        return_if_fail( status == FX_NO_MORE_SPACE);
        ram_disk.fx_media_total_clusters = temp;

        /* Make IO error in _fx_fault_tolerant_reset_log_file. */
        if ( i == 0)
        {
            _fx_ram_driver_io_error_request = 4;
            status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
            return_if_fail( status == FX_IO_ERROR);
        }
        
        /* Enable the Fault-tolerant feature.  */                            
        status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
        return_if_fail( status == FX_SUCCESS);

        /* Check the fault tolerant feature flag.  */
        return_if_fail( ram_disk.fx_media_fault_tolerant_enabled == FX_TRUE);

        /* Atempt to exceed the space limitation in _fx_fault_tolerant_add_FAT_log. */
        temp = ram_disk.fx_media_fault_tolerant_file_size;    
        temp1 = ram_disk.fx_media_fault_tolerant_state;
        ram_disk.fx_media_fault_tolerant_state |= FX_FAULT_TOLERANT_STATE_STARTED;
        ram_disk.fx_media_fault_tolerant_state &= (UCHAR)~FX_FAULT_TOLERANT_STATE_SET_FAT_CHAIN;
        ram_disk.fx_media_fault_tolerant_file_size = 0xffff0000;
        status = _fx_utility_FAT_entry_write( &ram_disk, 1, 2);
        ram_disk.fx_media_fault_tolerant_file_size = temp;    
        ram_disk.fx_media_fault_tolerant_state = (UCHAR)temp1;
        return_if_fail( status == FX_NO_MORE_SPACE);

        /* Read the boot sector.  */
        source_buffer = ((UCHAR *) ram_disk.fx_media_driver_info);

        /* Copy the RAM sector into the destination.  */
        _fx_utility_memory_copy(source_buffer, buffer, ram_disk.fx_media_bytes_per_sector);         

        /* Check whether the boot index is used. */
        start_cluster = _fx_utility_32_unsigned_read(buffer + FX_FAULT_TOLERANT_BOOT_INDEX);  
        return_if_fail( start_cluster != 0);

        /* Get the FAT table address.  */
        source_buffer = ((UCHAR *) ram_disk.fx_media_driver_info) + ((ram_disk.fx_media_reserved_sectors + ram_disk.fx_media_hidden_sectors) * ram_disk.fx_media_bytes_per_sector);

        bytes_per_cluster = ram_disk.fx_media_bytes_per_sector * ram_disk.fx_media_sectors_per_cluster;
        clusters = (FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE + bytes_per_cluster - 1) / bytes_per_cluster;
                                                
        for (j = 0; j < clusters; j++)
        {
            status = _fx_utility_FAT_entry_read(&ram_disk, start_cluster + j, &value);
            return_if_fail (status == FX_SUCCESS);

            if (j < (clusters - 1))
            {
                return_if_fail (value == (start_cluster + j + 1));
            }
            else
            {
                return_if_fail (value == ram_disk.fx_media_fat_last);
            }
        }

        /* Get the start sector. */
        start_sector = (start_cluster - FX_FAT_ENTRY_START) * ram_disk.fx_media_sectors_per_cluster + ram_disk.fx_media_data_sector_start;

        /* Read the log sector.  */
        source_buffer =  ((UCHAR *) ram_disk.fx_media_driver_info) + ((start_sector + ram_disk.fx_media_hidden_sectors) * ram_disk.fx_media_bytes_per_sector);

        /* Copy the one cluster into the destination.  */
        _fx_utility_memory_copy(source_buffer, buffer, 1 * ram_disk.fx_media_sectors_per_cluster * ram_disk.fx_media_bytes_per_sector);        

        /* Verify ID field. */
        return_if_fail(_fx_utility_32_unsigned_read(buffer) == FX_FAULT_TOLERANT_ID);

        /* Get the header size.  */
        size = _fx_utility_16_unsigned_read(buffer + 4);    

        /* Verify Size field. */
        return_if_fail( size == FX_FAULT_TOLERANT_LOG_HEADER_SIZE + FX_FAULT_TOLERANT_FAT_CHAIN_SIZE);

        /* Calculate checksum of header. */
        checksum = _fx_fault_tolerant_calculate_checksum(buffer, FX_FAULT_TOLERANT_LOG_HEADER_SIZE);
        return_if_fail( checksum == 0);

        /* Verify checksum of undo logs. */
        checksum = _fx_fault_tolerant_calculate_checksum(buffer + FX_FAULT_TOLERANT_FAT_CHAIN_OFFSET, FX_FAULT_TOLERANT_FAT_CHAIN_SIZE);
        return_if_fail( checksum == 0);
                     
        /* Check undo log. */
        flag = *(buffer + FX_FAULT_TOLERANT_FAT_CHAIN_OFFSET + 2); 
        return_if_fail( flag == 0);

        /* Close the media.  */
        status =  fx_media_close(&ram_disk);
        return_if_fail( status == FX_SUCCESS);
    }

    /* Output successful.  */     
    printf("SUCCESS!\n");
    test_control_return(0);
}         

#else  

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_enable_test_application_define(void *first_unused_memory)
#endif
{
    
    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant Enable Test.............................N/A\n");  

    test_control_return(255);
}
#endif
