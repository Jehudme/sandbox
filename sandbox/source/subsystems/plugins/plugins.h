#pragma once

#include "sandbox/core/ecs.h"
#include "sandbox/event_bus/plugin_events.h"
#include "sandbox/subsystems/plugins/iplugins.h"

namespace sandbox::modules {

    class plugins : public iplugins {
    public:
        plugins(world& ecs);
        ~plugins() override;

        void load(std::string_view virtual_path, std::string_view entry_point = "SandboxLibraryMain") override;

    private:
        world* m_ecs{nullptr};
    };

} // namespace sandbox::modules
