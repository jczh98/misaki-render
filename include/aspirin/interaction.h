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

    bool is_valid() const { return t != math::Infinity<Float>; }
};

template <typename Float_, typename Spectrum_>
struct SurfaceInteraction : Interaction<Float_, Spectrum_> {
    APR_IMPORT_CORE_TYPES(Float_)
    using Spectrum       = Spectrum_;
    using Emitter        = Emitter<Float, Spectrum>;
    using BSDF           = BSDF<Float, Spectrum>;
    using Shape          = Shape<Float, Spectrum>;
    using PositionSample = PositionSample<Float, Spectrum>;

    using ShapePtr   = const Shape *;
    using EmitterPtr = const Emitter *;
    using BSDFPtr    = const BSDF *;

    ShapePtr shape = nullptr;
    Vector2 uv;
    Vector3 n;
    Frame3 sh_frame;
    Vector3 wi;
    uint32_t prim_index;

    explicit SurfaceInteraction(const PositionSample &ps)
        : Interaction<Float, Spectrum>(0.f, ps.p), uv(ps.uv), n(ps.n),
          sh_frame(Frame3(ps.n)) {}

    Vector3 to_world(const Vector3 &v) const { return sh_frame.to_world(v); }

    Vector3 to_local(const Vector3 &v) const { return sh_frame.to_local(v); }
};

} // namespace aspirin