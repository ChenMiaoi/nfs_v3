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
  if (${ENABLE_LOGGING})
    target_link_libraries(${lib_name}_static spdlog)
  endif()
  message(STATUS "Will Generated static library: ${lib_name}_static")
endfunction()

function(rpcgen_files file_name file_src enable_xdr)
  set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/rpc)
  if (NOT EXISTS ${OUTPUT_DIR})
    file(MAKE_DIRECTORY ${OUTPUT_DIR})
    message(STATUS "Created directory: ${OUTPUT_DIR}")
  else()
    message(STATUS "Directory already exists: ${OUTPUT_DIR}")
  endif()

  if (NOT EXISTS ${OUTPUT_DIR}/rpcgen_${file_name}.h)
    execute_process(
      COMMAND rpcgen -h ${file_src} -o ${OUTPUT_DIR}/rpcgen_${file_name}.h
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      RESULT_VARIABLE RPCGEN_RESULT
      ERROR_VARIABLE RPCGEN_ERROR
    )
    if (NOT RPCGEN_RESULT EQUAL 0)
      message(FATAL_ERROR "rpcgen failed: ${RPCGEN_ERROR}")
    endif()

    message(STATUS "rpcgen generated header files in ${OUTPUT_DIR}:")
    message(STATUS " - rpcgen_${file_name}.h")
  else()
    message(STATUS "already has header files in ${OUTPUT_DIR}:")
    message(STATUS " - rpcgen_${file_name}.h")
  endif()

  file(READ ${OUTPUT_DIR}/rpcgen_${file_name}.h FILE_CONTENT)
  string(REPLACE "CLIENT" "void" FILE_CONTENT "${FILE_CONTENT}")
  string(REPLACE "bool_t" "uint32_t" FILE_CONTENT "${FILE_CONTENT}")
  string(REPLACE "u_int" "uint32_t" FILE_CONTENT "${FILE_CONTENT}")
  string(REPLACE "SVCXPRT" "void" FILE_CONTENT "${FILE_CONTENT}")
  string(REPLACE "xdr" "zdr" FILE_CONTENT "${FILE_CONTENT}")
  string(REPLACE "XDR" "zdr_t" FILE_CONTENT "${FILE_CONTENT}")
  string(REPLACE "#include <rpc/rpc.h>" "#include <zdr/zdr.h>" FILE_CONTENT "${FILE_CONTENT}")

  file(WRITE ${OUTPUT_DIR}/rpcgen_${file_name}.h "${FILE_CONTENT}")
  message(STATUS "Modified rpcgen_${file_name}.h with custom replacements.")

  if (${enable_xdr}) 
    if (NOT EXISTS ${OUTPUT_DIR}/rpcgen_${file_name}_xdr.cc)
      execute_process(
        COMMAND rpcgen -c ${file_src} -o ${OUTPUT_DIR}/rpcgen_${file_name}_xdr.cc
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE RPCGEN_RESULT
        ERROR_VARIABLE RPCGEN_ERROR
      )
      if (NOT RPCGEN_RESULT EQUAL 0)
        message(FATAL_ERROR "rpcgen failed: ${RPCGEN_ERROR}")
      endif()

      message(STATUS "rpcgen generated header files in ${OUTPUT_DIR}:")
      message(STATUS " - rpcgen_${file_name}_xdr.cc")
    else()
      message(STATUS "already has source files in ${OUTPUT_DIR}:")
      message(STATUS " - rpcgen_${file_name}_xdr.cc")
    endif()
  endif()
endfunction()

function(enable_test_module module_name)
  set(TEST_ROOT ${CMAKE_SOURCE_DIR}/test)
  foreach(mod_name LISTS IN ${ARGN})
    file(GLOB_RECURSE MODULE_SOURCE ${TEST_ROOT}/${module_name}/*.cc)
    foreach(mod_src ${MODULE_SOURCE})
      get_filename_component(filename ${mod_src} NAME_WE)
      set(test_exe test_${module_name}_${filename})
      add_executable(${test_exe} ${mod_src})
      target_link_libraries(${test_exe} PRIVATE nfs_v3 rpc_v2 mount gtest gtest_main pthread)
      target_link_directories(${test_exe} PRIVATE ${CMAKE_BINARY_DIR}/lib)
      include_directories(PRIAVTE ${CMAKE_SOURCE_DIR}/include)
      add_test(NAME ${test_exe} COMMAND ${test_exe})

      message(STATUS  "Generate TEST")
      message(STATUS " - ${test_exe}")
    endforeach()
  endforeach()
endfunction()
