add_library(libautotee SHARED autotee.c)
add_executable(autotee_test test_autotee.c)
target_link_libraries(libautotee mein_got)
target_link_libraries(autotee_test libautotee)


