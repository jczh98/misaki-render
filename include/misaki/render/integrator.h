#pragma once

#include "misaki/core/fwd.h"
#include "misaki/core/object.h"

namespace misaki {

class MSK_EXPORT Integrator : public Object {
public:
    virtual bool render(Scene *scene, Sensor *sensor);

    MSK_DECLARE_CLASS()
protected:
    Integrator(const Properties &props);
    virtual ~Integrator();

protected:
    uint32_t m_block_size;
};

class MSK_EXPORT MonteCarloIntegrator : public Integrator {
public:
    bool render(Scene *scene, Sensor *sensor) override;

    void render_block(const Scene *scene, const Sensor *sensor,
                      Sampler *sampler, ImageBlock *block,
                      size_t sample_count) const;

    void render_sample(const Scene *scene, const Sensor *sensor,
                       Sampler *sampler, ImageBlock *block,
                       const Eigen::Vector2f &pos,
                       float diff_scale_factor) const;

    virtual Spectrum sample(const Scene *scene, Sampler *sampler,
                            const RayDifferential &ray_, const Medium* medium = nullptr) const = 0;

    MSK_DECLARE_CLASS()
protected:
    MonteCarloIntegrator(const Properties &props);
    virtual ~MonteCarloIntegrator();

protected:
    std::mutex m_mutex;
};

} // namespace misaki