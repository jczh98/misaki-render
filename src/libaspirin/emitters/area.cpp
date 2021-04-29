#include <aspirin/emitter.h>
#include <aspirin/mesh.h>
#include <aspirin/properties.h>
#include <aspirin/records.h>
#include <aspirin/texture.h>
#include <aspirin/warp.h>

namespace aspirin {

class AreaLight final : public Emitter {
public:
    AreaLight(const Properties &props) : Emitter(props) {
        m_radiance = props.texture<Texture>("radiance", 1.f);

        m_flags = +EmitterFlags::Surface;
    }

    virtual std::pair<Ray, Spectrum>
    sample_ray(const Vector2 &sample, const Vector2 &sample2) const override {
        PositionSample ps = m_shape->sample_position(sample);
        SurfaceInteraction si(ps);
        Vector3 local = warp::square_to_cosine_hemisphere(sample2);

        Spectrum power = m_radiance->eval_3(si) * math::Pi<Float> / ps.pdf;

        return { Ray(ps.p, si.to_world(local), 0), power };
    }

    std::pair<DirectionSample, Spectrum>
    sample_direction(const Interaction &it,
                     const Vector2 &sample) const override {
        auto ds = m_shape->sample_direction(it, sample);
        SurfaceInteraction si(ds);
        ds.object = this;
        if (ds.d.dot(ds.n) < 0.f && ds.pdf != 0.f) {
            return { ds, m_radiance->eval_3(si) / ds.pdf };
        } else {
            ds.pdf = 0;
            return { ds, Spectrum::Zero() };
        }
    }

    Float pdf_direction(const Interaction &ref,
                        const DirectionSample &ds) const override {
        return m_shape->pdf_direction(ref, ds);
    }

    Spectrum eval(const SurfaceInteraction &si) const override {
        return Frame3::cos_theta(si.wi) > 0.f ? m_radiance->eval_3(si)
                                              : Spectrum::Zero();
    }

    APR_DECLARE_CLASS()
private:
    ref<Texture> m_radiance;
};

APR_IMPLEMENT_CLASS(AreaLight, Emitter)
APR_INTERNAL_PLUGIN(AreaLight, "area")

} // namespace aspirin