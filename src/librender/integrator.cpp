#include <fstream>
#include <misaki/core/logger.h>
#include <misaki/core/manager.h>
#include <misaki/core/properties.h>
#include <misaki/core/utils.h>
#include <misaki/render/bsdf.h>
#include <misaki/render/emitter.h>
#include <misaki/render/film.h>
#include <misaki/render/integrator.h>
#include <misaki/render/interaction.h>
#include <misaki/render/mesh.h>
#include <misaki/render/records.h>
#include <misaki/render/scene.h>
#include <misaki/render/sensor.h>
#include <tbb/parallel_for.h>

namespace misaki {

Integrator::Integrator(const Properties &props) {
    m_block_size = props.int_("block_size", MSK_BLOCK_SIZE);
}

Integrator::~Integrator() {}

bool Integrator::render(Scene *scene, Sensor *sensor) {
    MSK_NOT_IMPLEMENTED("render");
}

MonteCarloIntegrator::MonteCarloIntegrator(const Properties &props)
    : Integrator(props) {}

MonteCarloIntegrator::~MonteCarloIntegrator() {}

bool MonteCarloIntegrator::render(Scene *scene, Sensor *sensor) {
    auto film      = sensor->film();
    auto film_size = film->size();
    auto total_spp = sensor->sampler()->sample_count();
    Log(Info, "Starting render job ({}x{}, {} sample)", film_size.x(),
        film_size.y(), total_spp);
    int m_block_size = MSK_BLOCK_SIZE;
    BlockGenerator gen(film_size, Eigen::Vector2i::Zero(), m_block_size);
    size_t total_blocks = gen.block_count();
    ProgressBar pbar(total_blocks, 70);
    Timer timer;
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, total_blocks, 1),
        [&](const tbb::blocked_range<size_t> &range) {
            auto sampler          = sensor->sampler()->clone();
            ref<ImageBlock> block = new ImageBlock(
                Eigen::Vector2i::Constant(m_block_size), film->filter());
            for (auto i = range.begin(); i != range.end(); ++i) {
                auto [offset, size, block_id] = gen.next_block();
                block->set_offset(offset);
                block->set_size(size);
                render_block(scene, sensor, sampler, block, total_spp);
                film->put(block);
                pbar.update();
            }
        });
    pbar.done();
    Log(Info, "Rendering finished. (took {})",
        time_string(timer.value(), true));
    return true;
}

void MonteCarloIntegrator::render_block(const Scene *scene,
                                        const Sensor *sensor, Sampler *sampler,
                                        ImageBlock *block,
                                        size_t sample_count) const {
    block->clear();
    auto &size              = block->size();
    auto &offset            = block->offset();
    float diff_scale_factor = float(1) / std::sqrt(sample_count);
    for (int y = 0; y < size.y(); ++y) {
        for (int x = 0; x < size.x(); ++x) {
            Eigen::Vector2f pos = Eigen::Vector2f(x, y);
            if (pos.x() >= size.x() || pos.y() >= size.y())
                continue;
            pos = pos + offset.template cast<float>();
            for (int s = 0; s < sample_count; ++s) {
                render_sample(scene, sensor, sampler, block, pos,
                              diff_scale_factor);
            }
        }
    }
}

void MonteCarloIntegrator::render_sample(const Scene *scene,
                                         const Sensor *sensor, Sampler *sampler,
                                         ImageBlock *block,
                                         const Eigen::Vector2f &pos,
                                         float diff_scale_factor) const {
    Eigen::Vector2f position_sample = pos + sampler->next2d();
    auto [ray, ray_weight] =
        sensor->sample_ray_differential(position_sample, sampler->next2d());
    ray.scale_differential(diff_scale_factor);
    auto result = sample(scene, sampler, ray);
    block->put(position_sample, result);
}

MSK_IMPLEMENT_CLASS(Integrator, Object, "integrator")
MSK_IMPLEMENT_CLASS(MonteCarloIntegrator, Integrator)

} // namespace misaki