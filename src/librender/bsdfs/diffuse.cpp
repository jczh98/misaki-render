#include <misaki/render/bsdf.h>
#include <misaki/core/logger.h>
#include <misaki/core/manager.h>
#include <misaki/core/properties.h>
#include <misaki/render/texture.h>
#include <misaki/core/warp.h>

namespace misaki {

class SmoothDiffuse final : public BSDF {
public:
    SmoothDiffuse(const Properties &props) : BSDF(props) {
        m_reflectance = props.texture("reflectance", 0.5f);
        m_flags       = +BSDFFlags::DiffuseReflection;
        m_components.push_back(m_flags);
    }

    std::pair<BSDFSample, Spectrum>
    sample(const BSDFContext &ctx, const SurfaceInteraction &si, float sample1,
           const Eigen::Vector2f &sample) const override {
        float cos_theta_i = Frame::cos_theta(si.wi);
        BSDFSample bs;
        if (cos_theta_i <= 0.f || !ctx.is_enabled(BSDFFlags::DiffuseReflection))
            return { bs, Spectrum::Zero() };
        bs.wo                = warp::square_to_cosine_hemisphere(sample);
        bs.pdf               = warp::square_to_cosine_hemisphere_pdf(bs.wo);
        bs.eta               = 1.f;
        bs.sampled_type      = +BSDFFlags::DiffuseReflection;
        bs.sampled_component = 0;
        return { bs,
                 bs.pdf > 0.f ? m_reflectance->eval_3(si) : Spectrum::Zero() };
    }

    Spectrum eval(const BSDFContext &ctx, const SurfaceInteraction &si,
                  const Eigen::Vector3f &wo) const override {
        if (!ctx.is_enabled(BSDFFlags::DiffuseReflection))
            return Spectrum::Zero();

        float cos_theta_i = Frame::cos_theta(si.wi),
              cos_theta_o = Frame::cos_theta(wo);
        if (cos_theta_i > 0.f && cos_theta_o > 0.f) {
            return m_reflectance->eval_3(si) * math::InvPi<float> * cos_theta_o;
        } else {
            return Spectrum::Zero();
        }
    }

    float pdf(const BSDFContext &ctx, const SurfaceInteraction &si,
              const Eigen::Vector3f &wo) const override {
        if (!ctx.is_enabled(BSDFFlags::DiffuseReflection))
            return 0.f;
        if (Frame::cos_theta(si.wi) > 0.f && Frame::cos_theta(wo) > 0.f) {
            return warp::square_to_cosine_hemisphere_pdf(wo);
        } else {
            return 0.f;
        }
    }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "SmoothDiffuse[" << std::endl
            << "  reflectance = " << string::indent(m_reflectance->to_string())
            << std::endl
            << "]";
        return oss.str();
    }

    MSK_DECLARE_CLASS()
protected:
    ref<Texture> m_reflectance;
};

MSK_IMPLEMENT_CLASS(SmoothDiffuse, BSDF)
MSK_REGISTER_INSTANCE(SmoothDiffuse, "diffuse")

} // namespace misaki