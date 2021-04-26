#include <aspirin/bsdf.h>
#include <aspirin/emitter.h>
#include <aspirin/interaction.h>
#include <aspirin/logger.h>
#include <aspirin/medium.h>
#include <aspirin/plugin.h>
#include <aspirin/properties.h>
#include <aspirin/records.h>
#include <aspirin/shape.h>
#include <iostream>

namespace aspirin {

Shape::Shape(const Properties &props) : m_id(props.id()) {
    m_world_transform = props.transform("to_world", Transform4());
    for (auto &[name, obj] : props.objects()) {
        auto *emitter = dynamic_cast<Emitter *>(obj.get());
        auto *bsdf    = dynamic_cast<BSDF *>(obj.get());
        auto *medium  = dynamic_cast<Medium *>(obj.get());
        if (emitter) {
            if (m_emitter)
                Throw("Only one light can be specified by a shape.");
            m_emitter = emitter;
        } else if (bsdf) {
            if (m_bsdf)
                Throw("Only one bsdf can be specified by a shape.");
            m_bsdf = bsdf;
        } else if (medium) {
            if (name == "interior") {
                if (m_interior_medium)
                    Throw("Only a single interior medium can be specified per "
                          "shape.");
                m_interior_medium = medium;
            } else if (name == "exterior") {
                if (m_exterior_medium)
                    Throw("Only a single exterior medium can be specified per "
                          "shape.");
                m_exterior_medium = medium;
            }
        } else {
            Throw("Tired to add unsuppored object of type \"{}\"",
                  obj->to_string());
        }
    }
    if (!m_bsdf)
        m_bsdf = PluginManager::instance()->create_object<BSDF>(
            Properties("diffuse"));
}

void Shape::set_children() {
    if (m_emitter) {
        m_emitter->set_shape(this);
    }
}

PositionSample Shape::sample_position(const Vector2 &sample) const {
    APR_NOT_IMPLEMENTED("sample_position");
}

Float Shape::pdf_position(const PositionSample &ps) const {
    APR_NOT_IMPLEMENTED("pdf_position");
}

DirectionSample Shape::sample_direction(const Interaction &it,
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

Float Shape::pdf_direction(const Interaction &it,
                           const DirectionSample &ds) const {
    Float pdf = pdf_position(ds), dp = std::abs(ds.d.dot(ds.n));

    pdf *= (dp != 0.f) ? (ds.dist * ds.dist) / dp : 0.f;

    return pdf;
}

BoundingBox3 Shape::bbox() const { APR_NOT_IMPLEMENTED("bbox"); }

BoundingBox3 Shape::bbox(uint32_t index) const { APR_NOT_IMPLEMENTED("bbox"); }

Float Shape::surface_area() const { APR_NOT_IMPLEMENTED("surface_area"); }

SurfaceInteraction
Shape::compute_surface_interaction(const Ray &ray,
                                   PreliminaryIntersection pi) const {
    APR_NOT_IMPLEMENTED("compute_surface_point");
}

#if defined(APR_ENABLE_EMBREE)

RTCGeometry Shape::embree_geometry(RTCDevice device) const {
    APR_NOT_IMPLEMENTED("embree_geometry");
}
#endif

APR_IMPLEMENT_CLASS(Shape, Object, "shape")

} // namespace aspirin