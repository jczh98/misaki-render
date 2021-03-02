#include <aspirin/properties.h>
#include <aspirin/sensor.h>

namespace aspirin {

class PerspectiveCamera final : public ProjectiveCamera {
 public:
  PerspectiveCamera(const Properties &props) : ProjectiveCamera(props) {
    m_fov = props.get_float("fov", 30);
    m_camera_to_sample =
        Transform4::scale({m_film->size().x(), m_film->size().y(), 1}) * Transform4::scale({-0.5f, -0.5f * m_aspect, 1.f}) * Transform4::translate({-1.f, -1.f / m_aspect, 0.f}) * Transform4::perspective(m_fov, m_near_clip, m_far_clip);
    m_sample_to_camera = m_camera_to_sample.inverse();
  }

  std::pair<Ray, Vector3> sample_ray(const Vector2 &pos_sample) const {
    Ray ray;
    auto near_p = m_sample_to_camera.apply_point({pos_sample.x(), pos_sample.y(), 0.f});
    auto d = math::normalize(near_p);
    Float inv_z = 1.f / d.z();
    ray.mint = m_near_clip * inv_z;
    ray.maxt = m_far_clip * inv_z;
    ray.o = m_world_transform.apply_point({0.f, 0.f, 0.f});
    ray.d = m_world_transform.apply_vector(d);
    ray.update();
    return {ray, 1.0f};
  }

  MSK_DECL_COMP(ProjectiveCamera)

 private:
  Transform4 m_camera_to_sample,
      m_sample_to_camera;
  Float m_fov;
};

MSK_EXPORT_PLUGIN(PerspectiveCamera)

}  // namespace aspirin