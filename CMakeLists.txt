cmake_minimum_required(VERSION 3.17)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake
  CACHE STRING "Vcpkg toolchain file")
  
project(misaki-render)

set(CMAKE_CXX_STANDARD 17)

set(RENDER_DIR ${PROJECT_SOURCE_DIR}) 
set(RENDER_BIN_DIR ${RENDER_DIR}/dist) 

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${RENDER_BIN_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:RelWithDebInfo>:Release>$<$<CONFIG:MinSizeRel>:Release>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${RENDER_BIN_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:RelWithDebInfo>:Release>$<$<CONFIG:MinSizeRel>:Release>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${RENDER_BIN_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:RelWithDebInfo>:Release>$<$<CONFIG:MinSizeRel>:Release>)
  
find_package(Eigen3 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)
find_package(TBB CONFIG REQUIRED)
find_package(OpenImageIO CONFIG REQUIRED)
find_package(embree 3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")

option(MSK_ENABLE_EMBREE  "Use Embree for ray tracing intersection" ON)

if (MSK_ENABLE_EMBREE)
    add_compile_options(-DMSK_ENABLE_EMBREE)
endif ()

include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(include)

add_subdirectory(ext ext_build)

add_subdirectory(src)