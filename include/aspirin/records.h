#pragma once

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

    PositionSample() {}
    
    PositionSample(const SurfaceInteraction &si)
        : p(si.p), n(si.sh_frame.n), uv(si.uv), delta(false) {}
};

template <typename Float_, typename Spectrum_>
struct DirectionSample : public PositionSample<Float_, Spectrum_> {
    using Base = PositionSample<Float_, Spectrum_>;
    using typename Base::Float;
    using typename Base::Spectrum;
    using typename Base::Vector2;
    using typename Base::Vector3;
    using Interaction        = Interaction<Float, Spectrum>;
    using SurfaceInteraction = SurfaceInteraction<Float, Spectrum>;

    Vector3 d;
    Float dist;

    DirectionSample() : PositionSample<Float, Spectrum>() {}

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