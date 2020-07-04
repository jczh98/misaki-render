#include <misaki/render/endpoint.h>
#include <misaki/render/logger.h>
#include <misaki/render/properties.h>

namespace misaki::render {

Endpoint::Endpoint(const Properties &props) : m_id(props.id()) {
  m_world_transform = props.transform("to_world", Transform4());
}

std::pair<Vector3, Vector3> Endpoint::sample_direction(const Vector2 &sample, const PointGeometry &geom) const {
  MSK_NOT_IMPLEMENTED("sample_direction");
}

Float Endpoint::pdf_direction(const PointGeometry &geom, const Vector3 &wo) const {
  MSK_NOT_IMPLEMENTED("pdf_direction");
}

std::pair<PointGeometry, Vector3> Endpoint::sample_position(const Vector2 &sample) const {
  MSK_NOT_IMPLEMENTED("sample_position");
}

Float Endpoint::pdf_position(const PointGeometry &geom) const {
  MSK_NOT_IMPLEMENTED("pdf_position");
}

std::pair<DirectSample, Color3>
Endpoint::sample_direct(const PointGeometry &geom_ref, const Vector2 &position_sample) const {
  MSK_NOT_IMPLEMENTED("sample_direct");
}

Float Endpoint::pdf_direct(const PointGeometry &geom_ref, const DirectSample &ds) const {
  MSK_NOT_IMPLEMENTED("pdf_direct");
}

Color3 Endpoint::eval(const PointGeometry &geom, const Vector3 &wi) const {
  MSK_NOT_IMPLEMENTED("eval");
}

void Endpoint::set_shape(Shape *shape) {
  m_shape = shape;
}

void Endpoint::set_scene(const Scene *scene) {
}

MSK_REGISTER_CLASS(Endpoint)

}  // namespace misaki::render