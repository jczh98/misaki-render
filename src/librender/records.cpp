#include <misaki/render/records.h>
#include <misaki/render/shape.h>
#include <misaki/render/emitter.h>

namespace misaki {

void DirectIllumSample::set_query(const Ray &ray, const SceneInteraction &si) {
    p       = si.p;
    n       = si.sh_frame.n;
    uv      = si.uv;
    object  = si.shape->emitter();
    d       = ray.d;
    dist    = si.t;
}

} // namespace misaki