#include <misaki/render/bsdf.h>
#include <misaki/render/emitter.h>
#include <misaki/render/film.h>
#include <misaki/render/imageblock.h>
#include <misaki/render/integrator.h>
#include <misaki/render/interaction.h>
#include <misaki/core/logger.h>
#include <misaki/render/mesh.h>
#include <misaki/core/properties.h>
#include <misaki/core/manager.h>
#include <misaki/render/records.h>
#include <misaki/render/scene.h>
#include <misaki/render/sensor.h>
#include <tbb/parallel_for.h>

#include <fstream>
#include <iostream>
#include <memory>

namespace misaki {

class DebugIntegrator final : public Integrator {
public:
    DebugIntegrator(const Properties &props) : Integrator(props) {}

    bool render(Scene *scene, Sensor *sensor) override {
        auto film      = sensor->film();
        auto film_size = film->size();
        auto total_spp = sensor->sampler()->sample_count();
        Log(Info, "Starting render job ({}x{}, {} sample)", film_size.x(),
            film_size.y(), total_spp);
        int m_block_size = APR_BLOCK_SIZE;
        BlockGenerator gen(film_size, Eigen::Vector2i::Zero(), m_block_size);
        size_t total_blocks = gen.block_count();
        tbb::parallel_for(
            tbb::blocked_range<size_t>(0, total_blocks, 1),
            [&](const tbb::blocked_range<size_t> &range) {
                auto sampler = sensor->sampler()->clone();
                auto block   = std::make_unique<ImageBlock>(
                    Eigen::Vector2i::Constant(m_block_size), film->filter());
                for (auto i = range.begin(); i != range.end(); ++i) {
                    auto [offset, size, block_id] = gen.next_block();
                    block->set_offset(offset);
                    block->set_size(size);
                    block->clear();
                    for (int y = 0; y < size.y(); ++y) {
                        for (int x = 0; x < size.x(); ++x) {
                            auto pos = Eigen::Vector2i(x, y);
                            if (pos.x() >= size.x() || pos.y() >= size.y())
                                continue;
                            auto pos_ = (pos + offset).template cast<float>();
                            auto position_sample   = pos_ + sampler->next2d();
                            auto [ray, ray_weight] = sensor->sample_ray(
                                position_sample, sampler->next2d());
                            auto si = scene->ray_intersect(ray);
                            if (si.is_valid()) {
                                auto spec =
                                    Spectrum(std::abs(si.sh_frame.n.x()),
                                             std::abs(si.sh_frame.n.y()),
                                             std::abs(si.sh_frame.n.z()));
                                block->put(position_sample, spec);
                            }
                        }
                    }
                    film->put(block.get());
                }
            });
        return true;
    }

    MSK_DECLARE_CLASS()
private:
};

MSK_IMPLEMENT_CLASS(DebugIntegrator, Integrator)
MSK_REGISTER_INSTANCE(DebugIntegrator, "debug")

} // namespace misaki