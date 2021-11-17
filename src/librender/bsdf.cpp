#include <misaki/render/bsdf.h>
#include <misaki/core/logger.h>
#include <misaki/core/properties.h>
#include <misaki/render/shape.h>

namespace misaki {

BSDF::BSDF(const Properties &props)
    : m_flags(+BSDFFlags::None), m_id(props.id()) {}

BSDF::~BSDF() {}

Spectrum BSDF::eval_null_transmission(const SurfaceInteraction &) const {
    return Spectrum::Zero();
}

std::string BSDF::id() const { return m_id; }

const BSDF *SurfaceInteraction::bsdf(const RayDifferential &ray) {
    const BSDFPtr bsdf = shape->bsdf();

    if (!has_uv_partials() && bsdf->needs_differentials()) {
        compute_uv_partials(ray);
    }
    return bsdf;
}

MSK_IMPLEMENT_CLASS(BSDF, Object, "bsdf")

} // namespace misaki