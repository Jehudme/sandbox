#pragma once

#include <functional>

namespace sandbox::events::runner {

    // 1. The Runtime Interrupts
    struct state_change {
        enum class action {
            Quit,
            Pause,
            Resume
        };

        action state_request;
    };

    // 2. The Execution Handshake
    struct execution_handshake {
        bool is_async = false;
        mutable std::function<void()> callback;
    };

} // namespace sandbox::events::runner