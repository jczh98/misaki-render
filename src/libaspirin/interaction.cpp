#include <aspirin/bsdf.h>
#include <aspirin/interaction.h>
#include <aspirin/logger.h>
#include <aspirin/records.h>
#include <aspirin/scene.h>
#include <aspirin/shape.h>

namespace aspirin {

template <typename Float, typename Spectrum>
SurfaceInteraction<Float, Spectrum>::SurfaceInteraction(
    const PositionSample &ps)
    : Interaction<Float, Spectrum>(0.f, ps.p), uv(ps.uv), n(ps.n),
      sh_frame(Frame3(ps.n)) {}

} // namespace aspirin