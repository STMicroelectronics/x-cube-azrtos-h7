cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0057 NEW)

project(levelx_test LANGUAGES C)

# Set build configurations
set(BUILD_CONFIGURATIONS default_build_coverage 
                         free_sector_verify_build 
                         full_build 
                         standalone_build
                         standalone_free_sector_verify_build
                         standalone_full_build
                         new_driver_interface_build
                         nor_obsolete_cache_build
                         nor_mapping_cache_build
                         nor_obsolete_mapping_cache_build)
set(CMAKE_CONFIGURATION_TYPES
    ${BUILD_CONFIGURATIONS}
    CACHE STRING "list of supported configuration types" FORCE)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
                                             ${CMAKE_CONFIGURATION_TYPES})
list(GET CMAKE_CONFIGURATION_TYPES 0 BUILD_TYPE)
if((NOT CMAKE_BUILD_TYPE) OR (NOT ("${CMAKE_BUILD_TYPE}" IN_LIST
                                   CMAKE_CONFIGURATION_TYPES)))
  set(CMAKE_BUILD_TYPE
      "${BUILD_TYPE}"
      CACHE STRING "Build Type of the project" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}.")
set(FX_FAULT_TOLERANT_DEFINITIONS
    -DFX_ENABLE_FAULT_TOLERANT -DFX_UPDATE_FILE_SIZE_ON_ALLOCATE
    -DFX_FAULT_TOLERANT_TRANSACTION_FAIL_FUNCTION)
set(default_build_coverage "")
set(free_sector_verify_build -DLX_FREE_SECTOR_DATA_VERIFY)
set(full_build -DLX_FREE_SECTOR_DATA_VERIFY
               -DLX_DIRECT_READ
               -DLX_NAND_FLASH_DIRECT_MAPPING_CACHE
               -DLX_NOR_DISABLE_EXTENDED_CACHE
               -DLX_THREAD_SAFE_ENABLE)
# For Standalone builds LX_STANADLONE_ENABLE is defined in line 61
set(standalone_build -DLX_STANDALONE_ENABLE)
set(standalone_free_sector_verify_build -DLX_STANDALONE_ENABLE ${free_sector_verify_build})
set(standalone_full_build -DLX_STANDALONE_ENABLE ${full_build})
set(new_driver_interface_build -DLX_NOR_ENABLE_CONTROL_BLOCK_FOR_DRIVER_INTERFACE
                               -DLX_NAND_ENABLE_CONTROL_BLOCK_FOR_DRIVER_INTERFACE)
set(nor_obsolete_cache_build   -DLX_NOR_ENABLE_OBSOLETE_COUNT_CACHE)
set(nor_mapping_cache_build -DLX_NOR_ENABLE_MAPPING_BITMAP)
set(nor_obsolete_mapping_cache_build -DLX_NOR_ENABLE_MAPPING_BITMAP
                               -DLX_NOR_ENABLE_OBSOLETE_COUNT_CACHE)

add_compile_options(
  -m32
  -std=c99
  -ggdb
  -g3
  -gdwarf-2
  -fdiagnostics-color
  -Werror
  ${${CMAKE_BUILD_TYPE}})
add_link_options(-m32)

enable_testing()

if(CMAKE_BUILD_TYPE MATCHES "standalone.*")
  set(LX_STANDALONE_ENABLE 
      ON 
      CACHE BOOL "LevelX standalone enabled(No Azure RTOS ThreadX)" FORCE)
  set(FX_STANDALONE_ENABLE 
      ON 
      CACHE BOOL "FileX standalone enabled(No Azure RTOS ThreadX)" FORCE)
endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../.. levelx)
add_subdirectory(regression)
add_subdirectory(samples)

# Coverage
if(CMAKE_BUILD_TYPE MATCHES ".*_coverage")
  target_compile_options(levelx PRIVATE -fprofile-arcs -ftest-coverage)
  target_link_options(levelx PRIVATE -fprofile-arcs -ftest-coverage)
endif()


# Build ThreadX library once
execute_process(COMMAND ${CMAKE_CURRENT_LIST_DIR}/run.sh build_libs)
add_custom_target(build_libs ALL COMMAND ${CMAKE_CURRENT_LIST_DIR}/run.sh
                                             build_libs)
add_dependencies(levelx build_libs)
target_include_directories(levelx PUBLIC ${CMAKE_BINARY_DIR}/../libs/inc)
if(NOT LX_STANDALONE_ENABLE)
    add_library(threadx SHARED IMPORTED GLOBAL)
    add_library("azrtos::threadx" ALIAS threadx)
    set_target_properties(
      threadx PROPERTIES IMPORTED_LOCATION
                         ${CMAKE_BINARY_DIR}/../libs/threadx/libthreadx.so)
    add_library(filex SHARED IMPORTED GLOBAL)
    add_library("azrtos::filex" ALIAS filex)
    set_target_properties(filex PROPERTIES IMPORTED_LOCATION
                   ${CMAKE_BINARY_DIR}/../libs/filex/libfilex.so)
else()
    get_filename_component(
    externals ${CMAKE_CURRENT_SOURCE_DIR} ABSOLUTE)
    add_subdirectory(${externals}/filex filex)
    add_library("azrtos::filex" ALIAS filex)
endif()

target_compile_options(
  levelx
  PRIVATE -Werror
          -Wall
          -Wextra
          -pedantic
          -fmessage-length=0
          -fsigned-char
          -ffunction-sections
          -fdata-sections
          -Wunused
          -Wuninitialized
          -Wmissing-declarations
          -Wconversion
          -Wpointer-arith
          -Wshadow
          -Wlogical-op
          -Waggregate-return
          -Wfloat-equal)
