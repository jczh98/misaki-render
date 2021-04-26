#include <aspirin/logger.h>
#include <aspirin/properties.h>
#include <aspirin/rfilter.h>

namespace aspirin {

ReconstructionFilter::ReconstructionFilter(const Properties &props) {}

ReconstructionFilter::~ReconstructionFilter() {}

Float ReconstructionFilter::eval(Float x) const { APR_NOT_IMPLEMENTED("eval"); }

APR_IMPLEMENT_CLASS(ReconstructionFilter, Object, "rfilter")

} // namespace aspirin