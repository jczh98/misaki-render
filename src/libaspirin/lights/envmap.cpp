#include <aspirin/imageio.h>
#include <aspirin/light.h>
#include <aspirin/properties.h>
#include <aspirin/records.h>
#include <aspirin/scene.h>
#include <aspirin/texture.h>

namespace aspirin {

class EnvironmentMap final : public Light {
 public:
  EnvironmentMap(const Properties &props) : Light(props) {
    m_bsphere = BoundingSphere3(Vector3(0.f), 1.f);
    auto filename = props.string("filename");
    try {
      m_bitmap = read_float_rgb_image(get_file_resolver()->resolve(filename).string());
    } catch (std::exception &e) {
      Throw("envmap: {}", e.what());
    }
    m_resolution = Vector2u(m_bitmap.shape());
    std::vector<Float> luminace(math::hprod(m_resolution));
    for (int i = 0; i < m_bitmap.data_size(); ++i) {
      luminace[i] = m_bitmap.raw_data()[i].luminance() * std::sin(math::Pi<Float> * (Float(i) / m_resolution.y() + 0.5f) / m_resolution.x());
    }
    m_dist.init(luminace.data(), m_resolution.x(), m_resolution.y());
    m_flags = +LightFlags::Infinite;
  }

  void set_scene(const Scene *scene) override {
    m_bsphere = scene->bbox().bounding_sphere();
    m_bsphere.radius = std::max(RayEpsilon<Float>,
                                m_bsphere.radius * (1.f + RayEpsilon<Float>));
  }

  std::pair<DirectSample, Color3>
  sample_direct(const PointGeometry &geom_ref, const Vector2 &sample) const override {
    auto [uv, pdf] = m_dist.sample(sample);
    Float theta = uv.y() * math::Pi<Float>,
          phi = uv.x() * (2.f * math::Pi<Float>);
    auto sin_theta = std::sin(theta), sin_phi = std::sin(phi);
    auto cos_theta = std::cos(theta), cos_phi = std::cos(phi);
    Vector3 d = {sin_phi * sin_theta, cos_theta, -cos_phi * sin_theta};
    Float dist = 2.f * m_bsphere.radius;
    Float inv_sin_theta =
        math::safe_rsqrt(std::max(math::sqr(d.x()) + math::sqr(d.z()), math::sqr(Epsilon<Float>)));
    d = m_world_transform.transform_affine_point(d);
    DirectSample ds;
    ds.geom = PointGeometry::make_infinite(geom_ref.p + d * dist, -d, uv);
    ds.pdf = pdf > 0.f ? pdf * inv_sin_theta * (1.f / (2.f * math::sqr(math::Pi<Float>))) : 0.f;
    ds.d = d;
    ds.dist = dist;
    return {ds, texture::linear_sample2d(uv, m_bitmap) / ds.pdf};
  }

  Float pdf_direct(const PointGeometry &geom_ref, const DirectSample &ds) const {
    auto d = m_world_transform.inverse().transform_affine_point(ds.d);
    Vector2 uv = {std::atan2(d.x(), -d.z()) * math::InvTwoPi<Float>,
                  math::safe_acos(d.y()) * math::InvPi<Float>};
    uv = uv - Vector2(math::floor2int(uv));
    Float inv_sin_theta =
        math::safe_rsqrt(std::max(math::sqr(d.x()) + math::sqr(d.z()), math::sqr(Epsilon<Float>)));
    return m_dist.pdf(uv) * inv_sin_theta * (1.f / (2.f * math::sqr(math::Pi<Float>)));
  }

  Color3 eval(const SceneInteraction &si) const override {
    auto v = m_world_transform.inverse().transform_affine_point(-si.wi);
    Vector2 uv = {std::atan2(v.x(), -v.z()) * math::InvTwoPi<Float>,
                  math::safe_acos(v.y()) * math::InvPi<Float>};
    uv = uv - Vector2(math::floor2int(uv));
    return texture::linear_sample2d(uv, m_bitmap);
  }

  MSK_DECL_COMP(Light)
 private:
  Distribution2D m_dist;
  BoundingSphere3 m_bsphere;
  math::Tensor<Color3, 2> m_bitmap;
  Vector2u m_resolution;
};

MSK_EXPORT_PLUGIN(EnvironmentMap)

}  // namespace aspirin