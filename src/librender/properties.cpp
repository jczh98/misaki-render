#include <misaki/core/logger.h>
#include <misaki/core/properties.h>
#include <misaki/render/texture.h>
#include <misaki/core/manager.h>

#include <sstream>
#include <unordered_map>

namespace misaki {

#define DEFINE_PROPERTY_ACCESSOR(TYPE, TAG_NAME, SETTER, GETTER)               \
    void Properties::SETTER(const std::string &name, TYPE const &value,        \
                            bool warn_duplicates) {                            \
        if (has_property(name) && warn_duplicates)                             \
            Log(Warn, "Property \"{}\" was specified multiple times!", name);  \
        d->entries[name].data = (TYPE) value;                                  \
    }                                                                          \
                                                                               \
    TYPE const &Properties::GETTER(const std::string &name) const {            \
        const auto it = d->entries.find(name);                                 \
        if (it == d->entries.end())                                            \
            Throw("Property \"{}\" has not been specified!", name);            \
        if (!std::holds_alternative<TYPE>(it->second.data))                    \
            Throw(                                                             \
                "The property \"{}\" has the wrong TYPE (expected <" #TAG_NAME \
                ">).",                                                         \
                name);                                                         \
        return (TYPE const &) it->second.data;                                 \
    }                                                                          \
                                                                               \
    TYPE const &Properties::GETTER(const std::string &name,                    \
                                   TYPE const &def_val) const {                \
        const auto it = d->entries.find(name);                                 \
        if (it == d->entries.end())                                            \
            return def_val;                                                    \
        if (!std::holds_alternative<TYPE>(it->second.data))                    \
            Throw(                                                             \
                "The property \"{}\" has the wrong TYPE (expected <" #TAG_NAME \
                ">).",                                                         \
                name);                                                         \
        return (TYPE const &) it->second.data;                                 \
    }

DEFINE_PROPERTY_ACCESSOR(bool, boolean, set_bool, bool_)
DEFINE_PROPERTY_ACCESSOR(int, integer, set_int, int_)
DEFINE_PROPERTY_ACCESSOR(float, float, set_float, float_)
DEFINE_PROPERTY_ACCESSOR(std::string, string, set_string, string)
DEFINE_PROPERTY_ACCESSOR(Eigen::Vector3f, vector, set_vector3, vector3)
DEFINE_PROPERTY_ACCESSOR(Color3, color, set_color, color)
DEFINE_PROPERTY_ACCESSOR(Transform4f, transform, set_transform, transform)
DEFINE_PROPERTY_ACCESSOR(NamedReference, ref, set_named_reference,
                         named_reference)
DEFINE_PROPERTY_ACCESSOR(ref<Object>, object, set_object, object)
DEFINE_PROPERTY_ACCESSOR(const void *, pointer, set_pointer, pointer)

Properties::Properties()
    : d(new PropertiesPrivate()) {
}

Properties::Properties(const std::string &instance_name)
    : d(new PropertiesPrivate()) {
    d->instance_name = instance_name;
}

Properties::Properties(const Properties &props)
    : d(new PropertiesPrivate(*props.d)) {
}

Properties::~Properties() = default;

void Properties::operator=(const Properties &props) { (*d) = *props.d; }

bool Properties::has_property(const std::string &name) const {
    return d->entries.find(name) != d->entries.end();
}

namespace {
struct PropertyTypeVisitor {
    using Type = Properties::Type;

    Type operator()(const std::nullptr_t &) {
        throw std::runtime_error("Internal error");
    }

    Type operator()(const bool &) { return Type::Bool; }
    Type operator()(const int &) { return Type::Int; }
    Type operator()(const float &) { return Type::Float; }
    Type operator()(const Eigen::Vector3f &) { return Type::Vector3; }
    Type operator()(const std::string &) { return Type::String; }
    Type operator()(const Transform4f &) { return Type::Transform; }
    Type operator()(const Color3 &) { return Type::Color; }
    Type operator()(const NamedReference &) { return Type::NamedReference; }
    Type operator()(const ref<Object> &) { return Type::Object; }
    Type operator()(const void *&) { return Type::Pointer; }
};

struct StreamVisitor {
    std::ostream &os;

    explicit StreamVisitor(std::ostream &os)
        : os(os) {
    }

    void operator()(const std::nullptr_t &) {
        throw std::runtime_error("Internal error");
    }

    void operator()(const bool &b) { os << (b ? "true" : "false"); }
    void operator()(const int &i) { os << i; }
    void operator()(const float &f) { os << f; }
    void operator()(const Eigen::Vector3f &t) { os << t; }
    void operator()(const std::string &s) { os << "\"" << s << "\""; }
    void operator()(const Transform4f &t) { os << t; }
    void operator()(const Color3 &t) { os << t; }

    void operator()(const NamedReference &nr) {
        os << "\"" << (const std::string &) nr << "\"";
    }

    void operator()(const ref<Object> &o) { os << o->to_string(); }
    void operator()(const void *&p) { os << p; }
};
} // namespace

Properties::Type Properties::type(const std::string &name) const {
    const auto it = d->entries.find(name);
    if (it == d->entries.end())
        Throw("type(): Could not find property named \"{}\"!", name);

    return std::visit(PropertyTypeVisitor(), it->second.data);
}

const std::string &Properties::instance_name() const {
    return d->instance_name;
}

void Properties::set_instance_name(const std::string &name) {
    d->instance_name = name;
}

const std::string &Properties::id() const { return d->id; }

void Properties::set_id(const std::string &id) { d->id = id; }

std::vector<std::string> Properties::property_names() const {
    std::vector<std::string> result;
    for (const auto &e : d->entries)
        result.push_back(e.first);
    return result;
}

std::vector<std::pair<std::string, NamedReference>>
Properties::named_references() const {
    std::vector<std::pair<std::string, NamedReference>> result;
    result.reserve(d->entries.size());
    for (auto &e : d->entries) {
        auto type = std::visit(PropertyTypeVisitor(), e.second.data);
        if (type != Type::NamedReference)
            continue;
        auto const &value = (const NamedReference &) e.second.data;
        result.emplace_back(e.first, value);
    }
    return result;
}

std::vector<std::pair<std::string, ref<Object>>> Properties::objects() const {
    std::vector<std::pair<std::string, ref<Object>>> result;
    result.reserve(d->entries.size());
    for (auto &e : d->entries) {
        auto type = std::visit(PropertyTypeVisitor(), e.second.data);
        if (type != Type::Object)
            continue;
        result.emplace_back(e.first, (const ref<Object> &) e.second);
    }
    return result;
}

bool Properties::operator==(const Properties &p) const {
    if (d->instance_name != p.d->instance_name || d->id != p.d->id ||
        d->entries.size() != p.d->entries.size())
        return false;

    for (const auto &e : d->entries) {
        auto it = p.d->entries.find(e.first);
        if (it == p.d->entries.end())
            return false;
        if (e.second.data != it->second.data)
            return false;
    }

    return true;
}

ref<Texture> Properties::texture(const std::string &name) const {
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
    } else if (p_type == Properties::Type::Float) {
        Properties props("uniform");
        props.set_float("value", float_(name));
        return (Texture *) InstanceManager::get()
                           ->create_instance<Texture>(props)
                           .get();
    } else {
        Throw("The property \"{}\" has the wrong type (expected "
              " <spectrum> or <texture>).",
              name);
    }
}

ref<Texture> Properties::texture(const std::string &name,
                                 ref<Texture> &def_val) const {
    if (!has_property(name))
        return def_val;
    return texture(name);
}

ref<Texture> Properties::texture(const std::string &name, float def_val) const {
    if (!has_property(name)) {
        Properties props("srgb");
        props.set_color("value", Color3::Constant(def_val));
        return (Texture *) InstanceManager::get()
                           ->create_instance<Texture>(props)
                           .get();
    }
    return texture(name);
}

ref<Object> Properties::find_object(const std::string &name) const {
    const auto it = d->entries.find(name);
    if (it == d->entries.end())
        return ref<Object>();

    if (!std::holds_alternative<ref<Object>>(it->second.data))
        Throw("The property \"{}\" has the wrong type.", name);

    return (const ref<Object> &) it->second.data;
}

std::string Properties::to_string() const {
    std::stringstream os;
    auto it = d->entries.begin();

    os << "Properties[" << std::endl
        << "  instance_name = \"" << (d->instance_name) << "\"," << std::endl
        << "  id = \"" << d->id << "\"," << std::endl
        << "  elements = (" << std::endl;
    while (it != d->entries.end()) {
        os << "    \"" << it->first << "\" -> ";
        std::visit(StreamVisitor(os), it->second.data);
        if (++it != d->entries.end())
            os << ",";
        os << std::endl;
    }
    os << "  )" << std::endl << "]" << std::endl;

    return os.str();
}

std::ostream &operator<<(std::ostream &os, const Properties &p) {
    auto it = p.d->entries.begin();

    os << "Properties[" << std::endl
        << "  instance_name = \"" << (p.d->instance_name) << "\"," << std::endl
        << "  id = \"" << p.d->id << "\"," << std::endl
        << "  elements = (" << std::endl;
    while (it != p.d->entries.end()) {
        os << "    \"" << it->first << "\" -> ";
        std::visit(StreamVisitor(os), it->second.data);
        if (++it != p.d->entries.end())
            os << ",";
        os << std::endl;
    }
    os << "  )" << std::endl << "]" << std::endl;

    return os;
}

} // namespace misaki
