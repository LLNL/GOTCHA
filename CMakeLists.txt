# require modern CMake (> 3.0) because modern CMake provides a lot more features
# using version 3.1.3 (instead of just 3.0) for:
#   - CMAKE_CXX_KNOWN_FEATURES used in testing
#
cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)

# for VERSION and LANGUAGES in project(...)
cmake_policy(SET CMP0048 NEW)

set(LIBTOOL_INTERFACE 1)
set(LIBTOOL_REVISION 0)
set(LIBTOOL_AGE 1)

project(gotcha VERSION ${LIBTOOL_INTERFACE}.${LIBTOOL_REVISION}.${LIBTOOL_AGE} LANGUAGES C)

# CMake options
option(GOTCHA_ENABLE_TESTS "Enable internal tests" OFF)
option(GOTCHA_ENABLE_COVERAGE_TESTS "Enable coverage tests" OFF)
option(GOTCHA_BUILD_EXAMPLES "Enable building the examples" ON)

# built-in includes
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# When non-cached, applies these settings to this directory and subdirs
# regardless of the settings of another project that might include gotcha as subproject
set(CMAKE_C_EXTENSIONS ON) # enable GNU-extensions
set(CMAKE_C_STANDARD_REQUIRED ON) # require standard

# give precedence to gotcha cmake/ folder
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# these "libraries" hold build settings and other targets can "link" to them in order to inherit their build settings
add_library(gotcha-testing          INTERFACE)
add_library(gotcha-include          INTERFACE)
add_library(gotcha-compile-options  INTERFACE)

# default to hidden
set(DEFAULT_SYMBOL_VISIBILITY hidden)
# if testing, change visibility to default
if(GOTCHA_ENABLE_TESTS)
    enable_testing()
    set(DEFAULT_SYMBOL_VISIBILITY default)
endif()

# provides "GOTCHA_ENABLE_TESTING" definition when GOTCHA_ENABLE_TEST=ON
# provides "GOTCHA_ENABLE_COVERAGE_TESTING" definition when GOTCHA_ENABLE_COVERAGE_TESTS=ON
target_compile_definitions(gotcha-testing INTERFACE
    $<$<BOOL:${GOTCHA_ENABLE_TESTS}>:GOTCHA_ENABLE_TESTING>
    $<$<BOOL:${GOTCHA_ENABLE_COVERAGE_TESTS}>:GOTCHA_ENABLE_COVERAGE_TESTING>)

# set the compile flags
target_compile_options(gotcha-compile-options INTERFACE "-fvisibility=${DEFAULT_SYMBOL_VISIBILITY}")
# set the link flags
set_target_properties(gotcha-compile-options PROPERTIES
    INTERFACE_LINK_FLAGS "-fvisibility=${DEFAULT_SYMBOL_VISIBILITY}")

# set gotcha-include to provide one path when building but another after installation
target_include_directories(gotcha-include INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# export in same directory
install(TARGETS gotcha-include gotcha-compile-options
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        EXPORT gotcha-targets)

# add the subdirectories
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(test)      # check for GOTCHA_ENABLE_TESTS in this dir's CMakeLists.txt

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

# install configuration
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/gotcha-config.cmake" "${CMAKE_CURRENT_BINARY_DIR}/gotcha-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gotcha")
