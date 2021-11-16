#pragma once

#include "fwd.h"
#include "object.h"

namespace misaki {

class MSK_EXPORT Texture : public Object {
public:
    virtual float eval_1(const SurfaceInteraction &si) const;
    virtual Color3 eval_3(const SurfaceInteraction &si) const;
    virtual float mean() const;

    MSK_DECLARE_CLASS()
protected:
    Texture(const Properties &props);
    virtual ~Texture();

protected:
    std::string m_id;
};

} // namespace misaki