#include <misaki/render/bsdf.h>
#include <misaki/render/interaction.h>
#include <misaki/render/light.h>
#include <misaki/render/logger.h>
#include <misaki/render/properties.h>
#include <misaki/render/shape.h>

namespace misaki::render {

Shape::Shape(const Properties &props) : m_id(props.id()) {
  m_world_transform = props.transform("to_world", Transform4());
  for (auto &kv : props.components()) {
    auto bsdf = std::dynamic_pointer_cast<BSDF>(kv.second);
    auto light = std::dynamic_pointer_cast<Light>(kv.second);
    if (light) {
      if (m_light) Throw("Only one light can be specified by a shape.");
      m_light = light;
    } else if (bsdf) {
      if (m_bsdf) Throw("Only one bsdf can be specified by a shape.");
      m_bsdf = bsdf;
    } else {
      Throw("Tired to add unsuppored object of type \"{}\"", kv.second->to_string());
    }
  }
  if (!m_bsdf) m_bsdf = PluginManager::instance()->create_comp<BSDF>(Properties("diffuse"));
}

std::pair<PointGeometry, Float> Shape::sample_position(const Vector2 &sample) const {
  MSK_NOT_IMPLEMENTED("sample_position");
}

Float Shape::pdf_position(const PointGeometry &geom) const {
  MSK_NOT_IMPLEMENTED("pdf_position");
}

BoundingBox3 Shape::bbox() const {
  MSK_NOT_IMPLEMENTED("bbox");
}

BoundingBox3 Shape::bbox(uint32_t index) const {
  MSK_NOT_IMPLEMENTED("bbox");
}

Float Shape::surface_area() const {
  MSK_NOT_IMPLEMENTED("surface_area");
}

Shape::InterpolatedPoint Shape::compute_surface_point(int prim_index, const Vector2 &uv) const {
  MSK_NOT_IMPLEMENTED("compute_surface_point");
}

#if defined(MSK_ENABLE_EMBREE)
RTCGeometry Shape::embree_geometry(RTCDevice device) const {
  MSK_NOT_IMPLEMENTED("embree_geometry");
}
#endif

MSK_REGISTER_CLASS(Shape)

}  // namespace misaki::render