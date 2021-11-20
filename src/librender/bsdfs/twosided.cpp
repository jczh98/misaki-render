#include <misaki/render/bsdf.h>
#include <misaki/core/logger.h>
#include <misaki/core/manager.h>
#include <misaki/core/properties.h>
#include <misaki/render/texture.h>
#include <misaki/core/warp.h>

namespace misaki {

class TwoSidedBRDF final : public BSDF {
public:
    TwoSidedBRDF(const Properties &props) : BSDF(props) {
        auto bsdfs = props.objects();
        if (!bsdfs.empty())
            m_brdf[0] = static_cast<BSDF *>(bsdfs[0].second.get());
        if (bsdfs.size() == 2)
            m_brdf[1] = static_cast<BSDF *>(bsdfs[1].second.get());
        else if (bsdfs.size() > 2)
            Throw("At most two nested BSDFs can be specified!");
        if (!m_brdf[0])
            Throw("A nested one-sided material is required!");
        if (!m_brdf[1])
            m_brdf[1] = m_brdf[0];
        for (size_t i = 0; i < m_brdf[0]->component_count(); ++i) {
            m_components.push_back(m_brdf[0]->flags(i));
            m_flags = m_flags | m_components.back();
        }
        for (size_t i = 0; i < m_brdf[1]->component_count(); ++i) {
            m_components.push_back(m_brdf[1]->flags(i));
            m_flags = m_flags | m_components.back();
        }

        if (has_flag(m_flags, BSDFFlags::Transmission))
            Throw("Only materials without a transmission component can be "
                  "nested!");
    }

    std::pair<BSDFSample, Spectrum>
    sample(const BSDFContext &ctx_, const SceneInteraction &si_,
           float sample1, const Eigen::Vector2f &sample) const override {
        SceneInteraction si(si_);
        BSDFContext ctx(ctx_);
        BSDFSample bs;
        bool front_side = Frame::cos_theta(si.wi) > 0.f,
             back_side  = Frame::cos_theta(si.wi) < 0.f;
        auto ret        = std::make_pair(bs, Color3(0.f));
        if (front_side) {
            ret = m_brdf[0]->sample(ctx, si, sample1, sample);
        }
        if (back_side) {
            if (ctx.component != (uint32_t) -1)
                ctx.component -= (uint32_t) m_brdf[0]->component_count();
            si.wi.z() *= -1.f;
            auto result = m_brdf[1]->sample(ctx, si, sample1, sample);
            result.first.wo.z() *= -1.f;
            ret = result;
        }
        return ret;
    }

    Spectrum eval(const BSDFContext &ctx_, const SceneInteraction &si_,
                  const Eigen::Vector3f &wo_) const override {
        SceneInteraction si(si_);
        BSDFContext ctx(ctx_);
        Eigen::Vector3f wo(wo_);
        auto result     = Color3(0.f);
        bool front_side = Frame::cos_theta(si.wi) > 0.f,
             back_side  = Frame::cos_theta(si.wi) < 0.f;
        if (front_side) {
            result = m_brdf[0]->eval(ctx, si, wo);
        }
        if (back_side) {
            if (ctx.component != (uint32_t) -1)
                ctx.component -= (uint32_t) m_brdf[0]->component_count();
            si.wi.z() *= -1.f;
            wo.z() *= -1.f;
            result = m_brdf[1]->eval(ctx, si, wo);
        }
        return result;
    }

    float pdf(const BSDFContext &ctx_, const SceneInteraction &si_,
              const Eigen::Vector3f &wo_) const override {
        SceneInteraction si(si_);
        BSDFContext ctx(ctx_);
        Eigen::Vector3f wo(wo_);
        float result    = 0.f;
        bool front_side = Frame::cos_theta(si.wi) > 0.f,
             back_side  = Frame::cos_theta(si.wi) < 0.f;
        if (front_side) {
            result = m_brdf[0]->pdf(ctx, si, wo);
        }
        if (back_side) {
            if (ctx.component != (uint32_t) -1)
                ctx.component -= (uint32_t) m_brdf[0]->component_count();
            si.wi.z() *= -1.f;
            wo.z() *= -1.f;
            result = m_brdf[1]->pdf(ctx, si, wo);
        }
        return result;
    }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "TwoSidedBRDF[" << std::endl
            << "  brdf[0] = " << string::indent(m_brdf[0]) << std::endl
            << "  brdf[1] = " << string::indent(m_brdf[1]) << std::endl
            << "]";
        return oss.str();
    }

    MSK_DECLARE_CLASS()
private:
    ref<BSDF> m_brdf[2];
};

MSK_IMPLEMENT_CLASS(TwoSidedBRDF, BSDF)
MSK_REGISTER_INSTANCE(TwoSidedBRDF, "twosided")

} // namespace misaki