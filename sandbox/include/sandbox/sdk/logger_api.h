#pragma once
#include "sandbox/subsystems/logger/ilogger.h"
#include "sandbox/sdk/payload.h"
#include <glaze/glaze.hpp>
#include <string>
#include <expected>
#include <stdexcept>

namespace sandbox::sdk {

    class logger {
    public:
        explicit logger(ilogger* api) : m_api(api) {
            if (!m_api) throw std::invalid_argument("Logger API pointer is null");
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

        std::expected<void, std::string> log(const uint8_t* log_msg_fb, size_t size) {
            if (m_api->log(log_msg_fb, size) != 0) {
                return std::unexpected("Failed to log message");
            }
            return {};
        }

    private:
        ilogger* m_api;
    };

} // namespace sandbox::sdk
