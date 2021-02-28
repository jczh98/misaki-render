add_executable(aspirin main.cpp)
target_link_libraries(aspirin PRIVATE aspirin-render)

set_output_dir(aspirin)