#include <aspirin/component.h>
#include <aspirin/properties.h>

namespace aspirin {
std::string Component::to_string() const { return "Component"; }

std::shared_ptr<Component>
ComponentManager::create_instance(const Properties &props) {
    auto it = m_constructors.find(props.plugin_name());
    if (it == m_constructors.end()) {
        throw std::runtime_error(
            fmt::format("Could not find instance {}.", props.plugin_name()));
    }
    return it->second(props);
}

} // namespace aspirin