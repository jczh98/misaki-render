#include <misaki/logger.h>
#include <misaki/properties.h>
#include <misaki/rfilter.h>

namespace misaki {

ReconstructionFilter::ReconstructionFilter(const Properties &props) {}

ReconstructionFilter::~ReconstructionFilter() {}

Float ReconstructionFilter::eval(Float x) const { APR_NOT_IMPLEMENTED("eval"); }

APR_IMPLEMENT_CLASS(ReconstructionFilter, Object, "rfilter")

} // namespace misaki