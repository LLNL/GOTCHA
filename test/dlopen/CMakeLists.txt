add_library(num SHARED num.c)
add_executable(test_dlopen test_dlopen.c)
set_target_properties(test_dlopen
  PROPERTIES COMPILE_FLAGS "-DLIB_NAME_RAW=\"\"${CMAKE_CURRENT_BINARY_DIR}/libnum.so\"\""
  )
target_link_libraries(test_dlopen gotcha dl)
add_test(dlopen test_dlopen)
set_tests_properties(dlopen PROPERTIES
  ENVIRONMENT "GOTCHA_DEBUG=3 LIBNUM_DIR=${CMAKE_CURRENT_BINARY_DIR}"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

