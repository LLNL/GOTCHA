set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-z -Wl,relro -Wl,-z,combreloc,-z,now")
add_library(stacked_lib1 SHARED lib1.c)
add_library(stacked_tool1 SHARED tool1.c)
add_library(stacked_tool2 SHARED tool2.c)
add_executable(stacked_app app.c)
target_link_libraries(stacked_app gotcha)
target_link_libraries(stacked_app stacked_lib1 stacked_tool1 stacked_tool2)
add_test(test_stacked stacked_app)

