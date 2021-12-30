#include <misaki/core/logger.h>
#include <misaki/core/manager.h>
#include <misaki/core/object.h>
#include <misaki/core/properties.h>
#include <misaki/core/xml.h>

#include <fstream>
#include <iostream>
#include <pugixml.hpp>
#include <sstream>

namespace misaki::xml {

enum class Tag {
    Boolean,
    Integer,
    Float,
    String,
    Vector,
    RGB,
    Spectrum,
    Transform,
    Translate,
    Matrix,
    Rotate,
    Scale,
    LookAt,
    Object,
    NamedReference,
    Include,
    Alias,
    Default,
    Invalid
};

namespace detail {

// Throws if non-whitespace characters are found after the given index.
static void check_whitespace_only(const std::string &s, size_t offset) {
    for (size_t i = offset; i < s.size(); ++i) {
        if (!std::isspace(s[i]))
            Throw("Invalid trailing characters in floating point number \"{}\"",
              s);
    }
}

static float stof(const std::string &s) {
    size_t offset = 0;
    float result  = std::stof(s, &offset);
    check_whitespace_only(s, offset);
    return result;
}

static int64_t stoll(const std::string &s) {
    size_t offset  = 0;
    int64_t result = std::stoll(s, &offset);
    check_whitespace_only(s, offset);
    return result;
}

static std::unordered_map<std::string, Tag> *tags = nullptr;
static std::unordered_map<std::string, // e.g. bsdf.scalar_rgb
                          const Class *> *tag_class = nullptr;

// Called by Class::Class()
void register_class(const Class *class_) {
    assert(class_ != nullptr);

    if (!tags) {
        tags      = new std::unordered_map<std::string, Tag>();
        tag_class = new std::unordered_map<std::string, const Class *>();

        // Create an initial mapping of tag names to IDs
        (*tags)["boolean"]   = Tag::Boolean;
        (*tags)["integer"]   = Tag::Integer;
        (*tags)["float"]     = Tag::Float;
        (*tags)["string"]    = Tag::String;
        (*tags)["vector"]    = Tag::Vector;
        (*tags)["transform"] = Tag::Transform;
        (*tags)["translate"] = Tag::Translate;
        (*tags)["matrix"]    = Tag::Matrix;
        (*tags)["rotate"]    = Tag::Rotate;
        (*tags)["scale"]     = Tag::Scale;
        (*tags)["lookat"]    = Tag::LookAt;
        (*tags)["ref"]       = Tag::NamedReference;
        (*tags)["rgb"]       = Tag::RGB;
        (*tags)["spectrum"]  = Tag::Spectrum;
        (*tags)["include"]   = Tag::Include;
        (*tags)["alias"]     = Tag::Alias;
        (*tags)["default"]   = Tag::Default;
    }

    // Register the new class as an object tag
    const std::string &alias = class_->alias();

    if (tags->find(alias) == tags->end())
        (*tags)[alias] = Tag::Object;
    (*tag_class)[alias] = class_;

    if (alias == "texture")
        (*tag_class)["spectrum"] = class_;
}

// Called by Class::static_shutdown()
void cleanup() {
    delete tags;
    delete tag_class;
    tags      = nullptr;
    tag_class = nullptr;
}

// Helper function: map a position offset in bytes to a more readable
// line/column value
static std::string string_offset(const std::string &string, ptrdiff_t pos) {
    std::istringstream is(string);
    char buffer[1024];
    int line = 0, line_start = 0, offset = 0;
    while (is.good()) {
        is.read(buffer, sizeof(buffer));
        for (int i = 0; i < is.gcount(); ++i) {
            if (buffer[i] == '\n') {
                if (offset + i >= pos)
                    return fmt::format("line {}, col {}", line + 1,
                                       pos - line_start);
                ++line;
                line_start = offset + i;
            }
        }
        offset += (int) is.gcount();
    }
    return "byte offset " + std::to_string(pos);
}

// Helper function: map a position offset in bytes to a more readable
// line/column value
static std::string file_offset(const fs::path &filename, ptrdiff_t pos) {
    std::fstream is(filename.native());
    char buffer[1024];
    int line = 0, line_start = 0, offset = 0;
    while (is.good()) {
        is.read(buffer, sizeof(buffer));
        for (int i = 0; i < is.gcount(); ++i) {
            if (buffer[i] == '\n') {
                if (offset + i >= pos)
                    return fmt::format("line {}, col {}", line + 1,
                                       pos - line_start);
                ++line;
                line_start = offset + i;
            }
        }
        offset += (int) is.gcount();
    }
    return "byte offset " + std::to_string(pos);
}

struct XMLSource {
    std::string id;
    const pugi::xml_document &doc;
    std::function<std::string(ptrdiff_t)> offset;
    size_t depth = 0;

    template <typename... Args> [[noreturn]] void throw_error(
        const pugi::xml_node &n,
        const std::string &msg_, Args &&...args) {
        std::string msg = "Error while loading \"{}\" (at {}): " + msg_ + ".";
        Throw(msg.c_str(), id, offset(n.offset_debug()), args...);
    }
};

struct XMLObject {
    Properties props;
    const Class *clazz = nullptr;
    std::string src_id;
    std::string alias;
    std::function<std::string(ptrdiff_t)> offset;
    ref<Object> object;
    size_t location = 0;
};

enum class ColorMode { RGB, Spectral };

struct XMLParseContext {
    std::unordered_map<std::string, XMLObject> instances;
    Transform4f transform;
    size_t id_counter = 0;

    XMLParseContext() {
    }
};

// Helper function to check if attributes are fully specified
static void check_attributes(XMLSource &src, const pugi::xml_node &node,
                             std::set<std::string> &&attrs,
                             bool expect_all = true) {
    bool found_one = false;
    for (auto attr : node.attributes()) {
        auto it = attrs.find(attr.name());
        if (it == attrs.end())
            src.throw_error(node,
                            "unexpected attribute \"{}\" in element \"{}\"",
                            attr.name(), node.name());
        attrs.erase(it);
        found_one = true;
    }
    if (!attrs.empty() && (!found_one || expect_all))
        src.throw_error(node, "missing attribute \"{}\" in element \"{}\"",
                        *attrs.begin(), node.name());
}

void expand_value_to_xyz(XMLSource &src, pugi::xml_node &node) {
    if (node.attribute("value")) {
        auto list = string::tokenize(node.attribute("value").value());
        if (node.attribute("x") || node.attribute("y") || node.attribute("z"))
            src.throw_error(node, "can't mix and match \"value\" and "
                            "\"x\"/\"y\"/\"z\" attributes");
        if (list.size() == 1) {
            node.append_attribute("x") = list[0].c_str();
            node.append_attribute("y") = list[0].c_str();
            node.append_attribute("z") = list[0].c_str();
        } else if (list.size() == 3) {
            node.append_attribute("x") = list[0].c_str();
            node.append_attribute("y") = list[1].c_str();
            node.append_attribute("z") = list[2].c_str();
        } else {
            src.throw_error(
                node, "\"value\" attribute must have exactly 1 or 3 elements");
        }
        node.remove_attribute("value");
    }
}

Eigen::Vector3f parse_named_vector(XMLSource &src, pugi::xml_node &node,
                                   const std::string &attr_name) {
    auto vec_str = node.attribute(attr_name.c_str()).value();
    auto list    = string::tokenize(vec_str);
    if (list.size() != 3)
        src.throw_error(node, "\"{}\" attribute must have exactly 3 elements",
                        attr_name);
    try {
        return Eigen::Vector3f(detail::stof(list[0]), detail::stof(list[1]),
                               detail::stof(list[2]));
    } catch (...) {
        src.throw_error(node, "could not parse floating point values in \"{}\"",
                        vec_str);
    }
}

Eigen::Vector3f parse_vector(XMLSource &src, pugi::xml_node &node,
                             float def_val = 0.f) {
    std::string value;
    try {
        float x = def_val, y = def_val, z = def_val;
        value   = node.attribute("x").value();
        if (!value.empty())
            x = detail::stof(value);
        value = node.attribute("y").value();
        if (!value.empty())
            y = detail::stof(value);
        value = node.attribute("z").value();
        if (!value.empty())
            z = detail::stof(value);
        return Eigen::Vector3f(x, y, z);
    } catch (...) {
        src.throw_error(node, "could not parse floating point value \"{}\"",
                        value);
    }
}

ref<Object> create_texture_from_rgb(const std::string &name,
                                    Color<float, 3> color,
                                    bool within_emitter) {
    Properties props(within_emitter ? "srgb_d65" : "srgb");
    props.set_color("color", color);

    return InstanceManager::get()->create_instance(
        props, Class::for_name("Texture"));
}

ref<Object> create_texture_from_spectrum(
    const std::string &name, float const_value, std::vector<float> &wavelengths,
    std::vector<float> &values) {
    const Class *class_ = Class::for_name("Texture");

    if (wavelengths.empty()) {
        Properties props("uniform");
        props.set_float("value", const_value);

        ref<Object> obj =
            InstanceManager::get()->create_instance(props, class_);
        auto expanded = obj->expand();
        assert(expanded.size() <= 1);
        if (!expanded.empty())
            obj = expanded[0];
        return obj;
    } else {
        /* Values are scaled so that integrating the spectrum against the CIE
           curves and converting to sRGB yields (1, 1, 1) for D65. */
        float unit_conversion = 1.f;

        /* Detect whether wavelengths are regularly sampled and potentially
            apply the conversion factor. */
        bool is_regular = true;
        float interval  = 0.f;

        for (size_t n = 0; n < wavelengths.size(); ++n) {
            values[n] *= unit_conversion;

            if (n <= 0)
                continue;

            float distance = (wavelengths[n] - wavelengths[n - 1]);
            if (distance < 0.f)
                Throw("Wavelengths must be specified in increasing order!");
            if (n == 1)
                interval = distance;
            else if (std::abs(distance - interval) > math::Epsilon<float>)
                is_regular = false;
        }
        Properties props;
        if (is_regular) {
            props.set_instance_name("regular");
            props.set_int("size", wavelengths.size());
            props.set_float("lambda_min", wavelengths.front());
            props.set_float("lambda_max", wavelengths.back());
            props.set_pointer("values", values.data());
        } else {
            props.set_instance_name("irregular");
            props.set_int("size", wavelengths.size());
            props.set_pointer("wavelengths", wavelengths.data());
            props.set_pointer("values", values.data());
        }
        return InstanceManager::get()->create_instance(props, class_);
    }
}

static std::pair<std::string, std::string>
parse_xml(XMLSource &src, XMLParseContext &ctx, pugi::xml_node &node,
          Tag parent_tag, Properties &props, ParameterList &param,
          size_t &arg_counter, int depth, bool within_emitter = false,
          bool within_spectrum                                = false) {
    try {
        if (!param.empty()) {
            for (auto attr : node.attributes()) {
                std::string value = attr.value();
                if (value.find('$') == std::string::npos)
                    continue;
                for (const auto &kv : param)
                    string::replace_inplace(value, "$" + kv.first, kv.second);
                attr.set_value(value.c_str());
            }
        }
        // Skip comments
        if (node.type() == pugi::node_comment ||
            node.type() == pugi::node_declaration)
            return { "", "" };
        if (node.type() != pugi::node_element)
            src.throw_error(node, "unexpected content");
        auto it = tags->find(node.name());
        if (it == tags->end())
            src.throw_error(node, R"(unexpected tag "{}")", node.name());
        Tag tag                      = it->second;
        bool has_parent              = parent_tag != Tag::Invalid;
        bool parent_is_object        = has_parent && parent_tag == Tag::Object;
        bool current_is_object       = tag == Tag::Object;
        bool parent_is_transform     = parent_tag == Tag::Transform;
        bool current_is_transform_op =
            tag == Tag::Translate || tag == Tag::Rotate || tag == Tag::Scale ||
            tag == Tag::LookAt || tag == Tag::Matrix;
        if (!has_parent && !current_is_object)
            src.throw_error(node, R"(root element "{}" must be an object)",
                            node.name());
        if (parent_is_transform != current_is_transform_op) {
            if (parent_is_transform)
                src.throw_error(
                    node,
                    "transform nodes can only contain transform operations");
            else
                src.throw_error(
                    node,
                    "transform operations can only occur in a transform node");
        }
        if (has_parent && !parent_is_object &&
            !(parent_is_transform && current_is_transform_op))
            src.throw_error(node,
                            R"(node "{}" cannot occur as child of a property)",
                            node.name());
        if (std::string(node.name()) == "scene") {
            node.append_attribute("type") = "scene";
        } else if (tag == Tag::Transform)
            ctx.transform = Transform4f();
        if (node.attribute("name")) {
            auto name = node.attribute("name").value();
            if (string::starts_with(name, "_"))
                src.throw_error(
                    node,
                    R"(invalid parameter name "{}" in element "{}": leading underscores are reserved for internal identifiers.)",
                    name, node.name());
        } else if (current_is_object || tag == Tag::NamedReference) {
            node.append_attribute("name") =
                fmt::format("_arg_{}", arg_counter++).c_str();
        }
        if (node.attribute("id")) {
            auto id = node.attribute("id").value();
            if (string::starts_with(id, "_"))
                src.throw_error(
                    node,
                    R"(invalid id "{}" in element "{}": leading underscores are reserved for internal identifiers.)",
                    id, node.name());
        } else if (current_is_object) {
            node.append_attribute("id") =
                fmt::format("_unnamed_{}", ctx.id_counter++).c_str();
        }
        switch (tag) {
            case Tag::Object: {
                check_attributes(src, node, { "type", "id", "name" });
                std::string id        = node.attribute("id").value(),
                            name      = node.attribute("name").value(),
                            type      = node.attribute("type").value(),
                            node_name = node.name();
                Properties props_nested(type);
                props_nested.set_id(id);
                auto it_inst = ctx.instances.find(id);
                if (it_inst != ctx.instances.end())
                    src.throw_error(
                        node,
                        "\"{}\" has duplicate id \"{}\" (previous was at {})",
                        node_name, id, src.offset(it_inst->second.location));
                auto it2 = tag_class->find(node_name);
                if (it2 == tag_class->end())
                    src.throw_error(node,
                                    "could not retrieve class object for "
                                    "tag \"{}\"",
                                    node_name);
                size_t arg_counter_nested = 0;
                for (pugi::xml_node &ch : node.children()) {
                    auto [arg_name, nested_id] =
                        parse_xml(src, ctx, ch, tag, props_nested, param,
                                  arg_counter_nested, depth + 1,
                                  node_name == "emitter",
                                  node_name == "spectrum");
                    if (!nested_id.empty())
                        props_nested.set_named_reference(arg_name, nested_id);
                }
                auto &inst    = ctx.instances[id];
                inst.props    = props_nested;
                inst.clazz    = it2->second;
                inst.offset   = src.offset;
                inst.src_id   = src.id;
                inst.location = node.offset_debug();
                return { name, id };
            }
            break;
            case Tag::NamedReference: {
                check_attributes(src, node, { "name", "id" });
                auto id   = node.attribute("id").value();
                auto name = node.attribute("name").value();
                return std::make_pair(name, id);
            }
            break;
            case Tag::String: {
                check_attributes(src, node, { "name", "value" });
                props.set_string(node.attribute("name").value(),
                                 node.attribute("value").value());
            }
            break;
            case Tag::Float: {
                check_attributes(src, node, { "name", "value" });
                std::string value = node.attribute("value").value();
                float value_float;
                try {
                    value_float = detail::stof(value);
                } catch (...) {
                    src.throw_error(
                        node, "could not parse floating point value \"%s\"",
                        value);
                }
                props.set_float(node.attribute("name").value(), value_float);
            }
            break;
            case Tag::Integer: {
                check_attributes(src, node, { "name", "value" });
                std::string value = node.attribute("value").value();
                int64_t value_long;
                try {
                    value_long = detail::stoll(value);
                } catch (...) {
                    src.throw_error(
                        node, "could not parse integer value \"%s\"", value);
                }
                props.set_int(node.attribute("name").value(), value_long);
            }
            break;
            case Tag::Vector: {
                detail::expand_value_to_xyz(src, node);
                check_attributes(src, node, { "name", "x", "y", "z" });
                props.set_vector3(node.attribute("name").value(),
                                  detail::parse_vector(src, node));
            }
            break;
            case Tag::Matrix: {
                check_attributes(src, node, { "value" });
                auto tokens =
                    string::tokenize(node.attribute("value").value(), " ");
                if (tokens.size() != 16)
                    Throw("matrix: expected 16 values");
                Eigen::Matrix4f matrix;
                for (int i = 0; i < 4; ++i) {
                    for (int j = 0; j < 4; ++j) {
                        try {
                            matrix(i, j) = detail::stof(tokens[i * 4 + j]);
                        } catch (...) {
                            src.throw_error(
                                node,
                                "could not parse floating point value \"{}\"",
                                tokens[i * 4 + j]);
                        }
                    }
                }
                ctx.transform = Transform4f(matrix) * ctx.transform;
                break;
            }
            case Tag::RGB: {
                check_attributes(src, node, { "name", "value" });
                std::vector<std::string> tokens =
                    string::tokenize(node.attribute("value").value());

                if (tokens.size() == 1) {
                    tokens.push_back(tokens[0]);
                    tokens.push_back(tokens[0]);
                }
                if (tokens.size() != 3)
                    src.throw_error(
                        node,
                        "'rgb' tag requires one or three values (got \"%s\")",
                        node.attribute("value").value());

                Color3 color;
                try {
                    color =
                        Color3(detail::stof(tokens[0]), detail::stof(tokens[1]),
                               detail::stof(tokens[2]));
                } catch (...) {
                    src.throw_error(node, "could not parse RGB value \"%s\"",
                                    node.attribute("value").value());
                }

                if (!within_spectrum) {
                    std::string name = node.attribute("name").value();
                    ref<Object> obj  = detail::create_texture_from_rgb(
                        name, color, within_emitter);
                    props.set_object(name, obj);
                } else {
                    props.set_color("color", color);
                }
            }
            break;
            case Tag::Spectrum: {
                check_attributes(src, node, { "name", "value", "filename" },
                                 false);
                std::string name = node.attribute("name").value();
                std::vector<float> wavelengths, values;
                bool has_value    = !node.attribute("value").empty(),
                     has_filename = !node.attribute("filename").empty(),
                     is_constant  =
                         has_value &&
                         string::tokenize(node.attribute("value").value())
                         .size() == 1;
                float const_value = 1.f;
                if (has_value == has_filename) {
                    src.throw_error(node,
                                    "'spectrum' tag requires one of \"value\" "
                                    "or \"filename\" attributes");
                } else if (is_constant) {
                    /* A constant spectrum is specified. */
                    std::vector<std::string> tokens =
                        string::tokenize(node.attribute("value").value());

                    try {
                        const_value = detail::stof(tokens[0]);
                    } catch (...) {
                        src.throw_error(
                            node, "could not parse constant spectrum \"%s\"",
                            tokens[0]);
                    }
                } else {
                    if (has_value) {
                        std::vector<std::string> tokens =
                            string::tokenize(node.attribute("value").value());

                        for (const std::string &token : tokens) {
                            std::vector<std::string> pair =
                                string::tokenize(token, ":");
                            if (pair.size() != 2)
                                src.throw_error(node,
                                                "invalid spectrum (expected "
                                                "wavelength:value pairs)");

                            float wavelength, value;
                            try {
                                wavelength = detail::stof(pair[0]);
                                value      = detail::stof(pair[1]);
                            } catch (...) {
                                src.throw_error(node,
                                                "could not parse "
                                                "wavelength:value pair: \"%s\"",
                                                token);
                            }

                            wavelengths.push_back(wavelength);
                            values.push_back(value);
                        }
                    } else if (has_filename) {
                        // TODO
                    }
                }
                ref<Object> obj = detail::create_texture_from_spectrum(
                    name, const_value, wavelengths, values);

                props.set_object(name, obj);
            }
            break;
            case Tag::Transform: {
                check_attributes(src, node, { "name" });
                ctx.transform = Transform4f();
            }
            break;
            case Tag::LookAt: {
                check_attributes(src, node, { "origin", "target", "up" });

                auto origin = parse_named_vector(src, node, "origin");
                auto target = parse_named_vector(src, node, "target");
                auto up     = parse_named_vector(src, node, "up");

                auto result = Transform4f::lookat(origin, target, up);
                if (result.matrix().hasNaN())
                    src.throw_error(node, "invalid lookat transformation");
                ctx.transform = result * ctx.transform;
            }
            break;
            case Tag::Translate: {
                detail::expand_value_to_xyz(src, node);
                check_attributes(src, node, { "x", "y", "z" }, false);
                auto vec      = detail::parse_vector(src, node);
                ctx.transform = Transform4f::translate(vec) * ctx.transform;
            }
            break;
            case Tag::Scale: {
                detail::expand_value_to_xyz(src, node);
                check_attributes(src, node, { "x", "y", "z" }, false);
                auto vec      = detail::parse_vector(src, node, 1.f);
                ctx.transform = Transform4f::scale(vec) * ctx.transform;
            }
            break;
        }
        for (pugi::xml_node &ch : node.children())
            parse_xml(src, ctx, ch, tag, props, param, arg_counter, depth + 1);
        if (tag == Tag::Transform)
            props.set_transform(node.attribute("name").value(), ctx.transform);
    } catch (const std::exception &e) {
        if (strstr(e.what(), "Error while loading") == nullptr)
            src.throw_error(node, "{}", e.what());
        else
            throw;
    }
    return { "", "" };
}

static ref<Object> instantiate_node(XMLParseContext &ctx,
                                    const std::string &id) {
    auto it = ctx.instances.find(id);
    if (it == ctx.instances.end())
        Throw("reference to unknown object \"{}\"!", id);
    auto &inst = it->second;
    if (inst.object) {
        return inst.object;
    }
    Properties &props            = inst.props;
    const auto &named_references = props.named_references();
    for (auto &kv : named_references) {
        try {
            auto obj = instantiate_node(ctx, kv.second);
            props.set_object(kv.first, obj, false);
        } catch (const std::exception &e) {
            if (strstr(e.what(), "Error while loading") == nullptr)
                Throw("Error while loading \"{}\" (near {}): {}", inst.src_id,
                  inst.offset(inst.location), e.what());
            else
                throw;
        }
    }
    try {
        inst.object =
            InstanceManager::get()->create_instance(props, inst.clazz);
    } catch (const std::exception &e) {
        Throw("Error while loading \"{}\" (near {}): could not instantiate "
              "{} instance of type \"{}\": {}",
              inst.src_id, inst.offset(inst.location),
              string::to_lower(inst.clazz->name()), props.instance_name(),
              e.what());
    }
    return inst.object;
}

} // namespace detail

ref<Object> load_file(const fs::path &filename, ParameterList parameters) {
    if (!fs::exists(filename)) {
        Throw(R"("{}": file not exists.)", filename.string());
    }
    Log(Info, R"(Loading XML file "{}" ..)", filename.string());
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(
        filename.native().c_str(), pugi::parse_default | pugi::parse_comments);

    detail::XMLSource src{ filename.string(), doc, [=](ptrdiff_t pos) {
        return detail::file_offset(filename, pos);
    } };

    if (!result) {
        Throw(R"(Error while loading "{}" (at {}): {})", src.id,
              src.offset(result.offset), result.description());
    }

    pugi::xml_node root = doc.document_element();

    detail::XMLParseContext ctx;
    Properties props;
    size_t arg_counter = 0; // Unused
    auto [name, id]    = detail::parse_xml(src, ctx, root, Tag::Invalid, props,
                                           parameters, arg_counter, 0);
    return detail::instantiate_node(ctx, id);
}

} // namespace misaki::xml
