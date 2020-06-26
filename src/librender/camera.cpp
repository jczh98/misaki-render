#include <misaki/render/camera.h>
#include <misaki/render/logger.h>
#include <misaki/render/properties.h>

namespace misaki::render {

Camera::Camera(const Properties &props) : Endpoint(props) {
  for (auto &kv : props.components()) {
    auto film = std::dynamic_pointer_cast<Film>(kv.second);
    auto sampler = std::dynamic_pointer_cast<Sampler>(kv.second);
    if (film) {
      if (m_film) Throw("Camera can only have one film.");
      m_film = film;
    } else if (sampler) {
      if (m_sampler) Throw("Can only have one samplelr.");
      m_sampler = sampler;
    }
  }
  auto pmgr = PluginManager::instance();
  if (!m_film) {
    m_film = pmgr->create_comp<Film>(Properties("rgbfilm"));
  }
  if (!m_sampler) {
    m_sampler = pmgr->create_comp<Sampler>(Properties("independent"));
  }
  m_aspect = m_film->size().x() / (Float)m_film->size().y();
  m_resolution = Vector2(m_film->size());
}

std::pair<Ray, Vector3> Camera::sample_ray(const Vector2 &pos_sample) const {
  MSK_NOT_IMPLEMENTED("sample_ray");
}

ProjectiveCamera::ProjectiveCamera(const Properties &props) : Camera(props) {
  m_near_clip = props.get_float("near_clip", 1e-2f);
  m_far_clip = props.get_float("far_clip", 1e4f);
  m_focus_distance = props.get_float("focus_distance", m_far_clip);
}

MSK_REGISTER_CLASS(Camera)
MSK_REGISTER_CLASS(ProjectiveCamera)

}  // namespace misaki::render