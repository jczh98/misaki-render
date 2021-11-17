#include <misaki/render/endpoint.h>
#include <misaki/core/logger.h>
#include <misaki/render/medium.h>
#include <misaki/core/properties.h>
#include <misaki/render/records.h>

namespace misaki {

Endpoint::Endpoint(const Properties &props) {
    m_world_transform = props.transform("to_world", Transform4f());
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

std::pair<Ray, Spectrum> Endpoint::sample_ray(const Eigen::Vector2f &,
                                              const Eigen::Vector2f &) const {
    MSK_NOT_IMPLEMENTED("sample_ray");
}

std::pair<PositionSample, Spectrum>
Endpoint::sample_position(const Eigen::Vector2f &sample) const {
    MSK_NOT_IMPLEMENTED("sample_position");
}

float Endpoint::pdf_position(const PositionSample &ps) const {
    MSK_NOT_IMPLEMENTED("pdf_position");
}

std::pair<DirectionSample, Spectrum>
Endpoint::sample_direction(const Interaction &ref,
                           const Eigen::Vector2f &sample) const {
    MSK_NOT_IMPLEMENTED("sample_direction");
}

float Endpoint::pdf_direction(const Interaction &ref,
                              const DirectionSample &ds) const {
    MSK_NOT_IMPLEMENTED("pdf_direction");
}

Spectrum Endpoint::eval(const SurfaceInteraction &si) const {
    MSK_NOT_IMPLEMENTED("eval");
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

MSK_IMPLEMENT_CLASS(Endpoint, Object)

} // namespace misaki