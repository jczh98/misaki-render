#pragma once

#include "fwd.h"
#include "object.h"

namespace aspirin {

template <typename Float, typename Spectrum>
class APR_EXPORT Integrator : public Object {
public:
    enum RadianceQuery {
        /// Emitted radiance from a luminaire intersected by the ray
        EmittedRadiance          = 0x0001,

        /// Emitted radiance from a subsurface integrator */
        SubsurfaceRadiance       = 0x0002,

        /// Direct (surface) radiance */
        DirectSurfaceRadiance    = 0x0004,

        /*! \brief Indirect (surface) radiance, where the last bounce did not go
            through a Dirac delta BSDF */
        IndirectSurfaceRadiance  = 0x0008,

        /*! \brief Indirect (surface) radiance, where the last bounce went
           through a Dirac delta BSDF */
        CausticRadiance          = 0x0010,

        /// In-scattered radiance due to volumetric scattering (direct)
        DirectMediumRadiance     = 0x0020,

        /// In-scattered radiance due to volumetric scattering (indirect)
        IndirectMediumRadiance   = 0x0040,

        /// Distance to the next surface intersection
        Distance                 = 0x0080,

        /*! \brief Store an opacity value, which is equal to 1 when a shape
           was intersected and 0 when the ray passes through empty space.
           When there is a participating medium, it can also take on fractional
           values. */
        Opacity                  = 0x0100,

        /*! \brief A ray intersection may need to be performed. This can be set to
           zero if the caller has already provided the intersection */
        Intersection             = 0x0200,

        /* Radiance from volumes */
        VolumeRadiance           = DirectMediumRadiance | IndirectMediumRadiance,

        /// Radiance query without emitted radiance, ray intersection required
        RadianceNoEmission       = SubsurfaceRadiance | DirectSurfaceRadiance
                                    | IndirectSurfaceRadiance | CausticRadiance | DirectMediumRadiance
                                    | IndirectMediumRadiance | Intersection,

        /// Default radiance query, ray intersection required
        Radiance                 = RadianceNoEmission | EmittedRadiance,

        /// Radiance + opacity
        SensorRay                = Radiance | Opacity
    };

    using Scene  = Scene<Float, Spectrum>;
    using Sensor = Sensor<Float, Spectrum>;

    virtual bool render(Scene *scene, Sensor *sensor);

    APR_DECLARE_CLASS()
protected:
    Integrator(const Properties &props);
    virtual ~Integrator();
protected:
    uint32_t m_block_size;
};

APR_EXTERN_CLASS(Integrator)

} // namespace aspirin