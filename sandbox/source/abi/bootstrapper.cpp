#include "sandbox/abi/bootstrapper.h"
#include "core/bootstrapper.h"
#include <exception>
#include <iostream>
#include <flecs/addons/cpp/flecs.hpp>

ECS_COMPONENT_DECLARE(sandbox_bootstrapper_component_t);

bool sandbox_stage_service(const sandbox_service_info_t *info) {
  if (!info)
    return false;
  try {
    sandbox::core::bootstrapper_t::stage_service(*info);
    return true;
  } catch (const std::exception& e) {
    std::cerr << "sandbox_stage_service error: " << e.what() << "\n";
    return false;
  } catch (...) {
    std::cerr << "sandbox_stage_service unknown error\n";
    return false;
  }
}

bool sandbox_stage_module(const sandbox_module_info_t *info) {
  if (!info)
    return false;
  try {
    sandbox::core::bootstrapper_t::stage_module(*info);
    return true;
  } catch (const std::exception& e) {
    std::cerr << "sandbox_stage_module error: " << e.what() << "\n";
    return false;
  } catch (...) {
    std::cerr << "sandbox_stage_module unknown error\n";
    return false;
  }
}

void sandbox_load_library(ecs_world_t *ecs, const char *library_path) {
  if (ecs && library_path) {
    try {
      flecs::world world(ecs);
      sandbox::core::bootstrapper_t::load_library(world, library_path);
    } catch (const std::exception& e) {
      std::cerr << "sandbox_load_library error: " << e.what() << "\n";
    } catch (...) {
      std::cerr << "sandbox_load_library unknown error\n";
    }
  }
}

sandbox_bootstrapper_t *sandbox_get_bootstrapper(ecs_world_t *ecs) {
  if (!ecs)
    return nullptr;
  const sandbox_bootstrapper_component_t *comp =
      (const sandbox_bootstrapper_component_t *)ecs_singleton_get(
          ecs, sandbox_bootstrapper_component_t);
  if (!comp)
    return nullptr;
  return comp->internal_bootstrapper;
}

bool sandbox_bootstrapper_activate(sandbox_bootstrapper_t *bootstrapper,
                                   ecs_world_t *ecs, const char *architecture,
                                   const char *name, int version_major,
                                   int version_minor, int version_patch) {
  if (!bootstrapper || !ecs || !architecture || !name)
    return false;
  try {
    flecs::world world(ecs);
    reinterpret_cast<sandbox::core::bootstrapper_t *>(bootstrapper)
        ->activate(world, architecture, name, version_major, version_minor,
                   version_patch);
    return true;
  } catch (const std::exception& e) {
    std::cerr << "sandbox_bootstrapper_activate error: " << e.what() << "\n";
    return false;
  } catch (...) {
    std::cerr << "sandbox_bootstrapper_activate unknown error\n";
    return false;
  }
}

bool sandbox_bootstrapper_activate_string(sandbox_bootstrapper_t *bootstrapper,
                                          ecs_world_t *ecs,
                                          const char *module_str) {
  if (!bootstrapper || !ecs || !module_str)
    return false;
  try {
    flecs::world world(ecs);
    reinterpret_cast<sandbox::core::bootstrapper_t *>(bootstrapper)
        ->activate(world, module_str);
    return true;
  } catch (const std::exception& e) {
    std::cerr << "sandbox_bootstrapper_activate_string error: " << e.what() << "\n";
    return false;
  } catch (...) {
    std::cerr << "sandbox_bootstrapper_activate_string unknown error\n";
    return false;
  }
}

bool sandbox_bootstrapper_boot(sandbox_bootstrapper_t *bootstrapper,
                               ecs_world_t *ecs) {
  if (!bootstrapper || !ecs)
    return false;
  try {
    flecs::world world(ecs);
    reinterpret_cast<sandbox::core::bootstrapper_t *>(bootstrapper)
        ->boot(world);
    return true;
  } catch (const std::exception& e) {
    std::cerr << "sandbox_bootstrapper_boot error: " << e.what() << "\n";
    return false;
  } catch (...) {
    std::cerr << "sandbox_bootstrapper_boot unknown error\n";
    return false;
  }
}
