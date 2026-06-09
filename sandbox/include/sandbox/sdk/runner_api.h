#pragma once
#include "sandbox/subsystems/runner/irunner.h"
#include "sandbox/sdk/payload.h"
#include <glaze/glaze.hpp>
#include <string>
#include <expected>
#include <stdexcept>

#include "sandbox/core/ecs.h"

namespace sandbox::sdk {

    class runner {
    public:
        explicit runner(irunner* api) : m_api(api) {
            if (!m_api) throw std::invalid_argument("Runner API pointer is null");
        }

        explicit runner(flecs::world& ecs) {
            auto srv = ecs.get<sandbox::runner_service>();
            m_api = srv.api;
            if (!m_api) throw std::invalid_argument("Runner API is not available in ECS");
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

        std::expected<void, std::string> start_async(flecs::world& ecs) {
            if (m_api->start_async(ecs) != 0) return std::unexpected("Failed to start runner asynchronously");
            return {};
        }

        std::expected<void, std::string> run_sync(flecs::world& ecs) {
            if (m_api->run_sync(ecs) != 0) return std::unexpected("Failed to run runner synchronously");
            return {};
        }

        std::expected<void, std::string> quit() {
            if (m_api->quit() != 0) return std::unexpected("Failed to quit runner");
            return {};
        }

        std::expected<void, std::string> pause() {
            if (m_api->pause() != 0) return std::unexpected("Failed to pause runner");
            return {};
        }

        std::expected<void, std::string> resume() {
            if (m_api->resume() != 0) return std::unexpected("Failed to resume runner");
            return {};
        }

    private:
        irunner* m_api;
    };

} // namespace sandbox::sdk
