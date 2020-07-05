#include <misaki/render/interaction.h>
#include <misaki/render/light.h>
#include <misaki/render/logger.h>
#include <misaki/render/mesh.h>
#include <misaki/render/properties.h>
#include <misaki/render/warp.h>

namespace misaki::render {

Mesh::Mesh(const Properties &props) : Shape(props) {
  m_to_world = props.transform("to_world", Transform4());
  m_mesh = true;
  // Set children
  if (m_light) m_light->set_shape(this);
}

BoundingBox3 Mesh::bbox() const {
  return m_bbox;
}

BoundingBox3 Mesh::bbox(uint32_t index) const {
  assert(index <= m_face_count);
  auto idx = (const uint32_t *)face(index);
  Vector3 v0 = vertex_position(idx[0]),
          v1 = vertex_position(idx[1]),
          v2 = vertex_position(idx[2]);
  return BoundingBox3(math::cwise_min(v0, math::cwise_min(v1, v2)),
                      math::cwise_max(v0, math::cwise_max(v1, v2)));
}

Float Mesh::surface_area() const {
  return m_surface_area;
}

void Mesh::area_distr_build() {
  // Build surface area distribution
  std::vector<Float> table;
  for (uint32_t i = 0; i < m_face_count; ++i) {
    const auto tri_area = face_area(i);
    m_surface_area += tri_area;
    table.emplace_back(tri_area);
  }
  m_area_distr.init(table.data(), table.size());
}

std::tuple<Vector3, Vector3, Vector3, Vector2>
Mesh::compute_surface_point(int prim_index, const Vector2 &bary) const {
  Float b1 = bary.x(), b2 = bary.y(), b0 = 1.f - b1 - b2;
  auto fi = face_indices(prim_index);
  Vector3 p0 = vertex_position(fi[0]),
          p1 = vertex_position(fi[1]),
          p2 = vertex_position(fi[2]);

  Vector3 p = p0 * b0 + p1 * b1 + p2 * b2;
  auto n = math::normalize(math::cross(p1 - p0, p2 - p0));
  Vector3 ng = n;
  Vector3 ns = n;
  Vector2 uv = bary;
  if (has_vertex_texcoords()) {
    Vector2 uv0 = vertex_texcoord(fi[0]),
            uv1 = vertex_texcoord(fi[1]),
            uv2 = vertex_texcoord(fi[2]);
    auto intepolated_uv = uv0 * b0 + uv1 * b1 + uv2 * b2;
    uv = intepolated_uv;
  }
  if (has_vertex_normals()) {
    Vector3 n0 = vertex_normal(fi[0]),
            n1 = vertex_normal(fi[1]),
            n2 = vertex_normal(fi[2]);
    auto ns_ = math::normalize(n0 * b0 + n1 * b1 + n2 * b2);
    ns = ns_;
  }
  return {p, ng, ns, uv};
}

std::pair<PointGeometry, Float> Mesh::sample_position(const Vector2 &sample_) const {
  Vector2 sample = sample_;
  uint32_t face_idx;
  std::tie(face_idx, sample.y()) = m_area_distr.sample_reuse(sample.y());
  auto fi = face_indices(face_idx);
  Vector3 p0 = vertex_position(fi[0]),
          p1 = vertex_position(fi[1]),
          p2 = vertex_position(fi[2]);
  auto e0 = p1 - p0, e1 = p2 - p0;
  auto b = warp::square_to_uniform_triangle(sample);
  auto p = p0 + e0 * b.x() + e1 * b.y();
  auto uv = b;
  if (has_vertex_texcoords()) {
    auto uv0 = vertex_texcoord(fi[0]),
         uv1 = vertex_texcoord(fi[1]),
         uv2 = vertex_texcoord(fi[2]);
    uv = uv0 * (1.f - b.x() - b.y()) + uv1 * b.x() + uv2 * b.y();
  }
  auto ng = normalize(cross(e0, e1)), ns = ng;
  if (has_vertex_normals()) {
    auto n0 = vertex_normal(fi[0]),
         n1 = vertex_normal(fi[1]),
         n2 = vertex_normal(fi[2]);
    ns = normalize(n0 * (1.f - b.x() - b.y()) + n1 * b.x() + n2 * b.y());
  }
  return {PointGeometry::make_on_surface(p, ng, ns, uv), 1.f / m_surface_area};
}

Float Mesh::pdf_position(const PointGeometry &geom) const {
  return 1.f / m_surface_area;
}

#if defined(MSK_ENABLE_EMBREE)
RTCGeometry Mesh::embree_geometry(RTCDevice device) const {
  RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3,
                             m_vertices.get(), 0, sizeof(Float) * m_vertex_size, m_vertex_count);
  rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
                             m_faces.get(), 0, sizeof(uint32_t) * m_face_size, m_face_count);
  rtcCommitGeometry(geom);
  return geom;
}
#endif

MSK_REGISTER_CLASS(Mesh)

}  // namespace misaki::render