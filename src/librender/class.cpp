#include <misaki/class.h>
#include <misaki/logger.h>
#include <misaki/object.h>
#include <iostream>
#include <map>

namespace misaki {

namespace xml::detail {
void register_class(const Class *class_);
void cleanup();
} // namespace xml::detail

static std::map<std::string, Class *> *__classes;
const Class *m_class = nullptr;

Class::Class(const std::string &name, const std::string &parent,
             ConstructFunctor constr, const std::string &alias)
    : m_name(name), m_parent_name(parent), m_alias(alias), m_parent(nullptr),
      m_construct(constr) {

    if (m_alias.empty())
        m_alias = name;

    if (!__classes)
        __classes = new std::map<std::string, Class *>();

    (*__classes)[name] = this;

    if (!alias.empty())
        xml::detail::register_class(this);
}

const Class *Class::for_name(const std::string &name) {
    auto it = __classes->find(name);
    if (it != __classes->end())
        return it->second;
    return nullptr;
}

bool Class::derives_from(const Class *arg) const {
    const Class *class_ = this;

    while (class_) {
        if (arg == class_)
            return true;
        class_ = class_->parent();
    }

    return false;
}

void Class::initialize_once(Class *class_) {
    std::string key = class_->m_parent_name;
    if (key.empty())
        return;

    auto it = __classes->find(key);
    if (it != __classes->end()) {
        class_->m_parent = it->second;
        return;
    }

    std::cerr << "Critical error during the static RTTI initialization: "
              << std::endl
              << "Could not locate the base class '" << key
              << "' while initializing '" << class_->name() << "'";

    std::cerr << "!" << std::endl;
}

ref<Object> Class::construct(const Properties &props) const {
    if (!m_construct)
        Throw("RTTI error: Attempted to construct a "
              "non-constructible class ({})!",
              name());
    return m_construct(props);
}

void Class::static_initialization() {
    for (auto &pair : *__classes)
        initialize_once(pair.second);
    m_is_initialized = true;
}

void Class::static_shutdown() {
    if (!m_is_initialized)
        return;
    for (auto &pair : *__classes) {
        delete pair.second;
    }
    delete __classes;
    __classes        = nullptr;
    m_is_initialized = false;
    xml::detail::cleanup();
}

} // namespace misaki