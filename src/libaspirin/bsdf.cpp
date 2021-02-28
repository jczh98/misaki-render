#include <aspirin/bsdf.h>
#include <aspirin/logger.h>
#include <aspirin/properties.h>

namespace aspirin {

template <typename Spectrum>
BSDF<Spectrum>::BSDF(const Properties &props)
    : m_flags(+BSDFFlags::None), m_id(props.id()) {}

template <typename Spectrum>
std::pair<BSDFSample, Color3>
BSDF<Spectrum>::sample(const BSDFContext &ctx, const SceneInteraction &si,
                       Float sample1, const Vector2 &sample) const {
    ARP_NOT_IMPLEMENTED("sample");
}

template <typename Spectrum>
Color3 BSDF<Spectrum>::eval(const BSDFContext &ctx, const SceneInteraction &si,
                            const Vector3 &wo) const {
    ARP_NOT_IMPLEMENTED("eval");
}

template <typename Spectrum>
Float BSDF<Spectrum>::pdf(const BSDFContext &ctx, const SceneInteraction &si,
                          const Vector3 &wo) const {
    ARP_NOT_IMPLEMENTED("pdf");
}


} // namespace aspirin