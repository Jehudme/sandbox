#pragma once
#include <sandbox/api/abi_types.h>
#include <sandbox/api/payload.h>
#include <glaze/glaze.hpp>
#include <string>
#include <expected>
#include <sandbox/core/exceptions.h>

#include "sandbox/core/ecs.h"

namespace sandbox::sdk {

    class runner {
    public:
        explicit runner(const sandbox::runner_service* api) : m_api(api) {
            if (!m_api || !m_api->instance) throw sandbox::null_api_error("Runner API pointer is null");
        }

        explicit runner(flecs::world& ecs) {
            m_api = ecs.try_get<sandbox::runner_service>();
            if (!m_api || !m_api->instance) throw sandbox::null_api_error("Runner API is not available in ECS");
        }

        template <typename T>
        std::expected<void, std::string> set_property(const std::string& key, const T& value) {
            std::string json;
            auto err = glz::write_json(value, json);
            if (err) return std::unexpected("Failed to serialize property");
            m_api->set_property(m_api->instance, key.c_str(), json.c_str());
            return {};
        }

        template <typename T>
        std::expected<T, std::string> get_property(const std::string& key) const {
            payload p;
            if (m_api->get_property(m_api->instance, key.c_str(), p.get()) != 0) {
                return std::unexpected("Property not found or access error: " + key);
            }
            T value{};
            std::string json = p.as_string();
            auto err = glz::read_json(value, json);
            if (err) return std::unexpected("Failed to parse property JSON");
            return value;
        }

        std::expected<void, std::string> start_async(flecs::world& ecs) {
            if (m_api->start_async(m_api->instance, ecs) != 0) return std::unexpected("Failed to start runner asynchronously");
            return {};
        }

        std::expected<void, std::string> run_sync(flecs::world& ecs) {
            if (m_api->run_sync(m_api->instance, ecs) != 0) return std::unexpected("Failed to run runner synchronously");
            return {};
        }

        std::expected<void, std::string> quit() {
            if (m_api->quit(m_api->instance) != 0) return std::unexpected("Failed to quit runner");
            return {};
        }

        std::expected<void, std::string> pause() {
            if (m_api->pause(m_api->instance) != 0) return std::unexpected("Failed to pause runner");
            return {};
        }

        std::expected<void, std::string> resume() {
            if (m_api->resume(m_api->instance) != 0) return std::unexpected("Failed to resume runner");
            return {};
        }

    private:
        const sandbox::runner_service* m_api;
    };

} // namespace sandbox::sdk

namespace sandbox::api {
    inline std::expected<void, std::string> start_async(flecs::world& ecs) {
        return sandbox::sdk::runner(ecs).start_async(ecs);
    }
    inline std::expected<void, std::string> run_sync(flecs::world& ecs) {
        return sandbox::sdk::runner(ecs).run_sync(ecs);
    }
    inline std::expected<void, std::string> quit(flecs::world& ecs) {
        return sandbox::sdk::runner(ecs).quit();
    }
    inline std::expected<void, std::string> pause(flecs::world& ecs) {
        return sandbox::sdk::runner(ecs).pause();
    }
    inline std::expected<void, std::string> resume(flecs::world& ecs) {
        return sandbox::sdk::runner(ecs).resume();
    }
} // namespace sandbox::api

