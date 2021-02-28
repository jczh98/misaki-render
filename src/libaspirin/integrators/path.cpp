#include <aspirin/bsdf.h>
#include <aspirin/camera.h>
#include <aspirin/film.h>
#include <aspirin/integrator.h>
#include <aspirin/interaction.h>
#include <aspirin/light.h>
#include <aspirin/logger.h>
#include <aspirin/mesh.h>
#include <aspirin/properties.h>
#include <aspirin/records.h>
#include <aspirin/scene.h>
#include <tbb/parallel_for.h>

#include <fstream>

namespace aspirin {

class PathTracer final : public Integrator {
 public:
  PathTracer(const Properties &props) : Integrator(props) {
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
                  auto result = sample_ems(scene, sampler.get(), ray);
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
    Color3 throughput(1.f), result(0.f);
    Float emission_weight = 1.f, eta = 1.f;
    auto si = scene->ray_intersect(ray);
    auto light = si.light(scene);
    for (int depth = 1;; ++depth) {
      if (light != nullptr) {
        result += light->eval(si) * throughput * emission_weight;
      }
      if (depth >= m_rr_depth) {
        Float q = std::min(throughput.maxCoeff() * eta * eta, 0.95f);
        if (sampler->next1d() >= q) break;
        throughput *= 1.f / q;
      }
      if ((uint32_t)depth >= (uint32_t)m_max_depth || !si.is_valid()) break;
      // ------------------Direct sample light--------------------------
      BSDFContext ctx;
      const BSDF *bsdf = si.bsdf(ray);
      if (has_flag(bsdf->flags(), BSDFFlags::Smooth)) {
        auto [ds, emit_val] = scene->sample_direct_light(si.geom, sampler->next2d());
        if (ds.pdf != 0.f) {
          auto wo = si.to_local(ds.d);
          Color3 bsdf_val = bsdf->eval(ctx, si, wo);
          auto bsdf_pdf = bsdf->pdf(ctx, si, wo);
          Float mis = ds.geom.degenerated ? 1.f : mis_weight(ds.pdf, bsdf_pdf);
          result += throughput * bsdf_val * emit_val * mis;
        }
      }
      // --------------------- BSDF Sampling ------------------------
      // Sample BSDF * cos(theta)
      auto [bs, bsdf_val] = bsdf->sample(ctx, si, sampler->next1d(), sampler->next2d());
      throughput *= bsdf_val;
      eta *= bs.eta;
      ray = si.spawn_ray(si.to_world(bs.wo));
      auto si_bsdf = scene->ray_intersect(ray);
      light = si_bsdf.light(scene);
      auto ds = DirectSample::make_with_interactions(si_bsdf, si);
      if (light != nullptr) {
        auto light_pdf = !has_flag(bs.sampled_type, BSDFFlags::Delta) ? scene->pdf_direct_light(si.geom, ds, light) : 0.f;
        emission_weight = mis_weight(bs.pdf, light_pdf);
      }
      si = std::move(si_bsdf);
    }
    return result;
  }

  std::optional<Color3> sample_ems(const std::shared_ptr<Scene> &scene, Sampler *sampler, const Ray &ray_) const {
    auto ray = ray_;
    Color3 throughput(1.f), result(0.f);
    Float eta = 1.f;
    auto si = scene->ray_intersect(ray);
    auto light = si.light(scene);
    const Light *last_light = nullptr;
    bool last_specular = false;
    for (int depth = 1;; ++depth) {
      if (light != nullptr && (depth == 1 || last_light == light || last_specular)) {
        result += light->eval(si) * throughput;
      }
      if (depth >= m_rr_depth) {
        Float q = std::min(throughput.maxCoeff() * eta * eta, 0.95f);
        if (sampler->next1d() >= q) break;
        throughput *= 1.f / q;
      }
      if ((uint32_t)depth >= (uint32_t)m_max_depth || !si.is_valid()) break;
      // ------------------Direct sample light--------------------------
      BSDFContext ctx;
      const BSDF *bsdf = si.bsdf(ray);
      if (has_flag(bsdf->flags(), BSDFFlags::Smooth)) {
        auto [ds, emit_val] = scene->sample_direct_light(si.geom, sampler->next2d());
        if (ds.pdf != 0.f) {
          auto wo = si.to_local(ds.d);
          Color3 bsdf_val = bsdf->eval(ctx, si, wo);
          auto bsdf_pdf = bsdf->pdf(ctx, si, wo);
          result += throughput * bsdf_val * emit_val;
        }
        last_specular = false;
      } else {
        last_specular = true;
      }
      // --------------------- BSDF Sampling ------------------------
      // Sample BSDF * cos(theta)
      auto [bs, bsdf_val] = bsdf->sample(ctx, si, sampler->next1d(), sampler->next2d());
      throughput *= bsdf_val;
      eta *= bs.eta;
      ray = si.spawn_ray(si.to_world(bs.wo));
      auto si_bsdf = scene->ray_intersect(ray);
      light = si_bsdf.light(scene);
      last_light = si.light(scene);
      si = std::move(si_bsdf);
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
  int m_max_depth = -1, m_rr_depth = 5;
};

MSK_EXPORT_PLUGIN(PathTracer)

}  // namespace aspirin