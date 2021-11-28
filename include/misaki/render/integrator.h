#pragma once

#include "misaki/core/fwd.h"
#include "misaki/core/object.h"
#include "misaki/core/utils.h"

namespace misaki {

class MSK_EXPORT Integrator : public Object {
public:
    virtual bool render(Scene *scene, Sensor *sensor) = 0;

    MSK_DECLARE_CLASS()
protected:
    Integrator(const Properties &props) {}
    virtual ~Integrator() {}
};

class MSK_EXPORT SamplingIntegrator : public Integrator {
public:
    virtual std::vector<std::string> aov_names() const;

    virtual Spectrum sample(const Scene *scene, Sampler *sampler,
                            const RayDifferential &ray_,
                            const Medium *medium = nullptr,
                            float *aovs          = nullptr) const = 0;

    bool render(Scene *scene, Sensor *sensor) override;

    MSK_DECLARE_CLASS()
protected:
    SamplingIntegrator(const Properties &props);

    virtual ~SamplingIntegrator();

    void render_block(const Scene *scene, const Sensor *sensor,
                      Sampler *sampler, ImageBlock *block, float *aovs,
                      size_t sample_count) const;

    void render_sample(const Scene *scene, const Sensor *sensor,
                       Sampler *sampler, ImageBlock *block, float *aovs,
                       const Eigen::Vector2f &pos,
                       float diff_scale_factor) const;

protected:
    uint32_t m_block_size;
    Timer m_render_timer;
    bool m_hide_emitters;
};

class MSK_EXPORT MonteCarloIntegrator : public SamplingIntegrator {
public:
    MSK_DECLARE_CLASS()
protected:
    MonteCarloIntegrator(const Properties &props);
    virtual ~MonteCarloIntegrator();

protected:
    int m_max_depth;
    int m_rr_depth;
};

} // namespace misaki