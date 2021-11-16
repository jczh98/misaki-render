#include <misaki/interaction.h>
#include <misaki/records.h>
#include <misaki/shape.h>

namespace misaki {

SurfaceInteraction::SurfaceInteraction(const PositionSample &ps)
    : Interaction(0.f, ps.p), uv(ps.uv), n(ps.n), sh_frame(Frame3(ps.n)) {}

SurfaceInteraction::MediumPtr
SurfaceInteraction::target_medium(const Float &cos_theta) const {
    return cos_theta > 0 ? shape->exterior_medium() : shape->interior_medium();
}

SurfaceInteraction::BSDFPtr SurfaceInteraction::bsdf() const {
    return shape->bsdf();
}

bool SurfaceInteraction::is_medium_transition() const {
    return shape->is_medium_transition();
}

SurfaceInteraction
PreliminaryIntersection::compute_surface_interaction(const Ray &ray) {
    SurfaceInteraction si = shape->compute_surface_interaction(ray, *this);
    if (si.is_valid()) {
        si.prim_index = prim_index;
        si.shape      = shape;
        si.initialize_sh_frame();
        si.wi = si.to_local(-ray.d);
    } else {
        si.t  = math::Infinity<Float>;
        si.wi = -ray.d;
    }
    return si;
}

} // namespace misaki