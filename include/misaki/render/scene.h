#pragma once

#include <optional>

#include "emitter.h"
#include "misaki/core/fwd.h"
#include "misaki/core/object.h"
#include "misaki/core/ray.h"
#include "sensor.h"

namespace misaki {

class MSK_EXPORT Scene : public Object {
public:
    Scene(const Properties &props);

    bool ray_test(const Ray &ray) const;
    SurfaceInteraction ray_intersect(const Ray &ray) const;
    void accel_init(const Properties &props);
    void accel_release();

    std::pair<DirectionSample, Spectrum>
    sample_emitter_direction(const Interaction &ref,
                             const Eigen::Vector2f &sample,
                             bool test_visibility = true) const;
    float pdf_emitter_direction(const Interaction &ref,
                                const DirectionSample &ds) const;

    const Sensor *sensor() const { return m_sensor; }
    Sensor *sensor() { return m_sensor; }

    const Integrator *integrator() const { return m_integrator; }
    Integrator *integrator() { return m_integrator; }

    const Emitter *environment() const { return m_environment; }

    const std::vector<ref<Emitter>> &emitters() const { return m_emitters; }
    std::vector<ref<Emitter>> &emitters() { return m_emitters; }

    const std::vector<ref<Shape>> &shapes() const { return m_shapes; }
    std::vector<ref<Shape>> &shapes() { return m_shapes; }

    const BoundingBox3f &bbox() const { return m_bbox; }

    MSK_DECLARE_CLASS()
protected:
    ~Scene();

protected:
    void *m_accel = nullptr;
    ref<Integrator> m_integrator;
    ref<Sensor> m_sensor;
    std::vector<ref<Shape>> m_shapes;
    std::vector<ref<Emitter>> m_emitters;
    ref<Emitter> m_environment;
    BoundingBox3f m_bbox;
};

extern MSK_EXPORT void library_nop();

} // namespace misaki