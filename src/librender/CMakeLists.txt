
set(INC_DIR ../../include/misaki)

file(GLOB_RECURSE HEADERS 
        ${INC_DIR}/*.h
)

set(CORE_SRCS
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
        interaction.cpp
)

set(BSDF_SRCS
        bsdfs/diffuse.cpp
)

set(SAMPLER_SRCS
        samplers/independent.cpp
)

set(TEXTURE_SRCS
        textures/srgb.cpp 
        textures/checkerboard.cpp
)

set(SHAPE_SRCS
        shapes/obj.cpp
)

set(EMITTER_SRCS
        emitters/area.cpp 
        emitters/constant.cpp
)

set(SENSOR_SRCS
        sensors/perspective.cpp
)

set(FILTER_SRCS
        filters/gaussian.cpp
)

set(FILM_SRCS
        films/rgbfilm.cpp
)

set(INTEGRATOR_SRCS
        integrators/debug.cpp 
        #integrators/path.cpp
        #integrators/volpath.cpp 
        #integrators/sppm.cpp
        #integrators/photonmapper.cpp
)

set(PHASE_SRCS
        phase/isotropic.cpp
)

set(VOLUME_SRCS
        volume/constant3d.cpp
)

set(MEDIUM_SRCS
        media/homogeneous.cpp
)

source_group("Headers" FILES ${HEADERS})
source_group("Sources/Core" FILES ${CORE_SRCS})
source_group("Sources/Samplers" FILES ${SAMPLER_SRCS})
source_group("Sources/BSDFs" FILES ${BSDF_SRCS})
source_group("Sources/Textures" FILES ${TEXTURE_SRCS})
source_group("Sources/Shapes" FILES ${SHAPE_SRCS})
source_group("Sources/Emitters" FILES ${EMITTER_SRCS})
source_group("Sources/Sensors" FILES ${SENSOR_SRCS})
source_group("Sources/Filters" FILES ${FILTER_SRCS})
source_group("Sources/Films" FILES ${FILM_SRCS})
source_group("Sources/Integrators" FILES ${INTEGRATOR_SRCS})
source_group("Sources/Phases" FILES ${PHASE_SRCS})
source_group("Sources/Volumes" FILES ${VOLUME_SRCS})
source_group("Sources/Media" FILES ${MEDIUM_SRCS})

add_library(misaki-render SHARED
        ${HEADERS}
        ${CORE_SRCS}
        ${SAMPLER_SRCS}
        ${BSDF_SRCS}
        ${TEXTURE_SRCS}
        ${SHAPE_SRCS}
        ${EMITTER_SRCS}
        ${SENSOR_SRCS}
        ${FILTER_SRCS}
        ${FILM_SRCS}
        ${INTEGRATOR_SRCS}
        ${PHASE_SRCS}
        ${VOLUME_SRCS}
        ${MEDIUM_SRCS}
        )

target_link_libraries(misaki-render PUBLIC
        fmt::fmt
        spdlog::spdlog
        pugixml::pugixml
        TBB::tbb
        OpenImageIO::OpenImageIO OpenImageIO::OpenImageIO_Util
        embree
        ${STB_INCLUDE_DIRS}
        )

set_output_dir(misaki-render)
