#pragma once

#include "fwd.h"
#include "ray.h"

namespace aspirin {

// If a quantity requires incident direction wi, then use SceneInteraction as
// parameter. Otherwise uses PointGeometry
template <typename Spectrum> struct PointGeometry {
    bool degenerated;                     // True if surface is a point
    bool infinite;                        // True if point is at infinity
    Vector3 p{ 0.f };                     // Position
    Vector2 uv{ 0.f };                    // Texture coordinates
    Vector3 n{ 0.f };                     // Geometric normal
    Frame3 shading = Frame3({ 0, 0, 0 }); // Shading frame with shading normal

    static PointGeometry make_degenerated(const Vector3 &p) {
        PointGeometry geom;
        geom.degenerated = true;
        geom.infinite    = false;
        geom.p           = p;
        return geom;
    }

    static PointGeometry make_infinite(const Vector3 &p, const Vector3 &n,
                                       const Vector2 &uv) {
        PointGeometry geom;
        geom.p           = p;
        geom.n           = n;
        geom.uv          = uv;
        geom.shading     = Frame(n);
        geom.degenerated = false;
        geom.infinite    = true;
        return geom;
    }

    static PointGeometry make_on_surface(const Vector3 &p, const Vector3 &ng,
                                         const Vector3 &ns, const Vector2 &uv) {
        PointGeometry geom;
        geom.degenerated = false;
        geom.infinite    = false;
        geom.p           = p;
        geom.n           = ng;
        geom.shading     = Frame(ns);
        geom.uv          = uv;
        return geom;
    }
};

template <typename Spectrum> struct APR_EXPORT SceneInteraction {
    using PointGeometry = PointGeometry<Spectrum>;
    using Shape         = Shape<Spectrum>;
    using Ray           = Ray<Spectrum>;
    using Light         = Light<Spectrum>;
    using BSDF          = BSDF<Spectrum>;
    using Scene         = Scene<Spectrum>;

    enum Type : uint32_t {
        None = 0,
        CameraEndpoint,
        LightEndpoint,
        SurfaceInteraction,
        MediumInteraction,
        Endpoint,
        Intermediate
    };
    int type = None;
    PointGeometry geom;           // Interaction point information
    Vector3 wi;                   // Incident direction in local shading frame
    const Shape *shape = nullptr; // Scene interact with surface and records its
                                  // shape information

    bool is_type(uint32_t type_flag) const { return type == type_flag; }

    bool is_valid() const { return !is_type(Type::None); }

    static SceneInteraction make_surface_interaction(const PointGeometry &geom,
                                                     const Vector3 &wi_world,
                                                     const Shape *shape) {
        SceneInteraction si;
        si.type  = SurfaceInteraction;
        si.geom  = geom;
        si.wi    = si.to_local(wi_world);
        si.shape = shape;
        return si;
    }

    static SceneInteraction make_none(const Vector3 &wi) {
        SceneInteraction si;
        si.wi = wi;
        return si;
    }

    Vector3 to_world(const Vector3 &v) const {
        return geom.shading.to_world(v);
    }
    Vector3 to_local(const Vector3 &v) const {
        return geom.shading.to_local(v);
    }

    Ray spawn_ray(const Vector3f &d) const {
        return Ray(geom.p, d,
                   (1.f + geom.p.cwiseAbs().maxCoeff()) *
                       math::RayEpsilon<Float>,
                   math::Infinity<Float>, 0.f);
    }

    const Light *light(const std::shared_ptr<Scene> &scene) const;

    const BSDF *bsdf(const Ray &ray) const;
    const BSDF *bsdf() const;
};

} // namespace aspirin