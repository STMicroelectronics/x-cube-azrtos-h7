cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
cmake_policy(SET CMP0057 NEW)

project(regression_test LANGUAGES C)

set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../regression_test)

set(regression_test_cases
    ${SOURCE_DIR}/filex_directory_attributes_read_set_test.c
    ${SOURCE_DIR}/filex_directory_create_delete_test.c
    ${SOURCE_DIR}/filex_directory_default_path_get_set_test.c
    ${SOURCE_DIR}/filex_directory_duplicate_entries_test.c
    ${SOURCE_DIR}/filex_directory_first_next_find_test.c
    ${SOURCE_DIR}/filex_directory_local_path_test.c
    ${SOURCE_DIR}/filex_directory_long_short_get_test.c
    ${SOURCE_DIR}/filex_directory_naming_test.c
    ${SOURCE_DIR}/filex_directory_rename_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_corrupted_log_file_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_delete_large_data_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_directory_attributes_set_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_directory_attributes_set_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_directory_create_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_directory_create_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_directory_delete_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_directory_delete_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_directory_rename_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_directory_rename_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_enable_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_enable_1_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_enable_2_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_enable_3_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_enable_4_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_media_check_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_seek_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_random_seek_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_allocate_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_allocate_fat_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_allocate_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_allocate_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_allocate_undo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_attributes_set_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_attributes_set_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_best_allocate_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_best_allocate_fat_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_best_allocate_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_best_allocate_undo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_create_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_create_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_delete_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_delete_fat_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_delete_fat_multiple_sectors_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_delete_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_delete_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_delete_undo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_rename_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_rename_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_truncate_release_available_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_truncate_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_truncate_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_truncate_release_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_truncate_release_fat_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_truncate_release_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_truncate_release_undo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_write_available_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_write_data_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_write_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_write_fat_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_write_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_write_undo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_corruption_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_log_full_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_log_recover_directory_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_log_recover_fat_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_long_directory_create_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_long_directory_create_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_long_directory_rename_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_long_directory_rename_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_long_directory_rename_io_error_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_long_file_create_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_long_file_create_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_loop_write_data_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_media_full_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_write_large_data_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_write_large_data_fat_chain_cleanup_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_write_large_data_fat_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_write_large_data_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_write_large_data_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_write_large_data_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_write_large_data_undo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_recover_fail_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_unicode_directory_create_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_unicode_directory_create_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_unicode_directory_rename_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_unicode_directory_rename_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_unicode_file_create_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_unicode_file_create_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_unicode_file_rename_directory_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_unicode_file_rename_redo_log_interrupt_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_write_fat_crossover_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_write_fat_crossover_2_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_allocate_fat_crossover_test.c
    ${SOURCE_DIR}/filex_fault_tolerant_file_best_effort_allocate_fat_crossover_test.c
    ${SOURCE_DIR}/filex_file_allocate_truncate_test.c
    ${SOURCE_DIR}/filex_file_allocate_test.c
    ${SOURCE_DIR}/filex_file_attributes_read_set_test.c
    ${SOURCE_DIR}/filex_file_create_delete_test.c
    ${SOURCE_DIR}/filex_file_date_time_set_test.c
    ${SOURCE_DIR}/filex_file_naming_test.c
    ${SOURCE_DIR}/filex_file_read_write_test.c
    ${SOURCE_DIR}/filex_file_rename_test.c
    ${SOURCE_DIR}/filex_file_seek_test.c
    ${SOURCE_DIR}/filex_file_name_test.c
    ${SOURCE_DIR}/filex_media_abort_test.c
    ${SOURCE_DIR}/filex_media_cache_invalidate_test.c
    ${SOURCE_DIR}/filex_media_check_test.c
    ${SOURCE_DIR}/filex_media_flush_test.c
    ${SOURCE_DIR}/filex_media_format_open_close_test.c
    ${SOURCE_DIR}/filex_media_multiple_open_close_test.c
    ${SOURCE_DIR}/filex_media_read_write_sector_test.c
    ${SOURCE_DIR}/filex_media_volume_directory_entry_test.c
    ${SOURCE_DIR}/filex_media_volume_get_set_test.c
    ${SOURCE_DIR}/filex_media_hidden_sectors_test.c
    ${SOURCE_DIR}/filex_system_date_time_test.c
    ${SOURCE_DIR}/filex_unicode_directory_rename_test.c
    ${SOURCE_DIR}/filex_unicode_file_directory_rename_extra_test.c
    ${SOURCE_DIR}/filex_unicode_file_directory_rename_extra_2_test.c
    ${SOURCE_DIR}/filex_unicode_file_rename_test.c
    ${SOURCE_DIR}/filex_unicode_test.c
    ${SOURCE_DIR}/filex_unicode_2_test.c
    ${SOURCE_DIR}/filex_unicode_3_test.c
    ${SOURCE_DIR}/filex_unicode_4_test.c
    ${SOURCE_DIR}/filex_unicode_name_string_test.c
    ${SOURCE_DIR}/filex_unicode_directory_entry_test.c
    ${SOURCE_DIR}/filex_unicode_directory_entry_2_test.c
    ${SOURCE_DIR}/filex_unicode_directory_entry_change_test.c
    ${SOURCE_DIR}/filex_unicode_fat_entry_test.c
    ${SOURCE_DIR}/filex_unicode_fat_entry_1_test.c
    ${SOURCE_DIR}/filex_unicode_fat_entry_2_test.c
    ${SOURCE_DIR}/filex_unicode_fat_entry_3_test.c
    ${SOURCE_DIR}/filex_file_write_seek_test.c
    ${SOURCE_DIR}/filex_file_write_notify_test.c
    ${SOURCE_DIR}/filex_file_write_available_cluster_test.c
    ${SOURCE_DIR}/filex_utility_test.c
    ${SOURCE_DIR}/filex_utility_fat_flush_test.c)

if("-DFX_ENABLE_EXFAT" IN_LIST ${CMAKE_BUILD_TYPE})
  set(regression_test_cases_exfat
      ${SOURCE_DIR}/filex_directory_attributes_read_set_exfat_test.c
      ${SOURCE_DIR}/filex_directory_create_delete_exfat_test.c
      ${SOURCE_DIR}/filex_directory_default_path_get_set_exfat_test.c
      ${SOURCE_DIR}/filex_directory_first_next_find_exfat_test.c
      ${SOURCE_DIR}/filex_directory_local_path_exfat_test.c
      ${SOURCE_DIR}/filex_directory_naming_exfat_test.c
      ${SOURCE_DIR}/filex_directory_rename_exfat_test.c
      ${SOURCE_DIR}/filex_file_allocate_truncate_exfat_test.c
      ${SOURCE_DIR}/filex_file_attributes_read_set_exfat_test.c
      ${SOURCE_DIR}/filex_file_create_delete_exfat_test.c
      ${SOURCE_DIR}/filex_file_date_time_set_exfat_test.c
      ${SOURCE_DIR}/filex_file_naming_exfat_test.c
      ${SOURCE_DIR}/filex_file_read_write_exfat_test.c
      ${SOURCE_DIR}/filex_file_rename_exfat_test.c
      ${SOURCE_DIR}/filex_file_seek_exfat_test.c
      ${SOURCE_DIR}/filex_media_abort_exfat_test.c
      ${SOURCE_DIR}/filex_media_cache_invalidate_exfat_test.c
      ${SOURCE_DIR}/filex_media_check_exfat_test.c
      ${SOURCE_DIR}/filex_media_format_open_close_exfat_test.c
      ${SOURCE_DIR}/filex_media_close_with_file_opened_exfat_test.c
      ${SOURCE_DIR}/filex_media_flush_with_file_opened_exfat_test.c
      ${SOURCE_DIR}/filex_media_flush_exfat_test.c
      ${SOURCE_DIR}/filex_media_multiple_open_close_exfat_test.c
      ${SOURCE_DIR}/filex_media_read_write_sector_exfat_test.c
      ${SOURCE_DIR}/filex_media_volume_get_set_exfat_test.c
      ${SOURCE_DIR}/filex_bitmap_flush_exfat_test.c)
endif()

add_library(test_utility ${SOURCE_DIR}/fx_ram_driver_test.c
                         ${SOURCE_DIR}/filextestcontrol.c)
target_link_libraries(test_utility PUBLIC azrtos::filex)
target_compile_definitions(test_utility PUBLIC BATCH_TEST CTEST)

foreach(test_case ${regression_test_cases} ${regression_test_cases_exfat})
  get_filename_component(test_name ${test_case} NAME_WE)
  add_executable(${test_name} ${test_case})
  target_link_libraries(${test_name} PRIVATE test_utility)
  add_test(${CMAKE_BUILD_TYPE}::${test_name} ${test_name})
endforeach()
