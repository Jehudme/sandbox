#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <expected>
#include "sandbox/core/ecs.h"

namespace sandbox {

    struct requirement {
        enum class kind { service, module };
        enum class strictness { require, expect };

        kind target_kind;
        strictness policy;
        std::string target_name;
        uint8_t min_major{1};
        uint8_t min_minor{0};
    };

    struct service_info {
        std::string name;
        uint8_t version_major{1};
        uint8_t version_minor{0};
    };

    struct module_info {
        std::string name;
        uint8_t version_major{1};
        uint8_t version_minor{0};
        uint8_t version_patch{0};

        bool is_loaded{false};

        std::string provides_service{""};

        std::function<std::expected<void, std::string>(flecs::world& ecs)> import_fn;

        std::vector<requirement> requirements;
    };

    struct library_registry {
        std::vector<service_info> services;
        std::vector<module_info> modules;
    };

    /// Returns the per-library static registry by reference
    inline library_registry& get_local_registry() {
        static library_registry registry;
        return registry;
    }

    template<typename TModule>
    inline module_info create_module_info(
        std::string name,
        uint8_t v_major,
        uint8_t v_minor,
        uint8_t v_patch,
        std::vector<requirement> reqs,
        std::string provides_service = "")
    {
        module_info info;
        info.name = std::move(name);
        info.version_major = v_major;
        info.version_minor = v_minor;
        info.version_patch = v_patch;
        info.provides_service = std::move(provides_service);
        info.requirements = std::move(reqs);

        info.import_fn = [](flecs::world& ecs) -> std::expected<void, std::string> {
            try {
                ecs.import<TModule>();
                return {};
            } catch (const std::exception& e) {
                return std::unexpected(e.what());
            } catch (...) {
                return std::unexpected("Unknown exception occurred during module import.");
            }
        };

        return info;
    }

    inline service_info create_service_info(
        std::string name,
        uint8_t v_major,
        uint8_t v_minor)
    {
        service_info info;
        info.name = std::move(name);
        info.version_major = v_major;
        info.version_minor = v_minor;
        return info;
    }

} // namespace sandbox