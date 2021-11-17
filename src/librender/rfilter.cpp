#include <misaki/core/logger.h>
#include <misaki/core/properties.h>
#include <misaki/render/rfilter.h>

namespace misaki {

ReconstructionFilter::ReconstructionFilter(const Properties &props) {}

ReconstructionFilter::~ReconstructionFilter() {}

float ReconstructionFilter::eval(float x) const { MSK_NOT_IMPLEMENTED("eval"); }

MSK_IMPLEMENT_CLASS(ReconstructionFilter, Object, "rfilter")

} // namespace misaki