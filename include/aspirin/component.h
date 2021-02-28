#pragma once

#include "platform.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

namespace aspirin {

class Properties;

class APR_EXPORT Component {
public:
    virtual std::string to_string() const;
};

class APR_EXPORT ComponentManager {
public:
    using CreateShared =
        std::function<std::shared_ptr<Component>(const Properties &)>;

    static ComponentManager *instance() {
        static ComponentManager instance;
        return &instance;
    }

    void register_comp(const std::string &name, const CreateShared &creator) {
        m_constructors[name] = creator;
    }

    std::shared_ptr<Component> create_instance(const Properties &props);
    template <typename T>
    std::shared_ptr<T> create_instance(const Properties &props) {
        return std::dynamic_pointer_cast<T>(create_instance(props));
    }

private:
    std::unordered_map<std::string, CreateShared> m_constructors;
};

#define APR_REGIST_NODE(CLASS, NAME)                                           \
    class Registor_##CLASS {                                                   \
    public:                                                                    \
        Registor_##CLASS() {                                                   \
            aspirin::ComponentManager::instance()->register_node(              \
                NAME, [&](const aspirin::Properties &props) {                  \
                    return std::make_shared<CLASS>(props);                     \
                });                                                            \
        }                                                                      \
    } registor_##CLASS;

} // namespace aspirin