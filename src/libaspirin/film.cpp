#include <aspirin/film.h>
#include <aspirin/imageblock.h>
#include <aspirin/logger.h>
#include <aspirin/plugin.h>
#include <aspirin/properties.h>

namespace aspirin {

template <typename Float, typename Spectrum>
Film<Float, Spectrum>::Film(const Properties &props) {
    m_size = { props.get_int("width", 640), props.get_int("height", 320) };
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

template <typename Float, typename Spectrum> Film<Float, Spectrum>::~Film() {}

template <typename Float, typename Spectrum>
void Film<Float, Spectrum>::put(const ImageBlock *block) {
    APR_NOT_IMPLEMENTED("put");
}

template <typename Float, typename Spectrum>
void Film<Float, Spectrum>::set_destination_file(const fs::path &filename) {
    APR_NOT_IMPLEMENTED("set_destination_file");
}

template <typename Float, typename Spectrum>
void Film<Float, Spectrum>::develop() {
    APR_NOT_IMPLEMENTED("develop");
}

template <typename Float, typename Spectrum>
std::string Film<Float, Spectrum>::to_string() const {
    std::ostringstream oss;
    oss << "Film[" << std::endl
        << "  size = " << m_size << "," << std::endl
        << "  m_filter = " << m_filter->to_string() << std::endl
        << "]";
    return oss.str();
}

APR_IMPLEMENT_CLASS_VARIANT(Film, Object, "film")
APR_INSTANTIATE_CLASS(Film)

} // namespace aspirin