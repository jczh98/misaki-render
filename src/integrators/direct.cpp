#include <misaki/render/bsdf.h>
#include <misaki/render/camera.h>
#include <misaki/render/film.h>
#include <misaki/render/integrator.h>
#include <misaki/render/interaction.h>
#include <misaki/render/light.h>
#include <misaki/render/logger.h>
#include <misaki/render/mesh.h>
#include <misaki/render/properties.h>
#include <misaki/render/scene.h>
#include <tbb/parallel_for.h>

#include <fstream>

namespace misaki::render {

class DirectIntegrator final : public Integrator {
 public:
  DirectIntegrator(const Properties &props) : Integrator(props) {
    m_light_samples = props.get_int("light_samples", 1);
    m_bsdf_samples = props.get_int("bsdf_samples", 1);
    size_t sum = m_light_samples + m_bsdf_samples;
    m_weight_bsdf = 1.f / (Float)m_bsdf_samples;
    m_weight_lum = 1.f / (Float)m_light_samples;
    m_frac_bsdf = m_bsdf_samples / (Float)sum;
    m_frac_lum = m_light_samples / (Float)sum;
  }

  bool render(const std::shared_ptr<Scene> &scene) {
    auto camera = scene->camera();
    auto film = camera->film();
    auto film_size = film->size();
    auto total_spp = camera->sampler()->sample_count();
    Log(Info, "Starting render job ({}x{}, {} sample)", film_size.x(), film_size.y(), total_spp);
    BlockGenerator gen(film_size, m_block_size);
    size_t total_blocks = gen.block_count();
    util::ProgressBar pbar(total_blocks, 70);
    util::Timer timer;
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, total_blocks, 1),
        [&](const tbb::blocked_range<size_t> &range) {
          auto sampler = camera->sampler()->clone();
          auto block = std::make_unique<ImageBlock>(m_block_size, film->filter());
          for (auto i = range.begin(); i != range.end(); ++i) {
            auto [offset, size] = gen.next_block();
            block->set_offset(offset);
            block->set_size(size);
            block->clear();
            for (int y = 0; y < size.y(); ++y) {
              for (int x = 0; x < size.x(); ++x) {
                Vector2 pos = Vector2(x, y);
                if (pos.x() >= size.x() || pos.y() >= size.y()) continue;
                pos = pos + Vector2(offset);
                for (int s = 0; s < total_spp; ++s) {
                  auto position_sample = pos + sampler->next2d();
                  auto [ray, ray_weight] = camera->sample_ray(position_sample);
                  auto result = sample(scene, sampler.get(), ray);
                  if (result)
                    block->put(position_sample, Color4(*result));
                  else
                    block->put(position_sample, Color4(0.f, 0.f, 0.f, 0.f));
                }
              }
            }
            film->put(block.get());
            pbar.update();
          }
        });
    pbar.done();
    Log(Info, "Rendering finished. (took {})", util::time_string(timer.value(), true));
    return true;
  }

  std::optional<Color3> sample(const std::shared_ptr<Scene> &scene, Sampler *sampler, const Ray &ray_) const {
    auto ray = ray_;
    Color3 result(0.f);
    auto si = scene->ray_intersect(ray);
    if (!si) {
      const auto wi = -ray.d;
      if (scene->environment()) result += scene->environment()->eval({}, wi);
      return result;
    }
    const Light *light = si->shape->light();
    const auto wi = si->to_local(-ray.d);
    if (light != nullptr) {
      result += light->eval(si->geom, wi);
    }
    // -----------------Light Sampling-----------------------
    BSDFContext ctx;
    const BSDF *bsdf = si->shape->bsdf();
    if (has_flag(bsdf->flags(), BSDFFlags::Diffuse)) {
      for (size_t i = 0; i < m_light_samples; ++i) {
        auto [ds, emit_val] = scene->sample_direct_light(si->geom, sampler->next2d());
        if (ds.pdf != 0.f) {
          auto wo = si->to_local(ds.d);
          Color3 bsdf_val = bsdf->eval(ctx, si->geom, wi, wo);
          auto bsdf_pdf = bsdf->pdf(ctx, si->geom, wi, wo);
          Float mis = ds.geom.degenerated ? 1.f : mis_weight(ds.pdf * m_frac_lum, bsdf_pdf * m_frac_bsdf) * m_weight_lum;
          result += mis * bsdf_val * emit_val;
        }
      }
    }
    // ------------------BSDF Sampling-----------------------
    for (size_t i = 0; i < m_bsdf_samples; ++i) {
      auto [bs, bsdf_val] = bsdf->sample(ctx, si->geom, wi, sampler->next2d());
      if (bsdf_val != 0.f) {
        auto wo_world = si->to_world(bs.wo);
        auto new_ray = ray;
        new_ray.spawn(si->geom, wo_world);
        auto si_bsdf = scene->ray_intersect(new_ray);
        if (si_bsdf) {
          auto wi_bsdf = si_bsdf->to_local(-new_ray.d);
          const Light *light_bsdf = si_bsdf->shape->light();
          if (light_bsdf != nullptr) {
            auto emit_val = light_bsdf->eval(si_bsdf->geom, wi_bsdf);
            auto ds_bsdf = DirectSample::make_between_geometries(si_bsdf->geom, si->geom);
            auto light_pdf = scene->pdf_direct_light(si_bsdf->geom, ds_bsdf, light_bsdf);
            Float mis = mis_weight(bs.pdf * m_frac_bsdf, light_pdf * m_frac_lum) * m_weight_bsdf;
            result += bsdf_val * emit_val * mis;
          }
        } else {
          DirectSample ds;
          ds.d = ray.d;
          const auto light_env = scene->environment();
          if (light_env != nullptr) {
            auto emit_val = light_env->eval({}, -wo_world);
            auto light_pdf = !has_flag(bs.sampled_type, BSDFFlags::Delta) ? scene->pdf_direct_light(si->geom, ds, light_env) : 0.f;
            Float mis = mis_weight(bs.pdf * m_frac_bsdf, light_pdf * m_frac_lum) * m_weight_bsdf;
            result += bsdf_val * emit_val * mis;
          }
        }
      }
    }
    return result;
  }

  Float mis_weight(Float pdf_a, Float pdf_b) const {
    pdf_a *= pdf_a;
    pdf_b *= pdf_b;
    return pdf_a > 0.f ? pdf_a / (pdf_a + pdf_b) : 0.f;
  }

  MSK_DECL_COMP(Integrator)
 private:
  int m_light_samples = -1, m_bsdf_samples = 5;
  Float m_frac_bsdf, m_frac_lum;
  Float m_weight_bsdf, m_weight_lum;
};

MSK_EXPORT_PLUGIN(DirectIntegrator)

}  // namespace misaki::render