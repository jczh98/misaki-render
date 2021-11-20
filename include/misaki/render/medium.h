#pragma once

#include "misaki/core/fwd.h"
#include "misaki/core/object.h"
#include "misaki/core/ray.h"

namespace misaki {

struct MediumSample {
    float t;
    Eigen::Vector3f p;
    Spectrum sigma_s;
    Spectrum sigma_a;
    Spectrum transmittance;
    float pdf;
};

class MSK_EXPORT Medium : public Object {

public:
    virtual bool sample_distance(MediumSample &ms, const Ray &ray, float sample,
                                 uint32_t channel) const = 0;

    /// Compute the transmittance and PDF
    virtual Spectrum eval_transmittance(const Ray &ray) const = 0;

    const PhaseFunction *phase_function() const {
        return m_phase_function.get();
    }

    bool is_homogeneous() const { return m_is_homogeneous; }
    std::string id() const override { return m_id; }
    std::string to_string() const override = 0;

    MSK_DECLARE_CLASS()
protected:
    Medium();
    Medium(const Properties &props);
    virtual ~Medium();

protected:
    ref<PhaseFunction> m_phase_function;
    bool m_is_homogeneous;

    std::string m_id;
};

} // namespace misaki