#pragma once
#include <sandbox/api/abi_types.h>
#include <sandbox/api/payload.h>
#include <glaze/glaze.hpp>
#include <string>
#include <expected>
#include <vector>
#include <sandbox/core/exceptions.h>
#include "sandbox/core/ecs.h"
#include <sandbox/generated/schemas/filesystem_generated.h>

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

        std::expected<void, std::string> mount(const std::string& physical_path, const std::string& virtual_prefix, bool read_only = false) {
            if (m_api->mount(m_api->instance, physical_path.c_str(), virtual_prefix.c_str(), read_only) != 0) {
                return std::unexpected("Failed to mount path");
            }
            return {};
        }

        std::expected<void, std::string> unmount(const std::string& virtual_prefix) {
            if (m_api->unmount(m_api->instance, virtual_prefix.c_str()) != 0) {
                return std::unexpected("Failed to unmount path");
            }
            return {};
        }

        std::expected<std::string, std::string> read_text(const std::string& virtual_path) const {
            payload p;
            if (m_api->read(m_api->instance, virtual_path.c_str(), p.get()) != 0) {
                return std::unexpected("Failed to read file");
            }
            return p.as_string();
        }

        std::expected<std::vector<std::byte>, std::string> read_binary(const std::string& virtual_path) const {
            payload p;
            if (m_api->read(m_api->instance, virtual_path.c_str(), p.get()) != 0) {
                return std::unexpected("Failed to read file");
            }
            auto data_ptr = reinterpret_cast<const std::byte*>(p.data());
            return std::vector<std::byte>(data_ptr, data_ptr + p.size());
        }

        std::expected<void, std::string> write(const std::string& virtual_path, const std::string& data, bool append = false) {
            if (m_api->write(m_api->instance, virtual_path.c_str(), reinterpret_cast<const uint8_t*>(data.data()), data.size(), append) != 0) {
                return std::unexpected("Failed to write to file");
            }
            return {};
        }

        std::expected<void, std::string> write(const std::string& virtual_path, const std::vector<std::byte>& data, bool append = false) {
            if (m_api->write(m_api->instance, virtual_path.c_str(), reinterpret_cast<const uint8_t*>(data.data()), data.size(), append) != 0) {
                return std::unexpected("Failed to write to file");
            }
            return {};
        }

        std::expected<std::vector<std::string>, std::string> list(const std::string& virtual_path, bool recursive = false) const {
            payload p;
            if (m_api->list(m_api->instance, virtual_path.c_str(), recursive, p.get()) != 0) {
                return std::unexpected("Failed to list directory");
            }
            if (!p.data() || p.size() == 0) return std::vector<std::string>{};

            auto fb_list = flatbuffers::GetRoot<sandbox::schemas::StringList>(p.data());
            std::vector<std::string> result;
            if (fb_list && fb_list->items()) {
                result.reserve(fb_list->items()->size());
                for (const auto& item : *fb_list->items()) {
                    result.push_back(item->str());
                }
            }
            return result;
        }

        std::expected<void, std::string> remove(const std::string& virtual_path) {
            if (m_api->remove(m_api->instance, virtual_path.c_str()) != 0) return std::unexpected("Failed to remove");
            return {};
        }

        std::expected<void, std::string> mkdir(const std::string& virtual_path) {
            if (m_api->mkdir(m_api->instance, virtual_path.c_str()) != 0) return std::unexpected("Failed to make directory");
            return {};
        }

        std::expected<void, std::string> rename(const std::string& old_path, const std::string& new_path) {
            if (m_api->rename(m_api->instance, old_path.c_str(), new_path.c_str()) != 0) return std::unexpected("Failed to rename");
            return {};
        }

        std::expected<void, std::string> copy(const std::string& src, const std::string& dst) {
            if (m_api->copy(m_api->instance, src.c_str(), dst.c_str()) != 0) return std::unexpected("Failed to copy");
            return {};
        }

        std::expected<void, std::string> move(const std::string& src, const std::string& dst) {
            if (m_api->move(m_api->instance, src.c_str(), dst.c_str()) != 0) return std::unexpected("Failed to move");
            return {};
        }

        std::expected<std::string, std::string> absolute(const std::string& virtual_path) const {
            payload p;
            if (m_api->absolute(m_api->instance, virtual_path.c_str(), p.get()) != 0) return std::unexpected("Failed to get absolute path");
            return p.as_string();
        }

        struct file_state {
            size_t size;
            bool is_directory;
            uint64_t modified_time;
        };

        std::expected<file_state, std::string> state(const std::string& virtual_path) const {
            payload p;
            if (m_api->state(m_api->instance, virtual_path.c_str(), p.get()) != 0) {
                return std::unexpected("Failed to get state");
            }
            if (!p.data() || p.size() == 0) return std::unexpected("Empty payload for state");
            auto fb_meta = flatbuffers::GetRoot<sandbox::schemas::FileMetadata>(p.data());
            if (!fb_meta) return std::unexpected("Failed to parse FileMetadata");

            return file_state{
                .size = fb_meta->size(),
                .is_directory = fb_meta->type() == sandbox::schemas::FileType_Directory,
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

