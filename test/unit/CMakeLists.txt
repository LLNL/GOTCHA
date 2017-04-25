add_library(internal_unit_sample SHARED testing_lib.c)
find_library(check NAME libcheck.a)
add_executable(unit_tests gotcha_unit_tests.c)
target_link_libraries(unit_tests check gotcha internal_unit_sample)
