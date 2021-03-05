#include <aspirin/bsdf.h>
#include <aspirin/emitter.h>
#include <aspirin/film.h>
#include <aspirin/imageblock.h>
#include <aspirin/integrator.h>
#include <aspirin/interaction.h>
#include <aspirin/logger.h>
#include <aspirin/mesh.h>
#include <aspirin/properties.h>
#include <aspirin/records.h>
#include <aspirin/scene.h>
#include <aspirin/sensor.h>
#include <tbb/parallel_for.h>

#include <fstream>
#include <iostream>
#include <memory>

namespace aspirin {

template <typename Float, typename Spectrum>
class DebugIntegrator final : public Integrator<Float, Spectrum> {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Base = Integrator<Float, Spectrum>;
    using typename Base::Scene;
    using typename Base::Sensor;
    using ImageBlock = ImageBlock<Float, Spectrum>;

    DebugIntegrator(const Properties &props) : Base(props) {}

    bool render(Scene *scene, Sensor *sensor) override {
        auto film      = sensor->film();
        auto film_size = film->size();
        auto total_spp = sensor->sampler()->sample_count();
        Log(Info, "Starting render job ({}x{}, {} sample)", film_size.x(),
            film_size.y(), total_spp);
        int m_block_size = APR_BLOCK_SIZE;
        BlockGenerator gen(film_size, m_block_size);
        size_t total_blocks = gen.block_count();
        tbb::parallel_for(
            tbb::blocked_range<size_t>(0, total_blocks, 1),
            [&](const tbb::blocked_range<size_t> &range) {
                auto sampler = sensor->sampler()->clone();
                auto block   = std::make_unique<ImageBlock>(
                    Vector2i::Constant(m_block_size), film->filter());
                for (auto i = range.begin(); i != range.end(); ++i) {
                    auto [offset, size] = gen.next_block();
                    block->set_offset(offset);
                    block->set_size(size);
                    block->clear();
                    for (int y = 0; y < size.y(); ++y) {
                        for (int x = 0; x < size.x(); ++x) {
                            Vector2 pos = Vector2(x, y);
                            if (pos.x() >= size.x() || pos.y() >= size.y())
                                continue;
                            pos = pos + offset.template cast<Float>();
                            for (int s = 0; s < total_spp; ++s) {
                                auto position_sample = pos + sampler->next2d();
                                auto [ray, ray_weight] =
                                    sensor->sample_ray(position_sample);
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
                    }
                    film->put(block.get());
                }
            });
        return true;
    }

    APR_DECLARE_CLASS()
private:
};

APR_IMPLEMENT_CLASS_VARIANT(DebugIntegrator, Integrator)
APR_INTERNAL_PLUGIN(DebugIntegrator, "debug")

} // namespace aspirin