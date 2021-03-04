#include <aspirin/logger.h>
#include <aspirin/plugin.h>
#include <aspirin/properties.h>

#include <iostream>
#include <mutex>
#include <unordered_map>

namespace aspirin {

namespace detail {

static std::unordered_map<std::string, std::string> *_plugins = nullptr;

inline std::string class_key(const std::string &name,
                             const std::string &variant) {
    return name + "." + variant;
}

void register_internal_plugin(const std::string &name,
                              const std::string &plugin_name) {
    std::cout << "fuck" << std::endl;
    if (!_plugins) {
        _plugins = new std::unordered_map<std::string, std::string>();
    }
    (*_plugins)[plugin_name] = name;
}

void clear_internal_plugins() {
    delete _plugins;
    _plugins = nullptr;
}

} // namespace detail

ref<Object> PluginManager::create_object(const Properties &props,
                                         const Class *class_) {
    assert(class_ != nullptr);
    if (class_->name() == "Scene")
        return class_->construct(props);
    auto it = detail::_plugins->find(props.plugin_name());
    const Class *plugin_class =
        (it != detail::_plugins->end())
            ? Class::for_name(it->second, class_->variant())
            : nullptr;
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