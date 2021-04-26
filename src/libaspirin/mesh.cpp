#include <aspirin/emitter.h>
#include <aspirin/interaction.h>
#include <aspirin/logger.h>
#include <aspirin/mesh.h>
#include <aspirin/properties.h>
#include <aspirin/warp.h>
#include <iostream>

namespace aspirin {

Mesh::Mesh(const Properties &props) : Shape(props) {
    m_to_world = props.transform("to_world", Transform4());
    m_is_mesh  = true;
    set_children();
    recompute_bbox();
}

Mesh::~Mesh() {}

BoundingBox3 Mesh::bbox() const { return m_bbox; }

void Mesh::recompute_bbox() {
    m_bbox.reset();
    for (uint32_t i = 0; i < m_vertex_count; ++i)
        m_bbox.expand(vertex_position(i));
}

BoundingBox3 Mesh::bbox(uint32_t index) const {
    assert(index <= m_face_count);
    auto idx   = (const uint32_t *) face(index);
    Vector3 v0 = vertex_position(idx[0]), v1 = vertex_position(idx[1]),
            v2 = vertex_position(idx[2]);
    return BoundingBox3(v0.cwiseMin(v1.cwiseMin(v2)),
                        v0.cwiseMax(v1.cwiseMax(v2)));
}

Float Mesh::surface_area() const { return m_surface_area; }

void Mesh::area_distr_build() {
    // Build surface area distribution
    std::vector<Float> table;
    for (uint32_t i = 0; i < m_face_count; ++i) {
        const auto tri_area = face_area(i);
        m_surface_area += tri_area;
        table.emplace_back(tri_area);
    }
    m_area_distr.init(table.data(), static_cast<int>(table.size()));
}

SurfaceInteraction
Mesh::compute_surface_interaction(const Ray &ray,
                                  PreliminaryIntersection pi) const {
    Float b1 = pi.prim_uv.x(), b2 = pi.prim_uv.y(), b0 = 1.f - b1 - b2;
    auto fi    = face_indices(pi.prim_index);
    Vector3 p0 = vertex_position(fi[0]), p1 = vertex_position(fi[1]),
            p2  = vertex_position(fi[2]);
    Vector3 dp0 = p1 - p0, dp1 = p2 - p0;

    SurfaceInteraction si;
    if (!pi.is_valid()) {
        si.t = math::Infinity<Float>;
        return si;
    }
    si.t                         = pi.t;
    si.p                         = p0 * b0 + p1 * b1 + p2 * b2;
    si.n                         = dp0.cross(dp1).normalized();
    si.uv                        = pi.prim_uv;
    std::tie(si.dp_du, si.dp_dv) = coordinate_system(si.n);
    if (has_vertex_texcoords()) {
        Vector2 uv0 = vertex_texcoord(fi[0]), uv1 = vertex_texcoord(fi[1]),
                uv2 = vertex_texcoord(fi[2]);
        si.uv       = uv0 * b0 + uv1 * b1 + uv2 * b2;
        // Compute position partials wrt. the UV parameterization
        Vector2 duv0 = uv1 - uv0, duv1 = uv2 - uv0;
        Float det     = duv0.x() * duv1.y() - duv0.y() * duv1.x(),
              inv_det = 1.f / det;
        if (det != 0.f) {
            si.dp_du = (duv1.y() * dp0 - duv0.y() * dp1) * inv_det;
            si.dp_dv = (-duv1.x() * dp0 + duv0.x() * dp1) * inv_det;
        }
    }
    if (has_vertex_normals()) {
        Vector3 n0 = vertex_normal(fi[0]), n1 = vertex_normal(fi[1]),
                n2    = vertex_normal(fi[2]);
        si.sh_frame.n = (n0 * b0 + n1 * b1 + n2 * b2).normalized();
        si.dn_du = si.dn_dv = Vector3::Zero();
        // Compute the normal partials wrt. [u, v] in local tangent space
        Vector3 N = b0 * n1 + b1 * n2 + b2 * n0;
        Float il  = Float(1) / std::sqrt(N.squaredNorm());
        N *= il;

        si.dn_du = (n1 - n0) * il;
        si.dn_dv = (n2 - n0) * il;

        si.dn_du = -N * N.dot(si.dn_du) + si.dn_du;
        si.dn_dv = -N * N.dot(si.dn_dv) + si.dn_dv;
    } else {
        si.sh_frame.n = si.n;
    }
    return si;
}

PositionSample Mesh::sample_position(const Vector2 &sample_) const {
    Vector2 sample = sample_;
    uint32_t face_idx;
    std::tie(face_idx, sample.y()) = m_area_distr.sample_reuse(sample.y());
    Vector3 fi                     = face_indices(face_idx);
    Vector3 p0 = vertex_position(fi[0]), p1 = vertex_position(fi[1]),
            p2 = vertex_position(fi[2]);
    Vector3 e0 = p1 - p0, e1 = p2 - p0;
    Vector2 b = warp::square_to_uniform_triangle(sample);

    PositionSample ps;
    ps.p       = p0 + e0 * b.x() + e1 * b.y();
    Vector2 uv = b;
    if (has_vertex_texcoords()) {
        auto uv0 = vertex_texcoord(fi[0]), uv1 = vertex_texcoord(fi[1]),
             uv2 = vertex_texcoord(fi[2]);
        uv       = uv0 * (1.f - b.x() - b.y()) + uv1 * b.x() + uv2 * b.y();
    }
    Vector3 ng = e0.cross(e1).normalized(), ns = ng;
    if (has_vertex_normals()) {
        auto n0 = vertex_normal(fi[0]), n1 = vertex_normal(fi[1]),
             n2 = vertex_normal(fi[2]);
        ns =
            (n0 * (1.f - b.x() - b.y()) + n1 * b.x() + n2 * b.y()).normalized();
    }
    ps.n     = ns;
    ps.pdf   = 1.f / m_surface_area;
    ps.delta = false;
    ps.uv    = uv;
    return ps;
}

Float Mesh::pdf_position(const PositionSample &ps) const {
    return 1.f / m_surface_area;
}

#if defined(APR_ENABLE_EMBREE)

RTCGeometry Mesh::embree_geometry(RTCDevice device) const {
    RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
    rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0,
                               RTC_FORMAT_FLOAT3, m_vertices.get(), 0,
                               sizeof(Float) * m_vertex_size, m_vertex_count);
    rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
                               m_faces.get(), 0, sizeof(uint32_t) * m_face_size,
                               m_face_count);
    rtcCommitGeometry(geom);
    return geom;
}
#endif

APR_IMPLEMENT_CLASS(Mesh, Shape)

} // namespace aspirin