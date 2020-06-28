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
    Color3 throughput(1.f), result(0.f);
    auto si = scene->ray_intersect(ray);
    for (int depth = 1;; ++depth) {
      if (!si) {
        // Handle enviroment lighting
        break;
      }
      const Light *light = si->shape->light();
      const auto wi = si->to_local(-ray.d);
      if (light != nullptr) {
        result += light->eval(si->geom, wi) * throughput;
      }
      if (depth >= m_rr_depth) {
        Float q = std::min(throughput.maxCoeff(), 0.95f);
        if (sampler->next1d() >= q) break;
        throughput *= 1.f / q;
      }
      if ((uint32_t)depth >= (uint32_t)m_max_depth) break;
      // ------------------Direct sample light--------------------------
      BSDFContext ctx;
      const BSDF *bsdf = si->shape->bsdf();
      if (has_flag(bsdf->flags(), BSDFFlags::Diffuse)) {
        auto [ds, emit_val] = scene->sample_direct_light(si->geom, sampler->next2d());
        auto wo = si->to_local(ds.d);
        auto bsdf_val = bsdf->eval(ctx, si->geom, wi, wo) * std::abs(math::dot(ds.d, si->geom.sh_frame.n));
        auto bsdf_pdf = bsdf->pdf(ctx, si->geom, wi, wo);
        result += throughput * bsdf_val * emit_val;
      }
      // --------------------- BSDF Sampling ------------------------
      // Sample BSDF * cos(theta)
      auto [bs, bsdf_val] = bsdf->sample(ctx, si->geom, wi, sampler->next2d());
      auto wo_world = si->to_world(bs.wo);
      //throughput *= bsdf_val * std::abs(math::dot(wo_world, si->geom.sh_frame.n)) / bs.pdf;
      ray.spawn(si->geom, wo_world);
      auto si_bsdf = scene->ray_intersect(ray);
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
  int m_max_depth = -1, m_rr_depth = 3;
};

MSK_EXPORT_PLUGIN(PathTracer)

}  // namespace misaki::render