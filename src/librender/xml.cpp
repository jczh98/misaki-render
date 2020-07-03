#include <misaki/render/component.h>
#include <misaki/render/logger.h>
#include <misaki/render/properties.h>
#include <misaki/render/xml.h>

#include <fstream>
#include <pugixml.hpp>
#include <sstream>

namespace misaki::render::xml {

enum class Tag {
  Boolean,
  Integer,
  Float,
  String,
  Vector,
  RGB,
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
      Throw("Invalid trailing characters in floating point number \"{}\"", s);
  }
}

static Float stof(const std::string &s) {
  size_t offset = 0;
  Float result = std::stof(s, &offset);
  check_whitespace_only(s, offset);
  return result;
}

static int64_t stoll(const std::string &s) {
  size_t offset = 0;
  int64_t result = std::stoll(s, &offset);
  check_whitespace_only(s, offset);
  return result;
}

// Helper function: map a position offset in bytes to a more readable line/column value
static std::string string_offset(const std::string &string, ptrdiff_t pos) {
  std::istringstream is(string);
  char buffer[1024];
  int line = 0, line_start = 0, offset = 0;
  while (is.good()) {
    is.read(buffer, sizeof(buffer));
    for (int i = 0; i < is.gcount(); ++i) {
      if (buffer[i] == '\n') {
        if (offset + i >= pos)
          return fmt::format("line {}, col {}", line + 1, pos - line_start);
        ++line;
        line_start = offset + i;
      }
    }
    offset += (int)is.gcount();
  }
  return "byte offset " + std::to_string(pos);
}

// Helper function: map a position offset in bytes to a more readable line/column value
static std::string file_offset(const fs::path &filename, ptrdiff_t pos) {
  std::fstream is(filename.native());
  char buffer[1024];
  int line = 0, line_start = 0, offset = 0;
  while (is.good()) {
    is.read(buffer, sizeof(buffer));
    for (int i = 0; i < is.gcount(); ++i) {
      if (buffer[i] == '\n') {
        if (offset + i >= pos)
          return fmt::format("line {}, col {}", line + 1, pos - line_start);
        ++line;
        line_start = offset + i;
      }
    }
    offset += (int)is.gcount();
  }
  return "byte offset " + std::to_string(pos);
}

struct XMLSource {
  std::string id;
  const pugi::xml_document &doc;
  std::function<std::string(ptrdiff_t)> offset;
  size_t depth = 0;

  template <typename... Args>
  [[noreturn]] void throw_error(const pugi::xml_node &n, const std::string &msg_, Args &&... args) {
    std::string msg = "Error while loading \"{}\" (at {}): " + msg_ + ".";
    Throw(msg.c_str(), id, offset(n.offset_debug()), args...);
  }
};

struct XMLObject {
  Properties props;
  std::string src_id;
  std::function<std::string(ptrdiff_t)> offset;
  std::shared_ptr<Component> comp;
  std::string type;
  size_t location = 0;
};

struct XMLParseContext {
  std::unordered_map<std::string, XMLObject> instances;
  Transform4 transform;
  size_t id_counter = 0;
};

// Helper function to check if attributes are fully specified
static void check_attributes(XMLSource &src, const pugi::xml_node &node,
                             std::set<std::string> &&attrs, bool expect_all = true) {
  bool found_one = false;
  for (auto attr : node.attributes()) {
    auto it = attrs.find(attr.name());
    if (it == attrs.end())
      src.throw_error(node, "unexpected attribute \"{}\" in element \"{}\"", attr.name(), node.name());
    attrs.erase(it);
    found_one = true;
  }
  if (!attrs.empty() && (!found_one || expect_all))
    src.throw_error(node, "missing attribute \"{}\" in element \"{}\"", *attrs.begin(), node.name());
}

void expand_value_to_xyz(XMLSource &src, pugi::xml_node &node) {
  if (node.attribute("value")) {
    auto list = string::tokenize(node.attribute("value").value());
    if (node.attribute("x") || node.attribute("y") || node.attribute("z"))
      src.throw_error(node, "can't mix and match \"value\" and \"x\"/\"y\"/\"z\" attributes");
    if (list.size() == 1) {
      node.append_attribute("x") = list[0].c_str();
      node.append_attribute("y") = list[0].c_str();
      node.append_attribute("z") = list[0].c_str();
    } else if (list.size() == 3) {
      node.append_attribute("x") = list[0].c_str();
      node.append_attribute("y") = list[1].c_str();
      node.append_attribute("z") = list[2].c_str();
    } else {
      src.throw_error(node, "\"value\" attribute must have exactly 1 or 3 elements");
    }
    node.remove_attribute("value");
  }
}

Vector3 parse_named_vector(XMLSource &src, pugi::xml_node &node, const std::string &attr_name) {
  auto vec_str = node.attribute(attr_name.c_str()).value();
  auto list = string::tokenize(vec_str);
  if (list.size() != 3)
    src.throw_error(node, "\"{}\" attribute must have exactly 3 elements", attr_name);
  try {
    return Vector3(detail::stof(list[0]),
                   detail::stof(list[1]),
                   detail::stof(list[2]));
  } catch (...) {
    src.throw_error(node, "could not parse floating point values in \"{}\"", vec_str);
  }
}

Vector3 parse_vector(XMLSource &src, pugi::xml_node &node, Float def_val = 0.f) {
  std::string value;
  try {
    Float x = def_val, y = def_val, z = def_val;
    value = node.attribute("x").value();
    if (!value.empty()) x = detail::stof(value);
    value = node.attribute("y").value();
    if (!value.empty()) y = detail::stof(value);
    value = node.attribute("z").value();
    if (!value.empty()) z = detail::stof(value);
    return Vector3(x, y, z);
  } catch (...) {
    src.throw_error(node, "could not parse floating point value \"{}\"", value);
  }
}

static std::pair<std::string, std::string> parse_xml(XMLSource &src, XMLParseContext &ctx,
                                                     pugi::xml_node &node, Tag parent_tag,
                                                     Properties &props, ParameterList &param,
                                                     size_t &arg_counter, int depth) {
  static std::map<std::string, Tag> tags;
  static std::map<std::string, std::string> tag_alias;
  static std::once_flag flag;
  std::call_once(flag, [&]() {
    tags["boolean"] = Tag::Boolean;
    tags["integer"] = Tag::Integer;
    tags["integer"] = Tag::Integer;
    tags["float"] = Tag::Float;
    tags["string"] = Tag::String;
    tags["vector"] = Tag::Vector;
    tags["transform"] = Tag::Transform;
    tags["translate"] = Tag::Translate;
    tags["matrix"] = Tag::Matrix;
    tags["rotate"] = Tag::Rotate;
    tags["scale"] = Tag::Scale;
    tags["lookat"] = Tag::LookAt;
    tags["ref"] = Tag::NamedReference;
    tags["rgb"] = Tag::RGB;
    tags["include"] = Tag::Include;
    tags["alias"] = Tag::Alias;
    tags["default"] = Tag::Default;
    tags["scene"] = Tag::Object;
    tags["integrator"] = Tag::Object;
    tags["camera"] = Tag::Object;
    tags["film"] = Tag::Object;
    tags["sampler"] = Tag::Object;
    tags["shape"] = Tag::Object;
    tags["bsdf"] = Tag::Object;
    tags["light"] = Tag::Object;
    tags["texture"] = Tag::Object;
    tag_alias["scene"] = "Scene";
    tag_alias["integrator"] = "Integrator";
    tag_alias["camera"] = "Camera";
    tag_alias["film"] = "Film";
    tag_alias["sampler"] = "Sampler";
    tag_alias["shape"] = "Shape";
    tag_alias["bsdf"] = "BSDF";
    tag_alias["light"] = "Light";
    tag_alias["texture"] = "Texture";
  });
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
    if (node.type() == pugi::node_comment || node.type() == pugi::node_declaration)
      return {"", ""};
    if (node.type() != pugi::node_element)
      src.throw_error(node, "unexpected content");
    auto it = tags.find(node.name());
    if (it == tags.end()) src.throw_error(node, R"(unexpected tag "{}")", node.name());
    Tag tag = it->second;
    bool has_parent = parent_tag != Tag::Invalid;
    bool parent_is_object = has_parent && parent_tag == Tag::Object;
    bool current_is_object = tag == Tag::Object;
    bool parent_is_transform = parent_tag == Tag::Transform;
    bool current_is_transform_op = tag == Tag::Translate || tag == Tag::Rotate ||
                                   tag == Tag::Scale || tag == Tag::LookAt ||
                                   tag == Tag::Matrix;
    if (!has_parent && !current_is_object) src.throw_error(node, R"(root element "{}" must be an object)", node.name());
    if (parent_is_transform != current_is_transform_op) {
      if (parent_is_transform)
        src.throw_error(node, "transform nodes can only contain transform operations");
      else
        src.throw_error(node, "transform operations can only occur in a transform node");
    }
    if (has_parent && !parent_is_object && !(parent_is_transform && current_is_transform_op))
      src.throw_error(node, R"(node "{}" cannot occur as child of a property)", node.name());
    if (std::string(node.name()) == "scene") {
      node.append_attribute("type") = "scene";
    } else if (tag == Tag::Transform)
      ctx.transform = Transform4();
    if (node.attribute("name")) {
      auto name = node.attribute("name").value();
      if (string::starts_with(name, "_"))
        src.throw_error(
            node, R"(invalid parameter name "{}" in element "{}": leading underscores are reserved for internal identifiers.)", name, node.name());
    } else if (current_is_object || tag == Tag::NamedReference) {
      node.append_attribute("name") = fmt::format("_arg_{}", arg_counter++).c_str();
    }
    if (node.attribute("id")) {
      auto id = node.attribute("id").value();
      if (string::starts_with(id, "_"))
        src.throw_error(
            node, R"(invalid id "{}" in element "{}": leading underscores are reserved for internal identifiers.)", id, node.name());
    } else if (current_is_object) {
      node.append_attribute("id") = fmt::format("_unnamed_{}", ctx.id_counter++).c_str();
    }
    switch (tag) {
      case Tag::Object: {
        check_attributes(src, node, {"type", "id", "name"});
        std::string id = node.attribute("id").value(),
                    name = node.attribute("name").value(),
                    type = node.attribute("type").value(),
                    node_name = node.name();
        Properties props_nested(type);
        props_nested.set_id(id);
        auto it_inst = ctx.instances.find(id);
        if (it_inst != ctx.instances.end())
          src.throw_error(node, "\"{}\" has duplicate id \"{}\" (previous was at {})",
                          node_name, id, src.offset(it_inst->second.location));
        auto it2 = tag_alias.find(node_name);
        if (it2 == tag_alias.end())
          src.throw_error(node,
                          "could not retrieve class object for "
                          "tag \"{}\"",
                          node_name);
        size_t arg_counter_nested = 0;
        for (pugi::xml_node &ch : node.children()) {
          auto [arg_name, nested_id] =
              parse_xml(src, ctx, ch, tag, props_nested, param,
                        arg_counter_nested, depth + 1);
          if (!nested_id.empty())
            props_nested.set_named_reference(arg_name, nested_id);
        }
        auto &inst = ctx.instances[id];
        inst.props = props_nested;
        inst.type = it2->second;
        inst.offset = src.offset;
        inst.src_id = src.id;
        inst.location = node.offset_debug();
        return {name, id};
      } break;
      case Tag::NamedReference: {
        check_attributes(src, node, {"name", "id"});
        auto id = node.attribute("id").value();
        auto name = node.attribute("name").value();
        return std::make_pair(name, id);
      } break;
      case Tag::String: {
        check_attributes(src, node, {"name", "value"});
        props.set_string(node.attribute("name").value(), node.attribute("value").value());
      } break;
      case Tag::Float: {
        check_attributes(src, node, {"name", "value"});
        std::string value = node.attribute("value").value();
        Float value_float;
        try {
          value_float = detail::stof(value);
        } catch (...) {
          src.throw_error(node, "could not parse floating point value \"%s\"", value);
        }
        props.set_float(node.attribute("name").value(), value_float);
      } break;
      case Tag::Integer: {
        check_attributes(src, node, {"name", "value"});
        std::string value = node.attribute("value").value();
        int64_t value_long;
        try {
          value_long = detail::stoll(value);
        } catch (...) {
          src.throw_error(node, "could not parse integer value \"%s\"", value);
        }
        props.set_int(node.attribute("name").value(), value_long);
      } break;
      case Tag::Vector: {
        detail::expand_value_to_xyz(src, node);
        check_attributes(src, node, {"name", "x", "y", "z"});
        props.set_vector3(node.attribute("name").value(),
                          detail::parse_vector(src, node));
      } break;
      case Tag::Matrix: {
        check_attributes(src, node, {"value"});
        auto tokens = string::tokenize(node.attribute("value").value(), " ");
        if (tokens.size() != 16) Throw("matrix: expected 16 values");
        Matrix4 matrix;
        for (int i = 0; i < 4; ++i) {
          for (int j = 0; j < 4; ++j) {
            try {
              matrix(i, j) = detail::stof(tokens[i * 4 + j]);
            } catch (...) {
              src.throw_error(node, "could not parse floating point value \"{}\"",
                              tokens[i * 4 + j]);
            }
          }
        }
        ctx.transform = Transform4(matrix) * ctx.transform;
        break;
      }
      case Tag::RGB: {
        check_attributes(src, node, {"name", "value"});
        std::vector<std::string> tokens = string::tokenize(node.attribute("value").value());

        if (tokens.size() == 1) {
          tokens.push_back(tokens[0]);
          tokens.push_back(tokens[0]);
        }
        if (tokens.size() != 3)
          src.throw_error(node, "'rgb' tag requires one or three values (got \"%s\")",
                          node.attribute("value").value());

        Color3 color;
        try {
          color = Color3(detail::stof(tokens[0]),
                         detail::stof(tokens[1]),
                         detail::stof(tokens[2]));
        } catch (...) {
          src.throw_error(node, "could not parse RGB value \"%s\"", node.attribute("value").value());
        }
        Properties props2("srgb");
        props2.set_color("color", color);
        auto obj = PluginManager::instance()->create_comp(props2, refl::type::get_by_name("Texture"));
        props.set_component(node.attribute("name").value(), obj);
      } break;
      case Tag::Transform: {
        check_attributes(src, node, {"name"});
        ctx.transform = Transform4();
      } break;
      case Tag::LookAt: {
        check_attributes(src, node, {"origin", "target", "up"});

        auto origin = parse_named_vector(src, node, "origin");
        auto target = parse_named_vector(src, node, "target");
        auto up = parse_named_vector(src, node, "up");

        auto result = Transform4::lookat(origin, target, up);
        if (result.matrix().hasNaN())
          src.throw_error(node, "invalid lookat transformation");
        ctx.transform = result * ctx.transform;
      } break;
      case Tag::Translate: {
        detail::expand_value_to_xyz(src, node);
        check_attributes(src, node, {"x", "y", "z"}, false);
        auto vec = detail::parse_vector(src, node);
        ctx.transform = Transform4::translate(vec) * ctx.transform;
      } break;
      case Tag::Scale: {
        detail::expand_value_to_xyz(src, node);
        check_attributes(src, node, {"x", "y", "z"}, false);
        auto vec = detail::parse_vector(src, node, 1.f);
        ctx.transform = Transform4::scale(vec) * ctx.transform;
      } break;
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
  return {"", ""};
}

static std::shared_ptr<Component> instantiate_node(XMLParseContext &ctx, const std::string &id) {
  auto it = ctx.instances.find(id);
  if (it == ctx.instances.end())
    Throw("reference to unknown object \"{}\"!", id);
  auto &inst = it->second;
  if (inst.comp) {
    return inst.comp;
  }
  Properties &props = inst.props;
  const auto &named_references = props.named_references();
  for (auto &kv : named_references) {
    try {
      auto obj = instantiate_node(ctx, kv.second);
      props.set_component(kv.first, obj, false);
    } catch (const std::exception &e) {
      if (strstr(e.what(), "Error while loading") == nullptr)
        Throw("Error while loading \"{}\" (near {}): {}",
              inst.src_id, inst.offset(inst.location), e.what());
      else
        throw;
    }
  }
  try {
    inst.comp = PluginManager::instance()->create_comp(props, refl::type::get_by_name(inst.type));
  } catch (const std::exception &e) {
    Throw(
        "Error while loading \"{}\" (near {}): could not instantiate "
        "{} plugin of type \"{}\": {}",
        inst.src_id, inst.offset(inst.location),
        string::to_lower(inst.type), props.plugin_name(),
        e.what());
  }
  return inst.comp;
}

}  // namespace detail

std::shared_ptr<Component> load_file(const fs::path &filename, ParameterList parameters) {
  if (!fs::exists(filename)) {
    Throw(R"("{}": file not exists.)", filename.string());
  }
  Log(Info, R"(Loading XML file "{}" ..)", filename.string());
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file(filename.native().c_str(), pugi::parse_default | pugi::parse_comments);

  detail::XMLSource src{
      filename.string(), doc,
      [=](ptrdiff_t pos) { return detail::file_offset(filename, pos); }};

  if (!result) {
    Throw(R"(Error while loading "{}" (at {}): {})", src.id, src.offset(result.offset), result.description());
  }

  pugi::xml_node root = doc.document_element();

  detail::XMLParseContext ctx;
  Properties props;
  size_t arg_counter = 0;  // Unused
  auto [name, id] = detail::parse_xml(src, ctx, root, Tag::Invalid, props, parameters, arg_counter, 0);
  return detail::instantiate_node(ctx, id);
}

}  // namespace misaki::render::xml