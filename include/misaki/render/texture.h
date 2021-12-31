#pragma once

#include "misaki/core/fwd.h"
#include "misaki/core/object.h"

namespace misaki {

class MSK_EXPORT Texture : public Object {
public:
    virtual float eval_1(const SceneInteraction &si) const;
    virtual Spectrum eval(const SceneInteraction &si) const;
    virtual Color3 eval_3(const SceneInteraction &si) const;
    virtual float mean() const;

    static ref<Texture> D65(float scale = 1.f);

    MSK_DECLARE_CLASS()
protected:
    Texture(const Properties &props);
    virtual ~Texture();

protected:
    std::string m_id;
};

} // namespace misaki