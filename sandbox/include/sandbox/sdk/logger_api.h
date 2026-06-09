#pragma once
#include "sandbox/subsystems/logger/ilogger.h"
#include "sandbox/sdk/payload.h"
#include <glaze/glaze.hpp>
#include <string>
#include <expected>
#include <stdexcept>
#include "sandbox/core/ecs.h"
#include "sandbox/generated/schemas/common_generated.h"

namespace sandbox::sdk {

    class logger {
    public:
        explicit logger(ilogger* api) : m_api(api) {
            if (!m_api) throw std::invalid_argument("Logger API pointer is null");
        }

        explicit logger(flecs::world& ecs) {
            auto srv = ecs.get<sandbox::logger_service>();
            m_api = srv.api;
            if (!m_api) throw std::invalid_argument("Logger API is not available in ECS");
        }

        template <typename T>
        std::expected<void, std::string> set_property(const std::string& key, const T& value) {
            std::string json;
            auto err = glz::write_json(value, json);
            if (err) return std::unexpected("Failed to serialize property");
            m_api->set_property(key.c_str(), json.c_str());
            return {};
        }

        template <typename T>
        std::expected<T, std::string> get_property(const std::string& key) const {
            payload p;
            if (m_api->get_property(key.c_str(), p.get()) != 0) {
                return std::unexpected("Property not found or access error: " + key);
            }
            T value{};
            std::string json = p.as_string();
            auto err = glz::read_json(value, json);
            if (err) return std::unexpected("Failed to parse property JSON");
            return value;
        }

        std::expected<void, std::string> log(int level, const std::string& message) {
            flatbuffers::FlatBufferBuilder builder;
            auto msg_offset = builder.CreateString(message);
            auto log_fb = sandbox::schemas::CreateLogMessage(builder, static_cast<sandbox::schemas::LogLevel>(level), msg_offset);
            builder.Finish(log_fb);
            
            if (m_api->log(builder.GetBufferPointer(), builder.GetSize()) != 0) {
                return std::unexpected("Failed to log message");
            }
            return {};
        }

    private:
        ilogger* m_api;
    };

} // namespace sandbox::sdk
