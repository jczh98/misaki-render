#pragma once

#include "fwd.h"
#include "interaction.h"

namespace aspirin {

struct PositionSample {

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

struct DirectionSample : public PositionSample {
    Vector3 ref;
    Vector3 ref_n;
    Vector3 d;
    Float dist;

    DirectionSample() : PositionSample(), d(Vector3::Zero()), dist(0.f) {}

    DirectionSample(const SurfaceInteraction &it, const Interaction &ref)
        : PositionSample(it) {
        d    = it.p - ref.p;
        dist = d.norm();
        d /= dist;
        if (!it.is_valid())
            d = -it.wi;
    }

    DirectionSample(const PositionSample &base) : PositionSample(base) {}
};

} // namespace aspirin