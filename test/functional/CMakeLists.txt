set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
add_executable(functional_app app.cpp)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -g")
target_link_libraries(functional_app gotcha)
add_test(test_functional functional_app)

