#include <aspirin/emitter.h>
#include <aspirin/mesh.h>
#include <aspirin/properties.h>
#include <aspirin/records.h>
#include <aspirin/scene.h>
#include <aspirin/texture.h>
#include <aspirin/warp.h>

namespace aspirin {

template <typename Float, typename Spectrum>
class ConstantBackgroundEmitter final : public Emitter<Float, Spectrum> {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Base = Emitter<Float, Spectrum>;
    using Base::m_flags;
    using Base::m_shape;
    using Base::m_world_transform;
    using Texture = Texture<Float, Spectrum>;
    using typename Base::DirectSample;
    using typename Base::Interaction;
    using typename Base::Scene;
    using typename Base::Shape;
    using typename Base::SurfaceInteraction;

    ConstantBackgroundEmitter(const Properties &props) : Base(props) {
        m_bsphere  = BoundingSphere3(Vector3::Constant(0.f), 1.f);
        m_radiance = props.texture<Texture>("radiance", 1.f);

        m_flags = +EmitterFlags::Infinite;
    }

    void set_scene(const Scene *scene) override {
        m_bsphere = scene->bbox().bounding_sphere();
        m_bsphere.radius =
            std::max(math::RayEpsilon<Float>,
                     m_bsphere.radius * (1.f + math::RayEpsilon<Float>) );
    }

    std::pair<DirectSample, Spectrum>
    sample_direct(const Interaction &it, const Vector2 &sample) const override {
        Vector3 d  = warp::square_to_uniform_sphere(sample);
        Float dist = 2.f * m_bsphere.radius;

        DirectSample ds;
        ds.p      = it.p + d * dist;
        ds.n      = -d;
        ds.uv     = Vector2(0.f, 0.f);
        ds.pdf    = warp::square_to_uniform_sphere_pdf(d);
        ds.delta  = false;
        ds.object = this;
        ds.d      = d;
        ds.dist   = dist;

        SurfaceInteraction si;

        return { ds, m_radiance->eval_3(si) / ds.pdf };
    }

    Float pdf_direct(const Interaction &ref,
                     const DirectSample &ds) const override {
        return warp::square_to_uniform_sphere_pdf(ds.d);
    }

    Spectrum eval(const SurfaceInteraction &si) const override {
        return m_radiance->eval_3(si);
    }

    APR_DECLARE_CLASS()
private:
    ref<Texture> m_radiance;
    BoundingSphere3 m_bsphere;
};

APR_IMPLEMENT_CLASS_VARIANT(ConstantBackgroundEmitter, Emitter)
APR_INTERNAL_PLUGIN(ConstantBackgroundEmitter, "constant")

} // namespace aspirin