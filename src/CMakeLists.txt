
set(GOTCHA_SOURCES
    gotcha_utils.c
    gotcha.c
    gotcha_auxv.c
    libc_wrappers.c
    elf_ops.c
    hash.c
    tool.c
    library_filters.c
    gotcha_dl.c
    translations.c
)

# create an interface to these headers so they show up in an IDE
file(GLOB GOTCHA_INTERNAL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
add_library(gotcha-internal-headers INTERFACE)
target_sources(gotcha-internal-headers INTERFACE ${GOTCHA_INTERNAL_HEADERS})
target_include_directories(gotcha-internal-headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# build the shared library
add_library(gotcha SHARED ${GOTCHA_SOURCES})

# "link" to interface libraries
target_link_libraries(gotcha
    PUBLIC  gotcha-include      # other CMake projects that "link" to GOTCHA library will get include path
    PRIVATE                     # other projects will not see these build settings
        gotcha-compile-options
        gotcha-internal-headers
        gotcha-testing)

# miscellaneous output settings
set_target_properties(gotcha PROPERTIES
    SOVERSION ${LIBTOOL_INTERFACE}
    VERSION "${LIBTOOL_INTERFACE}.${LIBTOOL_REVISION}.${LIBTOOL_AGE}")

# if testing is enabled
if(GOTCHA_ENABLE_TESTS)
    add_library(gotcha_no_libc SHARED ${GOTCHA_SOURCES})
    target_link_libraries(gotcha_no_libc PUBLIC gotcha-include gotcha-compile-options gotcha-testing)
    target_compile_definitions(gotcha_no_libc PRIVATE FORCE_NO_LIBC)
    set_target_properties(gotcha_no_libc PROPERTIES SOVERSION ${LIBTOOL_INTERFACE})
    set_target_properties(gotcha_no_libc PROPERTIES VERSION "${LIBTOOL_INTERFACE}.${LIBTOOL_REVISION}.${LIBTOOL_AGE}")
endif()

# since "gotcha" target has "gotcha-include" target as PUBLIC, must export "gotcha-include" target
# "gotcha-compile-options" is exported for use in other projects, e.g.
#
#   find_package(gotcha)
#   add_library(mylib SHARED ...)
#   target_link_libraries(mylib PRIVATE gotcha gotcha-compile-options)
#
install(
    TARGETS     gotcha
    EXPORT      gotcha-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
    EXPORT      gotcha-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gotcha)

add_subdirectory(example)
