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

        engine(const engine&) = delete;
        engine& operator=(const engine&) = delete;

        engine(engine&&) noexcept = default;
        engine& operator=(engine&&) noexcept = default;

        void initialize(const engine_arguments &arguments);
        void finalize();

    public:
        flecs::world ecs;
    };

}