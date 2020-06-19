#include <misaki/render/camera.h>
#include <misaki/render/film.h>
#include <misaki/render/integrator.h>
#include <misaki/render/logger.h>
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
                auto position_sample = pos + sampler->next2d();
                block->put(position_sample, Color4(Color3(0.f, 0.f, float(i) / total_blocks)));
              }
            }
            film->put(block.get());
          }
        });
    return true;
  }

  MSK_DECL_COMP(Integrator)
 private:
};

MSK_EXPORT_PLUGIN(PathTracer)

}  // namespace misaki::render