#pragma once

#include "sandbox/core/ecs.h"
#include "sandbox/events/runner.h"
#include "sandbox/utilities/events.h"

namespace sandbox::runner_controls {

    // ============================================================================
    // Type-Safe Inline Helper Functions
    // ============================================================================

    inline void start_async(flecs::world ecs) {
        sandbox::events::publish(ecs, events::runner::state_change{
            events::runner::state_change::action::Start
        });
    }

    inline void run_sync(flecs::world ecs) {
        sandbox::events::publish(ecs, events::runner::state_change{
            events::runner::state_change::action::Run
        });
    }

    inline void quit(flecs::world ecs) {
        sandbox::events::publish(ecs, events::runner::state_change{
            events::runner::state_change::action::Quit
        });
    }

    inline void pause(flecs::world ecs) {
        sandbox::events::publish(ecs, events::runner::state_change{
            events::runner::state_change::action::Pause
        });
    }

    inline void resume(flecs::world ecs) {
        sandbox::events::publish(ecs, events::runner::state_change{
            events::runner::state_change::action::Resume
        });
    }

} // namespace sandbox::runner_controls

// ============================================================================
// Engine Control Macros (Matches the SANDBOX_INFO style)
// ============================================================================

#define SANDBOX_ENGINE_START(world)  sandbox::runner_controls::start_async(world)
#define SANDBOX_ENGINE_RUN(world)    sandbox::runner_controls::run_sync(world)
#define SANDBOX_ENGINE_QUIT(world)   sandbox::runner_controls::quit(world)
#define SANDBOX_ENGINE_PAUSE(world)  sandbox::runner_controls::pause(world)
#define SANDBOX_ENGINE_RESUME(world) sandbox::runner_controls::resume(world)