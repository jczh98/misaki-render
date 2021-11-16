#pragma once

#include "fwd.h"
#include "object.h"

namespace misaki {

class APR_EXPORT Sampler : public Object {
public:
    virtual ref<Sampler> clone() = 0;
    virtual void seed(uint64_t seed_value);
    virtual Float next1d();
    virtual Vector2 next2d();
    size_t sample_count() const { return m_sample_count; }

    APR_DECLARE_CLASS()
protected:
    Sampler(const Properties &props);
    virtual ~Sampler();

protected:
    size_t m_sample_count;
    uint64_t m_base_seed;
};

} // namespace misaki