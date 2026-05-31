#pragma once

namespace sandbox::events::runner {

    struct state_change {
        enum class action {
            Start, // Spawns an async background thread
            Run,   // Blocks the calling thread (Synchronous)
            Quit,  // Terminates the loop gracefully
            Pause, // Suspends ECS progression (Puts thread to sleep)
            Resume // Wakes the thread and resumes progression
        };

        action state_request;
    };

} // namespace sandbox::events::runner