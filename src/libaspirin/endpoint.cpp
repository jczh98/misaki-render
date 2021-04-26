#include <aspirin/endpoint.h>
#include <aspirin/logger.h>
#include <aspirin/medium.h>
#include <aspirin/properties.h>
#include <aspirin/records.h>

namespace aspirin {

Endpoint::Endpoint(const Properties &props) {
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

Endpoint::~Endpoint() {}

std::pair<Ray, Spectrum> Endpoint::sample_ray(const Vector2 &,
                                              const Vector2 &) const {
    APR_NOT_IMPLEMENTED("sample_ray");
}

std::pair<PositionSample, Spectrum>
Endpoint::sample_position(const Vector2 &sample) const {
    APR_NOT_IMPLEMENTED("sample_position");
}

Float Endpoint::pdf_position(const PositionSample &ps) const {
    APR_NOT_IMPLEMENTED("pdf_position");
}

std::pair<DirectionSample, Spectrum>
Endpoint::sample_direction(const Interaction &ref,
                           const Vector2 &sample) const {
    APR_NOT_IMPLEMENTED("sample_direction");
}

Float Endpoint::pdf_direction(const Interaction &ref,
                              const DirectionSample &ds) const {
    APR_NOT_IMPLEMENTED("pdf_direction");
}

Spectrum Endpoint::eval(const SurfaceInteraction &si) const {
    APR_NOT_IMPLEMENTED("eval");
}

void Endpoint::set_shape(Shape *shape) {
    if (m_shape)
        Throw("An endpoint can be only be attached to a single shape.");

    m_shape = shape;
}

void Endpoint::set_medium(Medium *medium) {
    if (m_medium)
        Throw("An endpoint can be only be attached to a single shape.");

    m_medium = medium;
}

void Endpoint::set_scene(const Scene *scene) {}

APR_IMPLEMENT_CLASS(Endpoint, Object)

} // namespace aspirin