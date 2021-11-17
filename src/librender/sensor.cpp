#include <misaki/core/logger.h>
#include <misaki/core/manager.h>
#include <misaki/core/properties.h>
#include <misaki/render/sensor.h>

namespace misaki {

Sensor::Sensor(const Properties &props) : Endpoint(props) {
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
    auto imgr = InstanceManager::get();
    if (!m_film) {
        m_film = static_cast<Film *>(
            imgr->create_instance<Film>(Properties("rgbfilm")));
    }
    if (!m_sampler) {
        m_sampler = static_cast<Sampler *>(
            imgr->create_instance<Sampler>(Properties("independent")));
    }
    m_aspect     = m_film->size().x() / (float) m_film->size().y();
    m_resolution = Eigen::Vector2f(m_film->size().x(), m_film->size().y());
}

Sensor::~Sensor() {}

std::pair<RayDifferential, Spectrum>
Sensor::sample_ray_differential(const Eigen::Vector2f &sample,
                                const Eigen::Vector2f &sample3) const {

    auto [temp_ray, result_spec] = sample_ray(sample, sample3);

    RayDifferential result_ray(temp_ray);

    Eigen::Vector2f dx(1.f / m_resolution.x(), 0.f);
    Eigen::Vector2f dy(0.f, 1.f / m_resolution.y());

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

ProjectiveCamera::ProjectiveCamera(const Properties &props) : Sensor(props) {
    m_near_clip      = props.float_("near_clip", 1e-2f);
    m_far_clip       = props.float_("far_clip", 1e4f);
    m_focus_distance = props.float_("focus_distance", m_far_clip);
}

ProjectiveCamera::~ProjectiveCamera() {}

MSK_IMPLEMENT_CLASS(Sensor, Endpoint, "sensor")
MSK_IMPLEMENT_CLASS(ProjectiveCamera, Sensor)

} // namespace misaki