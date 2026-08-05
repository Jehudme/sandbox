#include "engine.h"

#include <flecs/addons/cpp/flecs.hpp>

#include "bootstrapper.h"
#include "exceptions.h"
#include "sandbox/abi/bootstrapper.h"
#include "sandbox/abi/handle.h"
#include "sandbox/abi/properties.h"
#include <iostream>
#include <sandbox/sdk/logs.hpp>

extern "C" void sandbox_register_library_loader(ecs_world_t *ecs);

namespace sandbox::core {
engine_t::engine_t() = default;
engine_t::~engine_t() = default;

void engine_t::initialize(properties_t &properties) {
  entity_world.reset();

  m_arguments = std::make_unique<properties_t>(properties);
  m_bootstrapper = std::make_unique<bootstrapper_t>();

  sandbox_properties_handle_t properties_handle;
  properties_handle.token = reinterpret_cast<uintptr_t>(m_arguments.get());

  // Refactor Path for "booting-configuration/handle"
  entity_world.entity("::sandbox::configuration::handle")
      .set<uint64_t>(properties_handle.token);

  sandbox_register_library_loader(entity_world.c_ptr());

  register_bootstrapper();

  boot();
}

void engine_t::register_bootstrapper() {
  ECS_COMPONENT_DEFINE(entity_world.c_ptr(), sandbox_bootstrapper_component_t);
  sandbox_bootstrapper_component_t bootstrapper_component;
  bootstrapper_component.internal_bootstrapper =
      reinterpret_cast<sandbox_bootstrapper_t *>(m_bootstrapper.get());
  ecs_set_id(entity_world.c_ptr(), ecs_id(sandbox_bootstrapper_component_t),
             ecs_id(sandbox_bootstrapper_component_t),
             sizeof(sandbox_bootstrapper_component_t), &bootstrapper_component);
}

void engine_t::boot() {
  if (!m_arguments || !m_bootstrapper) {
    std::cerr << "Engine arguments or bootstrapper is null\n";
    sandbox::modules::logs::error(entity_world, "Engine arguments or bootstrapper is null");
    return;
  }

  if (m_arguments->has({"booting-configuration", "libraries"})) {
    if (auto libraries = m_arguments->get<std::vector<std::string>>(
            {"booting-configuration", "libraries"})) {
      for (const auto &library_path : *libraries) {
        try {
          bootstrapper_t::load_library(entity_world, library_path);
        } catch (const library_load_error &e) {
          std::cerr << "Library load error skipped: " << e.what() << "\n";
          sandbox::modules::logs::warn(
              entity_world, "Library load error skipped: {}", e.what());
        } catch (const std::exception &e) {
          std::cerr << "Unexpected error indexing library '" << library_path
                    << "': " << e.what() << "\n";
          sandbox::modules::logs::error(
              entity_world, "Unexpected error indexing library '{}': {}",
              library_path, e.what());
        }
      }
    }
  }

  if (m_arguments->has({"booting-configuration", "modules"})) {
    if (auto modules =
            m_arguments->get<std::vector<std::string>>({"booting-configuration", "modules"})) {
      for (const auto &module_name : *modules) {
        try {
          m_bootstrapper->activate(entity_world, module_name);
        } catch (const module_activation_error &e) {
          std::cerr << "Failed to activate module '" << module_name
                    << "': " << e.what() << "\n";
          sandbox::modules::logs::warn(entity_world,
                                       "Failed to activate module '{}': {}",
                                       module_name, e.what());
        } catch (const std::exception &e) {
          std::cerr << "Unexpected error activating module '" << module_name
                    << "': " << e.what() << "\n";
          sandbox::modules::logs::error(
              entity_world, "Unexpected error activating module '{}': {}",
              module_name, e.what());
        }
      }
    }
  }

  try {
    m_bootstrapper->boot(entity_world);
  } catch (const std::exception &e) {
    std::cerr << "Failed to boot sandbox: " << e.what() << "\n";
    sandbox::modules::logs::error(entity_world, "Failed to boot sandbox: {}",
                                  e.what());
  }
}
} // namespace sandbox::core
