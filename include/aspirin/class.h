#pragma once

#include "aspirin.h"
#include <functional>
#include <string>

namespace aspirin {

class APR_EXPORT Class {
public:
    using ConstructFunctor = std::function<Object *(const Properties &)>;

    Class(const std::string &name, const std::string &parent,
          const std::string &variant = "", ConstructFunctor construct = {},
          const std::string &alias = "");

    const std::string &name() const { return m_name; }

    const std::string &variant() const { return m_variant; }

    const std::string &alias() const { return m_alias; }

    bool is_constructible() const { return (bool) m_construct; }

    const Class *parent() const { return m_parent; }

    bool derives_from(const Class *clazz) const;

    static const Class *for_name(const std::string &name,
                                 const std::string &variant = "");

    ref<Object> construct(const Properties &props) const;

    static bool rtti_is_initialized() { return m_is_initialized; }

    static void static_initialization();

    static void static_shutdown();

private:
    static void initialize_once(Class *clazz);

private:
    std::string m_name, m_parent_name, m_variant, m_alias;
    Class *m_parent;
    ConstructFunctor m_construct;
    static bool m_is_initialized;
};

#define APR_CLASS(x) x::m_class

extern APR_EXPORT const Class *m_class;

#define APR_DECLARE_CLASS()                                                    \
    virtual const Class *clazz() const override;                               \
                                                                               \
public:                                                                        \
    static Class *m_class;

#define APR_IMPLEMENT_CLASS(Name, Parent, ...)                                 \
    Class *Name::m_class = new Class(                                          \
        #Name, #Parent, "", ::aspirin::detail::get_construct_functor<Name>(),  \
        ##__VA_ARGS__);                                                        \
    const Class *Name::clazz() const { return m_class; }

#define APR_IMPLEMENT_CLASS_VARIANT(Name, Parent, ...)                         \
    template <typename Float, typename Spectrum>                               \
    Class *Name<Float, Spectrum>::m_class = new Class(                         \
        #Name, #Parent, ::aspirin::detail::get_variant<Float, Spectrum>(),     \
        ::aspirin::detail::get_construct_functor<Name<Float, Spectrum>>(),     \
        ##__VA_ARGS__);                                                        \
    template <typename Float, typename Spectrum>                               \
    const Class *Name<Float, Spectrum>::clazz() const {                        \
        return m_class;                                                        \
    }

#define APR_INTERNAL_PLUGIN(Class, PluginName)                                 \
    APR_EXPORT struct Plugin_##Class {                                         \
        Plugin_##Class() {                                                     \
            detail::register_internal_plugin(#Class, PluginName);              \
        }                                                                      \
    } plugin_##Class;                                                          \
    APR_INSTANTIATE_CLASS(Class)

namespace detail {
template <typename, typename Arg, typename = void>
struct is_constructiblee : std::false_type {};

template <typename T, typename Arg>
struct is_constructiblee<T, Arg,
                         std::void_t<decltype(new T(std::declval<Arg>()))>>
    : std::true_type {};

template <typename T, typename Arg>
constexpr bool is_constructible_v = is_constructiblee<T, Arg>::value;

template <typename T,
          std::enable_if_t<is_constructible_v<T, const Properties &>, int> = 0>
Class::ConstructFunctor get_construct_functor() {
    return [](const Properties &p) -> Object * { return new T(p); };
}
template <typename T,
          std::enable_if_t<!is_constructible_v<T, const Properties &>, int> = 0>
Class::ConstructFunctor get_construct_functor() {
    return {};
}
} // namespace detail

} // namespace aspirin
