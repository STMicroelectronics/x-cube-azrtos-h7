/* This is the test control routine for the FileX FAT file system.  All tests are dispatched from this routine.  */

#ifndef FX_STANDALONE_ENABLE
#include "tx_api.h"
#else
#include   <pthread.h>
#endif
#include "fx_api.h"
#include "fx_file.h"
#include "fx_directory.h"
#include "fx_fault_tolerant.h"
#include "fx_media.h"
#include "fx_system.h"
#include "fx_unicode.h"
#include <stdio.h>
#include <stdlib.h>

#if defined(EXPECTED_MAJOR_VERSION) && (!defined(FILEX_MAJOR_VERSION) || FILEX_MAJOR_VERSION != EXPECTED_MAJOR_VERSION)
#error "FILEX_MAJOR_VERSION"
#endif /* Check FILEX_MAJOR_VERSION */

#if defined(EXPECTED_MINOR_VERSION) && (!defined(FILEX_MINOR_VERSION) || FILEX_MINOR_VERSION != EXPECTED_MINOR_VERSION)
#error "FILEX_MINOR_VERSION"
#endif /* Check FILEX_MINOR_VERSION */

#define TEST_STACK_SIZE         8192 * 2

#ifndef TEST_TIMEOUT_BASE
#define TEST_TIMEOUT_BASE       1
#endif

/* 1 minute. */
#define TEST_TIMEOUT_LOW        (6000 * TEST_TIMEOUT_BASE)
/* 15 minutes. */
#define TEST_TIMEOUT_MID        (90000 * TEST_TIMEOUT_BASE)
/* 60 minutes. */
#define TEST_TIMEOUT_HIGH       (360000 * TEST_TIMEOUT_BASE * 12)
#ifdef FX_ENABLE_FAULT_TOLERANT
/* buffer for fault tolerant */
#define FAULT_TOLERANT_SIZE     FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE
#else
#define     FAULT_TOLERANT_SIZE     0
#endif

/* Define the test control ThreadX objects...  */
#ifndef FX_STANDALONE_ENABLE
TX_THREAD       test_control_thread;
#ifndef TEST_TIMEOUT_DISABLE
TX_SEMAPHORE    test_control_sema;
UCHAR           test_control_thread_stack[TEST_STACK_SIZE];
#endif
#endif

#ifdef FX_STANDALONE_ENABLE
void    tx_application_define(void *first_unused_memory);
#define tx_kernel_enter()       tx_application_define(0);
#endif
/* Define the test control global variables.   */

ULONG           test_control_return_status;
ULONG           test_control_successful_tests;
ULONG           test_control_failed_tests;
ULONG           test_control_na_tests;

/* Remember the start of free memory.  */

//static ULONG    test_memory_space[1024 * 1024];
ULONG            test_memory_space[10000000];
UCHAR           *test_free_memory_ptr = (UCHAR*)test_memory_space;

#ifndef FX_STANDALONE_ENABLE
extern volatile UINT       _tx_thread_preempt_disable;
extern volatile ULONG      _tx_thread_system_state;
extern TX_THREAD          *_tx_thread_current_ptr;
extern TX_THREAD           _tx_timer_thread;
#endif

/* Define test entry pointer type.  */

typedef  struct TEST_ENTRY_STRUCT
{
VOID        (*test_entry)(void *);
UINT        timeout;
} TEST_ENTRY;

/* Define the prototypes for the test entry points.  */
void    filex_fault_tolerant_corrupted_log_file_test_application_define(void *);
void    filex_fault_tolerant_enable_test_application_define(void *);
void    filex_fault_tolerant_enable_1_test_application_define(void *first_unused_memory);
void    filex_fault_tolerant_enable_2_test_application_define(void *first_unused_memory);
void    filex_fault_tolerant_enable_3_test_application_define(void *first_unused_memory);
void    filex_fault_tolerant_enable_4_test_application_define(void *first_unused_memory);
void    filex_fault_tolerant_media_check_application_define(void *);
void    filex_fault_tolerant_media_full_test_application_define(void *);      
void    filex_fault_tolerant_loop_write_data_test_application_define(void *);    
void    filex_fault_tolerant_file_seek_test_application_define(void *); 
void    filex_fault_tolerant_file_random_seek_test_application_define(void *); 
void    filex_fault_tolerant_file_write_available_test_application_define(void *);
void    filex_fault_tolerant_file_write_data_interrupt_test_application_define(void *); 
void    filex_fault_tolerant_file_write_fat_interrupt_test_application_define(void *);   
void    filex_fault_tolerant_file_write_directory_interrupt_test_application_define(void *);   
void    filex_fault_tolerant_file_write_undo_log_interrupt_test_application_define(void *);  
void    filex_fault_tolerant_file_write_redo_log_interrupt_test_application_define(void *);    
void    filex_fault_tolerant_file_write_fat_crossover_test_application_define(void *);
void    filex_fault_tolerant_file_write_fat_crossover_2_test_application_define(void *first_unused_memory);
void    filex_fault_tolerant_file_create_directory_interrupt_test_application_define(void *); 
void    filex_fault_tolerant_file_create_redo_log_interrupt_test_application_define(void *);        
void    filex_fault_tolerant_file_delete_fat_interrupt_test_application_define(void *);   
void    filex_fault_tolerant_file_delete_fat_multple_sectors_test_application_define(void *); 
void    filex_fault_tolerant_file_delete_directory_interrupt_test_application_define(void *);    
void    filex_fault_tolerant_file_delete_test_application_define(void *);
void    filex_fault_tolerant_file_delete_undo_log_interrupt_test_application_define(void *);        
void    filex_fault_tolerant_file_delete_redo_log_interrupt_test_application_define(void *);    
void    filex_fault_tolerant_file_rename_directory_interrupt_test_application_define(void *); 
void    filex_fault_tolerant_file_rename_redo_log_interrupt_test_application_define(void *);     
void    filex_fault_tolerant_file_allocate_fat_crossover_test_application_define(void *first_unused_memory);
void    filex_fault_tolerant_file_best_effort_allocate_fat_crossover_test_application_define(void *first_unused_memory);
void    filex_fault_tolerant_file_allocate_fat_interrupt_test_application_define(void *);  
void    filex_fault_tolerant_file_allocate_directory_interrupt_test_application_define(void *);  
void    filex_fault_tolerant_file_allocate_undo_log_interrupt_test_application_define(void *);  
void    filex_fault_tolerant_file_allocate_redo_log_interrupt_test_application_define(void *);   
void    filex_fault_tolerant_file_allocate_test_application_define(void *);   
void    filex_fault_tolerant_file_best_allocate_fat_interrupt_test_application_define(void *);  
void    filex_fault_tolerant_file_best_allocate_directory_interrupt_test_application_define(void *);   
void    filex_fault_tolerant_file_best_allocate_undo_log_interrupt_test_application_define(void *);  
void    filex_fault_tolerant_file_best_allocate_redo_log_interrupt_test_application_define(void *);  
void    filex_fault_tolerant_file_truncate_release_available_test_application_define(void *first_unused_memory);
void    filex_fault_tolerant_file_truncate_directory_interrupt_test_application_define(void *);  
void    filex_fault_tolerant_file_truncate_redo_log_interrupt_test_application_define(void *);   
void    filex_fault_tolerant_file_truncate_release_fat_interrupt_test_application_define(void *);   
void    filex_fault_tolerant_file_truncate_release_directory_interrupt_test_application_define(void *);  
void    filex_fault_tolerant_file_truncate_release_undo_log_interrupt_test_application_define(void *);   
void    filex_fault_tolerant_file_truncate_release_redo_log_interrupt_test_application_define(void *); 
void    filex_fault_tolerant_file_attributes_set_directory_interrupt_test_application_define(void *);   
void    filex_fault_tolerant_file_attributes_set_redo_log_interrupt_test_application_define(void *); 
void    filex_fault_tolerant_file_corruption_test_application_define(void *first_unused_memory);
void    filex_fault_tolerant_directory_create_directory_interrupt_test_application_define(void *); 
void    filex_fault_tolerant_directory_create_redo_log_interrupt_test_application_define(void *);  
void    filex_fault_tolerant_directory_delete_directory_interrupt_test_application_define(void *); 
void    filex_fault_tolerant_directory_delete_redo_log_interrupt_test_application_define(void *);   
void    filex_fault_tolerant_directory_rename_directory_interrupt_test_application_define(void *); 
void    filex_fault_tolerant_directory_rename_redo_log_interrupt_test_application_define(void *);    
void    filex_fault_tolerant_directory_attributes_set_directory_interrupt_test_application_define(void *);   
void    filex_fault_tolerant_directory_attributes_set_redo_log_interrupt_test_application_define(void *);   
void    filex_fault_tolerant_log_recover_fat_test_application_define(void *);    
void    filex_fault_tolerant_log_recover_directory_test_application_define(void *);  
void    filex_fault_tolerant_log_full_test_application_define(void *);  
void    filex_fault_tolerant_log_expand_test_application_define(void *);       
void    filex_fault_tolerant_log_expand_interrupt_test_application_define(void *); 
void    filex_fault_tolerant_long_directory_create_directory_interrupt_test_application_define(void *first_unused_memory);   
void    filex_fault_tolerant_long_directory_create_redo_log_interrupt_test_application_define(void *first_unused_memory);  
void    filex_fault_tolerant_long_directory_rename_directory_interrupt_test_application_define(void *first_unused_memory);   
void    filex_fault_tolerant_long_directory_rename_redo_log_interrupt_test_application_define(void *first_unused_memory);  
void    filex_fault_tolerant_long_file_create_directory_interrupt_test_application_define(void *first_unused_memory);   
void    filex_fault_tolerant_long_file_create_redo_log_interrupt_test_application_define(void *first_unused_memory);  
void    filex_fault_tolerant_long_directory_rename_io_error_test_application_define(void *first_unused_memory);
void    filex_fault_tolerant_write_large_data_test_application_define(void *);      
void    filex_fault_tolerant_write_large_data_interrupt_test_application_define(void *);
void    filex_fault_tolerant_write_large_data_fat_chain_cleanup_interrupt_test_application_define(void *);   
void    filex_fault_tolerant_write_large_data_fat_interrupt_test_application_define(void *); 
void    filex_fault_tolerant_write_large_data_directory_interrupt_test_application_define(void *); 
void    filex_fault_tolerant_write_large_data_undo_log_interrupt_test_application_define(void *);    
void    filex_fault_tolerant_write_large_data_redo_log_interrupt_test_application_define(void *);   
void    filex_fault_tolerant_delete_large_data_test_application_define(void *);                             
void    filex_fault_tolerant_recover_fail_test_application_define(void *first_unused_memory);
void    filex_fault_tolerant_unicode_directory_create_directory_interrupt_test_application_define(void *); 
void    filex_fault_tolerant_unicode_directory_create_redo_log_interrupt_test_application_define(void *);   
void    filex_fault_tolerant_unicode_directory_rename_directory_interrupt_test_application_define(void *); 
void    filex_fault_tolerant_unicode_directory_rename_redo_log_interrupt_test_application_define(void *);
void    filex_fault_tolerant_unicode_file_create_directory_interrupt_test_application_define(void *); 
void    filex_fault_tolerant_unicode_file_create_redo_log_interrupt_test_application_define(void *); 
void    filex_fault_tolerant_unicode_file_rename_directory_interrupt_test_application_define(void *);
void    filex_fault_tolerant_unicode_file_rename_redo_log_interrupt_test_application_define(void *); 
void    filex_media_format_open_close_application_define(void *);
void    filex_media_multiple_format_open_close_application_define(void *first_unused_memory);
void    filex_media_flush_application_define(void *first_unused_memory);
void    filex_media_abort_application_define(void *first_unused_memory);
void    filex_media_cache_invalidate_application_define(void *first_unused_memory);
void    filex_media_volume_get_set_application_define(void *first_unused_memory);
void    filex_media_read_write_sector_application_define(void *first_unused_memory);
void    filex_media_check_application_define(void *first_unused_memory);
void    filex_media_hidden_sectors_test_application_define(void *first_unused_memory);
void    filex_system_date_time_application_define(void *first_unused_memory);
void    filex_directory_create_delete_application_define(void *first_unused_memory);
void    filex_directory_default_get_set_application_define(void *first_unused_memory);
void    filex_directory_duplicate_entries_application_define(void *first_unused_memory);
void    filex_directory_naming_application_define(void *first_unused_memory);
void    filex_directory_first_next_find_application_define(void *first_unused_memory);
void    filex_directory_local_path_application_define(void *first_unused_memory);
void    filex_directory_rename_application_define(void *first_unused_memory);
void    filex_directory_long_short_get_application_define(void *first_unused_memory);
void    filex_directory_attributes_read_set_application_define(void *first_unused_memory);
void    filex_file_create_delete_application_define(void *first_unused_memory);
void    filex_file_naming_application_define(void *first_unused_memory);
void    filex_file_read_write_application_define(void *first_unused_memory);
void    filex_file_write_seek_application_define(void *first_unused_memory);
void    filex_file_name_application_define(void *first_unused_memory);
void    filex_file_write_notify_application_define(void *first_unused_memory);
void    filex_file_write_available_cluster_application_define(void *first_unused_memory);
void    filex_file_seek_application_define(void *first_unused_memory);
void    filex_file_allocate_truncate_application_define(void *first_unused_memory);
void    filex_file_allocate_application_define(void *first_unused_memory);
void    filex_file_attributes_read_set_application_define(void *first_unused_memory);
void    filex_file_date_time_set_application_define(void *first_unused_memory);
void    filex_file_rename_application_define(void *first_unused_memory);
void    filex_unicode_application_define(void *first_unused_memory);
void    filex_unicode_2_application_define(void *first_unused_memory);
void    filex_unicode_3_application_define(void *first_unused_memory);
void    filex_unicode_4_application_define(void *first_unused_memory);
void    filex_unicode_directory_rename_application_define(void *first_unused_memory);
void    filex_unicode_file_directory_rename_extra_test_application_define(void *first_unused_memory);
void    filex_unicode_file_directory_rename_extra_2_test_application_define(void *first_unused_memory);
void    filex_unicode_file_rename_application_define(void *first_unused_memory);
void    filex_unicode_name_string_application_define(void *first_ununsed_memory);
void    filex_unicode_directory_entry_test_application_define(void *first_unused_memory);
void    filex_unicode_directory_entry_2_test_application_define(void *first_unused_memory);
void    filex_unicode_directory_entry_change_test_application_define(void *first_unused_memory);
void    filex_unicode_fat_entry_test_application_define(void *first_unused_memory);
void    filex_unicode_fat_entry_1_test_application_define(void *first_unused_memory);
void    filex_unicode_fat_entry_2_test_application_define(void *first_unused_memory);
void    filex_unicode_fat_entry_3_test_application_define(void *first_unused_memory);
void    filex_media_format_open_close_exfat_application_define(void *first_unused_memory);
void    filex_media_close_with_file_opened_exfat_application_define(void *first_unused_memory);
void    filex_media_flush_with_file_opened_exfat_application_define(void *first_unused_memory);
void    filex_media_multiple_format_open_close_exfat_application_define(void *first_unused_memory);
void    filex_media_flush_exfat_application_define(void *first_unused_memory);
void    filex_media_abort_exfat_application_define(void *first_unused_memory);
void    filex_media_cache_invalidate_exfat_application_define(void *first_unused_memory);
void    filex_media_volume_directory_entry_application_define(void *first_unused_memory);
void    filex_media_volume_get_set_exfat_application_define(void *first_unused_memory);
void    filex_media_read_write_sector_exfat_application_define(void *first_unused_memory);
void    filex_media_check_exfat_application_define(void *first_unused_memory);
void    filex_directory_create_delete_exfat_application_define(void *first_unused_memory);
void    filex_directory_default_get_set_exfat_application_define(void *first_unused_memory);
void    filex_directory_naming_exfat_application_define(void *first_unused_memory);
void    filex_directory_first_next_find_exfat_application_define(void *first_unused_memory);
void    filex_directory_local_path_exfat_application_define(void *first_unused_memory);
void    filex_directory_rename_exfat_application_define(void *first_unused_memory);
void    filex_directory_attributes_read_set_exfat_application_define(void *first_unused_memory);
void    filex_file_create_delete_exfat_application_define(void *first_unused_memory);
void    filex_file_naming_exfat_application_define(void *first_unused_memory);
void    filex_file_read_write_exfat_application_define(void *first_unused_memory);
void    filex_file_seek_exfat_application_define(void *first_unused_memory);
void    filex_file_allocate_truncate_exfat_application_define(void *first_unused_memory);
void    filex_file_attributes_read_set_exfat_application_define(void *first_unused_memory);
void    filex_file_date_time_set_exfat_application_define(void *first_unused_memory);
void    filex_file_rename_exfat_application_define(void *first_unused_memory);
void    filex_utility_application_define(void *first_unused_memory);
void    filex_utility_fat_flush_application_define(void *first_unused_memory);
void    filex_bitmap_flush_exfat_application_define(void *first_unused_memory);
void    test_application_define(void *first_unused_memory);


/* Define the array of test entry points.  */

TEST_ENTRY  test_control_tests[] = 
{                                  

#ifdef CTEST
    {test_application_define, TEST_TIMEOUT_HIGH},
#else /* CTEST */
    {filex_fault_tolerant_corrupted_log_file_test_application_define, TEST_TIMEOUT_LOW},
    {filex_fault_tolerant_enable_test_application_define, TEST_TIMEOUT_LOW},
    {filex_fault_tolerant_enable_1_test_application_define, TEST_TIMEOUT_LOW},
    {filex_fault_tolerant_enable_2_test_application_define, TEST_TIMEOUT_LOW},
    {filex_fault_tolerant_enable_3_test_application_define, TEST_TIMEOUT_LOW},
    {filex_fault_tolerant_enable_4_test_application_define, TEST_TIMEOUT_LOW},
    {filex_fault_tolerant_media_check_application_define, TEST_TIMEOUT_LOW},
    //{filex_fault_tolerant_media_full_test_application_define, TEST_TIMEOUT_HIGH},    
    {filex_fault_tolerant_loop_write_data_test_application_define, TEST_TIMEOUT_HIGH},        
    {filex_fault_tolerant_file_seek_test_application_define, TEST_TIMEOUT_LOW},
    {filex_fault_tolerant_file_random_seek_test_application_define, TEST_TIMEOUT_MID},
    {filex_fault_tolerant_file_write_data_interrupt_test_application_define, TEST_TIMEOUT_LOW},
    {filex_fault_tolerant_file_write_available_test_application_define, TEST_TIMEOUT_LOW},
    {filex_fault_tolerant_file_write_fat_interrupt_test_application_define, TEST_TIMEOUT_LOW},   
    {filex_fault_tolerant_file_write_directory_interrupt_test_application_define, TEST_TIMEOUT_LOW},  
    {filex_fault_tolerant_file_write_undo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW},   
    {filex_fault_tolerant_file_write_redo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW},   
    {filex_fault_tolerant_file_write_fat_crossover_test_application_define, TEST_TIMEOUT_LOW},   
    {filex_fault_tolerant_file_write_fat_crossover_2_test_application_define, TEST_TIMEOUT_LOW},
    {filex_fault_tolerant_file_allocate_fat_crossover_test_application_define, TEST_TIMEOUT_LOW},
    {filex_fault_tolerant_file_best_effort_allocate_fat_crossover_test_application_define, TEST_TIMEOUT_LOW},
    {filex_fault_tolerant_file_create_directory_interrupt_test_application_define, TEST_TIMEOUT_LOW},  
    {filex_fault_tolerant_file_create_redo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW},
    {filex_fault_tolerant_file_delete_fat_interrupt_test_application_define, TEST_TIMEOUT_LOW},   
    {filex_fault_tolerant_file_delete_fat_multple_sectors_test_application_define, TEST_TIMEOUT_LOW},   
    {filex_fault_tolerant_file_delete_directory_interrupt_test_application_define, TEST_TIMEOUT_LOW},  
    {filex_fault_tolerant_file_delete_test_application_define,TEST_TIMEOUT_LOW },
    {filex_fault_tolerant_file_delete_undo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW},  
    {filex_fault_tolerant_file_delete_redo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW}, 
    {filex_fault_tolerant_file_rename_directory_interrupt_test_application_define, TEST_TIMEOUT_LOW}, 
    {filex_fault_tolerant_file_rename_redo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW}, 
    {filex_fault_tolerant_file_allocate_fat_interrupt_test_application_define, TEST_TIMEOUT_LOW},    
    {filex_fault_tolerant_file_allocate_directory_interrupt_test_application_define, TEST_TIMEOUT_LOW},     
    {filex_fault_tolerant_file_allocate_undo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW},   
    {filex_fault_tolerant_file_allocate_redo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW}, 
    {filex_fault_tolerant_file_allocate_test_application_define, TEST_TIMEOUT_LOW}, 
    {filex_fault_tolerant_file_best_allocate_fat_interrupt_test_application_define, TEST_TIMEOUT_LOW},    
    {filex_fault_tolerant_file_best_allocate_directory_interrupt_test_application_define, TEST_TIMEOUT_LOW},   
    {filex_fault_tolerant_file_best_allocate_undo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW},    
    {filex_fault_tolerant_file_best_allocate_redo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW},  
    {filex_fault_tolerant_file_truncate_release_available_test_application_define, TEST_TIMEOUT_LOW},    
    {filex_fault_tolerant_file_truncate_directory_interrupt_test_application_define, TEST_TIMEOUT_LOW},    
    {filex_fault_tolerant_file_truncate_redo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW}, 
    {filex_fault_tolerant_file_truncate_release_fat_interrupt_test_application_define, TEST_TIMEOUT_LOW},    
    {filex_fault_tolerant_file_truncate_release_directory_interrupt_test_application_define, TEST_TIMEOUT_LOW},    
    {filex_fault_tolerant_file_truncate_release_undo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW},     
    {filex_fault_tolerant_file_truncate_release_redo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW}, 
    {filex_fault_tolerant_file_attributes_set_directory_interrupt_test_application_define, TEST_TIMEOUT_LOW},   
    {filex_fault_tolerant_file_attributes_set_redo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW},  
    {filex_fault_tolerant_file_corruption_test_application_define, TEST_TIMEOUT_HIGH},
    {filex_fault_tolerant_directory_create_directory_interrupt_test_application_define, TEST_TIMEOUT_LOW},    
    {filex_fault_tolerant_directory_create_redo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW},   
    {filex_fault_tolerant_directory_delete_directory_interrupt_test_application_define, TEST_TIMEOUT_LOW},    
    {filex_fault_tolerant_directory_delete_redo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW},   
    {filex_fault_tolerant_directory_rename_directory_interrupt_test_application_define, TEST_TIMEOUT_LOW},    
    {filex_fault_tolerant_directory_rename_redo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW},       
    {filex_fault_tolerant_directory_attributes_set_directory_interrupt_test_application_define, TEST_TIMEOUT_LOW},   
    {filex_fault_tolerant_directory_attributes_set_redo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW}, 
    {filex_fault_tolerant_log_recover_fat_test_application_define, TEST_TIMEOUT_LOW},        
    {filex_fault_tolerant_log_recover_directory_test_application_define, TEST_TIMEOUT_LOW},   
    {filex_fault_tolerant_log_full_test_application_define, TEST_TIMEOUT_LOW},   
    {filex_fault_tolerant_long_directory_create_directory_interrupt_test_application_define, TEST_TIMEOUT_HIGH},                  
    {filex_fault_tolerant_long_directory_create_redo_log_interrupt_test_application_define, TEST_TIMEOUT_HIGH},  
#if 1
    {filex_fault_tolerant_long_directory_rename_directory_interrupt_test_application_define, TEST_TIMEOUT_HIGH},                  
    {filex_fault_tolerant_long_directory_rename_redo_log_interrupt_test_application_define, TEST_TIMEOUT_HIGH},  
#endif
    {filex_fault_tolerant_long_file_create_directory_interrupt_test_application_define, TEST_TIMEOUT_HIGH},                  
    {filex_fault_tolerant_long_file_create_redo_log_interrupt_test_application_define, TEST_TIMEOUT_HIGH}, 
    {filex_fault_tolerant_long_directory_rename_io_error_test_application_define, TEST_TIMEOUT_HIGH}, 
    {filex_fault_tolerant_recover_fail_test_application_define, TEST_TIMEOUT_LOW},
    {filex_fault_tolerant_write_large_data_test_application_define, TEST_TIMEOUT_HIGH},      
    {filex_fault_tolerant_write_large_data_interrupt_test_application_define, TEST_TIMEOUT_HIGH},    
    {filex_fault_tolerant_write_large_data_fat_chain_cleanup_interrupt_test_application_define, TEST_TIMEOUT_HIGH},  
    {filex_fault_tolerant_write_large_data_fat_interrupt_test_application_define, TEST_TIMEOUT_HIGH},   
    {filex_fault_tolerant_write_large_data_directory_interrupt_test_application_define, TEST_TIMEOUT_HIGH},   
    {filex_fault_tolerant_write_large_data_undo_log_interrupt_test_application_define, TEST_TIMEOUT_HIGH}, 
    {filex_fault_tolerant_write_large_data_redo_log_interrupt_test_application_define, TEST_TIMEOUT_HIGH},  
    {filex_fault_tolerant_delete_large_data_test_application_define, TEST_TIMEOUT_HIGH},                                    
    {filex_fault_tolerant_unicode_directory_create_directory_interrupt_test_application_define, TEST_TIMEOUT_LOW},          
    {filex_fault_tolerant_unicode_directory_create_redo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW},           
    {filex_fault_tolerant_unicode_directory_rename_directory_interrupt_test_application_define, TEST_TIMEOUT_LOW},          
    {filex_fault_tolerant_unicode_directory_rename_redo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW},             
    {filex_fault_tolerant_unicode_file_create_directory_interrupt_test_application_define, TEST_TIMEOUT_LOW},             
    {filex_fault_tolerant_unicode_file_create_redo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW},             
    {filex_fault_tolerant_unicode_file_rename_directory_interrupt_test_application_define, TEST_TIMEOUT_LOW},            
    {filex_fault_tolerant_unicode_file_rename_redo_log_interrupt_test_application_define, TEST_TIMEOUT_LOW},      
    {filex_media_format_open_close_application_define, TEST_TIMEOUT_LOW},
    {filex_media_multiple_format_open_close_application_define, TEST_TIMEOUT_LOW},
    {filex_media_flush_application_define, TEST_TIMEOUT_LOW},
    {filex_media_abort_application_define, TEST_TIMEOUT_LOW},
    {filex_media_cache_invalidate_application_define, TEST_TIMEOUT_LOW},
    {filex_media_volume_directory_entry_application_define, TEST_TIMEOUT_LOW},
    {filex_media_volume_get_set_application_define, TEST_TIMEOUT_LOW},
    {filex_media_read_write_sector_application_define, TEST_TIMEOUT_LOW},
    {filex_media_check_application_define, TEST_TIMEOUT_LOW},
    {filex_media_hidden_sectors_test_application_define, TEST_TIMEOUT_LOW},
    {filex_system_date_time_application_define, TEST_TIMEOUT_LOW},
    {filex_directory_create_delete_application_define, TEST_TIMEOUT_LOW},
    {filex_directory_default_get_set_application_define, TEST_TIMEOUT_LOW},
    {filex_directory_duplicate_entries_application_define, TEST_TIMEOUT_LOW},
    {filex_directory_naming_application_define, TEST_TIMEOUT_LOW},
    {filex_directory_first_next_find_application_define, TEST_TIMEOUT_LOW},
#if 1
    {filex_directory_local_path_application_define, TEST_TIMEOUT_LOW},
#endif
    {filex_directory_rename_application_define, TEST_TIMEOUT_LOW},
    {filex_directory_long_short_get_application_define, TEST_TIMEOUT_LOW},
    {filex_directory_attributes_read_set_application_define, TEST_TIMEOUT_LOW},
    {filex_file_create_delete_application_define, TEST_TIMEOUT_LOW},
    {filex_file_naming_application_define, TEST_TIMEOUT_LOW},
#if 1
    {filex_file_read_write_application_define, TEST_TIMEOUT_LOW},
    {filex_file_write_seek_application_define, TEST_TIMEOUT_LOW},
    {filex_file_name_application_define, TEST_TIMEOUT_LOW},
    {filex_file_write_notify_application_define, TEST_TIMEOUT_LOW},
    {filex_file_write_available_cluster_application_define, TEST_TIMEOUT_LOW},
    {filex_file_seek_application_define, TEST_TIMEOUT_LOW},
    {filex_file_allocate_truncate_application_define, TEST_TIMEOUT_LOW},
    {filex_file_allocate_application_define, TEST_TIMEOUT_LOW},
#endif
    {filex_file_attributes_read_set_application_define, TEST_TIMEOUT_LOW},
    {filex_file_date_time_set_application_define, TEST_TIMEOUT_LOW},
    {filex_file_rename_application_define, TEST_TIMEOUT_LOW},
    {filex_unicode_application_define, TEST_TIMEOUT_LOW},
    {filex_unicode_2_application_define, TEST_TIMEOUT_LOW},
    {filex_unicode_3_application_define, TEST_TIMEOUT_LOW},
    {filex_unicode_4_application_define, TEST_TIMEOUT_LOW},
    {filex_unicode_directory_entry_test_application_define, TEST_TIMEOUT_LOW},
    {filex_unicode_directory_entry_2_test_application_define, TEST_TIMEOUT_LOW},
    {filex_unicode_directory_entry_change_test_application_define,TEST_TIMEOUT_LOW },
    {filex_unicode_directory_rename_application_define, TEST_TIMEOUT_LOW},
    {filex_unicode_fat_entry_test_application_define, TEST_TIMEOUT_LOW},
    {filex_unicode_fat_entry_1_test_application_define, TEST_TIMEOUT_LOW},
    {filex_unicode_fat_entry_2_test_application_define, TEST_TIMEOUT_LOW},
    {filex_unicode_fat_entry_3_test_application_define, TEST_TIMEOUT_LOW},
    {filex_unicode_file_directory_rename_extra_test_application_define, TEST_TIMEOUT_LOW},
    {filex_unicode_file_directory_rename_extra_2_test_application_define, TEST_TIMEOUT_LOW},
    {filex_unicode_file_rename_application_define, TEST_TIMEOUT_LOW},
    {filex_unicode_name_string_application_define, TEST_TIMEOUT_LOW},
#ifdef FX_ENABLE_EXFAT
    {filex_media_format_open_close_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_media_close_with_file_opened_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_media_flush_with_file_opened_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_media_multiple_format_open_close_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_media_flush_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_media_abort_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_media_cache_invalidate_exfat_application_define, TEST_TIMEOUT_LOW},
#if 1
    {filex_media_volume_get_set_exfat_application_define, TEST_TIMEOUT_LOW},
#endif
    {filex_media_read_write_sector_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_media_check_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_directory_create_delete_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_directory_default_get_set_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_directory_naming_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_directory_first_next_find_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_directory_local_path_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_directory_rename_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_directory_attributes_read_set_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_file_create_delete_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_file_naming_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_file_read_write_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_file_seek_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_file_allocate_truncate_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_file_attributes_read_set_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_file_date_time_set_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_file_rename_exfat_application_define, TEST_TIMEOUT_LOW},
    {filex_bitmap_flush_exfat_application_define, TEST_TIMEOUT_LOW},
#endif /* FX_ENABLE_EXFAT */
    {filex_utility_application_define, TEST_TIMEOUT_LOW},
    {filex_utility_fat_flush_application_define, TEST_TIMEOUT_LOW},
    
#endif /* CTEST */
    {FX_NULL, TEST_TIMEOUT_LOW}
};
/* Define thread prototypes.  */

void  test_control_thread_entry(ULONG thread_input);
void  test_control_return(UINT status);
void  test_control_cleanup(void);


/* Define necessary exernal references.  */
#ifndef FX_STANDALONE_ENABLE
#ifdef __ghs
extern TX_MUTEX                 __ghLockMutex;
#endif

extern TX_TIMER                 *_tx_timer_created_ptr;
extern ULONG                    _tx_timer_created_count;
#ifndef TX_TIMER_PROCESS_IN_ISR
extern TX_THREAD                _tx_timer_thread;
#endif
extern TX_THREAD                *_tx_thread_created_ptr;
extern ULONG                    _tx_thread_created_count;
extern TX_SEMAPHORE             *_tx_semaphore_created_ptr;
extern ULONG                    _tx_semaphore_created_count;
extern TX_QUEUE                 *_tx_queue_created_ptr;
extern ULONG                    _tx_queue_created_count;
extern TX_MUTEX                 *_tx_mutex_created_ptr;
extern ULONG                    _tx_mutex_created_count;
extern TX_EVENT_FLAGS_GROUP     *_tx_event_flags_created_ptr;
extern ULONG                    _tx_event_flags_created_count;
extern TX_BYTE_POOL             *_tx_byte_pool_created_ptr;
extern ULONG                    _tx_byte_pool_created_count;
extern TX_BLOCK_POOL            *_tx_block_pool_created_ptr;
extern ULONG                    _tx_block_pool_created_count;
#endif

extern FX_MEDIA *               _fx_system_media_opened_ptr;
extern ULONG                    _fx_system_media_opened_count;


static FX_MEDIA                 media_instance;
static FX_FILE                  file_instance;
static FX_LOCAL_PATH            local_path_instance;


static void fake_driver(FX_MEDIA *media_ptr)
{
    FX_PARAMETER_NOT_USED(media_ptr);
}

/* Define main entry point.  */

int main()
{              
ULONG       actual;   
              
    /* Print out some test information banners.  */      
    printf("%s\n", _fx_version_id);
    actual =  FX_MAX_LONG_NAME_LEN;
    if (actual < 256)
        printf("**** ERROR *****  FileX and tests must be built with FX_MAX_LONG_NAME_LEN=256\n");
    actual =  FX_MAX_LAST_NAME_LEN;
    if (actual < 256)
        printf("**** ERROR *****  FileX and tests must be built with FX_MAX_LAST_NAME_LEN=256\n");
    
#ifdef FX_FAULT_TOLERANT 
    printf("Immediately write feature is built-in.\n");
#else   
    printf("immediately write feature is not built-in.\n");
#endif
    
#ifdef FX_ENABLE_FAULT_TOLERANT    
    printf("Fault tolerant feature is built-in.\n");
#else      
    printf("Fault tolerant feature is not built-in.\n");
#endif      

    printf("\n");

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}


/* Define what the initial system looks like.  */

void    tx_application_define(void *first_unused_memory)
{

#ifndef FX_STANDALONE_ENABLE
ULONG   saved_system_state;
UINT    status;
ULONG64 available_bytes;
ULONG   temp;
CHAR    *string_ptr;
UINT    attributes;
UCHAR   buffer[100];
UCHAR   unicode_name[] = { 1,0,2,0,3,0,4,0,5,0,6,0,0,0};
#ifdef FX_ENABLE_FAULT_TOLERANT
UCHAR   fault_tolerant_buffer[FAULT_TOLERANT_SIZE];
#endif /* FX_ENABLE_FAULT_TOLERANT */

    FX_PARAMETER_NOT_USED(first_unused_memory);

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Perform initialization, NULL thread, and time thread tests against the error checking shell.  */
    
    /* Check caller error checking for fx_fault_tolerant_enable.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;
    
    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
    
    /* Call the _fxe_media_abort to cause and error.  */
    status =  _fxe_fault_tolerant_enable(&media_instance, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_fault_tolerant_enable!\n");
    }
    
    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_media_abort to cause and error.  */
    status =  _fxe_fault_tolerant_enable(&media_instance, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_fault_tolerant_enable!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_media_abort to cause and error.  */
    status =  _fxe_fault_tolerant_enable(&media_instance, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_fault_tolerant_enable!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Perform initialization, NULL thread, and time thread tests against the error checking shell.  */
    
    /* Check caller error checking for fx_media_abort.  */
    
    /* Save the system state.  */
#ifndef FX_STANDALONE_ENABLE
    saved_system_state =  _tx_thread_system_state;
#endif

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
    
    /* Call the _fxe_media_abort to cause and error.  */
    status =  _fxe_media_abort(&media_instance);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_abort!\n");
    }
    
    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_media_abort to cause and error.  */
    status =  _fxe_media_abort(&media_instance);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_abort!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;

    /* Call the _fxe_media_abort to cause and error.  */
    status =  _fxe_media_abort(&media_instance);

    /* Set the current thread back to NULL.  */

    _tx_thread_current_ptr =  FX_NULL;

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_abort!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;

    /* Check caller error checking for fx_media_invalidate.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;
    
    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
    
    /* Call the _fxe_media_cache_invalidate to cause and error.  */
    status =  _fxe_media_cache_invalidate(&media_instance);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_invalidate!\n");
    }
    
    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_media_cache_invalidate to cause and error.  */
    status =  _fxe_media_cache_invalidate(&media_instance);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_invalidate!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_media_cache_invalidate to cause and error.  */
    status =  _fxe_media_cache_invalidate(&media_instance);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_cache_invalidate!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_media_check.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;
    
    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
    
    /* Call the _fxe_media_check to cause and error.  */
    status =  _fxe_media_check(&media_instance, (UCHAR *) &media_instance, 0, 0, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_check!\n");
    }
    
    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_media_check to cause and error.  */
    status =  _fxe_media_check(&media_instance, (UCHAR *) &media_instance, 0, 0, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_check!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_media_check to cause and error.  */
    status =  _fxe_media_check(&media_instance, (UCHAR *) &media_instance, 0, 0, FX_NULL);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_check!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;

    /* Check caller error checking for fx_media_close.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;
    
    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
    
    /* Call the _fxe_media_close to cause and error.  */
    status =  _fxe_media_close(&media_instance);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_close!\n");
    }
    
    /* Call the _fxe_media_close to cause and error.  */
    status =  _fxe_media_close(&media_instance);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_close!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_media_close to cause and error.  */
    status =  _fxe_media_close(&media_instance);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_close!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_media_close to cause and error.  */
    status =  _fxe_media_close(&media_instance);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_close!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;

    /* Check caller error checking for fx_media_close_notify_set.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;
      
    /* Call the _fxe_media_close_notify_set to cause and error.  */
    status =  _fxe_media_close_notify_set(FX_NULL, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_close_notify_set!\n");
    }
    
    /* Call the _fxe_media_close_notify_set to cause and error.  */
    status =  _fxe_media_close_notify_set(&media_instance, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_close_notify_set!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_media_close_notify_set to cause and error.  */
    status =  _fxe_media_close_notify_set(&media_instance, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_close_notify_set!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_media_close_notify_set to cause and error.  */
    status =  _fxe_media_close_notify_set(&media_instance, FX_NULL);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_close_notify_set!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_media_extended_space_available.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;
    
    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  0;
    
    /* Fake test control thread running.  */
    _tx_thread_system_state =  0;
    _tx_thread_current_ptr =  &test_control_thread;

    /* Call the _fxe_media_extended_space_available to cause and error.  */
    status =  _fxe_media_extended_space_available(&media_instance, &available_bytes);

    _tx_thread_system_state =  saved_system_state;
    _tx_thread_current_ptr =  FX_NULL;

    /* Check for the error.  */
    if (status != FX_MEDIA_NOT_OPEN)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_extended_space_available!\n");
    }

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
    
    /* Call the _fxe_media_extended_space_available to cause and error.  */
    status =  _fxe_media_extended_space_available(FX_NULL, &available_bytes);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_extended_space_available!\n");
    }

    /* Call the _fxe_media_extended_space_available to cause and error.  */
    status =  _fxe_media_extended_space_available(&media_instance, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_extended_space_available!\n");
    }
    
    /* Call the _fxe_media_extended_space_available to cause and error.  */
    status =  _fxe_media_extended_space_available(&media_instance, &available_bytes);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_extended_space_available!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_media_extended_space_available to cause and error.  */
    status =  _fxe_media_extended_space_available(&media_instance, &available_bytes);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_extended_space_available!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_media_extended_space_available to cause and error.  */
    status =  _fxe_media_extended_space_available(&media_instance, &available_bytes);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_extended_space_available!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_media_flush.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;
    
    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
       
    /* Call the _fxe_media_flush to cause and error.  */
    status =  _fxe_media_flush(&media_instance);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_flush!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_media_flush to cause and error.  */
    status =  _fxe_media_flush(&media_instance);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_flush!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_media_flush to cause and error.  */
    status =  _fxe_media_flush(&media_instance);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_flush!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_media_format.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;
    
    /* Call the _fxe_media_format to cause an error.  */
    status =  _fxe_media_format(&media_instance, FX_NULL, FX_NULL, FX_NULL, 0,
                    FX_NULL, 0, 0, 0, 0, 0, 0, 0, 0);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_format!\n");
    }

    /* Call the _fxe_media_format to cause an error.  */
    status =  _fxe_media_format(&media_instance, fake_driver, FX_NULL, FX_NULL, 0,
                    FX_NULL, 0, 0, 0, 0, 0, 0, 0, 0);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_format!\n");
    }
    
    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
       
    /* Call the _fxe_media_format to cause an error.  */
    status =  _fxe_media_format(&media_instance, fake_driver, FX_NULL, buffer, 0,
                    FX_NULL, 0, 0, 0, 0, 0, 0, 0, 0);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_format!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_media_format to cause an error.  */
    status =  _fxe_media_format(&media_instance, fake_driver, FX_NULL, buffer, 0,
                    FX_NULL, 0, 0, 0, 0, 0, 0, 0, 0);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_format!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_media_format to cause an error.  */
    status =  _fxe_media_format(&media_instance, fake_driver, FX_NULL, buffer, 0,
                    FX_NULL, 0, 0, 0, 0, 0, 0, 0, 0);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_format!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;

    /* Check caller error checking for fx_media_open.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;
    
    /* Call the _fxe_media_open to cause an error.  */
    status =  _fxe_media_open(&media_instance, FX_NULL, FX_NULL, FX_NULL,
                      FX_NULL, 0, 0);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_open!\n");
    }


    /* Call the _fxe_media_open to cause an error.  */
    status =  _fxe_media_open(&media_instance, FX_NULL, fake_driver, FX_NULL,
                      FX_NULL, 0, 0);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_open!\n");
    }


    /* Call the _fxe_media_open to cause an error.  */
    status =  _fxe_media_open(&media_instance, FX_NULL, fake_driver, FX_NULL,
                      buffer, 0, 0);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_open!\n");
    }


    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
       
    /* Call the _fxe_media_format to cause an error.  */
    status =  _fxe_media_open(&media_instance, FX_NULL, fake_driver, FX_NULL,
                      buffer, 0, sizeof(FX_MEDIA));
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_open!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_media_open to cause an error.  */
    status =  _fxe_media_open(&media_instance, FX_NULL, fake_driver, FX_NULL,
                      buffer, 0, sizeof(FX_MEDIA));
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_open!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_media_open to cause an error.  */
    status =  _fxe_media_open(&media_instance, FX_NULL, fake_driver, FX_NULL,
                      buffer, 0, sizeof(FX_MEDIA));

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_open!\n");
    }
#endif

    /* Set the current thread to the test control thread, for error checking on the sector cache size.  */
    _tx_thread_current_ptr =  &test_control_thread;
    
    /* Save the actual sector cache size.  */
    temp =  _fx_system_media_max_sector_cache;
    _fx_system_media_max_sector_cache =  3;
    
    /* Call the _fxe_media_open to cause an error.  */
    status =  _fxe_media_open(&media_instance, FX_NULL, fake_driver, FX_NULL,
                      buffer, 0, sizeof(FX_MEDIA));

    /* Restore the max sector cache size.  */
    _fx_system_media_max_sector_cache =  temp;

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_MEDIA_INVALID)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_open!\n");
    }

    /* Set the current thread to the test control thread, for error checking on the sector cache size.  */
    _tx_thread_current_ptr =  &test_control_thread;
    
    /* Save the actual sector cache size.  */
    temp =  _fx_system_media_max_sector_cache;
    _fx_system_media_max_sector_cache =  6;
    
    /* Call the _fxe_media_open to cause an error.  */
    status =  _fxe_media_open(&media_instance, FX_NULL, fake_driver, FX_NULL,
                      buffer, 0, sizeof(FX_MEDIA));

    /* Restore the max sector cache size.  */
    _fx_system_media_max_sector_cache =  temp;

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_MEDIA_INVALID)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_open!\n");
    }

    /* Set the current thread to the test control thread, for error checking on the sector cache size.  */
    _tx_thread_current_ptr =  &test_control_thread;
    
    /* Save the actual FAT cache size.  */
    temp =  _fx_system_media_max_fat_cache;
    _fx_system_media_max_fat_cache =  3;
    
    /* Call the _fxe_media_open to cause an error.  */
    status =  _fxe_media_open(&media_instance, FX_NULL, fake_driver, FX_NULL,
                      buffer, 0, sizeof(FX_MEDIA));

    /* Restore the max FAT cache size.  */
    _fx_system_media_max_fat_cache =  temp;

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_MEDIA_INVALID)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_open!\n");
    }

    /* Set the current thread to the test control thread, for error checking on the sector cache size.  */
    _tx_thread_current_ptr =  &test_control_thread;
    
    /* Save the actual FAT cache size.  */
    temp =  _fx_system_media_max_fat_cache;
    _fx_system_media_max_fat_cache =  6;
    
    /* Call the _fxe_media_open to cause an error.  */
    status =  _fxe_media_open(&media_instance, FX_NULL, fake_driver, FX_NULL,
                      buffer, 0, sizeof(FX_MEDIA));

    /* Restore the max FAT cache size.  */
    _fx_system_media_max_fat_cache =  temp;

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_MEDIA_INVALID)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_open!\n");
    }

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_media_open_notify_set.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;
    
    /* Call the _fxe_media_open_notify_set to cause an error.  */
    status =  _fxe_media_open_notify_set(FX_NULL, FX_NULL);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_open_notify_set!\n");
    }

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
       
    /* Call the _fxe_media_open_notify_set to cause an error.  */
    status =  _fxe_media_open_notify_set(&media_instance, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_open_notify_set!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_media_open_notify_set to cause an error.  */
    status =  _fxe_media_open_notify_set(&media_instance, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_open_notify_set!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_media_open_notify_set to cause an error.  */
    status =  _fxe_media_open_notify_set(&media_instance, FX_NULL);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_open_notify_set!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_media_read.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;
    
    /* Call the _fxe_media_read to cause an error.  */
    status =  _fxe_media_read(&media_instance, 0, FX_NULL);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_read!\n");
    }

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
       
    /* Call the _fxe_media_read to cause an error.  */
    status =  _fxe_media_read(&media_instance, 0, buffer);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_read!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_media_read to cause an error.  */
    status =  _fxe_media_read(&media_instance, 0, buffer);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_read!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_media_read to cause an error.  */
    status =  _fxe_media_read(&media_instance, 0, buffer);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_read!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_media_space_available.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;
    
    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  0;
    
    /* Fake test control thread running.  */
    _tx_thread_system_state =  0;
    _tx_thread_current_ptr =  &test_control_thread;

    /* Call the _fxe_media_space_available to cause and error.  */
    status =  _fxe_media_space_available(&media_instance, &temp);

    _tx_thread_system_state =  saved_system_state;
    _tx_thread_current_ptr =  FX_NULL;

    /* Check for the error.  */
    if (status != FX_MEDIA_NOT_OPEN)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_space_available!\n");
    }

    /* Call the _fxe_media_space_available to cause an error.  */
    status =  _fxe_media_space_available(&media_instance, FX_NULL);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_space_available!\n");
    }

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
       
    /* Call the _fxe_media_space_available to cause an error.  */
    status =  _fxe_media_space_available(&media_instance, &temp);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_space_available!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_media_space_available to cause an error.  */
    status =  _fxe_media_space_available(&media_instance, &temp);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_space_available!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_media_space_available to cause an error.  */
    status =  _fxe_media_space_available(&media_instance, &temp);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_space_available!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_media_volume_get.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;
    
    /* Call the _fxe_media_volume_get to cause an error.  */
    status =  _fxe_media_volume_get(&media_instance, FX_NULL, 0);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_volume_get!\n");
    }

    /* Call the _fxe_media_volume_get_extended to cause an error.  */
    status = _fxe_media_volume_get_extended(&media_instance, FX_NULL, 1, 0);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_media_volume_get_extended!\n");
    }

    /* Call the _fxe_media_volume_get_extended to cause an error.  */
    status = _fxe_media_volume_get_extended(&media_instance, (CHAR*)buffer, 0, 0);

    /* Check for the error.  */
    if (status != FX_BUFFER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_media_volume_get_extended!\n");
    }

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
       
    /* Call the _fxe_media_volume_get to cause an error.  */
    status =  _fxe_media_volume_get(&media_instance, (CHAR *)buffer, 0);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_volume_get!\n");
    }

    /* Call the _fxe_media_volume_get_extended to cause an error.  */
    status = _fxe_media_volume_get_extended(&media_instance, (CHAR*)buffer, sizeof(buffer), 0);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_media_volume_get_extended!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_media_volume_get to cause an error.  */
    status =  _fxe_media_volume_get(&media_instance, (CHAR *)buffer, 0);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_volume_get!\n");
    }

    /* Call the _fxe_media_volume_get_extended to cause an error.  */
    status = _fxe_media_volume_get_extended(&media_instance, (CHAR*)buffer, sizeof(buffer), 0);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_media_volume_get_extended!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_media_volume_get to cause an error.  */
    status =  _fxe_media_volume_get(&media_instance, (CHAR *)buffer, 0);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_volume_get!\n");
    }

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr = &_tx_timer_thread;

    /* Call the _fxe_media_volume_get_extended to cause an error.  */
    status = _fxe_media_volume_get_extended(&media_instance, (CHAR*)buffer, 1, 0);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr = FX_NULL;

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_media_volume_get_extended!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_media_volume_set.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;
    
    /* Call the _fxe_media_volume_set to cause an error.  */
    status =  _fxe_media_volume_set(&media_instance, FX_NULL);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_volume_set!\n");
    }

    /* Call the _fxe_media_volume_set to cause an error.  */
    buffer[0] =  0;
    status =  _fxe_media_volume_set(&media_instance, (CHAR *)buffer);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_volume_set!\n");
    }

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
       
    /* Call the _fxe_media_volume_set to cause an error.  */
    status =  _fxe_media_volume_set(&media_instance, "MYVOL");
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_volume_set!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_media_volume_set to cause an error.  */
    status =  _fxe_media_volume_set(&media_instance, "MYVOL");
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_volume_set!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_media_volume_set to cause an error.  */
    status =  _fxe_media_volume_set(&media_instance, "MYVOL");

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_volume_set!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_media_volume_set.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
       
    /* Call the _fxe_media_write to cause an error.  */
    status =  _fxe_media_write(&media_instance, 0, buffer);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_write!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_media_write to cause an error.  */
    status =  _fxe_media_write(&media_instance, 0, buffer);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_write!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_media_write to cause an error.  */
    status =  _fxe_media_write(&media_instance, 0, buffer);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_media_write!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_unicode_directory_create.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
       
    /* Call the _fxe_unicode_directory_create with a NULL source name to cause an error.  */
    status =  _fxe_unicode_directory_create(&media_instance, FX_NULL, 0, "SHORT");
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_directory_create!\n");
    }

    /* Call the _fxe_unicode_directory_create with a zero source name lenght to cause an error.  */
    status =  _fxe_unicode_directory_create(&media_instance, unicode_name, 0, "SHORT");
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_directory_create!\n");
    }

    /* Call the _fxe_unicode_directory_create with a NULL source short name to cause an error.  */
    status =  _fxe_unicode_directory_create(&media_instance, unicode_name, 8, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_directory_create!\n");
    }

    /* Call the _fxe_unicode_directory_create to cause an FX_CALLER_ERROR error.  */
    status =  _fxe_unicode_directory_create(&media_instance, unicode_name, 6,"SHORT");
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_directory_create!\n");
    }

    /* Call the _fxe_unicode_directory_create to cause an error.  */
    status = _fxe_unicode_directory_create(&media_instance, unicode_name, 6, "SHORT");

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_unicode_directory_create!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_unicode_directory_create to cause an error.  */
    status =  _fxe_unicode_directory_create(&media_instance, unicode_name, 6, "SHORT");
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_directory_create!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_unicode_directory_create to cause an error.  */
    status =  _fxe_unicode_directory_create(&media_instance, (UCHAR *)"UNICODE NAME'\0''\0'", 6, "SHORT");

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_directory_create!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_unicode_directory_rename.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
       
    /* Call the _fxe_unicode_directory_rename with a NULL old unicode name to cause an error.  */
    status =  _fxe_unicode_directory_rename(&media_instance, FX_NULL, 0, FX_NULL, 0, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_directory_rename!\n");
    }

    /* Call the _fxe_unicode_directory_rename with a zero length old unicode name to cause an error.  */
    status =  _fxe_unicode_directory_rename(&media_instance, (UCHAR *)"OLD_UNICODE_NAME", 0, FX_NULL, 0, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_directory_rename!\n");
    }

    /* Call the _fxe_unicode_directory_rename with a NULL new unicode name to cause an error.  */
    status =  _fxe_unicode_directory_rename(&media_instance, (UCHAR *)"OLD_UNICODE_NAME", 16, FX_NULL, 0, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_directory_rename!\n");
    }

    /* Call the _fxe_unicode_directory_rename with a zero lenght new unicode name to cause an error.  */
    status =  _fxe_unicode_directory_rename(&media_instance, (UCHAR *)"OLD_UNICODE_NAME", 16, (UCHAR *)"NEW_UNICODE_NAME", 0, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_directory_rename!\n");
    }

    /* Call the _fxe_unicode_directory_rename with a NULL new short name to cause an error.  */
    status =  _fxe_unicode_directory_rename(&media_instance, (UCHAR *)"OLD_UNICODE_NAME", 16, (UCHAR *)"NEW_UNICODE_NAME", 16, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_directory_rename!\n");
    }

    /* Call the _fxe_unicode_directory_rename to cause an error.  */
    status =  _fxe_unicode_directory_rename(&media_instance, (UCHAR *)"OLD_UNICODE_NAME", 16, (UCHAR *)"NEW_UNICODE_NAME", 16, "SHORT");
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_directory_rename!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_unicode_directory_rename to cause an error.  */
    status =  _fxe_unicode_directory_rename(&media_instance, (UCHAR *)"OLD_UNICODE_NAME", 16, (UCHAR *)"NEW_UNICODE_NAME", 16, "SHORT");
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_directory_rename!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_unicode_directory_rename to cause an error.  */
    status =  _fxe_unicode_directory_rename(&media_instance, (UCHAR *)"OLD_UNICODE_NAME", 16, (UCHAR *)"NEW_UNICODE_NAME", 16, "SHORT");

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_file_rename!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_unicode_file_create.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
       
    /* Call the _fxe_unicode_file_create with a NULL source name to cause an error.  */
    status =  _fxe_unicode_file_create(&media_instance, FX_NULL, 0, "SHORT");
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_file_create!\n");
    }

    /* Call the _fxe_unicode_file_create with a zero source name lenght to cause an error.  */
    status =  _fxe_unicode_file_create(&media_instance, (UCHAR *)"UNICODE NAME", 0, "SHORT");
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_file_create!\n");
    }

    /* Call the _fxe_unicode_file_create with a NULL source short name to cause an error.  */
    status =  _fxe_unicode_file_create(&media_instance, (UCHAR *)"UNICODE NAME", 6, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_file_create!\n");
    }

    /* Call the _fxe_unicode_file_create to cause an error.  */
    status =  _fxe_unicode_file_create(&media_instance, (UCHAR *)"UNICODE NAME", 6, "SHORT");
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_file_create!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_unicode_file_create to cause an error.  */
    status =  _fxe_unicode_file_create(&media_instance, (UCHAR *)"UNICODE NAME", 6, "SHORT");
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_file_create!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_unicode_file_create to cause an error.  */
    status =  _fxe_unicode_file_create(&media_instance, (UCHAR *)"UNICODE NAME", 6, "SHORT");

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_file_create!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_unicode_file_rename.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
       
    /* Call the _fxe_unicode_file_rename with a NULL old unicode name to cause an error.  */
    status =  _fxe_unicode_file_rename(&media_instance, FX_NULL, 0, FX_NULL, 0, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_file_rename!\n");
    }

    /* Call the _fxe_unicode_file_rename with a zero length old unicode name to cause an error.  */
    status =  _fxe_unicode_file_rename(&media_instance, (UCHAR *)"OLD_UNICODE_NAME", 0, FX_NULL, 0, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_file_rename!\n");
    }

    /* Call the _fxe_unicode_file_rename with a NULL new unicode name to cause an error.  */
    status =  _fxe_unicode_file_rename(&media_instance, (UCHAR *)"OLD_UNICODE_NAME", 16, FX_NULL, 0, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_file_rename!\n");
    }

    /* Call the _fxe_unicode_file_rename with a zero lenght new unicode name to cause an error.  */
    status =  _fxe_unicode_file_rename(&media_instance, (UCHAR *)"OLD_UNICODE_NAME", 16, (UCHAR *)"NEW_UNICODE_NAME", 0, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_file_rename!\n");
    }

    /* Call the _fxe_unicode_file_rename with a NULL new short name to cause an error.  */
    status =  _fxe_unicode_file_rename(&media_instance, (UCHAR *)"OLD_UNICODE_NAME", 16, (UCHAR *)"NEW_UNICODE_NAME", 16, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_file_rename!\n");
    }

    /* Call the _fxe_unicode_file_rename to cause an error.  */
    status =  _fxe_unicode_file_rename(&media_instance, (UCHAR *)"OLD_UNICODE_NAME", 16, (UCHAR *)"NEW_UNICODE_NAME", 16, "SHORT");
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_file_rename!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_unicode_file_rename to cause an error.  */
    status =  _fxe_unicode_file_rename(&media_instance, (UCHAR *)"OLD_UNICODE_NAME", 16, (UCHAR *)"NEW_UNICODE_NAME", 16, "SHORT");
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_file_rename!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_unicode_file_rename to cause an error.  */
    status =  _fxe_unicode_file_rename(&media_instance, (UCHAR *)"OLD_UNICODE_NAME", 16, (UCHAR *)"NEW_UNICODE_NAME", 16, "SHORT");

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_file_rename!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_unicode_name_get.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
       
    /* Call the _fxe_unicode_name_get with a NULL short name to cause an error.  */
    status =  _fxe_unicode_name_get(&media_instance, FX_NULL, FX_NULL, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_name_get!\n");
    }

    /* Call the _fxe_unicode_name_get with a NULL destination name to cause an error.  */
    status =  _fxe_unicode_name_get(&media_instance, "SHORTNAME", FX_NULL, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_name_get!\n");
    }

    /* Call the _fxe_unicode_name_get with a NULL destination name size to cause an error.  */
    status =  _fxe_unicode_name_get(&media_instance, "SHORTNAME", (UCHAR *)"DESTINATION", FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_name_get!\n");
    }

    /* Call the _fxe_unicode_name_get to cause an error.  */
    status =  _fxe_unicode_name_get(&media_instance, "SHORTNAME", (UCHAR *)"DESTINATION", &temp);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_name_get!\n");
    }

    /* Call the _fxe_unicode_name_get_extended with a NULL short name to cause an error.  */
    status = _fxe_unicode_name_get_extended(FX_NULL, FX_NULL, FX_NULL, FX_NULL, 0);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_unicode_name_get_extended!\n");
    }

    /* Call the _fxe_unicode_name_get_extended with a NULL short name to cause an error.  */
    status = _fxe_unicode_name_get_extended(&media_instance, FX_NULL, FX_NULL, FX_NULL, 0);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_unicode_name_get_extended!\n");
    }

    /* Call the _fxe_unicode_name_get_extended with a NULL destination name to cause an error.  */
    status = _fxe_unicode_name_get_extended(&media_instance, "SHORTNAME", FX_NULL, FX_NULL, 0);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_unicode_name_get_extended!\n");
    }

    /* Call the _fxe_unicode_name_get_extended with a NULL destination name size to cause an error.  */
    status = _fxe_unicode_name_get_extended(&media_instance, "SHORTNAME", (UCHAR*)"DESTINATION", FX_NULL, 0);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_unicode_name_get_extended!\n");
    }

    /* Call the _fxe_unicode_name_get_extended to cause an error.  */
    status = _fxe_unicode_name_get_extended(&media_instance, "SHORTNAME", (UCHAR*)"DESTINATION", &temp, 1);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_unicode_name_get_extended!\n");
    }

    /* Call the _fxe_unicode_name_get_extended to cause an error.  */
    status = _fxe_unicode_name_get_extended(&media_instance, "SHORTNAME", (UCHAR*)"DESTINATION", &temp, 0);

    /* Check for the error.  */
    if (status != FX_BUFFER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_unicode_name_get_extended!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_unicode_name_get to cause an error.  */
    status =  _fxe_unicode_name_get(&media_instance, "SHORTNAME", (UCHAR *)"DESTINATION", &temp);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_name_get!\n");
    }

    /* Call the _fxe_unicode_name_get_extended to cause an error.  */
    status = _fxe_unicode_name_get_extended(&media_instance, "SHORTNAME", (UCHAR*)"DESTINATION", &temp, 1);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_unicode_name_get_extended!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_unicode_name_get to cause an error.  */
    status =  _fxe_unicode_name_get(&media_instance, "SHORTNAME", (UCHAR *)"DESTINATION", &temp);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_name_get!\n");
    }

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr = &_tx_timer_thread;

    /* Call the _fxe_unicode_name_get_extended to cause an error.  */
    status = _fxe_unicode_name_get_extended(&media_instance, "SHORTNAME", (UCHAR*)"DESTINATION", &temp, 1);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr = FX_NULL;

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_unicode_name_get_extended!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_unicode_short_name_get.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
       
    /* Call the _fxe_unicode_short_name_get with a NULL source name to cause an error.  */
    status =  _fxe_unicode_short_name_get(&media_instance, FX_NULL, 0, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_short_name_get!\n");
    }

    /* Call the _fxe_unicode_short_name_get with a zero length source name to cause an error.  */
    status =  _fxe_unicode_short_name_get(&media_instance, (UCHAR *)"SOURCENAME", 0, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_short_name_get!\n");
    }

    /* Call the _fxe_unicode_short_name_get with a NULL destination name to cause an error.  */
    status =  _fxe_unicode_short_name_get(&media_instance, (UCHAR *)"SOURCENAME", 16, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_short_name_get!\n");
    }

    /* Call the _fxe_unicode_short_name_get to cause an error.  */
    status =  _fxe_unicode_short_name_get(&media_instance, (UCHAR *)"SOURCENAME", 16, "DESTINATION");
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_short_name_get!\n");
    }

    /* Call the _fxe_unicode_short_name_get_extended with a NULL source name to cause an error.  */
    status = _fxe_unicode_short_name_get_extended(&media_instance, FX_NULL, 0, FX_NULL, 0);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_unicode_short_name_get_extended!\n");
    }

    /* Call the _fxe_unicode_short_name_get_extended with a zero length source name to cause an error.  */
    status = _fxe_unicode_short_name_get_extended(&media_instance, (UCHAR*)"SOURCENAME", 0, FX_NULL, 0);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_unicode_short_name_get_extended!\n");
    }

    /* Call the _fxe_unicode_short_name_get_extended with a NULL destination name to cause an error.  */
    status = _fxe_unicode_short_name_get_extended(&media_instance, (UCHAR*)"SOURCENAME", 16, FX_NULL, 0);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_unicode_short_name_get_extended!\n");
    }

    /* Call the _fxe_unicode_short_name_get_extended with a NULL destination name to cause an error.  */
    status = _fxe_unicode_short_name_get_extended(FX_NULL, FX_NULL, 0, FX_NULL, 0);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_unicode_short_name_get_extended!\n");
    }

    /* Call the _fxe_unicode_short_name_get_extended to cause an error.  */
    status = _fxe_unicode_short_name_get_extended(&media_instance, (UCHAR*)"SOURCENAME", 16, "DESTINATION", 1);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_unicode_short_name_get_extended!\n");
    }

    /* Call the _fxe_unicode_short_name_get_extended to cause an error.  */
    status = _fxe_unicode_short_name_get_extended(&media_instance, (UCHAR*)"SOURCENAME", 16, "DESTINATION", 0);

    /* Check for the error.  */
    if (status != FX_BUFFER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_unicode_short_name_get_extended!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_unicode_short_name_get to cause an error.  */
    status =  _fxe_unicode_short_name_get(&media_instance, (UCHAR *)"SOURCENAME", 16, "DESTINATION");
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_short_name_get!\n");
    }

    /* Call the _fxe_unicode_short_name_get_extended to cause an error.  */
    status = _fxe_unicode_short_name_get_extended(&media_instance, (UCHAR*)"SOURCENAME", 16, "DESTINATION", 1);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_unicode_short_name_get_extended!\n");
    }
#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_unicode_short_name_get to cause an error.  */
    status =  _fxe_unicode_short_name_get(&media_instance, (UCHAR *)"SOURCENAME", 16, "DESTINATION");

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_unicode_short_name_get!\n");
    }

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr = &_tx_timer_thread;

    /* Call the _fxe_unicode_short_name_get to cause an error.  */
    status = _fxe_unicode_short_name_get_extended(&media_instance, (UCHAR*)"SOURCENAME", 16, "DESTINATION", 1);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr = FX_NULL;

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_unicode_short_name_get!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;

    /* Check caller error checking for fx_file_write_notify.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
       
    /* Call the _fxe_file_write_notify_set with a NULL file pointer to cause an error.  */
    status =  _fxe_file_write_notify_set(FX_NULL, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_write_notify_set!\n");
    }

    /* Call the _fxe_file_write_notify_set to cause an error.  */
    status =  _fxe_file_write_notify_set(&file_instance, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_write_notify_set!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_write_notify_set to cause an error.  */
    status =  _fxe_file_write_notify_set(&file_instance, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_write_notify_set!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_write_notify_set to cause an error.  */
    status =  _fxe_file_write_notify_set(&file_instance, FX_NULL);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_write_notify_set!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_write.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;
       
    /* Call the _fxe_file_write with a NULL buffer pointer to cause an error.  */
    status =  _fxe_file_write(&file_instance, FX_NULL, 20);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_write!\n");
    }

    /* Call the _fxe_file_write with a NULL buffer pointer to cause an error.  */
    status =  _fxe_file_write(&file_instance, buffer, 20);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_write!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_write with a NULL buffer pointer to cause an error.  */
    status =  _fxe_file_write(&file_instance, buffer, 20);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_write!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_write with a NULL buffer pointer to cause an error.  */
    status =  _fxe_file_write(&file_instance, buffer, 20);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_write!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_truncate_release.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Call the _fxe_file_truncate_release to cause an error.  */
    status =  _fxe_file_truncate_release(&file_instance, 10);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_truncate_release!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_truncate_release to cause an error.  */
    status =  _fxe_file_truncate_release(&file_instance, 10);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_truncate_release!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_truncate_release to cause an error.  */
    status =  _fxe_file_truncate_release(&file_instance, 10);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_truncate_release!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_truncate.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Call the _fxe_file_truncate to cause an error.  */
    status =  _fxe_file_truncate(&file_instance, 10);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_truncate!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_truncate to cause an error.  */
    status =  _fxe_file_truncate(&file_instance, 10);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_truncate!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_truncate to cause an error.  */
    status =  _fxe_file_truncate(&file_instance, 10);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_truncate!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_seek.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Call the _fxe_file_seek to cause an error.  */
    status =  _fxe_file_seek(&file_instance, 10);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_seek!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_seek to cause an error.  */
    status =  _fxe_file_seek(&file_instance, 10);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_seek!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_seek to cause an error.  */
    status =  _fxe_file_seek(&file_instance, 10);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_seek!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_rename.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Call the _fxe_file_rename to cause an error.  */
    status =  _fxe_file_rename(&media_instance, "OLDNAME", "NEWNAME");
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_rename!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_rename to cause an error.  */
    status =  _fxe_file_rename(&media_instance, "OLDNAME", "NEWNAME");

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_rename!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_rename to cause an error.  */
    status =  _fxe_file_rename(&media_instance, "OLDNAME", "NEWNAME");

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_rename!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_relative_seek.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Call the _fxe_file_relative_seek to cause an error.  */
    status =  _fxe_file_relative_seek(&file_instance, 0, FX_SEEK_BEGIN);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_relative_seek!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_relative_seek to cause an error.  */
    status =  _fxe_file_relative_seek(&file_instance, 0, FX_SEEK_BEGIN);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_relative_seek!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_relative_seek to cause an error.  */
    status =  _fxe_file_relative_seek(&file_instance, 0, FX_SEEK_BEGIN);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_relative_seek!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_read.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Call the _fxe_file_read with NULL buffer to cause an error.  */
    status =  _fxe_file_read(&file_instance, FX_NULL, 20, &temp);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_read!\n");
    }

    /* Call the _fxe_file_read with NULL actual size to cause an error.  */
    status =  _fxe_file_read(&file_instance, buffer, 20, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_read!\n");
    }

    /* Call the _fxe_file_read to cause an error.  */
    status =  _fxe_file_read(&file_instance, buffer, 20, &temp);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_read!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_read to cause an error.  */
    status =  _fxe_file_read(&file_instance, buffer, 20, &temp);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_read!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_read to cause an error.  */
    status =  _fxe_file_read(&file_instance, buffer, 20, &temp);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_read!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_open.  */
    
    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  0;

    /* Call the _fx_file_open with a 0 media ID to cause an error.  */
    status =  _fx_file_open(&media_instance, FX_NULL, "TEST", FX_OPEN_FOR_READ);
    
    /* Check for the error.  */
    if (status != FX_MEDIA_NOT_OPEN)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fx_file_open!\n");
    }
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_file_open with NULL file pointer to cause an error.  */
    status =  _fxe_file_open(&media_instance, FX_NULL, "TEST", FX_OPEN_FOR_READ, 20);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_open!\n");
    }

    /* Call the _fxe_file_open with bad control block size to cause an error.  */
    status =  _fxe_file_open(&media_instance, &file_instance, "TEST", FX_OPEN_FOR_READ, 20);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_open!\n");
    }

    /* Call the _fxe_file_open with bad open type to cause an error.  */
    status =  _fxe_file_open(&media_instance, &file_instance, "TEST", (FX_OPEN_FOR_READ+FX_OPEN_FOR_READ_FAST+FX_OPEN_FOR_WRITE+99), sizeof(FX_FILE));
    
    /* Check for the error.  */
    if (status != FX_ACCESS_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_open!\n");
    }

    /* Call the _fxe_file_open to cause an error.  */
    status =  _fxe_file_open(&media_instance, &file_instance, "TEST", FX_OPEN_FOR_READ, sizeof(FX_FILE));
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_open!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_open to cause an error.  */
    status =  _fxe_file_open(&media_instance, &file_instance, "TEST", FX_OPEN_FOR_READ, sizeof(FX_FILE));

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_open!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_open to cause an error.  */
    status =  _fxe_file_open(&media_instance, &file_instance, "TEST", FX_OPEN_FOR_READ, sizeof(FX_FILE));

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_open!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_extended_truncate_release.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_file_extended_truncate_release to cause an error.  */
    status =  _fxe_file_extended_truncate_release(&file_instance, 20);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_truncate_release!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_extended_truncate_release to cause an error.  */
    status =  _fxe_file_extended_truncate_release(&file_instance, 20);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_extended_truncate_release!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_extended_truncate_release to cause an error.  */
    status =  _fxe_file_extended_truncate_release(&file_instance, 20);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_extended_truncate_release!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_extended_truncate.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_file_extended_truncate to cause an error.  */
    status =  _fxe_file_extended_truncate(&file_instance, 20);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_truncate!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_extended_truncate to cause an error.  */
    status =  _fxe_file_extended_truncate(&file_instance, 20);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_extended_truncate!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_extended_truncate to cause an error.  */
    status =  _fxe_file_extended_truncate(&file_instance, 20);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_extended_truncate!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_extended_seek.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_file_extended_seek to cause an error.  */
    status =  _fxe_file_extended_seek(&file_instance, 20);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_extended_seek!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_extended_seek to cause an error.  */
    status =  _fxe_file_extended_seek(&file_instance, 20);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_extended_seek!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_extended_seek to cause an error.  */
    status =  _fxe_file_extended_seek(&file_instance, 20);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_extended_seek!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_extended_relative_seek.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_file_extended_relative_seek to cause an error.  */
    status =  _fxe_file_extended_relative_seek(&file_instance, 20, FX_SEEK_END);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_extended_relative_seek!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_extended_relative_seek to cause an error.  */
    status =  _fxe_file_extended_relative_seek(&file_instance, 20, FX_SEEK_FORWARD);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_extended_relative_seek!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_extended_relative_seek to cause an error.  */
    status =  _fxe_file_extended_relative_seek(&file_instance, 20, FX_SEEK_BEGIN);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_extended_relative_seek!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_extended_best_effort_allocate.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_file_extended_best_effort_allocate with NULL actual size allocated to cause an error.  */
    status =  _fxe_file_extended_best_effort_allocate(&file_instance, 20, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_extended_best_effort_allocate!\n");
    }

    /* Call the _fxe_file_extended_best_effort_allocate to cause an error.  */
    status =  _fxe_file_extended_best_effort_allocate(&file_instance, 20, &available_bytes);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_extended_best_effort_allocate!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_extended_best_effort_allocate to cause an error.  */
    status =  _fxe_file_extended_best_effort_allocate(&file_instance, 20, &available_bytes);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_extended_best_effort_allocate!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_extended_best_effort_allocate to cause an error.  */
    status =  _fxe_file_extended_best_effort_allocate(&file_instance, 20, &available_bytes);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_extended_best_effort_allocate!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_extended_allocate.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_file_extended_allocate to cause an error.  */
    status =  _fxe_file_extended_allocate(&file_instance, 20);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_extended_allocate!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_extended_allocate to cause an error.  */
    status =  _fxe_file_extended_allocate(&file_instance, 20);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_extended_allocate!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_extended_allocate to cause an error.  */
    status =  _fxe_file_extended_allocate(&file_instance, 20);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_extended_allocate!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_delete.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_file_delete to cause an error.  */
    status =  _fxe_file_delete(&media_instance, "MYFILE");
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_delete!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_delete to cause an error.  */
    status =  _fxe_file_delete(&media_instance, "MYFILE");

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_delete!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_delete to cause an error.  */
    status =  _fxe_file_delete(&media_instance, "MYFILE");

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_delete!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_date_time_set.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_file_date_time_set to cause an error.  */
    status =  _fxe_file_date_time_set(&media_instance, "TEST.TXT", 
                2017, 12, 31, 23, 59, 58);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_date_time)set!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_date_time_set to cause an error.  */
    status =  _fxe_file_date_time_set(&media_instance, "TEST.TXT", 
                2017, 12, 31, 23, 59, 58);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_date_time_set!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_date_time_set to cause an error.  */
    status =  _fxe_file_date_time_set(&media_instance, "TEST.TXT", 
                2017, 12, 31, 23, 59, 58);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_date_time_set!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_create.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_file_create to cause an error.  */
    status =  _fxe_file_create(&media_instance, "TEST.TXT");
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_create!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_create to cause an error.  */
    status =  _fxe_file_create(&media_instance, "TEST.TXT");

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_create!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_create to cause an error.  */
    status =  _fxe_file_create(&media_instance, "TEST.TXT");

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_create!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_close.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_file_close to cause an error.  */
    status =  _fxe_file_close(&file_instance);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_close!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_close to cause an error.  */
    status =  _fxe_file_close(&file_instance);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_close!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_close to cause an error.  */
    status =  _fxe_file_close(&file_instance);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_close!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_best_effort_allocate.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_file_best_effort_allocate with a NULL actual_size_allocated to cause an error.  */
    status =  _fxe_file_best_effort_allocate(&file_instance, 20, FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_best_effort_allocate!\n");
    }

    /* Call the _fxe_file_best_effort_allocate to cause an error.  */
    status =  _fxe_file_best_effort_allocate(&file_instance, 20, &temp);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_best_effort_allocate!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_best_effort_allocate to cause an error.  */
    status =  _fxe_file_best_effort_allocate(&file_instance, 20, &temp);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_best_effort_allocate!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_best_effort_allocate to cause an error.  */
    status =  _fxe_file_best_effort_allocate(&file_instance, 20, &temp);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_best_effort_allocate!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_attributes_set.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_file_attributes_set to cause an error.  */
    status =  _fxe_file_attributes_set(&media_instance, "TEST.TXT", FX_READ_ONLY);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_attributes_set!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_attributes_set to cause an error.  */
    status =  _fxe_file_attributes_set(&media_instance, "TEST.TXT", FX_READ_ONLY);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_attributes_set!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_attributes_set to cause an error.  */
    status =  _fxe_file_attributes_set(&media_instance, "TEST.TXT", FX_READ_ONLY);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_attributes_set!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_attributes_read.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_file_attributes_read with NULL attributes variable to cause an error.  */
    status =  _fxe_file_attributes_read(&media_instance, "TEST.TXT", FX_NULL);
    
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_attributes_read!\n");
    }

    /* Call the _fxe_file_attributes_read to cause an error.  */
    status =  _fxe_file_attributes_read(&media_instance, "TEST.TXT", &attributes);
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_attributes_read!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_attributes_read to cause an error.  */
    status =  _fxe_file_attributes_read(&media_instance, "TEST.TXT", &attributes);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_attributes_read!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_attributes_read to cause an error.  */
    status =  _fxe_file_attributes_read(&media_instance, "TEST.TXT", &attributes);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_attributes_read!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_file_allocate.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_file_allocate to cause an error.  */
    status =  _fxe_file_allocate(&file_instance, 20);
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_allocate!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_file_allocate to cause an error.  */
    status =  _fxe_file_allocate(&file_instance, 20);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_allocate!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_file_allocate to cause an error.  */
    status =  _fxe_file_allocate(&file_instance, 20);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_file_allocate!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_directory_short_name_get.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_directory_short_name_get with NULL long file name to cause an error.  */
    status =  _fxe_directory_short_name_get(&media_instance, FX_NULL, "SHORT");
       
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_short_name_get!\n");
    }

    /* Call the _fxe_directory_short_name_get_extended with NULL long file name to cause an error.  */
    status = _fxe_directory_short_name_get_extended(&media_instance, FX_NULL, "SHORT", 0);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_directory_short_name_get_extended!\n");
    }

    /* Call the _fxe_directory_short_name_get with NULL short name to cause an error.  */
    status =  _fxe_directory_short_name_get(&media_instance, "LONGNAME", FX_NULL);
       
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_short_name_get!\n");
    }

    /* Call the _fxe_directory_short_name_get_extended with NULL short name to cause an error.  */
    status = _fxe_directory_short_name_get_extended(&media_instance, "LONGNAME", FX_NULL, 0);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_directory_short_name_get_extended!\n");
    }

    /* Call the _fxe_directory_short_name_get_extended with NULL short name to cause an error.  */
    status = _fxe_directory_short_name_get_extended(&media_instance, "LONGNAME", "SHORT", 0);

    /* Check for the error.  */
    if (status != FX_BUFFER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_directory_short_name_get_extended!\n");
    }

    /* Call the _fxe_directory_short_name_get to cause an error.  */
    status =  _fxe_directory_short_name_get(&media_instance, "LONGNAME", "SHORT");
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_short_name_get!\n");
    }

    /* Call the _fxe_directory_short_name_get_extended to cause an error.  */
    status = _fxe_directory_short_name_get_extended(&media_instance, "LONGNAME", "SHORT", 1);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_directory_short_name_get_extended!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_directory_short_name_get to cause an error.  */
    status =  _fxe_directory_short_name_get(&media_instance, "LONGNAME", "SHORT");

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_short_name_get!\n");
    }

    /* Call the _fxe_directory_short_name_get_extended to cause an error.  */
    status = _fxe_directory_short_name_get_extended(&media_instance, "LONGNAME", "SHORT", 1);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_directory_short_name_get_extended!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_directory_short_name_get to cause an error.  */
    status =  _fxe_directory_short_name_get(&media_instance, "LONGNAME", "SHORT");

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_short_name_get!\n");
    }

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_directory_short_name_get_extended to cause an error.  */
    status =  _fxe_directory_short_name_get_extended(&media_instance, "LONGNAME", "SHORT", 1);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_short_name_get_extended!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_directory_rename.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_directory_rename to cause an error.  */
    status =  _fxe_directory_rename(&media_instance, "LONGNAME", "SHORT");
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_rename!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_directory_rename to cause an error.  */
    status =  _fxe_directory_rename(&media_instance, "LONGNAME", "SHORT");

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_rename!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_directory_rename to cause an error.  */
    status =  _fxe_directory_rename(&media_instance, "LONGNAME", "SHORT");

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_rename!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_directory_next_full_entry_find.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_directory_next_full_entry_find to cause an error.  */
    status =  _fxe_directory_next_full_entry_find(&media_instance, FX_NULL, FX_NULL, FX_NULL, 
                                                                   FX_NULL, FX_NULL, FX_NULL,
                                                                   FX_NULL, FX_NULL, FX_NULL);
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_next_full_entry_find!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_directory_next_full_entry_find to cause an error.  */
    status =  _fxe_directory_next_full_entry_find(&media_instance, FX_NULL, FX_NULL, FX_NULL, 
                                                                   FX_NULL, FX_NULL, FX_NULL,
                                                                   FX_NULL, FX_NULL, FX_NULL);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_next_full_entry_find!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_directory_next_full_entry_find to cause an error.  */
    status =  _fxe_directory_next_full_entry_find(&media_instance, FX_NULL, FX_NULL, FX_NULL, 
                                                                   FX_NULL, FX_NULL, FX_NULL,
                                                                   FX_NULL, FX_NULL, FX_NULL);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_next_full_entry_find!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_directory_next_entry_find.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_directory_next_entry_find to cause an error.  */
    status =  _fxe_directory_next_entry_find(&media_instance, FX_NULL);
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_next_entry_find!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_directory_next_entry_find to cause an error.  */
    status =  _fxe_directory_next_entry_find(&media_instance, FX_NULL);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_next_entry_find!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_directory_next_entry_find to cause an error.  */
    status =  _fxe_directory_next_entry_find(&media_instance, FX_NULL);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_next_entry_find!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_directory_name_test.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_directory_name_test to cause an error.  */
    status =  _fxe_directory_name_test(&media_instance, "DIRNAME");
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_name_test!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_directory_name_test to cause an error.  */
    status =  _fxe_directory_name_test(&media_instance, "DIRNAME");

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_name_test!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_directory_name_test to cause an error.  */
    status =  _fxe_directory_name_test(&media_instance, "DIRNAME");

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_name_test!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_directory_long_name_get.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_directory_long_name_get with NULL short name to cause an error.  */
    status =  _fxe_directory_long_name_get(&media_instance, FX_NULL, "LONGNAME");
       
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_long_name_get!\n");
    }

    /* Call the _fxe_directory_long_name_get_extended with NULL short name to cause an error.  */
    status = _fxe_directory_long_name_get_extended(&media_instance, FX_NULL, "LONGNAME", 0);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_directory_long_name_get_extended!\n");
    }

    /* Call the _fxe_directory_long_name_get with NULL long name to cause an error.  */
    status =  _fxe_directory_long_name_get(&media_instance, "SHORTNAME", FX_NULL);
       
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_long_name_get!\n");
    }

    /* Call the _fxe_directory_long_name_get_extended with NULL long name to cause an error.  */
    status = _fxe_directory_long_name_get_extended(&media_instance, "SHORTNAME", FX_NULL, 0);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_directory_long_name_get_extended!\n");
    }

    /* Call the _fxe_directory_long_name_get_extended with zero long name buffer length to cause an error.  */
    status = _fxe_directory_long_name_get_extended(&media_instance, "SHORTNAME", "LONGNAME", 0);

    /* Check for the error.  */
    if (status != FX_BUFFER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_directory_long_name_get_extended!\n");
    }

    /* Call the _fxe_directory_long_name_get to cause an error.  */
    status =  _fxe_directory_long_name_get(&media_instance, "SHORTNAME", "LONGNAME");
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_long_name_get!\n");
    }

    /* Call the _fxe_directory_long_name_get_extended to cause an error.  */
    status = _fxe_directory_long_name_get_extended(&media_instance, "SHORTNAME", "LONGNAME", 1);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_directory_long_name_get_extended!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_directory_long_name_get to cause an error.  */
    status =  _fxe_directory_long_name_get(&media_instance, "SHORTNAME", "LONGNAME");

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_long_name_get!\n");
    }

    /* Call the _fxe_directory_long_name_get_extended to cause an error.  */
    status = _fxe_directory_long_name_get_extended(&media_instance, "SHORTNAME", "LONGNAME", 1);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_directory_long_name_get_extended!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_directory_long_name_get to cause an error.  */
    status =  _fxe_directory_long_name_get(&media_instance, "SHORTNAME", "LONGNAME");

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_long_name_get!\n");
    }

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr = &_tx_timer_thread;

    /* Call the _fxe_directory_long_name_get_extended to cause an error.  */
    status = _fxe_directory_long_name_get_extended(&media_instance, "SHORTNAME", "LONGNAME", 1);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr = FX_NULL;

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;

        /* Print error message.  */
        printf("Error in checking of fxe_directory_long_name_get_extended!\n");
    }
#endif

    /* media_ptr is NULL. */
    status = _fxe_directory_local_path_get(FX_NULL, FX_NULL);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_local_path_get!\n");
    }

    /* return_path_name is NULL. */
    status = _fxe_directory_local_path_get(&media_instance, FX_NULL);

    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_local_path_get!\n");
    }

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_directory_local_path_set.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_directory_local_path_set with NULL short name to cause an error.  */
    status =  _fxe_directory_local_path_set(&media_instance, FX_NULL, FX_NULL, 0);
       
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_local_path_set!\n");
    }

    /* Call the _fxe_directory_local_path_set with 0 path control block size to cause an error.  */
    status =  _fxe_directory_local_path_set(&media_instance, &local_path_instance, FX_NULL, 0);
       
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_local_path_set!\n");
    }

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_directory_local_path_restore.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_directory_local_path_restore with NULL local path pointer to cause an error.  */
    status =  _fxe_directory_local_path_restore(&media_instance, FX_NULL);
       
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_local_path_restore!\n");
    }

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_directory_information_get.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_directory_information_get with NULL media name to cause an error.  */
    status =  _fxe_directory_information_get(FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL);
       
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_information_get!\n");
    }

    /* Call the _fxe_directory_information_get with NULL attributes pointer to cause an error.  */
    status =  _fxe_directory_information_get(&media_instance, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL);
       
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_information_get!\n");
    }

    /* Call the _fxe_directory_information_get with one non-NULL pointer to cause an error.  */
    status =  _fxe_directory_information_get(&media_instance, FX_NULL, &attributes, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL);
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_information_get!\n");
    }

    /* Call the _fxe_directory_information_get with one non-NULL pointer to cause an error.  */
    status =  _fxe_directory_information_get(&media_instance, FX_NULL, FX_NULL, (ULONG*)&attributes, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL);
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_information_get!\n");
    }

    /* Call the _fxe_directory_information_get with one non-NULL pointer to cause an error.  */
    status =  _fxe_directory_information_get(&media_instance, FX_NULL, FX_NULL, FX_NULL, &attributes, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL);
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_information_get!\n");
    }

    /* Call the _fxe_directory_information_get with one non-NULL pointer to cause an error.  */
    status =  _fxe_directory_information_get(&media_instance, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &attributes, FX_NULL, FX_NULL, FX_NULL, FX_NULL);
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_information_get!\n");
    }

    /* Call the _fxe_directory_information_get with one non-NULL pointer to cause an error.  */
    status =  _fxe_directory_information_get(&media_instance, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &attributes, FX_NULL, FX_NULL, FX_NULL);
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_information_get!\n");
    }

    /* Call the _fxe_directory_information_get with one non-NULL pointer to cause an error.  */
    status =  _fxe_directory_information_get(&media_instance, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &attributes, FX_NULL, FX_NULL);
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_information_get!\n");
    }

    /* Call the _fxe_directory_information_get with one non-NULL pointer to cause an error.  */
    status =  _fxe_directory_information_get(&media_instance, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &attributes, FX_NULL);
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_information_get!\n");
    }

    /* Call the _fxe_directory_information_get with one non-NULL pointer to cause an error.  */
    status =  _fxe_directory_information_get(&media_instance, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, &attributes);
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_information_get!\n");
    }

    /* Call the _fxe_directory_information_get to cause an error.  */
    status =  _fxe_directory_information_get(&media_instance, FX_NULL, &attributes, (ULONG *)&attributes, &attributes, &attributes, &attributes, &attributes, &attributes, &attributes);
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_information_get!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_directory_information_get to cause an error.  */
    status =  _fxe_directory_information_get(&media_instance, FX_NULL, &attributes, (ULONG *)&attributes, &attributes, &attributes, &attributes, &attributes, &attributes, &attributes);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_information_get!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_directory_information_get to cause an error.  */
    status =  _fxe_directory_information_get(&media_instance, FX_NULL, &attributes, (ULONG *)&attributes, &attributes, &attributes, &attributes, &attributes, &attributes, &attributes);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_information_get!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_directory_first_full_entry_find.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_directory_first_full_entry_find to cause an error.  */
    status =  _fxe_directory_first_full_entry_find(&media_instance, FX_NULL, FX_NULL, FX_NULL, 
                                                    FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL);
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_first_full_entry_find!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_directory_first_full_entry_find to cause an error.  */
    status =  _fxe_directory_first_full_entry_find(&media_instance, FX_NULL, FX_NULL, FX_NULL, 
                                                    FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_first_full_entry_find!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_directory_first_full_entry_find to cause an error.  */
    status =  _fxe_directory_first_full_entry_find(&media_instance, FX_NULL, FX_NULL, FX_NULL, 
                                                    FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_first_full_entry_find!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_directory_first_entry_find.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_directory_first_entry_find to cause an error.  */
    status =  _fxe_directory_first_entry_find(&media_instance, FX_NULL);
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_first_entry_find!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_directory_first_entry_find to cause an error.  */
    status =  _fxe_directory_first_entry_find(&media_instance, FX_NULL);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_first_entry_find!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_directory_first_entry_find to cause an error.  */
    status =  _fxe_directory_first_entry_find(&media_instance, FX_NULL);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_first_entry_find!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_directory_delete.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_directory_delete to cause an error.  */
    status =  _fxe_directory_delete(&media_instance, FX_NULL);
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_delete!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_directory_delete to cause an error.  */
    status =  _fxe_directory_delete(&media_instance, FX_NULL);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_delete!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_directory_delete to cause an error.  */
    status =  _fxe_directory_delete(&media_instance, FX_NULL);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_delete!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_directory_default_set.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_directory_default_set to cause an error.  */
    status =  _fxe_directory_default_set(&media_instance, FX_NULL);
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_default_set!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_directory_default_set to cause an error.  */
    status =  _fxe_directory_default_set(&media_instance, FX_NULL);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_default_set!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_directory_default_set to cause an error.  */
    status =  _fxe_directory_default_set(&media_instance, FX_NULL);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_default_set!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_directory_default_get.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_directory_default_get to cause an error.  */
    status =  _fxe_directory_default_get(&media_instance, &string_ptr);
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_default_get!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_directory_default_get to cause an error.  */
    status =  _fxe_directory_default_get(&media_instance, &string_ptr);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_default_get!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_directory_default_get to cause an error.  */
    status =  _fxe_directory_default_get(&media_instance, &string_ptr);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_default_get!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_directory_create.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_directory_create to cause an error.  */
    status =  _fxe_directory_create(&media_instance, "RETURN");
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_create!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_directory_create to cause an error.  */
    status =  _fxe_directory_create(&media_instance, "RETURN");

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_create!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_directory_create to cause an error.  */
    status =  _fxe_directory_create(&media_instance, "RETURN");

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_create!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_directory_attributes_set.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_directory_attributes_set to cause an error.  */
    status =  _fxe_directory_attributes_set(&media_instance, "RETURN", FX_HIDDEN);
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_attributes_set!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_directory_attributes_set to cause an error.  */
    status =  _fxe_directory_attributes_set(&media_instance, "RETURN", FX_HIDDEN);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_attributes_set!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_directory_attributes_set to cause an error.  */
    status =  _fxe_directory_attributes_set(&media_instance, "RETURN", FX_HIDDEN);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_attributes_set!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;


    /* Check caller error checking for fx_directory_attributes_read.  */
    
    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_directory_attributes_read with a NULL attributes pointer to cause an error.  */
    status =  _fxe_directory_attributes_read(&media_instance, "RETURN", FX_NULL);
       
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_attributes_read!\n");
    }

    /* Call the _fxe_directory_attributes_read to cause an error.  */
    status =  _fxe_directory_attributes_read(&media_instance, "RETURN", &attributes);
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_attributes_read!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;
    
    /* Call the _fxe_directory_attributes_read to cause an error.  */
    status =  _fxe_directory_attributes_read(&media_instance, "RETURN", &attributes);

    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_attributes_read!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_directory_attributes_read to cause an error.  */
    status =  _fxe_directory_attributes_read(&media_instance, "RETURN", &attributes);

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of fxe_directory_attributes_read!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;

    /* Check PTR error checking for _fxe_directory_default_get_copy.  */

    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;     

    CHAR            path_name_buffer[32];

    /* Setup the fake media structure.  */
    media_instance.fx_media_id =  FX_MEDIA_ID;

    /* Call the _fxe_directory_default_get_copy with a NULL attributes pointer to cause an error.  */
    status =  _fxe_directory_default_get_copy(NULL, path_name_buffer, sizeof(path_name_buffer));
       
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of _fxe_directory_default_get_copy!\n");
    }

    /* Call the _fxe_directory_default_get_copy with a NULL attributes pointer to cause an error.  */
    status =  _fxe_directory_default_get_copy(&media_instance, NULL, sizeof(path_name_buffer));
       
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of _fxe_directory_default_get_copy!\n");
    }

    /* Call the _fxe_directory_default_get_copy to cause an error.  */
    status =  _fxe_directory_default_get_copy(&media_instance, path_name_buffer, sizeof(path_name_buffer));
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of _fxe_directory_default_get_copy!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;

    /* Call the _fxe_directory_default_get_copy to cause an error.  */
    status =  _fxe_directory_default_get_copy(&media_instance, path_name_buffer, sizeof(path_name_buffer));
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of _fxe_directory_default_get_copy!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_directory_default_get_copy to cause an error.  */
    status =  _fxe_directory_default_get_copy(&media_instance, path_name_buffer, sizeof(path_name_buffer));

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of _fxe_directory_default_get_copy!\n");
    }
#endif

    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;

    /* Check PTR error checking for _fxe_directory_local_path_get_copy.  */

    /* Save the system state.  */
    saved_system_state =  _tx_thread_system_state;

    /* Call the _fxe_directory_local_path_get_copy with a NULL attributes pointer to cause an error.  */
    status =  _fxe_directory_local_path_get_copy(NULL, path_name_buffer, sizeof(path_name_buffer));
       
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of _fxe_directory_local_path_get_copy!\n");
    }

    /* Call the _fxe_directory_local_path_get_copy with a NULL attributes pointer to cause an error.  */
    status =  _fxe_directory_local_path_get_copy(&media_instance, NULL, sizeof(path_name_buffer));
       
    /* Check for the error.  */
    if (status != FX_PTR_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of _fxe_directory_local_path_get_copy!\n");
    }

    /* Call the _fxe_directory_local_path_get_copy to cause an error.  */
    status =  _fxe_directory_local_path_get_copy(&media_instance, path_name_buffer, sizeof(path_name_buffer));
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of _fxe_directory_local_path_get_copy!\n");
    }

    /* Clear the system state to simulate an ISR caller.  */
    _tx_thread_system_state =  0;

    /* Call the _fxe_directory_local_path_get_copy to cause an error.  */
    status =  _fxe_directory_local_path_get_copy(&media_instance, path_name_buffer, sizeof(path_name_buffer));
       
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of _fxe_directory_local_path_get_copy!\n");
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Set the current thread to the timer thread, for caller checking.  */
    _tx_thread_current_ptr =  &_tx_timer_thread;
    
    /* Call the _fxe_directory_local_path_get_copy to cause an error.  */
    status =  _fxe_directory_local_path_get_copy(&media_instance, path_name_buffer, sizeof(path_name_buffer));

    /* Set the current thread back to NULL.  */
    _tx_thread_current_ptr =  FX_NULL;
    
    /* Check for the error.  */
    if (status != FX_CALLER_ERROR)
    {

        /* Increment the failed tests counter.  */    
        test_control_failed_tests++;
        
        /* Print error message.  */
        printf("Error in checking of _fxe_directory_local_path_get_copy!\n");
    }
#endif
    /* Restore the system state.  */
    _tx_thread_system_state =  saved_system_state;

    /* Create the test control thread.  */
    tx_thread_create(&test_control_thread, "test control thread", test_control_thread_entry, 0,  
            test_control_thread_stack, TEST_STACK_SIZE, 
            0, 0, TX_NO_TIME_SLICE, TX_AUTO_START);

#ifndef NETXTEST_TIMEOUT_DISABLE
    /* Create the test control semaphore.  */
    tx_semaphore_create(&test_control_sema, "Test control semaphore", 0);
#endif

#else
    /* Skipping the error checking shell(caller error) tests for FileX Standalone  */
    test_control_thread_entry(0);
#endif /* FX_STANDALONE_ENABLE */
}



/* Define the test control thread.  This thread is responsible for dispatching all of the 
   tests in the ThreadX test suite.  */

void  test_control_thread_entry(ULONG thread_input)
{

UINT    i;

    FX_PARAMETER_NOT_USED(thread_input);

    /* Loop to process all tests...  */
    i =  0;
    while (test_control_tests[i].test_entry != FX_NULL)
    {

        /* Dispatch the test.  */
        (test_control_tests[i++].test_entry)(test_free_memory_ptr);
#ifndef FX_STANDALONE_ENABLE
#ifdef TEST_TIMEOUT_DISABLE
        /* Suspend control test to allow test to run.  */
        tx_thread_suspend(&test_control_thread);
#else
        if(tx_semaphore_get(&test_control_sema, test_control_tests[i - 1].timeout))
        {
    
            /* Test case timeouts. */
            printf("ERROR!\n");
            test_control_failed_tests++;

        }
#endif
#endif
        /* Test finished, cleanup in preparation for the next test.  */
        test_control_cleanup();

#ifdef BATCH_TEST
        fflush(stdout);
#endif
    }

    /* Finished with all tests, print results and return!  */
    printf("**** Testing Complete ****\n");
    printf("**** Test Summary:  Tests Passed:  %lu   Tests Failed:  %lu\n", test_control_successful_tests, test_control_failed_tests);

#ifdef BATCH_TEST
    exit((INT)test_control_failed_tests);
#endif
}


void  test_control_return(UINT status)
{
#ifndef FX_STANDALONE_ENABLE
UINT    old_posture =  TX_INT_ENABLE;
#endif

    /* Save the status in a global.  */
    test_control_return_status =  status;

    /* Ensure interrupts are enabled.  */
#ifndef FX_STANDALONE_ENABLE
    old_posture =  tx_interrupt_control(TX_INT_ENABLE);
#endif
    /* Determine if it was successful or not.  */
    /* Define 254 as a special error code without output. */
#ifndef FX_STANDALONE_ENABLE
    if (((status > 0) && (status < 254)) || (_tx_thread_preempt_disable) || (old_posture == TX_INT_DISABLE)) 
#else
    if((status > 0) && (status < 254))
#endif 
    {
        printf("                                                             **** ERROR CODE: %u *****\n", status);
        test_control_failed_tests++;
    }
    else if (status == 0)
        test_control_successful_tests++;
    else if (status == 254)
        test_control_failed_tests++;
    else if (status == 255)
        test_control_na_tests++;

#ifndef FX_STANDALONE_ENABLE
#ifdef NETXTEST_TIMEOUT_DISABLE
    /* Resume the control thread to fully exit the test.  */
    tx_thread_resume(&test_control_thread);
#else
    tx_semaphore_put(&test_control_sema);
#endif

#else

    /* Finished with all tests, print results and return!  */
    printf("**** Testing Complete ****\n");
    printf("**** Test Summary:  Tests Passed:  %lu   Tests Failed:  %lu\n", test_control_successful_tests, test_control_failed_tests);
    
    pthread_exit(NULL);
#endif
}


void  test_control_cleanup(void)
{
#ifndef FX_STANDALONE_ENABLE

TX_MUTEX        *mutex_ptr;
TX_THREAD       *thread_ptr;


    /* Delete all FX_MEDIA instances.   */
    while (_fx_system_media_opened_ptr)
    {

        /* Close all media instances.  */
        fx_media_close(_fx_system_media_opened_ptr);
    }

    /* Delete all queues.  */
    while(_tx_queue_created_ptr)
    {

        /* Delete queue.  */
        tx_queue_delete(_tx_queue_created_ptr);
    }

    /* Delete all semaphores.  */
    while(_tx_semaphore_created_ptr)
    {

#ifndef TEST_TIMEOUT_DISABLE
        if(_tx_semaphore_created_ptr != &test_control_sema)
        {

        /* Delete semaphore.  */
        tx_semaphore_delete(_tx_semaphore_created_ptr);
    }
        else if(_tx_semaphore_created_count == 1)
            break;
        else
        {
            /* Delete semaphore.  */
            tx_semaphore_delete(_tx_semaphore_created_ptr -> tx_semaphore_created_next);
        }
#else
        /* Delete semaphore.  */
        tx_semaphore_delete(_tx_semaphore_created_ptr);
#endif
    }

    /* Delete all event flag groups.  */
    while(_tx_event_flags_created_ptr)
    {

        /* Delete event flag group.  */
        tx_event_flags_delete(_tx_event_flags_created_ptr);
    }

    /* Delete all byte pools.  */
    while(_tx_byte_pool_created_ptr)
    {

        /* Delete byte pool.  */
        tx_byte_pool_delete(_tx_byte_pool_created_ptr);
    }

    /* Delete all block pools.  */
    while(_tx_block_pool_created_ptr)
    {

        /* Delete block pool.  */
        tx_block_pool_delete(_tx_block_pool_created_ptr);
    }

    /* Delete all timers.  */
    while(_tx_timer_created_ptr)
    {

        /* Deactivate timer.  */
        tx_timer_deactivate(_tx_timer_created_ptr);

        /* Delete timer.  */
        tx_timer_delete(_tx_timer_created_ptr);
    }

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

    /* Delete all threads, except for timer thread, and test control thread.  */
    while (_tx_thread_created_ptr)
    {

        /* Setup working pointer.  */
        thread_ptr =  _tx_thread_created_ptr;


#ifdef TX_TIMER_PROCESS_IN_ISR

        /* Determine if there are more threads to delete.  */
        if (_tx_thread_created_count == 1)
            break;

        /* Determine if this thread is the test control thread.  */
        if (thread_ptr == &test_control_thread)
        {

            /* Move to the next thread pointer.  */
            thread_ptr =  thread_ptr -> tx_thread_created_next;
//            thread_ptr =  thread_ptr -> tx_created_next;
        }
#else

        /* Determine if there are more threads to delete.  */
        if (_tx_thread_created_count == 2)
            break;

        /* Move to the thread not protected.  */
        while ((thread_ptr == &_tx_timer_thread) || (thread_ptr == &test_control_thread))
        {

            /* Yes, move to the next thread.  */
            thread_ptr =  thread_ptr -> tx_thread_created_next;
//            thread_ptr =  thread_ptr -> tx_created_next;
        }
#endif

        /* First terminate the thread to ensure it is ready for deletion.  */
        tx_thread_terminate(thread_ptr);

        /* Delete the thread.  */
        tx_thread_delete(thread_ptr);
    }

    /* At this point, only the test control thread and the system timer thread and/or mutex should still be
       in the system.  */
#endif
}


