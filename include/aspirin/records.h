#pragma once

#include "fwd.h"
#include "interaction.h"

namespace aspirin {

template <typename Float_, typename Spectrum_> struct PositionSample {
    using Float    = Float_;
    using Spectrum = Spectrum_;
    APR_IMPORT_CORE_TYPES(Float_)
    using SurfaceInteraction = SurfaceInteraction<Float, Spectrum>;

    Vector3 p;
    Vector3 n;
    Vector2 uv;
    Float pdf; // Probability density at the sample
    bool delta;

    const Object *object = nullptr;

    PositionSample()
        : p(Vector3::Zero()), n(Vector3::Zero()), uv(Vector2::Zero()), pdf(0.f),
          delta(false) {}

    PositionSample(const SurfaceInteraction &si)
        : p(si.p), n(si.sh_frame.n), uv(si.uv), delta(false) {}
};

template <typename Float_, typename Spectrum_>
struct DirectSample : public PositionSample<Float_, Spectrum_> {
    using Base = PositionSample<Float_, Spectrum_>;
    using Base::delta;
    using Base::n;
    using Base::object;
    using Base::p;
    using Base::pdf;
    using typename Base::Float;
    using typename Base::Spectrum;
    using typename Base::SurfaceInteraction;
    using typename Base::Vector2;
    using typename Base::Vector3;
    using Interaction = Interaction<Float, Spectrum>;

    Vector3 ref;
    Vector3 ref_n;
    Vector3 d;
    Float dist;

    DirectSample()
        : PositionSample<Float, Spectrum>(), d(Vector3::Zero()), dist(0.f) {}

    DirectSample(const SurfaceInteraction &it, const Interaction &ref)
        : PositionSample<Float, Spectrum>(it) {
        d    = it.p - ref.p;
        dist = d.norm();
        d /= dist;
        if (!it.is_valid())
            d = -it.wi;
    }

    DirectSample(const PositionSample<Float, Spectrum> &base)
        : PositionSample<Float, Spectrum>(base) {}
};

} // namespace aspirin