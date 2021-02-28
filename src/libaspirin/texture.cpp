#include <aspirin/logger.h>
#include <aspirin/properties.h>
#include <aspirin/texture.h>

namespace aspirin {

template <typename Spectrum>
Texture<Spectrum>::Texture(const Properties &props) : m_id(props.id()) {}

template <typename Spectrum>
Float Texture<Spectrum>::eval_1(const PointGeometry &geom) const {
    ARP_NOT_IMPLEMENTED("eval_1");
}

template <typename Spectrum>
Color3 Texture<Spectrum>::eval_3(const PointGeometry &geom) const {
    ARP_NOT_IMPLEMENTED("eval_3");
}

template <typename Spectrum> Float Texture<Spectrum>::mean() const {
    ARP_NOT_IMPLEMENTED("mean");
}

} // namespace aspirin