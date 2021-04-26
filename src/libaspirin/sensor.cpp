#include <aspirin/logger.h>
#include <aspirin/plugin.h>
#include <aspirin/properties.h>
#include <aspirin/sensor.h>

namespace aspirin {


Sensor::Sensor(const Properties &props)
    : Endpoint(props) {
    for (auto &[name, obj] : props.objects()) {
        auto *film    = dynamic_cast<Film *>(obj.get());
        auto *sampler = dynamic_cast<Sampler *>(obj.get());
        if (film) {
            if (m_film)
                Throw("Camera can only have one film.");
            m_film = film;
        } else if (sampler) {
            if (m_sampler)
                Throw("Can only have one samplelr.");
            m_sampler = sampler;
        }
    }
    auto pmgr = PluginManager::instance();
    if (!m_film) {
        m_film = static_cast<Film *>(
            pmgr->create_object<Film>(Properties("rgbfilm")));
    }
    if (!m_sampler) {
        m_sampler = static_cast<Sampler *>(
            pmgr->create_object<Sampler>(Properties("independent")));
    }
    m_aspect     = m_film->size().x() / (Float) m_film->size().y();
    m_resolution = Vector2(m_film->size().x(), m_film->size().y());
}


Sensor::~Sensor() {}


std::pair<RayDifferential, Spectrum>
Sensor::sample_ray_differential(const Vector2 &sample, const Vector2 &sample3) const {

    auto [temp_ray, result_spec] = sample_ray(sample, sample3);

    RayDifferential result_ray(temp_ray);

    Vector2 dx(1.f / m_resolution.x(), 0.f);
    Vector2 dy(0.f, 1.f / m_resolution.y());

    // Sample a result_ray for X+1
    std::tie(temp_ray, std::ignore) = sample_ray(sample + dx, sample3);

    result_ray.o_x = temp_ray.o;
    result_ray.d_x = temp_ray.d;

    // Sample a result_ray for Y+1
    std::tie(temp_ray, std::ignore) = sample_ray(sample + dy, sample3);

    result_ray.o_y               = temp_ray.o;
    result_ray.d_y               = temp_ray.d;
    result_ray.has_differentials = true;

    return { result_ray, result_spec };
}


ProjectiveCamera::ProjectiveCamera(const Properties &props)
    : Sensor(props) {
    m_near_clip      = props.get_float("near_clip", 1e-2f);
    m_far_clip       = props.get_float("far_clip", 1e4f);
    m_focus_distance = props.get_float("focus_distance", m_far_clip);
}


ProjectiveCamera::~ProjectiveCamera() {}

APR_IMPLEMENT_CLASS(Sensor, Endpoint, "sensor")
APR_IMPLEMENT_CLASS(ProjectiveCamera, Sensor)

} // namespace aspirin