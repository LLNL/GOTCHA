add_library(num SHARED num.c)
add_executable(test_dlopen test_dlopen.c)
target_link_libraries(test_dlopen gotcha dl)
add_test(dlopen test_dlopen)
set_tests_properties(dlopen PROPERTIES
  ENVIRONMENT "GOTCHA_DEBUG=3"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

