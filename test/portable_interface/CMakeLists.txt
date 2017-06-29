set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-z -Wl,relro -Wl,-z,combreloc,-z,now")
add_library(portable_autotee SHARED autotee.c)
add_executable(portable_autotee_test test_autotee.c)
target_link_libraries(portable_autotee gotcha)
target_link_libraries(portable_autotee_test autotee)
add_test(test_portable_interface autotee_test)

