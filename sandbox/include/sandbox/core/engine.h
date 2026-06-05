#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <unordered_map>

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

        /// Initialises all core subsystems, mounts VFS paths, and loads manifest plugins.
        /// Throws std::runtime_error on any unrecoverable failure (e.g. missing VFS mount).
        void initialize(const arguments& args);

        /// Signals the runner to stop and tears down the ECS world.
        /// Safe to call explicitly; the destructor will not double-finalize.
        void finalize();

    public:
        // Public ECS world — required for plugin interop across the DLL boundary.
        // Consumers should prefer accessing subsystems via the service components
        // (e.g. ecs.get<runner_service>()) rather than interacting with ecs directly.
        flecs::world ecs;

    private:
        bool m_initialized{false};
    };

    struct engine::arguments {
        std::filesystem::path app_mount;
        bool dev_mode{false};
        std::unordered_map<std::string, std::string> module_args;
    };

}