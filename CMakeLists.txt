cmake_minimum_required(VERSION 3.17)
project(nekoba)

set(CMAKE_CXX_STANDARD 17)

find_package(Eigen3 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)
find_package(TBB CONFIG REQUIRED)
find_package(OpenImageIO CONFIG REQUIRED)
find_package(embree 3 REQUIRED)

option(NEK_ENABLE_EMBREE  "Use Embree for ray tracing intersection" ON)

include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(include)

add_subdirectory(src)