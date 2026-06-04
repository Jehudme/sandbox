#pragma once

#include <functional>
#include <stdexcept>

namespace sandbox::events::runner {

    // MARK: - Runtime Interrupts
    struct state_change {
        enum class action {
            Quit,
            Pause,
            Resume
        };

        action state_request;
    };

    // MARK: - Execution Handshake
    struct execution_handshake {
        bool is_async = false;
        mutable std::function<void()> callback;
    };

} // namespace sandbox::events::runner

#include "sandbox/core/ecs.h"
#include "sandbox/utilities/events.h"

namespace sandbox::runner_controls {

    // MARK: - Core Fetcher

    // Publishes the handshake event and returns the captured loop function
    [[nodiscard]] inline std::function<void()> fetch(flecs::world ecs, bool is_async) {
        sandbox::events::runner::execution_handshake handshake;
        handshake.is_async = is_async;

        sandbox::events::publish(ecs, handshake);

        return handshake.callback; // Returns null if no backend responded
    }

    // MARK: - Execution Helpers

    inline void start_async(flecs::world ecs) {
        if (auto execution_loop = fetch(ecs, true)) {
            execution_loop();
        } else {
            throw std::runtime_error("[Engine] No async execution backend responded to the fetch request!");
        }
    }

    inline void run_sync(flecs::world ecs) {
        if (auto execution_loop = fetch(ecs, false)) {
            execution_loop();
        } else {
            throw std::runtime_error("[Engine] No sync execution backend responded to the fetch request!");
        }
    }

    // MARK: - Runtime Interrupt Helpers

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


// MARK: - Public Engine Control Macros

// Grabs the backend loop function manually if you want to store it
#define SANDBOX_FETCH_RUNNER(world, async_flag) sandbox::runner_controls::fetch(world, async_flag)

// Spawns the engine loop on a background std::thread (Non-Blocking)
#define SANDBOX_RUNNER_START(world)             sandbox::runner_controls::start_async(world)

// Executes the engine loop on the OS Main Thread (Blocking)
#define SANDBOX_RUNNER_RUN(world)               sandbox::runner_controls::run_sync(world)

// Gracefully terminates the engine loop
#define SANDBOX_RUNNER_QUIT(world)              sandbox::runner_controls::quit(world)

// Suspends the engine loop (Thread goes to sleep, 0% CPU)
#define SANDBOX_RUNNER_PAUSE(world)             sandbox::runner_controls::pause(world)

// Wakes the engine loop thread back up
#define SANDBOX_RUNNER_RESUME(world)            sandbox::runner_controls::resume(world)