#pragma once

#include "component.h"
#include "fwd.h"

namespace aspirin {

template <typename Spectrum> class APR_EXPORT Texture : public Component {
public:
    using PointGeometry = PointGeometry<Spectrum>;

    Texture(const Properties &props);
    virtual Float eval_1(const PointGeometry &geom) const;
    virtual Color3 eval_3(const PointGeometry &geom) const;
    virtual Float mean() const;

protected:
    std::string m_id;
};

} // namespace aspirin