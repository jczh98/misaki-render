#include <misaki/render/bsdf.h>
#include <misaki/render/properties.h>

namespace misaki::render {

class TwoSidedBRDF final : public BSDF {
 public:
  TwoSidedBRDF(const Properties &props) : BSDF(props) {
    auto bsdfs = props.components();
    if (bsdfs.size() > 0)
      m_brdf[0] = std::dynamic_pointer_cast<BSDF>(bsdfs[0].second);
    if (bsdfs.size() == 2)
      m_brdf[1] = std::dynamic_pointer_cast<BSDF>(bsdfs[1].second);
    else if (bsdfs.size() > 2)
      Throw("At most two nested BSDFs can be specified!");
    if (!m_brdf[0]) Throw("A nested one-sided material is required!");
    if (!m_brdf[1]) m_brdf[1] = m_brdf[0];
    for (size_t i = 0; i < m_brdf[0]->component_count(); ++i) {
      m_components.push_back(m_brdf[0]->flags(i));
      m_flags = m_flags | m_components.back();
    }
    for (size_t i = 0; i < m_brdf[1]->component_count(); ++i) {
      m_components.push_back(m_brdf[1]->flags(i));
      m_flags = m_flags | m_components.back();
    }

    if (has_flag(m_flags, BSDFFlags::Transmission))
      Throw("Only materials without a transmission component can be nested!");
  }

  std::pair<BSDFSample, Color3> sample(const BSDFContext &ctx_,
                                       const SceneInteraction &si_,
                                       Float sample1,
                                       const Vector2 &sample) const override {
    SceneInteraction si(si_);
    BSDFContext ctx(ctx_);
    BSDFSample bs;
    bool front_side = Frame::cos_theta(si.wi) > 0.f,
         back_side = Frame::cos_theta(si.wi) < 0.f;
    auto ret = std::make_pair(bs, Color3(0.f));
    if (front_side) {
      ret = m_brdf[0]->sample(ctx, si, sample1, sample);
    }
    if (back_side) {
      if (ctx.component != (uint32_t)-1)
        ctx.component -= (uint32_t)m_brdf[0]->component_count();
      si.wi.z() *= -1.f;
      auto result = m_brdf[1]->sample(ctx, si, sample1, sample);
      result.first.wo.z() *= -1.f;
      ret = result;
    }
    return ret;
  }

  Color3 eval(const BSDFContext &ctx_,
              const SceneInteraction &si_,
              const Vector3 &wo_) const override {
    SceneInteraction si(si_);
    BSDFContext ctx(ctx_);
    Vector3 wo(wo_);
    auto result = Color3(0.f);
    bool front_side = Frame::cos_theta(si.wi) > 0.f,
         back_side = Frame::cos_theta(si.wi) < 0.f;
    if (front_side) {
      result = m_brdf[0]->eval(ctx, si, wo);
    }
    if (back_side) {
      if (ctx.component != (uint32_t)-1)
        ctx.component -= (uint32_t)m_brdf[0]->component_count();
      si.wi.z() *= -1.f;
      wo.z() *= -1.f;
      result = m_brdf[1]->eval(ctx, si, wo);
    }
    return result;
  }

  Float pdf(const BSDFContext &ctx_,
            const SceneInteraction &si_,
            const Vector3 &wo_) const override {
    SceneInteraction si(si_);
    BSDFContext ctx(ctx_);
    Vector3 wo(wo_);
    Float result = 0.f;
    bool front_side = Frame::cos_theta(si.wi) > 0.f,
         back_side = Frame::cos_theta(si.wi) < 0.f;
    if (front_side) {
      result = m_brdf[0]->pdf(ctx, si, wo);
    }
    if (back_side) {
      if (ctx.component != (uint32_t)-1)
        ctx.component -= (uint32_t)m_brdf[0]->component_count();
      si.wi.z() *= -1.f;
      wo.z() *= -1.f;
      result = m_brdf[1]->pdf(ctx, si, wo);
    }
    return result;
  }

  MSK_DECL_COMP(BSDF)
 private:
  std::shared_ptr<BSDF> m_brdf[2];
};

MSK_EXPORT_PLUGIN(TwoSidedBRDF)

}  // namespace misaki::render