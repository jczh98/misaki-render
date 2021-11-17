#pragma once

#include "misaki/core/fwd.h"
#include "misaki/core/object.h"

namespace misaki {

class MSK_EXPORT Volume : public Object {
public:
    virtual Spectrum eval(const Interaction &si) const;

    MSK_DECLARE_CLASS()
protected:
    Volume(const Properties &props);
    virtual ~Volume() {}

protected:
    Transform4f m_world_to_local;
    BoundingBox3f m_bbox;
};

} // namespace misaki