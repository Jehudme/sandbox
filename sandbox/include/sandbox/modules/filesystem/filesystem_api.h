#pragma once
#include <sandbox/core/payload.h>
#include <glaze/glaze.hpp>
#include <string>
#include <expected>
#include <vector>
#include <sandbox/core/exceptions.h>
#include "sandbox/core/ecs.h"
#include <sandbox/modules/filesystem/filesystem_schema.h>
#include <sandbox/modules/filesystem/ifilesystem.h>

namespace sandbox::sdk {

    class filesystem {
    public:
        explicit filesystem(const sandbox::filesystem_service* api) : m_api(api) {
            if (!m_api || !m_api->instance) throw sandbox::null_api_error("Filesystem API pointer is null");
        }

        explicit filesystem(flecs::world& ecs) {
            m_api = ecs.try_get<sandbox::filesystem_service>();
            if (!m_api || !m_api->instance) throw sandbox::null_api_error("Filesystem API is not available in ECS");
        }

        template <typename T>
        std::expected<void, std::string> set_property(const std::string& key, const T& value) {
            std::string json;
            auto err = glz::write_json(value, json);
            if (err) return std::unexpected("Failed to serialize property");
            
            flatbuffers::FlatBufferBuilder builder;
            auto fb_key = builder.CreateString(key);
            auto fb_val = builder.CreateString(json);
            auto args = sandbox::schemas::filesystem::CreateSetPropertyArgs(builder, fb_key, fb_val);
            builder.Finish(args);

            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::filesystem::FilesystemCommand_SetProperty), builder.GetBufferPointer(), builder.GetSize());
            return {};
        }

        template <typename T>
        std::expected<T, std::string> get_property(const std::string& key) const {
            payload p;
            int32_t result = 0;
            
            flatbuffers::FlatBufferBuilder builder;
            auto fb_key = builder.CreateString(key);
            auto args = sandbox::schemas::filesystem::CreateGetPropertyArgs(builder, fb_key, reinterpret_cast<uint64_t>(p.get()), reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);

            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::filesystem::FilesystemCommand_GetProperty), builder.GetBufferPointer(), builder.GetSize());
            
            if (result != 0) return std::unexpected("Property not found or access error: " + key);
            
            T value{};
            std::string json = p.as_string();
            auto err = glz::read_json(value, json);
            if (err) return std::unexpected("Failed to parse property JSON");
            return value;
        }

        std::expected<void, std::string> mount(const std::string& physical_path, const std::string& virtual_prefix, bool read_only = false) {
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto args = sandbox::schemas::filesystem::CreateMountArgs(builder, builder.CreateString(physical_path), builder.CreateString(virtual_prefix), read_only, reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::filesystem::FilesystemCommand_Mount), builder.GetBufferPointer(), builder.GetSize());
            if (result != 0) return std::unexpected("Failed to mount path");
            return {};
        }

        std::expected<void, std::string> unmount(const std::string& virtual_prefix) {
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto args = sandbox::schemas::filesystem::CreateUnmountArgs(builder, builder.CreateString(virtual_prefix), reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::filesystem::FilesystemCommand_Unmount), builder.GetBufferPointer(), builder.GetSize());
            if (result != 0) return std::unexpected("Failed to unmount path");
            return {};
        }

        std::expected<std::string, std::string> read_text(const std::string& virtual_path) const {
            payload p;
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto args = sandbox::schemas::filesystem::CreateReadArgs(builder, builder.CreateString(virtual_path), reinterpret_cast<uint64_t>(p.get()), reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::filesystem::FilesystemCommand_Read), builder.GetBufferPointer(), builder.GetSize());
            if (result != 0) return std::unexpected("Failed to read file");
            return p.as_string();
        }

        std::expected<std::vector<std::byte>, std::string> read_binary(const std::string& virtual_path) const {
            payload p;
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto args = sandbox::schemas::filesystem::CreateReadArgs(builder, builder.CreateString(virtual_path), reinterpret_cast<uint64_t>(p.get()), reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::filesystem::FilesystemCommand_Read), builder.GetBufferPointer(), builder.GetSize());
            if (result != 0) return std::unexpected("Failed to read file");
            auto data_ptr = reinterpret_cast<const std::byte*>(p.data());
            return std::vector<std::byte>(data_ptr, data_ptr + p.size());
        }

        std::expected<void, std::string> write(const std::string& virtual_path, const std::string& data, bool append = false) {
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto data_vec = builder.CreateVector(reinterpret_cast<const uint8_t*>(data.data()), data.size());
            auto args = sandbox::schemas::filesystem::CreateWriteArgs(builder, builder.CreateString(virtual_path), data_vec, append, reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::filesystem::FilesystemCommand_Write), builder.GetBufferPointer(), builder.GetSize());
            if (result != 0) return std::unexpected("Failed to write to file");
            return {};
        }

        std::expected<void, std::string> write(const std::string& virtual_path, const std::vector<std::byte>& data, bool append = false) {
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto data_vec = builder.CreateVector(reinterpret_cast<const uint8_t*>(data.data()), data.size());
            auto args = sandbox::schemas::filesystem::CreateWriteArgs(builder, builder.CreateString(virtual_path), data_vec, append, reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::filesystem::FilesystemCommand_Write), builder.GetBufferPointer(), builder.GetSize());
            if (result != 0) return std::unexpected("Failed to write to file");
            return {};
        }

        std::expected<std::vector<std::string>, std::string> list(const std::string& virtual_path, bool recursive = false) const {
            payload p;
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto args = sandbox::schemas::filesystem::CreateListArgs(builder, builder.CreateString(virtual_path), recursive, reinterpret_cast<uint64_t>(p.get()), reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::filesystem::FilesystemCommand_List), builder.GetBufferPointer(), builder.GetSize());
            
            if (result != 0) return std::unexpected("Failed to list directory");
            if (!p.data() || p.size() == 0) return std::vector<std::string>{};

            auto fb_list = flatbuffers::GetRoot<sandbox::schemas::filesystem::StringList>(p.data());
            std::vector<std::string> res;
            if (fb_list && fb_list->items()) {
                res.reserve(fb_list->items()->size());
                for (const auto& item : *fb_list->items()) {
                    res.push_back(item->str());
                }
            }
            return res;
        }

        std::expected<void, std::string> remove(const std::string& virtual_path) {
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto args = sandbox::schemas::filesystem::CreateRemoveArgs(builder, builder.CreateString(virtual_path), reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::filesystem::FilesystemCommand_Remove), builder.GetBufferPointer(), builder.GetSize());
            if (result != 0) return std::unexpected("Failed to remove");
            return {};
        }

        std::expected<void, std::string> mkdir(const std::string& virtual_path) {
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto args = sandbox::schemas::filesystem::CreateMkdirArgs(builder, builder.CreateString(virtual_path), reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::filesystem::FilesystemCommand_Mkdir), builder.GetBufferPointer(), builder.GetSize());
            if (result != 0) return std::unexpected("Failed to make directory");
            return {};
        }

        std::expected<void, std::string> rename(const std::string& old_path, const std::string& new_path) {
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto args = sandbox::schemas::filesystem::CreateRenameArgs(builder, builder.CreateString(old_path), builder.CreateString(new_path), reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::filesystem::FilesystemCommand_Rename), builder.GetBufferPointer(), builder.GetSize());
            if (result != 0) return std::unexpected("Failed to rename");
            return {};
        }

        std::expected<void, std::string> copy(const std::string& src, const std::string& dst) {
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto args = sandbox::schemas::filesystem::CreateCopyArgs(builder, builder.CreateString(src), builder.CreateString(dst), reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::filesystem::FilesystemCommand_Copy), builder.GetBufferPointer(), builder.GetSize());
            if (result != 0) return std::unexpected("Failed to copy");
            return {};
        }

        std::expected<void, std::string> move(const std::string& src, const std::string& dst) {
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto args = sandbox::schemas::filesystem::CreateMoveArgs(builder, builder.CreateString(src), builder.CreateString(dst), reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::filesystem::FilesystemCommand_Move), builder.GetBufferPointer(), builder.GetSize());
            if (result != 0) return std::unexpected("Failed to move");
            return {};
        }

        std::expected<std::string, std::string> absolute(const std::string& virtual_path) const {
            payload p;
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto args = sandbox::schemas::filesystem::CreateAbsoluteArgs(builder, builder.CreateString(virtual_path), reinterpret_cast<uint64_t>(p.get()), reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::filesystem::FilesystemCommand_Absolute), builder.GetBufferPointer(), builder.GetSize());
            if (result != 0) return std::unexpected("Failed to get absolute path");
            return p.as_string();
        }

        struct file_state {
            size_t size;
            bool is_directory;
            uint64_t modified_time;
        };

        std::expected<file_state, std::string> state(const std::string& virtual_path) const {
            payload p;
            int32_t result = 0;
            flatbuffers::FlatBufferBuilder builder;
            auto args = sandbox::schemas::filesystem::CreateStateArgs(builder, builder.CreateString(virtual_path), reinterpret_cast<uint64_t>(p.get()), reinterpret_cast<uint64_t>(&result));
            builder.Finish(args);
            m_api->execute_command(m_api->instance, static_cast<uint32_t>(sandbox::schemas::filesystem::FilesystemCommand_State), builder.GetBufferPointer(), builder.GetSize());
            
            if (result != 0) return std::unexpected("Failed to get state");
            if (!p.data() || p.size() == 0) return std::unexpected("Empty payload for state");
            
            auto fb_meta = flatbuffers::GetRoot<sandbox::schemas::filesystem::FileMetadata>(p.data());
            if (!fb_meta) return std::unexpected("Failed to parse FileMetadata");

            return file_state{
                .size = fb_meta->size(),
                .is_directory = fb_meta->type() == sandbox::schemas::filesystem::FileType_Directory,
                .modified_time = fb_meta->modification_time()
            };
        }

    private:
        const sandbox::filesystem_service* m_api;
    };

} // namespace sandbox::sdk

namespace sandbox::api {
    inline std::expected<std::string, std::string> read_text(flecs::world& ecs, const std::string& virtual_path) {
        return sandbox::sdk::filesystem(ecs).read_text(virtual_path);
    }
    inline std::expected<std::vector<std::byte>, std::string> read_binary(flecs::world& ecs, const std::string& virtual_path) {
        return sandbox::sdk::filesystem(ecs).read_binary(virtual_path);
    }
    inline std::expected<void, std::string> write(flecs::world& ecs, const std::string& virtual_path, const std::string& data, bool append = false) {
        return sandbox::sdk::filesystem(ecs).write(virtual_path, data, append);
    }
    inline std::expected<void, std::string> write(flecs::world& ecs, const std::string& virtual_path, const std::vector<std::byte>& data, bool append = false) {
        return sandbox::sdk::filesystem(ecs).write(virtual_path, data, append);
    }
} // namespace sandbox::api
