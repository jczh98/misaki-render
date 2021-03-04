#include <aspirin/logger.h>
#include <aspirin/properties.h>
#include <aspirin/texture.h>

namespace aspirin {

template <typename Float, typename Spectrum>
Texture<Float, Spectrum>::Texture(const Properties &props) : m_id(props.id()) {}

template <typename Float, typename Spectrum>
Texture<Float, Spectrum>::~Texture() {}

template <typename Float, typename Spectrum>
Float Texture<Float, Spectrum>::eval_1(const SurfaceInteraction &si) const {
    APR_NOT_IMPLEMENTED("eval_1");
}

template <typename Float, typename Spectrum>
typename Texture<Float, Spectrum>::Color3
Texture<Float, Spectrum>::eval_3(const SurfaceInteraction &si) const {
    APR_NOT_IMPLEMENTED("eval_3");
}

template <typename Float, typename Spectrum>
Float Texture<Float, Spectrum>::mean() const {
    APR_NOT_IMPLEMENTED("mean");
}

APR_IMPLEMENT_CLASS_VARIANT(Texture, Object, "texture")
APR_INSTANTIATE_CLASS(Texture)

} // namespace aspirin