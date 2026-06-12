#pragma once

#include "sandbox/core/platform.h"

#include <vector>
#include <string>
#include <cstdint>

struct ecs_world_t;
namespace flecs { struct world; }

namespace sandbox {

    struct activation_request {
        std::string module_name;
        uint8_t major = 0;
        uint8_t minor = 0;
        uint8_t patch = 0;
    };

    class SANDBOX_API engine {
    public:
        engine(const char* json_config, const std::vector<activation_request>& explicit_activations = {});
        ~engine();

        engine(const engine&) = delete;
        engine& operator=(const engine&) = delete;

        engine(engine&&) noexcept;
        engine& operator=(engine&&) noexcept;

        void run();

        void register_static_library(void (*library_entry_point)(ecs_world_t*));

        flecs::world& get_ecs();

    private:
        struct impl;
        impl* m_impl;
    };

}