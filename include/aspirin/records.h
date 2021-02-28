#pragma once

#include "interaction.h"

namespace aspirin {

template <typename Spectrum> struct PositionSample {
    using PointGeometry = PointGeometry<Spectrum>;

    PointGeometry geom;
};

template <typename Spectrum> struct DirectSample {
    using PointGeometry    = PointGeometry<Spectrum>;
    using SceneInteraction = SceneInteraction<Spectrum>;

    PointGeometry geom;
    Float pdf{ 0.f };
    Vector3 d{ 0.f };
    Float dist{ 0.f };
    static DirectSample make_with_interactions(const SceneInteraction &sampled,
                                               const SceneInteraction &ref) {
        DirectSample ds;
        ds.geom = sampled.geom;
        ds.d    = sampled.geom.p - ref.geom.p;
        ds.dist = ds.d.norm();
        ds.d /= ds.dist;
        if (!sampled.is_valid())
            ds.d = -sampled.wi;
        return ds;
    }
};

} // namespace aspirin