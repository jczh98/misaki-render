
add_executable(misaki-cli main.cpp)
target_link_libraries(misaki-cli PRIVATE
        misaki-render)
set_target_properties(misaki-cli PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/dist"
        RUNTIME_OUTPUT_DIRECTORY   "${CMAKE_BINARY_DIR}/dist")