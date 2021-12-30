#include <misaki/core/manager.h>
#include <misaki/core/properties.h>
#include <misaki/render/sensor.h>

namespace misaki {

class PerspectiveCamera final : public ProjectiveCamera {
public:
    PerspectiveCamera(const Properties &props)
        : ProjectiveCamera(props) {
        m_fov              = props.float_("fov", 30);
        m_camera_to_sample =
            Transform4f::scale(
                Eigen::Vector3f(m_film->size().x(), m_film->size().y(), 1.f)) *
            Transform4f::scale(Eigen::Vector3f(-0.5f, -0.5f * m_aspect, 1.f)) *
            Transform4f::translate(
                Eigen::Vector3f(-1.f, -1.f / m_aspect, 0.f)) *
            Transform4f::perspective(m_fov, m_near_clip, m_far_clip);
        m_sample_to_camera = m_camera_to_sample.inverse();
    }

    std::pair<Ray, Spectrum>
    sample_ray(const float wavelength_sample,
               const Eigen::Vector2f &pos_sample,
               const Eigen::Vector2f &) const override {
        auto [wavelengths, wav_weight] =
            sample_wavelength<float, Spectrum::SizeAtCompileTime>(
                wavelength_sample);
        Ray ray;
        const Eigen::Vector3f near_p = m_sample_to_camera.apply_point(
            { pos_sample.x(), pos_sample.y(), 0.f });
        const Eigen::Vector3f d = near_p.normalized();
        float inv_z = 1.f / d.z();
        ray.mint = m_near_clip * inv_z;
        ray.maxt = m_far_clip * inv_z;
        ray.wavelengths = wavelengths;
        ray.o = m_world_transform.apply_point({ 0.f, 0.f, 0.f });
        ray.d = m_world_transform.apply_vector(d);
        ray.update();
        return { ray, wav_weight };
    }

    MSK_DECLARE_CLASS()
private:
    Transform4f m_camera_to_sample, m_sample_to_camera;
    float m_fov;
};

MSK_IMPLEMENT_CLASS(PerspectiveCamera, ProjectiveCamera)
MSK_REGISTER_INSTANCE(PerspectiveCamera, "perspective")

} // namespace misaki
