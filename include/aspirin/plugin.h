#pragma once

#include "object.h"

namespace aspirin {

class APR_EXPORT PluginManager {
public:
    static PluginManager *instance() {
        static PluginManager instance;
        return &instance;
    }

    ref<Object> create_object(const Properties &, const Class *);
    template <typename T> ref<T> create_object(const Properties &props) {
        return static_cast<T *>(create_object(props, APR_CLASS(T)).get());
    }

protected:
    PluginManager()  = default;
    ~PluginManager() = default;
};

} // namespace aspirin