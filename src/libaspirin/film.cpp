#include <aspirin/film.h>
#include <aspirin/imageblock.h>
#include <aspirin/logger.h>
#include <aspirin/plugin.h>
#include <aspirin/properties.h>

namespace aspirin {


Film::Film(const Properties &props) {
    m_size = { props.int_("width", 640), props.int_("height", 320) };
    for (auto &[name, obj] : props.objects()) {
        auto *rfilter = dynamic_cast<ReconstructionFilter *>(obj.get());
        if (rfilter) {
            if (m_filter)
                Throw("A film can only have one filter");
            m_filter = rfilter;
        } else {
            Throw("Tried to add an unsupported component of type {}",
                  obj->to_string());
        }
    }
    if (!m_filter) {
        m_filter =
            PluginManager::instance()->create_object<ReconstructionFilter>(
                Properties("gaussian"));
    }
}

 Film::~Film() {}


void Film::put(const ImageBlock *block) {
    APR_NOT_IMPLEMENTED("put");
}


void Film::set_destination_file(const fs::path &filename) {
    APR_NOT_IMPLEMENTED("set_destination_file");
}


void Film::develop() {
    APR_NOT_IMPLEMENTED("develop");
}


std::string Film::to_string() const {
    std::ostringstream oss;
    oss << "Film[" << std::endl
        << "  size = " << m_size << "," << std::endl
        << "  m_filter = " << m_filter->to_string() << std::endl
        << "]";
    return oss.str();
}

APR_IMPLEMENT_CLASS(Film, Object, "film")

} // namespace aspirin