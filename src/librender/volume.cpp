#include <misaki/properties.h>
#include <misaki/volume.h>

namespace misaki {

Volume::Volume(const Properties &props) {
    m_world_to_local = props.transform("to_world", Transform4f()).inverse();
}

Spectrum Volume::eval(const Interaction &) const {
    MSK_NOT_IMPLEMENTED("eval");
}

MSK_IMPLEMENT_CLASS(Volume, Object, "volume")

} // namespace misaki