#include <aspirin/logger.h>
#include <aspirin/properties.h>
#include <aspirin/rfilter.h>

namespace aspirin {

template <typename Float, typename Spectrum>
ReconstructionFilter<Float, Spectrum>::ReconstructionFilter(
    const Properties &props) {}

template <typename Float, typename Spectrum>
 ReconstructionFilter<Float, Spectrum>::~ReconstructionFilter() {}

template <typename Float, typename Spectrum>
Float ReconstructionFilter<Float, Spectrum>::eval(Float x) const {
    APR_NOT_IMPLEMENTED("eval");
}

APR_IMPLEMENT_CLASS_VARIANT(ReconstructionFilter, Object, "rfilter")
APR_INSTANTIATE_CLASS(ReconstructionFilter)

} // namespace aspirin