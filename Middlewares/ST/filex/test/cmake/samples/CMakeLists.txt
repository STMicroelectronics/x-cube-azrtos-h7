cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
cmake_policy(SET CMP0057 NEW)

project(samples LANGUAGES C)

set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../samples)

set(sample_files
    ${SOURCE_DIR}/demo_filex.c)

foreach(sample_file ${sample_files})
  get_filename_component(sample_file_name ${sample_file} NAME_WE)
  add_executable(${sample_file_name} ${sample_file}
                 ${CMAKE_CURRENT_LIST_DIR}/../../regression_test/fx_ram_driver_test.c)
  target_compile_options(${sample_file_name} PRIVATE -DSAMPLE_BUILD)
  target_link_libraries(${sample_file_name} PRIVATE azrtos::filex)
endforeach()
