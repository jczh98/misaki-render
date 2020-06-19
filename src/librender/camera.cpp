#include <misaki/render/camera.h>
#include <misaki/render/logger.h>
#include <misaki/render/properties.h>

namespace misaki::render {

Camera::Camera(const Properties &props) {
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

MSK_REGISTER_CLASS(Camera)

}  // namespace misaki::render