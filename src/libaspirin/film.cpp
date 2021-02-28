#include <aspirin/film.h>
#include <aspirin/imageblock.h>
#include <aspirin/logger.h>
#include <aspirin/properties.h>

namespace aspirin {

template <typename Spectrum> Film<Spectrum>::Film(const Properties &props) {
    m_size = { props.get_int("width", 640), props.get_int("height", 320) };
    for (auto &kv : props.components()) {
        auto rfilter =
            std::dynamic_pointer_cast<ReconstructionFilter>(kv.second);
        if (rfilter) {
            if (m_filter)
                Throw("A film can only have one filter");
            m_filter = rfilter;
        } else {
            Throw("Tried to add an unsupported component of type {}",
                  kv.second->to_string());
        }
    }
    if (!m_filter) {
        m_filter = ComponentManager::instance()->create_instance<ReconstructionFilter>(
            Properties("gaussian"));
    }
}

template <typename Spectrum> void Film<Spectrum>::put(const ImageBlock *block) {
    ARP_NOT_IMPLEMENTED("put");
}

template <typename Spectrum>
void Film<Spectrum>::set_destination_file(const fs::path &filename) {
    ARP_NOT_IMPLEMENTED("set_destination_file");
}

template <typename Spectrum> void Film<Spectrum>::develop() {
    ARP_NOT_IMPLEMENTED("develop");
}

template <typename Spectrum> std::string Film<Spectrum>::to_string() const {
    std::ostringstream oss;
    oss << "Film[" << std::endl
        << "  size = " << m_size << "," << std::endl
        << "  m_filter = " << m_filter->to_string() << std::endl
        << "]";
    return oss.str();
}

} // namespace aspirin