option(GOTCHA_ENABLE_TESTS "Enable internal tests" Off)
option(GOTCHA_ENABLE_COVERAGE_TESTS "Enable coverage tests" Off)
project(gotcha)
include(CheckCXXCompilerFlag)
include(cmake/gotcha.cmake)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)

set(LIBTOOL_INTERFACE 1)
set(LIBTOOL_REVISION 0)
set(LIBTOOL_AGE 1)

set(DEFAULT_SYMBOL_VISIBILITY hidden)

if(GOTCHA_ENABLE_TESTS)
  enable_testing()
  set(DEFAULT_SYMBOL_VISIBILITY default)
  add_definitions("-DGOTCHA_ENABLE_TESTING")
  if(GOTCHA_ENABLE_COVERAGE_TESTS)
    add_definitions("-DGOTCHA_ENABLE_COVERAGE_TESTING")
  endif()
endif()
cmake_minimum_required(VERSION 2.8)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_subdirectory(include)
add_subdirectory(src)
if(GOTCHA_ENABLE_TESTS)
add_subdirectory(test)
setup_coverage_target()
endif()

include(CMakePackageConfigHelpers)

# Configure gotcha-config-version.cmake
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/gotcha-config-version.cmake"
  VERSION ${LIBTOOL_INTERFACE}.${LIBTOOL_REVISION}.${LIBTOOL_AGE}
  COMPATIBILITY SameMajorVersion)

set(gotcha_INSTALL_INCLUDE_DIR include/)

# Configure gotcha-config.cmake
configure_package_config_file(
  "cmake/config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/gotcha-config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gotcha"
  PATH_VARS gotcha_INSTALL_INCLUDE_DIR)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/gotcha-config.cmake" "${CMAKE_CURRENT_BINARY_DIR}/gotcha-config-version.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gotcha")
