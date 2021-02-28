#pragma once

#include "shape.h"

namespace aspirin {

template <typename Spectrum> class APR_EXPORT Mesh : public Shape<Spectrum> {
public:
    using PointGeometry = PointGeometry<Spectrum>;
    Mesh(const Properties &props);

    uint32_t vertex_count() const { return m_vertex_count; }
    uint32_t face_count() const { return m_face_count; }

    Float *vertices() { return m_vertices.get(); }
    const Float *vertices() const { return m_vertices.get(); }

    uint32_t *faces() { return m_faces.get(); }
    const uint32_t *faces() const { return m_faces.get(); }

    APR_INLINE Float *vertex(const uint32_t &index) {
        return m_vertices.get() + m_vertex_size * index;
    }
    APR_INLINE const Float *vertex(const uint32_t &index) const {
        return m_vertices.get() + m_vertex_size * index;
    }

    APR_INLINE uint32_t *face(const uint32_t &index) {
        return m_faces.get() + m_face_size * index;
    }
    APR_INLINE const uint32_t *face(const uint32_t &index) const {
        return m_faces.get() + m_face_size * index;
    }

    APR_INLINE Vector3 face_indices(uint32_t index) const {
        return Vector3(*(face(index)), *(face(index) + 1), *(face(index) + 2));
    }

    APR_INLINE Vector3 vertex_position(uint32_t index) const {
        return Vector3(*(vertex(index)), *(vertex(index) + 1),
                       *(vertex(index) + 2));
    }

    APR_INLINE Vector3 vertex_normal(uint32_t index) const {
        auto normal = vertex(index) + m_normal_offset;
        return Vector3(*normal, *(normal + 1), *(normal + 2));
    }

    APR_INLINE Vector2 vertex_texcoord(uint32_t index) const {
        auto texcoord = vertex(index) + m_texcoord_offset;
        return Vector2(*texcoord, *(texcoord + 1));
    }

    auto face_area(uint32_t index) const {
        auto fi = face_indices(index);

        auto p0 = vertex_position(fi[0]), p1 = vertex_position(fi[1]),
             p2 = vertex_position(fi[2]);
        return 0.5f * (p1 - p0).cross(p2 - p0).norm();
    }

    bool has_vertex_normals() const { return m_normal_offset != 0; }
    bool has_vertex_texcoords() const { return m_texcoord_offset != 0; }

    std::pair<PointGeometry, Float>
    sample_position(const Vector2 &sample) const override;
    Float pdf_position(const PointGeometry &geom) const override;

    virtual std::tuple<Vector3, Vector3, Vector3, Vector2>
    compute_surface_point(int prim_index, const Vector2 &uv) const override;

    void area_distr_build();

    BoundingBox3 bbox() const override;
    BoundingBox3 bbox(uint32_t index) const override;
    Float surface_area() const override;

#if defined(MSK_ENABLE_EMBREE)
    virtual RTCGeometry embree_geometry(RTCDevice device) const override;
#endif

protected:
    std::unique_ptr<Float[]> m_vertices;
    std::unique_ptr<uint32_t[]> m_faces;
    uint32_t m_vertex_size = 0, m_face_size = 0;
    uint32_t m_normal_offset = 0, m_texcoord_offset = 0;
    uint32_t m_vertex_count = 0, m_face_count = 0;

    Distribution1D m_area_distr;
    Float m_surface_area;
    std::string m_name;
    BoundingBox3 m_bbox;
    Transform4 m_to_world;
};

} // namespace aspirin