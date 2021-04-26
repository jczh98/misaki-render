#include <aspirin/logger.h>
#include <aspirin/plugin.h>
#include <aspirin/properties.h>

#include <iostream>
#include <mutex>
#include <unordered_map>

namespace aspirin {

namespace detail {

static std::unordered_map<std::string, std::string> *_plugins = nullptr;

void register_internal_plugin(const std::string &name,
                              const std::string &plugin_name) {
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
        (it != detail::_plugins->end()) ? Class::for_name(it->second) : nullptr;
    if (plugin_class == nullptr) {
        // Now we do not have a plugin instance
        throw std::runtime_error("Error while loading internal plugin");
    }
    assert(plugin_class != nullptr);
    auto object = plugin_class->construct(props);
    if (!object->clazz()->derives_from(class_)) {
        const Class *oc = object->clazz();
        if (oc->parent())
            oc = oc->parent();
        Throw("Type mismatch when loading plugin \"{}\": Expected "
              "an instance of type \"{}\", got an instance of "
              "type \"{}\"",
              props.plugin_name(), class_->name(), oc->name());
    }
    return object;
}

} // namespace aspirin