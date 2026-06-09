#pragma once
#include <cstdint>
#include "sandbox/core/abi_types.h"
#include "sandbox/event_bus/logger_events.h"

namespace sandbox {

    class ilogger {
    public:
        virtual ~ilogger() = default;
        [[nodiscard]] virtual int32_t log(const uint8_t* log_msg_fb, size_t size) = 0;
        
        virtual void set_property(const char* key, const char* json_value) = 0;
        virtual int32_t get_property(const char* key, sandbox_payload* out_payload) const = 0;
    };

    struct logger_service {
        ilogger* api{nullptr};
    };

} // namespace sandbox
