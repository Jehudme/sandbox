#pragma once
#include "sandbox/subsystems/filesystem/ifilesystem.h"
#include "sandbox/sdk/payload.h"
#include <glaze/glaze.hpp>
#include <string>
#include <expected>
#include <vector>
#include <stdexcept>

namespace sandbox::sdk {

    class filesystem {
    public:
        explicit filesystem(ifilesystem* api) : m_api(api) {
            if (!m_api) throw std::invalid_argument("Filesystem API pointer is null");
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

        std::expected<void, std::string> mount(const std::string& physical_path, const std::string& virtual_prefix, bool read_only = false) {
            if (m_api->mount(physical_path.c_str(), virtual_prefix.c_str(), read_only) != 0) {
                return std::unexpected("Failed to mount path");
            }
            return {};
        }

        std::expected<void, std::string> unmount(const std::string& virtual_prefix) {
            if (m_api->unmount(virtual_prefix.c_str()) != 0) {
                return std::unexpected("Failed to unmount path");
            }
            return {};
        }

        std::expected<std::string, std::string> read_text(const std::string& virtual_path) const {
            payload p;
            if (m_api->read(virtual_path.c_str(), p.get()) != 0) {
                return std::unexpected("Failed to read file");
            }
            return p.as_string();
        }

        std::expected<std::vector<uint8_t>, std::string> read_binary(const std::string& virtual_path) const {
            payload p;
            if (m_api->read(virtual_path.c_str(), p.get()) != 0) {
                return std::unexpected("Failed to read file");
            }
            return std::vector<uint8_t>(p.data(), p.data() + p.size());
        }

        std::expected<void, std::string> write(const std::string& virtual_path, const std::string& data, bool append = false) {
            if (m_api->write(virtual_path.c_str(), reinterpret_cast<const uint8_t*>(data.data()), data.size(), append) != 0) {
                return std::unexpected("Failed to write to file");
            }
            return {};
        }

        std::expected<void, std::string> write(const std::string& virtual_path, const std::vector<uint8_t>& data, bool append = false) {
            if (m_api->write(virtual_path.c_str(), data.data(), data.size(), append) != 0) {
                return std::unexpected("Failed to write to file");
            }
            return {};
        }

        std::expected<std::vector<std::string>, std::string> list(const std::string& virtual_path, bool recursive = false) const {
            payload p;
            if (m_api->list(virtual_path.c_str(), recursive, p.get()) != 0) {
                return std::unexpected("Failed to list directory");
            }
            std::vector<std::string> result;
            std::string json = p.as_string();
            auto err = glz::read_json(result, json);
            if (err) return std::unexpected("Failed to parse list result");
            return result;
        }

        std::expected<void, std::string> remove(const std::string& virtual_path) {
            if (m_api->remove(virtual_path.c_str()) != 0) return std::unexpected("Failed to remove");
            return {};
        }

        std::expected<void, std::string> mkdir(const std::string& virtual_path) {
            if (m_api->mkdir(virtual_path.c_str()) != 0) return std::unexpected("Failed to make directory");
            return {};
        }

        std::expected<void, std::string> rename(const std::string& old_path, const std::string& new_path) {
            if (m_api->rename(old_path.c_str(), new_path.c_str()) != 0) return std::unexpected("Failed to rename");
            return {};
        }

        std::expected<void, std::string> copy(const std::string& src, const std::string& dst) {
            if (m_api->copy(src.c_str(), dst.c_str()) != 0) return std::unexpected("Failed to copy");
            return {};
        }

        std::expected<void, std::string> move(const std::string& src, const std::string& dst) {
            if (m_api->move(src.c_str(), dst.c_str()) != 0) return std::unexpected("Failed to move");
            return {};
        }

        std::expected<std::string, std::string> absolute(const std::string& virtual_path) const {
            payload p;
            if (m_api->absolute(virtual_path.c_str(), p.get()) != 0) return std::unexpected("Failed to get absolute path");
            return p.as_string();
        }

    private:
        ifilesystem* m_api;
    };

} // namespace sandbox::sdk
