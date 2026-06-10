#pragma once
#include <cstdint>
#include <string>
#include <format>
#include <optional>
#include <sandbox/api/abi_types.h>
#include "generated/schemas/logger_generated.h"
#include <flatbuffers/flatbuffers.h>

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

#define INTERNAL_SANDBOX_LOG_PUBLISH(world_context, severity_enum, throw_override_val, format_literal, ...) \
    do { \
        if ((world_context).template has<sandbox::logger_service>()) { \
            std::string msg; \
            try { \
                msg = std::format(format_literal, ##__VA_ARGS__); \
            } catch (const std::exception& e) { \
                msg = std::string("FORMAT ERROR: ") + e.what(); \
            } \
            flatbuffers::FlatBufferBuilder builder; \
            auto fmsg = builder.CreateString(msg); \
            auto ffile = builder.CreateString(__FILE__); \
            sandbox::schemas::logger::LogMessageBuilder lmb(builder); \
            lmb.add_level(sandbox::schemas::logger::LogLevel_##severity_enum); \
            lmb.add_message(fmsg); \
            lmb.add_source_file(ffile); \
            lmb.add_source_line(__LINE__); \
            std::optional<bool> override_val = throw_override_val; \
            lmb.add_throw_on_error(override_val.value_or(false)); \
            builder.Finish(lmb.Finish()); \
            if ((world_context).template get<sandbox::logger_service>().api->log(builder.GetBufferPointer(), builder.GetSize()) == -1) { \
                throw std::runtime_error(format_literal); \
            } \
        } \
    } while(0)

#ifndef NDEBUG
    #define SANDBOX_TRACE(world, format, ...) INTERNAL_SANDBOX_LOG_PUBLISH(world, Trace, std::nullopt, format, ##__VA_ARGS__)
    #define SANDBOX_DEBUG(world, format, ...) INTERNAL_SANDBOX_LOG_PUBLISH(world, Debug, std::nullopt, format, ##__VA_ARGS__)
#else
    #define SANDBOX_TRACE(world, format, ...) (void)0
    #define SANDBOX_DEBUG(world, format, ...) (void)0
#endif

#define SANDBOX_INFO(world, format, ...)  INTERNAL_SANDBOX_LOG_PUBLISH(world, Info,  std::nullopt, format, ##__VA_ARGS__)
#define SANDBOX_WARN(world, format, ...)  INTERNAL_SANDBOX_LOG_PUBLISH(world, Warn,  std::nullopt, format, ##__VA_ARGS__)
#define SANDBOX_ERROR(world, format, ...) INTERNAL_SANDBOX_LOG_PUBLISH(world, Error, std::nullopt, format, ##__VA_ARGS__)
#define SANDBOX_FATAL(world, format, ...) INTERNAL_SANDBOX_LOG_PUBLISH(world, Fatal, std::nullopt, format, ##__VA_ARGS__)

#define SANDBOX_WARN_THROW(world, format, ...)  INTERNAL_SANDBOX_LOG_PUBLISH(world, Warn,  std::optional<bool>{true}, format, ##__VA_ARGS__)
#define SANDBOX_ERROR_THROW(world, format, ...) INTERNAL_SANDBOX_LOG_PUBLISH(world, Error, std::optional<bool>{true}, format, ##__VA_ARGS__)
#define SANDBOX_FATAL_THROW(world, format, ...) INTERNAL_SANDBOX_LOG_PUBLISH(world, Fatal, std::optional<bool>{true}, format, ##__VA_ARGS__)
