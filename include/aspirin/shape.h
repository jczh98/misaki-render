#pragma once

#include "fwd.h"
#include "object.h"
#include "records.h"
#if defined(APR_ENABLE_EMBREE)
#include <embree3/rtcore.h>
#endif

namespace aspirin {

class APR_EXPORT Shape : public Object {
public:
    virtual PositionSample sample_position(const Vector2 &sample) const;
    virtual Float pdf_position(const PositionSample &ps) const;

    virtual DirectionSample sample_direction(const Interaction &it,
                                             const Vector2 &sample) const;
    virtual Float pdf_direction(const Interaction &it,
                                const DirectionSample &ds) const;

    virtual SurfaceInteraction
    compute_surface_interaction(const Ray &ray,
                                PreliminaryIntersection pi) const;

    bool is_mesh() const { return m_is_mesh; }

    const BSDF *bsdf() const { return m_bsdf; }
    BSDF *bsdf() { return m_bsdf; }

    bool is_emitter() const { return (bool) m_emitter; }
    const Emitter *emitter() const { return m_emitter; }
    Emitter *emitter() { return m_emitter.get(); }

    /// Does the surface of this shape mark a medium transition?
    bool is_medium_transition() const {
        return m_interior_medium.get() != nullptr ||
               m_exterior_medium.get() != nullptr;
    }

    /// Return the medium that lies on the interior of this shape
    const Medium *interior_medium() const { return m_interior_medium.get(); }

    /// Return the medium that lies on the exterior of this shape
    const Medium *exterior_medium() const { return m_exterior_medium.get(); }

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
    ref<Medium> m_interior_medium;
    ref<Medium> m_exterior_medium;
    Transform4 m_world_transform;
    std::string m_id;

    bool m_is_mesh = false;
};

} // namespace aspirin