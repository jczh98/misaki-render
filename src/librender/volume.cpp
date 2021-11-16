#include <misaki/properties.h>
#include <misaki/volume.h>

namespace misaki {

Volume::Volume(const Properties &props) {
    m_world_to_local = props.transform("to_world", Transform4()).inverse();
}

Spectrum Volume::eval(const Interaction &) const {
    APR_NOT_IMPLEMENTED("eval");
}

APR_IMPLEMENT_CLASS(Volume, Object, "volume")

} // namespace misaki