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

class MSK_EXPORT ConstantSpectrumTexture : public Texture {
public:
    ConstantSpectrumTexture(const Color3 &value) : Texture(Properties()), m_value(value) {}

    float eval_1(const SurfaceInteraction &si) const override {
        return (m_value.x() + m_value.y() + m_value.z()) / 3.f;
    }

    Color3 eval_3(const SurfaceInteraction &si) const override {
        return m_value;
    }

    float mean() const override {
        return (m_value.x() + m_value.y() + m_value.z()) / 3.f;
    }

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