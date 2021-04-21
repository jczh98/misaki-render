#include <aspirin/properties.h>
#include <aspirin/sensor.h>

namespace aspirin {

template <typename Float, typename Spectrum>
class PerspectiveCamera final : public ProjectiveCamera<Float, Spectrum> {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Base = ProjectiveCamera<Float, Spectrum>;
    using Base::m_aspect;
    using Base::m_far_clip;
    using Base::m_film;
    using Base::m_near_clip;
    using Base::m_world_transform;
    using typename Base::Ray;

    PerspectiveCamera(const Properties &props) : Base(props) {
        m_fov = props.get_float("fov", 30);
        m_camera_to_sample =
            Transform4::scale(
                Vector3(m_film->size().x(), m_film->size().y(), 1.f)) *
            Transform4::scale(Vector3(-0.5f, -0.5f * m_aspect, 1.f)) *
            Transform4::translate(Vector3(-1.f, -1.f / m_aspect, 0.f)) *
            Transform4::perspective(m_fov, m_near_clip, m_far_clip);
        m_sample_to_camera = m_camera_to_sample.inverse();
    }

    std::pair<Ray, Spectrum>
    sample_ray(const Vector2 &pos_sample, const Vector2 &) const override {
        Ray ray;
        auto near_p = m_sample_to_camera.apply_point(
            { pos_sample.x(), pos_sample.y(), 0.f });
        auto d      = near_p.normalized();
        Float inv_z = 1.f / d.z();
        ray.mint    = m_near_clip * inv_z;
        ray.maxt    = m_far_clip * inv_z;
        ray.o       = m_world_transform.apply_point({ 0.f, 0.f, 0.f });
        ray.d       = m_world_transform.apply_vector(d);
        ray.update();
        return { ray, Spectrum::Constant(1.f) };
    }

    APR_DECLARE_CLASS()
private:
    Transform4 m_camera_to_sample, m_sample_to_camera;
    Float m_fov;
};

APR_IMPLEMENT_CLASS_VARIANT(PerspectiveCamera, ProjectiveCamera)
APR_INTERNAL_PLUGIN(PerspectiveCamera, "perspective")

} // namespace aspirin