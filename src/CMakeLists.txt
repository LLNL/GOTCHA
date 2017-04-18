add_library(mein_got SHARED  gotcha_utils.c gotcha.c gotcha_auxv.c)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
install(TARGETS mein_got DESTINATION lib)
add_subdirectory(example)
