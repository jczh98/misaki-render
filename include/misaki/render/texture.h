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

    MSK_DECLARE_CLASS()
protected:
    Texture(const Properties &props);
    virtual ~Texture();

protected:
    std::string m_id;
};

class MSK_EXPORT ConstantSpectrumTexture : public Texture {
public:
    ConstantSpectrumTexture(const Color3 &value);

    float eval_1(const SceneInteraction &si) const override;

    Spectrum eval(const SceneInteraction &si) const override;

    Color3 eval_3(const SceneInteraction &si) const override;

    float mean() const override;

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "ConstantSpectrumTexture[" << std::endl
            << "  value = " << m_value << std::endl
            << "]";
        return oss.str();
    }

    MSK_DECLARE_CLASS()
private:
    Color3 m_value;
};

} // namespace misaki