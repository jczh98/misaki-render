
set(INC_DIR ../../include/nekoba/render)

add_library(aspirin-render SHARED
        component.cpp
        properties.cpp
        logger.cpp
        string.cpp
        imageio.cpp
        imageblock.cpp
        xml.cpp
        endpoint.cpp
        camera.cpp
        rfilter.cpp
        film.cpp
        sampler.cpp
        shape.cpp
        mesh.cpp
        bsdf.cpp
        light.cpp
        scene.cpp
        texture.cpp
        integrator.cpp
        interaction.cpp

        cameras/perspective.cpp
        filters/gaussian.cpp
        films/rgbfilm.cpp
        samplers/independent.cpp
        shapes/obj.cpp
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
