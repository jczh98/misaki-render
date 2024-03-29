#pragma once

#include "fwd.h"
#include <functional>
#include <string>

namespace misaki {

class MSK_EXPORT Class {
public:
    using ConstructFunctor = std::function<Object *(const Properties &)>;

    Class(const std::string &name, const std::string &parent,
          ConstructFunctor construct = {}, const std::string &alias = "");

    const std::string &name() const { return m_name; }

    const std::string &alias() const { return m_alias; }

    bool is_constructible() const { return (bool) m_construct; }

    const Class *parent() const { return m_parent; }

    bool derives_from(const Class *clazz) const;

    static const Class *for_name(const std::string &name);

    ref<Object> construct(const Properties &props) const;

    static bool rtti_is_initialized() { return m_is_initialized; }

    static void static_initialization();

    static void static_shutdown();

private:
    static void initialize_once(Class *clazz);

private:
    std::string m_name, m_parent_name, m_alias;
    Class *m_parent;
    ConstructFunctor m_construct;
    inline static bool m_is_initialized = false;
};

#define MSK_CLASS(x) x::m_class

extern MSK_EXPORT const Class *m_class;

#define MSK_DECLARE_CLASS()                                                    \
    virtual const Class *clazz() const override;                               \
                                                                               \
public:                                                                        \
    static Class *m_class;

#define MSK_IMPLEMENT_CLASS(Name, Parent, ...)                                 \
    Class *Name::m_class = new Class(                                          \
        #Name, #Parent, ::misaki::detail::get_construct_functor<Name>(),      \
        ##__VA_ARGS__);                                                        \
    const Class *Name::clazz() const { return m_class; }

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

} // namespace misaki
