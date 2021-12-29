#include <misaki/render/bsdf.h>
#include <misaki/core/logger.h>
#include <misaki/core/manager.h>
#include <misaki/core/properties.h>
#include <misaki/render/texture.h>
#include <misaki/render/fresnel.h>
#include <misaki/core/warp.h>

namespace misaki {

class SmoothDielectric final : public BSDF {
public:
    SmoothDielectric(const Properties &props) : BSDF(props) {
        float int_ior = props.float_("int_ior", 1.49);
        float ext_ior = props.float_("ext_ior", 1.00028);
        m_eta         = int_ior / ext_ior;
        m_specular_reflectance =
            props.texture("specular_reflectance", 1.f);
        m_specular_transmittance =
            props.texture("specular_transmittance", 1.f);
        m_components.push_back(+BSDFFlags::DeltaReflection);
        m_components.push_back(+BSDFFlags::DeltaTransmission);
        m_flags = m_components[0] | m_components[1];
    }

    std::pair<BSDFSample, Spectrum>
    sample(const BSDFContext &ctx, const SceneInteraction &si, float sample1,
           const Eigen::Vector2f &sample) const override {
        bool has_reflection   = ctx.is_enabled(BSDFFlags::DeltaReflection, 0),
             has_transmission = ctx.is_enabled(BSDFFlags::DeltaTransmission, 1);
        float cos_theta_i     = Frame::cos_theta(si.wi);
        auto [r_i, cos_theta_t, eta_it, eta_ti] = fresnel(cos_theta_i, m_eta);
        float t_i                               = 1.f - r_i;
        BSDFSample bs;
        bool selected_r;
        if (has_reflection && has_transmission) {
            selected_r = sample.x() <= r_i;
            bs.pdf     = selected_r ? r_i : t_i;
        } else {
            if (has_reflection || has_transmission) {
                selected_r = has_reflection;
                bs.pdf     = 1.f;
            } else {
                return { bs, Spectrum::Zero() };
            }
        }
        bs.sampled_component = selected_r ? 0 : 1;
        bs.sampled_type      = selected_r ? +BSDFFlags::DeltaReflection
                                          : +BSDFFlags::DeltaTransmission;
        bs.wo =
            selected_r ? reflect(si.wi) : refract(si.wi, cos_theta_t, eta_ti);
        bs.eta                 = selected_r ? 1.f : eta_it;
        Spectrum reflectance   = Spectrum::Constant(1.f),
                 transmittance = Spectrum::Constant(1.f);
        if (selected_r)
            reflectance = m_specular_reflectance->eval_3(si);
        if (!selected_r)
            transmittance = m_specular_transmittance->eval_3(si);
        Spectrum weight;
        if (has_reflection && has_transmission)
            weight.setConstant(1.f);
        else if (has_reflection || has_transmission) {
            weight.setConstant(has_reflection ? r_i : t_i);
        }
        if (selected_r)
            weight *= reflectance;
        if (!selected_r) {
            float factor = (ctx.mode == TransportMode::Radiance) ? eta_ti : 1.f;
            weight *= transmittance * factor * factor;
        }
        return { bs, weight };
    }

    Spectrum eval(const BSDFContext &ctx, const SceneInteraction &si,
                  const Eigen::Vector3f &wo) const override {
        return Spectrum::Zero();
    }

    float pdf(const BSDFContext &ctx, const SceneInteraction &si,
              const Eigen::Vector3f &wo) const override {
        return 0;
    }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "SmoothDielectric[" << std::endl;
        if (m_specular_reflectance)
            oss << "  specular_reflectance = "
                << string::indent(m_specular_reflectance) << "," << std::endl;
        if (m_specular_transmittance)
            oss << "  specular_transmittance = "
                << string::indent(m_specular_transmittance) << ", "
                << std::endl;
        oss << "  eta = " << m_eta << "," << std::endl << "]";
        return oss.str();
    }

    MSK_DECLARE_CLASS()
private:
    float m_eta;
    ref<Texture> m_specular_reflectance, m_specular_transmittance;
};

MSK_IMPLEMENT_CLASS(SmoothDielectric, BSDF)
MSK_REGISTER_INSTANCE(SmoothDielectric, "dielectric")

} // namespace misaki