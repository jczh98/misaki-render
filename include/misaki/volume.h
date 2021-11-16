#pragma once

#include "fwd.h"
#include "object.h"

namespace misaki {

class APR_EXPORT Volume : public Object {
public:
    virtual Spectrum eval(const Interaction &si) const;

    APR_DECLARE_CLASS()
protected:
    Volume(const Properties &props);
    virtual ~Volume() {}

protected:
    Transform4 m_world_to_local;
    BoundingBox3 m_bbox;
};

} // namespace misaki