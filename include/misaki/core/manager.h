#pragma once

#include "object.h"

namespace misaki {

namespace detail {
extern void register_internal_plugin(const std::string &name,
                                     const std::string &plugin_name);
extern void clear_internal_plugins();
} // namespace detail

class MSK_EXPORT InstanceManager {
public:
    static InstanceManager *get() {
        static InstanceManager instance;
        return &instance;
    }

    ref<Object> create_instance(const Properties &, const Class *);
    template <typename T> ref<T> create_instance(const Properties &props) {
        return static_cast<T *>(create_instance(props, MSK_CLASS(T)).get());
    }

    void register_instance(const std::string &name,
                           const std::string &instance_name);

    static void static_initialization();

    static void static_shutdown();

protected:
    InstanceManager()  = default;
    ~InstanceManager() = default;

    inline static bool m_is_initialized = false;
};

#define MSK_REGISTER_INSTANCE(Class, InstanceName)                             \
    MSK_EXPORT struct Instance_##Class {                                       \
        Instance_##Class() {                                                   \
            InstanceManager::get()->register_instance(#Class, InstanceName);   \
        }                                                                      \
    } instance_##Class;

} // namespace misaki