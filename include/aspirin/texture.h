#pragma once

#include "object.h"
#include "fwd.h"

namespace aspirin {

template <typename Float, typename Spectrum>
class APR_EXPORT Texture : public Object {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using SurfaceInteraction = SurfaceInteraction<Float, Spectrum>;

    virtual Float eval_1(const SurfaceInteraction &si) const;
    virtual Color3 eval_3(const SurfaceInteraction &si) const;
    virtual Float mean() const;

    APR_DECLARE_CLASS()
protected:
    Texture(const Properties &props);
    virtual ~Texture();

protected:
    std::string m_id;
};

APR_EXTERN_CLASS(Texture)

} // namespace aspirin