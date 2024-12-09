cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
cmake_policy(SET CMP0057 NEW)

project(samples LANGUAGES C)

set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../samples)

set(sample_files
    ${SOURCE_DIR}/demo_filex_nand_flash.c
    ${SOURCE_DIR}/demo_filex_nor_flash.c)

foreach(sample_file ${sample_files})
  get_filename_component(sample_file_name ${sample_file} NAME_WE)
  add_executable(${sample_file_name} ${sample_file})
  target_link_libraries(${sample_file_name} PRIVATE azrtos::filex)
  target_link_libraries(${sample_file_name} PRIVATE azrtos::levelx)
endforeach()
