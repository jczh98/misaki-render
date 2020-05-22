#include <misaki/render/camera.h>
#include <misaki/render/properties.h>

namespace misaki::render {

class PerspectiveCamera final : public Camera {
 public:
  PerspectiveCamera(const Properties &props) : Camera(props) {

  }
  MSK_DECL_COMP(Camera)
};

MSK_EXPORT_PLUGIN(PerspectiveCamera)

}