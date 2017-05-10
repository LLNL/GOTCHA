add_library(internal_unit_sample SHARED testing_lib.c)
include(ExternalProject)
find_package(Git REQUIRED)
ExternalProject_Add(
    check_project
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    GIT_REPOSITORY https://github.com/libcheck/check
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/tpl"
    BUILD_COMMAND "make"
    INSTALL_COMMAND make install
    LOG_DOWNLOAD ON
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/tpl/include)
LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/tpl/lib)

find_library(check NAME libcheck.a)
add_executable(unit_tests gotcha_unit_tests.c)
target_link_libraries(unit_tests check gotcha internal_unit_sample m rt)
set_source_files_properties(testing_lib.c PROPERTIES LINK_FLAGS -Wl,--hash-style=sysv)
add_library(internal_unit_sample_elf SHARED testing_lib.c)
set_target_properties(internal_unit_sample_elf PROPERTIES LINK_FLAGS -Wl,--hash-style=sysv)
add_executable(unit_tests_elf gotcha_unit_tests.c)
target_link_libraries(unit_tests_elf check gotcha internal_unit_sample_elf m rt)
add_test(full_unit unit_tests)
