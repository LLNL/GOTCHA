add_library(internal_unit_sample SHARED testing_lib.c)
include(ExternalProject)
find_package(Git REQUIRED)
ExternalProject_Add(
    check_project
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    GIT_REPOSITORY https://github.com/libcheck/check
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
)
find_library(check NAME libcheck.a)
add_executable(unit_tests gotcha_unit_tests.c)
target_link_libraries(unit_tests check gotcha internal_unit_sample)
