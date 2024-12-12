cmake_minimum_required(VERSION 3.5.0)
project(nfs_v3 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/Macros.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(ENABLE_SHARED_LIBS "Enable building shared libraries" ON)
option(ENABLE_RPC_XDR "Enable building rpc xdr sources" OFF)
option(ENABLE_TESTS "Enable building tests" ON)
option(ENABLE_LOGGING "Enable building logging" ON)

if(${ENABLE_TESTS})
  add_subdirectory(third_party/googletest)
  add_subdirectory(test)
endif()

if(${ENABLE_LOGGING})
  set(SPDLOG_BUILD_SHARED ON)
  add_subdirectory(third_party/spdlog)
endif()

include_directories(
  include
  include/nfs 
  include/rpc
  include/mount

  build/rpc
)

set(RPC_SOURCE_ROOT ${CMAKE_SOURCE_DIR}/src/rpc)
set(NFS_SOURCE_ROOT ${CMAKE_SOURCE_DIR}/src/nfs)
set(MOUNT_SOURCE_ROOT ${CMAKE_SOURCE_DIR}/src/mount)

rpcgen_files(mount rpc/mount.x ENABLE_RPC_XDR)
rpcgen_files(nfs_v3 rpc/nfs_v3.x ENABLE_RPC_XDR)

set(RPC_SOURCE 
  ${RPC_SOURCE_ROOT}/rpc.cc
  ${RPC_SOURCE_ROOT}/auth.cc
)

set(NFS_SOURCE 
  ${NFS_SOURCE_ROOT}/v3/nfs_v3.cc
)

set(MOUNT_SOURCE 
  ${MOUNT_SOURCE_ROOT}/v1/mount_v1.cc
  ${MOUNT_SOURCE_ROOT}/v3/mount_v3.cc
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
