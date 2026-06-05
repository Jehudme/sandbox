#pragma once

#include <string>
#include <vector>
#include <unordered_set>

#include "sandbox/core/ecs.h"
#include "sandbox/core/module_info.h"
#include "sandbox/core/platform.h"

namespace sandbox {

    /// Orchestrates ordered module loading by resolving inter-module dependencies
    /// and service requirements before importing into the ECS world.
    struct SANDBOX_API bootstrapper {
    public:
        explicit bootstrapper(flecs::world& ecs);
        ~bootstrapper() = default;

        bootstrapper(const bootstrapper&) = delete;
        bootstrapper& operator=(const bootstrapper&) = delete;
        bootstrapper(bootstrapper&&) = default;
        bootstrapper& operator=(bootstrapper&&) = default;

        /// Appends module descriptors from a loaded library into the staging list.
        void stage(const std::vector<module_info>& info);

        /// Queues a module name for activation (must match a staged module's name).
        /// @return true if a matching staged module was found; false otherwise.
        bool activate(const std::string& module_name);

        /// Resolves the full dependency tree and imports all activated modules in order.
        /// Throws std::runtime_error on deadlock or unsatisfied required services.
        void execute(flecs::world& ecs);

    private:
        std::vector<module_info>     m_modules;
        std::vector<std::string>     m_explicit_activations;
        std::unordered_set<std::string> m_active_module_names;

        /// Expands m_active_module_names by cascading all required dependencies.
        void resolve_activations();

        bool is_service_provided_by_active_modules(const std::string& service_name) const;
        module_info* find_dormant_service_provider(const std::string& service_name, uint8_t min_major, uint8_t min_minor);
        bool is_module_loaded(const std::string& name) const;
        bool is_service_loaded(const std::string& service_name) const;
        bool all_activated_modules_loaded() const;
    };

} // namespace sandbox