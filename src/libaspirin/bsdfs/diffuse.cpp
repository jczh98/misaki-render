#include <aspirin/bsdf.h>
#include <aspirin/logger.h>
#include <aspirin/plugin.h>
#include <aspirin/properties.h>
#include <aspirin/texture.h>
#include <aspirin/warp.h>

namespace aspirin {

class SmoothDiffuse final : public BSDF {
public:
    SmoothDiffuse(const Properties &props) : BSDF(props) {
        m_reflectance = props.texture<Texture>("reflectance", 0.5f);
        m_flags       = +BSDFFlags::DiffuseReflection;
        m_components.push_back(m_flags);
    }

    std::pair<BSDFSample, Spectrum>
    sample(const BSDFContext &ctx, const SurfaceInteraction &si, Float sample1,
           const Vector2 &sample) const override {
        Float cos_theta_i = Frame3::cos_theta(si.wi);
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
                  const Vector3 &wo) const override {
        if (!ctx.is_enabled(BSDFFlags::DiffuseReflection))
            return Spectrum::Zero();

        Float cos_theta_i = Frame3::cos_theta(si.wi),
              cos_theta_o = Frame3::cos_theta(wo);
        if (cos_theta_i > 0.f && cos_theta_o > 0.f) {
            return m_reflectance->eval_3(si) * math::InvPi<Float> * cos_theta_o;
        } else {
            return Spectrum::Zero();
        }
    }

    Float pdf(const BSDFContext &ctx, const SurfaceInteraction &si,
              const Vector3 &wo) const override {
        if (!ctx.is_enabled(BSDFFlags::DiffuseReflection))
            return 0.f;
        if (Frame3::cos_theta(si.wi) > 0.f && Frame3::cos_theta(wo) > 0.f) {
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

    APR_DECLARE_CLASS()
protected:
    ref<Texture> m_reflectance;
};

APR_IMPLEMENT_CLASS(SmoothDiffuse, BSDF)
APR_INTERNAL_PLUGIN(SmoothDiffuse, "diffuse")

} // namespace aspirin