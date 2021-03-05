
set(INC_DIR ../../include/nekoba/render)

add_library(aspirin-render SHARED
        string.cpp
        class.cpp
        object.cpp
        plugin.cpp
        properties.cpp
        logger.cpp
        imageblock.cpp
        imageio.cpp
        fresolver.cpp
        xml.cpp
        endpoint.cpp
        sensor.cpp
        emitter.cpp
        rfilter.cpp
        film.cpp
        bsdf.cpp
        shape.cpp
        mesh.cpp
        sampler.cpp
        integrator.cpp
        scene.cpp
        texture.cpp

        samplers/independent.cpp
        bsdfs/diffuse.cpp
        textures/srgb.cpp
        shapes/obj.cpp
        emitters/area.cpp
        sensors/perspective.cpp
        filters/gaussian.cpp
        films/rgbfilm.cpp
        integrators/debug.cpp
        )

target_link_libraries(aspirin-render PUBLIC
        fmt::fmt
        spdlog::spdlog
        pugixml::pugixml
        TBB::tbb
        OpenImageIO::OpenImageIO OpenImageIO::OpenImageIO_Util
        embree
        )

set_output_dir(aspirin-render)
