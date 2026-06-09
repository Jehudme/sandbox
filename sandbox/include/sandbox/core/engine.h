#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <any>

#include "sandbox/core/platform.h"

#include "sandbox/utilities/properties.h"

namespace sandbox {

    struct engine_environment {
        sandbox::properties config;
    };

    class SANDBOX_API engine {
    public:
        engine();
        ~engine();

        engine(const engine&) = delete;
        engine& operator=(const engine&) = delete;

        engine(engine&&) noexcept;
        engine& operator=(engine&&) noexcept;

        /// Initialises all core subsystems, mounts VFS paths, and loads manifest plugins.
        /// Throws std::runtime_error on any unrecoverable failure (e.g. missing VFS mount).
        void initialize(const sandbox::properties& config);

        /// Signals the runner to stop and tears down the ECS world.
        /// Safe to call explicitly; the destructor will not double-finalize.
        void finalize();

        /// Provides a C-ABI friendly accessor to the internal flecs::world
        [[nodiscard]] void* get_world() const;

    private:
        struct impl;
        impl* m_impl;
    };



}