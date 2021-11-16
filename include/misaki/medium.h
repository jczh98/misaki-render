#pragma once

#include "fwd.h"
#include "object.h"
#include "ray.h"

namespace misaki {

class MSK_EXPORT Medium : public Object {

public:
    /// Sample a free-flight distance in the medium.
    virtual std::pair<MediumInteraction, float>
    sample_interaction(const Ray &ray, float sample,
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