
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
        ${INC_DIR}/film.h film.cpp
        ${INC_DIR}/imageblock.h imageblock.cpp
        ${INC_DIR}/rfilter.h rfilter.cpp
        ${INC_DIR}/interaction.h interaction.cpp
        ${INC_DIR}/sampler.h sampler.cpp
        ${INC_DIR}/imageio.h imageio.cpp
        ${INC_DIR}/integrator.h integrator.cpp
        ${INC_DIR}/shape.h shape.cpp
        ${INC_DIR}/mesh.h mesh.cpp
        ${INC_DIR}/endpoint.h endpoint.cpp
        ${INC_DIR}/bsdf.h bsdf.cpp
        ${INC_DIR}/light.h light.cpp
        ${INC_DIR}/texture.h texture.cpp
        )
set_target_properties(misaki-render PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/dist"
        RUNTIME_OUTPUT_DIRECTORY   "${CMAKE_BINARY_DIR}/dist")
target_compile_definitions(misaki-render PRIVATE -DMSK_EXPORTS)
target_compile_definitions(misaki-render PUBLIC -D_CRT_SECURE_NO_WARNINGS)
SET(EMBREE_LIBRARY ${EMBREE_ROOT_DIR}/lib/embree3.lib)
target_link_libraries(misaki-render PUBLIC
        misaki-utils
        fmt::fmt-header-only
        RTTR::Core
        pugixml
        TBB::tbb
        OpenImageIO::OpenImageIO OpenImageIO::OpenImageIO_Util
        ${EMBREE_LIBRARY}
        )
