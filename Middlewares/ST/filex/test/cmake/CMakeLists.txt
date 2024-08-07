cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0057 NEW)

project(threadx_test LANGUAGES C)

# Set build configurations
set(BUILD_CONFIGURATIONS
    default_build_coverage no_cache_build exfat_build exfat_no_cache_build
    exfat_no_check_build exfat_build_4096 no_cache_standalone_build
    fault_tolerant_build_coverage fault_tolerant_exfat_build no_check_build no_cache_fault_tolerant_build
    standalone_build_coverage exfat_standalone_build_coverage exfat_standalone_build_2048
    standalone_fault_tolerant_build_coverage exfat_standalone_fault_tolerant_build_coverage
    standalone_no_cache_fault_tolerant_build)
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
set(no_cache_build -DFX_DISABLE_CACHE -DFX_DISABLE_FAT_ENTRY_REFRESH)
set(fault_tolerant_build_coverage ${FX_FAULT_TOLERANT_DEFINITIONS})
# FX_EXFAT_SECTOR_SIZE is a test symbol only used to change exFAt format option
set(fault_tolerant_exfat_build ${FX_FAULT_TOLERANT_DEFINITIONS}
                               -DFX_ENABLE_EXFAT
                               -DFX_EXFAT_SECTOR_SIZE=4096
                               -DFX_EXFAT_MAX_CACHE_SIZE=FX_EXFAT_SECTOR_SIZE
                               -DTX_ENABLE_EVENT_TRACE)
set(exfat_build -DFX_ENABLE_EXFAT -DFX_EXFAT_SECTOR_SIZE=512
                               -DFX_EXFAT_MAX_CACHE_SIZE=FX_EXFAT_SECTOR_SIZE)
set(exfat_no_cache_build ${exfat_build} -DFX_DISABLE_CACHE)
set(exfat_build_4096 -DFX_ENABLE_EXFAT -DFX_EXFAT_SECTOR_SIZE=4096
                               -DFX_EXFAT_MAX_CACHE_SIZE=FX_EXFAT_SECTOR_SIZE)
set(exfat_no_check_build -DFX_ENABLE_EXFAT -DFX_DISABLE_ERROR_CHECKING -DFX_EXFAT_SECTOR_SIZE=2048
                               -DFX_EXFAT_MAX_CACHE_SIZE=FX_EXFAT_SECTOR_SIZE)
set(exfat_standalone_build_coverage -DFX_STANDALONE_ENABLE -DFX_ENABLE_EXFAT -DFX_EXFAT_SECTOR_SIZE=2048
                               -DFX_EXFAT_MAX_CACHE_SIZE=FX_EXFAT_SECTOR_SIZE)
set(exfat_standalone_fault_tolerant_build_coverage ${FX_FAULT_TOLERANT_DEFINITIONS}
                                                   -DFX_STANDALONE_ENABLE -DFX_ENABLE_EXFAT -DFX_EXFAT_SECTOR_SIZE=4096
                                                   -DFX_EXFAT_MAX_CACHE_SIZE=FX_EXFAT_SECTOR_SIZE)   
set(exfat_standalone_build_1024 -DFX_STANDALONE_ENABLE -DFX_ENABLE_EXFAT -DFX_EXFAT_SECTOR_SIZE=1024
                               -DFX_EXFAT_MAX_CACHE_SIZE=FX_EXFAT_SECTOR_SIZE)
set(standalone_build_coverage  -DFX_STANDALONE_ENABLE)
set(standalone_fault_tolerant_build_coverage ${FX_FAULT_TOLERANT_DEFINITIONS}
                                             -DFX_STANDALONE_ENABLE)       
set(no_cache_standalone_build -DFX_DISABLE_CACHE -DFX_STANDALONE_ENABLE)
set(no_check_build ${FX_COMPILE_DEFINITIONS} -DFX_DISABLE_ERROR_CHECKING)
set(no_cache_fault_tolerant_build ${no_cache_build} ${FX_FAULT_TOLERANT_DEFINITIONS})
set(standalone_no_cache_fault_tolerant_build ${no_cache_build} ${FX_FAULT_TOLERANT_DEFINITIONS} -DFX_STANDALONE_ENABLE)

add_compile_options(
  -m32
  -std=c99
  -ggdb
  -g3
  -gdwarf-2
  -fdiagnostics-color
  -Werror
  -DFX_REGRESSION_TEST
  ${${CMAKE_BUILD_TYPE}})
add_link_options(-m32)

if(CMAKE_BUILD_TYPE MATCHES ".*standalone.*")
  set(FX_STANDALONE_ENABLE
      ON
      CACHE BOOL "Enable Filex in standalone mode")
endif()

enable_testing()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../.. filex)
add_subdirectory(regression)
add_subdirectory(samples)

# Coverage
if(CMAKE_BUILD_TYPE MATCHES ".*_coverage")
  target_compile_options(filex PRIVATE -fprofile-arcs -ftest-coverage)
  target_link_options(filex PRIVATE -fprofile-arcs -ftest-coverage)
endif()

# Build ThreadX library once
if(NOT FX_STANDALONE_ENABLE)
    execute_process(COMMAND ${CMAKE_CURRENT_LIST_DIR}/run.sh build_libs)
    add_custom_target(build_libs ALL COMMAND ${CMAKE_CURRENT_LIST_DIR}/run.sh
                                             build_libs)
    add_dependencies(filex build_libs)
    target_include_directories(filex PUBLIC ${CMAKE_BINARY_DIR}/../libs/inc)
    add_library(threadx SHARED IMPORTED GLOBAL)
    add_library("azrtos::threadx" ALIAS threadx)
    set_target_properties(
      threadx PROPERTIES IMPORTED_LOCATION
                         ${CMAKE_BINARY_DIR}/../libs/threadx/libthreadx.so)
endif()

target_compile_options(
  filex
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

