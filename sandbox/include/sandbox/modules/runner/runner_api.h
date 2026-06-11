#pragma once
#include <sandbox/core/payload.h>
#include <glaze/glaze.hpp>
#include <string>
#include <expected>
#include <sandbox/core/exceptions.h>
#include "sandbox/core/ecs.h"
#include <sandbox/modules/runner/runner_schema.h>
#include <sandbox/modules/runner/irunner.h>

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
            
            flatbuffers::FlatBufferBuilder builder;
            auto fb_key = builder.CreateString(key);
            auto fb_val = builder.CreateString(json);
            auto args = sandbox::schemas::runner::CreateSetPropertyArgs(builder, fb_key, fb_val);
            builder.Finish(args);

            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::runner::RunnerCommand_SetProperty), builder.GetBufferPointer(), builder.GetSize());
            return {};
        }

        template <typename T>
        std::expected<T, std::string> get_property(const std::string& key) const {
            payload p;
            int32_t result = 0;
            
            flatbuffers::FlatBufferBuilder builder;
            auto fb_key = builder.CreateString(key);
            auto args = sandbox::schemas::runner::CreateGetPropertyArgs(builder, fb_key, reinterpret_cast<uint64_t>(p.get()), reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);

            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::runner::RunnerCommand_GetProperty), builder.GetBufferPointer(), builder.GetSize());
            
            if (result != 0) return std::unexpected("Property not found or access error: " + key);
            
            T value{};
            std::string json = p.as_string();
            auto err = glz::read_json(value, json);
            if (err) return std::unexpected("Failed to parse property JSON");
            return value;
        }

        std::expected<void, std::string> start_async(flecs::world& ecs) {
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto args = sandbox::schemas::runner::CreateStartAsyncArgs(builder, reinterpret_cast<uint64_t>(&ecs), reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::runner::RunnerCommand_StartAsync), builder.GetBufferPointer(), builder.GetSize());
            if (result != 0) return std::unexpected("Failed to start runner asynchronously");
            return {};
        }

        std::expected<void, std::string> run_sync(flecs::world& ecs) {
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto args = sandbox::schemas::runner::CreateRunSyncArgs(builder, reinterpret_cast<uint64_t>(&ecs), reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::runner::RunnerCommand_RunSync), builder.GetBufferPointer(), builder.GetSize());
            if (result != 0) return std::unexpected("Failed to run runner synchronously");
            return {};
        }

        std::expected<void, std::string> quit() {
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto args = sandbox::schemas::runner::CreateQuitArgs(builder, reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::runner::RunnerCommand_Quit), builder.GetBufferPointer(), builder.GetSize());
            if (result != 0) return std::unexpected("Failed to quit runner");
            return {};
        }

        std::expected<void, std::string> pause() {
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto args = sandbox::schemas::runner::CreatePauseArgs(builder, reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::runner::RunnerCommand_Pause), builder.GetBufferPointer(), builder.GetSize());
            if (result != 0) return std::unexpected("Failed to pause runner");
            return {};
        }

        std::expected<void, std::string> resume() {
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto args = sandbox::schemas::runner::CreateResumeArgs(builder, reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::runner::RunnerCommand_Resume), builder.GetBufferPointer(), builder.GetSize());
            if (result != 0) return std::unexpected("Failed to resume runner");
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
