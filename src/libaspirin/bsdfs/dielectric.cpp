#include <aspirin/bsdf.h>
#include <aspirin/fresnel.h>
#include <aspirin/properties.h>
#include <aspirin/texture.h>

namespace aspirin {

template <typename Float, typename Spectrum>
class SmoothDielectric final : public BSDF<Float, Spectrum> {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Base = BSDF<Float, Spectrum>;
    using Base::m_components;
    using Base::m_flags;
    using typename Base::BSDFSample;
    using typename Base::SurfaceInteraction;
    using Texture = Texture<Float, Spectrum>;

    SmoothDielectric(const Properties &props) : Base(props) {
        Float int_ior = props.get_float("int_ior", 1.49);
        Float ext_ior = props.get_float("ext_ior", 1.00028);
        m_eta         = int_ior / ext_ior;
        m_specular_reflectance =
            props.texture<Texture>("specular_reflectance", 1.f);
        m_specular_transmittance =
            props.texture<Texture>("specular_transmittance", 1.f);
        m_components.push_back(+BSDFFlags::DeltaReflection);
        m_components.push_back(+BSDFFlags::DeltaTransmission);
        m_flags = m_components[0] | m_components[1];
    }

    std::pair<BSDFSample, Spectrum>
    sample(const BSDFContext &ctx, const SurfaceInteraction &si, Float sample1,
           const Vector2 &sample) const override {
        bool has_reflection   = ctx.is_enabled(BSDFFlags::DeltaReflection, 0),
             has_transmission = ctx.is_enabled(BSDFFlags::DeltaTransmission, 1);
        Float cos_theta_i     = Frame3::cos_theta(si.wi);
        auto [r_i, cos_theta_t, eta_it, eta_ti] = fresnel(cos_theta_i, m_eta);
        Float t_i                               = 1.f - r_i;
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
        Color3 weight;
        if (has_reflection && has_transmission)
            weight = 1.f;
        else if (has_reflection || has_transmission) {
            weight = has_reflection ? r_i : t_i;
        }
        if (selected_r)
            weight *= reflectance;
        if (!selected_r) {
            Float factor = (ctx.mode == TransportMode::Radiance) ? eta_ti : 1.f;
            weight *= transmittance * factor * factor;
        }
        return { bs, weight };
    }

    Spectrum eval(const BSDFContext &ctx, const SurfaceInteraction &si,
                  const Vector3 &wo) const override {
        return Spectrum::Zero();
    }

    Float pdf(const BSDFContext &ctx, const SurfaceInteraction &si,
              const Vector3 &wo) const override {
        return 0;
    }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "SmoothDielectric[" << std::endl;
        if (m_specular_reflectance)
            oss << "  specular_reflectance = " << string::indent(m_specular_reflectance) << "," << std::endl;
        if (m_specular_transmittance)
            oss << "  specular_transmittance = " << string::indent(m_specular_transmittance) << ", " << std::endl;
        oss << "  eta = " << m_eta << "," << std::endl
            << "]";
        return oss.str();
    }

    APR_DECLARE_CLASS()
private:
    Float m_eta;
    ref<Texture> m_specular_reflectance, m_specular_transmittance;
};

APR_IMPLEMENT_CLASS_VARIANT(SmoothDielectric, BSDF)
APR_INTERNAL_PLUGIN(SmoothDielectric, "dielectric")

} // namespace aspirin