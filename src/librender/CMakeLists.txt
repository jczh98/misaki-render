
set(INC_DIR ../../include/misaki/render)

add_library(misaki-render SHARED
        ${INC_DIR}/platform.h
        ${INC_DIR}/fwd.h
        component.cpp
        ${INC_DIR}/logger.h logger.cpp
        ${INC_DIR}/properties.h properties.cpp
        ${INC_DIR}/plugin.h plugin.cpp
        ${INC_DIR}/scene.h scene.cpp
        ${INC_DIR}/camera.h camera.cpp
        ${INC_DIR}/xml.h xml.cpp
        )
set_target_properties(misaki-render PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/dist"
        RUNTIME_OUTPUT_DIRECTORY   "${CMAKE_BINARY_DIR}/dist")
target_compile_definitions(misaki-render PRIVATE -DMSK_EXPORTS)
target_compile_definitions(misaki-render PUBLIC -D_CRT_SECURE_NO_WARNINGS)
target_link_libraries(misaki-render PUBLIC
        misaki-utils
        fmt::fmt
        RTTR::Core
        pugixml
        TBB::tbb
        )
