#include <aspirin/properties.h>
#include <aspirin/volume.h>

namespace aspirin {

template <typename Float, typename Spectrum>
Volume<Float, Spectrum>::Volume(const Properties &props) {
    m_world_to_local = props.transform("to_world", Transform4()).inverse();
}

template <typename Float, typename Spectrum>
Spectrum Volume<Float, Spectrum>::eval(const Interaction &) const {
    APR_NOT_IMPLEMENTED("eval");
}

APR_IMPLEMENT_CLASS_VARIANT(Volume, Object, "volume")
APR_INSTANTIATE_CLASS(Volume)

} // namespace aspirin