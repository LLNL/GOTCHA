add_library(wrap_me SHARED sampleLib.c)
add_library(mein_got SHARED  gotcha_utils.c gnuy_lookups.c)
add_executable(symb_look symbolLookup.c)
target_link_libraries(symb_look wrap_me mein_got)
