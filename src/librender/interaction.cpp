#include <misaki/render/interaction.h>
#include <misaki/render/records.h>
#include <misaki/render/shape.h>

namespace misaki {

SurfaceInteraction::SurfaceInteraction(const PositionSample &ps)
    : Interaction(0.f, ps.p), uv(ps.uv), n(ps.n), sh_frame(Frame(ps.n)) {}

SurfaceInteraction::MediumPtr
SurfaceInteraction::target_medium(const float &cos_theta) const {
    return cos_theta > 0 ? shape->exterior_medium() : shape->interior_medium();
}

SurfaceInteraction::BSDFPtr SurfaceInteraction::bsdf() const {
    return shape->bsdf();
}

bool SurfaceInteraction::is_medium_transition() const {
    return shape->is_medium_transition();
}

SceneInteraction::SceneInteraction(const PositionSample &ps)
    : p(ps.p), uv(ps.uv), n(ps.n), sh_frame(Frame(ps.n)) {}

const Medium*
SceneInteraction::target_medium(const float &cos_theta) const {
    return cos_theta > 0 ? shape->exterior_medium() : shape->interior_medium();
}

const BSDF *SceneInteraction::bsdf() const {
    return shape->bsdf();
}

bool SceneInteraction::is_medium_transition() const {
    return shape->is_medium_transition();
}

SceneInteraction
PreliminaryIntersection::compute_scene_interaction(const Ray &ray) {
    SceneInteraction si = shape->compute_scene_interaction(ray, *this);
    if (si.is_valid()) {
        si.prim_index = prim_index;
        si.shape      = shape;
        si.initialize_sh_frame();
        si.wi = si.to_local(-ray.d);
    } else {
        si.t  = math::Infinity<float>;
        si.wi = -ray.d;
    }
    return si;
}

} // namespace misaki