#include <aspirin/emitter.h>
#include <aspirin/mesh.h>
#include <aspirin/properties.h>
#include <aspirin/records.h>
#include <aspirin/texture.h>

namespace aspirin {

template <typename Float, typename Spectrum>
class AreaLight final : public Emitter<Float, Spectrum> {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Base = Emitter<Float, Spectrum>;
    using Base::m_flags;
    using Base::m_shape;
    using Base::m_world_transform;
    using Texture = Texture<Float, Spectrum>;
    using typename Base::DirectSample;
    using typename Base::Interaction;
    using typename Base::Shape;
    using typename Base::SurfaceInteraction;

    AreaLight(const Properties &props) : Base(props) {
        m_radiance = props.texture<Texture>("radiance", 1.f);

        m_flags = +EmitterFlags::Surface;
    }

    std::pair<DirectSample, Spectrum>
    sample_direct(const Interaction &it, const Vector2 &sample) const override {
        auto ds = m_shape->sample_direct(it, sample);
        SurfaceInteraction si(ds);
        ds.object = this;
        if (ds.d.dot(ds.n) < 0.f && ds.pdf != 0.f) {
            return { ds, m_radiance->eval_3(si) / ds.pdf };
        } else {
            ds.pdf = 0;
            return { ds, Spectrum::Zero() };
        }
    }

    Float pdf_direct(const Interaction &ref, const DirectSample &ds) const override {
        return m_shape->pdf_direct(ref, ds);
    }

    Spectrum eval(const SurfaceInteraction &si) const override {
        return Frame3::cos_theta(si.wi) > 0.f ? m_radiance->eval_3(si)
                                              : Spectrum::Zero();
    }

    APR_DECLARE_CLASS()
private:
    ref<Texture> m_radiance;
};

APR_IMPLEMENT_CLASS_VARIANT(AreaLight, Emitter)
APR_INTERNAL_PLUGIN(AreaLight, "area")

} // namespace aspirin