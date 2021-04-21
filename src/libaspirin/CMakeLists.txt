
set(INC_DIR ../../include/nekoba/render)

add_library(aspirin-render SHARED
        string.cpp
        class.cpp
        object.cpp
        plugin.cpp
        properties.cpp
        logger.cpp
        imageblock.cpp
        bitmap.cpp
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
        utils.cpp
        phase.cpp
        volume.cpp
        medium.cpp

        samplers/independent.cpp
        bsdfs/diffuse.cpp bsdfs/twosided.cpp bsdfs/dielectric.cpp
        textures/srgb.cpp textures/checkerboard.cpp
        shapes/obj.cpp
        emitters/area.cpp emitters/constant.cpp
        sensors/perspective.cpp
        filters/gaussian.cpp
        films/rgbfilm.cpp
        # Integrators
        integrators/debug.cpp integrators/path.cpp
        integrators/volpath.cpp integrators/sppm.cpp
        # Phases
        phase/isotropic.cpp
        volume/constant3d.cpp
        media/homogeneous.cpp
        )

target_link_libraries(aspirin-render PUBLIC
        fmt::fmt
        spdlog::spdlog
        pugixml::pugixml
        TBB::tbb
        OpenImageIO::OpenImageIO OpenImageIO::OpenImageIO_Util
        embree
        ${STB_INCLUDE_DIRS}
        )

set_output_dir(aspirin-render)
