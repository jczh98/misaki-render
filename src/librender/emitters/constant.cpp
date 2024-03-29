#include <misaki/core/manager.h>
#include <misaki/core/properties.h>
#include <misaki/core/warp.h>
#include <misaki/render/emitter.h>
#include <misaki/render/mesh.h>
#include <misaki/render/records.h>
#include <misaki/render/scene.h>
#include <misaki/render/texture.h>

namespace misaki {

class ConstantBackgroundEmitter final : public Emitter {
public:
    ConstantBackgroundEmitter(const Properties &props) : Emitter(props) {
        m_bsphere  = BoundingSphere3f(Eigen::Vector3f::Constant(0.f), 1.f);
        m_radiance = props.texture("radiance", Texture::D65(1.f));

        m_flags = +EmitterFlags::Infinite;
    }

    void set_scene(const Scene *scene) override {
        m_bsphere = scene->bbox().bounding_sphere();
        m_bsphere.radius =
            std::max(math::RayEpsilon<float>,
                     m_bsphere.radius * (1.f + math::RayEpsilon<float>) );
        m_surface_area =
            4.f * math::Pi<float> * m_bsphere.radius * m_bsphere.radius;
    }

    virtual std::pair<Ray, Spectrum>
    sample_ray(const Eigen::Vector2f &sample2,
               const Eigen::Vector2f &sample3) const override {
        // const Eigen::Vector3f v0 = warp::square_to_uniform_sphere(sample2);
        // const Eigen::Vector3f origin = m_bsphere.center + v0 *
        // m_bsphere.radius;

        // const Eigen::Vector3f v1 =
        // warp::square_to_cosine_hemisphere(sample3); const Eigen::Vector3f
        // direction = Frame(-v0).to_world(v1);

        // SurfaceInteraction si;
        // si.t  = 0.f;
        // si.p  = origin;
        // si.uv = sample2;
        // si.wi = direction; // Points toward the scene

        // Spectrum power =
        //    m_radiance->eval_3(si) * m_surface_area * math::Pi<float>;

        // return { Ray(origin, direction, 0), power };
        MSK_NOT_IMPLEMENTED("sample_ray");
    }

    std::pair<DirectIllumSample, Spectrum>
    sample_direct(const SceneInteraction &ref,
                  const Eigen::Vector2f &sample) const override {
        Eigen::Vector3f d = warp::square_to_uniform_sphere(sample);
        float dist        = 2.f * m_bsphere.radius;

        DirectIllumSample ds;
        ds.p      = ref.p + d * dist;
        ds.n      = -d;
        ds.uv     = Eigen::Vector2f(0.f, 0.f);
        ds.pdf    = warp::square_to_uniform_sphere_pdf(d);
        ds.delta  = false;
        ds.object = this;
        ds.d      = d;
        ds.dist   = dist;

        SceneInteraction si;

        return { ds, m_radiance->eval(si) / ds.pdf };
    }

    float pdf_direct(const DirectIllumSample &ds) const override {
        return warp::square_to_uniform_sphere_pdf(ds.d);
    }

    Spectrum eval(const SceneInteraction &si) const override {
        return m_radiance->eval(si);
    }

    MSK_DECLARE_CLASS()
private:
    ref<Texture> m_radiance;
    BoundingSphere3f m_bsphere;
    float m_surface_area;
};

MSK_IMPLEMENT_CLASS(ConstantBackgroundEmitter, Emitter)
MSK_REGISTER_INSTANCE(ConstantBackgroundEmitter, "constant")

} // namespace misaki