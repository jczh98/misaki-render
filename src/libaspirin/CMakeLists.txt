
set(INC_DIR ../../include/nekoba/render)

add_library(aspirin-render SHARED
        component.cpp
        properties.cpp
        logger.cpp
        string.cpp
        xml.cpp
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
