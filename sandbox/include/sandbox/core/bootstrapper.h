#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "sandbox/core/ecs.h"
#include "sandbox/core/module_info.h"
#include "sandbox/core/platform.h"

namespace sandbox {

    struct SANDBOX_API bootstrapper {
    public:
        explicit bootstrapper(flecs::world& ecs);
        ~bootstrapper() = default;

        bootstrapper(const bootstrapper&) = delete;
        bootstrapper& operator=(const bootstrapper&) = delete;
        bootstrapper(bootstrapper&&) = default;
        bootstrapper& operator=(bootstrapper&&) = default;

        void stage(const library_registry& registry);

        void activate(const std::string& module_name, uint8_t min_major = 0, uint8_t min_minor = 0);

        void execute(flecs::world& ecs);

    private:
        std::vector<module_info> m_modules;
        std::vector<service_info> m_services;

        struct explicit_activation {
            std::string module_name;
            uint8_t min_major;
            uint8_t min_minor;
        };
        std::vector<explicit_activation> m_explicit_activations;

        // name -> index in m_modules
        std::unordered_map<std::string, std::size_t> m_active_modules;

        struct version_constraint {
            uint8_t min_major;
            uint8_t min_minor;
            std::string requester;
        };
        std::unordered_map<std::string, version_constraint> m_version_constraints;

        void resolve_activations(flecs::world& ecs);
        void audit_service_collisions(flecs::world& ecs);

        bool is_module_loaded(const std::string& name) const;
        bool is_service_loaded(const std::string& service_name) const;
        bool all_activated_modules_loaded() const;
        bool is_service_active(const std::string& service_name) const;

        std::size_t find_best_service_provider(
            flecs::world& ecs,
            const std::string& service_name,
            uint8_t min_major,
            uint8_t min_minor) const;

        std::size_t find_best_module_version(
            const std::string& module_name,
            uint8_t min_major,
            uint8_t min_minor) const;
    };

} // namespace sandbox