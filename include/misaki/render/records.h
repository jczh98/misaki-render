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

struct DirectionSample {
    Eigen::Vector3f d;
    float pdf;
};

struct DirectIllumSample : public PositionSample {
    Eigen::Vector3f ref;
    Eigen::Vector3f ref_n;
    Eigen::Vector3f d;
    float dist;

    DirectIllumSample() {}

    DirectIllumSample(const PositionSample &base) : PositionSample(base) {}

    void set_query(const Ray &ray, const SceneInteraction &si);
};

} // namespace misaki