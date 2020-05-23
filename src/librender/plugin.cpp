#include <misaki/render/plugin.h>
#include <misaki/render/logger.h>
#include <misaki/render/properties.h>
#include <misaki/utils/system/shared_library.h>
#include <unordered_map>
#include <mutex>

namespace misaki::render {

struct Plugin {
  Plugin(const fs::path &path) {
    library = std::make_unique<system::SharedLibrary>(path.string());
    using StringFunc = const char *(*)();
    try {
      plugin_name = ((StringFunc) library->load_symbol("plugin_name"))();
    } catch (std::exception &e) {
      library.release();
      throw std::runtime_error(fmt::format("Error loading plugin: {}", e.what()));
    }
  }
  const char* plugin_name;
  std::unique_ptr<system::SharedLibrary> library;
};

struct PluginManager::PluginManagerPrivate {
  std::unordered_map<std::string, Plugin*> plugins;
  std::mutex mutex;

  Plugin *plugin(const std::string &name) {
    std::lock_guard<std::mutex> guard(mutex);
    auto it = plugins.find(name);
    if (it != plugins.end()) {
      return it->second;
    }
    auto filename = fs::path("plugins") / name;
#if defined(MSK_PLATFORM_WINDOWS)
    filename.replace_extension(".dll");
#elif defined(MSK_PLATFORM_APPLE)
    filename.replace_extension(".dylib");
#else
    filename.replace_extension(".so");
#endif
    auto resolver = get_file_resolver();
    auto resolved = resolver->resolve(filename);
    if (fs::exists(resolved)) {
      Log(Info, R"(Loading plugin "{}" ..)", filename.string());
      Plugin *plugin = new Plugin(filename);
      plugins[name] = plugin;
      return plugin;
    }
    Throw("Plugin {} not found", name.c_str());
  }
};

PluginManager::PluginManager() : d(new PluginManagerPrivate()){}

PluginManager::~PluginManager() {
  std::lock_guard<std::mutex> guard(d->mutex);
  for (auto &pair : d->plugins) delete pair.second;
}

std::shared_ptr<Component> PluginManager::create_comp(const Properties &props) {
  const Plugin *plugin = d->plugin(props.plugin_name());
  auto type = refl::type::get_by_name(plugin->plugin_name);
  // Needs to check whether type exists
  auto instanced_type = type.create({props}).get_value<std::shared_ptr<void>>();
  auto comp = std::reinterpret_pointer_cast<Component>(instanced_type);
  return comp;
}

std::shared_ptr<Component> PluginManager::create_comp(const Properties &props, const refl::type &type) {
  const Plugin *plugin = d->plugin(props.plugin_name());
  auto plugin_type = refl::type::get_by_name(plugin->plugin_name);
  // Needs to check whether type exists
  if (!plugin_type.is_derived_from(type)) {
    auto base_type = *type.get_base_classes().end();
    Throw(R"(Type mismatch when create component "{}": Expected an instance of type "{}", got an instance of type "{}".)",
          props.plugin_name(), type.get_name().to_string(), base_type.get_name().to_string());
  }
  auto instanced_type = plugin_type.create({props}).get_value<std::shared_ptr<void>>();
  auto comp = std::reinterpret_pointer_cast<Component>(instanced_type);
  return comp;
}
}