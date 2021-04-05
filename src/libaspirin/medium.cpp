#include <aspirin/medium.h>
#include <aspirin/phase.h>
#include <aspirin/properties.h>
#include <aspirin/scene.h>
#include <aspirin/volume.h>

namespace aspirin {

template <typename Float, typename Spectrum>
Medium<Float, Spectrum>::Medium() : m_is_homogeneous(false) {}

template <typename Float, typename Spectrum>
Medium<Float, Spectrum>::Medium(const Properties &props) : m_id(props.id()) {

    for (auto &[name, obj] : props.objects()) {
        auto *phase = dynamic_cast<PhaseFunction *>(obj.get());
        if (phase) {
            if (m_phase_function)
                Throw(
                    "Only a single phase function can be specified per medium");
            m_phase_function = phase;
        }
    }
    if (!m_phase_function) {
        // Create a default isotropic phase function
        m_phase_function =
            PluginManager::instance()->create_object<PhaseFunction>(
                Properties("isotropic"));
    }
}

template <typename Float, typename Spectrum>
Medium<Float, Spectrum>::~Medium() {}

APR_IMPLEMENT_CLASS_VARIANT(Medium, Object, "medium")
APR_INSTANTIATE_CLASS(Medium)

} // namespace aspirin