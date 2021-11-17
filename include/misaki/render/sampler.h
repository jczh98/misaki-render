#pragma once

#include "misaki/core/fwd.h"
#include "misaki/core/object.h"

namespace misaki {

class MSK_EXPORT Sampler : public Object {
public:
    virtual ref<Sampler> clone() = 0;
    virtual void seed(uint64_t seed_value);
    virtual float next1d();
    virtual Eigen::Vector2f next2d();
    size_t sample_count() const { return m_sample_count; }

    MSK_DECLARE_CLASS()
protected:
    Sampler(const Properties &props);
    virtual ~Sampler();

protected:
    size_t m_sample_count;
    uint64_t m_base_seed;
};

} // namespace misaki