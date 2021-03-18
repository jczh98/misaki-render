#include <aspirin/bsdf.h>
#include <aspirin/emitter.h>
#include <aspirin/film.h>
#include <aspirin/integrator.h>
#include <aspirin/interaction.h>
#include <aspirin/logger.h>
#include <aspirin/medium.h>
#include <aspirin/mesh.h>
#include <aspirin/phase.h>
#include <aspirin/properties.h>
#include <aspirin/records.h>
#include <aspirin/scene.h>
#include <aspirin/sensor.h>
#include <aspirin/utils.h>
#include <fstream>
#include <tbb/parallel_for.h>

namespace aspirin {

template <typename Float, typename Spectrum>
class VolumetricPathIntegrator  final : public Integrator<Float, Spectrum> {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Base = Integrator<Float, Spectrum>;
    using typename Base::Scene;
    using typename Base::Sensor;
    using Emitter              = Emitter<Float, Spectrum>;
    using Ray                  = Ray<Float, Spectrum>;
    using RayDifferential      = RayDifferential<Float, Spectrum>;
    using ImageBlock           = ImageBlock<Float, Spectrum>;
    using Sampler              = Sampler<Float, Spectrum>;
    using Interaction          = Interaction<Float, Spectrum>;
    using DirectionSample      = DirectionSample<Float, Spectrum>;
    using SurfaceInteraction   = SurfaceInteraction<Float, Spectrum>;
    using Medium               = Medium<Float, Spectrum>;
    using MediumInteraction    = MediumInteraction<Float, Spectrum>;
    using PhaseFunctionContext = PhaseFunctionContext<Float, Spectrum>;
    using PhaseFunction        = PhaseFunction<Float, Spectrum>;
    using MediumPtr            = const Medium *;
    using EmitterPtr           = const Emitter *;

    VolumetricPathIntegrator (const Properties &props) : Base(props) {}

    bool render(Scene *scene, Sensor *sensor) {
        auto film      = sensor->film();
        auto film_size = film->size();
        auto total_spp = sensor->sampler()->sample_count();
        Log(Info, "Starting render job ({}x{}, {} sample)", film_size.x(),
            film_size.y(), total_spp);
        int m_block_size = APR_BLOCK_SIZE;
        BlockGenerator gen(film_size, m_block_size);
        size_t total_blocks = gen.block_count();
        ProgressBar pbar(total_blocks, 70);
        Timer timer;
        tbb::parallel_for(
            tbb::blocked_range<size_t>(0, total_blocks, 1),
            [&](const tbb::blocked_range<size_t> &range) {
                auto sampler          = sensor->sampler()->clone();
                ref<ImageBlock> block = new ImageBlock(
                    Vector2i::Constant(m_block_size), film->filter());
                for (auto i = range.begin(); i != range.end(); ++i) {
                    auto [offset, size] = gen.next_block();
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

    void render_block(const Scene *scene, const Sensor *sensor,
                      Sampler *sampler, ImageBlock *block,
                      size_t sample_count) const {
        block->clear();
        auto &size              = block->size();
        auto &offset            = block->offset();
        Float diff_scale_factor = Float(1) / std::sqrt(sample_count);
        for (int y = 0; y < size.y(); ++y) {
            for (int x = 0; x < size.x(); ++x) {
                Vector2 pos = Vector2(x, y);
                if (pos.x() >= size.x() || pos.y() >= size.y())
                    continue;
                pos = pos + offset.template cast<Float>();
                for (int s = 0; s < sample_count; ++s) {
                    render_sample(scene, sensor, sampler, block, pos,
                                  diff_scale_factor);
                }
            }
        }
    }

    void render_sample(const Scene *scene, const Sensor *sensor,
                       Sampler *sampler, ImageBlock *block, const Vector2 &pos,
                       Float diff_scale_factor) const {
        auto position_sample = pos + sampler->next2d();
        auto [ray, ray_weight] =
            sensor->sample_ray_differential(position_sample);
        ray.scale_differential(diff_scale_factor);
        const Medium *medium = sensor->medium();
        auto result          = sample(scene, sampler, ray, medium);
        if (result)
            block->put(position_sample, *result);
    }

    std::optional<Color3> sample(const Scene *scene, Sampler *sampler,
                                 const RayDifferential &ray_,
                                 const Medium *initial_medium) const {
        RayDifferential ray = ray_;
        Float eta           = 1.f;
        Spectrum throughput = Spectrum::Constant(1.f),
                 result     = Spectrum::Zero();
        MediumPtr medium    = initial_medium;
        MediumInteraction mi;
        uint32_t depth = 0;
        auto channel   = std::min((uint32_t) sampler->next1d() * 3, 2u);
        SurfaceInteraction si;
        bool needs_intersection = true;
        for (int bounce = 0;; ++bounce) {
            // Russian roulette
            if (depth >= m_rr_depth) {
                Float q = std::min(throughput.maxCoeff() * eta * eta, 0.95f);
                if (sampler->next1d() >= q)
                    break;
                throughput *= 1.f / q;
            }
            if ((uint32_t) depth >= (uint32_t) m_max_depth || !si.is_valid())
                break;
        }
        return Spectrum::Zero();
    }

    /// Samples an emitter in the scene and evaluates its attenuated
    /// contribution
    std::tuple<Spectrum, DirectionSample>
    sample_emitter(const Interaction &ref_interaction,
                   bool is_medium_interaction, const Scene *scene,
                   Sampler *sampler, MediumPtr medium, uint32_t channel) const {
        auto [ds, emitter_val] = scene->sample_emitter_direction(
            ref_interaction, sampler->next2d(), false);
        if (ds.pdf == 0.f)
            return { Spectrum::Zero(), ds };
        Spectrum transmittance(1.0f);
        Ray ray          = ref_interaction.spawn_ray(ds.d);
        ray.mint         = is_medium_interaction ? 0.f : ray.mint;
        Float total_dist = 0.f;
        SurfaceInteraction si;
        si.t = math::Infinity<Float>;
        while (true) {
            Float remaining_dist =
                ds.dist * (1.f - math::ShadowEpsilon<Float>) -total_dist;
            ray.maxt = remaining_dist;
            if (remaining_dist <= 0.f)
                break;

            bool escaped_medium = false;
            if (medium != nullptr) {
                auto mi =
                    medium->sample_interaction(ray, sampler->next1d(), channel);
                if (medium->is_homogeneous() && mi.is_valid())
                    ray.maxt = std::min(mi.t, remaining_dist);
                si = scene->ray_intersect(ray);
                if (si.t < mi.t)
                    mi.t = math::Infinity<Float>;
                if (mi.t > remaining_dist && mi.is_valid())
                    total_dist = ds.dist;
                if (mi.t > remaining_dist)
                    mi.t = math::Infinity<Float>;
                escaped_medium = !mi.is_valid();
                if (mi.is_valid()) {
                    total_dist += mi.t;
                    ray.o    = mi.p;
                    ray.mint = 0.f;
                    si.t -= mi.t;
                    transmittance *=
                        mi.sigma_n.array() / mi.combined_extinction.array();
                }
            } else {
                total_dist += si.t;
                if (si.is_valid()) {
                    auto bsdf         = si.bsdf(ray);
                    Spectrum bsdf_val = bsdf->eval_null_transmission(si);
                    transmittance *= bsdf_val;
                    ray      = si.spawn_ray(ray.d);
                    ray.maxt = remaining_dist;

                    if (si.is_medium_transition()) {
                        medium = si.target_medium(ray.d);
                    }
                }
            }
        }
        return { emitter_val * transmittance, ds };
    }

    Float mis_weight(Float pdf_a, Float pdf_b) const {
        pdf_a *= pdf_a;
        pdf_b *= pdf_b;
        return pdf_a > 0.f ? pdf_a / (pdf_a + pdf_b) : 0.f;
    }

    APR_DECLARE_CLASS()
private:
    int m_max_depth = -1, m_rr_depth = 5;
    std::mutex m_mutex;
};

APR_IMPLEMENT_CLASS_VARIANT(VolumetricPathIntegrator , Integrator)
APR_INTERNAL_PLUGIN(VolumetricPathIntegrator , "volpath")

} // namespace aspirin