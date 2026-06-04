#pragma once
#include <expected>
#include <string>

#include "sandbox/event_bus/logger_events.h"

namespace sandbox {

    class ilogger {
    public:
        virtual ~ilogger() = default;
        [[nodiscard]] virtual std::expected<void, std::string> log(const events::log& log_event) = 0;
    };

    struct logger_service {
        ilogger* api{nullptr};
    };

} // namespace sandbox
