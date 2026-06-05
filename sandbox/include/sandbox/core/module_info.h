#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
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

    struct module_info {
        std::string name;
        uint8_t version_major{1};
        uint8_t version_minor{0};

        bool is_loaded{false};

        std::string provides_service{""};

        std::function<void(flecs::world& ecs)> import_fn;

        std::vector<requirement> requirements;
    };

    /// Returns the per-library static registry by reference so that
    /// SANDBOX_DECLARE_MODULE registrations accumulate into the same container
    /// across all translation units within a single shared library.
    inline std::vector<module_info>& get_local_registry() {
        static std::vector<module_info> infos;
        return infos;
    }

    /// Constructs a module_info by value, capturing a typed ECS import lambda.
    /// The lambda avoids needing to store TModule directly (it may be incomplete
    /// at the call site) while still producing a callable that Flecs can invoke.
    template<typename TModule>
    inline module_info create_module_info(
        std::string name,
        uint8_t v_major,
        uint8_t v_minor,
        std::vector<requirement> reqs,
        std::string provides_service = "")
    {
        module_info info;
        info.name = std::move(name);
        info.version_major = v_major;
        info.version_minor = v_minor;
        info.provides_service = std::move(provides_service);
        info.requirements = std::move(reqs);

        info.import_fn = [](flecs::world& ecs) {
            ecs.import<TModule>();
        };

        return info;
    }

} // namespace sandbox