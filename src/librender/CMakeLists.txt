
set(INC_DIR ../../include/misaki/render)

add_library(misaki-render SHARED
        ${INC_DIR}/platform.h
        ${INC_DIR}/fwd.h
        component.cpp
        ${INC_DIR}/logger.h logger.cpp
        ${INC_DIR}/properties.h properties.cpp
        ${INC_DIR}/plugin.h plugin.cpp
        ${INC_DIR}/camera.h camera.cpp
        )
set_target_properties(misaki-render PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/dist"
        RUNTIME_OUTPUT_DIRECTORY   "${CMAKE_BINARY_DIR}/dist")
target_compile_definitions(misaki-render PRIVATE -DMSK_EXPORTS)
target_compile_definitions(misaki-render PUBLIC -D_CRT_SECURE_NO_WARNINGS)
target_link_libraries(misaki-render PUBLIC
        misaki-utils
        fmt
        rttr_core
        )

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_custom_command(TARGET misaki-render POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/external/rttr/bin/rttr_core_d.dll"
            "${CMAKE_BINARY_DIR}/dist/rttr_core_d.dll")
else()
    add_custom_command(TARGET misaki-render POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/external/rttr/bin/rttr_core.dll"
            "${CMAKE_BINARY_DIR}/dist/rttr_core.dll")
endif()