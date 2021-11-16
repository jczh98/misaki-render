add_executable(misaki-cli main.cpp)
target_link_libraries(misaki-cli PRIVATE misaki-render)

set_output_dir(misaki-cli)