#include "application_module.h"

#include "sandbox/sdk/bootstrapper.hpp"
#include "sandbox/sdk/library_loader.hpp"
#include <sandbox/sdk/configuration.hpp>
#include <sandbox/sdk/filesystem.hpp>
#include <sandbox/sdk/logs.hpp>

#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// 1. Define the native shared library extension based on the compilation target
#if defined(_WIN32)
inline constexpr std::string_view NATIVE_LIB_EXTENSION = ".dll";
#elif defined(__APPLE__)
inline constexpr std::string_view NATIVE_LIB_EXTENSION = ".dylib";
#else
inline constexpr std::string_view NATIVE_LIB_EXTENSION =
    ".so"; // Linux / BSD / WebAssembly
#endif

#include <sandbox/sdk/application.hpp>

namespace sandbox::modules {

// Module requirements
static sandbox_requirement_info_t application_requirements[] = {
    {SANDBOX_REQUIREMENT_KIND_MODULE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
     "filesystem", "sandbox", 1, 0, -1},
    {SANDBOX_REQUIREMENT_KIND_MODULE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
     "configuration", "sandbox", 1, 0, -1},
    {SANDBOX_REQUIREMENT_KIND_MODULE, SANDBOX_REQUIREMENT_STRICTNESS_REQUIRED,
     "logs", "sandbox", 1, 0, -1}};

void stage_filesystem(flecs::world &ecs);
void orchestrate_modules(flecs::world &ecs);
void ingest_configuration(flecs::world &ecs);

// OS Path Resolution Helpers
static fs::path get_os_temp_workspace(const std::string &app_name) {
  return fs::temp_directory_path() / app_name;
}

static fs::path get_os_appdata_workspace(const std::string &app_name) {
  fs::path base_path;

#if defined(_WIN32)
  const char *local_app = std::getenv("LOCALAPPDATA");
  base_path = local_app
                  ? fs::path(local_app)
                  : fs::path(std::getenv("USERPROFILE")) / "AppData" / "Local";
#elif defined(__APPLE__)
  const char *home = std::getenv("HOME");
  base_path = home ? fs::path(home) / "Library" / "Application Support"
                   : fs::temp_directory_path();
#elif defined(__EMSCRIPTEN__)
  base_path = "/data";
#else
  const char *xdg_data = std::getenv("XDG_DATA_HOME");
  if (xdg_data && *xdg_data != '\0') {
    base_path = fs::path(xdg_data);
  } else {
    const char *home = std::getenv("HOME");
    base_path =
        home ? fs::path(home) / ".local" / "share" : fs::temp_directory_path();
  }
#endif
  return base_path / app_name;
}

application_t::application_t(flecs::world &ecs) {
  try {
    sandbox::modules::logs::info(ecs, "Starting Application Module");
    stage_filesystem(ecs);
    ingest_configuration(ecs);
    orchestrate_modules(ecs);

    std::vector<std::string> modules;
    sandbox::bootstrapper bootstrapper(ecs);
    sandbox::modules::configuration::get_properties(ecs).get_array<std::string>("engine/sandbox", modules);

    for (const auto& module : modules) {
      bootstrapper.activate(module);
    }

    sandbox::modules::logs::info(ecs, "Application Module Initialized");
  } catch (const std::exception &e) {
    sandbox::modules::logs::error(ecs, "Application Error: {}", e.what());
    throw;
  }
}

void stage_filesystem(flecs::world &ecs) {
  const std::string engine_namespace = "SandboxEngine";

  fs::path cache_path =
      get_os_temp_workspace(engine_namespace) / "plugin_cache";
  fs::path saves_path = get_os_appdata_workspace(engine_namespace) / "saves";

  fs::create_directories(cache_path);
  fs::create_directories(saves_path);

  sandbox::modules::filesystem::mount(ecs, cache_path.string().c_str(),
                                      "cache://", false);
  sandbox::modules::filesystem::mount(ecs, saves_path.string().c_str(),
                                      "save://", false);
}

void orchestrate_modules(flecs::world &ecs) {
  try {
    std::vector<std::string> plugins_paths =
        sandbox::modules::filesystem::list_files(ecs, "app://plugins");

    for (const std::string &plugin_path : plugins_paths) {
      std::filesystem::path path(plugin_path);
      if (path.extension() == NATIVE_LIB_EXTENSION) {
        try {
          sandbox::modules::logs::info(ecs, "Loading plugin: {}",
                                       path.string());
          auto library_data = sandbox::modules::filesystem::read_all_bytes(
              ecs, path.string().c_str());
          sandbox::core::library_loader::load_from_memory(ecs, library_data);
        } catch (const std::exception &e) {
          sandbox::modules::logs::error(ecs, "Failed to load plugin {}: {}",
                                        path.string(), e.what());
        }
      }
    }
  } catch (const std::exception &e) {
    // plugins folder doesn't exist or is invalid
    sandbox::modules::logs::info(
        ecs, "No plugins loaded (plugins directory not found or error): {}",
        e.what());
  }
}

void ingest_configuration(flecs::world &ecs) {
  try {
    sandbox::modules::logs::trace(
        ecs, "Checking if app://configuration.json exists...");
    if (sandbox::modules::filesystem::exists(ecs, "app://configuration.json")) {
      sandbox::modules::logs::trace(ecs, "Reading app://configuration.json...");
      std::string content = sandbox::modules::filesystem::read_all_text(
          ecs, "app://configuration.json");
      sandbox::modules::logs::trace(ecs, "Read {} bytes. Parsing properties...",
                                    content.size());
      if (!content.empty()) {
        sandbox::properties other(content, sandbox::properties::Format::JSON);
        sandbox::modules::logs::trace(ecs, "Properties parsed. Merging...");
        sandbox::modules::configuration::get_properties(ecs).merge("", other);
        sandbox::modules::logs::info(ecs, "Ingested app://configuration.json");
      }
    } else {
      sandbox::modules::logs::trace(ecs,
                                    "app://configuration.json does not exist.");
    }
  } catch (const std::exception &e) {
    sandbox::modules::logs::error(ecs, "Failed to ingest configuration: {}",
                                  e.what());
  }
}

SANDBOX_DECLARE_MODULE(
    application_t, {.name = "application",
                    .description = "Application Module",
                    .architecture = "sandbox",
                    .version_major = 1,
                    .version_minor = 0,
                    .version_patch = 0,
                    .service = &sandbox_application_service_t_info,
                    .requirements = application_requirements,
                    .requirement_count = sizeof(application_requirements) /
                                         sizeof(sandbox_requirement_info_t)})
} // namespace sandbox::modules
