
set(INC_DIR ../../include/nekoba/render)

add_library(nekoba-render SHARED
        reflect.cpp
        logger.cpp
        )

target_link_libraries(nekoba-render PUBLIC
        spdlog::spdlog
        pugixml::pugixml
        TBB::tbb
        OpenImageIO::OpenImageIO OpenImageIO::OpenImageIO_Util
        embree
        )
