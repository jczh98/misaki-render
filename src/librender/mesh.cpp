#include <misaki/render/logger.h>
#include <misaki/render/mesh.h>
#include <misaki/render/properties.h>

namespace misaki::render {
Mesh::Mesh(const Properties &props) : Shape(props) {
  m_to_world = props.transform("to_world", Transform4());
  m_mesh = true;
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
  return 0;
}

Mesh::InterpolatedPoint Mesh::compute_surface_point(int prim_index, const Vector2 &uv) const {
  Float b1 = uv.x(), b2 = uv.y(), b0 = 1.f - b1 - b2;
  auto fi = face_indices(prim_index);
  Vector3 p0 = vertex_position(fi[0]),
          p1 = vertex_position(fi[1]),
          p2 = vertex_position(fi[2]);
  InterpolatedPoint ip;
  ip.p = p0 * b0 + p1 * b1 + p2 * p2;
  auto n = math::normalize(math::cross(p1 - p0, p2 - p0));
  ip.ng = n;
  ip.ns = n;
  ip.uv = uv;
  if (has_vertex_texcoords()) {
    Vector2 uv0 = vertex_texcoord(fi[0]),
            uv1 = vertex_texcoord(fi[1]),
            uv2 = vertex_texcoord(fi[2]);
    auto intepolated_uv = uv0 * b0 + uv1 * b1 + uv2 * b2;
    ip.uv = intepolated_uv;
  }
  if (has_vertex_normals()) {
    Vector3 n0 = vertex_normal(fi[0]),
            n1 = vertex_normal(fi[1]),
            n2 = vertex_normal(fi[2]);
    auto ns = math::normalize(n0 * b0 + n1 * b1 + n2 * b2);
    ip.ns = ns;
  }
  return ip;
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