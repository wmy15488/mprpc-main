cmake_minimum_required(VERSION 3.16)

project(mprpc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

option(MPRPC_USE_LOCAL_MYMUDUO "Link local mymuduo source tree" ON)
set(MYMUDUO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../muduo/mymuduo" CACHE PATH "Path to local mymuduo project")

find_package(Protobuf REQUIRED)
message(STATUS "Found protobuf ${Protobuf_VERSION}")

if(MPRPC_USE_LOCAL_MYMUDUO)
  if(EXISTS "${MYMUDUO_ROOT}/CMakeLists.txt")
    set(BUILD_EXAMPLES OFF CACHE BOOL "Disable mymuduo examples when building mprpc" FORCE)
    add_subdirectory("${MYMUDUO_ROOT}" third_party/mymuduo)
    if(TARGET mymuduo)
      set_target_properties(mymuduo PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/third_party/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/third_party/lib"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/third_party/lib"
      )
    endif()
    message(STATUS "Using local mymuduo from: ${MYMUDUO_ROOT}")
  else()
    message(FATAL_ERROR "mymuduo not found at ${MYMUDUO_ROOT}")
  endif()
endif()

add_subdirectory(proto)
add_subdirectory(src)
add_subdirectory(example)
add_subdirectory(test)
