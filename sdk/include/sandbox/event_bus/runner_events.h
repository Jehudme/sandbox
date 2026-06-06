#pragma once

#include <functional>
#include <stdexcept>

namespace sandbox::events::runner {

    // Runtime state change request
    struct state_change {
        enum class action {
            Quit,
            Pause,
            Resume
        };

        action state_request;
    };

// Execution handshake was removed in favor of direct locator lookup

} // namespace sandbox::events::runner
