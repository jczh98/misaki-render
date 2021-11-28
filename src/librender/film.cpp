#include <misaki/core/logger.h>
#include <misaki/core/manager.h>
#include <misaki/core/properties.h>
#include <misaki/render/film.h>
#include <misaki/render/imageblock.h>

namespace misaki {

Film::Film(const Properties &props) {
    m_size = { props.int_("width", 640), props.int_("height", 320) };

    // Crop window specified in pixels - by default, this matches the full
    // sensor area.
    Eigen::Vector2i crop_offset = Eigen::Vector2i(
        props.int_("crop_offset_x", 0), props.int_("crop_offset_y", 0));

    Eigen::Vector2i crop_size =
        Eigen::Vector2i(props.int_("crop_width", m_size.x()),
                        props.int_("crop_height", m_size.y()));

    set_crop_window(crop_offset, crop_size);

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

void Film::set_crop_window(const Eigen::Vector2i &crop_offset,
                           const Eigen::Vector2i &crop_size) {
    // TODO: need to optimize code
    if ((crop_offset.x() < 0 || crop_offset.y() < 0 || crop_size.x() <= 0 ||
         crop_size.y() <= 0 || (crop_offset + crop_size).x() > m_size.x() ||
         (crop_offset + crop_size).y() > m_size.y()))
        Throw("Invalid crop window specification!\n"
              "offset {} + crop size {} vs full size {}",
              crop_offset.x(), crop_size.x(), m_size.x());

    m_crop_size   = crop_size;
    m_crop_offset = crop_offset;
}

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