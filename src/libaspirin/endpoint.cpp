#include <aspirin/endpoint.h>
#include <aspirin/logger.h>
#include <aspirin/properties.h>
#include <aspirin/records.h>

namespace aspirin {
template <typename Spectrum>
Endpoint<Spectrum>::Endpoint(const Properties &props) : m_id(props.id()) {
  m_world_transform = props.transform("to_world", Transform4());
}

template <typename Spectrum>
std::pair<typename Endpoint<Spectrum>::Ray, Vector3> Endpoint<Spectrum>::sample_ray(const Vector2 &pos_sample) const {
  ARP_NOT_IMPLEMENTED("sample_ray");
}

template <typename Spectrum>
std::pair<Vector3, Vector3> Endpoint<Spectrum>::sample_direction(const Vector2 &sample, const PointGeometry &geom) const {
  ARP_NOT_IMPLEMENTED("sample_direction");
}

template <typename Spectrum>
Float Endpoint<Spectrum>::pdf_direction(const PointGeometry &geom, const Vector3 &wo) const {
  ARP_NOT_IMPLEMENTED("pdf_direction");
}

template <typename Spectrum>
std::pair<typename Endpoint<Spectrum>::PointGeometry, Vector3> Endpoint<Spectrum>::sample_position(const Vector2 &sample) const {
  ARP_NOT_IMPLEMENTED("sample_position");
}

template <typename Spectrum>
Float Endpoint<Spectrum>::pdf_position(const PointGeometry &geom) const {
  ARP_NOT_IMPLEMENTED("pdf_position");
}

template <typename Spectrum>
std::pair<typename Endpoint<Spectrum>::DirectSample, Color3>
Endpoint<Spectrum>::sample_direct(const PointGeometry &geom_ref, const Vector2 &position_sample) const {
  ARP_NOT_IMPLEMENTED("sample_direct");
}

template <typename Spectrum>
Float Endpoint<Spectrum>::pdf_direct(const PointGeometry &geom_ref, const DirectSample &ds) const {
  ARP_NOT_IMPLEMENTED("pdf_direct");
}

template <typename Spectrum>
Color3 Endpoint<Spectrum>::eval(const SceneInteraction &si) const {
  ARP_NOT_IMPLEMENTED("eval");
}

template <typename Spectrum>
void Endpoint<Spectrum>::set_shape(Shape *shape) {
  m_shape = shape;
}

template <typename Spectrum>
void Endpoint<Spectrum>::set_scene(const Scene *scene) {
}

APR_REGIST_NODE(Endpoint, "endpoint")

}  // namespace aspirin