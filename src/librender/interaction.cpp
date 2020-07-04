#include <misaki/render/bsdf.h>
#include <misaki/render/interaction.h>
#include <misaki/render/logger.h>
#include <misaki/render/scene.h>
#include <misaki/render/shape.h>

namespace misaki::render {

const BSDF *SceneInteraction::bsdf(
    const Ray &ray) const {
  return shape->bsdf();
}

const BSDF *SceneInteraction::bsdf() const {
  return shape->bsdf();
}

const Light *SceneInteraction::light(const std::shared_ptr<Scene> &scene) const {
  if (is_valid()) {
    return shape->light();
  } else
    return scene->environment();
}

}  // namespace misaki::render