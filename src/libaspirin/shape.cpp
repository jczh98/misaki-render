#include <aspirin/bsdf.h>
#include <aspirin/emitter.h>
#include <aspirin/interaction.h>
#include <aspirin/logger.h>
#include <aspirin/properties.h>
#include <aspirin/shape.h>

namespace aspirin {

template <typename Spectrum>
Shape<Spectrum>::Shape(const Properties &props) : m_id(props.id()) {
    m_world_transform = props.transform("to_world", Transform4());
    for (auto &kv : props.components()) {
        auto bsdf  = std::dynamic_pointer_cast<BSDF>(kv.second);
        auto light = std::dynamic_pointer_cast<Light>(kv.second);
        if (light) {
            if (m_light)
                Throw("Only one light can be specified by a shape.");
            m_light = light;
        } else if (bsdf) {
            if (m_bsdf)
                Throw("Only one bsdf can be specified by a shape.");
            m_bsdf = bsdf;
        } else {
            Throw("Tired to add unsuppored object of type \"{}\"",
                  kv.second->to_string());
        }
    }
    if (!m_bsdf)
        m_bsdf =
            PluginManager::instance()->create_comp<BSDF>(Properties("diffuse"));
}

template <typename Spectrum>
std::pair<PointGeometry, Float>
Shape<Spectrum>::sample_position(const Vector2 &sample) const {
    ARP_NOT_IMPLEMENTED("sample_position");
}

template <typename Spectrum>
Float Shape<Spectrum>::pdf_position(const PointGeometry &geom) const {
    ARP_NOT_IMPLEMENTED("pdf_position");
}

template <typename Spectrum> BoundingBox3 Shape<Spectrum>::bbox() const {
    ARP_NOT_IMPLEMENTED("bbox");
}

template <typename Spectrum>
BoundingBox3 Shape<Spectrum>::bbox(uint32_t index) const {
    ARP_NOT_IMPLEMENTED("bbox");
}

template <typename Spectrum> Float Shape<Spectrum>::surface_area() const {
    ARP_NOT_IMPLEMENTED("surface_area");
}

template <typename Spectrum>
std::tuple<Vector3, Vector3, Vector3, Vector2>
Shape<Spectrum>::compute_surface_point(int prim_index,
                                       const Vector2 &uv) const {
    ARP_NOT_IMPLEMENTED("compute_surface_point");
}

#if defined(MSK_ENABLE_EMBREE)
RTCGeometry Shape::embree_geometry(RTCDevice device) const {
    ARP_NOT_IMPLEMENTED("embree_geometry");
}
#endif

} // namespace aspirin