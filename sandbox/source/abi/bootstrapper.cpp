#include "sandbox/abi/bootstrapper.h"
#include "core/bootstrapper.h"
#include <exception>

ECS_COMPONENT_DECLARE(sandbox_bootstrapper_component_t);

bool sandbox_stage_service(const sandbox_service_info_t* info) {
    if (!info) return false;
    try {
        sandbox::core::bootstrapper_t::stage_service(*info);
        return true;
    } catch (...) {
        return false;
    }
}

bool sandbox_stage_module(const sandbox_module_info_t* info) {
    if (!info) return false;
    try {
        sandbox::core::bootstrapper_t::stage_module(*info);
        return true;
    } catch (...) {
        return false;
    }
}

void sandbox_index_library(const char* library_path) {
    if (library_path) {
        sandbox::core::bootstrapper_t::index_library(library_path);
    }
}

sandbox_bootstrapper_t* sandbox_get_bootstrapper(ecs_world_t* ecs) {
    if (!ecs) return nullptr;
    const sandbox_bootstrapper_component_t* comp = (const sandbox_bootstrapper_component_t*)ecs_singleton_get(ecs, sandbox_bootstrapper_component_t);
    if (!comp) return nullptr;
    return comp->internal_bootstrapper;
}

bool sandbox_bootstrapper_activate(sandbox_bootstrapper_t* bootstrapper, const char* architecture, const char* name, int version_major, int version_minor, int version_patch) {
    if (!bootstrapper || !architecture || !name) return false;
    try {
        reinterpret_cast<sandbox::core::bootstrapper_t*>(bootstrapper)->activate(architecture, name, version_major, version_minor, version_patch);
        return true;
    } catch (...) {
        // ABI boundary safely swallows exceptions to prevent C-plugin crashes
        return false;
    }
}

bool sandbox_bootstrapper_activate_string(sandbox_bootstrapper_t* bootstrapper, const char* module_str) {
    if (!bootstrapper || !module_str) return false;
    try {
        reinterpret_cast<sandbox::core::bootstrapper_t*>(bootstrapper)->activate(module_str);
        return true;
    } catch (...) {
        // Catch exceptions across ABI
        return false;
    }
}

bool sandbox_bootstrapper_boot(sandbox_bootstrapper_t* bootstrapper, ecs_world_t* ecs) {
    if (!bootstrapper || !ecs) return false;
    try {
        flecs::world world(ecs);
        reinterpret_cast<sandbox::core::bootstrapper_t*>(bootstrapper)->boot(world);
        return true;
    } catch (...) {
        return false;
    }
}
