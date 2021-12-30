
set(INC_DIR ../../include/misaki)

file(GLOB_RECURSE CORE_HDRS
        ${INC_DIR}/core/*.h
)

file(GLOB_RECURSE RENDER_HDRS
        ${INC_DIR}/render/*.h
)

file(GLOB_RECURSE UI_HDRS
        ${INC_DIR}/ui/*.h
)

set(CORE_SRCS
        string.cpp
        class.cpp
        object.cpp
        properties.cpp
        logger.cpp
        imageblock.cpp
        bitmap.cpp
        fresolver.cpp
        xml.cpp
        #endpoint.cpp
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
        manager.cpp
        records.cpp
        image.cpp
        spectrum.cpp
        srgb.cpp
)

set(UI_SRCS
        ui/viewer.cpp
        ui/imgui.cpp
        ui/imgui_demo.cpp
        ui/imgui_draw.cpp
        ui/imgui_tables.cpp
        ui/imgui_widgets.cpp
        ui/imgui_impl_opengl3.cpp
        ui/imgui_impl_glfw.cpp
)

set(BSDF_SRCS
        bsdfs/diffuse.cpp
        #bsdfs/dielectric.cpp
        #bsdfs/twosided.cpp
        #bsdfs/roughconductor.cpp
        #bsdfs/roughdielectric.cpp
)

set(SAMPLER_SRCS
        samplers/independent.cpp
)

set(SPECTRUM_SRCS
        spectra/d65.cpp
        spectra/regular.cpp
        spectra/srgb.cpp
)

set(TEXTURE_SRCS
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
        films/hdrfilm.cpp
)

set(INTEGRATOR_SRCS
        integrators/aov.cpp 
        integrators/path.cpp
        #integrators/volpath.cpp 
        #integrators/sppm.cpp
        #integrators/photonmapper.cpp
)

set(PHASE_SRCS
        phase/isotropic.cpp
)

set(VOLUME_SRCS
        #volume/constant3d.cpp
)

set(MEDIUM_SRCS
        #media/homogeneous.cpp
)

source_group("Headers/Core" FILES ${CORE_HDRS})
source_group("Headers/Render" FILES ${RENDER_HDRS})
source_group("Headers/UI" FILES ${UI_HDRS})

source_group("Sources/Core" FILES ${CORE_SRCS})
source_group("Sources/UI" FILES ${UI_SRCS})

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
source_group("Sources/Spectra" FILES ${SPECTRUM_SRCS})

add_library(misaki-render SHARED
        ${CORE_HDRS}
        ${RENDER_HDRS}
        ${UI_HDRS}

        ${CORE_SRCS}
        ${UI_SRCS}

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
        ${SPECTRUM_SRCS}
        )

target_include_directories(misaki-render PUBLIC
        ${RGB2SPEC_INCLUDE_DIRS}
)

target_link_libraries(misaki-render PUBLIC
        fmt::fmt
        spdlog::spdlog
        pugixml::pugixml
        TBB::tbb
        OpenImageIO::OpenImageIO OpenImageIO::OpenImageIO_Util
        embree
        glfw
        OpenGL::GL
        rgb2spec
        ${STB_INCLUDE_DIRS}
        )
