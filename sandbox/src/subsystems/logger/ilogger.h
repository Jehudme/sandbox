#pragma once
#include <cstdint>
#include <string>
#include <format>
#include <optional>
#include "sandbox/core/exceptions.h"
#include <sandbox/api/abi_types.h>
#include <sandbox/generated/schemas/logger_generated.h>
#include <flatbuffers/flatbuffers.h>

namespace sandbox {

    class ilogger {
    public:
        virtual ~ilogger() = default;
        [[nodiscard]] virtual int32_t log(const uint8_t* log_msg_fb, size_t size) = 0;
        
        virtual void set_property(const char* key, const char* json_value) = 0;
        virtual int32_t get_property(const char* key, sandbox_payload* out_payload) const = 0;
    };

} // namespace sandbox


