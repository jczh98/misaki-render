cmake_minimum_required(VERSION 3.17)
project(misaki-render)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/misaki-utils)
add_subdirectory(external/fmt)
add_subdirectory(external/rttr)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /std:c++17")
    add_compile_options("/wd4819") # Eigen half
endif ()

function(add_plugin)
    list(GET ARGV 0 TARGET)
    list(REMOVE_AT ARGV 0)
    add_library(${TARGET} SHARED ${ARGV})
    set_property(TARGET ${TARGET} PROPERTY POSITION_INDEPENDENT_CODE ON)
    set_target_properties(${TARGET} PROPERTIES PREFIX "")
    set_target_properties(${TARGET} PROPERTIES
            #LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/dist/plugins/"
            #ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/dist/plugins/"
            RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/dist/plugins")
    target_link_libraries(${TARGET} misaki-render)
endfunction()
include_directories(include)
add_subdirectory(src)
