cmake_minimum_required(VERSION 3.10)

project(mymuduo VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(Threads REQUIRED)

file(GLOB MYMUDUO_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
file(GLOB MYMUDUO_HEADERS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/include/mymuduo/*.h"
)

add_library(mymuduo SHARED ${MYMUDUO_SOURCES})
add_library(mymuduo::mymuduo ALIAS mymuduo)

target_link_libraries(mymuduo PUBLIC Threads::Threads)
target_include_directories(mymuduo
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mymuduo
)

set_target_properties(mymuduo PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 0
  PUBLIC_HEADER "${MYMUDUO_HEADERS}"
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/mymuduoConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/mymuduoConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/mymuduoConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mymuduo
)

install(TARGETS mymuduo
  EXPORT mymuduoTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mymuduo
)

install(EXPORT mymuduoTargets
  FILE mymuduoTargets.cmake
  NAMESPACE mymuduo::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mymuduo
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/mymuduoConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/mymuduoConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mymuduo
)

option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
  add_executable(echo_server examples/echo_server.cpp)
  target_link_libraries(echo_server PRIVATE mymuduo::mymuduo)
endif()