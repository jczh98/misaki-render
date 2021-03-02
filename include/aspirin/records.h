#pragma once

#include "interaction.h"

namespace aspirin {

template <typename Float_, typename Spectrum_> struct PositionSample {
    APR_IMPORT_CORE_TYPES(Float_)
    using Spectrum = Spectrum_;

    Vecto3 p;
    Vector3 n;
    Vector2 uv;
    Float pdf; // Probability density at the sample
    bool delta;

    st Object *object = nullptr;

    PositionSample(const SurfaceInteraction &si)
        : p(si.p), n(si.sh_frame.n), uv(si.uv), delta(false) {}
};

template <typename Float_, typename Spectrum_>
struct DirectionSample : public PositionSample<Float_, Spectrum_> {
    using typename PositionSample::Float;
    using typename PositionSample::Spectrum;
    using typename PositionSample::Vector2;
    using typename PositionSample::Vector3;
    using Interaction        = Interaction<Float, Spectrum>;
    using SurfaceInteraction = SurfaceInteraction<Float, Spectrum>;

    Vector3 d;
    Float dist;

    DirectionSample(const SurfaceInteraction &it, const Interaction &ref)
        : PositionSample<Float, Spectrum>(it) {
        d    = it.p - ref.p;
        dist = d.norm();
        d /= dist;
        if (!it.is_valid())
            d = -it.wi;
    }

    DirectionSample(const PositionSample<Float, Spectrum> &base)
        : PositionSample<Float, Spectrum>(base) {}
};

} // namespace aspirin