#include <aspirin/bsdf.h>
#include <aspirin/emitter.h>
#include <aspirin/interaction.h>
#include <aspirin/logger.h>
#include <aspirin/plugin.h>
#include <aspirin/properties.h>
#include <aspirin/shape.h>
#include <aspirin/records.h>

namespace aspirin {

template <typename Float, typename Spectrum>
Shape<Float, Spectrum>::Shape(const Properties &props) : m_id(props.id()) {
    m_world_transform = props.transform("to_world", Transform4());
    for (auto &[name, obj] : props.objects()) {
        Emitter *emitter = dynamic_cast<Emitter *>(obj.get());
        BSDF *bsdf       = dynamic_cast<BSDF *>(obj.get());
        if (emitter) {
            if (m_emitter)
                Throw("Only one light can be specified by a shape.");
            m_emitter = emitter;
        } else if (bsdf) {
            if (m_bsdf)
                Throw("Only one bsdf can be specified by a shape.");
            m_bsdf = bsdf;
        } else {
            Throw("Tired to add unsuppored object of type \"{}\"",
                  obj->to_string());
        }
    }
    if (!m_bsdf)
        m_bsdf = PluginManager::instance()->create_object<BSDF>(
            Properties("diffuse"));
}
template <typename Float, typename Spectrum>
void Shape<Float, Spectrum>::set_children() {
    if (m_emitter) {
        m_emitter->set_shape(this);
    }
}
template <typename Float, typename Spectrum>
typename Shape<Float, Spectrum>::PositionSample
Shape<Float, Spectrum>::sample_position(const Vector2 &sample) const {
    APR_NOT_IMPLEMENTED("sample_position");
}

template <typename Float, typename Spectrum>
Float Shape<Float, Spectrum>::pdf_position(const PositionSample &ps) const {
    APR_NOT_IMPLEMENTED("pdf_position");
}

template <typename Float, typename Spectrum>
typename Shape<Float, Spectrum>::DirectionSample
Shape<Float, Spectrum>::sample_direction(const Interaction &it,
                                         const Vector2 &sample) const {
    DirectionSample ds(sample_position(sample));
    ds.d = ds.p - it.p;

    Float dist_squared = ds.d.squaredNorm();
    ds.dist            = std::sqrt(dist_squared);
    ds.d /= ds.dist;

    Float dp = std::abs(ds.d.dot(ds.n));
    ds.pdf *= (dp != 0.f) ? dist_squared / dp : 0.f;
    ds.object = (const Object *) this;

    return ds;
}

template <typename Float, typename Spectrum>
Float Shape<Float, Spectrum>::pdf_direction(const Interaction &it,
                                            const DirectionSample &ds) const {
    Float pdf = pdf_position(ds), dp = std::abs(ds.d.dot(ds.n));

    pdf *= (dp != 0.f) ? (ds.dist * ds.dist) / dp : 0.f;

    return pdf;
}

template <typename Float, typename Spectrum>
typename Shape<Float, Spectrum>::BoundingBox3
Shape<Float, Spectrum>::bbox() const {
    APR_NOT_IMPLEMENTED("bbox");
}

template <typename Float, typename Spectrum>
typename Shape<Float, Spectrum>::BoundingBox3
Shape<Float, Spectrum>::bbox(uint32_t index) const {
    APR_NOT_IMPLEMENTED("bbox");
}

template <typename Float, typename Spectrum>
Float Shape<Float, Spectrum>::surface_area() const {
    APR_NOT_IMPLEMENTED("surface_area");
}

template <typename Float, typename Spectrum>
std::tuple<typename Shape<Float, Spectrum>::Vector3,
           typename Shape<Float, Spectrum>::Vector3,
           typename Shape<Float, Spectrum>::Vector3,
           typename Shape<Float, Spectrum>::Vector2>
Shape<Float, Spectrum>::compute_surface_point(int prim_index,
                                              const Vector2 &uv) const {
    APR_NOT_IMPLEMENTED("compute_surface_point");
}

#if defined(APR_ENABLE_EMBREE)
template <typename Float, typename Spectrum>
RTCGeometry Shape<Float, Spectrum>::embree_geometry(RTCDevice device) const {
    APR_NOT_IMPLEMENTED("embree_geometry");
}
#endif

APR_IMPLEMENT_CLASS_VARIANT(Shape, Object, "shape")
APR_INSTANTIATE_CLASS(Shape)

} // namespace aspirin