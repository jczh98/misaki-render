#include <misaki/render/emitter.h>
#include <misaki/render/medium.h>
#include <misaki/core/properties.h>

namespace misaki {

Emitter::Emitter(const Properties &props) {
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

Emitter::~Emitter() {}

std::pair<Ray, Spectrum> Emitter::sample_ray(const Eigen::Vector2f &,
                                              const Eigen::Vector2f &) const {
    MSK_NOT_IMPLEMENTED("sample_ray");
}

std::pair<PositionSample, Spectrum>
Emitter::sample_position(const Eigen::Vector2f &sample) const {
    MSK_NOT_IMPLEMENTED("sample_position");
}

float Emitter::pdf_position(const PositionSample &ps) const {
    MSK_NOT_IMPLEMENTED("pdf_position");
}

std::pair<DirectionSample, Spectrum>
Emitter::sample_direction(const PositionSample &ps,
                           const Eigen::Vector2f &sample) const {
    MSK_NOT_IMPLEMENTED("sample_direction");
}

float Emitter::pdf_direction(const PositionSample &ps,
                              const DirectionSample &ds) const {
    MSK_NOT_IMPLEMENTED("pdf_direction");
}

std::pair<DirectIllumSample, Spectrum>
Emitter::sample_direct(const SceneInteraction &ref,
                        const Eigen::Vector2f &sample) const {
    MSK_NOT_IMPLEMENTED("sample_direct");
}

float Emitter::pdf_direct(const DirectIllumSample &dis) const {
    MSK_NOT_IMPLEMENTED("pdf_direct");
}

Spectrum Emitter::eval(const SceneInteraction &si) const {
    MSK_NOT_IMPLEMENTED("eval");
}

void Emitter::set_shape(Shape *shape) {
    if (m_shape)
        Throw("An emitter can be only be attached to a single shape.");

    m_shape = shape;
}

void Emitter::set_medium(Medium *medium) {
    if (m_medium)
        Throw("An emitter can be only be attached to a single shape.");

    m_medium = medium;
}

void Emitter::set_scene(const Scene *scene) {}

MSK_IMPLEMENT_CLASS(Emitter, Object, "emitter")

} // namespace misaki