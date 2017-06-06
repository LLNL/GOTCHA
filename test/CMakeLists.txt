include(CheckCXXCompilerFlag)
add_subdirectory(rogot)
add_subdirectory(unit)
add_subdirectory(dlopen)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
add_subdirectory(hammer)
endif()

