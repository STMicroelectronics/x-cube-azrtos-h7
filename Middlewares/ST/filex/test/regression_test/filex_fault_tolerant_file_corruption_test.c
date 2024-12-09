/* This test case is design to reproduce file corruption bug.
 * https://expresslogic.zendesk.com/agent/tickets/1252 */
#include <stdio.h>
#include "fx_api.h"
#include "fx_fault_tolerant.h"
#include "fx_ram_driver_test.h"
extern void         test_control_return(UINT status);
void    filex_fault_tolerant_file_corruption_test_application_define(void *first_unused_memory);

#if (defined(FX_ENABLE_FAULT_TOLERANT) &&  !defined(FX_STANDALONE_ENABLE))
/*#define DEBUG*/

#define  MEDIA_MEMORY_SIZE      4096
#define  NO_OF_FATS             1U
#define  DIRECTORY_ENTRIES      1024U
#define  HIDDEN_SECTORS         0
/* The TOTAL_SECTORS is defined to 12800 in the ticket which the media is formatted to FAT16. */
#define  TOTAL_SECTORS          128000U
#define  SECTOR_SIZE            2048U
#define  SECTORS_PER_CLUSTER    1U
#define  HEADS                  1U
#define  SECTORS_PER_TRACK      1U
#define  NO_OF_FILES            17

extern void         _fx_ram_driver(FX_MEDIA *media_ptr);
extern UINT         (*driver_write_callback)(FX_MEDIA *media_ptr,
                                             UINT sector_type,
                                             UCHAR *block_ptr,
                                             UINT *operation_ptr);
extern VOID (*driver_fault_tolerant_enable_callback)(FX_MEDIA *media_ptr, 
                                                     UCHAR *fault_tolerant_memory_buffer,
                                                     ULONG log_size);
static UINT         my_driver_write_callback(FX_MEDIA *media_ptr,
                                             UINT sector_type,
                                             UCHAR *block_ptr,
                                             UINT *operation_ptr);
static VOID         my_fault_tolerant_enable_callback(FX_MEDIA *media_ptr, 
                                                      UCHAR *fault_tolerant_memory_buffer,
                                                      ULONG log_size);
static void         ftest_0_entry(ULONG thread_input);
static void         ftest_1_entry(ULONG thread_input);
static void         restart_thread_0();
static UCHAR       *cache_buffer;
static UCHAR       *fault_tolerant_buffer;
static TX_THREAD    ftest_0;
static TX_THREAD    ftest_1;
static FX_MEDIA     ram_disk;
static UINT         loop;
static UINT         restart;
static UCHAR        file_created[NO_OF_FILES];
static UCHAR        write_buffer[1024 * 1024];
static UCHAR        read_buffer[1024 * 1024];

#ifdef __ghs
extern TX_MUTEX     __ghLockMutex;
#endif
extern FX_MEDIA    *_fx_system_media_opened_ptr;
extern ULONG        _fx_system_media_opened_count;
extern TX_MUTEX    *_tx_mutex_created_ptr;
extern ULONG        _tx_mutex_created_count;

#define RAND                    ((rand() % 4000) + 1)
#define FAULT_TOLERANT_SIZE     FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE
#define DEMO_STACK_SIZE         4096
#ifdef DEBUG
#define TOTAL_LOOP              0xFFFFFFFF
#define WRITE_INTERRUPT_RATE    10000
#else
#define TOTAL_LOOP              100000
#define WRITE_INTERRUPT_RATE    1000000
#endif


static FX_FILE      my_file[NO_OF_FILES];
static UINT FILE_SIZE[NO_OF_FILES] = {1024*5, 1024*2, 1024*5, 1024*50, 1024*1, 1024*40, 1024*5, 1024*10, 1024*24, 1024*50, 1024*16, 1024*12, 1024*5, 1024*1, 1024*32, 1024*9, 1024*19};
static UCHAR local_buffer2[] =  "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_file_corruption_test_application_define(void *first_unused_memory)
#endif
{
UCHAR *pointer;
UINT status;

    loop = 0;
    restart = 0;

    /* Setup the working pointer.  */
    pointer =  (UCHAR *) first_unused_memory;

    /* Create the main thread.  */
    tx_thread_create(&ftest_0, "thread 0", ftest_0_entry, 0,
                     pointer, DEMO_STACK_SIZE,
                     4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer =  pointer + DEMO_STACK_SIZE;

    tx_thread_create(&ftest_1, "thread 1", ftest_1_entry, 0,
                     pointer, DEMO_STACK_SIZE,
                     3, 3, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer =  pointer + DEMO_STACK_SIZE;

    /* Setup memory for the RAM disk and the sector cache.  */
    cache_buffer =  pointer;
    pointer += MEDIA_MEMORY_SIZE;
    fault_tolerant_buffer = pointer;
    pointer += FAULT_TOLERANT_SIZE;

    /* Initialize the FileX system.  */
    fx_system_initialize();

    printf("FileX Test:   Fault Tolerant File Corruption Test....................");

    status =  fx_media_format(&ram_disk,
                             _fx_ram_driver,         // Driver entry
                             ram_disk_memory_large,  // RAM disk memory pointer
                             cache_buffer,           // Media buffer pointer
                             MEDIA_MEMORY_SIZE,      // Media buffer size
                             "MY_RAM_DISK",          // Volume Name
                             NO_OF_FATS,             // Number of FATs
                             DIRECTORY_ENTRIES,      // Directory Entries
                             HIDDEN_SECTORS,         // Hidden sectors
                             TOTAL_SECTORS,          // Total sectors
                             SECTOR_SIZE,            // Sector size
                             SECTORS_PER_CLUSTER,    // Sectors per cluster
                             HEADS,                  // Heads
                             SECTORS_PER_TRACK);     // Sectors per track
    return_if_fail(status == FX_SUCCESS);
}

static void         ftest_0_entry(ULONG thread_input)
{
CHAR        fileName[16];
LONG        actual = 0;
UINT        status;
UINT        i;
UINT        itr;
UCHAR       first_boot = FX_TRUE;
#ifndef DEBUG
LONG        start = (LONG)time(NULL);
#endif /* DEBUG */

    FX_PARAMETER_NOT_USED(thread_input);

    memset(&ram_disk, 0x0, sizeof(ram_disk));
    memset(cache_buffer, 0x0, MEDIA_MEMORY_SIZE);
    driver_write_callback = FX_NULL;

    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory_large,
                            cache_buffer, MEDIA_MEMORY_SIZE);
    return_if_fail(status == FX_SUCCESS);

    driver_fault_tolerant_enable_callback = my_fault_tolerant_enable_callback;

    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail(status == FX_SUCCESS);

#ifdef DEBUG
    printf("\nRestart at %u, LOOP = %u\n", restart, loop);
#endif /* DEBUG */

    for (;loop < TOTAL_LOOP; loop++)
    {

#ifndef DEBUG
        /* Break if the test runs more than 60 seconds.
         * In case the CPU is busy, TOTAL_LOOP maybe too large for the test. */
        if ((LONG)time(NULL) - start > 60)
        {
            break;
        }
#endif /* DEBUG */

        for(i = 0; i < NO_OF_FILES; i++)
        {
            sprintf(fileName, "TEST%d.TXT", i);
            status =  fx_file_open(&ram_disk, &my_file[i], fileName, FX_OPEN_FOR_READ);

            if (!file_created[i])
            {
                return_if_fail(status == FX_NOT_FOUND);
            }
            else
            {
                return_if_fail(status == FX_SUCCESS);

                memset(read_buffer, 0x0, FILE_SIZE[i]);

                for (itr = 0; itr < (FILE_SIZE[i] / 32); itr++)
                {
                    memcpy(&write_buffer[itr * 32], local_buffer2, 32);
                }

                status = fx_file_read(&my_file[i], read_buffer, FILE_SIZE[i], (ULONG *)&actual);
                if(status == FX_SUCCESS)
                {
                    if (memcmp(read_buffer, write_buffer, FILE_SIZE[i]))
                    {

                        /* Only one file could be not written after boot up. */
                        return_if_fail(first_boot == FX_TRUE);
                        first_boot = FX_FALSE;
                    }
                }
                else
                {

                    /* Only one file could be empty after boot up. */
                    return_if_fail(first_boot == FX_TRUE);
                    first_boot = FX_FALSE;
                }

                status =  fx_file_close(&my_file[i]);
                return_if_fail(status == FX_SUCCESS);
            }
        }

        for(i = 0; i < NO_OF_FILES; i++)
        {

            sprintf(fileName, "TEST%d.TXT", i);
            status =  fx_file_open(&ram_disk, &my_file[i], fileName, FX_OPEN_FOR_WRITE);
            if (!file_created[i])
            {
                return_if_fail(status == FX_NOT_FOUND);

                /* File has not been created since format. */
                status = fx_file_create(&ram_disk, fileName);
                return_if_fail(status == FX_SUCCESS);
                file_created[i] = FX_TRUE;

                status = fx_file_open(&ram_disk, &my_file[i], fileName, FX_OPEN_FOR_WRITE);
                return_if_fail(status == FX_SUCCESS);
            }
            else
            {
                status = fx_file_truncate_release(&my_file[i], 0);
                return_if_fail(status == FX_SUCCESS);
            }


            for (itr = 0; itr < (FILE_SIZE[i] / 32); itr++)
            {
                memcpy(&write_buffer[itr * 32], local_buffer2, 32);
            }

            fx_file_seek(&my_file[i], 0);
            status =  fx_file_write(&my_file[i], write_buffer, FILE_SIZE[i]);
            return_if_fail(status == FX_SUCCESS);

            status =  fx_file_close(&my_file[i]);
            return_if_fail(status == FX_SUCCESS);
        }

        driver_write_callback = my_driver_write_callback;
    }

    printf("SUCCESS!\n");
    test_control_return(0);
}

static void         ftest_1_entry(ULONG thread_input)
{

UINT ticks;

    FX_PARAMETER_NOT_USED(thread_input);

    for (;;)
    {

        ticks = ((tx_time_get() % (ULONG)(RAND)) + 10);
#ifdef DEBUG
        printf("\nSleep %u ticks\n", ticks);
#endif /* DEBUG */
        tx_thread_sleep(ticks);

        restart_thread_0();
    }
}

static void         restart_thread_0()
{
TX_MUTEX *mutex_ptr;

    tx_thread_terminate(&ftest_0);
    tx_thread_reset(&ftest_0);

    _fx_system_media_opened_ptr = FX_NULL;
    _fx_system_media_opened_count = 0;

    /* Delete all mutexes (except for system mutex).  */
    while(_tx_mutex_created_ptr)
    {

        /* Setup working mutex pointer.  */
        mutex_ptr =  _tx_mutex_created_ptr;

#ifdef __ghs

        /* Determine if the mutex is the GHS system mutex.  If so, don't delete!  */
        if (mutex_ptr == &__ghLockMutex)
        {

            /* Move to next mutex.  */
            mutex_ptr =  mutex_ptr -> tx_mutex_created_next;
        }

        /* Determine if there are no more mutexes to delete.  */
        if (_tx_mutex_created_count == 1)
            break;
#endif

        /* Delete mutex.  */
        tx_mutex_delete(mutex_ptr);
    }

    restart++;
    tx_thread_resume(&ftest_0);
}

static UINT         my_driver_write_callback(FX_MEDIA *media_ptr,
                                             UINT sector_type,
                                             UCHAR *block_ptr,
                                             UINT *operation_ptr)
{

    FX_PARAMETER_NOT_USED(media_ptr);
    FX_PARAMETER_NOT_USED(sector_type);
    FX_PARAMETER_NOT_USED(block_ptr);
    FX_PARAMETER_NOT_USED(operation_ptr);

    if ((rand() % WRITE_INTERRUPT_RATE) < 1)
    {
#ifdef DEBUG
        printf("\nWrite interrupted\n");
#endif /* DEBUG */

        /* Restart test by waking up thread 1. */
        tx_thread_wait_abort(&ftest_1);
        tx_thread_suspend(&ftest_0);
    }
    return(FX_SUCCESS);
}

static VOID         my_fault_tolerant_enable_callback(FX_MEDIA *media_ptr, 
                                                      UCHAR *fault_tolerant_memory_buffer,
                                                      ULONG log_size)
{

    FX_PARAMETER_NOT_USED(media_ptr);
    FX_PARAMETER_NOT_USED(fault_tolerant_memory_buffer);
    FX_PARAMETER_NOT_USED(log_size);

#ifdef DEBUG
    if (log_size != (FX_FAULT_TOLERANT_LOG_HEADER_SIZE + FX_FAULT_TOLERANT_FAT_CHAIN_SIZE))
    {
        return;
    }

    if (_fx_utility_32_unsigned_read(fault_tolerant_memory_buffer + 16) == media_ptr -> fx_media_fat_last)
    {
        printf("\nInvalid insertion_front detected.\n");
    }
#endif /* DEBUG */
}

#else

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_fault_tolerant_file_corruption_test_application_define(void *first_unused_memory)
#endif
{

    FX_PARAMETER_NOT_USED(first_unused_memory);

    /* Print out some test information banners.  */
    printf("FileX Test:   Fault Tolerant File Corruption Test....................N/A\n");

    test_control_return(255);
}
#endif

