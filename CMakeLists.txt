cmake_minimum_required(VERSION 3.5.0)
project(nfs_v3 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(ENABLE_SHARED_LIBS "Enable building shared libraries" ON)

include_directories(
  include
  include/nfs
  include/rpc
  include/mount
)

function(generate_libs lib_name lib_src)
  foreach(src IN LISTS ${lib_src})
    if(NOT EXISTS ${src})
      message(FATAL_ERROR "Source file ${src} does not exist.")
    endif()
  endforeach()

  if(ENABLE_SHARED_LIBS)
    add_library(${lib_name} SHARED ${${lib_src}})
    set_target_properties(${lib_name} PROPERTIES OUTPUT_NAME ${lib_name})
    message(STATUS "Will Generated shared library: ${lib_name}")
  endif()

  add_library(${lib_name}_static STATIC ${${lib_src}})
  set_target_properties(${lib_name}_static PROPERTIES OUTPUT_NAME ${lib_name})
  message(STATUS "Will Generated static library: ${lib_name}_static")
endfunction()

set(RPC_SOURCE_ROOT ${CMAKE_SOURCE_DIR}/src/rpc)
set(NFS_SOURCE_ROOT ${CMAKE_SOURCE_DIR}/src/nfs)
set(MOUNT_SOURCE_ROOT ${CMAKE_SOURCE_DIR}/src/mount)

set(RPC_SOURCE 
  ${RPC_SOURCE_ROOT}/rpc.cc
)

set(NFS_SOURCE 
  ${NFS_SOURCE_ROOT}/nfs.cc
)

set(MOUNT_SOURCE 
  ${MOUNT_SOURCE_ROOT}/mount.cc
)

generate_libs(rpc_v2 RPC_SOURCE)
generate_libs(nfs_v3 NFS_SOURCE)
generate_libs(mount MOUNT_SOURCE)

target_link_libraries(mount PRIVATE
  rpc_v2
)

target_link_libraries(nfs_v3 PRIVATE
  rpc_v2 mount
)

add_subdirectory(examples)
