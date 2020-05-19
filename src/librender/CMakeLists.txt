
set(INC_DIR ../../include/misaki/render)

add_library(misaki-render SHARED
        ${INC_DIR}/platform.h
        ${INC_DIR}/fwd.h
        component.cpp)
target_link_libraries(misaki-render PUBLIC
        misaki-utils)