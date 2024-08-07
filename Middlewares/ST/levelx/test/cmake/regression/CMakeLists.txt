cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
cmake_policy(SET CMP0057 NEW)

project(regression_test LANGUAGES C)

set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../regression)

set(regression_test_cases
    ${SOURCE_DIR}/levelx_nand_flash_test.c
    ${SOURCE_DIR}/levelx_nor_flash_test.c
    ${SOURCE_DIR}/levelx_nor_flash_test_cache.c)

foreach(test_case ${regression_test_cases} ${regression_test_cases_exfat})
  get_filename_component(test_name ${test_case} NAME_WE)
  add_executable(${test_name} ${test_case})
  target_link_libraries(${test_name} PRIVATE azrtos::filex)
  target_link_libraries(${test_name} PRIVATE azrtos::levelx)
  target_compile_definitions(${test_name} PRIVATE BATCH_TEST)
  add_test(${CMAKE_BUILD_TYPE}::${test_name} ${test_name})
endforeach()
