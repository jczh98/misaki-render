#pragma once

#include "component.h"
#include "fwd.h"
#include "plugin.h"

namespace misaki::render {

class NamedReference {
 public:
  NamedReference(const std::string &value) : m_value(value) {}
  operator const std::string &() const { return m_value; }
  bool operator==(const NamedReference &r) const { return r.m_value == m_value; }
  bool operator!=(const NamedReference &r) const { return r.m_value != m_value; }

 private:
  std::string m_value;
};

class MSK_EXPORT Properties {
 public:
  enum class Type {
    Bool,
    Int,
    Float,
    Vector3,
    Transform,
    Color,
    String,
    NamedReference,
    Component,
    Pointer
  };
  Properties();
  Properties(const std::string &plugin_name);
  Properties(const Properties &props);
  void operator=(const Properties &props);
  ~Properties();

  const std::string &plugin_name() const;
  void set_plugin_name(const std::string &name);
  bool has_property(const std::string &name) const;
  Type type(const std::string &name) const;
  const std::string &id() const;
  void set_id(const std::string &id);
  std::vector<std::string> property_names() const;
  std::vector<std::pair<std::string, NamedReference>> named_references() const;
  std::vector<std::pair<std::string, std::shared_ptr<Component>>> components() const;

  bool operator==(const Properties &props) const;
  bool operator!=(const Properties &props) const {
    return !operator==(props);
  }

  std::string to_string() const;
  MSK_EXPORT friend std::ostream &operator<<(std::ostream &os, const Properties &p);

 public:
#define DEFINE_PROPERTY_METHODS(TYPE, SETTER, GETTER)                                   \
  void SETTER(const std::string &name, const TYPE &value, bool warn_duplicates = true); \
  const TYPE &GETTER(const std::string &name) const;                                    \
  const TYPE &GETTER(const std::string &name, const TYPE &def_val) const;

  DEFINE_PROPERTY_METHODS(bool, set_bool, get_bool)
  DEFINE_PROPERTY_METHODS(int, set_int, get_int)
  DEFINE_PROPERTY_METHODS(float, set_float, get_float)
  DEFINE_PROPERTY_METHODS(std::string, set_string, string)
  DEFINE_PROPERTY_METHODS(NamedReference, set_named_reference, named_reference)
  DEFINE_PROPERTY_METHODS(Vector3, set_vector3, vector3)
  DEFINE_PROPERTY_METHODS(Color3, set_color, color)
  DEFINE_PROPERTY_METHODS(Transform4, set_transform, transform)
  DEFINE_PROPERTY_METHODS(std::shared_ptr<Component>, set_component, component)
  DEFINE_PROPERTY_METHODS(void *const, set_pointer, pointer)
#undef DEFINE_PROPERTY_METHODS
 private:
  std::shared_ptr<Component> find_component(const std::string &name) const;
  struct Entry {
    std::variant<bool, int, Float, std::string,
                 Vector3, Transform4, Color3,
                 NamedReference, std::shared_ptr<Component>,
                 const void *>
        data;
  };
  struct PropertiesPrivate {
    std::map<std::string, Entry> entries;
    std::string id, plugin_name;
  };
  std::unique_ptr<PropertiesPrivate> d;
};

}  // namespace misaki::render