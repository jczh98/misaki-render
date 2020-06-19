#include <misaki/render/camera.h>
#include <misaki/render/film.h>
#include <misaki/render/integrator.h>
#include <misaki/render/logger.h>
#include <misaki/render/properties.h>
#include <misaki/render/scene.h>
#include <tbb/tbb.h>

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
    Log(Info, "Starting render job ({}x{}, {} sample{})", film_size.x(), film_size.y(), total_spp);
    BlockGenerator gen(film_size, m_block_size);
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, total_spp, 1),
        [&](const tbb::blocked_range<size_t> &range) {
          auto sampler = camera->sampler()->clone();
          auto block = std::make_unique<ImageBlock>(m_block_size, film->filter());
          for (auto i = range.begin(); i != range.end(); ++i) {
            auto [offset, size] = gen.next_block();
            block->set_offset(offset);
            block->set_size(size);
            film->put(block.get());
          }
        });
  }

 private:
};

MSK_EXPORT_PLUGIN(PathTracer)

}  // namespace misaki::render