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
  Properties properties;
  std::string src_id;
  std::function<std::string(ptrdiff_t)> offset;
  std::shared_ptr<Component> comp;
};

struct XMLParseContext {
  std::unordered_map<std::string, XMLObject> instances;
  Transform4 transform;
  size_t id_counter;
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

static std::pair<std::string, std::string> parse_xml(XMLSource &src, XMLParseContext &ctx,
                                                     pugi::xml_node &node, Tag parent_tag,
                                                     Properties &props, ParameterList &param,
                                                     size_t &arg_counter, int depth) {
  try {
    if (!param.empty()) {
      for (auto attr : node.attributes()) {
        std::string value = attr.value();

      }
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }
  return { "", "" };
}

static std::shared_ptr<Component> instantiate_node(XMLParseContext &ctx, const std::string &id) {
  return {};
}

}  // namespace detail

std::shared_ptr<Component> load_file(const fs::path &filename, ParameterList parameters) {
  if (!fs::exists(filename)) {
    Throw(R"("{}": file not exists.)", filename.string());
  }
  Log(Info, R"(Loading XML file "{}" ..)", filename.string());
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file(filename.native().c_str(), pugi::parse_default | pugi::parse_comments);

  detail::XMLSource src {
    filename.string(), doc,
    [=](ptrdiff_t pos) { return detail::file_offset(filename, pos); }
  };

  if (!result) {
    Throw(R"(Error while loading "{}" (at {}): {})", src.id, src.offset(result.offset), result.description());
  }

  pugi::xml_node root = doc.document_element();

  detail::XMLParseContext ctx;
  Properties props;
  size_t arg_counter = 0; // Unused
  auto [name, id] = detail::parse_xml(src, ctx, root, Tag::Invalid, props, parameters, arg_counter, 0);
  return detail::instantiate_node(ctx, id);
}

}  // namespace misaki::render::xml