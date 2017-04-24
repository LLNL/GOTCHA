set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-z -Wl,relro -Wl,-z,combreloc,-z,now")
add_library(libautotee2 SHARED autotee.c)
add_executable(autotee_test2 test_autotee.c)
target_link_libraries(libautotee2 gotcha)
target_link_libraries(autotee_test2 libautotee2)


