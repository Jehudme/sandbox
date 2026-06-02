#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>

#include "arguments.h"
#include "platform.h"

#include "sandbox/core/ecs.h"
#include "sandbox/utilities/properties.h"

namespace sandbox {

    class SANDBOX_API engine {
    public:
        engine();
        ~engine();

        struct arguments;

        engine(const engine&) = delete;
        engine& operator=(const engine&) = delete;

        engine(engine&&) noexcept = default;
        engine& operator=(engine&&) noexcept = default;

        void initialize(const arguments &args);
        void finalize();

    public:
        flecs::world ecs;
    };

    struct engine::arguments {
        std::filesystem::path app_mount;
        bool dev_mode{false};

        std::unordered_map<std::string, std::string> module_args;
    };

}