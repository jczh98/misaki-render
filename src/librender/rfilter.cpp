#include <misaki/render/logger.h>
#include <misaki/render/properties.h>
#include <misaki/render/rfilter.h>

namespace misaki::render {

ReconstructionFilter::ReconstructionFilter(const Properties &props) {}

Float ReconstructionFilter::eval(Float x) const {
  MSK_NOT_IMPLEMENTED("eval");
}

MSK_REGISTER_CLASS(ReconstructionFilter)

}  // namespace misaki::render