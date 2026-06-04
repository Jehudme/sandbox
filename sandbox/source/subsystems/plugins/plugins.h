#pragma once

#include "sandbox/core/ecs.h"
#include "sandbox/event_bus/plugin_events.h"
#include "sandbox/subsystems/plugins/iplugins.h"

namespace sandbox::modules {

    class plugins : public iplugins {
    public:
        // MARK: - Subsystem Lifecycle
        plugins(world& ecs);
        ~plugins() override;

        // MARK: - Subsystem Implementation
        [[nodiscard]] std::expected<void, std::string> load(std::string_view virtual_path, std::string_view entry_point = "SandboxLibraryMain") override;

    private:
        world* m_ecs{nullptr};
    };

} // namespace sandbox::modules
