#include <aspirin/bsdf.h>
#include <aspirin/properties.h>
#include <aspirin/texture.h>

namespace aspirin {

class MaskBSDF final : public BSDF {
 public:
  MaskBSDF(const Properties &props) : BSDF(props) {
    m_opacity = props.texture<Texture>("opacity", 0.5f);
    for (auto &kv : props.components()) {
      auto bsdf = std::dynamic_pointer_cast<BSDF>(kv.second);
      if (bsdf) {
        if (m_nested_bsdf)
          Throw("Cannot specify more than one child BSDF");
        m_nested_bsdf = bsdf;
      }
    }
    if (!m_nested_bsdf)
      Throw("Child BSDF not specified");
    m_components.clear();
    for (size_t i = 0; i < m_nested_bsdf->component_count(); ++i)
      m_components.push_back(m_nested_bsdf->flags(i));
    m_components.push_back(+BSDFFlags::Null);
    m_flags = m_nested_bsdf->flags() | m_components.back();
  }

  std::pair<BSDFSample, Color3> sample(const BSDFContext &ctx,
                                       const SceneInteraction &si,
                                       Float sample1,
                                       const Vector2 &sample_) const override {
    Vector2 sample(sample_);
    uint32_t null_index = (uint32_t)component_count() - 1;
    bool sample_transmission = ctx.is_enabled(BSDFFlags::Null, null_index);
    bool sample_nested = ctx.component == (uint32_t)-1 || ctx.component < null_index;
    BSDFSample bs;
    Color3 result = 0.f;
    if (!sample_transmission && !sample_nested) return {bs, result};
    auto opacity = m_opacity->eval_3(si.geom);
    Float prob = opacity.luminance();
    if (sample_transmission && sample_nested) {
      if (sample.x() < prob) {
        sample.x() /= prob;
        auto [bs_, bs_val_] = m_nested_bsdf->sample(ctx, si, sample1, sample);
        return {bs_, bs_val_ * opacity};
      } else {
        bs.wo = -si.wi;
        bs.eta = 1.f;
        bs.sampled_component = null_index;
        bs.sampled_type = +BSDFFlags::Null;
        bs.pdf = 1.f - prob;
        result = (1.f - opacity) / bs.pdf;
        return {bs, result};
      }
    } else if (sample_transmission) {
      bs.wo = -si.wi;
      bs.eta = 1.f;
      bs.sampled_component = null_index;
      bs.sampled_type = +BSDFFlags::Null;
      bs.pdf = 1.f;
      result = 1.f - opacity;
      return {bs, result};
    } else {
      auto [bs_, bs_val_] = m_nested_bsdf->sample(ctx, si, sample1, sample);
      return {bs_, bs_val_ * opacity};
    }
  }

  Color3 eval(const BSDFContext &ctx,
              const SceneInteraction &si,
              const Vector3 &wo) const override {
    return m_nested_bsdf->eval(ctx, si, wo) * m_opacity->eval_3(si.geom);
  }

  Float pdf(const BSDFContext &ctx,
            const SceneInteraction &si,
            const Vector3 &wo) const override {
    uint32_t null_index = (uint32_t)component_count() - 1;
    bool sample_transmission = ctx.is_enabled(BSDFFlags::Null, null_index);
    bool sample_nested = ctx.component == (uint32_t)-1 || ctx.component < null_index;

    if (!sample_nested)
      return 0.f;

    Float result = m_nested_bsdf->pdf(ctx, si, wo);
    if (sample_transmission)
      result *= m_opacity->eval_3(si.geom).luminance();

    return result;
  }

  MSK_DECL_COMP(BSDF)
 private:
  std::shared_ptr<Texture> m_opacity;
  std::shared_ptr<BSDF> m_nested_bsdf;
};

MSK_EXPORT_PLUGIN(MaskBSDF)
}  // namespace aspirin