#include <misaki/core/logger.h>
#include <misaki/core/manager.h>
#include <misaki/core/properties.h>

#include <iostream>
#include <mutex>
#include <unordered_map>

namespace misaki {

static std::unordered_map<std::string, std::string> *__instances = nullptr;

ref<Object> InstanceManager::create_instance(const Properties &props,
                                             const Class *class_) {
    assert(class_ != nullptr);
    if (class_->name() == "Scene")
        return class_->construct(props);
    auto it = __instances->find(props.instance_name());
    const Class *instance_class =
        (it != __instances->end()) ? Class::for_name(it->second) : nullptr;
    if (instance_class == nullptr) {
        // Now we do not have a plugin instance
        throw std::runtime_error("Error while loading internal plugin");
    }
    assert(instance_class != nullptr);
    auto object = instance_class->construct(props);
    if (!object->clazz()->derives_from(class_)) {
        const Class *oc = object->clazz();
        if (oc->parent())
            oc = oc->parent();
        Throw("Type mismatch when loading plugin \"{}\": Expected "
              "an instance of type \"{}\", got an instance of "
              "type \"{}\"",
              props.instance_name(), class_->name(), oc->name());
    }
    return object;
}

void InstanceManager::register_instance(const std::string &name,
                                        const std::string &instance_name) {
    if (!m_is_initialized) {
        InstanceManager::static_initialization();
    }
    (*__instances)[instance_name] = name;
}

void InstanceManager::static_initialization() {
    if (!__instances) {
        __instances = new std::unordered_map<std::string, std::string>();
    }
    m_is_initialized = true;
}

void InstanceManager::static_shutdown() {
    delete __instances;
    __instances = nullptr;
}

} // namespace misaki