#pragma once

#include <cstdint>
#include <string>
#include <format>
#include <optional>
#include "sandbox/core/exceptions.h"
#include "sandbox/core/service_macro.h"

namespace sandbox {
    DECLARE_SANDBOX_SERVICE(logger_service, "logger_service", 1, 0)

    class ilogger {
    public:
        virtual ~ilogger() = default;
        // Native C++ interface methods implemented by logger.cpp
        virtual int32_t log(const uint8_t* log_msg_fb, size_t size) = 0;
        virtual void set_property(const char* key, const char* json_value) = 0;
        virtual int32_t get_property(const char* key, sandbox_payload* out_payload) const = 0;
    };

} // namespace sandbox
