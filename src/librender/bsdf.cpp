#include <misaki/render/bsdf.h>
#include <misaki/render/logger.h>
#include <misaki/render/properties.h>

namespace misaki::render {

BSDF::BSDF(const Properties &props) : m_flags(+BSDFFlags::None), m_id(props.id()) {
}

std::pair<BSDFSample, Color3> BSDF::sample(const BSDFContext &ctx,
                                           const PointGeometry &geom,
                                           const Vector3 &wi,
                                           const Vector2 &sample) const {
  MSK_NOT_IMPLEMENTED("sample");
}

Color3 BSDF::eval(const BSDFContext &ctx,
                  const PointGeometry &geom,
                  const Vector3 &wi,
                  const Vector3 &wo) const {
  MSK_NOT_IMPLEMENTED("eval");
}

Float BSDF::pdf(const BSDFContext &ctx,
                const PointGeometry &geom,
                const Vector3 &wi,
                const Vector3 &wo) const {
  MSK_NOT_IMPLEMENTED("pdf");
}

MSK_REGISTER_CLASS(BSDF)

}  // namespace misaki::render