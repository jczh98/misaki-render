#pragma once

#include "component.h"
#include "fwd.h"

namespace aspirin {

class APR_EXPORT PluginManager {
public:
    static PluginManager *instance() {
        static PluginManager instance;
        return &instance;
    }
    PluginManager();
    ~PluginManager();
    std::shared_ptr<Component> create_comp(const Properties &);
    template <typename T,
              std::enable_if_t<!std::is_same_v<T, Component>, int> = 0>
    std::shared_ptr<T> create_comp(const Properties &props) {
        return std::dynamic_pointer_cast<T>(create_comp(props));
    }

private:
    struct PluginManagerPrivate;
    std::unique_ptr<PluginManagerPrivate> d;
};

} // namespace aspirin