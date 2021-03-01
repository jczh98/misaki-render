#include <aspirin/logger.h>
#include <aspirin/plugin.h>
#include <aspirin/properties.h>

#include <mutex>
#include <unordered_map>

namespace aspirin {

ref<Object> PluginManager::create_object(const Properties &props,
                                         const Class *class_) {
    assert(class_ != nullptr);
    if (class_->name() == "Scene")
        return class_->construct(props);
    const Class *plugin_class =
        Class::for_name(props.plugin_name(), class_->variant());
    assert(plugin_class != nullptr);
    auto object = plugin_class->construct(props);
    if (!object->clazz()->derives_from(class_)) {
        const Class *oc = object->clazz();
        if (oc->parent())
            oc = oc->parent();
        Throw("Type mismatch when loading plugin \"{}\": Expected "
              "an instance of type \"{}\" (variant \"{}\"), got an instance of "
              "type \"{}\" (variant \"{}\")",
              props.plugin_name(), class_->name(), class_->variant(),
              oc->name(), oc->variant());
    }
    return object;
}

} // namespace aspirin