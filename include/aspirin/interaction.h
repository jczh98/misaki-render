#pragma once

#include "fwd.h"
#include "ray.h"

namespace aspirin {

template <typename Float_, typename Spectrum_> struct Interaction {
    using Float    = Float_;
    using Spectrum = Spectrum_;
    using Vector3  = Eigen::Matrix<Float, 3, 1>;
    using Ray      = Ray<Float, Spectrum>;

    Float t = math::Infinity<Float>;
    Vector3 p;

    Interaction() {}

    Interaction(Float t, const Vector3 &p) : t(t), p(p) {}

    bool is_valid() const { return t != math::Infinity<Float>; }

    Ray spawn_ray(const Vector3 &d) const {
        return Ray(p, d,
                   (1.f + p.cwiseAbs().maxCoeff()) * math::RayEpsilon<Float>,
                   math::Infinity<Float>, 0.f);
    }
};

template <typename Float_, typename Spectrum_>
struct SurfaceInteraction : public Interaction<Float_, Spectrum_> {
    using Float    = Float_;
    using Spectrum = Spectrum_;
    APR_IMPORT_CORE_TYPES(Float_)
    using Base = Interaction<Float, Spectrum>;
    using Base::is_valid;
    using Base::p;
    using typename Base::Ray;
    using Emitter         = Emitter<Float, Spectrum>;
    using BSDF            = BSDF<Float, Spectrum>;
    using Medium          = Medium<Float, Spectrum>;
    using Shape           = Shape<Float, Spectrum>;
    using PositionSample  = PositionSample<Float, Spectrum>;
    using Scene           = Scene<Float, Spectrum>;
    using RayDifferential = RayDifferential<Float, Spectrum>;

    using ShapePtr   = const Shape *;
    using EmitterPtr = const Emitter *;
    using BSDFPtr    = const BSDF *;
    using MediumPtr  = const Medium *;

    ShapePtr shape = nullptr;
    Vector2 uv;
    Vector3 n;

    /// Local shading frame
    Frame3 sh_frame;

    /// Incident direction in local shading frame
    Vector3 wi;

    uint32_t prim_index;

    /// Position partials wrt. the UV parameterization
    Vector3 dp_du, dp_dv;

    /// Normal partials wrt. the UV parameterization
    Vector3 dn_du, dn_dv;

    /// UV partials wrt. screen displacement
    Vector2 duv_dx, duv_dy;

    SurfaceInteraction()
        : Interaction<Float, Spectrum>(), sh_frame(Frame3(Vector3::Zero())) {}

    explicit SurfaceInteraction(const PositionSample &ps)
        : Interaction<Float, Spectrum>(0.f, ps.p), uv(ps.uv), n(ps.n),
          sh_frame(Frame3(ps.n)) {}

    Vector3 to_world(const Vector3 &v) const { return sh_frame.to_world(v); }

    Vector3 to_local(const Vector3 &v) const { return sh_frame.to_local(v); }

    /// Initialize local shading frame using Gram-schmidt orthogonalization
    void initialize_sh_frame() {
        Vector3 face_forward = -sh_frame.n * sh_frame.n.dot(dp_du) + dp_du;
        sh_frame.s           = face_forward.normalized();
        sh_frame.t           = sh_frame.n.cross(sh_frame.s);
    }

    void compute_uv_partials(const RayDifferential &ray) {
        if (!ray.has_differentials)
            return;
        Float d = n.dot(p), t_x = (d - n.dot(ray.o_x)) / n.dot(ray.d_x),
              t_y = (d - n.dot(ray.o_y)) / n.dot(ray.d_y);

        Vector3 dp_dx = ray.d_x * t_x + ray.o_x - p,
                dp_dy = ray.d_y * t_y + ray.o_y - p;

        Float a00 = dp_du.dot(dp_du), a01 = dp_du.dot(dp_dv),
              a11     = dp_dv.dot(dp_dv),
              inv_det = Float(1) / (a00 * a11 - a01 * a01);

        Float b0x = dp_du.dot(dp_dx), b1x = dp_dv.dot(dp_dx),
              b0y = dp_du.dot(dp_dy), b1y = dp_dv.dot(dp_dy);

        inv_det = std::isfinite(inv_det) ? inv_det : 0.f;

        duv_dx =
            Vector2(a11 * b0x - a01 * b1x, a00 * b1x - a01 * b0x) * inv_det;
        duv_dy =
            Vector2(a11 * b0y - a01 * b1y, a00 * b1y - a01 * b0y) * inv_det;
    }

    /// Check whether uv partials exists
    bool has_uv_partials() const {
        return (duv_dx.array() != 0.f || duv_dy.array() != 0.f).any();
    }

    /// Check whether normal partials exists
    bool has_n_partials() const {
        return (dn_du.array() != 0.f || dn_dv.array() != 0.f).any();
    }

    EmitterPtr emitter(const Scene *scene) const;

    BSDFPtr bsdf(const RayDifferential &ray);
    BSDFPtr bsdf() const { return shape->bsdf(); }

    MediumPtr target_medium(const Vector3 &d) const {
        return target_medium(d.dot(n));
    }

    MediumPtr target_medium(const Float &cos_theta) const {
        return cos_theta > 0 ? shape->exterior_medium()
                             : shape->interior_medium();
    }
};

template <typename Float_, typename Spectrum_>
struct MediumInteraction : Interaction<Float_, Spectrum_> {
    using Float    = Float_;
    using Spectrum = Spectrum_;
    APR_IMPORT_CORE_TYPES(Float_)
    using Base = Interaction<Float, Spectrum>;
    using Base::is_valid;
    using Base::p;
    using Base::t;

    using MediumPtr = const Medium<Float, Spectrum> *;

    MediumPtr medium = nullptr;
    Frame3 sh_frame;
    Vector3 wi;
    /// Scattering coefficient
    Spectrum sigma_s;
    /// Null-collision coefficient for delta tracking
    Spectrum sigma_n;
    /// Extinction coefficient
    Spectrum sigma_t;
    Spectrum combined_extinction;
    Float mint;

    MediumInteraction()
        : Interaction<Float, Spectrum>(), sh_frame(Frame3(Vector3::Zero())),
          wi(Vector3::Zero()), mint(0) {}

    Vector3 to_world(const Vector3 &v) const { return sh_frame.to_world(v); }

    Vector3 to_local(const Vector3 &v) const { return sh_frame.to_local(v); }
};

template <typename Float_, typename Spectrum_> struct PreliminaryIntersection {
    using Float    = Float_;
    using Spectrum = Spectrum_;
    APR_IMPORT_CORE_TYPES(Float_)
    using SurfaceInteraction = SurfaceInteraction<Float, Spectrum>;
    using Ray                = Ray<Float, Spectrum>;
    using ShapePtr           = const Shape<Float, Spectrum> *;

    Float t = math::Infinity<Float>;

    Vector2 prim_uv;

    uint32_t prim_index;

    uint32_t shape_index;

    ShapePtr shape = nullptr;

    bool is_valid() const { return t != math::Infinity<Float>; }

    SurfaceInteraction compute_surface_interaction(const Ray &ray) {
        SurfaceInteraction si = shape->compute_surface_interaction(ray, *this);
        if (si.is_valid()) {
            si.prim_index = prim_index;
            si.shape      = shape;
            si.wi         = si.to_local(-ray.d);
            si.initialize_sh_frame();
        } else {
            si.t  = math::Infinity<Float>;
            si.wi = -ray.d;
        }
        return si;
    }
};

} // namespace aspirin