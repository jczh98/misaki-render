#pragma once

#include "fwd.h"
#include "object.h"

namespace aspirin {

template <typename Float, typename Spectrum>
class APR_EXPORT Volume : public Object {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Texture     = Texture<Float, Spectrum>;
    using Interaction = Interaction<Float, Spectrum>;

    virtual Spectrum eval(const Interaction &si) const;

    APR_DECLARE_CLASS()
protected:
    Volume(const Properties &props);
    virtual ~Volume() {}

protected:
    Transform4 m_world_to_local;
    BoundingBox3 m_bbox;
};
APR_EXTERN_CLASS(Volume)

} // namespace aspirin