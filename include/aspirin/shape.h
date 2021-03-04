#pragma once

#include "fwd.h"
#include "object.h"
#if defined(APR_ENABLE_EMBREE)
#include <embree3/rtcore.h>
#endif

namespace aspirin {

template <typename Float, typename Spectrum>
class APR_EXPORT Shape : public Object {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Sensor             = Sensor<Float, Spectrum>;
    using BSDF               = BSDF<Float, Spectrum>;
    using Emitter            = Emitter<Float, Spectrum>;
    using Interaction        = Interaction<Float, Spectrum>;
    using PositionSample     = PositionSample<Float, Spectrum>;
    using DirectionSample    = DirectionSample<Float, Spectrum>;
    using SurfaceInteraction = SurfaceInteraction<Float, Spectrum>;

    virtual PositionSample sample_position(const Vector2 &sample) const;
    virtual Float pdf_position(const PositionSample &ps) const;

    virtual DirectionSample sample_direction(const Interaction &it,
                                             const Vector2 &sample) const;
    virtual Float pdf_direction(const Interaction &it,
                                const DirectionSample &ds) const;

    // Returns position, geometry normal, shading normal, texcoords
    virtual std::tuple<Vector3, Vector3, Vector3, Vector2>
    compute_surface_point(int prim_index, const Vector2 &uv) const;

    bool is_mesh() const {
        return m_is_mesh;
    }

    const BSDF *bsdf() const { return m_bsdf; }
    BSDF *bsdf() { return m_bsdf; }

    bool is_emitter() const { return (bool) m_emitter; }
    const Emitter *emitter() const { return m_emitter; }
    Emitter *emitter() { return m_emitter.get(); }

    virtual BoundingBox3 bbox() const;
    virtual BoundingBox3 bbox(uint32_t index) const;
    virtual Float surface_area() const;

#if defined(APR_ENABLE_EMBREE)
    virtual RTCGeometry embree_geometry(RTCDevice device) const;
#endif

    APR_DECLARE_CLASS()
protected:
    Shape(const Properties &props);
    void set_children();

protected:
    ref<Emitter> m_emitter;
    ref<BSDF> m_bsdf;
    Transform4 m_world_transform;
    std::string m_id;

    bool m_is_mesh = false;
};

} // namespace aspirin