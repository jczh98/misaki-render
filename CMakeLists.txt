cmake_minimum_required(VERSION 3.17)
project(misaki-render)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/misaki-utils)
add_subdirectory(external/fmt)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /std:c++17")
    add_compile_options("/wd4819") # Eigen half
endif ()

include_directories(include)
add_subdirectory(src)