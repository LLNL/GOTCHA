
add_subdirectory(unit)
add_subdirectory(dlopen)
add_subdirectory(stack)
add_subdirectory(priority)
add_subdirectory(ppc_stress_multi_module)
if(COMPILER_SUPPORTS_CXX11)
add_subdirectory(hammer)
endif()
add_subdirectory(rogot)
add_subdirectory(filter)
add_subdirectory(symver)

# FIXME: These tests fail currently in CI.
#add_subdirectory(multi_agent_dlopen)
#add_subdirectory(wrap_main)
