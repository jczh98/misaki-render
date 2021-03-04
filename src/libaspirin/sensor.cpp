#include <aspirin/logger.h>
#include <aspirin/plugin.h>
#include <aspirin/properties.h>
#include <aspirin/sensor.h>

namespace aspirin {

template <typename Float, typename Spectrum>
Sensor<Float, Spectrum>::Sensor(const Properties &props)
    : Endpoint<Float, Spectrum>(props) {
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
        m_film = static_cast<Film *>(pmgr->create_object<Film>(Properties("rgbfilm")));
    }
    if (!m_sampler) {
        m_sampler = static_cast<Sampler *>(pmgr->create_object<Sampler>(Properties("independent")));
    }
    m_aspect     = m_film->size().x() / (Float) m_film->size().y();
    m_resolution = Vector2(m_film->size().x(), m_film->size().y());
}

template <typename Float, typename Spectrum>
ProjectiveCamera<Float, Spectrum>::ProjectiveCamera(const Properties &props)
    : Sensor<Float, Spectrum>(props) {
    m_near_clip      = props.get_float("near_clip", 1e-2f);
    m_far_clip       = props.get_float("far_clip", 1e4f);
    m_focus_distance = props.get_float("focus_distance", m_far_clip);
}

APR_IMPLEMENT_CLASS_VARIANT(Sensor, Endpoint, "sensor")
APR_IMPLEMENT_CLASS_VARIANT(ProjectiveCamera, Sensor)

APR_INSTANTIATE_CLASS(Sensor)
APR_INSTANTIATE_CLASS(ProjectiveCamera)

} // namespace aspirin