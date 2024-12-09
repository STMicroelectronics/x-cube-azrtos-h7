/* This FileX test concentrates on the basic media read/write sector operation.  */
/* While FX_FAULT_TOLERANT is not defined, fx_utility_logical_sector_write will not cache other types of sectors but FX_DATA_SECTOR. */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_utility.h"
#include   "fx_ram_driver_test.h"
#include   <stdio.h>

void  test_control_return(UINT status);
#ifndef FX_DISABLE_CACHE
#define     DEMO_STACK_SIZE         4096
#define     CACHE_SIZE              256*128


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD               ftest_0;
#endif
static FX_MEDIA                ram_disk;

static UCHAR                   raw_sector_buffer[128];


/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                  *ram_disk_memory;
static UCHAR                  *direct_buffer;
static UCHAR                  *cache_buffer;
#else
static UCHAR                   direct_buffer[20*128];
static UCHAR                   cache_buffer[CACHE_SIZE];
#endif


/* Define thread prototypes.  */

void    filex_media_read_write_sector_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_media_read_write_sector_application_define(void *first_unused_memory)
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
    direct_buffer =  pointer;
    pointer =  pointer + 20*128;   
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
#ifdef FX_FAULT_TOLERANT
UINT        first_test = 1;
#endif /* FX_FAULT_TOLERANT */

ULONG64     logical_sector;
UINT        i, j;
ULONG       fat_entry;
ULONG       temp;
ULONG64     accessed_sectors[300];

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Media read/write sector test...........................");

#ifdef FX_FAULT_TOLERANT
TEST_START:
#endif /* FX_FAULT_TOLERANT */

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            2,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            256,                    // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    return_if_fail( status == FX_SUCCESS);
    
    /* try to read before the media has been opened */
    status =  fx_media_read(&ram_disk, 1, (VOID *) raw_sector_buffer); 
    return_if_fail( status == FX_MEDIA_NOT_OPEN);
    
    /* try to write before the media has been opened  */
    status =  fx_media_write(&ram_disk, 1, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_MEDIA_NOT_OPEN);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);
    
    /* try to write while the media is write protected  */
    ram_disk.fx_media_driver_write_protect = FX_TRUE;
    status =  fx_media_write(&ram_disk, 1, (VOID *) raw_sector_buffer);  
    ram_disk.fx_media_driver_write_protect = FX_FALSE;
    return_if_fail( status == FX_WRITE_PROTECT);
    
/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING

    /* send null pointer to generate an error */
    status = fx_media_read(FX_NULL, 0, (void *) raw_sector_buffer);
    return_if_fail( status == FX_PTR_ERROR);
    
    /* send null pointer to generate an error */
    status = fx_media_write(FX_NULL, 0, (void *) raw_sector_buffer);
    return_if_fail( status == FX_PTR_ERROR);

#endif /* FX_DISABLE_ERROR_CHECKING */

    /* Read the first FAT sector.  */
    status =  fx_media_read(&ram_disk, 1, (VOID *) raw_sector_buffer);  
    return_if_fail ((status == FX_SUCCESS) && 
        (raw_sector_buffer[0] == 0xF8) &&    /* _fx_media_format_media_type value set during media format */
        (raw_sector_buffer[1] == 0xFF) &&
        (raw_sector_buffer[2] == 0xFF));
    
    /* Write the same, unchanged sector back.  */
    status =  fx_media_write(&ram_disk, 1, (VOID *) raw_sector_buffer);  
    return_if_fail( status == FX_SUCCESS);

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
                            2,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            7000,                   // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

    /* Test for invalid sector.  */
    logical_sector = ram_disk.fx_media_total_sectors;
    status = _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
    return_if_fail( status == FX_SECTOR_INVALID);

    /* Test for invalid sector.  */
    logical_sector =  0;
    status =  _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
    return_if_fail( status == FX_SECTOR_INVALID);

    /* Test for invalid sector.  */
    logical_sector =  0xFFFFFFFF;
    status =  _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
    return_if_fail( status == FX_SECTOR_INVALID);
    
    /* Test for invalid sector.  */
    logical_sector =  ram_disk.fx_media_total_sectors;
    status =  _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 4, FX_DATA_SECTOR);
    return_if_fail( status == FX_SECTOR_INVALID);

    /* Test for invalid sector.  */
    logical_sector =  0;
    status =  _fx_utility_logical_sector_write(&ram_disk, logical_sector, direct_buffer, 4, FX_DATA_SECTOR);
    return_if_fail( status == FX_SECTOR_INVALID);

    /* Test for invalid sector.  */
    logical_sector =  0xFFFFFFFF;
    status =  _fx_utility_logical_sector_write(&ram_disk, logical_sector, direct_buffer, 4, FX_DATA_SECTOR);
    return_if_fail( status == FX_SECTOR_INVALID);
    
    /* Test for invalid sector.  */
    logical_sector =  ram_disk.fx_media_total_sectors;
    status =  _fx_utility_logical_sector_write(&ram_disk, logical_sector, direct_buffer, 4, FX_DATA_SECTOR);
    return_if_fail( status == FX_SECTOR_INVALID);
    
    /* Write to a sector not in the cache.  */
    status =  _fx_utility_logical_sector_write(&ram_disk, 6900, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);


    /* Read/write 256 sectors, filling the caches.  */
    logical_sector =  ram_disk.fx_media_data_sector_start + 4;
    for (i = 0; i < 256; i++)
    {
    
        status += _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
#ifdef FX_FAULT_TOLERANT
        if (first_test)
        {
            status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        }
        else
        {
#endif
            status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
#ifdef FX_FAULT_TOLERANT
        }
#endif
        logical_sector++;
    }
    
    /* At this point the cache is complete full.  */
    
    /* Now perform direct buffer read ram_disk.fx_media_data_sector_start on sectors before the cache.  */
    logical_sector =  + 2;
    status =  _fx_utility_logical_sector_read(&ram_disk, logical_sector, direct_buffer, 4, FX_DIRECTORY_SECTOR);
    
    /* Now perform direct buffer write.  */
#ifdef FX_FAULT_TOLERANT
    if (first_test)
    {
        status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, direct_buffer, 4, FX_DATA_SECTOR);
    }
    else
    {
#endif
        status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, direct_buffer, 4, FX_DIRECTORY_SECTOR);
#ifdef FX_FAULT_TOLERANT
    }
#endif
    
    /* Perform a flush.  */
    status += _fx_utility_logical_sector_flush(&ram_disk, logical_sector, 300, FX_TRUE);

    /* Perform another flush with nothing in the cache.  */
    status += _fx_utility_logical_sector_flush(&ram_disk, logical_sector, 300, FX_TRUE);

    /* Read/write 256 sectors, filling the caches.  */
    logical_sector =  ram_disk.fx_media_data_sector_start + 4;
    for (i = 0; i < 256; i++)
    {
    
        status += _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
#ifdef FX_FAULT_TOLERANT
        if (first_test)
        {
            status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        }
        else
        {
#endif
            status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
#ifdef FX_FAULT_TOLERANT
        }
#endif
        logical_sector++;
    }

    /* Now perform direct buffer read ram_disk.fx_media_data_sector_start on sectors before the cache.  */
    logical_sector = (ULONG64)-2;
    status =  _fx_utility_logical_sector_read(&ram_disk, logical_sector, direct_buffer, 4, FX_DIRECTORY_SECTOR);
    
    /* Now perform direct buffer write.  */
#ifdef FX_FAULT_TOLERANT
    if (first_test)
    {
        status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, direct_buffer, 4, FX_DATA_SECTOR);
    }
    else
    {
#endif
        status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, direct_buffer, 4, FX_DIRECTORY_SECTOR);
#ifdef FX_FAULT_TOLERANT
    }
#endif

    /* Perform a flush.  */
    status += _fx_utility_logical_sector_flush(&ram_disk, logical_sector, 300, FX_FALSE);

    /* Read/write 128 sectors, filling the caches and causing cache misses.  */
    logical_sector =  ram_disk.fx_media_data_sector_start + 4;
    for (i = 0; i < 512; i++)
    {
    
        status += _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
#ifdef FX_FAULT_TOLERANT
        if (first_test)
        {
            status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        }
        else
        {
#endif
            status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
#ifdef FX_FAULT_TOLERANT
        }
#endif
        logical_sector++;
    }

    /* Perform a flush.  */
    status += _fx_utility_logical_sector_flush(&ram_disk, logical_sector, 300, FX_FALSE);

    /* Read 256 sectors, filling the caches.  */
    logical_sector =  ram_disk.fx_media_data_sector_start + 4;
    for (i = 0; i < 256; i++)
    {
   
        status += _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
        logical_sector++;
    }

    /* Write same 256 sectors in reverse order.  */
    for (i = 0; i < 256; i++)
    {  
    
        logical_sector--;
#ifdef FX_FAULT_TOLERANT
        if (first_test)
        {
            status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        }
        else
        {
#endif
            status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
#ifdef FX_FAULT_TOLERANT
        }
#endif
    }

    /* Update FAT entires.  */
    for (i = 2; i < 6000; i++)
    {
        status +=  _fx_utility_FAT_entry_write(&ram_disk, (ULONG) i, (ULONG) i+1);
    }

    /* Flush the FAT.  */
    status += _fx_utility_FAT_map_flush(&ram_disk);
        
    /* Indicate that the driver needs to know!  */
    ram_disk.fx_media_driver_free_sector_update =  FX_TRUE;
        
    /* Update FAT entries again, but this time with free marker.  */
    for (i = 2; i < 6000; i++)
    {
        status +=  _fx_utility_FAT_entry_write(&ram_disk, (ULONG) i, FX_FREE_CLUSTER);
    }
   
    /* Flush the FAT.  */
    status += _fx_utility_FAT_map_flush(&ram_disk);
    ram_disk.fx_media_driver_free_sector_update =  FX_FALSE;

    /* Random test of 300 sectors.  */
    for (i = 0; i < 300; i++)
    {
        accessed_sectors[i] =  logical_sector + (ULONG64)(rand() % 5000); 
        status += _fx_utility_logical_sector_read(&ram_disk, accessed_sectors[i], ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        if ((i > 0) && (rand() & 1))
        {
            j = ((UINT)rand() % i);
            status += _fx_utility_logical_sector_write(&ram_disk, accessed_sectors[j], ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        }
    }

    /* Random test of 300 sectors with random logical sector flush.  */
    for (i = 0; i < 300; i++)
    {
        accessed_sectors[i] =  logical_sector + (ULONG64)(rand() % 5000); 
        status += _fx_utility_logical_sector_read(&ram_disk, accessed_sectors[i], ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        if ((i > 0) && (rand() & 1))
        {
            j = ((UINT)rand() % i);
            status += _fx_utility_logical_sector_write(&ram_disk, accessed_sectors[j], ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        }
        if ((i > 0) && (rand() & 1))
        {
            j = ((UINT)rand() % i);
            if (rand() & 1)
                status += _fx_utility_logical_sector_flush(&ram_disk, accessed_sectors[j], 1, FX_TRUE);
            else
                status += _fx_utility_logical_sector_flush(&ram_disk, accessed_sectors[j], 1, FX_FALSE);           
        }
    }

    /* Random test of 1000 FAT entry reads/writes.  */
    for (i = 0; i < 1000; i++)
    {
    
        j = ((UINT)rand() % 100);
        status +=  _fx_utility_FAT_entry_read(&ram_disk, j+2, &fat_entry);

        if (rand() & 1)
        {
            j = ((UINT)rand() % 100);
            if (rand() & 1)
                status +=  _fx_utility_FAT_entry_write(&ram_disk, j+2, j+3);
            else
                status +=  _fx_utility_FAT_entry_write(&ram_disk, j+2, FX_FREE_CLUSTER);                            
        }
    }
    
    /* Random test of 1000 FAT entry reads/writes with random FAT flush.  */
    for (i = 0; i < 1000; i++)
    {
    
        j = ((UINT)rand() % 100);
        status +=  _fx_utility_FAT_entry_read(&ram_disk, j+2, &fat_entry);

        if (rand() & 1)
        {
            j = ((UINT)rand() % 100);
            if (rand() & 1)
                status +=  _fx_utility_FAT_entry_write(&ram_disk, j+2, j+3);
            else
                status +=  _fx_utility_FAT_entry_write(&ram_disk, j+2, FX_FREE_CLUSTER);                            
        }
        
        if (rand() & 1)
        {
        
            status += _fx_utility_FAT_flush(&ram_disk);
        }

        if (rand() & 1)
        {
        
            status += _fx_utility_FAT_map_flush(&ram_disk);
        }
    }
    return_if_fail( status == FX_SUCCESS);

    /* Set the free sector update.  */
    ram_disk.fx_media_driver_free_sector_update  =  FX_TRUE;

    /* Random test of 1000 FAT entry reads/writes with random FAT flush and random errors.  */
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, ram_disk.fx_media_data_sector_start, 60000, FX_TRUE);
    _fx_ram_driver_io_error_request =  1000;
    for (i = 0; i < 300000; i++)
    {
               
        j =  ((UINT)rand() % 100);
         _fx_utility_FAT_entry_read(&ram_disk, j+2, &fat_entry);

        if (rand() & 1)
        {
            j = ((UINT)rand() % 100);
            if (rand() & 1)
                _fx_utility_FAT_entry_write(&ram_disk, j+2, j+3);
            else
                _fx_utility_FAT_entry_write(&ram_disk, j+2, FX_FREE_CLUSTER);                            
        }
 
        if (i & 1)
            _fx_utility_FAT_flush(&ram_disk);
        else
            _fx_utility_FAT_map_flush(&ram_disk);
        _fx_utility_logical_sector_flush(&ram_disk, ram_disk.fx_media_data_sector_start, 60000, FX_TRUE);
    }
    _fx_ram_driver_io_error_request =  0;

    /* Set the free sector update.  */
    ram_disk.fx_media_driver_free_sector_update  =  FX_FALSE;

    /* Flush out the logical sector cache.  */
    status = _fx_utility_logical_sector_flush(&ram_disk, ram_disk.fx_media_data_sector_start, 60000, FX_TRUE);

    /* Read/write 256 sectors, filling the caches.  */
    logical_sector =  ram_disk.fx_media_data_sector_start + 4;
    for (i = 0; i < 256; i++)
    {
    
        status += _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
#ifdef FX_FAULT_TOLERANT
        if (first_test)
        {
            status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        }
        else
        {
#endif
            status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
#ifdef FX_FAULT_TOLERANT
        }
#endif
        logical_sector++;
    }

    /* Now test the flush logic with write protection enabled.  */
    ram_disk.fx_media_driver_write_protect =  FX_TRUE;
    status += _fx_utility_logical_sector_flush(&ram_disk, ram_disk.fx_media_data_sector_start, 60000, FX_TRUE);
    ram_disk.fx_media_driver_write_protect =  FX_FALSE;
    return_if_fail( status == FX_SUCCESS);

    /* Flush out the logical sector cache.  */
    _fx_utility_logical_sector_flush(&ram_disk, ram_disk.fx_media_data_sector_start, 60000, FX_TRUE);

    /* Read/write 256 sectors, filling the caches.  */
    logical_sector =  ram_disk.fx_media_data_sector_start + 4;
    for (i = 0; i < 256; i++)
    {
    
        status += _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
#ifdef FX_FAULT_TOLERANT
        if (first_test)
        {
            status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        }
        else
        {
#endif
            status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
#ifdef FX_FAULT_TOLERANT
        }
#endif
        logical_sector++;
    }

    /* Now test the flush logic with an I/O error.  */
    _fx_ram_driver_io_error_request =  1;
    status += _fx_utility_logical_sector_flush(&ram_disk, ram_disk.fx_media_data_sector_start, 60000, FX_TRUE);
    _fx_ram_driver_io_error_request =  0;

    /* Flush out the logical sector cache.  */
    status =  _fx_utility_logical_sector_flush(&ram_disk, ram_disk.fx_media_data_sector_start, 60000, FX_TRUE);

    /* Read/write 256 sectors, filling the caches.  */
    logical_sector =  ram_disk.fx_media_data_sector_start + 4;
    for (i = 0; i < 256; i++)
    {
    
        status += _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
#ifdef FX_FAULT_TOLERANT
        if (first_test)
        {
            status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        }
        else
        {
#endif
            status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
#ifdef FX_FAULT_TOLERANT
        }
#endif
        logical_sector++;
    }

    /* Now test the flush logic with an I/O error.  */
    status += _fx_utility_logical_sector_flush(&ram_disk, ram_disk.fx_media_data_sector_start, 60000, FX_TRUE);

    /* Close the media.  */
    status += fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            2,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            7000,                   // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    
    /* Open the ram_disk.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE-1);
    return_if_fail( status == FX_SUCCESS);

    /* Test for invalid sector.  */
    logical_sector =  ram_disk.fx_media_total_sectors;
    status =  _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
    return_if_fail( status == FX_SECTOR_INVALID);

    /* Test for invalid sector.  */
    logical_sector =  0;
    status =  _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
    return_if_fail( status == FX_SECTOR_INVALID);

    /* Test for invalid sector.  */
    logical_sector =  0xFFFFFFFF;
    status =  _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
    return_if_fail( status == FX_SECTOR_INVALID);

    /* Test for invalid sector.  */
    logical_sector =  ram_disk.fx_media_total_sectors;
    status =  _fx_utility_logical_sector_read(&ram_disk, logical_sector, direct_buffer, 4, FX_DATA_SECTOR);
    return_if_fail( status == FX_SECTOR_INVALID);

    /* Test for invalid sector.  */
    logical_sector =  0;
    status =  _fx_utility_logical_sector_write(&ram_disk, logical_sector, direct_buffer, 4, FX_DATA_SECTOR);
    return_if_fail( status == FX_SECTOR_INVALID);

    /* Test for invalid sector.  */
    logical_sector =  0xFFFFFFFF;
    status =  _fx_utility_logical_sector_write(&ram_disk, logical_sector, direct_buffer, 4, FX_DATA_SECTOR);
    return_if_fail( status == FX_SECTOR_INVALID);
    
    /* Test for invalid sector.  */
    logical_sector =  ram_disk.fx_media_total_sectors;
    status =  _fx_utility_logical_sector_write(&ram_disk, logical_sector, direct_buffer, 4, FX_DATA_SECTOR);
    return_if_fail( status == FX_SECTOR_INVALID);
    
    status =  0;

    /* Read/write 256 sectors, filling the caches.  */
    logical_sector =  ram_disk.fx_media_data_sector_start + 4;
    for (i = 0; i < 256; i++)
    {
    
        status += _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
        status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        logical_sector++;
    }
    
    /* At this point the cache is complete full.  */
    
    /* Now perform direct buffer read ram_disk.fx_media_data_sector_start on sectors before the cache.  */
    logical_sector =  + 2;
    status =  _fx_utility_logical_sector_read(&ram_disk, logical_sector, direct_buffer, 4, FX_DIRECTORY_SECTOR);
    
    /* Now perform direct buffer write.  */
    status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, direct_buffer, 4, FX_DATA_SECTOR);
    
    /* Perform a flush.  */
    status += _fx_utility_logical_sector_flush(&ram_disk, logical_sector, 300, FX_TRUE);

    /* Perform another flush with nothing in the cache.  */
    status += _fx_utility_logical_sector_flush(&ram_disk, logical_sector, 300, FX_TRUE);

    /* Read/write 256 sectors, filling the caches.  */
    logical_sector =  ram_disk.fx_media_data_sector_start + 4;
    for (i = 0; i < 256; i++)
    {
    
        status += _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
        status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        logical_sector++;
    }

    /* Now perform direct buffer read ram_disk.fx_media_data_sector_start on sectors before the cache.  */
    logical_sector = (ULONG64)-2;
    status =  _fx_utility_logical_sector_read(&ram_disk, logical_sector, direct_buffer, 4, FX_DIRECTORY_SECTOR);
    
    /* Now perform direct buffer write.  */
    status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, direct_buffer, 4, FX_DATA_SECTOR);

    /* Perform a flush.  */
    status += _fx_utility_logical_sector_flush(&ram_disk, logical_sector, 300, FX_FALSE);

    /* Read/write 300 sectors, filling the caches and causing cache misses.  */
    logical_sector =  ram_disk.fx_media_data_sector_start + 4;
    for (i = 0; i < 300; i++)
    {
    
        status += _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
        status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        logical_sector++;
    }

    /* Perform a flush.  */
    status += _fx_utility_logical_sector_flush(&ram_disk, logical_sector, 300, FX_FALSE);

    /* Read 256 sectors, filling the caches.  */
    logical_sector =  ram_disk.fx_media_data_sector_start + 4;
    for (i = 0; i < 256; i++)
    {
   
        status += _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
        logical_sector++;
    }

    /* Write same 256 sectors in reverse order.  */
    for (i = 0; i < 256; i++)
    {  
    
        logical_sector--;
        status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
    }

    /* Update FAT entires.  */
    for (i = 2; i < 6000; i++)
    {
        status +=  _fx_utility_FAT_entry_write(&ram_disk, (ULONG) i, (ULONG) i+1);
    }

    /* Flush the FAT.  */
    status += _fx_utility_FAT_map_flush(&ram_disk);
        
    /* Indicate that the driver needs to know!  */
    ram_disk.fx_media_driver_free_sector_update =  FX_TRUE;
        
    /* Update FAT entries again, but this time with free marker.  */
    for (i = 2; i < 6000; i++)
    {
        status +=  _fx_utility_FAT_entry_write(&ram_disk, (ULONG) i, FX_FREE_CLUSTER);
    }
   
    /* Flush the FAT.  */
    status += _fx_utility_FAT_map_flush(&ram_disk);
    ram_disk.fx_media_driver_free_sector_update =  FX_FALSE;

    /* Test the case where the logical sector cache has something in it, but the write hasn't been found. */
    logical_sector =  ram_disk.fx_media_data_sector_start + 4;
    status += _fx_utility_logical_sector_read(&ram_disk, logical_sector+1, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
    status += _fx_utility_logical_sector_read(&ram_disk, logical_sector+2, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
    status += _fx_utility_logical_sector_read(&ram_disk, logical_sector+3, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
    status += _fx_utility_logical_sector_read(&ram_disk, logical_sector+4, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
    status += _fx_utility_logical_sector_write(&ram_disk, logical_sector+1, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);

    /* Random test of 300 sectors.  */
    for (i = 0; i < 300; i++)
    {
        accessed_sectors[i] = logical_sector + (ULONG64)(rand() % 5000); 
        status += _fx_utility_logical_sector_read(&ram_disk, accessed_sectors[i], ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        if ((i > 0) && (rand() & 1))
        {
            j = ((UINT)rand() % i);
            status += _fx_utility_logical_sector_write(&ram_disk, accessed_sectors[j], ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        }
    }

    /* Random test of 300 sectors with random logical sector flush.  */
    for (i = 0; i < 300; i++)
    {
        accessed_sectors[i] = logical_sector + (ULONG64)(rand() % 5000); 
        status += _fx_utility_logical_sector_read(&ram_disk, accessed_sectors[i], ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        if ((i > 0) && (rand() & 1))
        {
            j =  ((UINT)rand() % i);
            status += _fx_utility_logical_sector_write(&ram_disk, accessed_sectors[j], ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        }
        if ((i > 0) && (rand() & 1))
        {
            j =  ((UINT)rand() % i);
            if (rand() & 1)
                status += _fx_utility_logical_sector_flush(&ram_disk, accessed_sectors[j], 1, FX_TRUE);
            else
                status += _fx_utility_logical_sector_flush(&ram_disk, accessed_sectors[j], 1, FX_FALSE);           
        }
    }
    
    /* Random test of 1000 FAT entry reads/writes. */
    for (i = 0; i < 1000; i++)
    {
    
        j = ((UINT)rand() % 100);
        status +=  _fx_utility_FAT_entry_read(&ram_disk, j+2, &fat_entry);

        if (rand() & 1)
        {
            j = ((UINT)rand() % 100);
            if (rand() & 1)
                status +=  _fx_utility_FAT_entry_write(&ram_disk, j+2, j+3);
            else
                status +=  _fx_utility_FAT_entry_write(&ram_disk, j+2, FX_FREE_CLUSTER);                            
        }
    }
    
    /* Random test of 1000 FAT entry reads/writes with random FAT flush.  */
    for (i = 0; i < 1000; i++)
    {
    
        j = ((UINT)rand() % 100);
        status +=  _fx_utility_FAT_entry_read(&ram_disk, j+2, &fat_entry);

        if (rand() & 1)
        {
            j = ((UINT)rand() % 100);
            if (rand() & 1)
                status +=  _fx_utility_FAT_entry_write(&ram_disk, j+2, j+3);
            else
                status +=  _fx_utility_FAT_entry_write(&ram_disk, j+2, FX_FREE_CLUSTER);                            
        }
        
        if (rand() & 1)
        {
        
            status += _fx_utility_FAT_flush(&ram_disk);
        }

        if (rand() & 1)
        {
        
            status += _fx_utility_FAT_map_flush(&ram_disk);
        }
    }
    return_if_fail( status == FX_SUCCESS);
   
    /* Set the free sector update.  */
    ram_disk.fx_media_driver_free_sector_update  =  FX_TRUE;

    /* Random test of 1000 FAT entry reads/writes with random FAT flush and random errors.  */
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_flush(&ram_disk, ram_disk.fx_media_data_sector_start, 60000, FX_TRUE);
    _fx_ram_driver_io_error_request =  1000;
    for (i = 0; i < 300000; i++)
    {
               

        j = ((UINT)rand() % 100);
        _fx_utility_logical_sector_read(&ram_disk, (ULONG64) j, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);

        if (rand() & 1)
            _fx_utility_logical_sector_write(&ram_disk, (ULONG64) j, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        
        j = ((UINT)rand() % 100);
         _fx_utility_FAT_entry_read(&ram_disk, j+2, &fat_entry);

        if (rand() & 1)
        {
            j = ((UINT)rand() % 100);
            if (rand() & 1)
                _fx_utility_FAT_entry_write(&ram_disk, j+2, j+3);
            else
                _fx_utility_FAT_entry_write(&ram_disk, j+2, FX_FREE_CLUSTER);                            
        }
        
        if (i & 1)
            _fx_utility_FAT_flush(&ram_disk);
        else
            _fx_utility_FAT_map_flush(&ram_disk);
    }
    _fx_ram_driver_io_error_request =  0;

    /* Set the free sector update.  */
    ram_disk.fx_media_driver_free_sector_update  =  FX_FALSE;

    /* Read/write 256 sectors, filling the caches.  */
    logical_sector =  ram_disk.fx_media_data_sector_start + 4;
    for (i = 0; i < 256; i++)
    {
    
        status += _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
        status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        logical_sector++;
    }

    /* Now perform direct buffer read ram_disk.fx_media_data_sector_start on sectors before the cache.  */
    logical_sector =  ram_disk.fx_media_data_sector_start;
    status =  _fx_utility_logical_sector_read(&ram_disk, logical_sector, direct_buffer, 12, FX_DIRECTORY_SECTOR);
    
    /* Now perform direct buffer write.  */
    status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, direct_buffer, 12, FX_DATA_SECTOR);

    /* Flush out the logical sector cache.  */
    status += _fx_utility_logical_sector_flush(&ram_disk, ram_disk.fx_media_data_sector_start, 60000, FX_TRUE);

    /* Read/write 256 sectors, filling the caches.  */
    logical_sector =  ram_disk.fx_media_data_sector_start + 4;
    for (i = 0; i < 256; i++)
    {
    
        status += _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
        status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        logical_sector++;
    }
    return_if_fail( status == FX_SUCCESS);

    /* Now direct read a logical sector outside the cache and cause an I/O error in the process.  */
    _fx_ram_driver_io_error_request =  2;
    logical_sector =  ram_disk.fx_media_data_sector_start;
    status =  _fx_utility_logical_sector_read(&ram_disk, logical_sector, direct_buffer, 1, FX_DIRECTORY_SECTOR);
    _fx_ram_driver_io_error_request =  0;

    /* Flush out the logical sector cache.  */
    status = _fx_utility_logical_sector_flush(&ram_disk, ram_disk.fx_media_data_sector_start, 60000, FX_TRUE);

    /* Read/write 256 sectors, filling the caches.  */
    logical_sector =  ram_disk.fx_media_data_sector_start + 4;
    for (i = 0; i < 256; i++)
    {
    
        status += _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
        status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        logical_sector++;
    }

    /* Perform a direct read that exceeds the amount that can fit into the cache easily.  */
    logical_sector =  6800;
    status +=  _fx_utility_logical_sector_read(&ram_disk, logical_sector, direct_buffer, 20, FX_UNKNOWN_SECTOR);
    return_if_fail( status == FX_SUCCESS);

    /* Flush out the logical sector cache.  */
    status = _fx_utility_logical_sector_flush(&ram_disk, ram_disk.fx_media_data_sector_start, 60000, FX_TRUE);

    /* Read/write 256 sectors, filling the caches.  */
    logical_sector =  ram_disk.fx_media_data_sector_start + 4;
    for (i = 0; i < 256; i++)
    {
    
        status += _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        logical_sector++;
    }

    /* Perform a direct read that suffers an I/O error.  */
    logical_sector =  6800;
    _fx_ram_driver_io_error_request =  1;
    status +=  _fx_utility_logical_sector_read(&ram_disk, logical_sector, direct_buffer, 1, FX_UNKNOWN_SECTOR);
    _fx_ram_driver_io_error_request =  0;

    /* Flush out the logical sector cache.  */
    status = _fx_utility_logical_sector_flush(&ram_disk, ram_disk.fx_media_data_sector_start, 60000, FX_TRUE);

    /* Read/write 256 sectors, filling the caches.  */
    logical_sector =  ram_disk.fx_media_data_sector_start + 4;
    for (i = 0; i < 256; i++)
    {
    
        status += _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        logical_sector++;
    }

    /* Perform a direct read that should be successful.  */
    logical_sector =  6800;
    status +=  _fx_utility_logical_sector_read(&ram_disk, logical_sector, direct_buffer, 1, FX_UNKNOWN_SECTOR);
    return_if_fail( status == FX_SUCCESS);

    /* Flush out the logical sector cache.  */
    status = _fx_utility_logical_sector_flush(&ram_disk, ram_disk.fx_media_data_sector_start, 60000, FX_TRUE);

    /* Read/write 256 sectors, filling the caches.  */
    logical_sector =  ram_disk.fx_media_data_sector_start + 4;
    for (i = 0; i < 256; i++)
    {
    
        status += _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
        status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        logical_sector++;
    }

    /* Now test the flush logic with write protection enabled.  */
    ram_disk.fx_media_driver_write_protect =  FX_TRUE;
    status += _fx_utility_logical_sector_flush(&ram_disk, ram_disk.fx_media_data_sector_start, 60000, FX_TRUE);
    ram_disk.fx_media_driver_write_protect =  FX_FALSE;
    return_if_fail( status == FX_SUCCESS);

    /* Flush out the logical sector cache.  */
    _fx_utility_logical_sector_flush(&ram_disk, ram_disk.fx_media_data_sector_start, 60000, FX_TRUE);

    /* Read/write 256 sectors, filling the caches.  */
    logical_sector =  ram_disk.fx_media_data_sector_start + 4;
    for (i = 0; i < 256; i++)
    {
    
        status += _fx_utility_logical_sector_read(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DIRECTORY_SECTOR);
        status += _fx_utility_logical_sector_write(&ram_disk, logical_sector, ram_disk.fx_media_memory_buffer, 1, FX_DATA_SECTOR);
        logical_sector++;
    }

    /* Now test the flush logic with an I/O error.  */
    _fx_ram_driver_io_error_request =  1;
    status += _fx_utility_logical_sector_flush(&ram_disk, ram_disk.fx_media_data_sector_start, 60000, FX_TRUE);
    _fx_ram_driver_io_error_request =  0;

    /* Flush the FAT.  */
    _fx_utility_logical_sector_flush(&ram_disk, ram_disk.fx_media_data_sector_start, 60000, FX_TRUE);
    _fx_utility_FAT_flush(&ram_disk);

    /* Now test the FAT map flush with an I/O error on the read of the primary FAT sector.  */
    _fx_ram_driver_io_error_request =  1;
    status =  _fx_utility_FAT_map_flush(&ram_disk);
    _fx_ram_driver_io_error_request =  0;

    /* Now test the FAT map flush with an I/O error on the write of the secondary FAT sector.  */
    _fx_ram_driver_io_error_request =  3;
    status =  _fx_utility_FAT_map_flush(&ram_disk);
    _fx_ram_driver_io_error_request =  0;

    /* Close the media.  */
    status = fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* FAT12 FAT flush I/O error testing.  */
    
    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            2,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            2000,                   // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 

    /* Open the ram_disk.  */
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_if_fail( status == FX_SUCCESS);

    /* Now loop to write the FAT entries out with random I/O erorrs.  */
    fat_entry =  4;
    for (i = 0; i < 500000; i++) 
    {

        _fx_ram_driver_io_error_request = ((UINT)rand() % 5);
        if (_fx_ram_driver_io_error_request == 0)
            _fx_ram_driver_io_error_request = 1;
        _fx_utility_FAT_entry_read(&ram_disk, fat_entry, &temp);
        _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
        _fx_utility_FAT_entry_write(&ram_disk, fat_entry, fat_entry+1);
        _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);

        /* Move to next FAT entry.  */
        fat_entry++;          
        if (fat_entry >= (ram_disk.fx_media_available_clusters-1))
            fat_entry =  4;
    }
    for (i = 0; i < 500000; i++) 
    {

        _fx_ram_driver_io_error_request = ((UINT)rand() % 5);
        if (_fx_ram_driver_io_error_request == 0)
            _fx_ram_driver_io_error_request = 1;
        _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
        _fx_utility_FAT_entry_write(&ram_disk, fat_entry, fat_entry+1);

        /* Move to next FAT entry.  */
        fat_entry++;          
        if (fat_entry >= (ram_disk.fx_media_available_clusters-1))
            fat_entry =  4;
    }
    _fx_ram_driver_io_error_request =  0;

    /* Set the error flag to make logical sector write return an error.  */
    _fx_utility_logical_sector_write_error_request =  1;
    status =  _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_write_error_request =  70000;    /* This should not hit, but satisfy the code coverage.  */ 
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_write_error_request =  0;

    /* Close the media.  */
    status = fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* FAT16 FAT flush I/O error testing.  */
    
    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            2,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            7000,                   // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_if_fail( status == FX_SUCCESS);

    /* Now loop to write the FAT entries out with random I/O erorrs.  */
    fat_entry =  4;
    for (i = 0; i < 500000; i++) 
    {

        _fx_ram_driver_io_error_request = ((UINT)rand() % 5);
        if (_fx_ram_driver_io_error_request == 0)
            _fx_ram_driver_io_error_request = 1;
        _fx_utility_FAT_entry_read(&ram_disk, fat_entry, &temp);
        _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
        _fx_utility_FAT_entry_write(&ram_disk, fat_entry, fat_entry+1);
        _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);

        /* Move to next FAT entry.  */
        fat_entry++;          
        if (fat_entry >= (ram_disk.fx_media_available_clusters-1))
            fat_entry =  4;
    }
    for (i = 0; i < 500000; i++) 
    {

        _fx_ram_driver_io_error_request =  ((UINT)rand() % 5);
        if (_fx_ram_driver_io_error_request == 0)
            _fx_ram_driver_io_error_request = 1;
        _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
        _fx_utility_FAT_entry_write(&ram_disk, fat_entry, fat_entry+1);

        /* Move to next FAT entry.  */
        fat_entry++;          
        if (fat_entry >= (ram_disk.fx_media_available_clusters-1))
            fat_entry =  4;
    }
    _fx_ram_driver_io_error_request = 0;
    
    /* Set the error flag to make logical sector write return an error.  */
    _fx_utility_logical_sector_write_error_request =  1;
    status =  _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_write_error_request =  70000;    /* This should not hit, but satisfy the code coverage.  */ 
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_write_error_request =  0;

    /* Close the media.  */
    status = fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);
    
    /* FAT32 FAT flush I/O error testing.  */

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk, 
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size 
                            "MY_RAM_DISK",          // Volume Name
                            2,                      // Number of FATs
                            32,                     // Directory Entries
                            0,                      // Hidden sectors
                            70128,                   // Total sectors 
                            128,                    // Sector size   
                            1,                      // Sectors per cluster
                            1,                      // Heads
                            1);                     // Sectors per track 
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 128);
    return_if_fail( status == FX_SUCCESS);

    /* Now loop to write the FAT entries out with random I/O erorrs.  */
    fat_entry =  4;
    for (i = 0; i < 500000; i++) 
    {

        _fx_ram_driver_io_error_request =  ((UINT)rand() % 5);
        if (_fx_ram_driver_io_error_request == 0)
            _fx_ram_driver_io_error_request = 1;
        _fx_utility_FAT_entry_read(&ram_disk, fat_entry, &temp);
        _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
        _fx_utility_FAT_entry_write(&ram_disk, fat_entry, fat_entry+1);
        _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);

        /* Move to next FAT entry.  */
        fat_entry++;          
        if (fat_entry >= (ram_disk.fx_media_available_clusters-1))
            fat_entry =  4;
    }
    for (i = 0; i < 500000; i++) 
    {

        _fx_ram_driver_io_error_request =  ((UINT)rand() % 5);
        if (_fx_ram_driver_io_error_request == 0)
            _fx_ram_driver_io_error_request = 1;
        _fx_utility_logical_sector_flush(&ram_disk, 1, 60000, FX_TRUE);
        _fx_utility_FAT_entry_write(&ram_disk, fat_entry, fat_entry+1);

        /* Move to next FAT entry.  */
        fat_entry++;          
        if (fat_entry >= (ram_disk.fx_media_available_clusters-1))
            fat_entry =  4;
    }

    _fx_ram_driver_io_error_request =  0;

    /* Set the error flag to make logical sector write return an error.  */
    _fx_utility_logical_sector_write_error_request =  1;
    status =  _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_write_error_request =  70000;    /* This should not hit, but satisfy the code coverage.  */ 
    _fx_utility_FAT_flush(&ram_disk);
    _fx_utility_logical_sector_write_error_request =  0;
    
    /* Close the media.  */
    status = fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

#ifdef FX_FAULT_TOLERANT
    if (first_test)
    {
        first_test = 0;
        goto TEST_START;
    }
#endif
    printf("SUCCESS!\n");
    test_control_return(0);
}


#else
#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_media_read_write_sector_application_define(void *first_unused_memory)
#endif
{

    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   Media read/write sector test...........................N/A\n");

    test_control_return(255);
}
#endif

