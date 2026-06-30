#include "engine.h"

#include <flecs/addons/cpp/flecs.hpp>

#include "bootstrapper.h"
#include "sandbox/abi/bootstrapper.h"
#include "sandbox/abi/handle.h"
#include "sandbox/abi/properties.h"
#include "exceptions.h"
#include <sandbox/sdk/logs.hpp>

namespace sandbox::core {
    engine_t::engine_t() = default;
    engine_t::~engine_t() = default;

    void engine_t::initialize(properties_t& properties) {
        ecs.reset();
        
        m_arguments = std::make_unique<properties_t>(properties);
        m_bootstrapper = std::make_unique<bootstrapper_t>();

        sandbox_properties_handle_t properties_handle;
        properties_handle.token = reinterpret_cast<uintptr_t>(m_arguments.get());
        ecs.entity("::sandbox::configuration::handle").set<uint64_t>(properties_handle.token);

        save_bootstrapper();

        boot();
    }



    void engine_t::save_bootstrapper() {
        ECS_COMPONENT_DEFINE(ecs.c_ptr(), sandbox_bootstrapper_component_t);
        sandbox_bootstrapper_component_t boot_comp;
        boot_comp.internal_bootstrapper = reinterpret_cast<sandbox_bootstrapper_t*>(m_bootstrapper.get());
        ecs_set_id(ecs.c_ptr(), ecs_id(sandbox_bootstrapper_component_t), ecs_id(sandbox_bootstrapper_component_t), sizeof(sandbox_bootstrapper_component_t), &boot_comp);
    }

    void engine_t::boot() {
        if (!m_arguments || !m_bootstrapper) return;
        
        // 1. Index libraries
        if (m_arguments->has({"engine", "libraries"})) {
            if (auto libs = m_arguments->get<std::vector<std::string>>({"engine", "libraries"})) {
                for (const auto& lib : *libs) {
                    try {
                        bootstrapper_t::index_library(ecs, lib);
                    } catch (const library_load_error& e) {
                        sandbox::modules::logs::warn(ecs, "Library load error skipped: {}", e.what());
                    } catch (const std::exception& e) {
                        sandbox::modules::logs::error(ecs, "Unexpected error indexing library '{}': {}", lib, e.what());
                    }
                }
            }
        }
        
        // 2. Activate sandbox
        if (m_arguments->has({"engine", "sandbox"})) {
            if (auto mods = m_arguments->get<std::vector<std::string>>({"engine", "sandbox"})) {
                for (const auto& mod : *mods) {
                    try {
                        m_bootstrapper->activate(ecs, mod);
                    } catch (const module_activation_error& e) {
                        sandbox::modules::logs::warn(ecs, "Failed to activate module '{}': {}", mod, e.what());
                    } catch (const std::exception& e) {
                        sandbox::modules::logs::error(ecs, "Unexpected error activating module '{}': {}", mod, e.what());
                    }
                }
            }
        }
        
        m_bootstrapper->boot(ecs);
    }
}
