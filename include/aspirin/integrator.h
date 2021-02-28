#pragma once

#include "component.h"

namespace aspirin {

template <typename Spectrum> class APR_EXPORT Integrator : public Component {
public:
    using Scene = Scene<Spectrum>;

    Integrator(const Properties &props);
    virtual bool render(const std::shared_ptr<Scene> &scene);

protected:
    uint32_t m_block_size;
};

} // namespace aspirin