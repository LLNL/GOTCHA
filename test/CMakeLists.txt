
# by checking here instead of parent CMakeLists.txt, this CMakeLists.txt will show up in IDEs
if(NOT GOTCHA_ENABLE_TESTS)
    return()
endif()

include(gotcha_testing)
setup_coverage_target()

add_subdirectory(rogot)
add_subdirectory(unit)
add_subdirectory(dlopen)
add_subdirectory(stack)
add_subdirectory(priority)
add_subdirectory(multi_agent_dlopen)
add_subdirectory(ppc_stress_multi_module)
add_subdirectory(wrap_main)

if(CMAKE_VERSION VERSION_LESS 3.3)
    set(_MSG "\n Current CMake version (== ${CMAKE_VERSION}) is quite old...\n")
    set(_MSG "${_MSG} Probability suggests that an outdated CMake would correlate to an outdated C++ compiler...\n")
    set(_MSG "${_MSG} Thus, it would be probable that the C++ compiler does not support C++11...\n")
    set(_MSG "${_MSG} Checking for C++11 in an compiler-agnostic manner requires CMake >= 3.3...\n")
    set(_MSG "${_MSG} Disabling testing C++ example as a result...\n")
    message(AUTHOR_WARNING "${_MSG}")
    return()
endif()

# enables if(... IN_LIST ...)
cmake_policy(SET CMP0057 NEW)

include(CheckLanguage)

check_language(CXX)         # look for working C++ compiler
if(CMAKE_CXX_COMPILER)      # if compiler found
    enable_language(CXX)    # enable C++
    # get the known features
    get_property(CXX_KNOWN_FEATURES GLOBAL PROPERTY CMAKE_CXX_KNOWN_FEATURES)
    # if C++11 is supported, add subdirectory
    if("cxx_std_11" IN_LIST CXX_KNOWN_FEATURES)
        set(CMAKE_CXX_STANDARD 11)
        set(CMAKE_CXX_EXTENSIONS ON)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
        add_subdirectory(hammer)
    endif()
endif()
