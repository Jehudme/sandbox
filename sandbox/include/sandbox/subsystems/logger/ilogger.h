#pragma once
#include <expected>
#include <string>
#include <any>

#include "sandbox/event_bus/logger_events.h"

namespace sandbox {

    class ilogger {
    public:
        virtual ~ilogger() = default;
        [[nodiscard]] virtual std::expected<void, std::string> log(const events::log& log_event) = 0;
        
        virtual void set_property(const std::string& key, const std::any& value) = 0;
        virtual std::any get_property(const std::string& key) const = 0;
    };

    struct logger_service {
        ilogger* api{nullptr};
    };

} // namespace sandbox
