#include <misaki/render/logger.h>
#include <misaki/render/properties.h>
#include <misaki/render/shape.h>

namespace misaki::render {

Shape::Shape(const Properties &props) : m_id(props.id()) {
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