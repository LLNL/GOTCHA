add_library(thinwrappee SHARED wrappees.c)
add_executable(thinwrappers wrappers.c)
target_link_libraries(thinwrappers thinwrappee gotcha)
gotcha_add_test(thin thinwrappers)
