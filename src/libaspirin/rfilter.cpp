#include <aspirin/logger.h>
#include <aspirin/properties.h>
#include <aspirin/rfilter.h>

namespace aspirin {

template <typename Spectrum>
ReconstructionFilter<Spectrum>::ReconstructionFilter(const Properties &props) {}

template <typename Spectrum>
Float ReconstructionFilter<Spectrum>::eval(Float x) const {
  ARP_NOT_IMPLEMENTED("eval");
}


}  // namespace aspirin