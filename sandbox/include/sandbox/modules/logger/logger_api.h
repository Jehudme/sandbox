#pragma once
#include <sandbox/core/payload.h>
#include <glaze/glaze.hpp>
#include <string>
#include <expected>
#include <sandbox/core/exceptions.h>
#include "sandbox/core/ecs.h"
#include <sandbox/core/common_schema.h>
#include <sandbox/modules/logger/logger_schema.h>
#include <sandbox/modules/logger/ilogger.h>

namespace sandbox::sdk {

    class logger {
    public:
        explicit logger(const sandbox::logger_service* api) : m_api(api) {
            if (!m_api || !m_api->instance) throw sandbox::null_api_error("Logger API pointer is null");
        }

        explicit logger(flecs::world& ecs) {
            m_api = ecs.try_get<sandbox::logger_service>();
            if (!m_api || !m_api->instance) throw sandbox::null_api_error("Logger API is not available in ECS");
        }

        template <typename T>
        std::expected<void, std::string> set_property(const std::string& key, const T& value) {
            std::string json;
            auto err = glz::write_json(value, json);
            if (err) return std::unexpected("Failed to serialize property");
            
            flatbuffers::FlatBufferBuilder builder;
            auto fb_key = builder.CreateString(key);
            auto fb_val = builder.CreateString(json);
            auto args = sandbox::schemas::logger::CreateSetPropertyArgs(builder, fb_key, fb_val);
            builder.Finish(args);

            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::logger::LoggerCommand_SetProperty), builder.GetBufferPointer(), builder.GetSize());
            return {};
        }

        template <typename T>
        std::expected<T, std::string> get_property(const std::string& key) const {
            payload p;
            int32_t result = 0;
            
            flatbuffers::FlatBufferBuilder builder;
            auto fb_key = builder.CreateString(key);
            auto args = sandbox::schemas::logger::CreateGetPropertyArgs(builder, fb_key, reinterpret_cast<uint64_t>(p.get()), reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);

            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::logger::LoggerCommand_GetProperty), builder.GetBufferPointer(), builder.GetSize());
            
            if (result != 0) {
                return std::unexpected("Property not found or access error: " + key);
            }
            T value{};
            std::string json = p.as_string();
            auto err = glz::read_json(value, json);
            if (err) return std::unexpected("Failed to parse property JSON");
            return value;
        }

        std::expected<void, std::string> log(int level, const std::string& message) {
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto msg_offset = builder.CreateString(message);
            auto log_fb = sandbox::schemas::logger::CreateLogArgs(builder, static_cast<sandbox::schemas::logger::LogLevel>(level), msg_offset, 0, 0, false, reinterpret_cast<uint64_t>(&result));
            builder.Finish(log_fb);
            
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::logger::LoggerCommand_Log), builder.GetBufferPointer(), builder.GetSize());
            
            if (result != 0) {
                return std::unexpected("Failed to log message");
            }
            return {};
        }

    private:
        const sandbox::logger_service* m_api;
    };

} // namespace sandbox::sdk

namespace sandbox::api {
    inline std::expected<void, std::string> log(flecs::world& ecs, int level, const std::string& message) {
        return sandbox::sdk::logger(ecs).log(level, message);
    }
}

#define INTERNAL_SANDBOX_LOG_PUBLISH(world_context, severity_enum, throw_override_val, format_literal, ...) \
    do { \
        if ((world_context).template has<::sandbox::logger_service>()) { \
            std::string msg; \
            try { \
                msg = std::format(format_literal, ##__VA_ARGS__); \
            } catch (const std::exception& e) { \
                msg = std::string("FORMAT ERROR: ") + e.what(); \
            } \
            int32_t result = 0; \
            flatbuffers::FlatBufferBuilder builder; \
            auto fmsg = builder.CreateString(msg); \
            auto ffile = builder.CreateString(__FILE__); \
            ::sandbox::schemas::logger::LogArgsBuilder lmb(builder); \
            lmb.add_level(::sandbox::schemas::logger::LogLevel_##severity_enum); \
            lmb.add_message(fmsg); \
            lmb.add_source_file(ffile); \
            lmb.add_source_line(__LINE__); \
            std::optional<bool> override_val = throw_override_val; \
            lmb.add_throw_on_error(override_val.value_or(false)); \
            lmb.add_out_result_ptr(reinterpret_cast<uint64_t>(&result)); \
            builder.Finish(lmb.Finish()); \
            const ::sandbox::logger_service* srv = (world_context).template try_get<::sandbox::logger_service>(); \
            srv->execute_command(srv->instance, static_cast<uint32_t>(::sandbox::schemas::logger::LoggerCommand_Log), builder.GetBufferPointer(), builder.GetSize()); \
            if (result == -1) { \
                throw ::sandbox::boot_error(format_literal); \
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
