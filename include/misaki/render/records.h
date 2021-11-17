#pragma once

#include "misaki/core/fwd.h"
#include "interaction.h"

namespace misaki {

struct PositionSample {

    Eigen::Vector3f p;
    Eigen::Vector3f n;
    Eigen::Vector2f uv;
    float pdf; // Probability density at the sample
    bool delta;

    const Object *object = nullptr;

    PositionSample()
        : p(Eigen::Vector3f::Zero()), n(Eigen::Vector3f::Zero()),
          uv(Eigen::Vector2f::Zero()), pdf(0.f),
          delta(false) {}

    PositionSample(const SurfaceInteraction &si)
        : p(si.p), n(si.sh_frame.n), uv(si.uv), delta(false) {}
};

struct DirectionSample : public PositionSample {
    Eigen::Vector3f ref;
    Eigen::Vector3f ref_n;
    Eigen::Vector3f d;
    float dist;

    DirectionSample()
        : PositionSample(), d(Eigen::Vector3f::Zero()), dist(0.f) {}

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

} // namespace misaki