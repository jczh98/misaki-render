#include <misaki/render/medium.h>
#include <misaki/render/phase.h>
#include <misaki/core/properties.h>
#include <misaki/core/manager.h>
#include <misaki/render/scene.h>
#include <misaki/render/volume.h>

namespace misaki {

Medium::Medium() : m_is_homogeneous(false) {}

Medium::Medium(const Properties &props) : m_id(props.id()) {

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
            InstanceManager::get()->create_instance<PhaseFunction>(
                Properties("isotropic"));
    }
}

Medium::~Medium() {}

MSK_IMPLEMENT_CLASS(Medium, Object, "medium")

} // namespace misaki