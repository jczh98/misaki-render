#include <aspirin/bsdf.h>
#include <aspirin/interaction.h>
#include <aspirin/logger.h>
#include <aspirin/scene.h>
#include <aspirin/shape.h>

namespace aspirin {

template <typename Spectrum>
const typename SceneInteraction<Spectrum>::BSDF *
SceneInteraction<Spectrum>::bsdf(const Ray &ray) const {
    return shape->bsdf();
}

template <typename Spectrum>
const typename SceneInteraction<Spectrum>::BSDF *
SceneInteraction<Spectrum>::bsdf() const {
    return shape->bsdf();
}

template <typename Spectrum>
const typename SceneInteraction<Spectrum>::Light *
SceneInteraction<Spectrum>::light(const std::shared_ptr<Scene> &scene) const {
    if (is_valid()) {
        return shape->light();
    } else
        return scene->environment();
}

} // namespace aspirin