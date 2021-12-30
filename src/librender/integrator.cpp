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
SamplingIntegrator::SamplingIntegrator(const Properties &props)
    : Integrator(props) {
    m_block_size = (uint32_t) props.int_("block_size", MSK_BLOCK_SIZE);

    /// Disable direct visibility of emitters if needed
    m_hide_emitters = props.bool_("hide_emitters", false);
}

SamplingIntegrator::~SamplingIntegrator() {
}

std::vector<std::string> SamplingIntegrator::aov_names() const { return {}; }

bool SamplingIntegrator::render(Scene *scene, Sensor *sensor) {
    ref<Film> film            = sensor->film();
    Eigen::Vector2i film_size = film->size();
    auto total_spp            = sensor->sampler()->sample_count();

    std::vector<std::string> channels = aov_names();
    bool has_aovs                     = !channels.empty();
    // Insert default channels and set up the film
    for (size_t i = 0; i < 5; ++i)
        channels.insert(channels.begin() + i, std::string(1, "XYZAW"[i]));
    film->prepare(channels);

    m_render_timer.reset();

    Log(Info, "Starting render job ({}x{}, {} sample)", film_size.x(),
        film_size.y(), total_spp);

    BlockGenerator gen(film_size, Eigen::Vector2i::Zero(), m_block_size);

    size_t total_blocks = gen.block_count();

    ProgressBar pbar(total_blocks, 70);

    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, total_blocks, 1),
        [&](const tbb::blocked_range<size_t> &range) {
            ref<Sampler> sampler  = sensor->sampler()->clone();
            ref<ImageBlock> block =
                new ImageBlock(Eigen::Vector2i::Constant(m_block_size),
                               channels.size(), film->filter(), !has_aovs);

            std::unique_ptr<float[]> aovs(new float[channels.size()]);

            for (auto i = range.begin(); i != range.end(); ++i) {
                auto [offset, size, block_id] = gen.next_block();
                block->set_offset(offset);
                block->set_size(size);

                render_block(scene, sensor, sampler, block, aovs.get(),
                             total_spp);

                film->put(block);
                pbar.update();
            }
        });
    pbar.done();
    Log(Info, "Rendering finished. (took {})",
        time_string(m_render_timer.value(), true));
    return true;
}

void SamplingIntegrator::render_block(const Scene *scene, const Sensor *sensor,
                                      Sampler *sampler, ImageBlock *block,
                                      float *aovs, size_t sample_count) const {
    block->clear();
    Eigen::Vector2i size    = block->size();
    Eigen::Vector2i offset  = block->offset();
    float diff_scale_factor = float(1) / std::sqrt(sample_count);
    for (int y = 0; y < size.y(); ++y) {
        for (int x = 0; x < size.x(); ++x) {
            Eigen::Vector2f pos = Eigen::Vector2f(x, y);
            if (pos.x() >= size.x() || pos.y() >= size.y())
                continue;
            pos = pos + offset.template cast<float>();
            for (int s = 0; s < sample_count; ++s) {
                render_sample(scene, sensor, sampler, block, aovs, pos,
                              diff_scale_factor);
            }
        }
    }
}

void SamplingIntegrator::render_sample(const Scene *scene, const Sensor *sensor,
                                       Sampler *sampler, ImageBlock *block,
                                       float *aovs, const Eigen::Vector2f &pos,
                                       float diff_scale_factor) const {
    Eigen::Vector2f position_sample = pos + sampler->next2d();

    float wavelength_sample = sampler->next1d();

    auto [ray, ray_weight] =
        sensor->sample_ray_differential(wavelength_sample, position_sample,
                                        sampler->next2d());
    ray.scale_differential(diff_scale_factor);
    Spectrum result =
        sample(scene, sampler, ray, sensor->medium(), aovs + 5) * ray_weight;
    Eigen::Vector3f xyz = spectrum_to_xyz(result, ray.wavelengths);

    aovs[0] = xyz.x();
    aovs[1] = xyz.y();
    aovs[2] = xyz.z();
    aovs[3] = 1.f;
    aovs[4] = 1.f;

    block->put(position_sample, aovs);
}

MonteCarloIntegrator::MonteCarloIntegrator(const Properties &props)
    : SamplingIntegrator(props) {
    m_rr_depth = props.int_("rr_depth", 5);
    if (m_rr_depth <= 0)
        Throw("\"rr_depth\" must be set to a value greater than zero!");

    m_max_depth = props.int_("max_depth", -1);
    if (m_max_depth < 0 && m_max_depth != -1)
        Throw("\"max_depth\" must be set to -1 (infinite) or a value >= 0");
}

MonteCarloIntegrator::~MonteCarloIntegrator() {
}

MSK_IMPLEMENT_CLASS(Integrator, Object, "integrator")
MSK_IMPLEMENT_CLASS(SamplingIntegrator, Integrator)
MSK_IMPLEMENT_CLASS(MonteCarloIntegrator, SamplingIntegrator)

} // namespace misaki
