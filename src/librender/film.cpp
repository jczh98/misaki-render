#include <misaki/render/film.h>
#include <misaki/render/imageblock.h>
#include <misaki/render/logger.h>
#include <misaki/render/plugin.h>
#include <misaki/render/properties.h>

namespace misaki::render {

Film::Film(const Properties &props) {
  m_size = {props.get_int("width", 640), props.get_int("height", 320)};
  for (auto &kv : props.components()) {
    auto rfilter = std::dynamic_pointer_cast<ReconstructionFilter>(kv.second);
    if (rfilter) {
      if (m_filter) Throw("A film can only have one filter");
      m_filter = rfilter;
    } else {
      Throw("Tried to add an unsupported component of type {}", kv.second->to_string());
    }
  }
  if (!m_filter) {
    m_filter = PluginManager::instance()->create_comp<ReconstructionFilter>(Properties("gaussian"));
  }
}

std::string Film::to_string() const {
  std::ostringstream oss;
  oss << "Film[" << std::endl
      << "  size = " << m_size << "," << std::endl
      << "  m_filter = " << m_filter->to_string() << std::endl
      << "]";
  return oss.str();
}

MSK_REGISTER_CLASS(Film)

}  // namespace misaki::render