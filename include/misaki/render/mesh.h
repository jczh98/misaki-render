#pragma once

#include "shape.h"

namespace misaki {

class MSK_EXPORT Mesh : public Shape {
public:
    uint32_t vertex_count() const { return m_vertex_count; }
    uint32_t face_count() const { return m_face_count; }

    float *vertices() { return m_vertices.get(); }
    const float *vertices() const { return m_vertices.get(); }

    uint32_t *faces() { return m_faces.get(); }
    const uint32_t *faces() const { return m_faces.get(); }

    MSK_INLINE float *vertex(const uint32_t &index) {
        return m_vertices.get() + m_vertex_size * index;
    }
    MSK_INLINE const float *vertex(const uint32_t &index) const {
        return m_vertices.get() + m_vertex_size * index;
    }

    MSK_INLINE uint32_t *face(const uint32_t &index) {
        return m_faces.get() + m_face_size * index;
    }
    MSK_INLINE const uint32_t *face(const uint32_t &index) const {
        return m_faces.get() + m_face_size * index;
    }

    MSK_INLINE Eigen::Vector3f face_indices(uint32_t index) const {
        return Eigen::Vector3f(*(face(index)), *(face(index) + 1), *(face(index) + 2));
    }

    MSK_INLINE Eigen::Vector3f vertex_position(uint32_t index) const {
        return Eigen::Vector3f(*(vertex(index)), *(vertex(index) + 1),
                       *(vertex(index) + 2));
    }

    MSK_INLINE Eigen::Vector3f vertex_normal(uint32_t index) const {
        auto normal = vertex(index) + m_normal_offset;
        return Eigen::Vector3f(*normal, *(normal + 1), *(normal + 2));
    }

    MSK_INLINE Eigen::Vector2f vertex_texcoord(uint32_t index) const {
        auto texcoord = vertex(index) + m_texcoord_offset;
        return Eigen::Vector2f(*texcoord, *(texcoord + 1));
    }

    auto face_area(uint32_t index) const {
        auto fi = face_indices(index);

        auto p0 = vertex_position(fi[0]), p1 = vertex_position(fi[1]),
             p2 = vertex_position(fi[2]);
        return 0.5f * (p1 - p0).cross(p2 - p0).norm();
    }

    bool has_vertex_normals() const { return m_normal_offset != 0; }
    bool has_vertex_texcoords() const { return m_texcoord_offset != 0; }

    virtual PositionSample
    sample_position(const Eigen::Vector2f &sample) const override;
    virtual float pdf_position(const PositionSample &ps) const override;

    virtual SceneInteraction
    compute_scene_interaction(const Ray &ray,
                                PreliminaryIntersection pi) const override;

    void area_distr_build();
    void recompute_bbox();

    BoundingBox3f bbox() const override;
    BoundingBox3f bbox(uint32_t index) const override;
    float surface_area() const override;

#if defined(MSK_ENABLE_EMBREE)
    virtual RTCGeometry embree_geometry(RTCDevice device) const override;
#endif

protected:
    Mesh(const Properties &props);
    virtual ~Mesh();
    MSK_DECLARE_CLASS()
protected:
    std::unique_ptr<float[]> m_vertices;
    std::unique_ptr<uint32_t[]> m_faces;
    uint32_t m_vertex_size = 0, m_face_size = 0;
    uint32_t m_normal_offset = 0, m_texcoord_offset = 0;
    uint32_t m_vertex_count = 0, m_face_count = 0;

    Distribution1D m_area_distr;
    float m_surface_area;
    std::string m_name;
    BoundingBox3f m_bbox;
    Transform4f m_to_world;
};

} // namespace misaki