#include <misaki/film.h>
#include <misaki/imageblock.h>
#include <misaki/logger.h>
#include <misaki/manager.h>
#include <misaki/properties.h>

namespace misaki {

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
            InstanceManager::get()->create_instance<ReconstructionFilter>(
                Properties("gaussian"));
    }
}

Film::~Film() {}

void Film::put(const ImageBlock *block) { MSK_NOT_IMPLEMENTED("put"); }

void Film::set_destination_file(const fs::path &filename) {
    MSK_NOT_IMPLEMENTED("set_destination_file");
}

void Film::develop() { MSK_NOT_IMPLEMENTED("develop"); }

std::string Film::to_string() const {
    std::ostringstream oss;
    oss << "Film[" << std::endl
        << "  size = " << m_size << "," << std::endl
        << "  m_filter = " << m_filter->to_string() << std::endl
        << "]";
    return oss.str();
}

MSK_IMPLEMENT_CLASS(Film, Object, "film")

} // namespace misaki