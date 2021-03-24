#include <aspirin/endpoint.h>
#include <aspirin/logger.h>
#include <aspirin/medium.h>
#include <aspirin/properties.h>
#include <aspirin/records.h>

namespace aspirin {

template <typename Float, typename Spectrum>
Endpoint<Float, Spectrum>::Endpoint(const Properties &props) {
    m_world_transform = props.transform("to_world", Transform4());
    for (auto &[name, obj] : props.objects()) {
        auto *medium = dynamic_cast<Medium *>(obj.get());
        if (medium) {
            if (m_medium) {
                Throw("Only a single medium can be specified per endpoint");
            }
            m_medium = medium;
        }
    }
}

template <typename Float, typename Spectrum>
Endpoint<Float, Spectrum>::~Endpoint() {}

template <typename Float, typename Spectrum>
std::pair<typename Endpoint<Float, Spectrum>::Ray, Spectrum>
Endpoint<Float, Spectrum>::sample_ray(const Vector2 &pos_sample) const {
    APR_NOT_IMPLEMENTED("sample_ray");
}

template <typename Float, typename Spectrum>
std::pair<typename Endpoint<Float, Spectrum>::PositionSample, Spectrum>
Endpoint<Float, Spectrum>::sample_position(const Vector2 &sample) const {
    APR_NOT_IMPLEMENTED("sample_position");
}

template <typename Float, typename Spectrum>
Float Endpoint<Float, Spectrum>::pdf_position(const PositionSample &ps) const {
    APR_NOT_IMPLEMENTED("pdf_position");
}

template <typename Float, typename Spectrum>
std::pair<typename Endpoint<Float, Spectrum>::DirectSample, Spectrum>
Endpoint<Float, Spectrum>::sample_direct(const Interaction &ref,
                                         const Vector2 &sample) const {
    APR_NOT_IMPLEMENTED("sample_direct");
}

template <typename Float, typename Spectrum>
Float Endpoint<Float, Spectrum>::pdf_direct(const Interaction &ref,
                                            const DirectSample &ds) const {
    APR_NOT_IMPLEMENTED("pdf_direct");
}

template <typename Float, typename Spectrum>
Spectrum Endpoint<Float, Spectrum>::eval(const SurfaceInteraction &si) const {
    APR_NOT_IMPLEMENTED("eval");
}

template <typename Float, typename Spectrum>
void Endpoint<Float, Spectrum>::set_shape(Shape *shape) {
    if (m_shape)
        Throw("An endpoint can be only be attached to a single shape.");

    m_shape = shape;
}

template <typename Float, typename Spectrum>
void Endpoint<Float, Spectrum>::set_medium(Medium *medium) {
    if (m_medium)
        Throw("An endpoint can be only be attached to a single shape.");

    m_medium = medium;
}

template <typename Float, typename Spectrum>
void Endpoint<Float, Spectrum>::set_scene(const Scene *scene) {}

APR_IMPLEMENT_CLASS_VARIANT(Endpoint, Object)
APR_INSTANTIATE_CLASS(Endpoint)

} // namespace aspirin