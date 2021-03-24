#pragma once

#include <optional>

#include "fwd.h"
#include "object.h"
#include "records.h"

namespace aspirin {

template <typename Float, typename Spectrum>
class APR_EXPORT Endpoint : public Object {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Ray                = Ray<Float, Spectrum>;
    using Shape              = Shape<Float, Spectrum>;
    using Scene              = Scene<Float, Spectrum>;
    using Medium             = Medium<Float, Spectrum>;
    using SurfaceInteraction = SurfaceInteraction<Float, Spectrum>;
    using Interaction        = Interaction<Float, Spectrum>;
    using PositionSample     = PositionSample<Float, Spectrum>;
    using DirectSample       = DirectSample<Float, Spectrum>;

    // Returns Sampled ray with structred RaySample
    virtual std::pair<Ray, Spectrum>
    sample_ray(const Vector2 &pos_sample) const;

    // Returns direction, weight
    virtual std::pair<PositionSample, Spectrum> sample_position(const Vector2 &sample) const;
    virtual std::pair<DirectSample, Spectrum>
    sample_direct(const Interaction &ref, const Vector2 &sample) const;

    virtual Float pdf_position(const PositionSample &ps) const;
    virtual Float pdf_direct(const Interaction &ref,
                                const DirectSample &ds) const;

    virtual Spectrum eval(const SurfaceInteraction &si) const;

    virtual void set_shape(Shape *shape);
    virtual void set_scene(const Scene *scene);
    virtual void set_medium(Medium *medium);

    Shape *shape() { return m_shape; }
    const Shape *shape() const { return m_shape; }

    Medium *medium() { return m_medium; }
    const Medium *medium() const { return m_medium; }

    APR_DECLARE_CLASS()
protected:
    Endpoint(const Properties &props);

    virtual ~Endpoint();

protected:
    ref<Medium> m_medium;
    Transform4 m_world_transform;
    Shape *m_shape = nullptr;
    std::string m_id;
};

APR_EXTERN_CLASS(Endpoint)
} // namespace aspirin