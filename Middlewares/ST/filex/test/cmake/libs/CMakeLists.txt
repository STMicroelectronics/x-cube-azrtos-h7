cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(libs LANGUAGES C)

if(ENABLE_64)
  message(STATUS "Building for 64bit")
else()
  add_compile_options(-m32)
  add_link_options(-m32)
  message(STATUS "Building for 32bit")
endif()
message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}.")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../threadx threadx)
target_compile_options(threadx PRIVATE -DTX_ENABLE_EVENT_TRACE -DTX_LINUX_NO_IDLE_ENABLE)

get_target_property(dirs threadx INCLUDE_DIRECTORIES)
execute_process(COMMAND mkdir -p ${CMAKE_BINARY_DIR}/inc)
foreach(dir ${dirs})
  file(GLOB header_files ${dir}/*.h)
  foreach(header_file ${header_files})
    execute_process(COMMAND ln -sf ${header_file} ${CMAKE_BINARY_DIR}/inc)
  endforeach()
endforeach()
