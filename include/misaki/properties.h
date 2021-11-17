#pragma once

#include "fwd.h"
#include "logger.h"
#include "object.h"
#include <map>
#include <variant>

namespace misaki {

class NamedReference {
public:
    NamedReference(const std::string &value) : m_value(value) {}
    operator const std::string &() const { return m_value; }
    bool operator==(const NamedReference &r) const {
        return r.m_value == m_value;
    }
    bool operator!=(const NamedReference &r) const {
        return r.m_value != m_value;
    }

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
        Object,
        Pointer
    };
    Properties();
    explicit Properties(const std::string &instance_name);
    Properties(const Properties &props);
    void operator=(const Properties &props);
    ~Properties();

    const std::string &instance_name() const;
    void set_instance_name(const std::string &name);
    bool has_property(const std::string &name) const;
    Type type(const std::string &name) const;
    const std::string &id() const;
    void set_id(const std::string &id);
    std::vector<std::string> property_names() const;
    std::vector<std::pair<std::string, NamedReference>>
    named_references() const;
    std::vector<std::pair<std::string, ref<Object>>> objects() const;

    bool operator==(const Properties &props) const;
    bool operator!=(const Properties &props) const {
        return !operator==(props);
    }

    std::string to_string() const;
    MSK_EXPORT friend std::ostream &operator<<(std::ostream &os,
                                               const Properties &p);

public:
#define DEFINE_PROPERTY_METHODS(TYPE, SETTER, GETTER)                          \
    void SETTER(const std::string &name, const TYPE &value,                    \
                bool warn_duplicates = true);                                  \
    const TYPE &GETTER(const std::string &name) const;                         \
    const TYPE &GETTER(const std::string &name, const TYPE &def_val) const;

    DEFINE_PROPERTY_METHODS(bool, set_bool, bool_)
    DEFINE_PROPERTY_METHODS(int, set_int, int_)
    DEFINE_PROPERTY_METHODS(float, set_float, float_)
    DEFINE_PROPERTY_METHODS(std::string, set_string, string)
    DEFINE_PROPERTY_METHODS(NamedReference, set_named_reference,
                            named_reference)
    DEFINE_PROPERTY_METHODS(Eigen::Vector3f, set_vector3, vector3)
    DEFINE_PROPERTY_METHODS(Color3, set_color, color)
    DEFINE_PROPERTY_METHODS(Transform4f, set_transform, transform)
    DEFINE_PROPERTY_METHODS(ref<Object>, set_object, object)
    DEFINE_PROPERTY_METHODS(void *const, set_pointer, pointer)
#undef DEFINE_PROPERTY_METHODS

    // Texture
    ref<Texture> texture(const std::string &name) const;
    ref<Texture> texture(const std::string &name,
                         ref<Texture> &def_val) const;
    ref<Texture> texture(const std::string &name, float def_val) const;
    /*
    template <typename Texture>
    ref<Texture> texture(const std::string &name) const {
        if (!has_property(name)) {
            Throw(R"(Property {} has not been specified!)", name);
        }
        auto p_type = type(name);
        if (p_type == Properties::Type::Object) {
            ref<Object> object = find_object(name);
            if (!object->clazz()->derives_from(MSK_CLASS(Texture)))
                Throw("The property \"{}\" has the wrong type (expected "
                      " <spectrum> or <texture>).",
                      name);
            return (Texture *) object.get();
        } else if (p_type == Properties::Type::Color) {
            Properties props("srgb");
            props.set_color("color", color(name));
            return (Texture *) PluginManager::instance()
                ->create_object<Texture>(props)
                .get();
        } else if (p_type == Properties::Type::Float) {
            Properties props("srgb");
            props.set_color("color", Color3::Constant(float_(name)));
            return (Texture *) PluginManager::instance()
                ->create_object<Texture>(props)
                .get();
        } else {
            Throw("The property \"{}\" has the wrong type (expected "
                  " <spectrum> or <texture>).",
                  name);
        }
    }

    template <typename Texture>
    ref<Texture> texture(const std::string &name,
                         std::shared_ptr<Texture> &def_val) const {
        if (!has_property(name))
            return def_val;
        return texture<Texture>(name);
    }

    template <typename Texture>
    ref<Texture> texture(const std::string &name, float def_val) const {
        if (!has_property(name)) {
            Properties props("srgb");
            props.set_color("color", Color3::Constant(def_val));
            return (Texture *) PluginManager::instance()
                ->create_object<Texture>(props)
                .get();
        }
        return texture<Texture>(name);
    }

    /// Retrieve a 3D texture
    template <typename Volume>
    ref<Volume> volume(const std::string &name) const {

        if (!has_property(name))
            Throw("Property \"%s\" has not been specified!", name);

        auto p_type = type(name);
        if (p_type == Properties::Type::Object) {
            ref<Object> object = find_object(name);
            if (!object->clazz()->derives_from(MSK_CLASS(Volume::Texture)) &&
                !object->clazz()->derives_from(MSK_CLASS(Volume)))
                Throw("The property \"{}\" has the wrong type (expected "
                      " <spectrum>, <texture>. or <volume>).",
                      name);

            if (object->clazz()->derives_from(MSK_CLASS(Volume))) {
                return (Volume *) object.get();
            } else {
                Properties props("constvolume");
                props.set_object("color", object);
                return (Volume *) PluginManager::instance()
                    ->create_object<Volume>(props)
                    .get();
            }
        } else if (p_type == Properties::Type::Float) {
            Properties props("constvolume");
            props.set_float("color", float_(name));
            return (Volume *) PluginManager::instance()
                ->create_object<Volume>(props)
                .get();
        } else {
            Throw("The property \"{}\" has the wrong type (expected "
                  " <spectrum>, <texture> or <volume>).",
                  name);
        }
    }

    /// Retrieve a 3D texture (use the provided texture if no entry exists)
    template <typename Volume>
    ref<Volume> volume(const std::string &name, ref<Volume> def_val) const {
        if (!has_property(name))
            return def_val;
        return volume<Volume>(name);
    }

    template <typename Volume>
    ref<Volume> volume(const std::string &name, float def_val) const {
        if (!has_property(name)) {
            Properties props("constvolume");
            props.set_float("color", def_val);
            return (Volume *) PluginManager::instance()
                ->create_object<Volume>(props)
                .get();
        }
        return volume<Volume>(name);
    }
    */
private:
    ref<Object> find_object(const std::string &name) const;

    struct Entry {
        std::variant<bool, int, float, std::string, Eigen::Vector3f,
                     Transform4f, Color3,
                     NamedReference, ref<Object>, const void *>
            data;
    };
    struct PropertiesPrivate {
        std::map<std::string, Entry> entries;
        std::string id, instance_name;
    };
    std::unique_ptr<PropertiesPrivate> d;
};

} // namespace misaki